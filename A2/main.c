/*
 ******************************************************************************
* file      : main.c
* project   : EE 329 F'23 A2
* details   : 4x4 Keypad config for lighting LEDs corresponding to pressed value
* authors   : John Park
*           :
* version   : 0.1
* date      : 2023-OCT-10
 * compiler  : STM32CubeIDE Version: 1.13.1 (2023)
* target    : NUCLEO-L4A6ZG
 ******************************************************************************
 * revisions :
* 2023-OCT-8 created.
******************************************************************************
* origins   :
* modified from CubeIDE auto-generated main.c  (c) 2023 STMicro.
* modified from EE329 Lab Manual Sample Code
******************************************************************************

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "keypad.h"

void SystemClock_Config(void);
void Keypad_Config(void);
void light_LEDs(int keypress);
int Keypad_Debounce(int pressed);
int Keypad_WhichKeyIsPressed(void);
int Keypad_IsAnyKeyPressed(void);

int main(void) {


    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();


    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    // Initializing keyboard to GPIOD PINS (PD0-7)
    // Initializing clock for GPIOC pins
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOCEN);

	// clearing output pins (PC0-3) for LEDS
	GPIOC->MODER   &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2 | GPIO_MODER_MODE3);

	// setting PC0-3 to output mode (01), so LEDS can be turned on and off
	GPIOC->MODER   |=  (GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0 | GPIO_MODER_MODE2_0 | GPIO_MODER_MODE3_0);

	// setting type to push pull (0)
	GPIOC->OTYPER  &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1 | GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);

	// setting output pins to (00) no pull up or pull down resistor
	GPIOC->PUPDR   &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);

	// setting output bits to (11) for PC0-3 (very fast mode)
	GPIOC->OSPEEDR |=  ((3 << GPIO_OSPEEDR_OSPEED0_Pos) |
		                (3 << GPIO_OSPEEDR_OSPEED1_Pos) |
					    (3 << GPIO_OSPEEDR_OSPEED2_Pos) |
					    (3 << GPIO_OSPEEDR_OSPEED3_Pos) );
	// setting LEDS to off initialing (0) using BRR
	GPIOC->BRR = (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    // setting

    /* USER CODE END SysInit */
    /* Initialize all configured peripherals */
    /* USER CODE BEGIN 2 */

    /* USER CODE END 2 */

    /* Infinite loop */
  	/* USER CODE BEGIN WHILE */
	Keypad_Config();
  	while (1) {
  		//int pressed = NO_KEYPRESS;
  		while ((Keypad_IsAnyKeyPressed() != TRUE)) {

  		}
  		int pressed = Keypad_WhichKeyIsPressed();
  		int new_key = Keypad_Debounce(pressed);
  		if (new_key == pressed) {
  			light_LEDs(pressed);
  		}
  		//GPIOC->BSRR = (GPIO_PIN_0);
  		//for (i = 0; i < 500000; i++);
  		// if a keypad is pressed, find the key
  		//
  		// if no key is pressed do nothing
    }
  	/* USER CODE END WHILE */

  	/* USER CODE BEGIN 3 */

    /* USER CODE END 3 */

}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
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
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
