/*
 * MCP2515.c
 *
 * Created: 04.09.2017 10:54:52
 *  Author: Kirpichik
 */ 

#include "MCP2515.h"
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
struct MCP2515 modules[3] ={{&PORT_CAN_CS,CAN1_CS,&CAN_RST_PORT,CAN_RST,CAN125_16_CNF1,CAN125_16_CNF2,CAN125_16_CNF3},
							{&PORT_CAN_CS,CAN2_CS,&CAN_RST_PORT,CAN_RST,CAN125_16_CNF1,CAN125_16_CNF2,CAN125_16_CNF3},
							{&PORT_CAN_CS,CAN3_CS,&CAN_RST_PORT,CAN_RST,CAN125_16_CNF1,CAN125_16_CNF2,CAN125_16_CNF3}};


void mcp2515_init(uint8_t module)
{
	DDR_CAN_CS |= (1<<modules[module].CS_pin);
	PORT_CAN_CS |= (1<<modules[module].CS_pin);
	
	CAN_RST_DDR|=(1<<CAN_RST);
	CAN_RST_PORT|=(1<<CAN_RST);
	
	uint8_t oldSREG=SREG;
	
	cli();
	
	mcp2515_soft_reset(module);
	
	mcp2515_write_register(CNF1,modules[module].cnf1_byte,module);
	mcp2515_write_register(CNF2,modules[module].cnf2_byte,module);
	mcp2515_write_register(CNF3,modules[module].cnf3_byte,module);
	
	mcp2515_write_register(CANINTE,(1<<RX1IE)|(1<<RX0IE),module);
	
	mcp2515_write_register(CANINTE,(1<<RX1IE)|(1<<RX0IE),module);
	mcp2515_write_register( RXB0CTRL,(0<<RXM1)|(1<<RXM0)|(0<<BUKT),module);
	mcp2515_write_register( RXB1CTRL,(0<<RXM1)|(1<<RXM0),module);
	
	
	mcp2515_write_register( RXM0SIDH,0,module) ;   //249 11111001 111

	mcp2515_write_register( RXM0SIDL,0,module) ; //224
	mcp2515_write_register( RXM0EID8,0,module) ;

	mcp2515_write_register( RXM0EID0,0,module) ;

	mcp2515_write_register( RXM1SIDH,0,module) ;  //63 00111111
	mcp2515_write_register( RXM1SIDL,0,module) ; //192 110
	mcp2515_write_register( RXM1EID8,0,module) ;
	mcp2515_write_register( RXM1EID0,0,module) ;
	
	mcp2515_write_register(RXF0SIDH,87,module);   //0101 0111
	mcp2515_write_register(RXF0SIDL,224,module); //111

	mcp2515_write_register(RXF1SIDH,119,module);   //0111 0111
	mcp2515_write_register(RXF1SIDL,64,module); //010
	
	mcp2515_write_register(RXF2SIDH,0,module);   //0000 0000
	mcp2515_write_register(RXF2SIDL,0,module); //000

	mcp2515_write_register(RXF3SIDH,18,module);   //0001 0010
	mcp2515_write_register(RXF3SIDL,0,module); //000
	
	mcp2515_write_register(BFPCTRL,0,module);
	mcp2515_write_register(TXRTSCTRL,0,module);
	
	mcp2515_bit_modify(CANCTRL,(1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0)|(1<<CLKEN)|(1<<CLKPRE1)|(1<<CLKPRE0),4,module);
	
	SREG=oldSREG;
}

void mcp2515_soft_reset(uint8_t module)
{
	*modules[module].CS_port &= ~ ( 1 << modules[module].CS_pin) ;
	spi_putc(SPI_RESET);
	*modules[module].CS_port |= ( 1 << modules[module].CS_pin ) ;
	
}

void mcp2515_init_send(uint8_t module,uint8_t buffer)
{
	*modules[module].CS_port &= ~ ( 1 << modules[module].CS_pin) ;
	spi_putc(SPI_RTS|(1<<(buffer+1)));
	*modules[module].CS_port |= ( 1 << modules[module].CS_pin ) ;
	
}

uint8_t mcp2515_read_status(uint8_t module)
{
	uint8_t oldSREG=SREG;
	cli();
	
	uint8_t status;
	*modules[module].CS_port &= ~ ( 1 << modules[module].CS_pin) ;
	spi_putc(SPI_READ_STATUS);
	status = spi_putc(0xff);
	
	*modules[module].CS_port |= ( 1 << modules[module].CS_pin ) ;
	return status;
	SREG=oldSREG;
}

