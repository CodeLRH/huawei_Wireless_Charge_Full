# STM32CubeMX 配置指南
# 手写笔中继无线充电系统

## 一、芯片选型

根据赛题要求使用STM32F1系列，推荐以下芯片：

| 芯片型号 | 推荐度 | 说明 |
|---------|--------|------|
| **STM32F103C8T6** | ⭐⭐⭐⭐⭐ | 最常用，64KB Flash，20KB RAM，LQFP48封装 |
| STM32F103RCT6 | ⭐⭐⭐⭐ | 256KB Flash，48KB RAM，LQFP64封装 |
| STM32F103ZET6 | ⭐⭐⭐ | 512KB Flash，64KB RAM，LQFP144封装 |

**推荐选择：STM32F103C8T6**（蓝色药丸板最常用，资源足够）

---

## 二、外设需求分析

根据赛题，系统需要以下外设：

| 外设 | 功能 | 引脚需求 |
|-----|------|---------|
| **GPIO输入** | Hall传感器检测（笔是否放入） | 1个引脚 |
| **PWM输出** | 驱动无线充电发射线圈（110kHz） | 1个引脚 |
| **ADC** | 电池电压检测 | 1个引脚 |
| **I2C** | 充电管理IC通信（BQ25601等） | 2个引脚 |
| **UART** | 串口调试输出 | 2个引脚 |
| **GPIO输出** | 充电使能控制 | 1个引脚 |
| **外部中断** | 过压保护（可选） | 1个引脚 |

---

## 三、引脚分配方案（无冲突）

### 3.1 引脚分配表

| 引脚 | 功能 | 外设/模式 | 说明 |
|-----|------|----------|------|
| **PA0** | Hall传感器输入 | GPIO_Input | 检测手写笔是否放入 |
| **PA1** | PWM输出 | TIM2_CH1 | 驱动发射线圈（110kHz） |
| **PA2** | ADC输入 | ADC1_CH2 | 电池电压采样 |
| **PA3** | 充电使能 | GPIO_Output | 控制发射模块开关 |
| **PA4** | 过压保护 | GPIO_EXTI4 | 外部中断（可选） |
| **PA9** | UART TX | USART1_TX | 串口调试发送 |
| **PA10** | UART RX | USART1_RX | 串口调试接收 |
| **PB6** | I2C SCL | I2C1_SCL | 充电管理IC时钟 |
| **PB7** | I2C SDA | I2C1_SDA | 充电管理IC数据 |
| **PA13** | SWDIO | SYS_JTMS-SWDIO | 调试接口（保留） |
| **PA14** | SWCLK | SYS_JTCK-SWCLK | 调试接口（保留） |

### 3.2 引脚冲突检查

```
✅ 无冲突！所有引脚功能独立
✅ 保留SWD调试接口（PA13/PA14）
✅ TIM2_CH1使用PA1（非PA0，避免与Hall传感器冲突）
```

### 3.3 接线示意图

```
                    STM32F103C8T6
                   ┌──────────────┐
                   │              │
    Hall传感器 OUT │─→ PA0        │
                   │              │
    发射模块 EN    │←─ PA1 (PWM)  │  TIM2_CH1
                   │              │
    电池分压       │─→ PA2        │  ADC1_CH2
                   │              │
    发射模块 VCC   │←─ PA3        │  GPIO_OUT
                   │              │
    OVP检测        │─→ PA4        │  GPIO_EXTI
                   │              │
    UART TX        │←─ PA9        │  USART1_TX
                   │              │
    UART RX        │─→ PA10       │  USART1_RX
                   │              │
    I2C SCL        │←→ PB6        │  I2C1_SCL
                   │              │
    I2C SDA        │←→ PB7        │  I2C1_SDA
                   │              │
                   └──────────────┘
```

---

## 四、STM32CubeMX 详细配置步骤

### 4.1 新建工程

1. 打开 STM32CubeMX
2. 点击 **ACCESS TO MCU SELECTOR**
3. 搜索并选择 **STM32F103C8Tx**
4. 点击 **Start Project**

### 4.2 配置系统时钟（RCC）

1. 左侧 **Pinout & Configuration** → **System Core** → **RCC**
2. **High Speed Clock (HSE)**: 选择 **Crystal/Ceramic Resonator**
3. **Low Speed Clock (LSE)**: 保持默认（Disable）

