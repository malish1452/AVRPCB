/*
 * SPI.c
 *
 * Created: 31.08.2017 12:52:57
 *  Author: Kirpichik
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>


#include "SPI.h"
#include "AndroCar.h"

void init_SPI()
{
	DDR_SPI |= (1<<SPI_SCK)|(1<<SPI_MOSI)|(1<<SPI_SS);
	PORT_SPI &= ~((1<<SPI_SCK)|(1<<SPI_MOSI));
	SPCR = (1<<SPE)|(1<<MSTR)|(0<<SPR0)|(0<<SPR1);
	SPSR = (1<<SPI2X);
}

uint8_t spi_putc (uint8_t data)
{
	SPDR=data;
	while (!(SPSR&(1<<SPIF)));
	return SPDR;
}