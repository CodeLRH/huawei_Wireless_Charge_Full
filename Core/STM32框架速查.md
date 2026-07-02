# STM32无线充电框架速查 — 现场调试手册

> 比赛现场用，快速查找外设配置、IO口、主函数结构

---

## 一、CubeMX外设配置速查

### 1.1 时钟配置

```
HSE(8MHz晶振) → PLL(×9) → SYSCLK=72MHz
AHB = 72MHz (/1)
APB1 = 36MHz (/2) → TIM2时钟 = 72MHz (APB1定时器自动×2)
APB2 = 72MHz (/1) → ADC时钟 = 12MHz (/6)
```

### 1.2 GPIO配置

| CubeMX设置 | 引脚 | 模式 | 上下拉 | 速度 | 标签 |
|-----------|------|------|--------|------|------|
| Hall传感器 | PA0 | Input Mode | Pull-up | - | HALL_SENSOR |
| OVP中断 | PA4 | External Interrupt Mode (Rising) | No pull | - | OVP_PIN |
| 充电使能 | PA5 | Output Push Pull | No pull | Low | CHG_EN |
| 状态LED | PA6 | Output Push Pull | No pull | Low | STATUS_LED |

### 1.3 ADC配置

```
ADC1 → 单通道、单次转换、软件触发、轮询模式
Scan Conversion Mode: Disabled
Continuous Conversion Mode: Disabled
External Trig Conv: Software Start
Data Alignment: Right
Number of Conversion: 1

通道分配(运行时动态切换):
  Channel 2 (PA2): 电池电压 → Sampling Time 239.5 Cycles
  Channel 3 (PA3): 充电电流 → Sampling Time 239.5 Cycles
  Channel 8 (PB0): NTC温度  → Sampling Time 239.5 Cycles
```

### 1.4 TIM2 PWM配置

```
TIM2 → Channel 1 → PWM Mode 1
Prescaler: 0
Counter Period (ARR): 654
Pulse (CCR1): 0
CH Polarity: High
→ 频率 = 72MHz / 1 / 655 ≈ 110kHz

AFIO重映射: TIM2 Partial Remap 1
→ CH1输出到PA15(默认JTDI，需禁用JTAG)
```

### 1.5 I2C配置

```
I2C1 → Master Mode
Clock Speed: 100000 (100kHz)
Duty Cycle: Tlow/Thigh = 2
Addressing Mode: 7-bit
引脚: PB6(SCL), PB7(SDA) → 开漏复用输出
```

### 1.6 USART配置

```
USART1 → 异步模式
Baud Rate: 115200
Word Length: 8 Bits
Stop Bits: 1
Parity: None
引脚: PA9(TX) → 复用推挽, PA10(RX) → 浮空输入
NVIC中断: 使能(优先级2,0)
```

### 1.7 NVIC中断优先级

| 优先级 | 中断源 | 说明 |
|--------|--------|------|
| 1,0 | EXTI4 (PA4 OVP) | 最高，硬件过压立即响应 |
| 2,0 | USART1 | 串口 |
| 15,0 | SysTick | 系统心跳(最低) |

---

## 二、引脚分配总表

```
PA0  ← Hall传感器    (输入上拉，检测设备放入)
PA2  ← ADC电压       (模拟输入，电池电压分压采样)
PA3  ← ADC电流       (模拟输入，采样电阻+运放)
PA4  ← OVP中断       (EXTI上升沿，硬件过压保护)
PA5  → 充电使能      (推挽输出，高有效)
PA6  → 状态LED       (推挽输出)
PA9  → USART1_TX     (复用推挽，调试串口)
PA10 ← USART1_RX     (浮空输入)
PA15 → TIM2_CH1 PWM  (复用推挽，110kHz驱动，需AFIO重映射)
PB0  ← ADC温度       (模拟输入，NTC分压)
PB6  → I2C1_SCL      (开漏复用，充电IC时钟)
PB7  ↔ I2C1_SDA      (开漏复用，充电IC数据)
PA13 ← SWDIO         (SWD调试，保留)
PA14 ← SWCLK         (SWD调试，保留)
```

---

## 三、主函数框架

