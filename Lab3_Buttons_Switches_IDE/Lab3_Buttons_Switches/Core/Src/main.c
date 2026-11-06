/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "input_reading.h"
#include "input_processing.h"
#include "led_display.h"
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
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
static volatile uint32_t tim2_irq_count = 0U;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static uint8_t traffic_active = 0U;
static uint32_t cycle_start_ms = 0U;
static uint16_t previous_on_pins = 0U;

static void traffic_normal_run(void)
{
    /* Editing modes own the LEDs while normal mode is stopped. */
    if (app_get_mode() != 1U) {
        traffic_active = 0U;
        return;
    }

    uint32_t now_ms = HAL_GetTick();

    /* Restart the cycle whenever Mode 1 is entered. */
    if (traffic_active == 0U) {
        cycle_start_ms = now_ms;
        previous_on_pins = 0U;
        traffic_active = 1U;
    }

    uint32_t green_ms =
        (uint32_t)app_get_green_duration() * 1000U;

    uint32_t amber_ms =
        (uint32_t)app_get_amber_duration() * 1000U;

    /*
     * The existing SET logic maintains:
     * red = green + amber.
     */
    uint32_t half_cycle_ms = green_ms + amber_ms;
    uint32_t cycle_ms = 2U * half_cycle_ms;

    /* Defensive check; normal SET logic prevents zero durations. */
    if ((green_ms == 0U) || (amber_ms == 0U)) {
        traffic_leds_off();
        led_display_set_normal_values(0U, 0U);
        return;
    }

    uint32_t elapsed_ms =
        (uint32_t)(now_ms - cycle_start_ms);

    /* Skip completed cycles without accumulating timing drift. */
    if (elapsed_ms >= cycle_ms) {
        uint32_t cycles = elapsed_ms / cycle_ms;

        cycle_start_ms += cycles * cycle_ms;
        elapsed_ms %= cycle_ms;
    }

    uint32_t phase_elapsed_ms;
    uint8_t left;
    uint8_t right;
    uint16_t on_pins;

    if (elapsed_ms < green_ms) {
        /* Phase 0: road 1 RED, road 2 GREEN. */
        phase_elapsed_ms = elapsed_ms;

        left = (uint8_t)(
            (half_cycle_ms - phase_elapsed_ms + 999U) / 1000U
        );

        right = (uint8_t)(
            (green_ms - phase_elapsed_ms + 999U) / 1000U
        );

        on_pins = LED_RED_1_Pin | LED_GREEN_2_Pin;
    } else if (elapsed_ms < half_cycle_ms) {
        /* Phase 1: road 1 RED, road 2 AMBER. */
        phase_elapsed_ms = elapsed_ms - green_ms;

        left = (uint8_t)(
            (amber_ms - phase_elapsed_ms + 999U) / 1000U
        );

        right = left;

        on_pins = LED_RED_1_Pin | LED_YELLOW_2_Pin;
    } else if (elapsed_ms < half_cycle_ms + green_ms) {
        /* Phase 2: road 1 GREEN, road 2 RED. */
        phase_elapsed_ms = elapsed_ms - half_cycle_ms;

        left = (uint8_t)(
            (green_ms - phase_elapsed_ms + 999U) / 1000U
        );

        right = (uint8_t)(
            (half_cycle_ms - phase_elapsed_ms + 999U) / 1000U
        );

        on_pins = LED_GREEN_1_Pin | LED_RED_2_Pin;
    } else {
        /* Phase 3: road 1 AMBER, road 2 RED. */
        phase_elapsed_ms =
            elapsed_ms - half_cycle_ms - green_ms;

        left = (uint8_t)(
            (amber_ms - phase_elapsed_ms + 999U) / 1000U
        );

        right = left;

        on_pins = LED_YELLOW_1_Pin | LED_RED_2_Pin;
    }

    /* Your traffic LEDs are active-low. */
    if (on_pins != previous_on_pins) {
        traffic_leds_off();

        HAL_GPIO_WritePin(
            GPIOA,
            on_pins,
            GPIO_PIN_RESET
        );

        previous_on_pins = on_pins;
    }

    led_display_set_normal_values(left, right);
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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  input_reading_init();
  led_display_init();
  input_processing_init();

  tim2_irq_count = 0U;

  __HAL_TIM_SET_COUNTER(&htim2, 0U);
  __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);

  if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
  {
      Error_Handler();
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	    fsm_for_input_processing();

	    traffic_normal_run();

	    led_display_run();

	    __WFI();
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
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
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_RED_1_Pin|LED_YELLOW_1_Pin|LED_GREEN_1_Pin|LED_RED_2_Pin
                          |LED_YELLOW_2_Pin|LED_GREEN_2_Pin|EN0_Pin|EN1_Pin
                          |EN2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, a_Pin|b_Pin|c_Pin|k_Pin
                          |l_Pin|m_Pin|n_Pin|EN3_Pin
                          |d_Pin|e_Pin|f_Pin|g_Pin
                          |h_Pin|i_Pin|j_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : LED_RED_1_Pin LED_YELLOW_1_Pin LED_GREEN_1_Pin LED_RED_2_Pin
                           LED_YELLOW_2_Pin LED_GREEN_2_Pin EN0_Pin EN1_Pin
                           EN2_Pin */
  GPIO_InitStruct.Pin = LED_RED_1_Pin|LED_YELLOW_1_Pin|LED_GREEN_1_Pin|LED_RED_2_Pin
                          |LED_YELLOW_2_Pin|LED_GREEN_2_Pin|EN0_Pin|EN1_Pin
                          |EN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Button1_Pin Button2_Pin Button3_Pin */
  GPIO_InitStruct.Pin = Button1_Pin|Button2_Pin|Button3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : a_Pin b_Pin c_Pin k_Pin
                           l_Pin m_Pin n_Pin EN3_Pin
                           d_Pin e_Pin f_Pin g_Pin
                           h_Pin i_Pin j_Pin */
  GPIO_InitStruct.Pin = a_Pin|b_Pin|c_Pin|k_Pin
                          |l_Pin|m_Pin|n_Pin|EN3_Pin
                          |d_Pin|e_Pin|f_Pin|g_Pin
                          |h_Pin|i_Pin|j_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        tim2_irq_count++;
        button_reading();
        led_display_scan();
    }
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
#ifdef USE_FULL_ASSERT
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
