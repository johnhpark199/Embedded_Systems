/*
 ******************************************************************************
* file      : main.c
* project   : EE 329 F'23 A4
* details   : User Reaction timer, utilizing interrupts and flags
* authors   : John Park
*           :
* version   : 0.1
* date      : 2023-OCT-22
 * compiler  : STM32CubeIDE Version: 1.13.1 (2023)
* target    : NUCLEO-L4A6ZG
 ******************************************************************************
 * revisions :
* 2023-OCT-18 created.
******************************************************************************
* origins   :
* modified from CubeIDE auto-generated main.c  (c) 2023 STMicro.
* modified from EE329 Lab Manual Sample Code
/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "LCD.h"
#include "timer_inter.h"
#include "main.h"

// global variables
int react_cnt;
int react_flag;
int wait_cnt;
int wait_flag;

// includes
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void TIM2_IRQHandler(void);
void setup_TIM2(int iDutyCycle, int period);
void setup_MCO_CLK(void);
void setup_OBLED(void);
void delay_us(const uint32_t time_us);
int PBSW_debounce(void);
void setup_PBSW();
void RNG_Init(void);
int RNG_Get_Rand(int max);
void RDG_Get_Rand(void);
void LCD_config();


void setup_TIM2( int iDutyCycle, int period) {

   RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;           // enable clock for TIM2
   TIM2->DIER |= (TIM_DIER_CC1IE | TIM_DIER_UIE);  // enable event gen, rcv CCR1
   TIM2->ARR = period;                             // ARR = T = counts @4MHz
   TIM2->CCR1 = iDutyCycle;                        // ticks for duty cycle
   TIM2->SR &= ~(TIM_SR_CC1IF | TIM_SR_UIF);       // clr IRQ flag in status reg
   NVIC->ISER[0] |= (1 << (TIM2_IRQn & 0x1F));     // set NVIC interrupt: 0x1F
   __enable_irq();                                 // global IRQ enable
   TIM2->CR1 |= TIM_CR1_CEN;                       // start TIM2 CR1
}


void TIM2_IRQHandler(void) {

	if (TIM2->SR & TIM_SR_CC1IF) {      // triggered by CCR1 event ...
	    TIM2->SR &= ~(TIM_SR_CC1IF);     // manage the flag
	    if (wait_flag == 1) {
	    	wait_cnt -= 1;
	    }
	}

	if (TIM2->SR & TIM_SR_UIF) {        // triggered by ARR event ...
	    TIM2->SR &= ~(TIM_SR_UIF);       // manage the flag
	    if (react_flag == 1) {
	    	react_cnt += 1;
	    }
	}
}

int main(void) {

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	SysTick_Init();
	LED_init();

	LCD_config();
	LCD_init();

	setup_OBLED();
	setup_PBSW();
	RNG_Init();

	while (1) {
		// initializing global variables
		react_flag = 0;
		wait_flag = 0;
		wait_cnt = 0;
		react_cnt = 0;
		// writing initial statements to LCD
		LCD_command(0x02);
		LCD_write_str("EE 329 A4 REACT");
		LCD_command(0xC0);
		LCD_write_str("PUSH SW TO TRIG");
		// if initial reset is pressed
		int press_val = 0;

		// checking for first user button press
		while(press_val < 2) {
			press_val = PBSW_debounce();
		}
		// number of times cycled through 5kHz clock to get a random 0-10 time of waiting
		wait_cnt = (RNG_Get_Rand(10) * 5000);
		// setting wait variables to determine random amount of time to wait
		wait_flag = 1;
		setup_TIM2(800, 400);
		LCD_command(0xC0);
		LCD_write_str("TIME = N.MMM s  ");

		// enabling count comparison function in ISR to check when to turn on LED

		// waiting for random time to finish, variable will be set high when random count value is reached
		while (wait_cnt > 0);

		wait_flag = 0;
		// turn on blue on board LED
		GPIOB->BSRR = (GPIO_PIN_7);
		// waiting for user to press button after LED is turned on
		press_val = 0;

		react_flag = 1;
		// checking for button press
		while (press_val < 2) {

			press_val = PBSW_debounce();
			// exit if waiting longer than 10 s
			if (react_cnt > 50000) {
				react_cnt = 49999;
				break;
			}
		}
		react_flag = 0;
		// waiting to avoid debounce issues
		delay_us(500000);

		GPIOB->BRR = (GPIO_PIN_7);
		// after exits while loop, will iterate number of num_clks within ISR to
		// determine how fast the user is
		int time_arr[4];
		// changing number of clocks to milliseconds

		int speed_msecs = (react_cnt / 5);
		// getting seconds
		time_arr[3] = (speed_msecs / 1000);
		// getting 100s of milliseconds
		time_arr[2] = (speed_msecs / 100) % 10;
		// getting 10s of milliseconds
		time_arr[1] = (speed_msecs / 10) % 10;
		// getting milliseconds
		time_arr[0] = (speed_msecs % 10);

		LCD_command(0XC0);
		LCD_write_str("TIME = ");
		LCD_write_char(int_to_char(time_arr[3]));
		LCD_write_char('.');
		LCD_write_char(int_to_char(time_arr[2]));
		LCD_write_char(int_to_char(time_arr[1]));
		LCD_write_char(int_to_char(time_arr[0]));
		LCD_write_str(" s  ");

		// waiting for reset to reset game again
		press_val = 0;
		while (press_val < 2) {
			press_val = PBSW_debounce();
		}
		// waiting to avoid debounce issues
		delay_us(50000);
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  HAL_PWREx_EnableVddIO2();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : STLK_RX_Pin STLK_TX_Pin */
  GPIO_InitStruct.Pin = STLK_RX_Pin|STLK_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_SOF_Pin USB_ID_Pin USB_DM_Pin USB_DP_Pin */
  GPIO_InitStruct.Pin = USB_SOF_Pin|USB_ID_Pin|USB_DM_Pin|USB_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
