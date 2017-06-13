/*
 * EERTOSHAL.c
 *
 * Created: 07.03.2017 20:09:57
 *  Author: malish
 */ 
#include "EERTOSHAL.h"


inline void RunRTOS (void)
{
	TCCR2B  = 1<<WGM22|4<<CS20;				// Freq = CK/64 - ���������� ����� � ������������
											// ��������� ����� ���������� �������� ���������
	TCNT2 = 0;								// ���������� ��������� �������� ���������
	OCR2A  = LO(TimerDivider); 				// ���������� �������� � ������� ���������
	TIMSK2 = 0<<TOIE2|1<<OCIE2A;		        // ��������� ���������� RTOS - ������ ��

	//sei();
}