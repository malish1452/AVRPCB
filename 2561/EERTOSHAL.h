/*
 * EERTOSHAL.h
 *
 * Created: 07.03.2017 20:01:38
 *  Author: malish
 */ 


#ifndef EERTOSHAL_H_
#define EERTOSHAL_H_
#include "HAL.h"

#define STATUS_REG 			SREG
#define Interrupt_Flag		SREG_I
#define Disable_Interrupt	cli();
#define Enable_Interrupt	sei();

//RTOS Config
#define RTOS_ISR  			TIMER2_COMPA_vect
#define	TaskQueueSize		20
#define MainTimerQueueSize	15

extern void RunRTOS (void);




#endif /* EERTOSHAL_H_ */