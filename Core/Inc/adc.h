/**
  ******************************************************************************
  * @file    adc.h
  * @brief   ADC配置头文件 - 轮询模式(简单版)
  ******************************************************************************
  */

#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern ADC_HandleTypeDef hadc1;

/* ADC初始化 */
void MX_ADC1_Init(void);

/* ADC读取函数 */
uint16_t ADC_ReadChannel(uint32_t channel);

/* ADC通道定义 */
#define ADC_CH_VOLTAGE      ADC_CHANNEL_2   // PA2 - 电池电压
#define ADC_CH_CURRENT      ADC_CHANNEL_3   // PA3 - 充电电流
#define ADC_CH_TEMPERATURE  ADC_CHANNEL_8   // PB0 - NTC温度

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */
