/*
 * AD5206.h
 *
 * Created: 05.09.2017 16:12:33
 *  Author: Kirpichik
 */ 


#ifndef AD5206_H_
#define AD5206_H_
#include "avr/pgmspace.h"

#define RDAC_DDR		DDRG
#define RDAC_PORT		PORTG
#define RDAC_CS			5
#define RDAC_IS_A		1

extern void RDAC_init();
extern void RDAC_FullRes();
extern void RDAC_SetRes(uint8_t chanel, long res);

#endif /* AD5206_H_ */