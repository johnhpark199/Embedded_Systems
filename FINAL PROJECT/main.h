/*
 ******************************************************************************
* file      : main.h
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
* 2023-DEC-1 created.
******************************************************************************
* origins   :
* modified from CubeIDE auto-generated main.c  (c) 2023 STMicro.
* modified from EE329 Lab Manual Sample Code
*****************************************************************************/

#ifndef __MAIN_H
#define __MAIN_H

#define MEASURE 0
#define NONE 1
#define PITCH 2
#define ROLL 3
#define BOTH 4

#ifdef __cplusplus
extern "C" {
#endif
#

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

#include "custom.h"

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
