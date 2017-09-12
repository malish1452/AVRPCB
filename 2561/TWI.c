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

volatile uint8_t IIC_resume;						// ���������� ��������� ����������� IIC
					// ������ ������ Slave

IIC_Msg IIC_Buffer[IIC_MaxBuffer];

uint8_t IIC_index;




void DoNothing(void)
{
	
}
ISR(TWI_vect)								// ���������� TWI ��� ���� ���.
{
//	PORTG^=1;
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
			IIC_done();
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
				IIC_Buffer[IIC_index].addr &= 0xFE;	    	// ���� Addr+W
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
			IIC_Buffer[IIC_index].resume |= i2c_ERR_NA;	// ��� ������
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
			if(IIC_Buffer[IIC_index].index+1 == IIC_Buffer[IIC_index].count)			// ���� ����� �������� �� ���� �����, �� 
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
 
			if (IIC_Buffer[IIC_index].index+1 == IIC_Buffer[IIC_index].count)			// ���� ������� ��� ���� ���� �� ���, ��� �� ������ �������
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
	cli();
	
	if ((IIC_Buffer[IIC_index].resume&i2c_ERR_BF)==1)
	{
		//PORTG|=1;
		IIC_Buffer[IIC_index].resume&=~i2c_ERR_BF;	
		IIC_Buffer[IIC_index].index=0;
		TWCR = 	1<<TWSTA|0<<TWSTO|1<<TWINT|1<<TWEN|1<<TWIE;
	}
	else
	{	PORTC^=1;
		IIC_Buffer[IIC_index].state=2;
		SetTimerTask(IIC_Buffer[IIC_index].waiter,1);
		for (int k=0;k<IIC_MaxBuffer;k++)
		{
			while (IIC_Buffer[k].state==3)
			{
				for (int i=k;i<IIC_MaxBuffer-1;i++)
				{
					
					IIC_Buffer[i].addr=IIC_Buffer[i+1].addr;
					IIC_Buffer[i].count=IIC_Buffer[i+1].count;
					for (int t=0;t<8;t++)
					{
						IIC_Buffer[i].data[t]=IIC_Buffer[i+1].data[t];
					}
					IIC_Buffer[i].index=IIC_Buffer[i+1].index;
					IIC_Buffer[i].reg_addr=IIC_Buffer[i+1].reg_addr;
					IIC_Buffer[i].resume=IIC_Buffer[i+1].resume;
					IIC_Buffer[i].state=IIC_Buffer[i+1].state;
					IIC_Buffer[i].waiter=IIC_Buffer[i+1].waiter;
					if (i==IIC_MaxBuffer-1)
					{
								IIC_Buffer[IIC_MaxBuffer].addr=0;
								IIC_Buffer[IIC_MaxBuffer].count=0;
								IIC_Buffer[IIC_MaxBuffer].index=0;
								IIC_Buffer[IIC_MaxBuffer].reg_addr=0;
								IIC_Buffer[IIC_MaxBuffer].waiter=&DoNothing;
								IIC_Buffer[IIC_MaxBuffer].state=0;
					}
				}	
			}
		}
		int i=0;
		while ((IIC_Buffer[i].state!=1)&&(i<IIC_MaxBuffer)) i++;
		if (i<IIC_MaxBuffer)  //����� ��� ��������� ������
		{
			IIC_index=i;
			TWCR = 	1<<TWSTA|
			0<<TWSTO|
			1<<TWINT|
			1<<TWEN|
			1<<TWIE;
		}
		else
		{
			IIC_resume=0; // ��� ���������� �����������
		}
	}
	sei();
}



void Add_Task(IIC_Msg Msg)
{
	cli();
	
	uint8_t i=0;
	while ((IIC_Buffer[i].state!=0)&&(i<IIC_MaxBuffer)) i++;
	if (i<IIC_MaxBuffer)
	{
				IIC_Buffer[i].addr=Msg.addr;
				IIC_Buffer[i].count=Msg.count;
				IIC_Buffer[i].index=0;
				IIC_Buffer[i].resume=Msg.resume;
				IIC_Buffer[i].reg_addr=Msg.reg_addr;
				IIC_Buffer[i].waiter=Msg.waiter;
				IIC_Buffer[i].state=1;
				for (int k=0;k<8;k++)
				{
					IIC_Buffer[i].data[k]=Msg.data[k];
				}
				
		if (IIC_resume==0)
		{	
			IIC_resume=1;
			IIC_index=i;
			TWCR = 	1<<TWSTA|
			0<<TWSTO|
			1<<TWINT|
			1<<TWEN|
			1<<TWIE;
		}
				
	}
	sei();
}

IIC_Msg Get_Msg(IIC_F waiter)
{
	IIC_Msg result;
	uint8_t i=0;
	while ((IIC_Buffer[i].waiter!=waiter)&&(i<IIC_MaxBuffer))
	{
		i++;
	}
	
	if (i==IIC_MaxBuffer)
	{
		PORTG|=(1<<1);
	}
	
	result.count=IIC_Buffer[i].count;
	for (int t=0;t<8;t++)
		{
			result.data[t]=IIC_Buffer[i].data[t];
		}
	result.resume=IIC_Buffer[i].resume;
	result.number=IIC_Buffer[i].number;
	return result;

}

void Delete_MSG(uint8_t index)
{
	IIC_Buffer[index].state=3;
	
}

void init_I2c()
{
	i2c_PORT|=(1<<i2c_SCL)|(1<<i2c_SDA);
	i2c_DDR&=~((1<<i2c_SCL)|(1<<i2c_SDA));
	IIC_index=0;
	
	for (uint8_t i=0;i<IIC_MaxBuffer;i++)
	{
		
		IIC_Buffer[i].addr=0;
		IIC_Buffer[i].count=0;
		IIC_Buffer[i].index=0;
		IIC_Buffer[i].reg_addr=0;
		IIC_Buffer[i].waiter=&DoNothing;
		IIC_Buffer[i].state=0;
		IIC_Buffer[i].number=i;
	}
	IIC_resume=0;
	TWBR=0x80;
	TWSR=0;
}
