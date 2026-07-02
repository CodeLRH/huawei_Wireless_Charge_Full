/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 手写笔中继无线充电系统 - 主程序(简单版，不使用DMA)
  ******************************************************************************
  * @attention
  *   华为终端硬件精英挑战赛 2025 - 赛题2
  *
  *   系统架构: Phone(发射端) → Pen Case(中继端) → Stylus(接收端)
  *   MCU: STM32F103C8T6 @ 72MHz
  *
  *   IO口分配:
  *   - PA0: Hall传感器输入(上拉)
  *   - PA2: ADC电池电压(IN2)
  *   - PA3: ADC充电电流(IN3)
  *   - PA4: OVP过压保护中断(EXTI4)
  *   - PA5: 充电使能(高有效)
  *   - PA6: 状态LED
  *   - PA9/PA10: USART1调试串口
  *   - PA15: TIM2_CH1 PWM输出(110kHz)
  *   - PB0: ADC NTC温度(IN8)
  *   - PB6/PB7: I2C1(充电IC)
  *
  ******************************************************************************
  */


	//  行号    内容                行数    需要改的频率
	//  ─────────────────────────────────────────────
	//  40-70   全局变量             30     很少改
	//  72-107  函数声明             35     很少改
	//  109-118 printf重定向         10     不改
	//  120-205 ADC函数              90     偶尔改(换电阻/增益)
	//  207-290 控制函数(PWM/LED)    80     偶尔改(调功率)
	//  293-318 I2C通信              25     很少改
	//  320-386 保护函数             65     可能改(调阈值)
	//  388-583 状态机 ★核心★        195     最可能改
	//  587-620 main()               30     很少改
	//  624-701 时钟/回调/错误        75     不改




/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "qi.c"         // 直接包含qi.c

#include <stdio.h>
#include <string.h>
#include <math.h>

/* ========== 全局变量 ========== */

/* 系统状态 */
volatile SystemState_t g_sysState = SYS_IDLE;
volatile ErrorType_t   g_errorType = ERR_NONE;

/* 传感器数据 */
volatile float g_batVoltage = 0.0f;         // 电池电压(V)
volatile float g_chargeCurrent = 0.0f;      // 充电电流(mA)
volatile float g_temperature = 25.0f;       // 温度(°C)

/* ADC滤波缓冲 */
static uint16_t g_adcBuf[3][ADC_FILTER_SIZE] = {0};
static uint8_t  g_adcBufIdx = 0;

/* Hall传感器 */
volatile uint8_t g_hallDetected = 0;

/* 充电控制 */
volatile uint8_t  g_chargePower = CHARGE_POWER_START;
volatile uint32_t g_chargeTimeSec = 0;
volatile uint32_t g_energy_mWs = 0;

/* 定时器 */
volatile uint32_t g_sysTick_ms = 0;

/* 效率计算 */
volatile float g_efficiency = 0.0f;
volatile uint32_t g_inputPower_mW = 0;
volatile uint32_t g_outputPower_mW = 0;
volatile uint32_t g_fodPowerLoss = 0;

/* ========== 函数声明 ========== */

/* 传感器函数 */
void ADC_SampleAll(void);
float ADC_GetVoltage(uint16_t adcVal);
float ADC_GetCurrent(uint16_t adcVal);
float ADC_GetTemperature(uint16_t adcVal);
uint16_t ADC_Filter(uint8_t channel, uint16_t newVal);

/* 控制函数 */
void PWM_SetDuty(uint8_t percent);
void Charge_Start(void);
void Charge_Stop(void);
void Charge_UpdatePower(void);
void LED_SetMode(LED_Mode_t mode);

/* I2C通信 */
uint8_t I2C_WriteReg(uint8_t devAddr, uint8_t reg, uint8_t data);
uint8_t I2C_ReadReg(uint8_t devAddr, uint8_t reg, uint8_t *data);

/* 保护函数 */
uint8_t Protection_CheckOVP(void);
uint8_t Protection_CheckOCP(void);
uint8_t Protection_CheckOTP(void);
uint8_t Protection_CheckFOD(void);
void Protection_HandleError(ErrorType_t err);

