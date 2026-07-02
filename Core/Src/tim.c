<<<<<<< HEAD
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   TIM2 PWM配置 - 110kHz无线充电驱动信号
  ******************************************************************************
  * @attention
  *   TIM2 CH1输出110kHz PWM信号到PA15
  *   频率计算: 72MHz / (Prescaler+1) / (ARR+1) = 72MHz / 1 / 655 ≈ 110kHz
  *   PA15需要AFIO重映射(默认是JTDI调试引脚)
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

TIM_HandleTypeDef htim2;

/**
  * @brief  TIM2初始化 - 110kHz PWM输出
  * @note   用于驱动无线充电发射线圈
  *         频率 = 72MHz / (0+1) / (654+1) ≈ 110kHz
  *         占空比通过PWM_SetDuty()动态调节
  */
void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* 时基配置: 预分频=0, ARR=654 → 110kHz */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;                          // 不分频，直接用72MHz
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;        // 向上计数
  htim2.Init.Period = 654;                            // ARR=654 → 72M/655≈110kHz
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* PWM模式1: 计数器<CCR时输出高电平 */
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;                               // 初始占空比0%
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;         // 高电平有效
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* 调用PostInit配置PA15复用引脚(AFIO重映射) */
  HAL_TIM_MspPostInit(&htim2);

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();
  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }
}
/**
  * @brief  TIM2 PostInit - 配置PWM输出引脚PA15
  * @note   PA15默认是JTDI(JTAG调试引脚)，需要AFIO重映射才能用作TIM2_CH1
  *         在stm32f1xx_hal_msp.c中已禁用JTAG(保留SWD)释放此引脚
  */
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(timHandle->Instance==TIM2)
  {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /* PA15 → TIM2_CH1 PWM输出(复用推挽) */
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;           // 复用推挽输出
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* TIM2部分重映射: CH1→PA15, CH2→PA3(本项目只用CH1) */
    __HAL_AFIO_REMAP_TIM2_PARTIAL_1();
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();
  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

=======
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   TIM2 PWM配置 - 110kHz无线充电驱动信号
  ******************************************************************************
  * @attention
  *   TIM2 CH1输出110kHz PWM信号到PA15
  *   频率计算: 72MHz / (Prescaler+1) / (ARR+1) = 72MHz / 1 / 655 ≈ 110kHz
  *   PA15需要AFIO重映射(默认是JTDI调试引脚)
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

TIM_HandleTypeDef htim2;

/**
  * @brief  TIM2初始化 - 110kHz PWM输出
  * @note   用于驱动无线充电发射线圈
  *         频率 = 72MHz / (0+1) / (654+1) ≈ 110kHz
  *         占空比通过PWM_SetDuty()动态调节
  */
void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* 时基配置: 预分频=0, ARR=654 → 110kHz */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;                          // 不分频，直接用72MHz
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;        // 向上计数
  htim2.Init.Period = 654;                            // ARR=654 → 72M/655≈110kHz
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* PWM模式1: 计数器<CCR时输出高电平 */
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;                               // 初始占空比0%
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;         // 高电平有效
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* 调用PostInit配置PA15复用引脚(AFIO重映射) */
  HAL_TIM_MspPostInit(&htim2);

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();
  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }
}
/**
  * @brief  TIM2 PostInit - 配置PWM输出引脚PA15
  * @note   PA15默认是JTDI(JTAG调试引脚)，需要AFIO重映射才能用作TIM2_CH1
  *         在stm32f1xx_hal_msp.c中已禁用JTAG(保留SWD)释放此引脚
  */
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(timHandle->Instance==TIM2)
  {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /* PA15 → TIM2_CH1 PWM输出(复用推挽) */
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;           // 复用推挽输出
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* TIM2部分重映射: CH1→PA15, CH2→PA3(本项目只用CH1) */
    __HAL_AFIO_REMAP_TIM2_PARTIAL_1();
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();
  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

>>>>>>> f47fa64b4617799bdd183bd4e4d56fd770ff0ee0
