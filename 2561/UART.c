/*
 * UART.c
 *
 * Created: 04.09.2017 10:38:06
 *  Author: Kirpichik
 */ 

#include "UART.h"

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



ISR (USART1_UDRE_vect) //Опустошение буфера
{
	UART_Send_Buffer_Empty();
}

ISR (USART1_TX_vect)   //Окончание передачи
{
	UART_Send_Ended();
}

ISR (USART1_RX_vect) //получен символ
{
	UART_Char_Received();
}



ISR (USART0_RX_vect) //получен символ
{
	UART0_Char_Received();
}


void UART1_init()
{
		UBRR1H = HI(bauddivider);
		UBRR1L = LO(bauddivider);
		UCSR1A = 0;
		UCSR1B = 1<<RXCIE1|0<<TXCIE1|0<<UDRIE1|1<<TXEN1|1<<RXEN1|0<<UCSZ12;
		UCSR1C=0b10000110;
		init_UART_Buffer();
}

void UART0_init()
{
	UBRR0H = HI(bauddivider);
	UBRR0L = LO(bauddivider);
	UCSR0A = 0;
	UCSR0B = 1<<RXCIE0|0<<TXCIE0|0<<UDRIE0|1<<TXEN0|1<<RXEN0|0<<UCSZ02;
	UCSR0C = 0b10000110;
}


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

void UART0_Char_Received()
{
	char received_char;
	received_char=UDR0;
	//UART_Add_Message(received_char);
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
	uint8_t oldSREG=SREG;
	cli();
	
	if (UART_Command_Stack.size>0)
	{
		
		
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
		
	}
	
	SREG=oldSREG;
}
