/*
 * smalltask.c
 *
 *  Created on: 12 sep. 2016
 *      Author: jorrit
 */
//==========================================================================================
// Includes
//==========================================================================================
#include <stdbool.h>
#include <inttypes.h>
#include <avr/io.h>
//#include <avr/iotn85.h>
#include "main.h"
#include "smalltask.h"
#include <avr/interrupt.h>
#include "hardware.h"
//==========================================================================================
// Definitions
//==========================================================================================

//==========================================================================================
// Types
//==========================================================================================

//==========================================================================================
// Global Values
//==========================================================================================
struct smalltask_value_t Smalltask_Value[SMALLTASK_TASK_COUNT];
uint16_t Smaltask_Uptime_ms;
uint32_t Smaltask_Uptime_s;
uint32_t Smalltask_Unixtime;

//==========================================================================================
// Internal Values
//==========================================================================================
//uint8_t smalltask_timerlast;
//uint8_t Smalltask_timernext;
//uint8_t smalltask_timermist;
uint8_t smalltask_timeroverflow;
//==========================================================================================
// Interrupts
//==========================================================================================

//==========================================================================================
// Internal Functions
//==========================================================================================
void Smalltask_run( uint8_t task )
{
	switch (task)
	{
#define TASK(a) case  TASK_##a: return a();
	SMALLTASK_TASK;
#undef TASK
	}
}
//==========================================================================================
// Global Functions
//==========================================================================================
ISR(TIM1_OVF_vect)
{
//	PORTB &= ~(1<<PORTB1);
//	PORTB ^= LED_OUTPUT;
	TCNT1 += 4;
	smalltask_timeroverflow++;
}


void Smalltask_init( void )
{
	smalltask_timeroverflow = 0;
//	smalltask_timerlast = 0;
//	Smalltask_timernext = smalltask_timerlast + 25;
//	Smaltask_Uptime_s = 0;
	//set counter
	TCCR1 = /*(1<<CTC1) |*/ (0<<CS13) | (1<<CS12) | (1<<CS11) | (0<<CS10);
	// F_CPU/Prescaler/1khz
	OCR1C = 250;
	//clear interupt flag
	TIFR |= (1<<TOV1);
	TIMSK |= (1<<TOIE1);
}


void Smalltask_dispatch( void )
{
	uint8_t i;


	if( smalltask_timeroverflow > 0)
	{
		smalltask_timeroverflow--;
		//Add internall timers
		if ( (++Smaltask_Uptime_ms) >= 1000 )
		{
			Smaltask_Uptime_s++;
			Smalltask_Unixtime++;
			Smaltask_Uptime_ms = 0;
		}

		//Do task
		for(i = 0; i < SMALLTASK_TASK_COUNT ; i++)
		{
			if ( Smalltask_Value[i].check )
			{
				if ( Smalltask_Value[i].use_counter )
				{
					if ( Smalltask_Value[i].time == 0 )
					{
						Smalltask_Value[i].check = false;
						Smalltask_run(i);
					}
					else
						 Smalltask_Value[i].time--;
				}
				else
				{
					if ( !Smalltask_Value[i].func() )
					{
						Smalltask_run(i);
						Smalltask_Value[i].check = false;
					}
				}
			}
		}
	}
}

void Smalltask_cancel_task(uint8_t task)
{
	Smalltask_Value[task].check = true;
}

void Smalltask_rerun(uint8_t task, uint16_t time)
{
	Smalltask_Value[task].use_counter = true;
	Smalltask_Value[task].time = time;
	Smalltask_Value[task].check = true;
}

void Smalltask_setfunc(uint8_t task, bool (*func)( void ))
{
	Smalltask_Value[task].use_counter = false;
	Smalltask_Value[task].func = func;
	Smalltask_Value[task].check = true;
}