void mcp2515_enable_module(uint8_t module)
{
	*modules[module].CS_port &= ~ ( 1 << modules[module].CS_pin) ;
}

void mcp2515_release_module(uint8_t module)
{
	*modules[module].CS_port |= ( 1 << modules[module].CS_pin ) ;
}

uint8_t mcp2515_can_send_message(CANMessage *p_message,uint8_t module)
{
	uint8_t oldSREG=SREG;
	cli();
	
	uint8_t status = mcp2515_read_status(module);
	
	uint8_t address;
	
	if (!(status&(1<<2)))
	{
		address=0x00;
	}
	else if (!(status&(1<<4)))
	{
		address=0x02;
	}
	else if (!(status&(1<<6)))
	{
		address=0x04;
	}
	else
	{
		return 0;
	}
	
	
	mcp2515_enable_module(module);
	
	spi_putc(SPI_WRITE_TX|address);
	
	spi_putc((uint8_t)( p_message->id>>3));
	spi_putc((uint8_t) ( p_message->id<<5));
	
	
	spi_putc(0x00);
	spi_putc(0x00);
	
	uint8_t length= p_message->length;
	
	if (length > 8)
	{
		length = 8;
	}

	if (p_message->rtr)
	{
		spi_putc((1<<RTR) | length);
	}
	else
	{
		spi_putc(length);
		
		for (uint8_t i=0;i<length;i++) {
			spi_putc(p_message->data[i]);
		}
	}
	
	mcp2515_release_module(module);
	
	asm volatile ("nop");
	
	mcp2515_enable_module(module);
	
	if (address==0)
	{
		spi_putc(SPI_RTS|1);
	}
	else
	{
		spi_putc(SPI_RTS|address);
	}
	
	mcp2515_release_module(module);
	
	return 1;
	
	SREG=oldSREG;
	
}

void mcp2515_can_process_message(CANMessage *p_message){
	//38A  кондиционер - прямая пересылка (906)
	//3BA кондиционер - прямая пересылка  (954)
	
	//201 Обороты и скорость (513)
	//400 Маршрутник		 (1024)
	
	//28F Экран - пересылка в зависимости от режима	(655)
	//290 Экран - пересылка в зависимости от режима (656)
	//291 Экран - пересылка в зависимости от режима (657)в
	
	//if ((p_message->id==0x38a)||(p_message->id==0x3ba))
	//{
		//can_send_message(p_message,0);
	//}
//
	//if (p_message->id==513)
	//{
		//current_rpm = p_message->data[0]*256+p_message->data[1];
		//current_speed = (double)(p_message->data[4]*256+p_message->data[5])/100;
	//}
	//
	//if (p_message->id==0x400)
	//{   if (p_message->data[2]!=0xff)
		//{
			//momental_fuel = (double)(p_message->data[2]*256+p_message->data[3])/10;
			//momental_fuel_resume=0;
		//}
		//else
		//{
			//momental_fuel=(double)1;
			//momental_fuel_resume=1;
		//}
		//average_fuel = (double)(p_message->data[5])/10;
		//distance_left = p_message->data[6]*256+p_message->data[7];
	//}
	//
	//if (p_message->id==0x28f)
	//{
		//car_monitor_setup[0]=p_message->data[0];
		//car_monitor_setup[1]=p_message->data[1];
		//car_monitor_setup[2]=p_message->data[2];
		//car_monitor_setup[3]=p_message->data[3];
		//car_monitor_setup[4]=p_message->data[4];
		//car_monitor_setup[5]=p_message->data[5];
		//car_monitor_setup[6]=p_message->data[6];
		//car_monitor_setup[7]=p_message->data[7];
	//}
	//if (p_message->id==0x290)
	//{
		//car_current_message[0]=p_message->data[1];
		//car_current_message[1]=p_message->data[2];
		//car_current_message[2]=p_message->data[3];
		//car_current_message[3]=p_message->data[4];
		//car_current_message[4]=p_message->data[5];
		//car_current_message[5]=p_message->data[6];
		//car_current_message[6]=p_message->data[7];
		//uint16_t new_sum =0;
		//for (int i=0;i<12;i++)
		//{
			//new_sum=new_sum+car_current_message[i];
		//}
		//if (new_sum!=car_previous_msg)
		//{
			//car_new_msg_flag=1;
			//car_previous_msg=new_sum;
		//}
	//}
	//if (p_message->id==0x291)
	//{
		//car_current_message[7]=p_message->data[1];
		//car_current_message[8]=p_message->data[2];
		//car_current_message[9]=p_message->data[3];
		//car_current_message[10]=p_message->data[4];
		//car_current_message[11]=p_message->data[5];
		//car_current_message[12]=p_message->data[6];
		//uint16_t new_sum =0;
		//for (int i=0;i<12;i++)
		//{
			//new_sum=new_sum+car_current_message[i];
		//}
		//if (new_sum!=car_previous_msg)
		//{
			//car_new_msg_flag=1;
			//car_previous_msg=new_sum;
		//}
	//}
}

