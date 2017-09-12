/*
 * Display.h
 *
 * Created: 04.09.2017 19:25:23
 *  Author: Kirpichik
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_
#define DISPLAY_NUMBER	2

#include "EERTOS.h"
#include "avr/pgmspace.h"

extern void monitor_message_sender(void);

extern void monitor_message_load(char *msg);

extern void change_resend(uint8_t new_status);



#endif /* DISPLAY_H_ */