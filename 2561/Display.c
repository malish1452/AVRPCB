/*
 * Display.c
 *
 * Created: 04.09.2017 19:24:30
 *  Author: Kirpichik
 */ 
#include "Display.h"
#include "MCP2515.h"
#include "avr/pgmspace.h"

char monitor_current_message[13]={32,48,49,50,32,32,32,32,32,32,32,32};
char monitor_current_setup[8]={128,0,0,0,0,0,0,0};
char monitor_default_setup[8]={128,0,0,0,0,0,0,0};

void monitor_message_load(char *msg)
{	
	uint8_t oldSREG=SREG;
	cli();
	for (uint8_t i=0;i<12;i++)
	{
		monitor_current_message[i]=msg[i];
	}
	monitor_message_sender();
	SREG=oldSREG;
}

void monitor_message_sender(void)																																																																																																																																																																																																																																																																																																																																																																							
{
		uint8_t oldSREG=SREG;
		cli();
		PORTA^=(1<<3);
		
		CANMessage message;
		
		message.id=0x28f;
		message.rtr=0;
		message.length=8;
		message.data[0]=monitor_current_setup[0];
		message.data[1]=monitor_current_setup[1];
		message.data[2]=monitor_current_setup[2];
		message.data[3]=monitor_current_setup[3];
		message.data[4]=monitor_current_setup[4];
		message.data[5]=monitor_current_setup[5];
		message.data[6]=monitor_current_setup[6];
		message.data[7]=monitor_current_setup[7];
		
		mcp2515_can_send_message(&message,DISPLAY_NUMBER);
		
		message.id=0x290;
		message.data[0]=128;
		message.data[1]=monitor_current_message[0];
		message.data[2]=monitor_current_message[1];
		message.data[3]=monitor_current_message[2];
		message.data[4]=monitor_current_message[3];
		message.data[5]=monitor_current_message[4];
		message.data[6]=monitor_current_message[5];
		message.data[7]=monitor_current_message[6];
				
		mcp2515_can_send_message(&message,DISPLAY_NUMBER);
		
		message.id=0x291;
		message.data[0]=0x87;
		message.data[1]=monitor_current_message[7];
		message.data[2]=monitor_current_message[8];
		message.data[3]=monitor_current_message[9];
		message.data[4]=monitor_current_message[10];
		message.data[5]=monitor_current_message[11];
		message.data[6]=0x20;
		message.data[7]=0x20;
		
		mcp2515_can_send_message(&message,DISPLAY_NUMBER);		
		SetTimerTask(monitor_message_sender,1000);
		SREG=oldSREG;
};

void change_resend(uint8_t new_status)
{
	////char message[12];
	//uint16_t tempvalint1,tempvalint2;
	//
	//if (new_status==0)
	//{
		//for (int i=0;i<8;i++)
		//{
			//monitor_current_setup[i]=car_monitor_setup[i];
		//}
		//
		//for (int i=0;i<12;i++)
		//{
			//monitor_current_message[i]=car_current_message[i];
		//}
	//}
	//
	//if (new_status>0)
	//{
		//for (int8_t i=0;i<8;i++)
		//{
			//monitor_current_setup[i]=monitor_default_setup[i];
		//}
	//}
	//
	//
	//if (new_status==1)
	//{
		//tempvalint1=average_fuel;
		//tempvalint2=trunc((average_fuel-tempvalint1)*10);
		//sprintf(monitor_current_message,"Avg: %3d.%01d L",tempvalint1,tempvalint2);
	//}
//
	//if (new_status==2)
	//{
		//tempvalint1=momental_fuel;
		//tempvalint2=trunc((momental_fuel-tempvalint1)*10);
		//if (momental_fuel_resume==0)
		//{
			//sprintf(monitor_current_message,"Mom:%3d.%01d L",tempvalint1,tempvalint2);
		//}
		//else
		//{
			//sprintf(monitor_current_message,"Mom: 1.0 L/h");
		//}
		//
	//}
//
	//if (new_status==3)
	//{
		//sprintf(monitor_current_message,"Rem: %4d Km",distance_left);
	//}
//
	//if (new_status==4)
	//{
		//tempvalint1=current_speed;
		//tempvalint2=trunc((current_speed-tempvalint1)*100);
		//sprintf(monitor_current_message," %3d.%02d Km/h",tempvalint1,tempvalint2);
	//}
//
	//if (new_status==5)
	//{
		//sprintf(monitor_current_message,"RPM:  %4d  ",current_rpm);
	//}
	//
	//if (new_status==6)
	//{
		//for (uint8_t i=0;i<12;i++)
		//{
			//monitor_current_message[i]=phone_message[i];
		//}
	//}
	//
	//Resend_status=new_status;
}
