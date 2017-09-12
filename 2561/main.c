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
#include "TWI.h"
#include "MCP2515.h"
#include "UART.h"
#include "Display.h"
#include "SPI.h"
#include "AD5206.h"
#include <util/delay.h>
#include "CSR8645.h"

uint8_t Vin_mode;
int stat;

long resistance; 

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

uint8_t button;
uint8_t media_mode;

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

ISR (INT4_vect)
{
	

	uint8_t i=0;
	while ((i<Can_MAX)&&(CAN_incoming_buffer[i].id!=0))
	{
		i++;
	}
	
	if (i>=Can_MAX)
	{
		i=Can_MAX-1;
	}

		mcp2515_can_read_message(&CAN_incoming_buffer[i],0);
		SetTimerTask(Process_CAN_Message,1);
}

ISR (INT5_vect)
{

	uint8_t i=0; 
	while ((i<Can_MAX)&&(CAN_incoming_buffer[i].id!=0))
	{
		i++;
	}
	if (i>=Can_MAX)
	{
		i=Can_MAX-1;
	}
		mcp2515_can_read_message(&CAN_incoming_buffer[i],1);
		SetTimerTask(Process_CAN_Message,11);
	
}

ISR (INT6_vect)
{

	uint8_t i=0;
	
	while ((i<Can_MAX)&&(CAN_incoming_buffer[i].id!=0))
	{
		i++;
	}
	if (i>=Can_MAX)
	{
		i=Can_MAX-1;
	}
	mcp2515_can_read_message(&CAN_incoming_buffer[i],2);
	SetTimerTask(Process_CAN_Message,1);
	

}

void I2c_getter(void)
{
	cli();
	IIC_Msg result;
    result=Get_Msg(&I2c_getter);
	char str[40];
	sprintf(str,"out:[%d], [%d], [%d], [%d], [%d], [%d], [%d]",  (result.data[0]>>4)*10+(result.data[0]&0x0F),(result.data[1]>>4)*10+(result.data[1]&0x0F),(result.data[2]>>4)*10+(result.data[2]&0x0F),result.data[3],(result.data[4]>>4)*10+(result.data[4]&0x0F),((result.data[5]&0x10)>>4)*10+(result.data[5]&0x0F),((result.data[5]>>7)+19)*100+(result.data[6]>>4)*10+(result.data[6]&0x0F));
	UART_Add_Message(str);
	Delete_MSG(result.number);
	sei();
}

void I2c_after_set(void)
{
		cli();
		UART_Add_Message("AFTER SENT");
		
		IIC_Msg result;
		result=Get_Msg(&I2c_after_set);
		Delete_MSG(result.number);
		sei();
}
void I2c_SendAddr(void)
{
	cli();
	IIC_Msg msg;
	msg.addr=0xD0;
	msg.reg_addr=0;
	msg.resume=i2c_sawsarp;
	msg.count=7;
	msg.waiter=&I2c_getter;
	Add_Task(msg);
	SetTimerTask(I2c_SendAddr,2000);
	sei();
	
}

void I2c_readtemp(void)
{
	cli();
	IIC_Msg msg;
	msg.addr=0xD0;
	msg.reg_addr=0x11;
	msg.resume=i2c_sawsarp;
	msg.count=1;
//	msg.waiter=&I2c_gettemp;
	Add_Task(msg);
	SetTimerTask(I2c_readtemp,2000);
	sei();
	
}

void I2c_set_time(uint8_t sec, uint8_t min, uint8_t hour, uint8_t date, uint8_t month, uint16_t year)
{
	cli();
	//LED_PORT0|=(1<<LED1);
	IIC_Msg setup;
	setup.addr=0xD0;
	setup.reg_addr=0;
//	setup.data[0]=0;
	setup.resume=i2c_sawp;
	setup.count=7;
	setup.index=0;
	setup.data[0]=((sec/10)*16)+sec%10;
	setup.data[1]=((min/10)*16)+min%10;
	setup.data[2]=((hour/10)*16)+hour%10;
	uint8_t a,m;
	uint16_t y,dow;
	a=(14-month)/12;
	y=year-a;
	m=month+12*a-2;
	dow=((date+y+y/4-y/100+y/400+(31*m)/12)%7)+1;
	setup.data[3]=dow;
	setup.data[4]=((date/10)*16)+date%10;
	a=(year/100)-19;
	y=year%100;
	setup.data[5]=(a*128)+((month/10)*16)+month%10;
	setup.data[6]=((y/10)*16)+y%10;
	setup.waiter=&I2c_after_set;
	Add_Task(setup);
	SetTimerTask(I2c_SendAddr,2000);
	sei();
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
		mcp2515_can_process_message(&CAN_incoming_buffer[0]);
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

void start_mcp2515()
{
	cli();
	
	for (uint8_t i =0 ; i<Can_MAX;i++)
	{
		CAN_incoming_buffer[i].id=0;
	}

	mcp2515_init(0);
	mcp2515_init(1);
	mcp2515_init(2);
	EIMSK=(1<<INT6)|(1<<INT5)|(1<<INT4);
	
	SetTask(Process_CAN_Message);
	SetTask(monitor_message_sender);
	sei();
}



void pushmedia(void)
{
	if (media_mode==0) 
	{
		if (button==0)
		{
			button=1;
		}
		else
		{
			button=button<<1;
		}
		char result[8];
		sprintf(result,"button:%d",button);
		UART_Add_Message(result);
		
		PORTC=button;
	}
	else
	{
		PORTC=0;
		UART_Add_Message("release");
	}
	
	if (media_mode==0)
	{
		media_mode=1;
		SetTimerTask(pushmedia,40);
	}
	else
	{
		media_mode=0;
		SetTimerTask(pushmedia,1000);
	}	
}


void return_state (void)
{
	stat=oldState;
	repeatState=0;
	change_resend(stat);
}


void RDAC_temp(int res)
{
	RDAC_SetRes(0,res*4);
	RDAC_SetRes(2,res*4);
	RDAC_SetRes(4,res*4);
	RDAC_SetRes(5,res*4);
}

int main(void)
{	
	LED_DDR|=(1<<LED0)|(1<<LED1)|(1<<LED2)|(1<<LED3);
	GPS_DDR|=(1<<GPS_EN);
	GPS_PORT|=(1<<GPS_EN);

	InitAll();
	InitRTOS();
	RunRTOS();
	UART1_init();
	UART0_init();
	UART_Add_Message("start");
	//I2c_set_time(0,56,16,5,7,2017);
	//stat=1;
	//
	//Vin_mode=0;
	//
	//media_state_counter=0;
	//current_ADC_input=0;
	//ADC_ready=1;
	//reenable_ADC();
	//pushmedia();
	//ADCSRA|=(1<<ADSC);
	

	init_SPI();
	RDAC_init();
	RDAC_FullRes();

	
	Init_All_PIO();
	for (int i=0; i<11;i++)
	{
		Push_PIO(i);
		_delay_ms(500);
		Release_PIO(i);
		_delay_ms(500);
	}
	start_mcp2515();	
	media_state_counter_resume=0;
	media_state_counter=0;
	char res[12];
	for (int j=0;j<300;j++)
	{
		//PORTA^=(1<<2);
		sprintf(res,"    %05d    ",j);
		monitor_message_load(res);
		_delay_us(1900);
	}	
    while (1) 
    {

		wdt_reset();
		TaskManager();
		UART_Command_Processor();
	
    }
}

