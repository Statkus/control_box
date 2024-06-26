/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "usbd_cdc_if.h"

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
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

void Delay_Us (uint16_t us);

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
  MX_USB_DEVICE_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin (M1_PUL_GPIO_Port, M1_PUL_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin (M2_PUL_GPIO_Port, M2_PUL_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin (M3_PUL_GPIO_Port, M3_PUL_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin (M4_PUL_GPIO_Port, M4_PUL_Pin, GPIO_PIN_SET);

  HAL_TIM_Base_Start(&htim3);

  uint16_t M_Pos[4] = {0};
  uint16_t M_Pos_Target[4] = {0};

  GPIO_PinState M_Dir[4] =
   {GPIO_PIN_RESET,
    GPIO_PIN_RESET,
    GPIO_PIN_RESET,
    GPIO_PIN_RESET};

  GPIO_TypeDef *M_PUL_GPIO_Port[4] =
   {M1_PUL_GPIO_Port,
    M2_PUL_GPIO_Port,
    M3_PUL_GPIO_Port,
    M4_PUL_GPIO_Port};

  const uint16_t M_PUL_Pin[4] =
   {M1_PUL_Pin,
    M2_PUL_Pin,
    M3_PUL_Pin,
    M4_PUL_Pin};

  GPIO_TypeDef *M_DIR_GPIO_Port[4] =
   {M1_DIR_GPIO_Port,
    M2_DIR_GPIO_Port,
    M3_DIR_GPIO_Port,
    M4_DIR_GPIO_Port};

  const uint16_t M_DIR_Pin[4] =
   {M1_DIR_Pin,
    M2_DIR_Pin,
    M3_DIR_Pin,
    M4_DIR_Pin};

  uint16_t M_Delay          = DEFAULT_DELAY;
  uint16_t Previous_M_Delay = DEFAULT_DELAY;

  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 500);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    if (Is_New_Data() > 0) {
      HAL_GPIO_WritePin (LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

      M_Pos_Target[0] = Get_M1_Pos_Target();
      M_Pos_Target[1] = Get_M2_Pos_Target();
      M_Pos_Target[2] = Get_M3_Pos_Target();
      M_Pos_Target[3] = Get_M4_Pos_Target();

      uint16_t Dist[4] = {0};

      for (int i = 0; i < 4; i++) {
        if (M_Pos[i] > M_Pos_Target[i]) {
          Dist[i] = M_Pos[i] - M_Pos_Target[i];
        } else {
          Dist[i] = M_Pos_Target[i] - M_Pos[i];
        }
      }

      if (Dist[0] > 0 ||
          Dist[1] > 0 ||
          Dist[2] > 0 ||
          Dist[3] > 0) {
        M_Delay =
         MAX (MESSAGE_PERIOD / MAX (MAX (MAX (Dist[0], Dist[1]), Dist[2]), Dist[3]), MIN_DELAY);
      }

      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 500 + (Get_Shaker_PWM() * 2));
      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, Get_Fan_PWM() * 4);
    }

    if (M_Pos[0] != M_Pos_Target[0] ||
        M_Pos[1] != M_Pos_Target[1] ||
        M_Pos[2] != M_Pos_Target[2] ||
        M_Pos[3] != M_Pos_Target[3]) {
      uint8_t Change_DIR = 0;

      for (int i = 0; i < 4; i++) {
        if (M_Pos[i] != M_Pos_Target[i]) {
          const GPIO_PinState New_Dir =
           (M_Pos[i] < M_Pos_Target[i]) ? GPIO_PIN_RESET : GPIO_PIN_SET;

          if (New_Dir != M_Dir[i]) {
            HAL_GPIO_WritePin (M_DIR_GPIO_Port[i], M_DIR_Pin[i], New_Dir);

            M_Dir[i] = New_Dir;

            Change_DIR = 1;
          }
        }
      }

      if (Change_DIR > 0) {
        if (M_Delay < DEFAULT_DELAY) {
          Previous_M_Delay = DEFAULT_DELAY;
        } else {
          Previous_M_Delay = M_Delay;
        }

        Delay_Us(DIR_DELAY);
      } else {
        if (Previous_M_Delay > M_Delay + DEFAULT_DELAY) {
          Previous_M_Delay = DEFAULT_DELAY;
        } else if (Previous_M_Delay > M_Delay) {
          Previous_M_Delay--;
        } else {
          Previous_M_Delay = M_Delay;
        }
      }

      for (int i = 0; i < 4; i++) {
        if (M_Pos[i] != M_Pos_Target[i]) {
          HAL_GPIO_WritePin (M_PUL_GPIO_Port[i], M_PUL_Pin[i], GPIO_PIN_RESET);
        }
      }

      Delay_Us(PUL_DOWN_DELAY);

      for (int i = 0; i < 4; i++) {
        if (M_Pos[i] != M_Pos_Target[i]) {
          if (M_Dir[i] == GPIO_PIN_RESET) {
            M_Pos[i]++;
          } else {
            M_Pos[i]--;
          }

          HAL_GPIO_WritePin (M_PUL_GPIO_Port[i], M_PUL_Pin[i], GPIO_PIN_SET);
        }
      }

      if (Previous_M_Delay > PUL_DOWN_DELAY + WAISTED_DELAY) {
        Delay_Us(Previous_M_Delay - PUL_DOWN_DELAY - WAISTED_DELAY);
      }
    } else {
      Previous_M_Delay = DEFAULT_DELAY;
    }

    HAL_GPIO_WritePin (LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 95;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 3;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1000;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, M1_PUL_Pin|M1_DIR_Pin|M2_PUL_Pin|M2_DIR_Pin
                          |M3_PUL_Pin|M3_DIR_Pin|M4_PUL_Pin|M4_DIR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : M1_PUL_Pin M1_DIR_Pin M2_PUL_Pin M2_DIR_Pin
                           M3_PUL_Pin M3_DIR_Pin M4_PUL_Pin M4_DIR_Pin */
  GPIO_InitStruct.Pin = M1_PUL_Pin|M1_DIR_Pin|M2_PUL_Pin|M2_DIR_Pin
                          |M3_PUL_Pin|M3_DIR_Pin|M4_PUL_Pin|M4_DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void Delay_Us (uint16_t us)
{
  __HAL_TIM_SET_COUNTER(&htim3, 0);
  while (__HAL_TIM_GET_COUNTER(&htim3) < us);
}

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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

