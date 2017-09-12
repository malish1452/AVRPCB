/*
 * AndroCar.h
 *
 * Created: 16.03.2017 8:41:56
 *  Author: malish
 */ 


#ifndef ANDROCAR_H_
#define ANDROCAR_H_

#include "HAL.h"

#define PIO0	0
#define PIO1	1
#define PIO2	2
#define PIO3	3
#define PIO4	4
#define PIO5	5
#define PIO9	6
#define PIO10	7
#define PIO11	8
#define PIO12	9
#define PIO13	10

#define LED0 			0
#define LED1			1
#define	LED2			2
#define	LED3			3

#define LED_PORT 		PORTA
#define LED_DDR			DDRA

#define DDR_SPI			DDRB
#define PORT_SPI		PORTB

#define ADC_MEDIA		7
#define ADC_CRUISE		6





#define SD_DDR			DDRE
#define SD_PORT			PORTE
#define SD_CS			2

#define GPS_DDR			DDRC
#define GPS_PORT		PORTC
#define GPS_EN			3

typedef void (*Process)(void);


#endif /* ANDROCAR_H_ */