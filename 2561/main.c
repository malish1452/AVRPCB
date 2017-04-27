/*
 * 2561.c
 *
 * Created: 07.03.2017 18:20:25
 * Author : malish
 */ 



#include <avr/io.h>
#include "HAL.h"
#include "EERTOS.h"
#include "AndroCar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int Vin;

int stat;


CANMessage CAN_incoming_buffer[40];
void Task1 (void);
void Task2 (void);
void Task3 (void);




ISR(RTOS_ISR) //Событие таймера
{
	TimerService();
}

ISR(ADC_vect)
{
	Vin=ADC;
}

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

ISR (SPI_STC_vect)
{
	//SPI_Send_Ready();
}

ISR (INT4_vect)
{
	LED_PORT1 |= (1<<LED3);
	uint8_t i=0;
	while ((CAN_incoming_buffer[i].id!=0)&&(i<40))
	{
		i++;
	}
	if (i>=40)
	{
		i=39;
		LED_PORT0^=(1<<LED1);
	}
		can_read_message(&CAN_incoming_buffer[i],0);
		SetTimerTask(Task1,1);
	
	LED_PORT1 &= ~(1<<LED3);
}

ISR (INT5_vect)
{
	LED_PORT0 ^= (1<<LED2);
	uint8_t i=0; 
	while ((CAN_incoming_buffer[i].id!=0)&&(i<40))
	{
		i++;
	}
	if (i<40)
	{ 
		can_read_message(&CAN_incoming_buffer[i],1);
		SetTimerTask(Task1,100);
	}
}

ISR (INT6_vect)
{
	uint8_t i=0;
	LED_PORT0 ^= (1<<LED1);
	while ((CAN_incoming_buffer[i].id!=0)&&(i<40))
	{
		i++;
	}
	if (i<40)
	{
		can_read_message(&CAN_incoming_buffer[i],2);
		SetTimerTask(Task1,50);
	}
}

void readstats(uint8_t module)
{
	char result[70];
	
	uint8_t t = mcp2515_read_register(TEC,module);
	//char result [30];
	sprintf(result,"TEC: %d",t);
	UART_Add_Message(result);
	
	t = mcp2515_read_register(REC,module);
	sprintf(result,"REC: %d",t);
	UART_Add_Message(result);
	
	t = mcp2515_read_register(EFLG,module);
	sprintf(result,"EFLG: %d",t);
	UART_Add_Message(result);
	
	t = mcp2515_read_register(CANINTE,module);
	sprintf(result,"CANINTE: %d",t);
	UART_Add_Message(result);
	
	t = mcp2515_read_register(CANINTF,module);
	sprintf(result,"CANINTF: %d",t);
	UART_Add_Message(result);
	
	//t = mcp2515_read_register(CANINTF,1);
	//sprintf(result,"CANINTF: %d",DDRB);
	//UART_Add_Message(result);
	//
	//t = mcp2515_read_register(CANINTF,2);
	//sprintf(result,"CANINTF: %d",);
	//UART_Add_Message(result);
	
	
	
	t = mcp2515_read_register(CANCTRL,module);
	sprintf(result,"CANCTRL: %d",t);
	UART_Add_Message(result);
	
	t = mcp2515_read_register(CANSTAT,module);
	sprintf(result,"CANSTAT: %d",t);
	UART_Add_Message(result);
	
	t = mcp2515_read_register(BFPCTRL,module);
	sprintf(result,"BFPCTRL: %d",t);
	UART_Add_Message(result);
	
	t = mcp2515_read_register(TXRTSCTRL,module);
	sprintf(result,"TXRTSCTRL: %d",t);
	UART_Add_Message(result);
}

void Task1 (void)
{	cli();
	char result[70];
	

	while (CAN_incoming_buffer[0].id!=0)
	{
		sprintf(result,"{\"A\":\"%X\",\"B\":\"%X\",\"C\":[\"%X\",\"%X\",\"%X\",\"%X\",\"%X\",\"%X\",\"%X\",\"%X\"]}",CAN_incoming_buffer[0].id,CAN_incoming_buffer[0].length,CAN_incoming_buffer[0].data[0],CAN_incoming_buffer[0].data[1],CAN_incoming_buffer[0].data[2],CAN_incoming_buffer[0].data[3],CAN_incoming_buffer[0].data[4],CAN_incoming_buffer[0].data[5],CAN_incoming_buffer[0].data[6],CAN_incoming_buffer[0].data[7]);
	//	UART_Add_Message(result);

		uint8_t i=0;
		while (CAN_incoming_buffer[i].id!=0)
		{	i++;	
			CAN_incoming_buffer[i-1].id=CAN_incoming_buffer[i].id;
			CAN_incoming_buffer[i-1].length=CAN_incoming_buffer[i].length;
			CAN_incoming_buffer[i-1].rtr=CAN_incoming_buffer[i].rtr;
			CAN_incoming_buffer[i-1].data[0]=CAN_incoming_buffer[i].data[0];
			CAN_incoming_buffer[i-1].data[1]=CAN_incoming_buffer[i].data[1];
			CAN_incoming_buffer[i-1].data[2]=CAN_incoming_buffer[i].data[2];
			CAN_incoming_buffer[i-1].data[3]=CAN_incoming_buffer[i].data[3];
			CAN_incoming_buffer[i-1].data[4]=CAN_incoming_buffer[i].data[4];
			CAN_incoming_buffer[i-1].data[5]=CAN_incoming_buffer[i].data[5];
			CAN_incoming_buffer[i-1].data[6]=CAN_incoming_buffer[i].data[6];
			CAN_incoming_buffer[i-1].data[7]=CAN_incoming_buffer[i].data[7];
		}
	}
	sei();
}

void Task2(void)
{
	SetTimerTask(Task3,5000);
	
	MCP2515_init(0);
	UART_Add_Message("--------");
	MCP2515_init(1);
	UART_Add_Message("--------");
	MCP2515_init(2);
	UART_Add_Message("--------");
	EIMSK|=(1<<INT4)|(1<<INT5)|(1<<INT6);
}



void Task3(void)
{
	//038A#43#00#00#00#00#00#00#00#
	//03BA#0C#53#1A#80#00#31#C2#40#
	cli();
	LED_PORT1 ^= (1<<LED4);

	change_resend(stat);
	//readstats(0);
	//readstats(1);
	//readstats(2);
	stat++;
	if (stat>5) stat=1;
	send_message_monitor(0);
	SetTimerTask(Task3,	2000);
	sei();
}




int main(void)
{
	InitAll();
	InitRTOS();
	RunRTOS();
	init_UART_Buffer();
	init_SPI();
	
	stat=1;
	
	for (uint8_t i =0 ; i<40;i++)
	{
		CAN_incoming_buffer[i].id=0;
	}

	SetTask(Task1);
	SetTask(Task2);
	
    /* Replace with your application code */
    while (1) 
    {
		wdt_reset();
		TaskManager();
		UART_Command_Processor();
    }
}

