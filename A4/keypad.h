/*
 ******************************************************************************
* file      : keypad.h
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
/*--------------------------------------------------------------------------*/

int Keypad_IsAnyKeyPressed(void);
int Keypad_WhichKeyIsPressed(void);
int Keypad_Debounce(int pressed);
void Keypad_Config(void);
void light_LEDs(int keypress);
char Keypad_return_char(char key);
int Keypad_read();

#ifndef SRC_KEYPAD_H_
#define SRC_KEYPAD_H_
#define BIT0 1
#define NUM_ROWS 4
#define NUM_COLS 4

// setting to GPOID AND PIN13 FOR USER PBSW
#define COL_PORT GPIOD
#define COL_PINS (GPIO_PIN_13)

#define ROW_PORT GPIOD
#define ROW_PINS (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)

#define NO_KEYPRESS 999
#define SETTLE 10000
#define TRUE 20
#define FALSE 21

#endif /* SRC_KEYPAD_H_ */
