/*
 ******************************************************************************
* file      : main.c
* project   : EE 329 F'23 A8
* details   : ADC
* authors   : John Park
*           :
* version   : 0.1
* date      : 2023-NOV-27
 * compiler  : STM32CubeIDE Version: 1.13.1 (2023)
* target    : NUCLEO-L4A6ZG
 ******************************************************************************
 * revisions :
* 2023-NOV-20 created.
******************************************************************************
* origins   :
* modified from CubeIDE auto-generated main.c  (c) 2023 STMicro.
* modified from EE329 Lab Manual Sample Code
******************************************************************************

/* Includes ----------------------------------------------------------------*/
#include "main.h"
#include "ADC.h"
#include "UART.h"

// function prototyping
void SystemClock_Config(void);
void ADC_init(void);
void SysTick_Init(void);
void delay_us(const uint32_t time_us);
void ADC1_2_IRQHandler(void);
void ADC_home_screen(void);
void config_UART(void);
int ADC_find_min(uint16_t counts[]);
int ADC_find_max(uint16_t counts[]);
int ADC_find_avg(uint16_t counts[]);
void ADC_print_counts(int min, int max, int avg);
void ADC_print_volts(int min, int max, int avg);
int PBSW_debounce(void );
void setup_PBSW(void);
void relay_init(void);
int ADC_get_current(int avg_counts);
void ADC_print_current(int milliamps);

// global variables
// stores array of 20 samples
uint16_t adcArr[20];
int sample_num;

void ADC_init(void) {

	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;         // turn on clock for ADC
	// power up & calibrate ADC
	ADC123_COMMON->CCR |= (1 << ADC_CCR_CKMODE_Pos); // clock source = HCLK/1
	ADC1->CR &= ~(ADC_CR_DEEPPWD);             // disable deep-power-down
	ADC1->CR |= (ADC_CR_ADVREGEN);             // enable V regulator - see RM 18.4.6
	// multiplying by 6 to account for 24 MHz clock
	delay_us(20 * 6);                              // wait 20us for ADC to power up
	ADC1->DIFSEL &= ~(ADC_DIFSEL_DIFSEL_5);    // PA0=ADC1_IN5, single-ended
	ADC1->CR &= ~(ADC_CR_ADEN | ADC_CR_ADCALDIF); // disable ADC, single-end calib
	ADC1->CR |= ADC_CR_ADCAL;                  // start calibration
	while (ADC1->CR & ADC_CR_ADCAL) {;}        // wait for calib to finish
	// enable ADC
	ADC1->ISR |= (ADC_ISR_ADRDY);              // set to clr ADC Ready flag
	ADC1->CR |= ADC_CR_ADEN;                   // enable ADC
	while(!(ADC1->ISR & ADC_ISR_ADRDY)) {;}    // wait for ADC Ready flag
	ADC1->ISR |= (ADC_ISR_ADRDY);              // set to clr ADC Ready flag
	// configure ADC sampling & sequencing
	ADC1->SQR1  |= (5 << ADC_SQR1_SQ1_Pos);    // sequence = 1 conv., ch 5
	//ADC1->SMPR1 |= (3 << ADC_SMPR1_SMP5_Pos);  // ch 5 sample time = 6.5 clocks
	//ADC1->SMPR1 |= (2 << ADC_SMPR1_SMP5_Pos);
	ADC1->SMPR1 |= (1 << ADC_SMPR1_SMP5_Pos);
	ADC1->CFGR  &= ~( ADC_CFGR_CONT  |         // single conversion mode
	                  ADC_CFGR_EXTEN |         // h/w trig disabled for s/w trig
	                  ADC_CFGR_RES   );        // 12-bit resolution
	// configure & enable ADC interrupt
	ADC1->IER |= ADC_IER_EOCIE;                // enable end-of-conv interrupt
	ADC1->ISR |= ADC_ISR_EOC;                  // set to clear EOC flag
	NVIC->ISER[0] = (1<<(ADC1_2_IRQn & 0x1F)); // enable ADC interrupt service
	__enable_irq();                            // enable global interrupts
	// configure GPIO pin PA0
	RCC->AHB2ENR  |= (RCC_AHB2ENR_GPIOAEN);    // connect clock to GPIOA
	GPIOA->MODER  |= (GPIO_MODER_MODE0);       // analog mode for PA0 (set MODER last)

	ADC1->CR |= ADC_CR_ADSTART;                // start 1st conversion
}


void ADC1_2_IRQHandler(void) {
    if (ADC1->ISR & ADC_ISR_EOC) {  // Check if end-of-conversion flag is set
    	adcArr[sample_num] = ADC1->DR;
    }
}


int main(void) {

	HAL_Init();
	SystemClock_Config();
	config_UART();
	ADC_home_screen();
	setup_PBSW();
	relay_init();
	// initializing ADC
	ADC_init();


	while (1) {
		if (PBSW_debounce() == 2) {
			GPIOB->BSRR = (GPIO_PIN_11);
			// create for loop calling ADSTART to get different samples rather than all of the same
			delay_us(500 * 6);
			sample_num = 0;
			for (int i = 0; i < 20; i++) {
				ADC1->CR |= ADC_CR_ADSTART;
				delay_us(5000);
				sample_num++;
			}
			// finding and displaying voltage and counts
			int min = ADC_find_min(adcArr);
			int max = ADC_find_max(adcArr);
			int avg = ADC_find_avg(adcArr);
			ADC_print_counts(min, max, avg);
			delay_us(5000000);
			ADC_print_volts(min, max, avg);

			// finding and display current
			int current = ADC_get_current(avg);
			ADC_print_current(current);
		}
		GPIOB->BRR = (GPIO_PIN_11);
	}
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
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_9;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
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