/* 状态机 */
void StateMachine_Run(void);
void State_Idle(void);
void State_Detecting(void);
void State_Charging(void);
void State_Full(void);
void State_Error(void);

/* printf重定向 */
#ifdef __GNUC__
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}
#else
int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif

/* ========== ADC函数(轮询模式，简单易懂) ========== */

/**
  * @brief  ADC滑动平均滤波
  * @param  channel: 0=电压, 1=电流, 2=温度
  * @param  newVal: 新采样值
  * @retval 滤波后的值
  */
uint16_t ADC_Filter(uint8_t channel, uint16_t newVal) {
    if (channel >= 3) return 0;
    g_adcBuf[channel][g_adcBufIdx] = newVal;

    uint32_t sum = 0;
    for (int i = 0; i < ADC_FILTER_SIZE; i++) {
        sum += g_adcBuf[channel][i];
    }
    return (uint16_t)(sum / ADC_FILTER_SIZE);
}

/**
  * @brief  采样所有ADC通道(轮询方式)
  * @note   依次读取电压、电流、温度，带滤波
  */
void ADC_SampleAll(void) {
    /* 读取3个通道 */
    uint16_t vRaw = ADC_ReadChannel(ADC_CH_VOLTAGE);
    uint16_t iRaw = ADC_ReadChannel(ADC_CH_CURRENT);
    uint16_t tRaw = ADC_ReadChannel(ADC_CH_TEMPERATURE);

    /* 滤波 */
    uint16_t vFilt = ADC_Filter(0, vRaw);
    uint16_t iFilt = ADC_Filter(1, iRaw);
    uint16_t tFilt = ADC_Filter(2, tRaw);

    /* 更新索引 */
    g_adcBufIdx = (g_adcBufIdx + 1) % ADC_FILTER_SIZE;

    /* 转换为实际值 */
    g_batVoltage = ADC_GetVoltage(vFilt);
    g_chargeCurrent = ADC_GetCurrent(iFilt);
    g_temperature = ADC_GetTemperature(tFilt);
}

/**
  * @brief  ADC值转电压(V)
  * @note   原理: ADC读数→实际电压，考虑电阻分压比
  *         Vadc = adcVal × 3.3V / 4096 (12位ADC，3.3V参考)
  *         Vbat = Vadc × (R1+R2)/R2    (分压电阻还原)
  *         例: R1=R2=10k → 分压比2:1 → Vbat = 2×Vadc
  */
float ADC_GetVoltage(uint16_t adcVal) {
    float vadc = (float)adcVal * 3.3f / 4096.0f;
    return vadc * (VBAT_R1_KOHM + VBAT_R2_KOHM) / VBAT_R2_KOHM;
}

/**
  * @brief  ADC值转电流(mA)
  * @note   原理: 电流流过采样电阻产生压降，经运放放大后采样
  *         Vadc = adcVal × 3.3V / 4096
  *         I = Vadc / (R_sense × Gain) × 1000 (转mA)
  *         例: R=0.1Ω, Gain=10 → 1A对应0.1V×10=1V
  */
float ADC_GetCurrent(uint16_t adcVal) {
    float vadc = (float)adcVal * 3.3f / 4096.0f;
    return (vadc / (CURRENT_SENSE_R_OHM * CURRENT_AMP_GAIN)) * 1000.0f;
}

/**
  * @brief  ADC值转温度(°C) - Steinhart-Hart方程
  * @note   原理: NTC热敏电阻阻值随温度变化，通过分压电路测量
  *         1. ADC值→Vadc→计算NTC电阻值(分压公式)
  *         2. 用Steinhart-Hart简化公式计算温度:
  *            1/T = 1/T0 + (1/B) × ln(R/R0)
  *         参数: R0=10kΩ@25°C, B=3950, 串联电阻10kΩ
  */
