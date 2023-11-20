/*
 ******************************************************************************
* file      : main.c
* project   : EE 329 F'23 A3
* details   : LCD decrementing timer with custom keypad input
* authors   : John Park
*           :
* version   : 0.1
* date      : 2023-OCT-17
 * compiler  : STM32CubeIDE Version: 1.13.1 (2023)
* target    : NUCLEO-L4A6ZG
 ******************************************************************************
 * revisions :
* 2023-OCT-13 created.
******************************************************************************
* origins   :
* modified from CubeIDE auto-generated main.c  (c) 2023 STMicro.
* modified from EE329 Lab Manual Sample Code
******************************************************************************

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "keypad.h"
#include "LCD.h"

void SystemClock_Config(void);
void SystemClock_Config(void);
void Keypad_Config(void);
int Keypad_Debounce(int pressed);
int Keypad_WhichKeyIsPressed(void);
int Keypad_IsAnyKeyPressed(void);
char Keypad_to_char(int keypress_num);
int Keypad_read();
void SysTick_Init(void);
void delay_us(const uint32_t time_us);
void LCD_config(void );
void LED_init(void);
void LCD_init( void );
void LCD_command( uint8_t command );
void LCD_write_char( uint8_t letter );
void LCD_write_str(char output[]);
void LCD_write_initial(void);
int LCD_validate_tens_input(int tens_pressed);
char int_to_char(int input_int);
int LCD_validate_ones_input(int ones_pressed);
int LCD_decrement(int time_arr[]);

int main(void) {

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

  	/* Configure the system clock */
  	SystemClock_Config();

  	LED_init();
  	Keypad_Config();
  	SysTick_Init();
  	LCD_config();
  	LCD_init();
  	LCD_write_initial();
  	// 0x80 -> top left position
  	// 0xC0 -> bottom left position
  	// 0xCC -> close to bottom right

  	while (1)  {

  		// in case reset was called and LCD was cleared
  		LCD_write_initial();
  		// reset keypad to top left if reseting process
  		LCD_command(0x80);
  		// waiting 250 ms to minimize debounce issues
  		delay_us(250000);
  		// initializing key variables to 0
  		int init_key = Keypad_read();
  		// initializing keypad reset variable
  		int keypad_reset = FALSE;
  		// wait 250 ms to prevent debounce issues
  		delay_us(250000);
  		// initializing array to store value of time
  		int time_arr[4];

  		// initialize time_arr to 0
  		for (int i = 0; i < 4; i++) {
  			time_arr[i] = 0;
  		}

  		if (init_key == 13) {
  			while (keypad_reset == FALSE) {
  				LCD_command(0xCB);

  				// assigning value to time array
  				time_arr[3] = LCD_validate_tens_input(Keypad_read());

  				if (time_arr[3] == 13) {
  					keypad_reset = TRUE;
  					// move cursor to top
  					LCD_command(0x80);
  					// clear LCD
  					LCD_command(0x01);
  					// wait for clearing to finish
  					delay_us(2000);
  					break;
  				}

  				// writing value to LCD
  				LCD_write_char(int_to_char(time_arr[3]));
  				// waiting 250 ms to prevent debounce issues
  				delay_us(250000);
  				// storing singular minutes to time_arr
  				time_arr[2] = LCD_validate_ones_input(Keypad_read());
  				if (time_arr[2] == 13) {
  					keypad_reset = TRUE;
  					// move cursor to top
  					LCD_command(0x80);
  					// clear LCD
  					LCD_command(0x01);
  					// wait for clearing to finish
  					delay_us(2000);
  					break;
  				}

  				// writing char to LCD
  				LCD_write_char(int_to_char(time_arr[2]));
  				// move to minutes section of display
  				LCD_command(0xCE);
  				// wait 250 ms to prevent debounce
  				delay_us(250000);
  				// storing 10s of secs to time_arr
  				time_arr[1] = LCD_validate_tens_input(Keypad_read());
  				if (time_arr[1] == 13) {
   					keypad_reset = TRUE;
  					// move cursor to top
  					LCD_command(0x80);
  					// clear LCD
  					LCD_command(0x01);
  					// wait for clearing to finish
  					delay_us(2000);
  				  	break;
  				}

  				// writing tens of secs to LCD
  				LCD_write_char(int_to_char(time_arr[1]));
  				// wait 250 ms
  				delay_us(250000);
  				// storing secs
  				time_arr[0] = LCD_validate_ones_input(Keypad_read());
  				if (time_arr[0] == 13) {
  				   	keypad_reset = TRUE;
  					// move cursor to top
  					LCD_command(0x80);
  					// clear LCD
  					LCD_command(0x01);
  					// wait for clearing to finish
  					delay_us(2000);
   				  	break;

  				}
  				// writing to LCD
  				LCD_write_char(int_to_char(time_arr[0]));
  				// delay 250 ms
  				delay_us(250000);

  				// if 15 is pressed start count
  				if (Keypad_read() == 15) {
  					break;
  				}
  			}
  			// entering while loop if keypad_reset hasn't been triggered
  			if (keypad_reset == FALSE) {

  				int timer_cnt = 0;
  				// initializing timer_cnt to make sure timer is not set to 0
  				for (int j = 0; j < 4; j++) {
  					timer_cnt = time_arr[j] + timer_cnt;
  				}

  				while (timer_cnt > 0 && keypad_reset == FALSE) {
  					// checking for reset
  					if (Keypad_IsAnyKeyPressed() == TRUE) {
  						if (Keypad_WhichKeyIsPressed() == 13) {
  							keypad_reset = TRUE;
  							break;
  						}
  					}

  					// decrement function
  					int temp_cnt = LCD_decrement(time_arr);
  					// integer division for 10s of mins
  					time_arr[3] = (temp_cnt / 1000) % 10;
  					// integer division to find minutes
  					time_arr[2] = (temp_cnt / 100) % 10;
  					if (time_arr[2] == 1) {
  						GPIOC->BSRR = (GPIO_PIN_0);
  					}
  					// integer division for tens of seconds
  					time_arr[1] = (temp_cnt / 10) % 10;
  					// mod to find seconds
  					time_arr[0] = temp_cnt % 10;


  					// wait 1 s
  					delay_us(1000000);
  					// move cursor to tens of mins
  					LCD_command(0xCB);
  					delay_us( 50 );
  					// waiting 50 us for LCD command
  					LCD_write_char(int_to_char(time_arr[3]));
  					LCD_write_char(int_to_char(time_arr[2]));

  					LCD_command(0xCE);
  					delay_us(50);
  					// waiting 50 us for LCD command
  					LCD_write_char(int_to_char(time_arr[1]));
  					LCD_write_char(int_to_char(time_arr[0]));

  					// updating timer_cnt
  					timer_cnt = 0;
  					for (int j = 0; j < 4; j++) {
  					  	timer_cnt = time_arr[j] + timer_cnt;


  					}
  				}
  			}
  		}
  	}
}
  /* USER CODE END 3 */


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
