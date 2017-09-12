/*
 * AD5206.c
 *
 * Created: 05.09.2017 16:12:18
 *  Author: Kirpichik
 */ 

#include "AD5206.h"

#include <avr/interrupt.h>
#include "SPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void RDAC_Enable()
{
	RDAC_PORT&=~(1<<RDAC_CS);
}

void RDAC_Disable()
{
	RDAC_PORT|=(1<<RDAC_CS);
}

void RDAC_init()
{
	uint8_t oldSREG=SREG;
	cli();
	RDAC_DDR|=(1<<RDAC_CS);
	RDAC_PORT|=(1<<RDAC_CS);
	RDAC_FullRes();	
	SREG=oldSREG;
}

void RDAC_SetRes(uint8_t chanel, long res)
{
	uint8_t oldSREG=SREG;
	cli();
	uint8_t value;
	value=(float)((res-25)*256/52300);
//	value=res;
	if (RDAC_IS_A==1)
	{
		value=255-value;
	}
	if (value>255)
	{
		value=255;
	}
	if (value<0)
	{
		value=0;
	}
	RDAC_Enable();
	spi_putc(chanel);
	spi_putc(value);
	RDAC_Disable();
	SREG=oldSREG;
}

void RDAC_FullRes()
{
	uint8_t oldSREG=SREG;
	cli();
	for (int i=0;i<6;i++)
	{
		RDAC_Enable();
		spi_putc(i);
		if (RDAC_IS_A==1)
		{
			spi_putc(0);
		}
		else
		{
			spi_putc(255);
		}
		RDAC_Disable();
	}
	SREG=oldSREG;
}

