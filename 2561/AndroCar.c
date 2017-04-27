/*
 * AndroCar.c
 *
 * Created: 16.03.2017 8:41:40
 *  Author: malish
 */ 

#include "AndroCar.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>


struct MCP2515 modules[3] ={{&PORT_CS,P_CS1,&PORT_RESET,P_RESET,CAN125_16_CNF1,CAN125_16_CNF2,CAN125_16_CNF3},
							{&PORT_CS,P_CS2,&PORT_RESET,P_RESET,CAN125_16_CNF1,CAN125_16_CNF2,CAN125_16_CNF3},
							{&PORT_CS,P_CS3,&PORT_RESET,P_RESET,CAN125_16_CNF1,CAN125_16_CNF2,CAN125_16_CNF3}};

static char* UART1_Send_Buffer[UARTSendBufferSize];

static uint8_t UART1_string_position;

static uint8_t New_command_parameter_length;

static uint8_t New_command_receive_status;   //0 - Receiving command ID; 1-Receiving command parameter;

//Command format is IDParameter;

char New_command[UARTCommandLength];

uint8_t New_command_id;

//static UARTCommand* UART1_Recieve_Buffer[UARTReceiveBufferSize];

//UARTCommand *Next_UART_Command, *Last_UART_Command;

Command_Stack UART_Command_Stack;




uint8_t MON_125_AVG[12]={65,118,103,58,32,50,48,46,48,32,76,32};		// 4-8 AAA.A  
uint8_t MON_125_Momental[12]={77,111,109,58,32,50,48,46,48,32,76,32};	//4-8 AAA.A  
uint8_t MON_125_Remain[12]={82,101,109,58,32,32,54,48,48,32,75,109};	//5-8 AAAA
uint8_t MON_125_Speed[12]={32,49,50,48,46,48,48,32,75,109,47,104};		//1-6 AAA.AA
uint8_t MON_125_Message[12]={84,69,83,84,32,77,69,83,83,65,71,69};

uint8_t Resend_status; 
/*	
	[0-2] - Message;
	[3] - Phone symbol;
	[4]	- Repeat AC;
	
	Message
	
	[000]   -	Repeat from CAR
	[001]	-	Show average fuel
	[010]	-	Show momental fuel
	[011]	-	Show remain fuel
	[100]	-	Show momental speed
	[101]	-	Message from Phone
	
	Phone symbol
	
	[0]		-	Disabled
	[1]		-	Enabled
	
	Repeat AC
	
	[0]		-	Repeat CAR
	[1]		-	Inner AC
*/

#pragma region Работа с UART

 //Функции работы с UART

void UART_Send_Buffer_Empty ()
{
		if (UART1_Send_Buffer[0][UART1_string_position]!=0)
		{
			UDR1=UART1_Send_Buffer[0][UART1_string_position];
			UART1_string_position++;
			UCSR1B |= 1<<TXCIE1;
		}
		else
		{  
			UDR1='\n';
			UCSR1B &= ~(1<<UDRIE1);
		}
};


void UART_Send_Ended()
{
	uint8_t i=1;
	
	UCSR1B &= ~(1<<TXCIE1);
	
	free(UART1_Send_Buffer[0]);
	
	while (!(UART1_Send_Buffer[i]==0))
	{
		UART1_Send_Buffer[i-1]=UART1_Send_Buffer[i];
		i=i+1;
	}
	
	UART1_Send_Buffer[i-1]=0;
	
	if (i>1)
	{
		UART1_string_position=0;
		UCSR1B |= 1<<UDRIE1;
	}
}

void UART_Char_Received()
{
		char received_char;
		
		received_char=UDR1;
		
		if (received_char=='\r') return;
		
		if (New_command_receive_status==0)
		{
			
			New_command_parameter_length=0;
			New_command_id=received_char;
			New_command[0]='\0';
			New_command_receive_status=1;
			
		}
		else
		{
		
			if (!(received_char=='\n')&&(New_command_parameter_length<(UARTCommandLength-1)))
			{
				
				New_command[New_command_parameter_length]=received_char;
				New_command_parameter_length++;
				New_command[New_command_parameter_length]='\0';
				
			}
			else
			{
				New_command_receive_status=0;
				if (!(UART_Command_Stack.size==UARTReceiveBufferSize))
				{
					UART_Command_Stack.ID[UART_Command_Stack.size]=New_command_id;
					strcpy(UART_Command_Stack.parameter[UART_Command_Stack.size],New_command);
					UART_Command_Stack.size++;
				}
			}	
		}
						
		
}

