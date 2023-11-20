/*
 ******************************************************************************
* file      : timer_inter.c
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
#include "LCD.h"
#include "keypad.h"
#include "timer_inter.h"

void delay_us(const uint32_t time_us);


void setup_MCO_CLK(void) {
   // Enable MCO, select MSI (4 MHz source)
   RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_MCOSEL)) | (RCC_CFGR_MCOSEL_0));
   // Configure MCO output on PA8
   RCC->AHB2ENR   |=  (RCC_AHB2ENR_GPIOAEN);
   GPIOA->MODER   &= ~(GPIO_MODER_MODE8);    	// alternate function mode
   GPIOA->MODER   |=  (GPIO_MODER_MODE8_1);
   GPIOA->OTYPER  &= ~(GPIO_OTYPER_OT8);     	// Push-pull output
   GPIOA->PUPDR   &= ~(GPIO_PUPDR_PUPD8);    	// no resistor
   GPIOA->OSPEEDR |=  (GPIO_OSPEEDR_OSPEED8);   // high speed
   GPIOA->AFR[1]  &= ~(GPIO_AFRH_AFSEL8);    	// select MCO function
}


void setup_OBLED(void) {

	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN);

	// clearing output pins (PC0-3) for LEDS
	GPIOB->MODER   &= ~(GPIO_MODER_MODE7);

	// setting PC0-3 to output mode (01), so LEDS can be turned on and off
	GPIOB->MODER   |=  (GPIO_MODER_MODE7_0);
		// setting type to push pull (0)
	GPIOB->OTYPER  &= ~(GPIO_OTYPER_OT7);

	// setting output pins to (00) no pull up or pull down resistor
	GPIOB->PUPDR   &= ~(GPIO_PUPDR_PUPD7);

	// setting output bits to (11) for PC0-3 (very fast mode)
	GPIOB->OSPEEDR |=  (3 << GPIO_OSPEEDR_OSPEED7_Pos);
	// setting LEDS to off initialing (0) using BRR
	GPIOB->BRR = (GPIO_PIN_7);
}


void setup_PBSW(void) {

	// assuming clock for GPIOC is already initialized
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOCEN);

	// setting input pins to 00 (input mode) of MODER (PD4-7) (clearing and setting MODER)
	GPIOC->MODER &= ~(GPIO_MODER_MODE13);

	// setting output type of output pins to (0) output push pull state
	GPIOC->OTYPER &= ~(GPIO_PUPDR_PUPD13);

	// setting all pins for either pull up or down (00) no resistor (clearing)
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD13);

	//
	GPIOC->PUPDR |= (GPIO_PUPDR_PUPD13_1);

	// preset columns to high (PD0-3 are columns)
	// setting output speed to (11) very high speed for all input and output bits (PD0-3)
	GPIOC->OSPEEDR |= (3 << GPIO_OSPEEDR_OSPEED13_Pos);

	// preset rows to low (PD4-7 are rows)
	//GPIOC->BRR = (GPIO_PIN_13);

}

// returns 2 if true otherwise returns 1 or 0
int PBSW_debounce(void ) {
	int test_1 = 0;
	// check if user PBSW is pressed
	if((GPIOC->IDR & (GPIO_PIN_13)) != 0) {
		test_1 += 1;
	}

	// wait 500 us
	delay_us(500);

	// check again
	if ((GPIOC->IDR & GPIO_PIN_13) != 0) {
		test_1 += 1;
	}
	return test_1;
}

/*
 **

Function:      RNG_Init
Credit:        Devon Bolt
Date of File:  October 22, 2023
Discription:   This functiomn is used to initilize the RNG random number
generator in order to use the internal number generator of
the board
***/
// Sets the bits in the RNG control register for proper operation
void RNG_Init(void){
    RCC->AHB2ENR |= (RCC_AHB2ENR_RNGEN);     //enable rng clk
    RCC->CRRCR |= (RCC_CRRCR_HSI48ON);     //enable other rng clk
    RNG->CR |= RNG_CR_CED;         // clock error detection disabled
    RNG->CR |= RNG_CR_RNGEN;     // RNG enabled
}

/*
 **

Function:      RNG_Get_Rand
Credit:        Devon Bolt
Date of File:  October 22, 2023
Discription:   This functiomn is used to obtain the value of the random
number generator from the parameter set of max and returning
it a random value set by the parammeter
***/
// Returns a random positive number less than or equal to max
int RNG_Get_Rand(int max){
    while(((RNG->SR) | RNG_SR_DRDY) != 1){        // waiting for new valid RNG value
        ;
    }
    return (RNG->DR) % (max+1);        // modulating the 32 bit random number so that it is bounded at max
}
