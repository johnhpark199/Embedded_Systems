/*
 ******************************************************************************
* file      : main.c
* project   : EE 329 F'23 A7
* details   : LPUART control for moving around box
* authors   : John Park
*           :
* version   : 0.1
* date      : 2023-NOV-15
 * compiler  : STM32CubeIDE Version: 1.13.1 (2023)
* target    : NUCLEO-L4A6ZG
 ******************************************************************************
 * revisions :
* 2023-NOV-14 created.
******************************************************************************
* origins   :
* modified from CubeIDE auto-generated main.c  (c) 2023 STMicro.
* modified from EE329 Lab Manual Sample Code
******************************************************************************

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "UART.h"
#include "stdlib.h"
#include <stdio.h>

// prototyping
void SystemClock_Config(void);
void LPUART_print( const char* );
void config_UART(void);
void LPUART1_IRQHandler( void  );
void LPUART_ESC_print( const char* message );
void A7_part4(void);
void LPUART_splash(void);
void SysTick_Init(void);
void delay_us(const uint32_t time_us);
void LPUART_draw_snail_jail(void);
int	abs (int);


// 0 for right
// 1 for left
// 2 for down
// 3 for up
int cursor_dir = 4;


int main(void) {
    HAL_Init();
    SystemClock_Config();
    config_UART();
    SysTick_Init();

    // clearing the screen
    LPUART_ESC_print("[2J");
    LPUART_splash();
    delay_us(50000000);
    LPUART_draw_snail_jail();

    int vert = 0;
    int horiz = 0;
    while (1) {

    	// moving left
    	if(cursor_dir == 1) {
    		horiz--;
    		// if hitting edge of boundary
    		if (horiz <= -20) {
    			char line_num[2];
    			// creating line num string
    			sprintf(line_num, "%d", -vert + 16);
    			char command[7] = "[";
    			strcat(command, line_num);
    			strcat(command, ";59H");
    			LPUART_ESC_print(command);
    			horiz = 19;
    		} else {
    			LPUART_ESC_print("[2D");
    		}

    		LPUART_print("a");
    		cursor_dir = 4;

    	// moving down
    	} else if(cursor_dir == 2) {
    		vert--;
    		// if hitting edge of boundary
    		if (vert <= -8) {
    			// casting int value of column number to string and concatenating it
    			// for LPUART_ESC_print function
    			char column_num[2];
    			sprintf(column_num, "%d", horiz + 40);
    			char line_num[7] = "[9;";
    			strcat(line_num, column_num);
    			strcat(line_num, "H");
    			// move to desired location
    			LPUART_ESC_print(line_num);

    			vert = 7;
    		} else {
    			// left one
    			LPUART_ESC_print("[1D");
    			// down one
    			LPUART_ESC_print("[1B");

    		}
    		LPUART_print("s");
    		cursor_dir = 4;

    	// moving up
    	} else if(cursor_dir == 3) {
    		vert++;
    		// if hitting edge of boundary
    		if (vert >= 8) {
    			char column_num[2];
				sprintf(column_num, "%d", horiz + 40);
    			char line_num[8] = "[23;";
    			strcat(line_num, column_num);
    			strcat(line_num, "H");
      			// move to desired location
       			LPUART_ESC_print(line_num);

       			vert = -7;
    		} else {
    			// left one
    			LPUART_ESC_print("[1D");
    			// up one
    			LPUART_ESC_print("[1A");

    		}
    		// printing character
    		LPUART_print("w");
    		cursor_dir = 4;

    	// else moving right, handling writing within interrdupt
    	} else if (cursor_dir == 0) {
    		horiz++;
    		// if hitting edge of boundary
    		if (horiz >= 20) {
    			char line_num[2];
    			// creating line num string
    			sprintf(line_num, "%d", -vert + 16);
       			char command[7] = "[";
       			strcat(command, line_num);
       			strcat(command, ";21H");
       			LPUART_ESC_print(command);
       			horiz = -19;
       		} else {

    		}
    		LPUART_print("d");
    		cursor_dir = 4;
    	}

    }
  /* USER CODE END 3 */
}

void LPUART1_IRQHandler( void  ) {

	uint8_t charRecv;
	if (LPUART1->ISR & USART_ISR_RXNE) {
		charRecv = LPUART1->RDR;
		switch ( charRecv ) {
			// using 'w' for up
			case 'w':
				/* USER: process R to ESCape code back to terminal */
				cursor_dir = 3;
				break;
			// using a for left
			case 'a':
				//LPUART1->TDR = charRecv;
				cursor_dir = 1;
				/* USER: process R to ESCape code back to terminal */
				break;
			// using s for down
			case 's':
				cursor_dir = 2;
				/* USER: process R to ESCape code back to terminal */
				break;
			case 'd':
				cursor_dir = 0;
         /* USER : handle other ESCape code cases */
			default:
				while( !(LPUART1->ISR & USART_ISR_TXE) )
					;    // wait for empty TX buffer
		}  // end switch
	}
}

void A7_part4(void) {
	// moving cursor down 3 lines
	LPUART_ESC_print("[3B");
	// moving cursor right 5 spaces
	LPUART_ESC_print("[5C");

	// text print 1
	LPUART_print("All good students read the");

	// down 1 line
	LPUART_ESC_print("[1B");
	// left 21 spaces
	LPUART_ESC_print("[21D");

	// setting to blinking mode
	LPUART_ESC_print("[5m");
	// text print 2
	LPUART_print("Reference Manual");
	// return to top left
	LPUART_ESC_print("[H");
	// turn off blinking
	LPUART_ESC_print("[0m");
	// text entry 3
	LPUART_print("Input:");
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
