<<<<<<< HEAD
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32f1xx_hal_msp.c
  * @brief        全局MSP(底层驱动)初始化 - AFIO和调试接口配置
  ******************************************************************************
  * @attention
  *   关键配置: 禁用JTAG(保留SWD)，释放PA15用于TIM2_CH1 PWM输出
  *   MSP(MCU Support Package)是HAL库的底层硬件抽象层
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief  全局MSP初始化 - AFIO和电源时钟配置
  * @note   关键操作: 禁用JTAG调试接口(保留SWD)，释放PA15引脚用于TIM2 PWM输出
  *         PA15默认是JTDI(JTAG数据输入)，禁用JTAG后可作为普通IO使用
  */
void HAL_MspInit(void)
{

  __HAL_RCC_AFIO_CLK_ENABLE();    // 使能AFIO时钟(引脚重映射需要)
  __HAL_RCC_PWR_CLK_ENABLE();     // 使能电源控制时钟

  /* 禁用JTAG，保留SWD调试 → 释放PA15(JTDI)用于TIM2_CH1 PWM */
  __HAL_AFIO_REMAP_SWJ_NOJTAG();

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
=======
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32f1xx_hal_msp.c
  * @brief        全局MSP(底层驱动)初始化 - AFIO和调试接口配置
  ******************************************************************************
  * @attention
  *   关键配置: 禁用JTAG(保留SWD)，释放PA15用于TIM2_CH1 PWM输出
  *   MSP(MCU Support Package)是HAL库的底层硬件抽象层
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief  全局MSP初始化 - AFIO和电源时钟配置
  * @note   关键操作: 禁用JTAG调试接口(保留SWD)，释放PA15引脚用于TIM2 PWM输出
  *         PA15默认是JTDI(JTAG数据输入)，禁用JTAG后可作为普通IO使用
  */
void HAL_MspInit(void)
{

  __HAL_RCC_AFIO_CLK_ENABLE();    // 使能AFIO时钟(引脚重映射需要)
  __HAL_RCC_PWR_CLK_ENABLE();     // 使能电源控制时钟

  /* 禁用JTAG，保留SWD调试 → 释放PA15(JTDI)用于TIM2_CH1 PWM */
  __HAL_AFIO_REMAP_SWJ_NOJTAG();

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
>>>>>>> f47fa64b4617799bdd183bd4e4d56fd770ff0ee0