float ADC_GetTemperature(uint16_t adcVal) {
    if (adcVal == 0 || adcVal >= 4095) return 99.9f;  // 异常值保护

    float vadc = (float)adcVal * 3.3f / 4096.0f;
    /* 分压公式: Vadc = Vcc × R_ntc / (R_series + R_ntc) → 反推R_ntc */
    float r_ntc = NTC_SERIES_R_KOHM * vadc / (3.3f - vadc);

    /* Steinhart-Hart方程: 1/T = 1/T0 + (1/B)*ln(R/R0) */
    float t_kelvin = 1.0f / (1.0f/NTC_T0_K + (1.0f/NTC_BETA) * logf(r_ntc / NTC_R0_KOHM));
    return t_kelvin - 273.15f;  // 开尔文→摄氏度
}

/* ========== 控制函数 ========== */

/**
  * @brief  设置PWM占空比
  * @param  percent: 0-100
  */
void PWM_SetDuty(uint8_t percent) {
    if (percent > 100) percent = 100;
    if (percent < PWM_DUTY_MIN) percent = PWM_DUTY_MIN;
    if (percent > PWM_DUTY_MAX) percent = PWM_DUTY_MAX;

    uint32_t pulse = (uint32_t)((uint32_t)percent * (PWM_ARR_VALUE + 1) / 100);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);
}

/**
  * @brief  开始充电
  */
void Charge_Start(void) {
    printf("[CHARGE] Start, power=%d%%\r\n", g_chargePower);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    PWM_SetDuty(g_chargePower);
    HAL_GPIO_WritePin(CHG_EN_GPIO_Port, CHG_EN_Pin, GPIO_PIN_SET);
    g_chargeTimeSec = 0;
    g_energy_mWs = 0;
}

/**
  * @brief  停止充电
  */
void Charge_Stop(void) {
    printf("[CHARGE] Stop\r\n");
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
    PWM_SetDuty(0);
    HAL_GPIO_WritePin(CHG_EN_GPIO_Port, CHG_EN_Pin, GPIO_PIN_RESET);
}

/**
  * @brief  更新充电功率(软启动)
  */
void Charge_UpdatePower(void) {
    static uint32_t lastUpdate = 0;
    if ((g_sysTick_ms - lastUpdate) < 1000) return;
    lastUpdate = g_sysTick_ms;

    if (g_chargePower < CHARGE_POWER_MAX) {
        g_chargePower += CHARGE_POWER_STEP;
        if (g_chargePower > CHARGE_POWER_MAX) g_chargePower = CHARGE_POWER_MAX;
        PWM_SetDuty(g_chargePower);
        printf("[CHARGE] Power -> %d%%\r\n", g_chargePower);
    }
}

/**
  * @brief  LED控制
  */
void LED_SetMode(LED_Mode_t mode) {
    static uint32_t lastToggle = 0;
    static uint8_t ledState = 0;

    switch (mode) {
        case LED_OFF:
            HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_RESET);
            break;
        case LED_ON:
            HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);
            break;
        case LED_SLOW_BLINK:
            if ((g_sysTick_ms - lastToggle) >= 500) {
                lastToggle = g_sysTick_ms;
                ledState = !ledState;
                HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin,
                                  ledState ? GPIO_PIN_SET : GPIO_PIN_RESET);
            }
            break;
        case LED_FAST_BLINK:
            if ((g_sysTick_ms - lastToggle) >= 100) {
                lastToggle = g_sysTick_ms;
                ledState = !ledState;
                HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin,
                                  ledState ? GPIO_PIN_SET : GPIO_PIN_RESET);
            }
            break;
    }
}

/* ========== I2C通信函数 ========== */

/**
  * @brief  I2C写寄存器
  * @param  devAddr: 7位设备地址
  * @param  reg: 寄存器地址
  * @param  data: 数据
  * @retval 0=成功, 1=失败
  */
uint8_t I2C_WriteReg(uint8_t devAddr, uint8_t reg, uint8_t data) {
    uint8_t buf[2] = {reg, data};
    if (HAL_I2C_Master_Transmit(&hi2c1, devAddr << 1, buf, 2, 100) != HAL_OK) {
        printf("[I2C] Write fail: 0x%02X\r\n", devAddr);
        return 1;
    }
    return 0;
}

