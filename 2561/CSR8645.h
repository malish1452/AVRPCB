/*
 * CSR8645.h
 *
 * Created: 09.09.2017 16:25:32
 *  Author: Kirpichik
 */ 


#ifndef CSR8645_H_
#define CSR8645_H_


#include "avr/pgmspace.h"

extern void Init_All_PIO();
extern void Push_PIO(uint8_t pio);
extern void Release_PIO(uint8_t pio);

struct PIO
{
	volatile uint8_t *Port;
	volatile uint8_t *Ddr;
	uint8_t pin;
};


#endif /* CSR8645_H_ */