```c
#include "main.h"
#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "qi.h"        // ← Qi协议模块

int main(void)
{
    /* === 第1步: 系统初始化 === */
    HAL_Init();              // HAL库初始化(SysTick=1ms)
    SystemClock_Config();    // 时钟: HSE 8MHz × PLL9 = 72MHz

    /* === 第2步: 外设初始化 === */
    MX_GPIO_Init();          // GPIO: Hall/OVP/CHG_EN/LED
    MX_ADC1_Init();          // ADC1: 电压/电流/温度(轮询模式)
    MX_I2C1_Init();          // I2C1: BQ25601充电IC(100kHz)
    MX_TIM2_Init();          // TIM2: 110kHz PWM(CH1→PA15)
    MX_USART1_UART_Init();   // USART1: 115200调试串口

    /* === 第3步: 协议初始化 === */
    Qi_Init();               // Qi协议状态机初始化

    /* === 第4步: 打印启动信息 === */
    printf("\r\n=== Wireless Charge System v1.0 ===\r\n");
    printf("[INIT] OVP=%.2fV OCP=%dmA OTP=%.0f°C\r\n",
           BATTERY_OVP_VOLTAGE, CURRENT_OCP_MA, TEMP_STOP_THRESHOLD);

    /* === 第5步: 主循环 === */
    while (1)
    {
        StateMachine_Run();  // 状态机(10ms周期)
        HAL_Delay(10);
    }
}
```

---

## 四、状态机框架(集成Qi协议)

```
┌──────────┐  Hall检测到  ┌───────────┐  Qi握手成功  ┌──────────┐
│   IDLE   │ ──────────→ │ DETECTING │ ──────────→ │ CHARGING │
│  空闲     │             │  检测中    │              │  充电中   │
│ LED慢闪   │             │ LED快闪    │              │ LED常亮   │
└──────────┘ ←────────── └───────────┘ ←────────── └──────────┘
     ↑      设备移除/超时       ↑         设备移除        │
     │                                                  │
     │      ┌──────────┐        │     ┌──────────┐      │
     └─────│  ERROR    │ ←──────┴──── │   FULL   │ ←────┘
           │  错误      │  保护触发    │   充满    │  充满确认
           │ LED快闪    │             │ LED灭     │
           └──────────┘             └──────────┘
```

### 各状态代码框架

```c
void State_Idle(void) {
    LED_SetMode(LED_SLOW_BLINK);
    // 轮询Hall传感器
    if (HAL_GPIO_ReadPin(HALL_SENSOR_GPIO_Port, HALL_SENSOR_Pin)) {
        g_sysState = SYS_DETECTING;
    }
}

void State_Detecting(void) {
    LED_SetMode(LED_FAST_BLINK);
    // 连续3次确认Hall信号
    if (stableCount >= 3) {
        // ★ Qi握手流程
        if (Qi_Handshake()) {
            g_sysState = SYS_CHARGING;
        }
    }
}

void State_Charging(void) {
    LED_SetMode(LED_ON);
    // ① 检查设备移除
    // ② ADC采样(电压/电流/温度)
    // ③ 功率计算
    // ④ 四重保护(去抖)
    // ⑤ 温度降额
    // ⑥ Qi功率传输循环
    Qi_PowerTransfer_Loop();
    // ⑦ 充满判断(持续3秒)
}

void State_Full(void) {
    LED_SetMode(LED_OFF);
    Qi_End_Power_Transfer(QI_END_CHARGE_COMPLETE);
    // 等待设备移除或电压下降
}

void State_Error(void) {
    LED_SetMode(LED_FAST_BLINK);
    Qi_End_Power_Transfer(QI_END_UNKNOWN);
    // 等待设备移除清除错误
}
```

---

## 五、ADC读取代码模板

```c
// 读取单个ADC通道(轮询模式)
uint16_t Read_ADC(uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    uint16_t val = (uint16_t)HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return val;
}

// ADC值转电压
float ADC_to_Voltage(uint16_t adc) {
    return adc * 3.3f / 4095.0f * 2.0f;  // 分压比2:1
}

// ADC值转电流(mA)
float ADC_to_Current(uint16_t adc) {
    return adc * 3.3f / 4095.0f / 0.1f / 10.0f * 1000.0f;
}

// ADC值转温度(°C) - Steinhart-Hart
float ADC_to_Temperature(uint16_t adc) {
    float v = adc * 3.3f / 4095.0f;
    float r = 10.0f * v / (3.3f - v);
    return 1.0f / (1.0f/298.15f + (1.0f/3950.0f)*logf(r/10.0f)) - 273.15f;
}
```

