/*
 ******************************************************************************
* file      : util_funcs2.h
* project   : EE 329 F'23 A5
* details   : custom voltage output generator, utilizing external DAC
* authors   : John Park
*           :
* version   : 0.1
* date      : 2023-OCT-28
 * compiler  : STM32CubeIDE Version: 1.13.1 (2023)
* target    : NUCLEO-L4A6ZG
 ******************************************************************************
 * revisions :
* 2023-OCT-24 created.
******************************************************************************
* origins   :
* modified from CubeIDE auto-generated main.c  (c) 2023 STMicro.
* modified from EE329 Lab Manual Sample Code
/* Includes ------------------------------------------------------------------*/

#ifndef INC_UTIL_FUNCS2_H_
#define INC_UTIL_FUNCS2_H_

void LCD_init(void);   	              // initialize LCD
void LCD_command(uint8_t command);   // send LCD a single 8-bit command
void LCD_write_char(uint8_t letter); // write a character to the LCD
void LCD_write_str(char output[]);
void LCD_write_initial(void);
void LCD_4b_command(uint8_t command );
void LCD_pulse_ENA( void );
void LCD_config(void );
void LED_init(void);
void SysTick_Init(void);

#define LCD_PORT GPIOE
#define LCD_DATA_BITS (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)
#define LCD_RS (GPIO_PIN_0)
#define LCD_EN (GPIO_PIN_3)
#define LCD_RW (GPIO_PIN_2)

#define BIT0 1
#define NUM_ROWS 4
#define NUM_COLS 4
#define COL_PORT GPIOD
#define COL_PINS (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3)

#define ROW_PORT GPIOD
#define ROW_PINS (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)

#define NO_KEYPRESS 999
#define SETTLE 10000
#define TRUE 20
#define FALSE 21

#endif /* INC_UTIL_FUNCS2_H_ */