### 4.3 配置时钟树

1. 点击顶部 **Clock Configuration** 标签
2. 设置如下：
   ```
   HSE = 8 MHz（外部晶振）
   PLL Source = HSE
   PLL Multiplier = ×9
   System Clock = 72 MHz
   AHB Prescaler = /1
   APB1 Prescaler = /2  （APB1最大36MHz）
   APB2 Prescaler = /1
   ```
3. 点击 **OK** 确认自动配置

### 4.4 配置 GPIO（Hall传感器 + 充电使能）

#### Hall传感器输入（PA0）
1. 在芯片图上点击 **PA0**
2. 选择 **GPIO_Input**
3. 左侧 **System Core** → **GPIO** → 点击 **PA0**
4. 配置参数：
   ```
   GPIO mode: Input mode
   GPIO Pull-up/Pull-down: Pull-up（传感器无磁场时高电平）
   User Label: HALL_SENSOR
   ```

#### 充电使能输出（PA3）
1. 在芯片图上点击 **PA3**
2. 选择 **GPIO_Output**
3. 左侧 **GPIO** → 点击 **PA3**
4. 配置参数：
   ```
   GPIO output level: Low（初始关闭）
   GPIO mode: Output Push Pull
   Maximum output speed: Low
   User Label: CHG_EN
   ```

#### 过压保护中断（PA4，可选）
1. 在芯片图上点击 **PA4**
2. 选择 **GPIO_EXTI4**
3. 左侧 **GPIO** → 点击 **PA4**
4. 配置参数：
   ```
   GPIO mode: External Interrupt Mode with Rising/Falling edge trigger detection
   GPIO Pull-up/Pull-down: No pull-up and no pull-down
   User Label: OVP_PIN
   ```
5. 左侧 **NVIC** → 勾选 **EXTI line[4] interrupt** → 设置优先级

### 4.5 配置 TIM2（PWM输出）

1. 左侧 **Timers** → **TIM2**
2. **Clock Source**: 选择 **Internal Clock**
3. **Channel1**: 选择 **PWM Generation CH1**
4. 配置参数：
   ```
   Prescaler (PSC): 0
   Counter Period (ARR): 654
   Pulse (CCR1): 327（初始50%占空比）
   PWM Mode: PWM mode 1
   CH Polarity: High
   ```
5. **计算说明**：
   ```
   PWM频率 = 72MHz / (0+1) / (654+1) = 109,909 Hz ≈ 110 kHz
   初始占空比 = 327 / 655 × 100% = 50%
   ```

### 4.6 配置 ADC1（电压采样）

1. 左侧 **Analog** → **ADC1**
2. 勾选 **IN2**（对应PA2）
3. 配置参数：
   ```
   Rank: 1
   Sampling Time: 239.5 Cycles（采样时间长，精度高）
   ```
4. **ADC Settings**:
   ```
   Scan Conversion Mode: Disabled
   Continuous Conversion Mode: Disabled
   Discontinuous Conversion Mode: Disabled
   End of Conversion Flag: End of single conversion
   Data Alignment: Right Alignment
   Number of Conversion: 1
   ```

### 4.7 配置 I2C1（充电管理IC）

1. 左侧 **Connectivity** → **I2C1**
2. **I2C**: 选择 **I2C**
3. 配置参数：
   ```
   I2C Speed Mode: Standard Mode
   I2C Clock Speed: 100000 Hz（100kHz）
   ```

### 4.8 配置 USART1（串口调试）

1. 左侧 **Connectivity** → **USART1**
2. **Mode**: 选择 **Asynchronous**（异步模式）
3. 配置参数：
   ```
   Baud Rate: 115200 Bits/s
   Word Length: 8 Bits
   Stop Bits: 1
   Parity: None
   Data Direction: Receive and Transmit
   Over Sampling: 16 Samples
   ```

### 4.9 配置 NVIC（中断优先级）

1. 左侧 **System Core** → **NVIC**
2. 配置优先级（数字越小优先级越高）：
   ```
   EXTI line[4 interrupt]: Preemption Priority = 1（OVP保护）
   USART1 global interrupt: Preemption Priority = 2
   ```

### 4.10 配置 SYS（调试接口）

1. 左侧 **System Core** → **SYS**
2. **Debug**: 选择 **Serial Wire**（SWD调试）
3. **Timebase Source**: 选择 **SysTick**

