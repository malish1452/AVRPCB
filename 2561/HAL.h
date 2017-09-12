/*
 * HAL.h
 *
 * Created: 07.03.2017 19:58:20
 *  Author: malish
 */ 

#define F_CPU 16000000L

#ifndef HAL_H_
#define HAL_H_


#include <avr/io.h>
#include <avr/interrupt.h>
#include "avrlibtypes.h"
#include "avrlibdefs.h"
#include "avr/pgmspace.h"
#include <avr/wdt.h>

//Clock Config
#define HI(x) ((x)>>8)
#define LO(x) ((x)& 0xFF)

//System Timer Config
#define Prescaler	  		64
#define	TimerDivider  		(F_CPU/Prescaler/1000)		// 1 mS


//USART Config




//PORT Defines




extern void InitAll(void);



#endif /* HAL_H_ */