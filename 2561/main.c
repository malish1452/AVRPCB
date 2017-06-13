/*
 * 2561.c
 *
 * Created: 07.03.2017 18:20:25
 * Author : malish
 */ 


// RTOS - Thanks for DI HALT http://easyelectronics.ru/


#include <avr/io.h>
#include "HAL.h"
#include "EERTOS.h"
#include "AndroCar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>



uint8_t Vin_mode;
int stat;



uint16_t media_state_counter;
uint8_t  media_state_counter_resume;
uint8_t  media_button; //0- nothing 1-+ 2-- 3-up 4-down 5-mode 6-mute

#define Can_MAX  40

uint8_t media_button_buffer[5][2]={{0,0},{0,0},{0,0},{0,0},{0,0}}; //{button,length(0-small|1-long)}
uint16_t ADC_inputs [2][3] = {{ADC_MEDIA,0,0},{ADC_CRUISE,0,0}}; //ADCPin, ADC cycle to skip , prevousVin
uint8_t current_ADC_input;
uint8_t ADC_ready;

CANMessage CAN_incoming_buffer[Can_MAX];
uint8_t oldState,repeatState;
void Process_CAN_Message (void);
void renew_message (void);
void reenable_ADC(void);
void return_state(void);
void count_media(void);
void media_sender(void);




ISR(RTOS_ISR) //Событие таймера
{
	TimerService();
}

ISR(ADC_vect)
{
	uint16_t Vin;
	Vin=ADC;
	Vin=((float)Vin/0.8291);
	int16_t delta;
	delta=abs(ADC_inputs[current_ADC_input][2]-Vin);
	if (delta>100)
	{
		Vin_mode=1;
		ADC_inputs[current_ADC_input][2]=Vin;
		if (current_ADC_input==0)
		{
			media_state_counter_resume=0;
		}

		ADC_inputs[current_ADC_input][1]=10;
	}
	else
	{
		if (Vin_mode==1)
		{
			if (current_ADC_input==0)
				{
					media_state_counter=0;
					media_state_counter_resume=1;
					SetTimerTask(count_media,1);
					if (Vin>940)
						{
							media_button=0;
						}
					else if (Vin>630)
							{
								media_button=1;
							}
						 else
							{
								media_button=2;
							}
				}
			Vin_mode=0;
		}
	}
	ADC_ready=1;
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
	while ((i<Can_MAX)&&(CAN_incoming_buffer[i].id!=0))
	{
		i++;
	}
	LED_PORT1 &= ~(1<<LED3);
	if (i>=Can_MAX)
	{
		i=Can_MAX-1;
	}

		can_read_message(&CAN_incoming_buffer[i],0);
		SetTimerTask(Process_CAN_Message,1);
}

ISR (INT5_vect)
{
	LED_PORT0 ^= (1<<LED2);
	uint8_t i=0; 
	while ((i<Can_MAX)&&(CAN_incoming_buffer[i].id!=0))
	{
		i++;
	}
	if (i>=Can_MAX)
	{
		i=Can_MAX-1;
	}
		can_read_message(&CAN_incoming_buffer[i],1);
		SetTimerTask(Process_CAN_Message,11);
	
}

ISR (INT6_vect)
{
	uint8_t i=0;
	LED_PORT0 ^= (1<<LED1);
	while ((i<Can_MAX)&&(CAN_incoming_buffer[i].id!=0))
	{
		i++;
	}
	if (i>=Can_MAX)
	{
		i=Can_MAX-1;
	}
	can_read_message(&CAN_incoming_buffer[i],2);
	SetTimerTask(Process_CAN_Message,1);
	LED_PORT0 ^= (1<<LED1);

}

void reenable_ADC(void)
{
	cli();
	if (ADC_ready==1)
	{
		if (current_ADC_input==0)
		{
			current_ADC_input=1;
		} 
		else
		{
			current_ADC_input=0;
		}
		if (ADC_inputs[current_ADC_input][1]==0)
		{
			ADMUX|=64+ADC_inputs[current_ADC_input][0];
			ADCSRA|=(1<<ADSC);
			ADC_ready=0;
		}
		else
		{
			ADC_inputs[current_ADC_input][1]--;	
		}
		
	}
	SetTimerTask(reenable_ADC,5);
	sei();
}

