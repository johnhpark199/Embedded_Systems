/*
 ******************************************************************************
* file      : UART.c
* project   : EE 329 F'23 A7
* details   : LPUART supplmental file
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
 */

#include "UART.h"
#include "main.h"
#include <stdio.h>

void LPUART_ESC_print( const char* message );
void LPUART_print( const char* message );

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
	LPUART1->CR1 |= (USART_CR1_TE | USART_CR1_RE);  // enable xmit & recv
	LPUART1->CR1 |= USART_CR1_RXNEIE;        // enable LPUART1 recv interrupt
	LPUART1->ISR &= ~(USART_ISR_RXNE);       // clear Recv-Not-Empty flag
	/* USER: set baud rate register (LPUART1->BRR) */
	LPUART1->BRR = 8889;
	NVIC->ISER[2] = (1 << (LPUART1_IRQn & 0x1F));   // enable LPUART1 ISR
	__enable_irq();                          // enable global interrupts

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

// assuming 30 rows by 80 columns terminal
// displaying game splash screen
void LPUART_splash(void) {

	// setting background to blue
	LPUART_ESC_print("[44m");
	// text color to white
	LPUART_ESC_print("[37m");
	// returning to home
	LPUART_ESC_print("[H");
	// printing top line
	for (int i = 0; i < 8; i++) {
		LPUART_print("----------");
	}
	// printing borders of start screen
	for (int j = 0; j < 12; j++) {
		LPUART_print("|");
		for (int k = 0; k < 8; k++) {
			LPUART_print("         ");
		}
		LPUART_print("      |");
	}

	// display game name and edges
	LPUART_print("|");
	for (int k = 0; k < 4; k++) {
		LPUART_print("        ");
	}
	LPUART_print("SNAIL JAIL!!??");
	for (int k = 0; k < 4; k++) {
		LPUART_print("        ");
	}
	LPUART_print("|");

	// printing edges below text
	for (int j = 0; j < 15; j++) {
		LPUART_print("|");
		for (int k = 0; k < 8; k++) {
			LPUART_print("         ");
		}
		LPUART_print("      |");
	}
	// printing bottom line
	for (int i = 0; i < 8; i++) {
		LPUART_print("----------");
	}
}

// drawing box for object (snail) to move around in
void LPUART_draw_snail_jail(void) {

	LPUART_ESC_print("[2J");
	// move to (5,5)
	LPUART_ESC_print("[8;20H");
	// print top line
	for(int i = 0; i < 8; i++) {
		LPUART_print("-----");
	}
	LPUART_print("-");
	// move to next location
	LPUART_ESC_print("[9;20H");

	// printing edges of box
	for(int j = 0; j < 15; j++) {
		LPUART_print("|");
		for(int k = 0; k < 8; k++) {
			LPUART_print("    ");
		}
		LPUART_print("       |");
		for(int l = 0; l < 39; l++) {
			LPUART_print(" ");
		}
	}
	// printing bottom line
	for(int i = 0; i < 8; i++) {
		LPUART_print("-----");
	}
	LPUART_print("-");
	// moving to approximate center
	// there is no true center due to size of box
	LPUART_ESC_print("[16;41H");
	// set text color to white
	LPUART_ESC_print("[37m");
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

