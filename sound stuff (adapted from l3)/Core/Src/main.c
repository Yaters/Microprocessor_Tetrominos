/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define ARM_MATH_CM4
#include "arm_math.h"
#define _USE_MATH_DEFINES
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac1_ch2;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DAC1_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint16_t sample;
int T=0;

/* Triangle SWV Waveform Sample Formula */
int triangle(int ms) {
  int value = 0;
  if (ms % 15 < 8) {
    value = ms % 15;
  } else {
    value = 15 - (ms % 15);
  } // value \in [0, 7]

  return value * 384; // [0, 2688]
}

/* Sawtooth SWV Waveform Sample Formula */
int saw(int ms) {
  int value = ms % 15;
  return value * 192; // [0, 2880]

}

/* Sinusoid SWV Waveform Sample Formula (using CMSIS-DSP */
int sine(int ms) {
  float angle = (ms % 15) * 2 * 3.14f / 15;
  float sine = arm_sin_f32(angle);
  return (int) ((sine + 1.0f) * 1365);
}

int mode = 0;
uint32_t dummyLUT[1] = {0};
uint32_t sineLUT[tPeriod] = {0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0};

uint32_t sineLUT_LOW[tPeriodLOW] = {0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0};

uint32_t sineLUT_HIGH[tPeriodHIGH] = {0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0};

/*
 * Pushbutton GPIO Callback function
 * - Toggles LED
 * - Toggles DAC off and on to switch DMA pData LUT array
 * - Also toggles the SWV waveform (which is entirely separate from the pDATA LUT)
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  // Turn speaker off
  HAL_TIM_Base_Stop(&htim2);
  HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_2);

  // Toggle LED
  if (GPIO_Pin == myButton_Pin) {
    GPIO_PinState ledStatus = HAL_GPIO_ReadPin(myLED_GPIO_Port, myLED_Pin);
    if (ledStatus == GPIO_PIN_RESET) {
      HAL_GPIO_WritePin(myLED_GPIO_Port, myLED_Pin, GPIO_PIN_SET);
    } else {
      HAL_GPIO_WritePin(myLED_GPIO_Port, myLED_Pin, GPIO_PIN_RESET);
    }

    // Toggle waveform
    mode = (mode + 1) % 4;

    // Turn speaker on
    HAL_TIM_Base_Start(&htim2);
    if (mode == 0) {
      HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, (uint32_t*)sineLUT_LOW, tPeriodLOW, DAC_ALIGN_12B_R);
    } else if (mode == 1) {
      HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, (uint32_t*)sineLUT, tPeriod, DAC_ALIGN_12B_R);
    } else if (mode == 2) {
      HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, (uint32_t*)sineLUT_HIGH, tPeriodHIGH, DAC_ALIGN_12B_R);
    } else {
      HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, (uint32_t*)dummyLUT, 1, DAC_ALIGN_12B_R);
    }
  }
}

int timestamp = 0;

/* Sine wave LUT initialization formulas */

// This targets ~1764 Hz (~A6)
void initializeSineLUT() {
  for (int t = 0; t < tPeriod; t++) {
    float angle = ((float) t / 25) * 2 * 3.141592653589;
    float sine = arm_sin_f32(angle);
    sineLUT[t] = (uint32_t) ((sine + 1.0f) * 1365); // This is the fp to DAC 12b value conversion formula
  }
}

// This targets ~1411 Hz (~F#6)
void initializeSineLUT_LOW() {
  for (int t = 0; t < tPeriodLOW; t++) {
    float angle = ((float) t / tPeriodLOW) * 2 * 3.141592653589;
    float sine = arm_sin_f32(angle);
    sineLUT_LOW[t] = (uint32_t) ((sine + 1.0f) * 1365);
  }
}

// This targets ~2117 Hz (~C#6)
void initializeSineLUT_HIGH() {
  for (int t = 0; t < tPeriodHIGH; t++) {
    float angle = ((float) t / tPeriodHIGH) * 2 * 3.141592653589;
    float sine = arm_sin_f32(angle);
    sineLUT_HIGH[t] = (uint32_t) ((sine + 1.0f) * 1365);
  }
}

/* Timer Interrupt non-DMA Callback (commented out) */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
//  HAL_TIM_Base_Stop_IT(&htim2);
//  HAL_TIM_Base_Start_IT(&htim2);
//  if (mode == 0) {
//    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, sineLUT_LOW[timestamp]);
//    timestamp = (timestamp + 1) % tPeriodLOW;
//  } else if (mode == 1) {
//    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, sineLUT[timestamp]);
//    timestamp = (timestamp + 1) % tPeriod;
//  } else {
//    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, sineLUT_HIGH[timestamp]);
//    timestamp = (timestamp + 1) % tPeriodHIGH;
//  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  // NOTE: The below MX init calls have been reorganized. DMA must be initialized before DAC for the DMA operation to properly work.
  // If CubeMX generation is performed functionality can be restored by switching lines 231 and 232.
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DAC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

//  ITM_Port32(31) = 1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  int i = 0;
//  int mode = 0;
  initializeSineLUT();
  initializeSineLUT_LOW();
  initializeSineLUT_HIGH();
  HAL_TIM_Base_Start(&htim2);
  HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, (uint32_t*)sineLUT_LOW, tPeriodLOW, DAC_ALIGN_12B_R);
//  HAL_DAC_Start(&hdac1, DAC_CHANNEL_2); // This is the interrupt DAC start
//  HAL_TIM_Base_Start_IT(&htim2);
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    if (mode == 0) {
      sample = triangle(i);
    } else if (mode == 1) {
      sample = saw(i);
    } else if (mode == 2) {
      sample = sine(i);
    }
//    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, sample);
//    HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);
    HAL_Delay(100);
//    HAL_DAC_Stop(&hdac1, DAC_CHANNEL_2);
    i++;
    if (i % 15 == 0) {
      T++;
    }
    // Old button polling code
//    buttonStatus = HAL_GPIO_ReadPin(myButton_GPIO_Port, myButton_Pin);
//    if (buttonStatus == 0) {
//      if (edge == 0) {
//        edge = 1;
//        mode++;
//        mode = mode % 3;
//      }
//    } else {
//      edge = 0;
//    }
  }
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT2 config
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_T2_TRGO;
  sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_DISABLE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 2721;
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
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(myLED_GPIO_Port, myLED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : myButton_Pin */
  GPIO_InitStruct.Pin = myButton_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(myButton_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : myLED_Pin */
  GPIO_InitStruct.Pin = myLED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(myLED_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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
