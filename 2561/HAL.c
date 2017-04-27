/*
 * HAL.c
 *
 * Created: 07.03.2017 23:57:16
 *  Author: malish
 */ 

#include "HAL.h"
#include "AndroCar.h"



inline void InitAll(void)
{

	//InitUSART
	
	UBRR1H = HI(bauddivider);
	UBRR1L = LO(bauddivider);
	UCSR1A = 0;
	UCSR1B = 1<<RXCIE1|0<<TXCIE1|0<<UDRIE1|1<<TXEN1|1<<RXEN1|0<<UCSZ12;
	UCSR1C=0b10000110;
	
	//InitPort
	LED_DDR0 |= 1<<LED1|1<<LED2;
	LED_DDR1 |= 1<<LED3|1<<LED4;
	
	//InitInt
	

	//InitADC
	
	ADMUX = 0<<REFS0|1<<REFS1|1<<ADLAR|1<<MUX4|1<<MUX3|1<<MUX2|1<<MUX1|0<<MUX0;
//	ADCSRB = 0<<MUX5;
	ADCSRA = 1<<ADEN|1<<ADIE|0<<ADATE;
}

