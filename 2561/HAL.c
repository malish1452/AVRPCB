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

	//InitADC
	
	ADMUX = 0<<REFS1|1<<REFS0|0<<ADLAR|0<<MUX4|0<<MUX3|0<<MUX2|0<<MUX1|0<<MUX0;
	ADCSRB = (0<<ACME);
	ADCSRA = 1<<ADEN|1<<ADIE|0<<ADATE|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

