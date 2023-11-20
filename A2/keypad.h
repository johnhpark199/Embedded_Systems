/*
 ******************************************************************************
* file      : keypad.h
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
int Keypad_IsAnyKeyPressed(void);
int Keypad_WhichKeyIsPressed(void);
int Keypad_Debounce(int pressed);

#ifndef SRC_KEYPAD_H_
#define SRC_KEYPAD_H_
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

#endif /* SRC_KEYPAD_H_ */
