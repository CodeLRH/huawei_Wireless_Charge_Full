/**
  ******************************************************************************
  * @file    adc.c
  * @brief   ADC配置 - 3通道轮询采样(不使用DMA，更简单)
  ******************************************************************************
  * @attention
  *   ADC通道分配:
  *   - 通道2 (PA2): 电池电压采样
  *   - 通道3 (PA3): 充电电流采样
  *   - 通道8 (PB0): NTC温度采样
  *
  *   使用方法: 调用 ADC_ReadChannel(channel) 读取单个通道
  ******************************************************************************
  */

#include "adc.h"

ADC_HandleTypeDef hadc1;

/**
  * @brief  ADC1初始化 - 单通道轮询模式
  * @note   配置为单通道、单次转换、软件触发、轮询读取
  *         不使用DMA和扫描模式，简单可靠
  *         实际使用时通过ADC_ReadChannel()动态切换通道
  */
void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  /* ADC1基本配置: 单通道单次转换模式 */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;          // 不扫描(单通道)
  hadc1.Init.ContinuousConvMode = DISABLE;             // 单次转换(非连续)
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;    // 软件触发(非硬件触发)
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;          // 12位结果右对齐
  hadc1.Init.NbrOfConversion = 1;                      // 只有1个转换通道
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /* 默认配置通道2(电压)，实际使用时ADC_ReadChannel()会重新配置 */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;   // 239.5周期，约20μs
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  ADC MSP底层初始化 - 配置ADC引脚为模拟输入
  * @note   3个ADC通道对应的GPIO配置为模拟模式:
  *         PA2→IN2(电压), PA3→IN3(电流), PB0→IN8(温度)
  */
void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
    /* 使能ADC1和GPIO时钟 */
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* PA2→ADC1_IN2(电池电压), PA3→ADC1_IN3(充电电流) - 模拟输入 */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;           // 模拟模式(禁用数字输入)
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PB0→ADC1_IN8(NTC温度) - 模拟输入 */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{
  if(adcHandle->Instance==ADC1)
  {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);
  }
}

/**
  * @brief  读取指定ADC通道(轮询方式)
  * @param  channel: ADC通道号
  *         - ADC_CHANNEL_2: 电池电压(PA2)
  *         - ADC_CHANNEL_3: 充电电流(PA3)
  *         - ADC_CHANNEL_8: NTC温度(PB0)
  * @retval ADC原始值(0-4095, 12位)
  * @note   流程: 重配通道→启动转换→等待完成→读取值→停止
  *         阻塞约30μs(239.5采样周期×1/12MHz)
  */
uint16_t ADC_ReadChannel(uint32_t channel)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  /* 动态配置要读取的通道 */
  sConfig.Channel = channel;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;   // 239.5周期采样
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

  /* 启动单次转换 */
  HAL_ADC_Start(&hadc1);

  /* 轮询等待转换完成(阻塞) */
  HAL_ADC_PollForConversion(&hadc1, 100);  // 超时100ms

  /* 读取12位ADC结果 */
  uint16_t value = (uint16_t)HAL_ADC_GetValue(&hadc1);

  /* 停止ADC(单次模式需要手动停止) */
  HAL_ADC_Stop(&hadc1);

  return value;
}
