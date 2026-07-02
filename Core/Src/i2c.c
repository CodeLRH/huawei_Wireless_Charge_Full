<<<<<<< HEAD
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   I2C1外设配置 - 与BQ25601充电IC通信
  ******************************************************************************
  * @attention
  *   I2C1配置: 100kHz标准模式, 7位地址
  *   引脚: PB6=SCL, PB7=SDA (开漏复用输出)
  *   目标设备: BQ25601充电管理IC (地址0x6B)
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;

/**
  * @brief  I2C1初始化 - 100kHz标准模式
  * @note   用于与BQ25601充电IC通信，读写充电参数寄存器
  */
void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;                    // 100kHz标准模式
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;            // 占空比2:1
  hi2c1.Init.OwnAddress1 = 0;                        // 主机无需自身地址
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT; // 7位地址模式
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;  // 允许时钟延展
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

}

/**
  * @brief  I2C1 MSP底层初始化 - 配置GPIO和时钟
  * @note   PB6/PB7配置为复用开漏输出(I2C总线要求)
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /* PB6→I2C1_SCL, PB7→I2C1_SDA, 复用开漏输出 */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;          // 开漏模式(I2C总线标准)
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_RCC_I2C1_CLK_ENABLE();                     // 使能I2C1外设时钟
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

=======
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   I2C1外设配置 - 与BQ25601充电IC通信
  ******************************************************************************
  * @attention
  *   I2C1配置: 100kHz标准模式, 7位地址
  *   引脚: PB6=SCL, PB7=SDA (开漏复用输出)
  *   目标设备: BQ25601充电管理IC (地址0x6B)
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;

/**
  * @brief  I2C1初始化 - 100kHz标准模式
  * @note   用于与BQ25601充电IC通信，读写充电参数寄存器
  */
void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;                    // 100kHz标准模式
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;            // 占空比2:1
  hi2c1.Init.OwnAddress1 = 0;                        // 主机无需自身地址
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT; // 7位地址模式
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;  // 允许时钟延展
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

}

/**
  * @brief  I2C1 MSP底层初始化 - 配置GPIO和时钟
  * @note   PB6/PB7配置为复用开漏输出(I2C总线要求)
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /* PB6→I2C1_SCL, PB7→I2C1_SDA, 复用开漏输出 */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;          // 开漏模式(I2C总线标准)
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_RCC_I2C1_CLK_ENABLE();                     // 使能I2C1外设时钟
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

>>>>>>> f47fa64b4617799bdd183bd4e4d56fd770ff0ee0
