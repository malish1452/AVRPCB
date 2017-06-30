/*
 * TWI.c
 *
 * Created: 28.06.2017 20:52:37
 *  Author: Kirpichik
 */ 
#include "TWI.h"
#include "EERTOS.h"
#include <stdio.h>
#include <avr/io.h>

volatile uint8_t i2c_Do;						// ���������� ��������� ����������� IIC
					// ������ ������ Slave

IIC_Msg IIC_Buffer[IIC_MaxBuffer];

uint8_t IIC_index;


uint8_t i2c_Buffer[i2c_MaxBuffer];				// ����� ��� ������ ������ � ������ Master
uint8_t i2c_index;								// ������ ����� ������
uint8_t i2c_ByteCount;							// ����� ���� ������������

uint8_t i2c_SlaveAddress;						// ����� ������������

uint8_t i2c_PageAddress[i2c_MaxPageAddrLgth];	// ����� ������ ������� (��� ������ � sawsarp)
uint8_t i2c_PageAddrIndex;						// ������ ������ ������ �������
uint8_t i2c_PageAddrCount;						// ����� ���� � ������ �������� ��� �������� Slave


ISR(TWI_vect)								// ���������� TWI ��� ���� ���.
{
	PORTG^=1;
switch(TWSR & 0xF8)											// �������� ���� ����������
	{
	case 0x00:	// Bus Fail (������� ��������)
			{
			IIC_Buffer[IIC_index].resume |= i2c_ERR_BF;
 
			TWCR = 	0<<TWSTA|
				1<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;  	// Go!
 
			break;
			}
 
	case 0x08:	// ����� ���, � ����� ��:
			{
			if( (IIC_Buffer[IIC_index].resume & i2c_type_msk)== i2c_sarp)			// � ����������� �� ������
				{
				IIC_Buffer[IIC_index].addr |= 0x01;					// ���� Addr+R
				}
			else											// ��� 
				{
				IIC_Buffer[IIC_index].addr &= 0xFE;					// ���� Addr+W
				}
 
			TWDR = IIC_Buffer[IIC_index].addr;				// ����� ������
			TWCR = 	0<<TWSTA|
				0<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;  								// Go!
			break;
			}
 
	case 0x10:	// ��������� ����� ���, � ����� ��
			{
			if( (IIC_Buffer[IIC_index].resume & i2c_type_msk) == i2c_sawsarp)		// � ����������� �� ������
				{
				IIC_Buffer[IIC_index].addr |= 0x01;					// ���� Addr+R
				}
			else
				{
				IIC_Buffer[IIC_index].addr &= 0xFE;					// ���� Addr+W
				}
 
			// To Do: �������� ���� ��������� ������ 
 
			TWDR = IIC_Buffer[IIC_index].addr;				// ����� ������
			TWCR = 	0<<TWSTA|
				0<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;  	// Go!
			break;
			}
 
	case 0x18:	// ��� ������ SLA+W �������� ACK, � �����:
			{

			if( (IIC_Buffer[IIC_index].resume & i2c_type_msk) == i2c_sawp)		// � ����������� �� ������
				{
				TWDR = IIC_Buffer[IIC_index].data[IIC_Buffer[IIC_index].index];				// ���� ���� ������
				IIC_Buffer[IIC_index].index++;							// ����������� ��������� ������
 
				TWCR = 	0<<TWSTA|
					0<<TWSTO|
					1<<TWINT|
					1<<TWEN|
					1<<TWIE;  // Go! 
 
				}
 
			if( (IIC_Buffer[IIC_index].resume & i2c_type_msk) == i2c_sawsarp)
				{
				TWDR = IIC_Buffer[IIC_index].reg_addr;	// ��� ���� ����� ������� (�� ���� ���� ���� ������)
														// ����������� ��������� ������ ��������
				TWCR = 	0<<TWSTA|
					0<<TWSTO|
					1<<TWINT|
					1<<TWEN|
					1<<TWIE;	// Go!
				}
			}
			break;
 
	case 0x20:	// ��� ������ SLA+W �������� NACK - ����� ���� �����, ���� ��� ��� ����.
			{
			IIC_Buffer[IIC_index].resume |= i2c_ERR_NA;	
															// ��� ������
			TWCR = 	0<<TWSTA|
				1<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;								// ���� ���� Stop
 
			IIC_done();						// ������������ ������� ������;
			break;
			}
 
	case 0x28: 	// ���� ������ �������, �������� ACK!  (���� sawp - ��� ��� ���� ������. ���� sawsarp - ���� ������ ��������)
			{	// � ������: 
			if( (IIC_Buffer[IIC_index].resume & i2c_type_msk) == i2c_sawp)		// � ����������� �� ������
				{
				if (IIC_Buffer[IIC_index].index == IIC_Buffer[IIC_index].count)				// ���� ��� ���� ������ ���������
					{																		
					TWCR = 	0<<TWSTA|
						1<<TWSTO|
						1<<TWINT|
						1<<TWEN|
						1<<TWIE;						// ���� Stop
 
					IIC_done();				// � ������� � ��������� �����
 
					}
				else
					{
					TWDR = IIC_Buffer[IIC_index].data[IIC_Buffer[IIC_index].index];			// ���� ���� ��� ���� ����
					IIC_Buffer[IIC_index].index++;
					TWCR = 	0<<TWSTA|						
						0<<TWSTO|
						1<<TWINT|
						1<<TWEN|
						1<<TWIE;  						// Go!
					}
				}
 
			if( (IIC_Buffer[IIC_index].resume & i2c_type_msk) == i2c_sawsarp)		// � ������ ������ ��
				{
					TWCR = 	1<<TWSTA|
						0<<TWSTO|
						1<<TWINT|
						1<<TWEN|
						1<<TWIE;						// ��������� ��������� �����!

				}	 
			}
			break;
 
	case 0x30:	//���� ����, �� �������� NACK ������ ���. 1� �������� �������� ������� � ��� ����. 2� ����� �������.
			{
			IIC_Buffer[IIC_index].resume |= i2c_ERR_NK;						// ������� ������ ������. ���� ��� �� ����, ��� ������. 
 
			TWCR = 	0<<TWSTA|
				1<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;							// ���� Stop
 
			IIC_done();					// ������������ ������� ������
 
			break;
			}
 
	case 0x38:	//  �������� �� ����. ������� ��� �� ���������
			{
			IIC_Buffer[IIC_index].resume |= i2c_ERR_LP;						// ������ ������ ������ ����������
 
			// ����������� ������� ������. 
			IIC_Buffer[IIC_index].index=0;

			TWCR = 	1<<TWSTA|
				0<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;							// ��� ������ ���� ����� �������� 
			break;										// ��������� �������� �����.
			}
 
	case 0x40: // ������� SLA+R �������� ���. � ������ ����� �������� �����
			{
			if(IIC_Buffer[IIC_index].index+1 == i2c_ByteCount)			// ���� ����� �������� �� ���� �����, �� 
				{
				TWCR = 	0<<TWSTA|
					0<<TWSTO|
					1<<TWINT|
					0<<TWEA|
					1<<TWEN|
					1<<TWIE;						// ������� ����, � � ����� ����� ������ NACK(Disconnect)
				}										// ��� ���� ������ ������, ��� ��� ������ �����. � �� �������� ����
			else
				{
				TWCR = 	0<<TWSTA|
					0<<TWSTO|
					1<<TWINT|
					1<<TWEA|
					1<<TWEN|
					1<<TWIE;						// ��� ������ ������ ���� � ������ ����� ACK
				}
 
			break;
			}
 
	case 0x48: // ������� SLA+R, �� �������� NACK. ������ slave ����� ��� ��� ��� ����. 
			{
			IIC_Buffer[IIC_index].resume |= i2c_ERR_NA;
														// ��� ������ No Answer
			TWCR = 	0<<TWSTA|
				1<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;							// ���� Stop
 
		IIC_done();					// ������������ �������� �������� ������
			break;
			}
 
	case 0x50: // ������� ����.
			{ 
			IIC_Buffer[IIC_index].data[IIC_Buffer[IIC_index].index] = TWDR;				// ������� ��� �� ������
			IIC_Buffer[IIC_index].index++;
 
			// To Do: �������� �������� ������������ ������. � �� ���� �� ��� ���� ���������
 
			if (IIC_Buffer[IIC_index].index+1 == i2c_ByteCount)			// ���� ������� ��� ���� ���� �� ���, ��� �� ������ �������
				{
				TWCR = 	0<<TWSTA|
					0<<TWSTO|
					1<<TWINT|
					0<<TWEA|
					1<<TWEN|
					1<<TWIE;						// ����������� ��� � ����� ������ NACK (Disconnect)
				}
			else
				{
				TWCR = 	0<<TWSTA|
					0<<TWSTO|
					1<<TWINT|
					1<<TWEA|
					1<<TWEN|
					1<<TWIE;						// ���� ���, �� ����������� ��������� ����, � � ����� ������ ���
				}
			break;
			}
 
	case 0x58:	// ��� �� ����� ��������� ����, ������� NACK ����� �������� � �����. 
			{
			IIC_Buffer[IIC_index].data[IIC_Buffer[IIC_index].index] = TWDR;				// ����� ���� � �����
			TWCR = 	0<<TWSTA|
				1<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;							// �������� Stop
 
			IIC_done();					// ���������� ����� ������
 
			break;
			}
 

 
	default:	break;
	}
}

