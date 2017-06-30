/*
 * TWI.h
 *
 * Created: 28.06.2017 21:08:16
 *  Author: Kirpichik
 */ 
#include <stdio.h>

#ifndef TWI_H_
#define TWI_H_

#define IIC_MaxBuffer 20


#define i2c_PORT	PORTD		// Порт где сидит нога TWI
#define i2c_DDR		DDRD
#define i2c_SCL		0		// Биты соответствующих выводов
#define i2c_SDA		1


#define i2c_MasterAddress 	0x32	// Адрес на который будем отзываться

#define i2c_MaxBuffer		3	// Величина буфера Master режима RX-TX. Зависит от того какой длины строки мы будем гонять
#define i2c_MaxPageAddrLgth	2	// Максимальная величина адреса страницы. Обычно адрес страницы это один или два байта.
// Зависит от типа ЕЕПРОМ или другой микросхемы.



#define i2c_type_msk	0b00001100	// Маска режима
#define i2c_sarp	0b00000000	// Start-Addr_R-Read-Stop  							Это режим простого чтения. Например из слейва или из епрома с текущего адреса
#define i2c_sawp	0b00000100	// Start-Addr_W-Write-Stop 							Это режим простой записи. В том числе и запись с адресом страницы.
#define i2c_sawsarp	0b00001000	// Start-Addr_W-WrPageAdr-rStart-Addr_R-Read-Stop 	Это режим с предварительной записью текущего адреса страницы

#define i2c_Err_msk	0b00110011	// Маска кода ошибок
#define i2c_Err_NO	0b00000000	// All Right! 						-- Все окей, передача успешна.
#define i2c_ERR_NA	0b00010000	// Device No Answer 				-- Слейв не отвечает. Т.к. либо занят, либо его нет на линии.
#define i2c_ERR_LP	0b00100000	// Low Priority 					-- нас перехватили собственным адресом, либо мы проиграли арбитраж
#define i2c_ERR_NK	0b00000010	// Received NACK. End Transmittion. -- Был получен NACK. Бывает и так.
#define i2c_ERR_BF	0b00000001	// BUS FAIL 						-- Автобус сломался. И этим все сказано. Можно попробовать сделать переинициализацию шины

#define i2c_Interrupted		0b10000000	// Transmiting Interrupted		Битмаска установки флага занятости
#define i2c_NoInterrupted 	0b01111111  	// Transmiting No Interrupted	Битмаска снятия флага занятости

#define i2c_Busy		0b01000000  	// Trans is Busy				Битмаска флага "Передатчик занят, руками не трогать".
#define i2c_Free		0b10111111  	// Trans is Free				Битмаска снятия флага занятости.
typedef void (*IIC_F)(void);

typedef struct
{
	uint8_t		addr;	
	uint8_t		reg_addr;
	uint8_t		data[7];
	uint8_t		index;
	uint8_t		count;
	uint8_t		resume;
	
	IIC_F		waiter;
	
} IIC_Msg;

extern void IIC_done();
extern void init_I2c();
extern void Add_Task(IIC_Msg Msg);

#endif /* TWI_H_ */