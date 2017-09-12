/*
 * UART.h
 *
 * Created: 04.09.2017 10:38:16
 *  Author: Kirpichik
 */ 

#define F_CPU 16000000L
#ifndef UART_H_
#define UART_H_

#define baudrate 115200L
#define bauddivider (F_CPU/(16*baudrate))  //somehow -1 brings errors on high speed;
#define HI(x) ((x)>>8)
#define LO(x) ((x)& 0xFF)

#include <avr/io.h>
#include <avr/interrupt.h>
#include "avrlibtypes.h"
#include "avrlibdefs.h"
#include "avr/pgmspace.h"
#include <avr/wdt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define UARTSendBufferSize 70
#define UARTReceiveBufferSize 30
#define UARTCommandLength  70


extern void UART_Send_Buffer_Empty ();
extern void UART_Send_Ended ();
extern void UART_Char_Received();
extern void UART0_Char_Received();
extern void UART_Add_Message(char *s);
extern void init_UART_Buffer();
extern void UART_Command_Processor();
extern void UART1_init();
extern void UART0_init();

struct Command_Stack {
	uint8_t ID [UARTReceiveBufferSize];
	char parameter[UARTReceiveBufferSize][UARTCommandLength];
	uint8_t  size;
};

typedef struct Command_Stack Command_Stack;

#endif /* UART_H_ */