/**
  * @brief  I2C读寄存器
  */
uint8_t I2C_ReadReg(uint8_t devAddr, uint8_t reg, uint8_t *data) {
    if (HAL_I2C_Master_Transmit(&hi2c1, devAddr << 1, &reg, 1, 100) != HAL_OK) return 1;
    if (HAL_I2C_Master_Receive(&hi2c1, devAddr << 1, data, 1, 100) != HAL_OK) return 1;
    return 0;
}

/* ========== 保护函数 ==========
 * 四重安全保护机制，任一触发立即停止充电:
 * 1. OVP: 过压保护 - 电池电压超过4.25V
 * 2. OCP: 过流保护 - 充电电流超过2500mA
 * 3. OTP: 过温保护 - 温度超过80°C
 * 4. FOD: 异物检测 - 发射/接收功率差>500mW(有金属异物吸收能量)
 */

/**
  * @brief  过压保护检查
  * @note   电池电压≥4.25V时触发，防止电池过充爆炸
  */
uint8_t Protection_CheckOVP(void) {
    if (g_batVoltage >= BATTERY_OVP_VOLTAGE) {
        printf("[PROTECT] OVP: %.2fV\r\n", g_batVoltage);
        return 1;
    }
    return 0;
}

/**
  * @brief  过流保护检查
  * @note   充电电流≥2500mA时触发，防止线路过热
  */
uint8_t Protection_CheckOCP(void) {
    if (g_chargeCurrent >= CURRENT_OCP_MA) {
        printf("[PROTECT] OCP: %.1fmA\r\n", g_chargeCurrent);
        return 1;
    }
    return 0;
}

/**
  * @brief  过温保护检查
  * @note   温度≥80°C时触发，防止热失控
  */
uint8_t Protection_CheckOTP(void) {
    if (g_temperature >= TEMP_STOP_THRESHOLD) {
        printf("[PROTECT] OTP: %.1f°C\r\n", g_temperature);
        return 1;
    }
    return 0;
}

/**
  * @brief  异物检测(FOD - Foreign Object Detection)
  * @note   原理: 比较发射功率和接收功率，差值>500mW说明有金属异物吸收能量
  *         条件: 输入功率>100mW(排除待机误判) 且 功率损耗>阈值
  */
uint8_t Protection_CheckFOD(void) {
    if (g_inputPower_mW > 100 && g_fodPowerLoss > FOD_POWER_LOSS_THRESHOLD) {
        printf("[PROTECT] FOD: %lumW loss\r\n", g_fodPowerLoss);
        return 1;
    }
    return 0;
}

/**
  * @brief  保护触发处理 - 停止充电并进入错误状态
  * @param  err: 错误类型(OVP/OCP/OTP/FOD/COMM)
  */
void Protection_HandleError(ErrorType_t err) {
    g_errorType = err;
    g_sysState = SYS_ERROR;          // 切换到错误状态
    Charge_Stop();                   // 停止PWM和充电使能
    printf("[ERROR] Type=%d\r\n", err);
}

/* ========== 状态机函数 ==========
 * 五状态有限状态机(FSM)，每10ms执行一次:
 *
 *   ┌──────┐  Hall检测到   ┌───────────┐  稳定3次   ┌──────────┐
 *   │ IDLE │ ───────────→ │ DETECTING │ ────────→ │ CHARGING │
 *   │ 空闲  │              │   检测中    │           │   充电中   │
 *   └──────┘ ←─────────── └───────────┘ ←──────── └──────────┘
 *       ↑    设备移除/超时       ↑         设备移除      │
 *       │                       │                       │
 *       │    ┌──────────┐       │    ┌──────────┐       │
 *       └─── │  ERROR   │ ←─────┴─── │   FULL   │ ←─────┘
 *            │  错误     │  保护触发   │   充满    │  电压≥4.2V且电流<50mA
 *            └──────────┘            └──────────┘
 */

