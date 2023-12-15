/*
 ******************************************************************************
* file      : servo_control.c
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

#include "servo_control.h"
#include "main.h"

// initialzing pins for servos
void servo_init(void) {

	// PC0->2 of MUX
	// init servo pin
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOCEN);
	GPIOC->MODER &= ~(GPIO_MODER_MODE0);
	GPIOC->MODER |= (GPIO_MODER_MODE0_0);
	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT0);
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD0);
	GPIOC->OSPEEDR |= (3 << GPIO_OSPEEDR_OSPEED0_Pos);


	// GPIOC PC3->1 OF MUX
	GPIOC->MODER &= ~(GPIO_MODER_MODE3);
	GPIOC->MODER |= (GPIO_MODER_MODE3_0);
	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT3);
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD3);
	GPIOC->OSPEEDR |= (3 << GPIO_OSPEEDR_OSPEED3_Pos);

	// enabling both interrupts at middle position (750) for both
	setup_TIM2(750);
	setup_TIM5(750);

}

// initializing TIM2
void setup_TIM2(int pulseWidth) {
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN; // enable clock for TIM2
	TIM2->DIER |= (TIM_DIER_CC1IE | TIM_DIER_UIE); // enable event gen, rcv CCR1
	TIM2->ARR = ARR_Period; // ARR = T = counts @4MHz
	TIM2->CCR1 = ((pulseWidth * ARR_Period) / 10000); // ticks for duty cycle
	TIM2->SR &= ~(TIM_SR_CC1IF | TIM_SR_UIF); // clr IRQ flag in status reg
	NVIC->ISER[0] |= (1 << (TIM2_IRQn & 0x1F)); // set NVIC interrupt: 0x1F
	__enable_irq(); // global IRQ enable
	TIM2->CR1 |= TIM_CR1_CEN; // start TIM2 CR1
}

// initializing TIM5
void setup_TIM5(int pulseWidth) {
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN; // enable clock for TIM5
	TIM5->DIER |= (TIM_DIER_CC1IE | TIM_DIER_UIE); // enable event gen, rcv CCR1
	TIM5->ARR = ARR_Period; // ARR = T = counts @4MHz
	TIM5->CCR1 = ((pulseWidth * ARR_Period) / 10000); // ticks for duty cycle
	TIM5->SR &= ~(TIM_SR_CC1IF | TIM_SR_UIF); // clr IRQ flag in status reg
	NVIC->ISER[1] |= (1 << (TIM5_IRQn & 0x1F)); // set NVIC interrupt: 0x1F
	__enable_irq(); // global IRQ enable
	TIM5->CR1 |= TIM_CR1_CEN; // start TIM2 CR1
}

// TIM5 ISR
void TIM5_IRQHandler(void) {
	if (TIM5->SR & TIM_SR_CC1IF)
	{ // triggered by CCR1 event ...
		TIM5->SR &= ~(TIM_SR_CC1IF); // manage the flag
		GPIOC -> BRR = GPIO_PIN_0; //toggle PIN 5 // <-- manage GPIO pin here
	}
	if (TIM5->SR & TIM_SR_UIF)
	{ // triggered by ARR event ...
		TIM5->SR &= ~(TIM_SR_UIF); // manage the flag
		GPIOC->BSRR = GPIO_PIN_0; //toggle PIN
	}
}

// TIM2 ISR
void TIM2_IRQHandler(void) {
	if (TIM2->SR & TIM_SR_CC1IF)
	{ 	// triggered by CCR1 event ...
		TIM2->SR &= ~(TIM_SR_CC1IF); // manage the flag
		GPIOC->BRR = (GPIO_PIN_3); //toggle PIN 5 // <-- manage GPIO pin here
	}
	if (TIM2->SR & TIM_SR_UIF)
	{ 	// triggered by ARR event ...
		TIM2->SR &= ~(TIM_SR_UIF); // manage the flag
		GPIOC->BSRR = (GPIO_PIN_3); //toggle PIN
	}
}

// updating CCR1 value and resetting count to create new PWM signal (TIM5) / right servo
void change_servo_pos_right(double pos) {

	// casting from 0->1 double to int for setting CCR1 value
	int pulseWidth = (pos * 500) + 500;
	// setting CCR1 value
	TIM5->CCR1 = ((pulseWidth * ARR_Period) / 10000);
	// resetting count value
	TIM5->CNT = 0;
}

// updating CCR1 value and resetting count to create new PWM signal (TIM2) / left servo
void change_servo_pos_left(double pos) {

	// casting from 0->1 double to int for setting CCR1 value
	int pulseWidth = (pos * 500) + 500;
	// setting CCR1 value
	TIM2->CCR1 = ((pulseWidth * ARR_Period) / 10000);
	// resetting count value
	TIM2->CNT = 0;
}

// Systick init
void SysTick_Init(void) {
	SysTick->CTRL |= (SysTick_CTRL_ENABLE_Msk |     	// enable SysTick Timer
                      SysTick_CTRL_CLKSOURCE_Msk); 	// select CPU clock
	SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk);  	// disable interrupt
}

// delay in us
void delay_us(const uint32_t time_us) {
	// set the counts for the specified delay

	SysTick->LOAD = (uint32_t)((time_us * (SystemCoreClock / 1000000)) - 1);

	SysTick->VAL = 0;                                  	 // clear timer count

	SysTick->CTRL &= ~(SysTick_CTRL_COUNTFLAG_Msk);    	 // clear count flag

	while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)); // wait for flag
}