void init_UART_Buffer()
{
	
	//Инициализация буфера отправки
	for (uint8_t i=0;i<UARTSendBufferSize;i++)
	{
		UART1_Send_Buffer[i]=0;
	}
	
	//Инициализация буфера новой команды
	
	UART_Command_Stack.size=0;
	
	
}



void UART_Add_Message(char *s)
{
	uint8_t oldSREG=SREG;
	
	cli();
	
	char* copy;
	copy =malloc(sizeof(*copy) * (strlen(s)+1));
	strcpy(copy,s);
	
	uint8_t k=0;
	
	while (!(UART1_Send_Buffer[k]==0))
	{
		k=k+1;
	}
	
	if (k<UARTSendBufferSize)
	{
		UART1_Send_Buffer[k]=copy;
	}

	
	if (!(UCSR1B&(1<<UDRIE1)))
	{
		UCSR1B |= 1<<UDRIE1;
		UART1_string_position=0;
	}
	SREG=oldSREG;
}


void UART_Command_Processor()
{
	
	
	if (UART_Command_Stack.size>0)
	{
		    uint8_t oldSREG=SREG;
		    cli();
		    
		//Ниже должна быть обработка команд согласно идентификатору
		char line[50];
		sprintf(line,"ID: %d, Par: %s",UART_Command_Stack.ID[0],UART_Command_Stack.parameter[0]);
		UART_Add_Message(line);
		
		
		
		
		
		//Уменьшение размера очереди
		UART_Command_Stack.size--;
			
		
		//Смещение очереди
		
		for (uint8_t i=0; i<UART_Command_Stack.size;i++)
		{
			UART_Command_Stack.ID[i]=UART_Command_Stack.ID[i+1];
			strcpy(UART_Command_Stack.parameter[i],UART_Command_Stack.parameter[i+1]);
		}
		SREG=oldSREG;
	}
	
	
}

#pragma endregion Работа с UART

#pragma region Работа с SPI


void init_SPI()
{
		DDR_SPI |= (1<<P_SCK)|(1<<P_MOSI)|(1<<P_SS);
		PORT_SPI &= ~((1<<P_SCK)|(1<<P_MOSI));

		DDR_CS |= (1<<P_CS1)|(1<<P_CS2)|(1<<P_CS3);
		PORT_CS |= (1<<P_CS1)|(1<<P_CS2)|(1<<P_CS3);
		
		DDRE &=(0<<4)|(0<<5)|(0<<6);
		PORTE|=(1<<4)|(1<<5)|(1<<6);
			
		DDRB|=(1<<P_RESET);
		PORT_RESET|=(1<<P_RESET);

		
		SPCR = (1<<SPE)|(1<<MSTR)|(0<<SPR0)|(0<<SPR1);
		SPSR = (1<<SPI2X);
		
		
		
}

uint8_t spi_putc (uint8_t data)
{
	SPDR=data;
	while (!(SPSR&(1<<SPIF)));
	return SPDR;
}


#pragma endregion Работа с SPI

#pragma region MCP2515
//Функции работы с MCP 2515