/**
  * @brief  空闲状态 - 等待设备放入
  * @note   LED慢闪(500ms)，轮询Hall传感器
  *         Hall=1(低电平，上拉被拉低)表示设备已放入
  */
void State_Idle(void) {
    LED_SetMode(LED_SLOW_BLINK);
    g_hallDetected = HAL_GPIO_ReadPin(HALL_SENSOR_GPIO_Port, HALL_SENSOR_Pin);

    if (g_hallDetected) {
        printf("[STATE] Device detected\r\n");
        g_sysState = SYS_DETECTING;   // 检测到设备，进入检测确认状态
    }
}

/**
  * @brief  检测状态 - 确认设备稳定存在
  * @note   需要连续3次(30ms)检测到Hall信号才确认，防止误触发
  *         超时2.5秒未确认则回到空闲状态
  */
void State_Detecting(void) {
    static uint32_t detectStart = 0;     // 检测开始时间
    static uint8_t stableCount = 0;      // 稳定计数器

    LED_SetMode(LED_FAST_BLINK);          // 快闪表示正在检测

    g_hallDetected = HAL_GPIO_ReadPin(HALL_SENSOR_GPIO_Port, HALL_SENSOR_Pin);
    if (g_hallDetected) {
        stableCount++;
        if (stableCount >= STABLE_COUNT_THRESHOLD) {
            /* 连续3次检测到 → 确认设备存在，启动Qi握手 */
            printf("[STATE] Stable, starting Qi\r\n");
            g_sysState = SYS_CHARGING;
            g_chargePower = CHARGE_POWER_START;
            Charge_Start();
            Qi_Start();   // 启动Qi握手(Ping→识别→协商)
            stableCount = 0;
            detectStart = 0;
        }
    } else {
        /* 设备移除 → 回到空闲 */
        stableCount = 0;
        g_sysState = SYS_IDLE;
        detectStart = 0;
    }

    /* 超时保护: 2.5秒内未确认则放弃 */
    if (detectStart == 0) detectStart = g_sysTick_ms;
    if ((g_sysTick_ms - detectStart) > DETECTION_PERIOD_MS * 5) {
        printf("[STATE] Detection timeout\r\n");
        g_sysState = SYS_IDLE;
        detectStart = 0;
    }
}

/**
  * @brief  充电状态 - 主充电循环
  * @note   核心逻辑流程:
  *   1. 检查设备是否移除(Hall)
  *   2. 采样ADC(电压/电流/温度)
  *   3. 计算功率和效率
  *   4. 四重保护检查(OVP/OCP/OTP/FOD)
  *   5. 温度过高时降功率运行
  *   6. 软启动逐步提升功率
  *   7. 判断是否充满(电压≥4.2V且电流<50mA)
  */
