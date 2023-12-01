/*
 ******************************************************************************
* file      : ADC.c
* project   : EE 329 F'23 A8
* details   : ADC
* authors   : John Park
*           :
* version   : 0.1
* date      : 2023-NOV-??
 * compiler  : STM32CubeIDE Version: 1.13.1 (2023)
* target    : NUCLEO-L4A6ZG
 ******************************************************************************
 * revisions :
* 2023-NOV-20 created.
******************************************************************************
* origins   :
* modified from CubeIDE auto-generated main.c  (c) 2023 STMicro.
* modified from EE329 Lab Manual Sample Code
******************************************************************************

includes------------------------------------------------------------------- */
#include "UART.h"
#include "main.h"

void SysTick_Init(void);
void delay_us(const uint32_t time_us);

void config_UART(void) {

	PWR->CR2 |= (PWR_CR2_IOSV);              // power avail on PG[15:2] (LPUART1)
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOGEN);   // enable GPIOG clock
	RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN; // enable LPUART clock bridge


	/* USER: configure GPIOG registers MODER/PUPDR/OTYPER/OSPEEDR then
	   select AF mode and specify which function with AFR[0] and AFR[1] */

	GPIOG->MODER &= ~(GPIO_MODER_MODE7 | GPIO_MODER_MODE8);

	GPIOG->MODER |= (GPIO_MODER_MODE7_1 | GPIO_MODER_MODE8_1);

	GPIOG->OTYPER &= ~(GPIO_PUPDR_PUPD7 | GPIO_PUPDR_PUPD8);

	// setting output speed to very high (11)
	GPIOG->OSPEEDR |= ((3 << GPIO_OSPEEDR_OSPEED7_Pos) |
						(3 << GPIO_OSPEEDR_OSPEED8_Pos));

	GPIOB->PUPDR |= (GPIO_PUPDR_PUPD7_1 | GPIO_PUPDR_PUPD8_1);

	// Setting for AF7 using AFR[0] or AFR[1]
	GPIOG->AFR[0] &= ~((0x000F << GPIO_AFRL_AFSEL7_Pos));
	GPIOG->AFR[0] |=  ((0x0008 << GPIO_AFRL_AFSEL7_Pos));

	// Setting for AF8 using AFR[0] or AFR[1]
	GPIOG->AFR[1] &= ~((0x000F << GPIO_AFRH_AFSEL8_Pos));
	GPIOG->AFR[1] |=  ((0x0008 << GPIO_AFRH_AFSEL8_Pos));

	LPUART1->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0); // 8-bit data
	LPUART1->CR1 |= USART_CR1_UE;                   // enable LPUART1
	LPUART1->CR1 |= (USART_CR1_TE );  // enable xmit & recv
	/* USER: set baud rate register (LPUART1->BRR) */

	// changed to account for 24 MHz clock, also removed receiving register configuration
	// receiving not required, so no point in having an extra ISR and interrupts enabled
	// still 115,200 buad rate
	LPUART1->BRR = 53333;               // enable global interrupts

}

void LPUART_print( const char* message ) {
	uint16_t iStrIdx = 0;

	while ( message[iStrIdx] != 0 ) {
		while(!(LPUART1->ISR & USART_ISR_TXE)) // wait for empty xmit buffer
			;
		LPUART1->TDR = message[iStrIdx];       // send this character
		iStrIdx++;                             // advance index to next char
	}
}



void LPUART_ESC_print( const char* message ) {
	// creating variable for escape string
	char* ESC = "\033";
	// sending ESC start string
	for(int i = 0; i < 4; i++ ) {
		while(!(LPUART1->ISR & USART_ISR_TXE));
		LPUART1->TDR = ESC[i];
	}

	uint16_t iStrIdx = 0;
	while ( message[iStrIdx] != 0 ) {
		while(!(LPUART1->ISR & USART_ISR_TXE)) // wait for empty xmit buffer
			;
		LPUART1->TDR = message[iStrIdx];       // send this character
		iStrIdx++;      // advance index to next char
	}
}

// configure SysTick timer for use with delay_us()
void SysTick_Init(void) {
	SysTick->CTRL |= (SysTick_CTRL_ENABLE_Msk |     	// enable SysTick Timer
                      SysTick_CTRL_CLKSOURCE_Msk); 	// select CPU clock
	SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk);  	// disable interrupt
}


void delay_us(const uint32_t time_us) {
	// set the counts for the specified delay

	SysTick->LOAD = (uint32_t)((time_us * (SystemCoreClock / 1000000)) - 1);

	SysTick->VAL = 0;                                  	 // clear timer count

	SysTick->CTRL &= ~(SysTick_CTRL_COUNTFLAG_Msk);    	 // clear count flag

	while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)); // wait for flag
}
