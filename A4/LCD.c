/*
 ******************************************************************************
* file      : LCD.c
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

#include "main.h"
#include "keypad.h"
#include "LCD.h"

// --------------------------------------------------- delay.c w/o #includes ---
// configure SysTick timer for use with delay_us().
// warning: breaks HAL_delay() by disabling interrupts for shorter delay timing.
void SysTick_Init(void) {
	SysTick->CTRL |= (SysTick_CTRL_ENABLE_Msk |     	// enable SysTick Timer
                      SysTick_CTRL_CLKSOURCE_Msk); 	// select CPU clock
	SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk);  	// disable interrupt
}

// delay in microseconds using SysTick timer to count CPU clock cycles
// do not call with 0 : error, maximum delay.
// careful calling with small nums : results in longer delays than specified:
//	   e.g. @4MHz, delay_us(1) = 10=15 us delay.
void delay_us(const uint32_t time_us) {
	// set the counts for the specified delay

	SysTick->LOAD = (uint32_t)((time_us * (SystemCoreClock / 1000000)) - 1);

	SysTick->VAL = 0;                                  	 // clear timer count

	SysTick->CTRL &= ~(SysTick_CTRL_COUNTFLAG_Msk);    	 // clear count flag

	while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)); // wait for flag
}

// write a character to the LCD

void LED_init(void) {
	// Initializing clock for GPIOC pins
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOCEN);

	// clearing output pins (PC0-3) for LEDS
	GPIOC->MODER   &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2 | GPIO_MODER_MODE3);

	// setting PC0-3 to output mode (01), so LEDS can be turned on and off
	GPIOC->MODER   |=  (GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0 | GPIO_MODER_MODE2_0 | GPIO_MODER_MODE3_0);

	// setting type to push pull (0)
	GPIOC->OTYPER  &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1 | GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);

	// setting output pins to (00) no pull up or pull down resistor
	GPIOC->PUPDR   &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);

	// setting output bits to (11) for PC0-3 (very fast mode)
	GPIOC->OSPEEDR |=  ((3 << GPIO_OSPEEDR_OSPEED0_Pos) |
		                (3 << GPIO_OSPEEDR_OSPEED1_Pos) |
					    (3 << GPIO_OSPEEDR_OSPEED2_Pos) |
					    (3 << GPIO_OSPEEDR_OSPEED3_Pos) );
	// setting LEDS to off initialing (0) using BRR
	GPIOC->BRR = (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
}

// USING GPIOE FOR LCD
// RS(pin 4) -> PE0
// R/W(pin 5) -> PE2 (PE1 unusable)
// E(pin 6) -> PE3
// DB0-3 (pins 7-10) -> None (4 bit nibble mode)
// DB4-7(pins 11-14) -> PE4-7
// set DB4 low to select 4 bit mode
// pin 16 grounded, pin 15 ~3.3 V

void LCD_config(void ) {

	// initializing clock for GPIOE
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOEEN);

	// clearing mode for all (00)
	GPIOE->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE2 | GPIO_MODER_MODE3 |
			GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7);

	// setting output data pins to output mode (01)
	GPIOE->MODER |= (GPIO_MODER_MODE0_0 | GPIO_MODER_MODE2_0 | GPIO_MODER_MODE3_0 |
			GPIO_MODER_MODE4_0 | GPIO_MODER_MODE5_0 | GPIO_MODER_MODE6_0 | GPIO_MODER_MODE7_0);

	// setting type to push pull (00)
	GPIOE->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3 | GPIO_OTYPER_OT4
			| GPIO_OTYPER_OT5 | GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7);

	// initializing pull up pull down to none (00)
	GPIOE->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3 | GPIO_PUPDR_PUPD4 |
			GPIO_PUPDR_PUPD5 | GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7);

	// setting first bit for medium speed pin output (
	GPIOE->OSPEEDR &= ~((GPIO_OSPEEDR_OSPEED0) |
            	(GPIO_OSPEEDR_OSPEED2) |
				(GPIO_OSPEEDR_OSPEED3) |
				(GPIO_OSPEEDR_OSPEED4) |
				(GPIO_OSPEEDR_OSPEED5) |
				(GPIO_OSPEEDR_OSPEED6) |
				(GPIO_OSPEEDR_OSPEED7));

	// setting second bit for medium speed output (
	GPIOE->OSPEEDR |= (GPIO_OSPEEDR_OSPEED0_0 | GPIO_OSPEEDR_OSPEED2_0 |
			GPIO_OSPEEDR_OSPEED3_0 | GPIO_OSPEEDR_OSPEED4_0 |
			GPIO_OSPEEDR_OSPEED5_0 | GPIO_OSPEEDR_OSPEED6_0 |
			GPIO_OSPEEDR_OSPEED7_0);

	GPIOE->BRR = (GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |
			GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
}

void LCD_init( void )  {   // RCC & GPIO config removed - leverage A1, A2 code

	// power-up wait 80 ms
    delay_us( 80000 );


    for ( int idx = 0; idx < 3; idx++ ) {  // wake up 1,2,3: DATA = 0011 XXXX
    	LCD_4b_command( 0x30 );             // HI 4b of 8b cmd, low nibble = X
    	delay_us( 200 );

    }
    // fcn set #4: 4b cmd set 4b mode - next 0x28:2-line
    LCD_4b_command( 0x28 );
    delay_us( 40 );

    // Display ON/OFF D[1] C[1] B[1]
    LCD_command(0x0F);
    // wait > 37 us, waiting 50 us
    delay_us(50);

    // Display clear
    LCD_command(0x01);
    // wait > 1.52 ms, waiting 5 ms
    delay_us(5000);

    // Entry Mode Set, I/D[1] (cursor moves right) S[0] (display not shifted)
    LCD_command(0x06);
    // wait 50 us, need wait > 37 us
    delay_us(50);

    // shifting cursor to right
    LCD_command(0x18);
    // wait > 37 us, waiting 50 us
    delay_us(50);

    // cursor return home
    LCD_command(0x02);


}

void LCD_pulse_ENA( void )  {
// ENAble line sends command on falling edge
// set to restore default then clear to trigger
   LCD_PORT->ODR   |= ( LCD_EN );         	// ENABLE = HI
   delay_us( 5 );                         // TDDR > 320 ns
   LCD_PORT->ODR   &= ~( LCD_EN );        // ENABLE = LOW
   delay_us( 5 );                         // low values flakey, see A3:p.1
}

void LCD_4b_command(uint8_t command ) {
// LCD command using high nibble only - used for 'wake-up' 0x30 commands
   LCD_PORT->ODR   &= ~( LCD_DATA_BITS ); 	// clear DATA bits
   LCD_PORT->ODR   |= ( command );   // DATA = command

   delay_us( 5 );
   LCD_pulse_ENA( );
}

void LCD_command( uint8_t command )  {
// send command to LCD in 4-bit instruction mode
// HIGH nibble then LOW nibble, timing sensitive
   LCD_PORT->ODR   &= ~( LCD_DATA_BITS );               // isolate cmd bits
   LCD_PORT->ODR   |= ( (command ) & LCD_DATA_BITS ); // HIGH shifted low
   delay_us( 5 );
   LCD_pulse_ENA( );                                    // latch HIGH NIBBLE

   LCD_PORT->ODR   &= ~( LCD_DATA_BITS );               // isolate cmd bits
   LCD_PORT->ODR   |= ( command << 4 & LCD_DATA_BITS );      // LOW nibble
   delay_us( 5 );
   LCD_pulse_ENA( );                                    // latch LOW NIBBLE
}

void LCD_write_char( uint8_t letter )  {
// calls LCD_command() w/char data; assumes all ctrl bits set LO in LCD_init()
   LCD_PORT->ODR   |= (LCD_RS);       // RS = HI for data to address
   delay_us( 5 );
   LCD_command( letter );             // character to print
   LCD_PORT->ODR   &= ~(LCD_RS);      // RS = LO
}

void LCD_write_str(char output[]) {
	// finding length of inputted string
	int length = strlen(output);
	for(int i = 0; i < length; i++) {
		LCD_write_char(output[i]);
	}
}


// writing initial LCD statement
void LCD_write_initial(void) {
	// creating strings
	char line_1[] = "EE 329 A3 TIMER";
	char line_2[] = "*=SET #=GO 00:00";

	// writing first line
	LCD_write_str(line_1);
	// move to second line
	LCD_command(0xC0);
	// wait 50 us, need to wait at least 37 us
	delay_us(50);
	// writing second line
	LCD_write_str(line_2);
}

int LCD_validate_tens_input(int tens_pressed) {

	// if key pressed is less than 3, no editing needs to be done
	if (tens_pressed < 4) {
		return tens_pressed;

	// return 0 if A or B is pressed return 0
	} else if (tens_pressed == 4 | tens_pressed == 8) {
		return 0;

	// if 4 or 5 is pressed, subtract one due to nature of 4x4 keyboard returns
	// (5 press returns 6)
	} else if (tens_pressed < 7) {
		return (tens_pressed - 1);

	// any other int inputs, default to 5
	} else if (tens_pressed < 12){
		return 5;

	// if 13 (*) is pressed return that value so it can be used to reset process
	} else if (tens_pressed == 13) {
		return 13;
	// return 0 if any other inputs are entered (C, *, 0, #, D)
	} else {
		return 0;
	}
}

char int_to_char(int input_int) {
	char out = input_int + '0';
	return out;
}

int LCD_validate_ones_input(int ones_pressed) {
	// if key pressed is 1-3, no editing needs to be done
	if (ones_pressed < 4) {
		return ones_pressed;

	// return 0 if A is pressed
	} else if (ones_pressed == 4) {
		return 0;

	// if 4-6 is pressed, subtract one due to nature of 4x4 keyboard returns
	// (5 press returns 6)
	} else if (ones_pressed < 8) {
		return (ones_pressed - 1);

	// B pressed, return 0
	} else if (ones_pressed == 8) {
		return 0;

	// if 7-9 pressed subtract 2 from pressed value to get true integer
	// if 7 is pressed, it is 9th key on keypad (9 would be ones_pressed value)
	} else if (ones_pressed < 12) {
		return (ones_pressed - 2);

	// if * is pressed return 13 so process resets
	} else if (ones_pressed == 13) {
		return 13;

	} else if (ones_pressed == 14) {
		return 0;
	// return 0 otherwise
	} else {
		return 9;
	}
}

int LCD_decrement(int time_arr[]) {

	// if the seconds place is 0, subtract from 10s
	if (time_arr[0] == 0) {

		// set seconds to 9
		time_arr[0] = 9;

		// if both seconds and tens of seconds are 0
		if(time_arr[1] == 0) {


			// set tens of seconds to 5
			time_arr[1] = 6;

			// if minutes is 0
			if(time_arr[2] == 0) {

				// remove one from tens of minutes
				time_arr[3] = time_arr[3] - 1;
				// set tens of minutes to 9
				time_arr[2] = 10;
			}
			// remove one from minutes
			time_arr[2] = time_arr[2] - 1;
		}
		// subtracting one from tens of seconds place
		time_arr[1] = time_arr[1] - 1;
	// decrement by one if the last digit isn't 0
	} else {
		time_arr[0] = time_arr[0] - 1;
	}

	// moving array to int so it can be returned

	int time = (time_arr[0]) + (time_arr[1] * 10) + (time_arr[2] * 100) + (time_arr[3] * 1000);
	return time;

}
