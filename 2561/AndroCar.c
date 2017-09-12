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

#include "SPI.h"




uint16_t car_previous_msg=0;
uint8_t car_new_msg_flag=0;

char phone_message[13]={32,65,110,100,114,111,95,67,97,114,32,32,32};
char phone_setup[8]={128,0,0,0,0,0,0,0};	
	
char car_current_message[13]={32,32,32,32,32,32,32,32,32,32,32,32};
char car_monitor_setup[8]={0,0,0,0,0,0,0,0};
double current_speed=23.34;
double momental_fuel=0.01;
uint8_t momental_fuel_resume =0;
double average_fuel=14.5;
uint16_t distance_left=1328;
uint16_t current_rpm=1;

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
	[101]	-	RPM
	[110]	-	Message from Phone
	
	Phone symbol
	
	[0]		-	Disabled
	[1]		-	Enabled
	
	Repeat AC
	
	[0]		-	Repeat CAR
	[1]		-	Inner AC
*/



