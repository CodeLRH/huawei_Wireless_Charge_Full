<<<<<<< HEAD
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   GPIO引脚配置 - 无线充电系统所有数字IO的初始化
  ******************************************************************************
  * @attention
  *   引脚分配:
  *   - PA0: Hall传感器(输入，内部上拉) → 检测设备是否放入
  *   - PA4: OVP过压保护(EXTI4上升沿中断) → 硬件过压立即响应
  *   - PA5: 充电使能(输出，高有效) → 控制充电通路开关
  *   - PA6: 状态LED(输出) → 指示系统状态
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
/**
  * @brief  GPIO初始化 - 配置所有数字IO引脚
  * @note   4个引脚: Hall输入、OVP中断、充电使能输出、LED输出
  */
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 使能GPIO端口时钟 (GPIOA/B/D) */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* 输出引脚默认拉低(充电关闭、LED灭) */
  HAL_GPIO_WritePin(GPIOA, CHG_EN_Pin|STATUS_LED_Pin, GPIO_PIN_RESET);

  /* PA0: Hall传感器 - 输入模式，内部上拉(无设备时为高电平) */
  GPIO_InitStruct.Pin = HALL_SENSOR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(HALL_SENSOR_GPIO_Port, &GPIO_InitStruct);

  /* PA4: OVP过压保护 - 上升沿触发中断(硬件过压立即响应) */
  GPIO_InitStruct.Pin = OVP_PIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OVP_PIN_GPIO_Port, &GPIO_InitStruct);

  /* PA5: 充电使能 + PA6: 状态LED - 推挽输出 */
  GPIO_InitStruct.Pin = CHG_EN_Pin|STATUS_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI4中断配置: 抢占优先级1, 子优先级0 */
  HAL_NVIC_SetPriority(EXTI4_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
=======
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   GPIO引脚配置 - 无线充电系统所有数字IO的初始化
  ******************************************************************************
  * @attention
  *   引脚分配:
  *   - PA0: Hall传感器(输入，内部上拉) → 检测设备是否放入
  *   - PA4: OVP过压保护(EXTI4上升沿中断) → 硬件过压立即响应
  *   - PA5: 充电使能(输出，高有效) → 控制充电通路开关
  *   - PA6: 状态LED(输出) → 指示系统状态
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
/**
  * @brief  GPIO初始化 - 配置所有数字IO引脚
  * @note   4个引脚: Hall输入、OVP中断、充电使能输出、LED输出
  */
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 使能GPIO端口时钟 (GPIOA/B/D) */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* 输出引脚默认拉低(充电关闭、LED灭) */
  HAL_GPIO_WritePin(GPIOA, CHG_EN_Pin|STATUS_LED_Pin, GPIO_PIN_RESET);

  /* PA0: Hall传感器 - 输入模式，内部上拉(无设备时为高电平) */
  GPIO_InitStruct.Pin = HALL_SENSOR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(HALL_SENSOR_GPIO_Port, &GPIO_InitStruct);

  /* PA4: OVP过压保护 - 上升沿触发中断(硬件过压立即响应) */
  GPIO_InitStruct.Pin = OVP_PIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OVP_PIN_GPIO_Port, &GPIO_InitStruct);

  /* PA5: 充电使能 + PA6: 状态LED - 推挽输出 */
  GPIO_InitStruct.Pin = CHG_EN_Pin|STATUS_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI4中断配置: 抢占优先级1, 子优先级0 */
  HAL_NVIC_SetPriority(EXTI4_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
>>>>>>> f47fa64b4617799bdd183bd4e4d56fd770ff0ee0