void State_Charging(void) {
    LED_SetMode(LED_ON);                 // 常亮表示正在充电

    /* ① Hall检测 - 设备移除立即停止 */
    g_hallDetected = HAL_GPIO_ReadPin(HALL_SENSOR_GPIO_Port, HALL_SENSOR_Pin);
    if (!g_hallDetected) {
        printf("[STATE] Device removed\r\n");
        Charge_Stop();
        g_sysState = SYS_IDLE;
        return;
    }

    /* ② 采样ADC(轮询方式，依次读取3个通道) */
    ADC_SampleAll();

    /* ③ 计算输出功率和FOD功率损耗 */
    g_outputPower_mW = (uint32_t)(g_batVoltage * g_chargeCurrent);
    g_fodPowerLoss = (g_inputPower_mW > g_outputPower_mW) ?
                     (g_inputPower_mW - g_outputPower_mW) : 0;
    g_energy_mWs += g_outputPower_mW;    // 累计充电能量(mWs)

    /* ④ 四重保护检查，任一触发立即停止充电 */
    if (Protection_CheckOVP()) { Protection_HandleError(ERR_OVP); return; }
    if (Protection_CheckOCP()) { Protection_HandleError(ERR_OCP); return; }
    if (Protection_CheckOTP()) { Protection_HandleError(ERR_OTP); return; }
    if (Protection_CheckFOD()) { Protection_HandleError(ERR_FOD); return; }

    /* ⑤ 温度降额: 超过60°C警告阈值时降低功率到30% */
    if (g_temperature >= TEMP_WARN_THRESHOLD) {
        printf("[WARN] High temp: %.1f°C\r\n", g_temperature);
        g_chargePower = CHARGE_POWER_REDUCE;
        PWM_SetDuty(g_chargePower);
    }

    /* ⑥ 软启动: 每秒递增5%功率，从20%逐步升到80% */
    Charge_UpdatePower();

    /* ⑥.5 Qi功率闭环: 自动调节占空比(内部每100ms执行一次) */
    Qi_Process();

    /* ⑦ 充满判断: 电压≥4.2V 且 电流<50mA(涓流阶段) */
    if (g_batVoltage >= BATTERY_FULL_VOLTAGE && g_chargeCurrent < 50) {
        printf("[STATE] Charge complete\r\n");
        Charge_Stop();
        g_sysState = SYS_FULL;
    }

    /* 计算充电效率 = 输出功率/输入功率 × 100% */
    g_efficiency = (g_inputPower_mW > 0) ?
                   (float)g_outputPower_mW * 100.0f / (float)g_inputPower_mW : 0;

    /* 每5秒打印一次充电状态(防止串口刷屏) */
    static uint32_t lastPrint = 0;
    if ((g_sysTick_ms - lastPrint) >= 5000) {
        lastPrint = g_sysTick_ms;
        printf("[CHARGE] V=%.2fV I=%.1fmA T=%.1f°C P=%d%%\r\n",
               g_batVoltage, g_chargeCurrent, g_temperature, g_chargePower);
    }
}

/**
  * @brief  充满状态 - 等待设备移除或电压下降
  * @note   LED灭，监控两个退出条件:
  *   1. 设备移除 → 回到空闲
  *   2. 电压降到4.0V以下 → 重新充电(自放电补偿)
  */
void State_Full(void) {
    LED_SetMode(LED_OFF);                // LED灭表示充满

    g_hallDetected = HAL_GPIO_ReadPin(HALL_SENSOR_GPIO_Port, HALL_SENSOR_Pin);
    if (!g_hallDetected) {
        printf("[STATE] Device removed\r\n");
        g_sysState = SYS_IDLE;           // 设备取出，回到空闲
        return;
    }

    /* 电压降到4.0V以下 → 电池自放电，需要补电 */
    if (g_batVoltage < BATTERY_RECHARGE_VOLTAGE) {
        printf("[STATE] Recharge needed\r\n");
        g_sysState = SYS_CHARGING;
        g_chargePower = CHARGE_POWER_START;
        Charge_Start();
    }
}

/**
  * @brief  错误状态 - 等待设备移除后清除错误
  * @note   LED快闪报警，设备取出后自动清除错误回到空闲
  *         这是一种安全设计: 必须物理移除设备才能复位保护
  */
void State_Error(void) {
    LED_SetMode(LED_FAST_BLINK);         // 快闪表示错误

    g_hallDetected = HAL_GPIO_ReadPin(HALL_SENSOR_GPIO_Port, HALL_SENSOR_Pin);
    if (!g_hallDetected) {
        /* 设备移除 → 清除错误，回到空闲状态 */
        printf("[STATE] Device removed, clear error\r\n");
        g_errorType = ERR_NONE;
        g_sysState = SYS_IDLE;
    }
}

/**
  * @brief  主状态机调度函数 - 每10ms调用一次
  * @note   根据当前状态分发到对应的状态处理函数
  *         default分支将未知状态重置为IDLE(防御性编程)
  */
void StateMachine_Run(void) {
    switch (g_sysState) {
        case SYS_IDLE:      State_Idle();      break;  // 空闲: 等待设备
        case SYS_DETECTING: State_Detecting();  break;  // 检测: 确认设备
        case SYS_CHARGING:  State_Charging();   break;  // 充电: 主循环
        case SYS_FULL:      State_Full();       break;  // 充满: 监控
        case SYS_ERROR:     State_Error();      break;  // 错误: 等待复位
        default:            g_sysState = SYS_IDLE; break; // 异常: 重置
    }
}

