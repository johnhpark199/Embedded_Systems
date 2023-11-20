/*
 ******************************************************************************
* file      : DAC.c
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
#include "DAC.h"
#include "main.h"


void SPI_init( void ) {

	// SPI config as specified @ STM32L4 RM0351 rev.9 p.1459
	// called by or with DAC_init()
	// build control registers CR1 & CR2 for SPI control of peripheral DAC
	// assumes no active SPI xmits & no recv data in process (BSY=0)
	// CR1 (reset value = 0x0000)
	SPI1->CR1 &= ~( SPI_CR1_SPE );             	// disable SPI for config
	SPI1->CR1 &= ~( SPI_CR1_RXONLY );          	// recv-only OFF
	SPI1->CR1 &= ~( SPI_CR1_LSBFIRST );        	// data bit order MSb:LSb
	SPI1->CR1 &= ~( SPI_CR1_CPOL | SPI_CR1_CPHA ); // SCLK polarity:phase = 0:0
	SPI1->CR1 |=	 SPI_CR1_MSTR;              	// MCU is SPI controller
	// CR2 (reset value = 0x0700 : 8b data)
	SPI1->CR2 &= ~( SPI_CR2_TXEIE | SPI_CR2_RXNEIE ); // disable FIFO intrpts
	SPI1->CR2 &= ~( SPI_CR2_FRF);              	// Moto frame format
   	SPI1->CR2 |=	 SPI_CR2_NSSP;              	// auto-generate NSS pulse
   	SPI1->CR2 |=	 SPI_CR2_DS;                	// 16-bit data
   	SPI1->CR2 |=	 SPI_CR2_SSOE;              	// enable SS output
   	// CR1
   	SPI1->CR1 |=	 SPI_CR1_SPE;               	// re-enable SPI for ops

}


// initializing DAC function
void DAC_init(void) {
	// PB0->SPI 1NSS = CS
	// PB3->SP1_SCK = SCK
	// PB5->SPI1_MOSI = SDI
	// VDD -> HIGH GPIO PIN
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN);                // GPIOB: DAC NSS/SCK/SDO
	RCC->APB2ENR |= (RCC_APB2ENR_SPI1EN);                 // SPI1 port

	/* USER ADD GPIO configuration of MODER/PUPDR/OTYPER/OSPEEDR registers HERE */
	// Clearing MODER registers
	GPIOB->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE3 | GPIO_MODER_MODE5);

	// setting output pins alternate function mode (10)
	GPIOB->MODER |= (GPIO_MODER_MODE0_1 | GPIO_MODER_MODE3_1 | GPIO_MODER_MODE5_1);

	// setting output type of output pins to (0) output push pull state
	GPIOB->OTYPER &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD3 | GPIO_PUPDR_PUPD5);

	// setting output speed to very high (11)
	GPIOB->OSPEEDR |= ((3 << GPIO_OSPEEDR_OSPEED0_Pos) |
	        (3 << GPIO_OSPEEDR_OSPEED3_Pos) |
			(3 << GPIO_OSPEEDR_OSPEED5_Pos));

	// setting resistors to pull down
	GPIOB->PUPDR |= (GPIO_PUPDR_PUPD0_1 | GPIO_PUPDR_PUPD3_1 | GPIO_PUPDR_PUPD5_1);


	// configure AFR for SPI1 function (1 of 3 SPI bits shown here)
	GPIOB->AFR[0] &= ~((0x000F << GPIO_AFRL_AFSEL0_Pos)); // clear nibble for bit 7 AF
	GPIOB->AFR[0] |=  ((0x0005 << GPIO_AFRL_AFSEL0_Pos)); // set b7 AF to SPI1 (fcn 5)

	GPIOB->AFR[0] &= ~((0x000F << GPIO_AFRL_AFSEL3_Pos));
	GPIOB->AFR[0] |=  ((0x0005 << GPIO_AFRL_AFSEL3_Pos));

	GPIOB->AFR[0] &= ~((0x000F << GPIO_AFRL_AFSEL5_Pos));
	GPIOB->AFR[0] |=  ((0x0005 << GPIO_AFRL_AFSEL5_Pos));


}


uint16_t DAC_volt_convert(float input) {
	float scale = 100.0;
	// convert to actual value user requests
	float bit_val = input / scale;

	// converting desired voltage percentage of 3.3 volts requested
	// account for 4.096 max voltage reference
	float percent_of_max = bit_val / 4.096;
	// changing value to 12 bit unsigned that DAC register can read
	int DAC_val = percent_of_max * 4095;
	return DAC_val;

}

void DAC_write(uint16_t data) {

	while (!(SPI1->SR & SPI_SR_TXE));

	// Write data to SPI data register to send
	SPI1->DR = data;

	// Wait until SPI transmission is complete
	while (SPI1->SR & SPI_SR_BSY);

}

