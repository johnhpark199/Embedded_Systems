/*
 ******************************************************************************
* file      : main.c
* project   : EE 329 F'23 A6
* details   : function generator
* authors   : John Park
*           :
* version   : 0.1
* date      : 2023-NOV-8
 * compiler  : STM32CubeIDE Version: 1.13.1 (2023)
* target    : NUCLEO-L4A6ZG
 ******************************************************************************
 * revisions :
* 2023-NOV-1 created.
******************************************************************************
* origins   :
* modified from CubeIDE auto-generated main.c  (c) 2023 STMicro.
* modified from EE329 Lab Manual Sample Code
*
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "DAC.h"
#include "LCD.h"
#include "timer_inter.h"
#include "keypad.h"
#include <math.h>

/* function prototypes */
void SystemClock_Config(void);
void DAC_write(uint16_t data);
void LCD_write_str(char output[]);
void SPI_init( void );
void DAC_init(void);
void LCD_command( uint8_t command );
void setup_TIM2( int iDutyCycle, int period);
void TIM2_IRQHandler(void);
void ramp_wave(int frequency, int direction);
void square_wave(int frequency, int duty_cycle);
void createSineTable(double sineTable[]);
uint16_t DAC_volt_convert(float input);
void sine_wave(int frequency);
int Keypad_read(void);
/* global variables */

// ramp value, counter value to be edited while increasing or decreasing ramp
int counter = 0;
// wave type flag, using 0 for square wave, 1 for ramp, 2 for sine
int wave_type = 0;
// global frequency variable 1->100, 2->200, etc.
int freq_globe = 1;
// 0 for high to low, 1 for low to high
int ramp_type;
// global sine table
double sine_table[TABLE_SIZE];
// duty cycle percentage global (1-9)
int duty_globe;


int main(void) {

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	/* Configure the system clock */
	SystemClock_Config();
	DAC_init();
	SPI_init();

	Keypad_Config();
	LED_init();
	SysTick_Init();
	LCD_config();
	LCD_init();
	LCD_command(0x80);
	LCD_write_str("SQU 100 Hz  LAST");
	LCD_command(0xC0);
	LCD_write_str("50% DUTY     '8'");
	createSineTable(sine_table);
	// setting ARR flag throw to every 20 us or 50KHz

	// creating and creating sine wave
	setup_TIM2(240000, 480000);
	square_wave(1, 5);

	while (1) {

		int waveform = Keypad_read();
		__disable_irq();
		// keypad entry for sine wave
		if (waveform == 7) {
			sine_wave(1);

		// keypad entry for sawtooth/ramp
		} else if (waveform == 9) {
			ramp_wave(1, 1);

		// keypad entry for square wave
		} else if (waveform == 10) {
			square_wave(1, duty_globe);

		// reset to default or toggle sawtooth polarity
		} else if (waveform == 11) {
			square_wave(1, 5);

		// decrease percent duty cycle
		} else if (waveform == 13) {
			duty_globe 	-= 1;
			if (duty_globe == 0) {
				duty_globe = 1;
			}
			square_wave(freq_globe, duty_globe);

		// increase percent duty cycle
		} else if (waveform == 15) {
			duty_globe 	+= 1;
			if (duty_globe == 10) {
				duty_globe = 9;
			}
			square_wave(freq_globe, duty_globe);

		// reset to 50 percent duty cycle or toggle sawtooth polarity
		} else if (waveform == 14) {
			duty_globe = 5;
			if (wave_type == 1) {
				ramp_wave(freq_globe, 0);
			}
		// keypad entry for frequency change
		} else if (waveform >= 1 && waveform <= 6) {
			if (waveform > 4) {
				waveform -= 1;
			}
			freq_globe = waveform;
			if (wave_type == 0) {
				square_wave(freq_globe, duty_globe);
			}
		}

		TIM2->CNT = 0;
		__enable_irq();
	}
}	/* USER CODE END 3 */


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
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
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
	    if (wave_type == 0) {
	    	DAC_write(4096 + 3000);
	    }
	}
	if (TIM2->SR & TIM_SR_UIF) {        // triggered by ARR event ...
	    TIM2->SR &= ~(TIM_SR_UIF);       // manage the flag

	    // wave type indicates wave type 2->sine
	    if (wave_type == 2) {
	    	DAC_write(sine_table[counter]);
	    	counter += (freq_globe * 3);
	    	if (counter >= 3000) {
	    		counter = 0;
	    	}
	    }

	    // if creating a square wave
	    if (wave_type == 0) {
	    	DAC_write(4096 + 1500);
	    }
	    // if producing a ramp wave
	    if (wave_type == 1) {
	    	DAC_write(counter + 4096);
	    	// if positive ramp slope
	    	if (ramp_type == 1) {
	    		counter += (freq_globe * 3);
	    		if (counter >= 3000) {
	    			counter = 0;
	    		}
	    	}
	    	// if negative ramp slope
	    	if (ramp_type == 0) {
	    		counter -= (freq_globe * 3);
	    		if (counter <= 0) {
	    			counter = 3000;
	    		}
	    	}
	    }
	}
}

// frequency to indicated requested frequency (0-5)
// direction for positive or negative ramp (1 for positive, 0 for negative)
void ramp_wave(int frequency, int direction) {

	wave_type = 1;
	freq_globe = frequency;
	ramp_type = direction;
	if (direction == 1) {
		counter = 0;
	} else {
		counter = 3000;
	}
	TIM2->ARR = 480;
	TIM2->CCR1 = 240;
}


// frequency to indicated requested frequency (0-5)
// duty cycle (1-9) indicates duty cycle percentage times 10
void square_wave(int frequency, int duty_cycle) {

	wave_type = 0;
	freq_globe = frequency;
	duty_globe = duty_cycle;
	float duty_percent = (10.0 - duty_cycle) / 10.0;
	TIM2->ARR = (480000 / frequency);
	TIM2->CCR1 = ((480000 / frequency) * duty_percent);
}


// frequency indicates frequency of sinusoidal waveform (1-9)
void sine_wave(int frequency) {
	// setting global waveform type variable to 2 (sin)
	wave_type = 2;
	counter = 0;
	freq_globe = frequency;
	TIM2->ARR = 480;
	TIM2->CCR1 = 240;
}

// credit: Pranav Mulpuru
// generating sin array to read from when sampling sine function
void createSineTable(double sineTable[]) {
    for (int i = 0; i < TABLE_SIZE; i++) {              // Sine Table length (data points)
        double radian = (2*M_PI*i)/TABLE_SIZE;          // Calc evenly distributed values
        sine_table[i] = DAC_volt_convert((HALF_VMAX*sin(radian)+DC_OFFSET) * 100) + 4096; // Amplify & shift sine wave

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