---

## 五、生成代码

### 5.1 项目设置

1. 点击顶部 **Project Manager** 标签
2. 填写配置：
   ```
   Project Name: Wireless_Charge
   Project Location: 选择保存路径
   Toolchain/IDE: STM32CubeIDE
   ```
3. **Code Generator** 设置：
   ```
   ☑️ Generate peripheral initialization as a pair of '.c/.h' files per peripheral
   ☑️ Keep user code when re-generating
   ☑️ Delete previously generated files when not re-generated
   ```

### 5.2 生成代码

1. 点击右上角 **GENERATE CODE**
2. 等待生成完成
3. 点击 **Open Project** 打开STM32CubeIDE

---

## 六、STM32CubeIDE 编译配置

### 6.1 导入工程后检查

1. 工程目录结构：
   ```
   Wireless_Charge/
   ├── Core/
   │   ├── Inc/          # 头文件
   │   │   ├── main.h
   │   │   ├── stm32f1xx_hal_conf.h
   │   │   └── ...
   │   └── Src/          # 源文件
   │       ├── main.c
   │       ├── stm32f1xx_hal_msp.c
   │       ├── stm32f1xx_it.c
   │       └── ...
   ├── Drivers/          # HAL库
   └── Wireless_Charge.ioc  # CubeMX配置文件
   ```

### 6.2 编译工程

1. 菜单栏 → **Project** → **Build All** (Ctrl+B)
2. 或者点击工具栏的锤子图标🔨
3. 确认 **Console** 窗口显示：
   ```
   Build Finished: 0 errors, 0 warnings
   ```

### 6.3 下载调试

1. 连接ST-Link调试器
2. 菜单栏 → **Run** → **Debug As** → **STM32 C/C++ Application**
3. 首次调试会提示配置调试器，选择 **ST-Link (ST-LINK)**

---

## 七、在 main.c 中添加用户代码

### 7.1 包含头文件

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */
```

### 7.2 重定向printf到串口

```c
/* USER CODE BEGIN 0 */
#ifdef __GNUC__
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#else
int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif
/* USER CODE END 0 */
```

### 7.3 ADC读取函数

```c
/* USER CODE BEGIN 4 */
float Read_Battery_Voltage(void) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    uint32_t raw = HAL_ADC_GetValue(&hadc1);
    float v_adc = raw * 3.3f / 4095.0f;
    // 电阻分压：R1=10k, R2=10k，分压比=2
    float v_bat = v_adc * 2.0f;
    return v_bat;
}

void Set_TX_Power(uint8_t percent) {
    if (percent > 100) percent = 100;
    uint16_t ccr = (uint16_t)((uint32_t)percent * 655 / 100);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccr);
}
/* USER CODE END 4 */
```

### 7.4 主循环逻辑

```c
/* USER CODE BEGIN 2 */
// 启动PWM
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
// 初始关闭发射
HAL_GPIO_WritePin(CHG_EN_GPIO_Port, CHG_EN_Pin, GPIO_PIN_RESET);
Set_TX_Power(0);
printf("System Init OK\r\n");
/* USER CODE END 2 */

