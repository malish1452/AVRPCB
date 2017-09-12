/*
 * MCP2515.h
 *
 * Created: 04.09.2017 10:55:05
 *  Author: Kirpichik
 */ 

#include "mcp2515_defs.h"
#include "avr/pgmspace.h"
#ifndef MCP2515_H_
#define MCP2515_H_
#include "SPI.h"



typedef struct
{
	uint16_t  id;
	uint8_t   rtr;
	uint8_t   length;
	uint8_t   data[8];
} CANMessage;

extern void mcp2515_init(uint8_t module);
extern void mcp2515_soft_reset(uint8_t module);
extern void mcp2515_init_send(uint8_t module,uint8_t buffer);
extern uint8_t mcp2515_read_status(uint8_t module);
extern void mcp2515_enable_module(uint8_t module);
extern void mcp2515_release_module(uint8_t module);



extern uint8_t mcp2515_can_send_message(CANMessage *p_message,uint8_t module);
extern void mcp2515_can_process_message(CANMessage *p_message);
extern void mcp2515_write_register ( uint8_t address, uint8_t data, uint8_t module );
extern uint8_t mcp2515_read_rx_status ( uint8_t module);
extern uint8_t mcp2515_read_register ( uint8_t address, uint8_t module );
extern void mcp2515_bit_modify ( uint8_t address, uint8_t mask, uint8_t data, uint8_t module );
extern uint8_t  mcp2515_can_read_message(CANMessage * p_message,uint8_t module);



// 125 KBPS 16MHZ  CNF1 03 CNF2 F0  CNF3 86
#define CAN125_16_CNF1 (0<<SJW1)|(0<<SJW0)|(0<<BRP5)|(0<<BRP4)|(0<<BRP3)|(0<<BRP2)|(1<<BRP1)|(1<<BRP0) //0000 0011
#define CAN125_16_CNF2 (1<<BTLMODE)|(1<<SAM)|(1<<PHSEG12)|(1<<PHSEG11)|(0<<PHSEG10)|(0<<PRSEG2)|(0<<PRSEG1)|(0<<PRSEG0) //1111 0000
#define CAN125_16_CNF3 (0<<SOF)|(0<<WAKFIL)|(1<<PHSEG22)|(1<<PHSEG21)|(0<<PHSEG20) // 1000 0110

// 500 KBPS 16MHZ  CNF1 00 CNF2 F0  CNF3 86
#define CAN500_16_CNF1 (0<<SJW1)|(0<<SJW0)|(0<<BRP5)|(0<<BRP4)|(0<<BRP3)|(0<<BRP2)|(0<<BRP1)|(0<<BRP0) //0000 0000
#define CAN500_16_CNF2 (1<<BTLMODE)|(1<<SAM)|(1<<PHSEG12)|(1<<PHSEG11)|(0<<PHSEG10)|(0<<PRSEG2)|(0<<PRSEG1)|(0<<PRSEG0) //1111 0000
#define CAN500_16_CNF3 (0<<SOF)|(0<<WAKFIL)|(1<<PHSEG22)|(1<<PHSEG21)|(0<<PHSEG20) // 1000 0110

#define DDR_CAN_CS		DDRB
#define PORT_CAN_CS		PORTB

#define CAN1_CS			4
#define CAN2_CS			5
#define CAN3_CS			6

#define CAN_RST_DDR		DDRE
#define CAN_RST_PORT	PORTE
#define CAN_RST			3




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



#endif /* MCP2515_H_ */