/* ========== 主函数 ==========
 * 程序入口，完成以下初始化后进入主循环:
 * 1. HAL库初始化(SysTick等)
 * 2. 系统时钟配置(HSE 8MHz × PLL9 = 72MHz)
 * 3. 外设初始化(GPIO/ADC/I2C/TIM/USART)
 * 4. 主循环: 每10ms执行一次状态机
 */

int main(void)
{
  HAL_Init();                    // HAL库初始化(SysTick=1ms)
  SystemClock_Config();          // 时钟: HSE 8MHz × PLL9 = 72MHz

  /* 外设初始化 */
  MX_GPIO_Init();                // GPIO: Hall/OVP/CHG_EN/LED
  MX_ADC1_Init();                // ADC1: 电压/电流/温度采样
  MX_I2C1_Init();                // I2C1: BQ25601充电IC通信
  MX_TIM2_Init();                // TIM2: 110kHz PWM输出
  MX_USART1_UART_Init();         // USART1: 115200调试串口

  /* 打印启动信息 */
  printf("\r\n=============================\r\n");
  printf("  Wireless Charge System v1.0\r\n");
  printf("  Huawei Competition 2025\r\n");
  printf("=============================\r\n");
  printf("[INIT] Battery: %.2fV OVP\r\n", BATTERY_OVP_VOLTAGE);
  printf("[INIT] OCP: %dmA OTP: %.1f°C\r\n", CURRENT_OCP_MA, TEMP_STOP_THRESHOLD);
  printf("[INIT] PWM: %d (110kHz)\r\n", PWM_ARR_VALUE);

  /* 主循环 - 每10ms执行一次状态机 */
  while (1)
  {
    StateMachine_Run();          // 状态机调度
    HAL_Delay(10);               // 10ms周期(100Hz)
  }
}

/* ========== 系统时钟配置 ==========
 * 时钟树:
 *   HSE(8MHz) → PLL(×9) → SYSCLK(72MHz)
 *   SYSCLK → AHB(72MHz) → APB1(36MHz, /2) → TIM2(72MHz, APB1定时器×2)
 *                       → APB2(72MHz) → ADC(12MHz, /6)
 */

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /* HSE 8MHz + PLL ×9 = 72MHz系统时钟 */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;        // 8MHz × 9 = 72MHz
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

  /* 总线时钟分配 */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;  // PLL输出作为系统时钟
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;   // AHB = 72MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;     // APB1 = 36MHz(最大)
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;     // APB2 = 72MHz
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();

  /* ADC时钟 = PCLK2/6 = 72/6 = 12MHz(最大14MHz) */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) Error_Handler();
}

/* ========== 回调函数 ========== */

/**
  * @brief  SysTick回调 - 每1ms调用一次
  * @note   由HAL库的SysTick_Handler()调用，提供毫秒级时间基准
  */
void HAL_SYSTICK_Callback(void) {
    g_sysTick_ms++;
}

/**
  * @brief  GPIO外部中断回调
  * @note   两个中断源:
  *   1. PA4(OVP): 硬件过压检测，立即停止充电(最高优先级保护)
  *   2. PA0(HALL): Hall传感器边沿中断，实时更新设备状态
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == OVP_PIN_Pin) {
        /* OVP硬件中断: 最高优先级保护，立即停止充电 */
        printf("[INT] OVP triggered!\r\n");
        Protection_HandleError(ERR_OVP);
    }
    if (GPIO_Pin == HALL_SENSOR_Pin) {
        /* Hall传感器边沿中断: 实时更新设备检测状态 */
        g_hallDetected = HAL_GPIO_ReadPin(HALL_SENSOR_GPIO_Port, HALL_SENSOR_Pin);
    }
}

/* ========== 错误处理 ========== */

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
    printf("Assert: %s:%lu\r\n", file, line);
}
#endif
