/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : 手写笔中继无线充电系统 - 头文件
  ******************************************************************************
  * @attention
  *   华为终端硬件精英挑战赛 2025 - 赛题2
  *   系统架构: Phone(发射端) → Pen Case(中继端) → Stylus(接收端)
  ******************************************************************************
  */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* ========== 比赛可配置参数区 ========== */
/* 比赛时只需修改这里，快速调整系统行为 */

/* 电池参数 */
#define BATTERY_FULL_VOLTAGE        4.2f    // 满电电压(V)
#define BATTERY_OVP_VOLTAGE         4.25f   // 过压保护阈值(V)
#define BATTERY_LOW_VOLTAGE         3.0f    // 低电压阈值(V)
#define BATTERY_RECHARGE_VOLTAGE    4.0f    // 再充电电压(V)

/* 温度保护阈值 */
#define TEMP_WARN_THRESHOLD         60.0f   // 温度警告阈值(°C)
#define TEMP_STOP_THRESHOLD         80.0f   // 温度停止充电阈值(°C)
#define TEMP_RESUME_THRESHOLD       50.0f   // 温度恢复充电阈值(°C)

/* 电流保护 */
#define CURRENT_OCP_MA              2500    // 过流保护阈值(mA)
#define CURRENT_PRECHARGE_MA        100     // 预充电流(mA)
#define CURRENT_CHARGE_MA           1000    // 恒流充电电流(mA)

/* 采样电阻与分压电阻 */
#define VBAT_R1_KOHM                10.0f   // 电池电压分压上臂(kΩ)
#define VBAT_R2_KOHM                10.0f   // 电池电压分压下臂(kΩ)
#define CURRENT_SENSE_R_OHM         0.1f    // 电流采样电阻(Ω)
#define CURRENT_AMP_GAIN            10.0f   // 电流放大倍数

/* NTC热敏电阻参数 */
#define NTC_R0_KOHM                 10.0f   // NTC在25°C时的电阻(kΩ)
#define NTC_T0_K                    298.15f // 参考温度25°C = 298.15K
#define NTC_BETA                    3950.0f // B值
#define NTC_SERIES_R_KOHM           10.0f   // 串联电阻(kΩ)

/* PWM参数 (TIM2_CH1 - 110kHz) */
#define PWM_ARR_VALUE               654     // ARR值 (72MHz/110kHz - 1)
#define PWM_DUTY_DEFAULT            50      // 默认占空比(%)
#define PWM_DUTY_MIN                10      // 最小占空比(%)
#define PWM_DUTY_MAX                90      // 最大占空比(%)

/* 充电功率控制 */
#define CHARGE_POWER_START          20      // 初始功率(%)
#define CHARGE_POWER_MAX            80      // 最大功率(%)
#define CHARGE_POWER_STEP           5       // 功率递增步长(%)
#define CHARGE_POWER_REDUCE         30      // 异常时功率降低(%)

/* 检测参数 */
#define DETECTION_PERIOD_MS         500     // 检测周期(ms)
#define ADC_FILTER_SIZE             8       // ADC滤波窗口大小
#define STABLE_COUNT_THRESHOLD      3       // 稳定计数阈值
#define FOD_POWER_LOSS_THRESHOLD    500     // FOD功率损耗阈值(mW)

/* I2C设备地址 */
#define I2C_ADDR_BQ25601            0x6B    // BQ25601充电IC地址(7位)

/* Qi协议参数 */
#define QI_SYNC_PATTERN             0x01    // Qi同步字节
#define QI_PACKET_HEADER            0x04    // Qi包头
#define QI_END_POWER_TRANSFER       0x0A    // 结束功率传输命令
#define QI_CONTROL_ERROR            0x03    // 控制误差包

/* ========== 引脚定义 ========== */

/* Hall传感器 - PA0 (输入，上拉) */
#define HALL_SENSOR_Pin             GPIO_PIN_0
#define HALL_SENSOR_GPIO_Port       GPIOA

/* 过压保护中断 - PA4 (EXTI4，上升沿) */
#define OVP_PIN_Pin                 GPIO_PIN_4
#define OVP_PIN_GPIO_Port           GPIOA
#define OVP_PIN_EXTI_IRQn           EXTI4_IRQn

/* 充电使能 - PA5 (输出，高有效) */
#define CHG_EN_Pin                  GPIO_PIN_5
#define CHG_EN_GPIO_Port            GPIOA

/* 状态LED - PA6 (输出) */
#define STATUS_LED_Pin              GPIO_PIN_6
#define STATUS_LED_GPIO_Port        GPIOA

/* ========== 系统状态枚举 ========== */

/* 系统状态 */
typedef enum {
    SYS_IDLE = 0,       // 空闲状态 - 等待设备放入
    SYS_DETECTING,      // 检测设备 - 确认设备存在
    SYS_CHARGING,       // 充电中 - 正在充电
    SYS_FULL,           // 充电完成 - 电池已满
    SYS_ERROR           // 错误状态 - 保护触发
} SystemState_t;

/* 错误类型 */
typedef enum {
    ERR_NONE = 0,       // 无错误
    ERR_OVP,            // 过压保护
    ERR_OCP,            // 过流保护
    ERR_OTP,            // 过温保护
    ERR_FOD,            // 异物检测
    ERR_COMM            // 通信错误
} ErrorType_t;

/* LED模式 */
typedef enum {
    LED_OFF = 0,        // 灭
    LED_ON,             // 常亮
    LED_SLOW_BLINK,     // 慢闪(500ms)
    LED_FAST_BLINK      // 快闪(100ms)
} LED_Mode_t;

/* ========== 函数声明 ========== */

/* 外设初始化函数 */
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_ADC1_Init(void);
void MX_I2C1_Init(void);
void MX_TIM2_Init(void);
void MX_USART1_UART_Init(void);

/* 错误处理 */
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