/* Infinite loop */
/* USER CODE BEGIN WHILE */
while (1) {
    // 1. 读取Hall传感器
    uint8_t pen_detected = (HAL_GPIO_ReadPin(HALL_SENSOR_GPIO_Port, HALL_SENSOR_Pin) == GPIO_PIN_RESET);

    if (pen_detected) {
        // 笔已放入 → 启动充电
        HAL_GPIO_WritePin(CHG_EN_GPIO_Port, CHG_EN_Pin, GPIO_PIN_SET);
        Set_TX_Power(50);  // 50%功率

        // 2. 读取电池电压
        float v_bat = Read_Battery_Voltage();

        // 3. 串口输出状态
        printf("CHG ON: V_bat = %.2f V\r\n", v_bat);

        // 4. 过压保护
        if (v_bat > 4.25f) {
            HAL_GPIO_WritePin(CHG_EN_GPIO_Port, CHG_EN_Pin, GPIO_PIN_RESET);
            Set_TX_Power(0);
            printf("[OVP] Over Voltage! Charging stopped.\r\n");
        }
    } else {
        // 笔未放入 → 停止充电
        HAL_GPIO_WritePin(CHG_EN_GPIO_Port, CHG_EN_Pin, GPIO_PIN_RESET);
        Set_TX_Power(0);
        printf("No pen detected.\r\n");
    }

    HAL_Delay(500);  // 500ms检测周期
}
/* USER CODE END WHILE */
```

### 7.5 过压保护中断回调（可选）

```c
/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == OVP_PIN_Pin) {
        // 紧急关断
        HAL_GPIO_WritePin(CHG_EN_GPIO_Port, CHG_EN_Pin, GPIO_PIN_RESET);
        Set_TX_Power(0);
        printf("[OVP IRQ] Emergency shutdown!\r\n");
    }
}
/* USER CODE END 4 */
```

---

## 八、常见问题与解决

### 8.1 编译错误

| 错误 | 原因 | 解决方案 |
|-----|------|---------|
| `undefined reference to '__io_putchar'` | printf重定向未实现 | 添加重定向代码 |
| `HAL_ADC_Start undeclared` | ADC未在CubeMX中使能 | 检查CubeMX配置 |
| `TIM2 not declared` | 定时器未配置 | 重新生成代码 |

### 8.2 引脚冲突检查清单

```
□ PA0 是否同时被多个功能占用？
□ PA13/PA14 是否保留给SWD调试？
□ I2C引脚是否正确选择（PB6/PB7）？
□ ADC通道与引脚是否对应？
□ 定时器通道与引脚是否对应？
```

### 8.3 时钟配置验证

在CubeMX的Clock Configuration页面确认：
- HCLK = 72 MHz
- APB1 Timer = 72 MHz（TIM2在APB1上）
- APB2 Timer = 72 MHz

---

## 九、硬件接线检查表

### 9.1 核心接线

| 序号 | 信号 | STM32引脚 | 外部模块 | 备注 |
|-----|------|----------|---------|------|
| 1 | Hall OUT | PA0 | Hall传感器A3144 | 需上拉电阻 |
| 2 | PWM | PA1 | 发射模块EN | 110kHz方波 |
| 3 | V_BAT分压 | PA2 | 电池电压分压点 | R1=10k,R2=10k |
| 4 | CHG_EN | PA3 | 发射模块使能 | 高电平有效 |
| 5 | TX | PA9 | USB转串口RX | 调试用 |
| 6 | RX | PA10 | USB转串口TX | 调试用 |
| 7 | SCL | PB6 | 充电管理IC | 100kHz |
| 8 | SDA | PB7 | 充电管理IC | 100kHz |

### 9.2 电源接线

| 接线点 | 电压 | 说明 |
|-------|------|------|
| STM32 VCC | 3.3V | 板载稳压器输出 |
| Hall传感器 VCC | 3.3V | 与MCU共地 |
| 发射模块 VCC | 5V | 程控电源供电 |
| 锂电池 | 3.7V | TP4056输出 |

---

## 十、调试技巧

### 10.1 串口调试命令

在串口助手中观察输出：
```
System Init OK
No pen detected.
No pen detected.
CHG ON: V_bat = 3.85 V
CHG ON: V_bat = 3.87 V
CHG ON: V_bat = 3.92 V
...
[OVP] Over Voltage! Charging stopped.
```

### 10.2 示波器测量点

| 测量点 | 目标波形 | 预期值 |
|-------|---------|-------|
| PA1引脚 | PWM方波 | 110kHz, 3.3Vpp |
| 发射线圈两端 | 正弦波 | 约110kHz |
| 电池电压 | 直流 | 3.0-4.2V |

### 10.3 万用表测量点

| 测量点 | 预期值 | 说明 |
|-------|-------|------|
| PA0对地 | 3.3V/0V | 笔不在/笔放入 |
| PA3对地 | 3.3V/0V | 充电开/关 |
| 电池两端 | 3.0-4.2V | 充电状态 |

---

## 十一、参考资料

1. STM32F103数据手册：DS5319
2. STM32F103参考手册：RM0008
3. STM32CubeMX用户手册：UM1718
4. HAL库用户手册：UM1850
5. 赛题资料：手写笔无线充电_外设功能分析.docx
6. 赛题资料：手写笔无线充电_硬件搭建方案推测.docx

---

**文档版本**: v1.0
**更新日期**: 2026-06-28
**适用芯片**: STM32F103C8T6
