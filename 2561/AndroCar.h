/*
 * AndroCar.h
 *
 * Created: 16.03.2017 8:41:56
 *  Author: malish
 */ 


#ifndef ANDROCAR_H_
#define ANDROCAR_H_

#include "HAL.h"
#include "mcp2515_defs.h"



#define UARTSendBufferSize 70
#define UARTReceiveBufferSize 30
#define UARTCommandLength  70


// 125 KBPS 16MHZ  CNF1 03 CNF2 F0  CNF3 86
#define CAN125_16_CNF1 (0<<SJW1)|(0<<SJW0)|(0<<BRP5)|(0<<BRP4)|(0<<BRP3)|(0<<BRP2)|(1<<BRP1)|(1<<BRP0) //0000 0011
#define CAN125_16_CNF2 (1<<BTLMODE)|(1<<SAM)|(1<<PHSEG12)|(1<<PHSEG11)|(0<<PHSEG10)|(0<<PRSEG2)|(0<<PRSEG1)|(0<<PRSEG0) //1111 0000
#define CAN125_16_CNF3 (0<<SOF)|(0<<WAKFIL)|(1<<PHSEG22)|(1<<PHSEG21)|(0<<PHSEG20) // 1000 0110

// 500 KBPS 16MHZ  CNF1 00 CNF2 F0  CNF3 86
#define CAN500_16_CNF1 (0<<SJW1)|(0<<SJW0)|(0<<BRP5)|(0<<BRP4)|(0<<BRP3)|(0<<BRP2)|(0<<BRP1)|(0<<BRP0) //0000 0000
#define CAN500_16_CNF2 (1<<BTLMODE)|(1<<SAM)|(1<<PHSEG12)|(1<<PHSEG11)|(0<<PHSEG10)|(0<<PRSEG2)|(0<<PRSEG1)|(0<<PRSEG0) //1111 0000
#define CAN500_16_CNF3 (0<<SOF)|(0<<WAKFIL)|(1<<PHSEG22)|(1<<PHSEG21)|(0<<PHSEG20) // 1000 0110

#define LED1 		0
#define LED2		1
#define	LED3		0
#define	LED4		1

#define LED_PORT0 	PORTG
#define LED_DDR0		DDRG

#define LED_PORT1 	PORTC
#define LED_DDR1		DDRC


#define DDR_SPI  DDRB
#define PORT_SPI  PORTB

#define DDR_CS  DDRB
#define PORT_CS  PORTB

#define P_SCK		1
#define P_MOSI		2
#define P_MISO		3
#define P_SS		0
#define P_CS1		6
#define P_CS2		5
#define P_CS3		4

#define PORT_RESET PORTB
#define P_RESET 7




struct MCP2515 
{

 volatile uint8_t *CS_port;
 uint8_t CS_pin;
 
 volatile uint8_t *Reset_port;
 uint8_t Reset_pin;
 
 uint8_t cnf1_byte;
 uint8_t cnf2_byte;
 uint8_t cnf3_byte;

};

typedef struct
{
	uint16_t  id;
	uint8_t   rtr;
	uint8_t   length;
	uint8_t   data[8];
} CANMessage;


typedef void (*Process)(void);

struct Command_Stack {
	uint8_t ID [UARTReceiveBufferSize];
	char parameter[UARTReceiveBufferSize][UARTCommandLength];
	uint8_t  size;
	};
	
typedef struct Command_Stack Command_Stack;



extern void UART_Send_Buffer_Empty ();
extern void UART_Send_Ended ();
extern void UART_Char_Received();
extern void UART_Add_Message(char *s);
extern void init_UART_Buffer();
extern void UART_Command_Processor();

extern uint8_t spi_putc (uint8_t data);
extern void SPI_Send_Ready();
extern void init_SPI();


extern uint8_t mcp2515_read_status(uint8_t module);
extern void mcp2515_enable_module(uint8_t module);
extern void mcp2515_release_module(uint8_t module);


extern void mcp2515_init_send(uint8_t module,uint8_t buffer);
extern uint8_t can_send_message(CANMessage *p_message,uint8_t module);
extern void MCP2515_init(uint8_t module);
extern void mcp2515_soft_reset(uint8_t module);
extern void mcp2515_write_register ( uint8_t address, uint8_t data, uint8_t module );
extern uint8_t mcp2515_read_register ( uint8_t address, uint8_t module );
extern void mcp2515_bit_modify ( uint8_t address, uint8_t mask, uint8_t data, uint8_t module );
extern uint8_t mcp2515_read_rx_status ( uint8_t module);

extern void send_message_monitor(uint8_t module);
extern uint8_t can_read_message(CANMessage * p_message,uint8_t module);
extern void change_resend(uint8_t new_status);
#endif /* ANDROCAR_H_ */