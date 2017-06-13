/*
 * EERTOSHAL.c
 *
 * Created: 07.03.2017 20:09:57
 *  Author: malish
 */ 
#include "EERTOSHAL.h"


inline void RunRTOS (void)
{
	TCCR2B  = 1<<WGM22|4<<CS20;				// Freq = CK/64 - Установить режим и предделитель
											// Автосброс после достижения регистра сравнения
	TCNT2 = 0;								// Установить начальное значение счётчиков
	OCR2A  = LO(TimerDivider); 				// Установить значение в регистр сравнения
	TIMSK2 = 0<<TOIE2|1<<OCIE2A;		        // Разрешаем прерывание RTOS - запуск ОС

	//sei();
}