void mcp2515_write_register ( uint8_t address, uint8_t data , uint8_t module)
{
	// / CS of the MCP2515 to low
	*modules[module].CS_port &= ~ ( 1 << modules[module].CS_pin) ;

	spi_putc ( SPI_WRITE ) ;
	spi_putc ( address ) ;
	spi_putc ( data ) ;

	// / CS Release line again
	*modules[module].CS_port |= ( 1 << modules[module].CS_pin ) ;
}

uint8_t mcp2515_read_rx_status ( uint8_t module)
{
	uint8_t oldSREG=SREG;
	cli();
	

	uint8_t data;

	// / CS of the MCP2515 to low
	*modules[module].CS_port &= ~ ( 1 << modules[module].CS_pin) ;

	spi_putc ( SPI_RX_STATUS ) ;
	data = spi_putc ( 0xff ) ;

	// The data is retransmitted again,
	// you need to evaluate only one of the two bytes.
	spi_putc ( 0xff ) ;

	// / CS Release line again
	*modules[module].CS_port |= ( 1 << modules[module].CS_pin ) ;
	SREG=oldSREG;
	return data;
}

uint8_t mcp2515_read_register ( uint8_t address, uint8_t module )
{
	uint8_t oldSREG=SREG;
	cli();
	
	uint8_t data;

	// / CS of the MCP2515 to low
	*modules[module].CS_port &= ~ ( 1 << modules[module].CS_pin) ;

	spi_putc(SPI_READ) ;
	spi_putc(address) ;

	data = spi_putc(0xff) ;

	// / CS Release line again
	*modules[module].CS_port |= ( 1 << modules[module].CS_pin ) ;
	
	SREG=oldSREG;
	
	return data;
}

void mcp2515_bit_modify ( uint8_t address, uint8_t mask, uint8_t data , uint8_t module )
{
	// / CS of the MCP2515 to low
	*modules[module].CS_port &= ~ ( 1 << modules[module].CS_pin) ;

	spi_putc ( SPI_BIT_MODIFY ) ;
	spi_putc ( address ) ;
	spi_putc ( mask ) ;
	spi_putc ( data ) ;

	// / CS Release line again
	*modules[module].CS_port |= ( 1 << modules[module].CS_pin ) ;
}

uint8_t mcp2515_can_read_message(CANMessage * p_message,uint8_t module)
{
	// Read status
	uint8_t status = mcp2515_read_rx_status (module) ;

	if ( status&(1<<6) )
	{
		// message in buffer 0

		mcp2515_enable_module(module); // CS Low
		spi_putc ( SPI_READ_RX ) ;
	}
	else if ( status&(1<<7) )
	{
		// message in buffer 1

		mcp2515_enable_module(module);  // CS Low
		spi_putc ( SPI_READ_RX | 0x04 ) ;
	}
	else {
		/* Error: No new message exists */
		return 0xff;
	}

	// Read the standard ID
	p_message-> id = ( uint16_t ) spi_putc ( 0xff ) << 3 ;
	p_message-> id |= ( uint16_t ) spi_putc ( 0xff ) >> 5 ;

	spi_putc ( 0xff ) ;
	spi_putc ( 0xff ) ;

	// Read the length
	uint8_t length =spi_putc(0xff);
	length&=0x0f;
	p_message-> length = length;

	// Read data
	for ( uint8_t i = 0 ; i <length; i ++ ) {
		p_message-> data [ i ] = spi_putc ( 0xff ) ;
	}

	mcp2515_release_module(module);

	if (status&(1<<3)) {
		p_message-> rtr = 1 ;
		} else {
		p_message-> rtr = 0 ;
	}

	// Cancel the interrupt flag
	if (status&(1<<6)) {
		mcp2515_bit_modify ( CANINTF, ( 1 << RX0IF ) , 0 ,module) ;
		} else {
		mcp2515_bit_modify ( CANINTF, ( 1 << RX1IF ) , 0 ,module) ;
	}

	return (status&0x07) ;
	

}