void IIC_done()
{
	SetTimerTask(IIC_Buffer[IIC_index].waiter,1);
	
}



void Add_Task(IIC_Msg Msg)
{
	uint8_t i=0;
	while ((IIC_Buffer[i].resume!=0)&&(i<IIC_MaxBuffer)) i++;
	if (i<IIC_MaxBuffer)
	{
				IIC_Buffer[i].addr=Msg.addr;
				IIC_Buffer[i].count=Msg.count;
				IIC_Buffer[i].index=0;
				IIC_Buffer[i].resume=Msg.resume;
				IIC_Buffer[i].reg_addr=Msg.reg_addr;
				IIC_Buffer[i].waiter=Msg.waiter;
				
	}
}
void DoNothing(void)
{
	
}

void init_I2c()
{
	i2c_PORT|=(1<<i2c_SCL)|(1<<i2c_SDA);
	i2c_DDR&=~((1<<i2c_SCL)|(1<<i2c_SDA));
	
	for (uint8_t i=0;i<IIC_MaxBuffer;i++)
	{
		IIC_Buffer[i].addr=0;
		IIC_Buffer[i].count=0;
		IIC_Buffer[i].index=0;
		IIC_Buffer[i].reg_addr=0;
		IIC_Buffer[i].waiter=&DoNothing;
	}
	
	TWBR=0x80;
	TWSR=0;
}
