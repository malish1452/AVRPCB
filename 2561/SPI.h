/*
 * SPI.h
 *
 * Created: 31.08.2017 12:53:09
 *  Author: Kirpichik
 */ 


#ifndef SPI_H_
#define SPI_H_

#define SPI_SS			0
#define SPI_SCK			1
#define SPI_MOSI		2
#define SPI_MISO		3

extern void init_SPI();

extern uint8_t spi_putc (uint8_t data);


#endif /* SPI_H_ */