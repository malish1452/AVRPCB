/*
 * HAL.h
 *
 * Created: 07.03.2017 19:58:20
 *  Author: malish
 */ 


#ifndef HAL_H_
#define HAL_H_


#include <avr/io.h>
#include <avr/interrupt.h>
#include "avrlibtypes.h"
#include "avrlibdefs.h"
#include "avr/pgmspace.h"
#include <avr/wdt.h>

//Clock Config
#define F_CPU 16000000L

//System Timer Config
#define Prescaler	  		64
#define	TimerDivider  		(F_CPU/Prescaler/1000)		// 1 mS


//USART Config
#define baudrate 115200L
#define bauddivider (F_CPU/(16*baudrate))  //somehow -1 brings errors on high speed;
#define HI(x) ((x)>>8)
#define LO(x) ((x)& 0xFF)



//PORT Defines




extern void InitAll(void);



#endif /* HAL_H_ */