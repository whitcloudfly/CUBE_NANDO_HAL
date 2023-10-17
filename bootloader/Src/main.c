/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "version.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* Flash layout
 * ------------ 0x08000000
 * |bootloader |
 * |14K        |
 * -------------0x08003800
 * |data       |
 * |2K         |
 * ------------ 0x08004000
 * |image1     |
 * |120K       |
 * -------------0x08022000
 * |image2     |
 * |120K       |
 * -------------0x08040000
 */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STR(a) #a
#define MAKE_STR(a) STR(a)
#define VERSION "\r\nBootloader ver: " MAKE_STR(SW_VERSION_MAJOR) "." \
    MAKE_STR(SW_VERSION_MINOR) "." MAKE_STR(SW_VERSION_BUILD) "\r\n"

#define APP1_ADDRESS_OFFSET 0x4000
#define APP1_ADDRESS (FLASH_BASE + APP1_ADDRESS_OFFSET)
#define APP2_ADDRESS_OFFSET 0x22000
#define APP2_ADDRESS (FLASH_BASE + APP2_ADDRESS_OFFSET)

#define BOOT_DATA_ADDRESS 0x08003800

typedef void (*app_func_t)(void);
typedef struct __attribute__((__packed__))
{
    uint8_t active_image; // 标识当前活动的应用程序映像编�?????????
} config_t;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  app_func_t app; // 函数指针，用于跳转到应用程序
  uint32_t jump_addr, vt_offset, sp_addr; // 跳转地址、向量表偏移量�?�堆栈指针地�?????????
  volatile config_t *config = (config_t *)BOOT_DATA_ADDRESS;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  printf(VERSION); // 打印引导程序版本�??????????

  printf("Start application: ");
  if (!config->active_image) // �??????????查当前活动的应用程序映像编号
  {
      printf("0\r\n");
      vt_offset = APP1_ADDRESS_OFFSET; // 应用程序1的向量表偏移�??????????
      jump_addr = *(__IO uint32_t *)(APP1_ADDRESS + 4); // 应用程序1的跳转地�??????????
      sp_addr = *(__IO uint32_t *)APP1_ADDRESS; // 应用程序1的堆栈指针地�??????????
  }
  else
  {
      printf("1\r\n");
      vt_offset = APP2_ADDRESS_OFFSET; // 应用程序2的向量表偏移�??????????
      jump_addr = *(__IO uint32_t *)(APP2_ADDRESS + 4); // 应用程序2的跳转地�??????????
      sp_addr = *(__IO uint32_t *)APP2_ADDRESS; // 应用程序2的堆栈指针地�??????????
  }

  /* 重新定位向量�?????????? */
  SCB->VTOR = FLASH_BASE | vt_offset;
  /* 初始化应用程序的堆栈指针 */
  __set_MSP(sp_addr);
  /* 启动应用程序 */
  app = (app_func_t)jump_addr;
  app();

  return 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  while (1)
//  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
