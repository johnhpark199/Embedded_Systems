/*
 ******************************************************************************
* file      : keypad.c
* project   : EE 329 F'23 A3
* details   : LCD decrementing timer with custom keypad input
* authors   : John Park
*           :
* version   : 0.1
* date      : 2023-OCT-17
 * compiler  : STM32CubeIDE Version: 1.13.1 (2023)
* target    : NUCLEO-L4A6ZG
 ******************************************************************************
 * revisions :
* 2023-OCT-13 created.
******************************************************************************
* origins   :
* modified from CubeIDE auto-generated main.c  (c) 2023 STMicro.
* modified from EE329 Lab Manual Sample Code
******************************************************************************

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "keypad.h"


void Keypad_Config(void)  {


	// Initializing clock for GPIOD pins
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIODEN);

	// setting input pins to 00 (input mode) of MODER (PD4-7) (clearing and setting MODER)
	GPIOD->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2 | GPIO_MODER_MODE3 | GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7);

	// setting output pins to output mode (01) MODER (PD0-3) setting MODER
	GPIOD->MODER |= (GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0 | GPIO_MODER_MODE2_0 | GPIO_MODER_MODE3_0);

	// setting output type of output pins to (0) output push pull state
	GPIOD->OTYPER &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);

	// setting all pins for either pull up or down (00) no resistor (clearing)
	GPIOD->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 |GPIO_PUPDR_PUPD2 |
			GPIO_PUPDR_PUPD3 |GPIO_PUPDR_PUPD4 |GPIO_PUPDR_PUPD5 |GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7);

	// enabling pull down resistor (01)
	GPIOD->PUPDR |= (GPIO_PUPDR_PUPD4_1 | GPIO_PUPDR_PUPD5_1 | GPIO_PUPDR_PUPD6_1 | GPIO_PUPDR_PUPD7_1);

	// preset columns to high (PD0-3 are columns)
	//GPIOD->BSRR = (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	// setting output speed to (11) very high speed for all input and output bits (PD0-3)
	GPIOD->OSPEEDR |= ((3 << GPIO_OSPEEDR_OSPEED0_Pos) |
	        (3 << GPIO_OSPEEDR_OSPEED1_Pos) |
			(3 << GPIO_OSPEEDR_OSPEED2_Pos) |
			(3 << GPIO_OSPEEDR_OSPEED3_Pos) |
			(3 << GPIO_OSPEEDR_OSPEED4_Pos) |
			(3 << GPIO_OSPEEDR_OSPEED5_Pos) |
			(3 << GPIO_OSPEEDR_OSPEED6_Pos) |
			(3 << GPIO_OSPEEDR_OSPEED7_Pos) );

	// preset rows to low (PD4-7 are rows)
	GPIOD->BRR = (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);


}

// -----------------------------------------------------------------------------
int Keypad_IsAnyKeyPressed(void) {
// drive all COLUMNS HI; see if any ROWS are HI
// return true if a key is pressed, false if not
// currently no debounce here - just looking for a key twitch
	//Keypad_Config();
	COL_PORT->BSRR = COL_PINS;         	      // set all columns HI
	for ( uint16_t idx=0; idx<SETTLE; idx++ );
	// let it settle
	if ((ROW_PORT->IDR & ROW_PINS) != 0 )        // got a keypress!
		return( TRUE );
	else
		return( FALSE );                          // nope.
}


// -----------------------------------------------------------------------------
int Keypad_WhichKeyIsPressed(void) {
// detect and encode a pressed key at {row,col}
// assumes a previous call to Keypad_IsAnyKeyPressed() returned TRUE
// verifies the Keypad_IsAnyKeyPressed() result (no debounce here),
// determines which key is pressed and returns the encoded key ID

   int8_t iRow=0, iCol=0, iKey=0;  // keypad row & col index, key ID result
   int8_t bGotKey = 0;             // bool for keypress, 0 = no press

   COL_PORT->BSRR = COL_PINS;

   // set all columns HI
   for ( iRow = 0; iRow < NUM_ROWS; iRow++ ) {
	  // check all ROWS
	  // reading 32 bits from IDR then anding with 1 shifted iRow bits over
      if ( ROW_PORT->IDR & (BIT0 << (4+iRow)) ) {
    	 // keypress in iRow!!
         COL_PORT->BRR = ( COL_PINS );
         // set all cols LO
         for ( iCol = 0; iCol < NUM_COLS; iCol++ ) {
        	// 1 col at a time
            COL_PORT->BSRR = ( BIT0 << (iCol) );

            // set this col HI
            // reading 32 bits from IDR then anding with with 1 shifted iRow bit over
            if (ROW_PORT->IDR & (BIT0 << (4+iRow) )) {
               // keypress in iCol!!
               bGotKey = 1;
               break;
               // exit for iCol loop
            }
         }
         if ( bGotKey ) {
            break;
         }
      }
   }
   //	encode {iRow,iCol} into LED word : row 1-3 : numeric, ‘1’-’9’
   //	                                   row 4   : ‘*’=10, ‘0’=15, ‘#’=12
   //                                      no press: send NO_KEYPRESS
   if ( bGotKey ) {
      int iKey = ( iRow * NUM_COLS ) + iCol + 1;
      // handle numeric keys ...

      //if ( iKey == KEY_ZERO )                 //    works for ‘*’, ‘#’ too
    	  //iKey = CODE_ZERO;
      return( iKey );                         // return encoded keypress
   } else {
	   return( NO_KEYPRESS );                     // unable to verify keypress
   }
}

int Keypad_Debounce(int pressed) {

	int delay = 10000;
	int delay_counter = 0;

	// waiting to make sure key remains pressed
	for (delay_counter = 0; delay_counter < delay; delay_counter++);

	int new_key = Keypad_WhichKeyIsPressed();

	if (new_key == pressed) {
		return new_key;
	} else {
		return FALSE;
	}
}

void light_LEDs(int keypress) {

	switch(keypress) {
		// 1 pressed
		case 1:
			// last 3 LEDs off for a 1
			GPIOC->BRR = (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
			// first LED on
			GPIOC->BSRR = (GPIO_PIN_0);
			break;
		// 2 pressed
		case 2:
			// pins 0, 2, and 3 off
			GPIOC->BRR = (GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3);
			// pin 2 on
			GPIOC->BSRR = (GPIO_PIN_1);
			break;
		// 3 pressed
		case 3:
			// pins 2 and 3 off
			GPIOC->BRR = (GPIO_PIN_2 | GPIO_PIN_3);
			// pins 0 and 1 on
			GPIOC->BSRR = (GPIO_PIN_0 | GPIO_PIN_1);
			break;
		// A pressed (using 10 for 'A')
		case 4:
			// pin 2 off
			GPIOC->BRR = (GPIO_PIN_0 | GPIO_PIN_2);
			// pins 0, 1, and 3 on
			GPIOC->BSRR = (GPIO_PIN_1 | GPIO_PIN_3);
			break;
		// 4 pressed
		case 5:
			// pins 0, 1, and 3 off
			GPIOC->BRR = (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3);
			// pin 2 on
			GPIOC->BSRR = (GPIO_PIN_2);
			break;
		// 5 pressed
		case 6:
			// pins 1 and 3 off
			GPIOC->BRR = (GPIO_PIN_1 | GPIO_PIN_3);
			// pins 0 and 2 on
			GPIOC->BSRR = (GPIO_PIN_0 | GPIO_PIN_2);
			break;
		// 6 pressed
		case 7:
			// pins 0 and 3 off
			GPIOC->BRR = (GPIO_PIN_0 | GPIO_PIN_3);
			// pins 1 and 2 on
			GPIOC->BSRR = (GPIO_PIN_1 | GPIO_PIN_2);
			break;
		// B pressed (using 11 for B)
		case 8:
			// pin 0 and 1 off
			GPIOC->BRR = (GPIO_PIN_2);
			// pin 2 and 3 on
			GPIOC->BSRR = (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3);
			break;
		// 7 pressed
		case 9:
			// pin 3 off
			GPIOC->BRR = (GPIO_PIN_3);
			// pin 0, 1, and 2 on
			GPIOC->BSRR = (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);
			break;
		// 8 pressed
		case 10:
			// 0, 1, and 2 off
			GPIOC->BRR = (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);
			// 3 on
			GPIOC->BSRR = (GPIO_PIN_3);
			break;
		// 9 pressed
		case 11:
			// 1 and 2 off
			GPIOC->BRR = (GPIO_PIN_1 | GPIO_PIN_2);
			// 3 and 1 on
			GPIOC->BSRR = (GPIO_PIN_0 | GPIO_PIN_3);
			break;
		// C pressed using 12 for 'C'
		case 12:
			// 0 and 1 off
			GPIOC->BRR = (GPIO_PIN_0 | GPIO_PIN_1);
			// 2 and 3 on
			GPIOC->BSRR = (GPIO_PIN_2 | GPIO_PIN_3);
			break;
		// '*' pressed, displaying 14
		case 13:
			// 0 off
			GPIOC->BRR = (GPIO_PIN_0);
			// 1, 2, and 3 pressed
			GPIOC->BSRR = (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
			break;
		// 0 pressed displaying 0
		case 14:
			// all bits off
			GPIOC->BRR = (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
			// none on
			break;
		// '#' pressed, display 15
		case 15:
			// all pins on
			GPIOC->BSRR = (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
			break;
		// 'D' pressed, display 13
		case 16:
			// pin 1 off
			GPIOC->BRR = (GPIO_PIN_1);
			// pins 1, 2, and 3 on
			GPIOC->BSRR = (GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3);
			// cases for the rest of inputs
			break;
		default:
			GPIOC->BRR = (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
			break;
	}
}

char Keypad_to_char(int keypress_num) {
	switch(keypress_num) {
			// 1 pressed
			case 1:
				return '1';
				break;
			// 2 pressed
			case 2:
				return '2';
				break;
			// 3 pressed
			case 3:
				return '3';
				break;
			// A pressed (using 10 for 'A')
			case 4:
				return 'A';
				break;
			// 4 pressed
			case 5:
				return '4';
				break;
			// 5 pressed
			case 6:
				return '5';
				break;
			// 6 pressed
			case 7:
				return '6';
				break;
			// B pressed
			case 8:
				return 'B';
				break;
			// 7 pressed
			case 9:
				return '7';
				break;
			// 8 pressed
			case 10:
				return '8';
				break;
			// 9 pressed
			case 11:
				return '9';
				break;
			// C pressed
			case 12:
				return 'C';
				break;
			// '*' pressed
			case 13:
				return '*';
				break;
			// 0 pressed
			case 14:
				return '0';
				break;
			// '#' pressed
			case 15:
				return '#';
				break;
			// 'D' pressed
			case 16:
				return 'D';
				break;
			default:
				break;
		}
}

int Keypad_read() {
	 while ((Keypad_IsAnyKeyPressed() != TRUE)) {

	 }
	 int pressed = Keypad_WhichKeyIsPressed();
	 int new_key = Keypad_Debounce(pressed);
	    if (new_key == pressed) {
	    	return pressed;
	    }
}
