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

volatile uint8_t i2c_Do;						// Переменная состояния передатчика IIC
					// Индекс буфера Slave

IIC_Msg IIC_Buffer[IIC_MaxBuffer];

uint8_t IIC_index;


uint8_t i2c_Buffer[i2c_MaxBuffer];				// Буфер для данных работы в режиме Master
uint8_t i2c_index;								// Индекс этого буфера
uint8_t i2c_ByteCount;							// Число байт передаваемых

uint8_t i2c_SlaveAddress;						// Адрес подчиненного

uint8_t i2c_PageAddress[i2c_MaxPageAddrLgth];	// Буфер адреса страниц (для режима с sawsarp)
uint8_t i2c_PageAddrIndex;						// Индекс буфера адреса страниц
uint8_t i2c_PageAddrCount;						// Число байт в адресе страницы для текущего Slave


ISR(TWI_vect)								// Прерывание TWI Тут наше все.
{
	PORTG^=1;
switch(TWSR & 0xF8)											// Отсекаем биты прескалера
	{
	case 0x00:	// Bus Fail (автобус сломался)
			{
			IIC_Buffer[IIC_index].resume |= i2c_ERR_BF;
 
			TWCR = 	0<<TWSTA|
				1<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;  	// Go!
 
			break;
			}
 
	case 0x08:	// Старт был, а затем мы:
			{
			if( (IIC_Buffer[IIC_index].resume & i2c_type_msk)== i2c_sarp)			// В зависимости от режима
				{
				IIC_Buffer[IIC_index].addr |= 0x01;					// Шлем Addr+R
				}
			else											// Или 
				{
				IIC_Buffer[IIC_index].addr &= 0xFE;					// Шлем Addr+W
				}
 
			TWDR = IIC_Buffer[IIC_index].addr;				// Адрес слейва
			TWCR = 	0<<TWSTA|
				0<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;  								// Go!
			break;
			}
 
	case 0x10:	// Повторный старт был, а затем мы
			{
			if( (IIC_Buffer[IIC_index].resume & i2c_type_msk) == i2c_sawsarp)		// В зависимости от режима
				{
				IIC_Buffer[IIC_index].addr |= 0x01;					// Шлем Addr+R
				}
			else
				{
				IIC_Buffer[IIC_index].addr &= 0xFE;					// Шлем Addr+W
				}
 
			// To Do: Добавить сюда обработку ошибок 
 
			TWDR = IIC_Buffer[IIC_index].addr;				// Адрес слейва
			TWCR = 	0<<TWSTA|
				0<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;  	// Go!
			break;
			}
 
	case 0x18:	// Был послан SLA+W получили ACK, а затем:
			{

			if( (IIC_Buffer[IIC_index].resume & i2c_type_msk) == i2c_sawp)		// В зависимости от режима
				{
				TWDR = IIC_Buffer[IIC_index].data[IIC_Buffer[IIC_index].index];				// Шлем байт данных
				IIC_Buffer[IIC_index].index++;							// Увеличиваем указатель буфера
 
				TWCR = 	0<<TWSTA|
					0<<TWSTO|
					1<<TWINT|
					1<<TWEN|
					1<<TWIE;  // Go! 
 
				}
 
			if( (IIC_Buffer[IIC_index].resume & i2c_type_msk) == i2c_sawsarp)
				{
				TWDR = IIC_Buffer[IIC_index].reg_addr;	// Или шлем адрес странцы (по сути тоже байт данных)
														// Увеличиваем указатель буфера страницы
				TWCR = 	0<<TWSTA|
					0<<TWSTO|
					1<<TWINT|
					1<<TWEN|
					1<<TWIE;	// Go!
				}
			}
			break;
 
	case 0x20:	// Был послан SLA+W получили NACK - слейв либо занят, либо его нет дома.
			{
			IIC_Buffer[IIC_index].resume |= i2c_ERR_NA;	
															// Код ошибки
			TWCR = 	0<<TWSTA|
				1<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;								// Шлем шине Stop
 
			IIC_done();						// Обрабатываем событие ошибки;
			break;
			}
 
	case 0x28: 	// Байт данных послали, получили ACK!  (если sawp - это был байт данных. если sawsarp - байт адреса страницы)
			{	// А дальше: 
			if( (IIC_Buffer[IIC_index].resume & i2c_type_msk) == i2c_sawp)		// В зависимости от режима
				{
				if (IIC_Buffer[IIC_index].index == IIC_Buffer[IIC_index].count)				// Если был байт данных последний
					{																		
					TWCR = 	0<<TWSTA|
						1<<TWSTO|
						1<<TWINT|
						1<<TWEN|
						1<<TWIE;						// Шлем Stop
 
					IIC_done();				// И выходим в обработку стопа
 
					}
				else
					{
					TWDR = IIC_Buffer[IIC_index].data[IIC_Buffer[IIC_index].index];			// Либо шлем еще один байт
					IIC_Buffer[IIC_index].index++;
					TWCR = 	0<<TWSTA|						
						0<<TWSTO|
						1<<TWINT|
						1<<TWEN|
						1<<TWIE;  						// Go!
					}
				}
 
			if( (IIC_Buffer[IIC_index].resume & i2c_type_msk) == i2c_sawsarp)		// В другом режиме мы
				{
					TWCR = 	1<<TWSTA|
						0<<TWSTO|
						1<<TWINT|
						1<<TWEN|
						1<<TWIE;						// Запускаем Повторный старт!

				}	 
			}
			break;
 
	case 0x30:	//Байт ушел, но получили NACK причин две. 1я передача оборвана слейвом и так надо. 2я слейв сглючил.
			{
			IIC_Buffer[IIC_index].resume |= i2c_ERR_NK;						// Запишем статус ошибки. Хотя это не факт, что ошибка. 
 
			TWCR = 	0<<TWSTA|
				1<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;							// Шлем Stop
 
			IIC_done();					// Отрабатываем событие выхода
 
			break;
			}
 
	case 0x38:	//  Коллизия на шине. Нашелся кто то поглавней
			{
			IIC_Buffer[IIC_index].resume |= i2c_ERR_LP;						// Ставим ошибку потери приоритета
 
			// Настраиваем индексы заново. 
			IIC_Buffer[IIC_index].index=0;

			TWCR = 	1<<TWSTA|
				0<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;							// Как только шина будет свободна 
			break;										// попробуем передать снова.
			}
 
	case 0x40: // Послали SLA+R получили АСК. А теперь будем получать байты
			{
			if(IIC_Buffer[IIC_index].index+1 == i2c_ByteCount)			// Если буфер кончится на этом байте, то 
				{
				TWCR = 	0<<TWSTA|
					0<<TWSTO|
					1<<TWINT|
					0<<TWEA|
					1<<TWEN|
					1<<TWIE;						// Требуем байт, а в ответ потом пошлем NACK(Disconnect)
				}										// Что даст понять слейву, что мол хватит гнать. И он отпустит шину
			else
				{
				TWCR = 	0<<TWSTA|
					0<<TWSTO|
					1<<TWINT|
					1<<TWEA|
					1<<TWEN|
					1<<TWIE;						// Или просто примем байт и скажем потом ACK
				}
 
			break;
			}
 
	case 0x48: // Послали SLA+R, но получили NACK. Видать slave занят или его нет дома. 
			{
			IIC_Buffer[IIC_index].resume |= i2c_ERR_NA;
														// Код ошибки No Answer
			TWCR = 	0<<TWSTA|
				1<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;							// Шлем Stop
 
		IIC_done();					// Отрабатываем выходную ситуацию ошибки
			break;
			}
 
	case 0x50: // Приняли байт.
			{ 
			IIC_Buffer[IIC_index].data[IIC_Buffer[IIC_index].index] = TWDR;				// Забрали его из буфера
			IIC_Buffer[IIC_index].index++;
 
			// To Do: Добавить проверку переполнения буфера. А то мало ли что юзер затребует
 
			if (IIC_Buffer[IIC_index].index+1 == i2c_ByteCount)			// Если остался еще один байт из тех, что мы хотели считать
				{
				TWCR = 	0<<TWSTA|
					0<<TWSTO|
					1<<TWINT|
					0<<TWEA|
					1<<TWEN|
					1<<TWIE;						// Затребываем его и потом пошлем NACK (Disconnect)
				}
			else
				{
				TWCR = 	0<<TWSTA|
					0<<TWSTO|
					1<<TWINT|
					1<<TWEA|
					1<<TWEN|
					1<<TWIE;						// Если нет, то затребываем следующий байт, а в ответ скажем АСК
				}
			break;
			}
 
	case 0x58:	// Вот мы взяли последний байт, сказали NACK слейв обиделся и отпал. 
			{
			IIC_Buffer[IIC_index].data[IIC_Buffer[IIC_index].index] = TWDR;				// Взяли байт в буфер
			TWCR = 	0<<TWSTA|
				1<<TWSTO|
				1<<TWINT|
				1<<TWEN|
				1<<TWIE;							// Передали Stop
 
			IIC_done();					// Отработали точку выхода
 
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
