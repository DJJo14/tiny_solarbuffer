/*
 * main.h
 *
 *  Created on: 20 jun. 2016
 *      Author: jorrit
 */

#ifndef MAIN_H_
#define MAIN_H_



//#define F_CPU 16000000UL

extern void Led_blink( void );
extern void button( void );
extern void adc_task( void );


enum{
	STATE_STARTINGUP,
	STATE_CHARGING,
	STATE_DISCHARGING,
	STATE_EMPTY,

};

enum{
	LED_OFF,
	LED_EMPLY,
	LED_UP,
	LED_DOWN,
	LED_FULL,
	LED_RUNNING,
};

#endif /* MAIN_H_ */
