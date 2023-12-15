/*
 ******************************************************************************
* file      : ADC.c
* project   : EE 329 F'23 A8
* details   : ADC
* authors   : John Park
*           :
* version   : 0.1
* date      : 2023-NOV-27
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
#include "ADC.h"
#include "main.h"
#include "UART.h"

// prototyping so delay_us can be used within ADC.c
void SysTick_Init(void);
void delay_us(const uint32_t time_us);
void LPUART_print( const char* message );
void LPUART_ESC_print( const char* message );
void ADC_count_justify(int count);
char* getDigit(int number, int position);
char* int_to_char(int input_int);
void ADC_write_volts(int millivolts);

// printing home screen for displaying base ADC values
// for 80x25 terminal screen
void ADC_home_screen(void) {
	// clearing screen
	LPUART_ESC_print("[2J");
	// setting text color to white
	LPUART_ESC_print("[37m");
	// moving to start of display area for setting voltage/count readouts
	LPUART_ESC_print("[3;31H");
	// printing top line
	LPUART_print("ADC counts volts");
	// moving to next line
	LPUART_ESC_print("[4;31H");
	// print line 2
	LPUART_print("MIN  0000  0.000 V");
	// moving down to line 3
	LPUART_ESC_print("[5;31H");
	// line 3
	LPUART_print("MAX  0000  0.000 V");
	// moving to line 4
	LPUART_ESC_print("[6;31H");
	// printing line 4
	LPUART_print("AVG  0000  0.000 V");
	// printing current default values
	LPUART_ESC_print("[7;31H");
	LPUART_print("coil current = 0.000 A");
}


// finding min value of 20 samples from ADC
int ADC_find_min(uint16_t counts[]) {
	int min = counts[0];
	for (int i = 0; i < 20; i++) {
		if (counts[i] < min) {
			min = counts[i];
		}
	}
	return min;
}

// finding max value of 20 samples from ADC
int ADC_find_max(uint16_t counts[]) {
	int max = counts[0];
	for (int i = 0; i < 20; i++) {
		if (counts[i] > max) {
			max = counts[i];
		}
	}
	return max;
}

// function to find avg counts of 20 samples from ADC
int ADC_find_avg(uint16_t counts[]) {
	int total = counts[0];
	for (int i = 1; i < 20; i++) {
		total += counts[i];
	}
	int avg = total/20;
	return avg;
}

// printing count values
void ADC_print_counts(int min, int max, int avg) {
	// moving to min counts spot
    LPUART_ESC_print("[4;36H");
	// printing and justifying
    ADC_count_justify(min);
    //LPUART_print("1234");
    // max counts spot
    LPUART_ESC_print("[5;36H");
    // printing max
    ADC_count_justify(max);
    // max counts spot
    LPUART_ESC_print("[6;36H");
    // printing max
    ADC_count_justify(avg);

}

// right justifying and printing counts
void ADC_count_justify(int count) {
	// assume num is at least 1 digit long
	int length = 1;
	int temp = count;
	// finding length of
	while (temp /= 10) {
	    length++;
	}
	// printing spaces to right justify
	for (int i = 4; i > length; i--) {
		LPUART_print(" ");
	}

	// casting to chars and printing
	for (int j = length; j > 0; j--) {
		char* digit = getDigit(count, j - 1);
		LPUART_print(digit);
	}
}

// getting individual digit from number and returning it as a char
char* getDigit(int number, int position) {
    int divisor = 1;

    for (int i = 0; i < position; i++) {
        divisor *= 10;
    }
    int num_out = ((number / divisor) % 10);
    return int_to_char(num_out);
}

// brute force casting an int to a char* for printing to LPUART
char* int_to_char(int input_int) {

	switch(input_int) {
	// brute force solution to casting an int to a char
		case 0:
			return "0";
			break;
		case 1:
			return "1";
			break;
		case 2:
			return "2";
			break;
		case 3:
			return "3";
			break;
		case 4:
			return "4";
			break;
		case 5:
			return "5";
			break;
		case 6:
			return "6";
			break;
		case 7:
			return "7";
			break;
		case 8:
			return "8";
			break;
		case 9:
			return "9";
			break;
		default:
			return "0";
			break;
	}
}


void ADC_print_volts(int min, int max, int avg) {
	// casting to floats and voltage
	int min_mill_volts = (min * 1000 * 0.98 / 4095) * 3.3;
	int max_mill_volts = (max * 1000 * 0.98 / 4095) * 3.3;
	int avg_mill_volts = (avg * 1000 * 0.98 / 4095) * 3.3;
	// moving to volts min position
	LPUART_ESC_print("[4;42H");
	// printing volts
	ADC_write_volts(min_mill_volts);
	// moving to volts max position
	LPUART_ESC_print("[5;42H");
	// printing volts
	ADC_write_volts(max_mill_volts);
	// moving to volts avg position
	LPUART_ESC_print("[6;42H");
	// writing avg milli volts
	ADC_write_volts(avg_mill_volts);
}

void ADC_write_volts(int millivolts) {

	int length = 1;
	int temp = millivolts;
	// finding length of
	while (temp /= 10) {
	    length++;
	}
	// printing spaces to right justify
	for (int i = 4; i > length; i--) {
		// printing . if needed
		if (i == 3) {
			LPUART_print(".");
		}
		LPUART_print("0");
	}

	// casting to chars and printing
	for (int j = length; j > 0; j--) {
		if (j == 3) {
			LPUART_print(".");
		}
		char* digit = getDigit(millivolts, j - 1);
		LPUART_print(digit);
	}
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


void relay_init(void) {
	// Initializing clock for GPIOD pins
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN);

	// setting input pins to 00 (input mode) of MODER (PD4-7) (clearing and setting MODER)
	GPIOB->MODER &= ~(GPIO_MODER_MODE11);

	// setting output pins to output mode (01) MODER (PD0-3) setting MODER
	GPIOB->MODER |= (GPIO_MODER_MODE11_0);

	// setting output type of output pins to (0) output push pull state
	GPIOB->OTYPER &= ~(GPIO_PUPDR_PUPD11);

	// setting all pins for either pull up or down (00) no resistor (clearing)
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD11);

	// setting output speed to (11) very high speed for all input and output bits (PD0-3)
	GPIOB->OSPEEDR |= (3 << GPIO_OSPEEDR_OSPEED11_Pos);

	// preset rows to low (PD4-7 are rows)
	GPIOB->BRR = (GPIO_PIN_11);
}

// calculating current across coil from the measured voltage of the ADC
int ADC_get_current(int avg_counts) {
	int milliamps = (avg_counts * 1000 / 4095) * 3.3 * 1.013 / 100;
	return milliamps;
}

// printing current output
void ADC_print_current(int milliamps) {
	// moving to position of milliamp printing
	LPUART_ESC_print("[7;46H");
	// printing milliamps
	ADC_write_volts(milliamps);
}