---

## 六、PWM控制代码模板

```c
// 设置占空比(0-100%)
void PWM_SetDuty(uint8_t percent) {
    if (percent > 100) percent = 100;
    if (percent < 10) percent = 10;   // 最小10%
    if (percent > 90) percent = 90;   // 最大90%
    uint32_t pulse = (uint32_t)percent * 655 / 100;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);
}

// 启动PWM
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

// 停止PWM
HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
```

---

## 七、I2C通信代码模板

```c
// 写寄存器(适用于BQ25601等充电IC)
uint8_t I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data) {
    return HAL_I2C_Mem_Write(&hi2c1, addr<<1, reg,
                             I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

// 读寄存器
uint8_t I2C_ReadReg(uint8_t addr, uint8_t reg, uint8_t *data) {
    return HAL_I2C_Mem_Read(&hi2c1, addr<<1, reg,
                            I2C_MEMADD_SIZE_8BIT, data, 1, 100);
}

// I2C扫描(调试用，找出总线上所有设备)
void I2C_Scan(void) {
    for (uint8_t addr = 1; addr < 127; addr++) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, addr<<1, 1, 10) == HAL_OK) {
            printf("  Found: 0x%02X\r\n", addr);
        }
    }
}
```

---

## 八、保护函数代码模板

```c
// 过压保护(OVP) - 硬件中断+软件双重检测
uint8_t Check_OVP(void) {
    return (g_batVoltage >= 4.25f);
}

// 过流保护(OCP) - 带去抖(连续3次)
uint8_t Check_OCP(void) {
    static uint8_t cnt = 0;
    if (g_chargeCurrent >= 2500.0f) { cnt++; if(cnt>=3){cnt=0;return 1;} }
    else { cnt = 0; }
    return 0;
}

// 过温保护(OTP) - 带去抖
uint8_t Check_OTP(void) {
    static uint8_t cnt = 0;
    if (g_temperature >= 80.0f) { cnt++; if(cnt>=3){cnt=0;return 1;} }
    else { cnt = 0; }
    return 0;
}

// 异物检测(FOD) - 功率损耗>500mW
uint8_t Check_FOD(void) {
    uint32_t loss = (g_inputPower_mW > g_outputPower_mW) ?
                    (g_inputPower_mW - g_outputPower_mW) : 0;
    return (g_inputPower_mW > 100 && loss > 500);
}
```

---

## 九、printf重定向模板

```c
// GCC编译器
#ifdef __GNUC__
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}
#endif

// Keil/IAR编译器
int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
```

---

## 十、现场调试流程

```
1. 烧录代码
   ↓
2. 打开串口助手(115200 8N1)
   ↓
3. 观察启动信息(确认外设初始化成功)
   ↓
4. 放入设备 → 观察状态机切换(IDLE→DETECTING→CHARGING)
   ↓
5. 示波器量PA15 → 确认110kHz PWM输出
   ↓
6. 万用表量PA2 → 对比ADC读数是否准确
   ↓
7. 串口观察充电日志(V/I/T/Power/Efficiency)
   ↓
8. 测试保护: 加热NTC(OTP)、加大负载(OCP)、放金属片(FOD)
   ↓
9. 测试充满: 等电压到4.2V，观察是否停止充电
```

---

## 十一、常见问题排查

| 现象 | 可能原因 | 排查方法 |
|------|---------|---------|
| 串口无输出 | USART1没初始化/波特率错 | 检查CubeMX配置，确认PA9/PA10 |
| PWM没输出 | TIM2没启动/引脚错 | 示波器量PA15，检查AFIO重映射 |
| ADC读数全是0 | ADC没初始化/通道错 | 万用表量PA2/PB0，对比ADC值 |
| I2C通信失败 | 地址错/时序不对 | 先跑I2C_Scan()找设备 |
| Hall检测不到 | 上拉没配/接线错 | 万用表量PA0电平 |
| OVP误触发 | 比较器阈值太低 | 检查PA4外部电路 |
| 充电不启动 | 状态机卡在某个状态 | 串口打印当前状态 |
| 效率很低 | LC不谐振/线圈没对齐 | 示波器量发射/接收波形 |