void count_media(void)
{
	cli();
	
	media_state_counter=media_state_counter+1;
	if ((media_state_counter_resume==1)&&(media_state_counter<=1000))
	{
	SetTimerTask(count_media,1);
	}
	else
	{
		
		if (media_button>=0)
		{
			uint8_t length=0;
			if (media_state_counter>1000) 
			{
				length=1;
				media_sender();
			}
			uint8_t i=0;
			while ((i<5)&&(media_button_buffer[i][0]>0))
			{
				i++;
			}
				media_button_buffer[i][0]=media_button;
				media_button_buffer[i][1]=length;
			if ((i==4)||(length=1))
			{
				media_sender();
			}
			else
			{
				SetTimerTask(media_sender,1500);	
			}
		}
	}
		
	sei();
}

void media_sender(void)
{
	cli();
	uint8_t need_to_send=0;
	char result[50];
	sprintf(result,"{\"id\":1, \"duration\":%d, \"buttons\": [",media_button_buffer[0][1]);
	uint8_t i=0;
	while ((media_button_buffer[i][0]>0)&&(i<5))
	{
		
		char temp[2];
		sprintf(temp," %d",media_button_buffer[0][0]);
		strcat(result,temp);
		need_to_send=1;
		i++;
		if ((media_button_buffer[i][0]>0)&&(i<5))
		{
			strcat(result,",");
		}
	}
	strcat(result," ] }");	
	if (need_to_send==1){
		for (int8_t k=1;k<5;k++)
		{
			media_button_buffer[k][0]=0;
			media_button_buffer[k][1]=0;
		}
		UART_Add_Message(result);		
	}
	
	sei();
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

void Process_CAN_Message (void)
{	uint8_t oldSREG=SREG;
	cli();
	while (CAN_incoming_buffer[0].id!=0)
	{
		can_process_message(&CAN_incoming_buffer[0]);
		uint8_t i=0;
		while ((i<=Can_MAX-2)&&(CAN_incoming_buffer[i].id!=0))
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
		if (i>Can_MAX-1){CAN_incoming_buffer[Can_MAX-1].id=0;}
	}
	SREG=oldSREG;
}

void start_SPI()
{
	cli();
	
	MCP2515_init(0);
	UART_Add_Message("--------");
	MCP2515_init(1);
	UART_Add_Message("--------");
	MCP2515_init(2);
	UART_Add_Message("--------");
	SetTimerTask(renew_message,50);
	EIMSK|=(1<<INT4)|(1<<INT5)|(1<<INT6);
	sei();
}



void renew_message(void)
{
	uint8_t oldSREG=SREG;
	cli();
	LED_PORT1 ^= (1<<LED4);
	
	if (get_new_msg_state()==0)
	{
		if (repeatState==0)
		{
			stat= PIND>>4;
			change_resend(stat);
		} 
		else
		{
			change_resend(0);
		}
	} 
	else
	{
		SetTimerTask(return_state,2000);
		change_resend(0);
		if (repeatState==0)
		{
			repeatState=1;
			oldState=stat;
		} 
	}
    send_message_monitor(0);
	SetTimerTask(renew_message,	100);
	SREG=oldSREG;
}

void return_state (void)
{
	stat=oldState;
	repeatState=0;
	change_resend(stat);
}

int main(void)
{
	InitAll();
	InitRTOS();
	RunRTOS();
	init_UART_Buffer();
	stat=1;
	DDRC|=(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7);
	DDRD|=(0<<4)|(0<<5)|(0<<6)|(0<<7);
	PORTC&=~((1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7));
	DDRF=0;
	PORTF=0;
	
	Vin_mode=0;
	
	media_state_counter=0;
	current_ADC_input=0;
	ADC_ready=1;
	reenable_ADC();
	//ADCSRA|=(1<<ADSC);
	for (uint8_t i =0 ; i<Can_MAX;i++)
	{
		CAN_incoming_buffer[i].id=0;
	}
	init_SPI();
	start_SPI();
	//show_counter();
		SetTask(Process_CAN_Message);
	media_state_counter_resume=0;
	media_state_counter=0;
//	SetTask(count_media);
	
    while (1) 
    {
		wdt_reset();
		TaskManager();
		UART_Command_Processor();
    }
}

