/*
 * CSR8645.c
 *
 * Created: 09.09.2017 16:25:17
 *  Author: Kirpichik
 */ 


#include "CSR8645.h"

struct PIO Pios[11]={{&PORTD,&DDRD,6},{&PORTD,&DDRD,7},{&PORTG,&DDRG,0},
					 {&PORTC,&DDRC,1},{&PORTC,&DDRC,0},{&PORTG,&DDRG,1},
				     {&PORTG,&DDRG,4},{&PORTD,&DDRD,5},{&PORTD,&DDRD,4},
					 {&PORTB,&DDRB,7},{&PORTG,&DDRG,3}};


void Init_All_PIO()
{
	for (int i=0;i<11;i++)
	{
		*Pios[i].Ddr|=(1<<Pios[i].pin);
	}
}

void Push_PIO(uint8_t pio)
{
	*Pios[pio].Port|=(1<<Pios[pio].pin);
}

void Release_PIO(uint8_t pio)
{
	*Pios[pio].Port&=~(1<<Pios[pio].pin);
}