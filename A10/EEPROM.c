/*
 ******************************************************************************
* file      : EEPROM.c
* project   : EE 329 F'23 A10
* details   : I2C/EEPROM
* authors   : John Park
*           :
* version   : 0.1
* date      : 2023-DEC-??
* compiler  : STM32CubeIDE Version: 1.13.1 (2023)
* target    : NUCLEO-L4A6ZG
 ******************************************************************************
 * revisions :
* 2023-NOV-29 created.
******************************************************************************
* origins   :
* modified from CubeIDE auto-generated main.c  (c) 2023 STMicro.
* modified from EE329 Lab Manual Sample Code
*****************************************************************************/

#include "EEPROM.h"
#include "main.h"

// initializes EEPROM
// PB8->I2C1_SCL (pin 6 EEPROM) -> REQUIRES PU resistor (2k)
// PB9->I2C1_SDA (pin 5 EEPROM) -> REQUIRES PU resistor (2k)
void EEPROM_init(void) {
	// configuring PB8 and PB9 for SPI alternate function mode

	// Initializing clock for GPIOD pins
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOFEN);

	// setting output type of output pins to (0) output push pull state
	GPIOF->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1);
	GPIOF->OTYPER |= (GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1);


	// setting all pins for either pull up or down (00) no resistor (clearing)
	GPIOF->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1);


	GPIOF->OSPEEDR |= (GPIO_OSPEEDR_OSPEED0 | GPIO_OSPEEDR_OSPEED1);

	//GPIOB->BSRR = (GPIO_PIN_8 | GPIO_PIN_9);

	// setting input pins to 00 (input mode) of MODER (PD4-7) (clearing and setting MODER)
	GPIOF->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1);

	// setting output pins to output mode (01) MODER (PD0-3) setting MODER
	GPIOF->MODER &= ~(GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0);
	GPIOF->MODER |= (GPIO_MODER_MODE0_1 | GPIO_MODER_MODE1_1);


	// setting PB8 and PB9 to alternate function mode
	GPIOF->AFR[0] &= ~((0x000F << GPIO_AFRL_AFSEL0_Pos));
	// shift 4 bits for AF4
	GPIOF->AFR[0] |=  ((0x0004 << GPIO_AFRL_AFSEL0_Pos));

	// setting PB8 and PB9 to alternate function mode
	GPIOF->AFR[0] &= ~((0x000F << GPIO_AFRL_AFSEL1_Pos));
	// shift 4 bits for AF4
	GPIOF->AFR[0] |= ((0x0004 << GPIO_AFRL_AFSEL1_Pos));

	// Configure I2C
	RCC->APB1ENR1 |= RCC_APB1ENR1_I2C2EN;  // enable I2C bus clock
	I2C2->CR1   &= ~( I2C_CR1_PE );        // put I2C into reset (release SDA, SCL)
	I2C2->CR1   &= ~( I2C_CR1_ANFOFF );    // filters: enable analog
	I2C2->CR1   &= ~( I2C_CR1_DNF );       // filters: disable digital
	// only works for 16 MHZ clock
	I2C2->TIMINGR = 0x00303D5B;            // 16 MHz SYSCLK timing from CubeMX
	I2C2->CR2   |=  ( I2C_CR2_AUTOEND );   // auto send STOP after transmission
	I2C2->CR2   &= ~( I2C_CR2_ADD10 );     // 7-bit address mode
	I2C2->CR1   |=  ( I2C_CR1_PE );        // enable I2C
	// FOR PENEVENNE
	// CONFIGURING PRESC, SCLDEL, AND SDADEL for I2C to function, otherwise
	// NOSTRECTCH BIT AS WELL. see 1280 and 1277 of RM
	// why doesn't this work
}

// reads from EEPROM
uint8_t EEPROM_read(void) {

	I2C2->CR2   &= ~( I2C_CR2_RD_WRN );
	I2C2->CR2 &= ~I2C_CR2_NBYTES;
	I2C2->CR2 |= ( 2 << I2C_CR2_NBYTES_Pos ); // 2 Bytes send = address
	I2C2->CR2   &= ~( I2C_CR2_SADD );      // clear device address
	I2C2->CR2   |=  ( EEPROM_ADDRESS << (I2C_CR2_SADD_Pos+1) ); // device addr SHL 1

	I2C2->CR2 |= I2C_CR2_START; // start I2C transaction
	while (!((I2C2->ISR) & (I2C_ISR_TXIS))) ; // wait for empty TDXR reg
	I2C2->TXDR = (EEPROM_MEMORY_ADDR >> 8); // write data to EEPROM
	//I2C2->ISR &= ~I2C_ISR_TXIS; // clear bit
	// repeat for address LSB
	// change NBYTES to 1; set RD_WRN bit
	// START
	while (!((I2C2->ISR) & (I2C_ISR_TXIS))) ; // wait for not empty RXDR
	// sending upper half of read location
	I2C2->TXDR = (EEPROM_MEMORY_ADDR);
	// wait

	//for (int i = 0; i < 10000000; i++);

	I2C2->CR2   |= ( I2C_CR2_RD_WRN );
	I2C2->CR2 &= ~I2C_CR2_NBYTES;
	I2C2->CR2 |= ( 1 << I2C_CR2_NBYTES_Pos );
	I2C2->CR2   &= ~( I2C_CR2_SADD );
    I2C2->CR2   |=  ( EEPROM_ADDRESS << (I2C_CR2_SADD_Pos+1) ); // device addr SHL 1
    I2C2->CR2   |=    I2C_CR2_START;       // start I2C WRITE op

    while (!((I2C2->ISR) & (I2C_ISR_RXNE))) ; // wait for not empty RXDR
    uint8_t EEPROM_RDATA = (I2C2->RXDR);    // read DATA

	return EEPROM_RDATA;
}

// writes to EEPROM
void EEPROM_write(uint8_t command) {

	I2C2->CR2   &= ~( I2C_CR2_RD_WRN );    // set WRITE mode
	I2C2->CR2   &= ~( I2C_CR2_NBYTES );    // clear Byte count
	I2C2->CR2   |=  ( 3 << I2C_CR2_NBYTES_Pos); // write 3 bytes (2 addr, 1 data)
	I2C2->CR2   &= ~( I2C_CR2_SADD );      // clear device address
	I2C2->CR2   |=  ( EEPROM_ADDRESS << (I2C_CR2_SADD_Pos+1) ); // device addr SHL 1
	I2C2->CR2   |=    I2C_CR2_START;       // start I2C WRITE op
	/* USER wait for I2C_ISR_TXIS to clear before writing each Byte, e.g. ... */
	// sending upper 8 bits
	while(!(I2C2->ISR & I2C_ISR_TXIS)) ;   // wait for start condition to transmit
	I2C2->TXDR = (EEPROM_MEMORY_ADDR >> 8); // xmit MSByte of address
	/* address high, address low, data  -  wait at least 5 ms before READ
	   the READ op has new NBYTES (WRITE 2 then READ 1) & new RD_WRN for 3rd Byte */
	// sending lower 8 bits
	while(!(I2C2->ISR & I2C_ISR_TXIS)) ;
	I2C2->TXDR = (EEPROM_MEMORY_ADDR);

	// sending 8 bit message/characters
	while(!(I2C2->ISR & I2C_ISR_TXIS)) ;
	I2C2->TXDR = (command);

}

