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


#define i2c_PORT	PORTD		// ���� ��� ����� ���� TWI
#define i2c_DDR		DDRD
#define i2c_SCL		0		// ���� ��������������� �������
#define i2c_SDA		1


#define i2c_MasterAddress 	0x32	// ����� �� ������� ����� ����������

#define i2c_MaxBuffer		3	// �������� ������ Master ������ RX-TX. ������� �� ���� ����� ����� ������ �� ����� ������
#define i2c_MaxPageAddrLgth	2	// ������������ �������� ������ ��������. ������ ����� �������� ��� ���� ��� ��� �����.
// ������� �� ���� ������ ��� ������ ����������.



#define i2c_type_msk	0b00001100	// ����� ������
#define i2c_sarp	0b00000000	// Start-Addr_R-Read-Stop  							��� ����� �������� ������. �������� �� ������ ��� �� ������ � �������� ������
#define i2c_sawp	0b00000100	// Start-Addr_W-Write-Stop 							��� ����� ������� ������. � ��� ����� � ������ � ������� ��������.
#define i2c_sawsarp	0b00001000	// Start-Addr_W-WrPageAdr-rStart-Addr_R-Read-Stop 	��� ����� � ��������������� ������� �������� ������ ��������

#define i2c_Err_msk	0b00110011	// ����� ���� ������
#define i2c_Err_NO	0b00000000	// All Right! 						-- ��� ����, �������� �������.
#define i2c_ERR_NA	0b00010000	// Device No Answer 				-- ����� �� ��������. �.�. ���� �����, ���� ��� ��� �� �����.
#define i2c_ERR_LP	0b00100000	// Low Priority 					-- ��� ����������� ����������� �������, ���� �� ��������� ��������
#define i2c_ERR_NK	0b00000010	// Received NACK. End Transmittion. -- ��� ������� NACK. ������ � ���.
#define i2c_ERR_BF	0b00000001	// BUS FAIL 						-- ������� ��������. � ���� ��� �������. ����� ����������� ������� ����������������� ����

#define i2c_Interrupted		0b10000000	// Transmiting Interrupted		�������� ��������� ����� ���������
#define i2c_NoInterrupted 	0b01111111  	// Transmiting No Interrupted	�������� ������ ����� ���������

#define i2c_Busy		0b01000000  	// Trans is Busy				�������� ����� "���������� �����, ������ �� �������".
#define i2c_Free		0b10111111  	// Trans is Free				�������� ������ ����� ���������.
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