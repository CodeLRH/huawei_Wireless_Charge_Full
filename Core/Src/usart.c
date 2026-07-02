<<<<<<< HEAD
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   USART1配置 - 调试串口(115200 8N1)
  ******************************************************************************
  * @attention
  *   USART1: 115200波特率, 8数据位, 无校验, 1停止位
  *   引脚: PA9=TX, PA10=RX
  *   用途: printf重定向输出调试信息
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/**
  * @brief  USART1初始化 - 115200 8N1调试串口
  * @note   用于printf重定向，输出系统调试信息
  */
void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;                      // 115200波特率
  huart1.Init.WordLength = UART_WORDLENGTH_8B;        // 8位数据
  huart1.Init.StopBits = UART_STOPBITS_1;             // 1位停止位
  huart1.Init.Parity = UART_PARITY_NONE;              // 无校验
  huart1.Init.Mode = UART_MODE_TX_RX;                 // 收发模式
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;        // 无流控
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;     // 16倍过采样
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}

/**
  * @brief  USART1 MSP底层初始化 - 配置GPIO和中断
  * @note   PA9=TX(复用推挽), PA10=RX(浮空输入)
  */
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
    __HAL_RCC_USART1_CLK_ENABLE();                    // 使能USART1时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* PA9→USART1_TX(复用推挽输出) */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA10→USART1_RX(浮空输入) */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1中断: 抢占优先级2, 子优先级0 */
    HAL_NVIC_SetPriority(USART1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

=======
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   USART1配置 - 调试串口(115200 8N1)
  ******************************************************************************
  * @attention
  *   USART1: 115200波特率, 8数据位, 无校验, 1停止位
  *   引脚: PA9=TX, PA10=RX
  *   用途: printf重定向输出调试信息
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/**
  * @brief  USART1初始化 - 115200 8N1调试串口
  * @note   用于printf重定向，输出系统调试信息
  */
void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;                      // 115200波特率
  huart1.Init.WordLength = UART_WORDLENGTH_8B;        // 8位数据
  huart1.Init.StopBits = UART_STOPBITS_1;             // 1位停止位
  huart1.Init.Parity = UART_PARITY_NONE;              // 无校验
  huart1.Init.Mode = UART_MODE_TX_RX;                 // 收发模式
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;        // 无流控
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;     // 16倍过采样
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}

/**
  * @brief  USART1 MSP底层初始化 - 配置GPIO和中断
  * @note   PA9=TX(复用推挽), PA10=RX(浮空输入)
  */
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
    __HAL_RCC_USART1_CLK_ENABLE();                    // 使能USART1时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* PA9→USART1_TX(复用推挽输出) */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA10→USART1_RX(浮空输入) */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1中断: 抢占优先级2, 子优先级0 */
    HAL_NVIC_SetPriority(USART1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

>>>>>>> f47fa64b4617799bdd183bd4e4d56fd770ff0ee0
