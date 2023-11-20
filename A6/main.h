/*
 ******************************************************************************
* file      : main.h
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#define TABLE_SIZE 3000
#define HALF_VMAX 1.5
#define DC_OFFSET 1.5

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