void MCP2515_init(uint8_t module)
{
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
	
		
	mcp2515_write_register( RXM0SIDH,249,module) ;   //11111001 111

	mcp2515_write_register( RXM0SIDL,224,module) ;
	mcp2515_write_register( RXM0EID8,0,module) ;   

	mcp2515_write_register( RXM0EID0,0,module) ;

	mcp2515_write_register( RXM1SIDH,63,module) ;  //00111111
	mcp2515_write_register( RXM1SIDL,192,module) ; //110
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

uint8_t can_send_message(CANMessage *p_message,uint8_t module)
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


void can_process_message(CANMessage *p_message){
	//38A  кондиционер - прямая пересылка (906)
	//3BA кондиционер - прямая пересылка  (954)
	
	//201 Обороты и скорость (513)
	//400 Маршрутник		 (1024)
	
	//28F Экран - пересылка в зависимости от режима	(655)
	//290 Экран - пересылка в зависимости от режима (656)
	//291 Экран - пересылка в зависимости от режима (657)
	
	if ((p_message->id==906)||(p_message->id==954))
	{
		can_send_message(p_message,0);
	}

	if (p_message->id==513)
	{
		//uint16_t RPM = p_message->data[0]*256+p_message->data[1];
		uint16_t Speed = p_message->data[4]*256+p_message->data[5];
		char strbuffer[5];
		sprintf(strbuffer,"%d",Speed);
		MON_125_Speed[1]=strbuffer[0];
		MON_125_Speed[2]=strbuffer[1];
		MON_125_Speed[3]=strbuffer[2];
		MON_125_Speed[5]=strbuffer[3];
		MON_125_Speed[6]=strbuffer[4];
	}
	
	if (p_message->id==1024)
	{
		uint16_t Momental = p_message->data[2]*256+p_message->data[3];
		uint16_t AVG = p_message->data[4];
		uint16_t Remain = p_message->data[5]*256+p_message->data[6];
		
		char strbuffer[5];
		sprintf(strbuffer,"%d",Momental);
		MON_125_Momental[4]=strbuffer[0];
		MON_125_Momental[5]=strbuffer[1];
		MON_125_Momental[6]=strbuffer[2];
		MON_125_Momental[8]=strbuffer[3];
		
		sprintf(strbuffer,"%d",AVG);
		MON_125_AVG[4]=strbuffer[0];
		MON_125_AVG[5]=strbuffer[1];
		MON_125_AVG[6]=strbuffer[2];
		MON_125_AVG[8]=strbuffer[3];
		
		sprintf(strbuffer,"%d",Remain);
		MON_125_Remain[5]=strbuffer[0];
		MON_125_Remain[6]=strbuffer[1];
		MON_125_Remain[7]=strbuffer[2];
		MON_125_Remain[8]=strbuffer[3];
	}
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


#pragma endregion MCP2515

#pragma region Работа с экраном

void change_resend(uint8_t new_status)
{
	Resend_status=new_status;
}


void send_message_monitor(uint8_t module)
{
		CANMessage message;
		
		message.id=0x28f;
		message.rtr=0;
		message.length=8;
		message.data[0]=(1<<7);
		message.data[1]=0x00;
		message.data[2]=0x00;
		message.data[3]=0x00;
		message.data[4]=0x00;
		message.data[5]=0x00;
		message.data[6]=0x00;
		message.data[7]=0x00;
		
		can_send_message(&message,module);
		
		uint8_t *msg;
		
		uint8_t msg_set = ((Resend_status<<5)>>5);
		
		if(msg_set==1)
		{
			msg=MON_125_AVG;
		}
		else if (msg_set==2)
		{
			msg=MON_125_Momental;
		} 
		else if (msg_set==3)
		{
			msg=MON_125_Remain;
		}
		else if (msg_set==4)
		{
			msg=MON_125_Speed;
		}
		else if (msg_set==5)
		{
			msg=MON_125_Message;
		}
		
		
		message.id=0x290;
		message.data[0]=0xC0;
		message.data[1]=msg[0];
		message.data[2]=msg[1];
		message.data[3]=msg[2];
		message.data[4]=msg[3];
		message.data[5]=msg[4];
		message.data[6]=msg[5];
		message.data[7]=msg[6];
				
		can_send_message(&message,module);
		message.id=0x291;
		message.data[0]=0x87;
		message.data[1]=msg[7];
		message.data[2]=msg[8];
		message.data[3]=msg[9];
		message.data[4]=msg[10];
		message.data[5]=msg[11];
		message.data[6]=msg[12];
		message.data[7]=0x20;
		
		can_send_message(&message,module);

		
};

uint8_t can_read_message(CANMessage * p_message,uint8_t module)
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

#pragma endregion Работа с экраном