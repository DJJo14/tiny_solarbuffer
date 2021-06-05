/*
 * smalltask.h
 *
 *  Created on: 12 sep. 2016
 *      Author: jorrit
 */

#ifndef SMALLTASK_H_
#define SMALLTASK_H_
//==========================================================================================
// Definitions
//==========================================================================================
#define SMALLTASK_TASK \
TASK( button ) \
TASK( Led_blink ) \
TASK( adc_task )





//==========================================================================================
// Types
//==========================================================================================
struct smalltask_value_t
{
	bool check;
	bool use_counter;
	union
	{
		uint16_t time;
		bool (*func)( void );
	};
};

enum
{
#define TASK( a ) TASK_##a,
	SMALLTASK_TASK
#undef TASK
	SMALLTASK_TASK_COUNT
};
//==========================================================================================
// Values
//==========================================================================================
extern struct smalltask_value_t Smalltask_Value[SMALLTASK_TASK_COUNT];
extern uint16_t Smaltask_Uptime_ms;
extern uint32_t Smaltask_Uptime_s;
extern uint32_t Smalltask_Unixtime;

extern uint8_t smalltask_timermist;
extern uint8_t Smalltask_timernext;
//==========================================================================================
// Functions
//==========================================================================================
extern void Smalltask_init( void );
extern void Smalltask_dispatch( void );
extern void Smalltask_cancel_task(uint8_t task);
extern void Smalltask_rerun(uint8_t task, uint16_t time);
extern void Smalltask_setfunc(uint8_t task, bool (*func)( void ));


#endif /* SMALLTASK_H_ */
