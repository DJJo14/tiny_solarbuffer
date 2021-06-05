/*
 * main.c
 *
 *  Created on: 20 jun. 2016
 *      Author: jorrit
 */
//==========================================================================================
// Includes
//==========================================================================================
//#undef __AVR_ATmega16__
//#define __AVR_ATtiny85__

#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/iotn85.h>
#include <inttypes.h>
#include <stdbool.h>
//#include <util/delay.h>
#include <avr/eeprom.h>
#include "main.h"
#include "smalltask.h"
#include "hardware.h"

//==========================================================================================
// Definitions
//==========================================================================================
//#define F_CPU 8000000UL
#define TIME_LONG_HOLD_BUTTON 5000
#define TIME_HOLD_BUTTON 250
#define TIME_BUTTON 50
#define BAUD   9600

#define VOLTAGE_FULL		1500
#define VOLTAGE_CHARGING	1200
#define VOLTAGE_EMPTY		1000
//#define VOLTAGE_DOWN
//:~/source/micronucleus/commandline$ sudo ./micronucleus --run ~/workspace/tiny_carnaval/Debug/tiny_carnaval.hex
//==========================================================================================
// Prototypes
//==========================================================================================

//==========================================================================================
// Global Values
//==========================================================================================
uint8_t state = STATE_STARTINGUP;
uint8_t led_state = LED_OFF;
//uint8_t EEMEM EE_mode = 1;
//bool startup = true;
//==========================================================================================
// Interrupts
//==========================================================================================


//==========================================================================================
// Internal Functions
//==========================================================================================
static void init_adc( void )
{
	ADMUX = (0<<REFS2) | (1<<REFS1) | (0<<REFS0) | (0<<ADLAR) | 3*(1<<MUX0);
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (0<<ADPS0);
	DIDR0 = (1<<ADC3D);
}

static void init( void )
{
	DDRB = SYSTEM_DDR;
	PORTB = 0;//(1<<DDB5);
	//do this to lower the Fcpu
//	CLKPR = (1<<CLKPCE);
//	CLKPR = (1<<CLKPS1) | (1<<CLKPS0);

//	PLLCSR &= (1<<PCKE);
//	mode = eeprom_read_byte( &EE_mode );
//	if( mode > 0xF0 )
//	{
//		mode = 1;
//	}
	Smalltask_init();
	init_adc();
	sei();	// __enable_interrupt();


}


static void set_led_state( uint8_t state )
{
	if (led_state == LED_OFF)
		led_state = state;
}

//==========================================================================================
// Main
//==========================================================================================
int main(void)
{
	init();

//	if(startup && !(PINB & (1<<DDB5))  )
//	Smalltask_rerun(TASK_button, TIME_BUTTON);
	Smalltask_rerun(TASK_Led_blink, 1);
	Smalltask_rerun(TASK_adc_task, 100);
	while(1)
	{

		Smalltask_dispatch();

	}
}


void Led_blink( void )
{
//	PORTB ^= LED_OUTPUT;
	static uint8_t count = 0;
	switch (led_state)
	{
	case LED_EMPLY:
		count = 1;
		led_state = LED_RUNNING;
		break;
	case LED_UP:
		count = 3;
		led_state = LED_RUNNING;
		break;
	case LED_DOWN:
		count = 5;
		led_state = LED_RUNNING;
		break;
	case LED_FULL:
		count = 7;
		led_state = LED_RUNNING;
		break;
	}

	if (count == 0 && led_state == LED_RUNNING )
	{
		led_state = LED_OFF;
		PORTB &= ~LED_OUTPUT;
		Smalltask_rerun(TASK_Led_blink, 2000);
		return;
	}
	if( led_state == LED_RUNNING )
	{
		count -= 1;
		PORTB ^= LED_OUTPUT;
	}
	Smalltask_rerun(TASK_Led_blink, 500);
}

void button( void )
{
//	static uint8_t i = 0;
//
//	if ( !(PINB & (1<<DDB5) ) && i == 0)
//	{
//		i++;
//	}
//	else if ( !(PINB & (1<<DDB5)))
//	{
//		if ( i < 250)
//			i++;
//	}
//	else if ( (PINB & (1<<DDB5)) && i > (TIME_HOLD_BUTTON/TIME_BUTTON) )
//	{
//		if ( i < TIME_LONG_HOLD_BUTTON/TIME_BUTTON)
//		{
//			Smalltask_cancel_task( mode );
//			if ( mode < MODE_COUNT-1 )
//				mode++;
//			else
//				mode = 1;
//			PORTB &= ~0x0F;
//			PORTB |= mode & 0x0f;
//		}
//		else
//		{
//			PORTB |= 0x0f;
//			eeprom_write_byte( &EE_mode , mode);
//		}
//		i = (TIME_HOLD_BUTTON/TIME_BUTTON)-1;
//	}
//	else if (i)
//	{
//		i--;
//		if ( i == 0)
//		{
//			PORTB &= ~0x0F;
//			Smalltask_rerun(mode, 50);
//		}
//
//	}
	Smalltask_rerun(TASK_button, TIME_BUTTON);
}

void adc_task( void )
{
//	uint16_t cap_voltage = 0;
	uint16_t temp;
	ADCSRA |= (1<<ADSC);
	while (ADCSRA & (1<<ADSC));
	temp = (uint16_t)((uint32_t) ADCW * 1595 / 1024);

//	if (cap_voltage > 1000)
//		PORTB ^= LED_OUTPUT;
	if (state == STATE_STARTINGUP)
	{
		if (temp < VOLTAGE_EMPTY)
		{
			state = STATE_EMPTY;
			set_led_state(LED_EMPLY);
		}
		else if ( temp > VOLTAGE_FULL )
		{
			state = STATE_DISCHARGING;
			set_led_state(LED_FULL);
			PORTB |= USB_OUTPUT;
		}
		else
		{
			state = STATE_DISCHARGING;
			set_led_state(LED_UP);
		}
//		cap_voltage = temp;
	}
	else if (state == STATE_DISCHARGING)
	{
		if (temp < VOLTAGE_CHARGING)
		{
			state = STATE_CHARGING;
			set_led_state(LED_UP);
			PORTB &= ~USB_OUTPUT;
		}
		Smalltask_rerun(TASK_adc_task, 250);
		return;
	}
	else if (state == STATE_CHARGING)
	{
		if (temp > VOLTAGE_FULL)
		{
			state = STATE_DISCHARGING;
			set_led_state(LED_DOWN);
			PORTB |= USB_OUTPUT;
		}
		else if (temp < VOLTAGE_EMPTY)
		{
			state = STATE_EMPTY;
			set_led_state(LED_EMPLY);
		}
		Smalltask_rerun(TASK_adc_task, 1000);
		return;
	}
	else if (state == STATE_EMPTY)
	{
		if (temp > VOLTAGE_CHARGING)
		{
			state = STATE_DISCHARGING;
			set_led_state(LED_UP);
		}
		Smalltask_rerun(TASK_adc_task, 2000);
		return;
	}

	Smalltask_rerun(TASK_adc_task, 1000);
}
