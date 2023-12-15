/*
 ******************************************************************************
* file      : main.c
* project   : EE 329 P23 Mustang
* details   : I2C/EEPROM
* authors   : John Park
* 			  Chris Hartmann
* 			  Breynyn Northey
*           :
* version   : 0.1
* date      : 2023-DEC-8
* compiler  : STM32CubeIDE Version: 1.13.1 (2023)
* target    : NUCLEO-L4A6ZG
 ******************************************************************************
 * revisions :
* 2023-DEC-8 created.
******************************************************************************
* origins   :
* modified from CubeIDE auto-generated main.c  (c) 2023 STMicro.
* modified from EE329 Lab Manual Sample Code
*****************************************************************************/

// #includes
#include "main.h"
#include "app_mems.h"
#include "iks01a3_motion_sensors.h"
#include "iks01a3_motion_sensors_ex.h"
#include "flight_control.h"
#include "servo_control.h"
#include <math.h>

// function prototypes
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int32_t IKS01A3_MOTION_SENSOR_Init(uint32_t Instance, uint32_t Functions);
int32_t IKS01A3_MOTION_SENSOR_Enable(uint32_t Instance, uint32_t Function);
int32_t IKS01A3_MOTION_SENSOR_GetAxes(uint32_t Instance, uint32_t Function, IKS01A3_MOTION_SENSOR_Axes_t *Axes);
int32_t IKS01A3_MOTION_SENSOR_DeInit(uint32_t Instance);
double get_roll_angle(int y, int z);
double get_pitch_angle(int x, int z);
void servo_init(void);
void setup_TIM2(int pulseWidth);
void setup_TIM5(int pulseWidth);
void TIM5_IRQHandler(void);
void TIM2_IRQHandler(void);
void SysTick_Init(void);
void delay_us(const uint32_t time_us);
void change_servo_pos_left(double pos);
void change_servo_pos_right(double pos);
double find_servo_out(double pitch_angle);
double comp_pitch(double pitchVar);
double comp_roll(double rollVar);
double comp_roll_demo(double rollVar);
double comp_pitch_demo(double pitchVar);

// global variables
int state;
double servo_pos;
double pitchVar;
double rollVar;


int main(void) {

	// the pins would have been initialized without the help of HAL if documentation
	// had provided better description of which pins to initialize and what their
	// functions are
	HAL_Init();

	// clock init
	SystemClock_Config();

	// initializing pins, same as HAL init, if better daughter board documentation provided
	// this would have been done by initializing registers and setting to specified function mode
	MX_GPIO_Init();
	// turning on SysTick, required for "delay_us()"
	SysTick_Init();

	// initializing accelerometer, return values if not 0 indicate an error in init ot enable
	int32_t ret1 = IKS01A3_MOTION_SENSOR_Init(IKS01A3_LSM6DSO_0, MOTION_ACCELERO);
	int32_t ret4 = IKS01A3_MOTION_SENSOR_Enable(IKS01A3_LSM6DSO_0, MOTION_ACCELERO);

	// enabling servo pins and interrupts
	servo_init();

	// accelerometer storage data variable
	IKS01A3_MOTION_SENSOR_Axes_t accel_data;

	// threshold for making adjustments to the flap positions, if angle falls below the abs of this
	// value, don't change flap position
	double deadband = 3;

	while (1) {

		// initializing data values to 0
		int32_t x_data_accel = 0;
		int32_t y_data_accel = 0;
		int32_t z_data_accel = 0;
		// waiting so PWM has time to write previous value to servo
		delay_us(500000);

		// measuring output data
		// taking 20 samples of accelerometer data
		for(int i = 0; i < 30; i++) {
			int32_t ret3 = IKS01A3_MOTION_SENSOR_GetAxes(IKS01A3_LSM6DSO_0, MOTION_ACCELERO,&accel_data);

			// if get axes returns a valid code add data
			if (ret3 == 0) {
				x_data_accel += accel_data.x;
				y_data_accel += accel_data.y;
				z_data_accel += accel_data.z;

			// otherwise go back one in loop and retake data
			} else {
				i -= 1;
			}
		}
		// taking average of 30 samples
		x_data_accel /= 30;
		y_data_accel /= 30;
		z_data_accel /= 30;

		// getting pitch and roll angles
		double pitchVar = get_pitch_angle(y_data_accel, z_data_accel);
		double rollVar = get_roll_angle(x_data_accel, z_data_accel);

		// if both values are greater enter roll state
		if((fabs(pitchVar) > deadband) && (fabs(rollVar) > deadband))
			state = BOTH;

		// if roll only needs to be changed enter roll state
		else if ((fabs(pitchVar) < deadband) && (fabs(rollVar) > deadband))
			state = ROLL;
		// if only pitch needs to be changed
		else if ((fabs(pitchVar) > deadband) && (fabs(rollVar) < deadband))
			state = PITCH;
		//if neither positions need to be changed
		else if ((fabs(pitchVar) < deadband) && (fabs(rollVar) < deadband))
			state = NONE;


		switch(state) {
			// no change
			case NONE:
				change_servo_pos_right(0.5);
				change_servo_pos_left(0.5);
				break;

			// changing for specified pitch angle
			case PITCH:
				servo_pos = comp_pitch_demo(pitchVar);
				change_servo_pos_right(servo_pos);
				change_servo_pos_left(1 - servo_pos);
				break;

			// changing for roll angle
			case ROLL:
				servo_pos = comp_roll_demo(rollVar);
				change_servo_pos_right(servo_pos);
				change_servo_pos_left(servo_pos);
				break;

			// changing for both pitch and roll correction
			case BOTH:
				double pitch = comp_pitch_demo(pitchVar);
				double roll = comp_roll_demo(rollVar);
				double avg_pos = (pitch + roll) / 2.0;
				change_servo_pos_right(avg_pos);
				change_servo_pos_left(avg_pos);
				break;

			default:
				break;
		}
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin : PC1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB10 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_10|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
