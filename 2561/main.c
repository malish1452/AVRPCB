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

ISR (INT0_vect)
{
	 LED_PORT ^=1<<LED2;
	 

	 uint8_t i=0;
	 while ((CAN_incoming_buffer[i].id!=0)&&(i<40))
	 {
		 
		 i++;
	 }
	 if (i<40)
	 {
		 can_read_message(&CAN_incoming_buffer[i],0);
		 SetTimerTask(Task1,100);
		 
	 }
	
}

ISR (INT1_vect)
{
	 
	 LED_PORT ^=1<<LED2;
	 
	
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

					
//	EIMSK&=~(1<<INT1);
//	EIFR&=~(1<<INTF1);
}




void Task1 (void)
{
			char result[70];
	
	LED_PORT ^=1<<LED1;
	
	//uint8_t t = mcp2515_read_register(TEC,1);
	//char result [30];
	//sprintf(result,"TEC: %d",t);
	//UART_Add_Message(result);
	//
	//t = mcp2515_read_register(REC,1);
	//sprintf(result,"REC: %d",t);
	//UART_Add_Message(result);
	//
	//t = mcp2515_read_register(EFLG,1);
	//sprintf(result,"EFLG: %d",t);
	//UART_Add_Message(result);
	//
	//t = mcp2515_read_register(CANINTE,1);
	//sprintf(result,"CANINTE: %d",t);
	//UART_Add_Message(result);
	//
	//t = mcp2515_read_register(CANINTF,1);
	//sprintf(result,"CANINTF: %d",t);
	//UART_Add_Message(result);
	//
	//t = mcp2515_read_register(CANCTRL,1);
	//sprintf(result,"CANCTRL: %d",t);
	//UART_Add_Message(result);
	//
	//t = mcp2515_read_register(CANSTAT,1);
	//sprintf(result,"CANSTAT: %d",t);
	//UART_Add_Message(result);
	//
	//t = mcp2515_read_register(BFPCTRL,1);
	//sprintf(result,"BFPCTRL: %d",t);
	//UART_Add_Message(result);
	//
	//t = mcp2515_read_register(TXRTSCTRL,1);
	//sprintf(result,"TXRTSCTRL: %d",t);
	//UART_Add_Message(result);
	
cli();
	while (CAN_incoming_buffer[0].id!=0)
	{
	
		//if (CAN_incoming_buffer[0].id!=0x401)
		//{
			
			
	
		sprintf(result,"{\"A\":\"%X\",\"B\":\"%X\",\"C\":[\"%X\",\"%X\",\"%X\",\"%X\",\"%X\",\"%X\",\"%X\",\"%X\"]}",CAN_incoming_buffer[0].id,CAN_incoming_buffer[0].length,CAN_incoming_buffer[0].data[0],CAN_incoming_buffer[0].data[1],CAN_incoming_buffer[0].data[2],CAN_incoming_buffer[0].data[3],CAN_incoming_buffer[0].data[4],CAN_incoming_buffer[0].data[5],CAN_incoming_buffer[0].data[6],CAN_incoming_buffer[0].data[7]);
		
		//sprintf(result,"s %d",5);
		
		UART_Add_Message(result);
		
		//}
		
		
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
	
	SetTimerTask(Task3,10000);
	MCP2515_init(0);
	MCP2515_init(1);
	
	UART_Add_Message("--------");
		EIMSK|=(1<<INT1)|(1<<INT0);

	
}

void Task3(void)
{
	
	//038A#43#00#00#00#00#00#00#00#
	//03BA#0C#53#1A#80#00#31#C2#40#
	
	change_resend(stat);
	
	stat++;
	if (stat>5) stat=1;
		
//	MCP2515_init(0);
//	MCP2515_init(1);	
  
	
	send_message_monitor(stat&1);
	
	
	

	//char result[30];
	//sprintf(result,"PORTS B0: %d, B1: %d",a,b);
	//UART_Add_Message(result);
	
	SetTimerTask(Task3,5510);
	
	
	
//	mcp2515_soft_reset(0);
//	mcp2515_soft_reset(1);
	
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

