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
#define F_CPU 8000000L

//System Timer Config
#define Prescaler	  		64
#define	TimerDivider  		(F_CPU/Prescaler/1000)		// 1 mS


//USART Config
#define baudrate 9600L
#define bauddivider (F_CPU/(16*baudrate)-1)
#define HI(x) ((x)>>8)
#define LO(x) ((x)& 0xFF)



//PORT Defines
#define LED1 		3
#define LED2		4
#define	LED3		5
#define	LED4		2

#define LED_PORT 	PORTE
#define LED_DDR		DDRE

#define DDR_SPI  DDRB
#define DDR_CS  DDRB

#define PORT_SPI  PORTB
#define PORT_CS  PORTB

#define P_SCK		1
#define P_MOSI		2
#define P_MISO		3
#define P_SS		0
#define P_CS1		4
#define P_CS2		5


extern void InitAll(void);



#endif /* HAL_H_ */