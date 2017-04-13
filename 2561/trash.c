/*
 * trash.c
 *
 * Created: 20.03.2017 12:40:36
 *  Author: malish
 */ 




//struct Message_Stack
//{
	//uint8_t Status[SPIMessagesCount]; // 0 - пусто , 1 - ожидает отправки, 2 - отправлено, 3 - получено и обработано
	//uint8_t OutputMessage[SPIMessagesCount];
	//uint8_t InputMessage[SPIMessagesCount];
	//uint8_t DelayAfterSend[SPIMessagesCount];
	//Process ProcessNeedsAnswer[SPIMessagesCount];
	//uint8_t Receiver[SPIMessagesCount];
	//uint8_t CS_State [SPIMessagesCount];
	//uint8_t size;
	//uint8_t top;
//};
//
//typedef struct Message_Stack Message_Stack;

//Message_Stack SPI_Stack;

/*
void SPI_Send_Ready()
{
    uint8_t k=0;
	
	while ((SPI_Stack.Status[k]!=2)&&(k<SPIMessagesCount)) 
	{k++;}
		
	if (k==SPIMessagesCount) {return;} //FUCK
	
	SPI_Stack.InputMessage[k]=SPDR;
	SPI_Stack.Status[k]=3;
	if (SPI_Stack.ProcessNeedsAnswer[k]!=NULL)
	{
		(SPI_Stack.ProcessNeedsAnswer[k])();	
	}
	
	
	if (SPI_Stack.CS_State[k]==0)
	{
		DDR_CS ^= (1<<SPI_Stack.Receiver[k]);	
	}
	
	SPI_Stack.size--;
	
	while (k<SPI_Stack.size)
	{

		SPI_Stack.CS_State[k]=SPI_Stack.CS_State[k+1];
		SPI_Stack.DelayAfterSend[k]=SPI_Stack.DelayAfterSend[k+1];
		SPI_Stack.InputMessage[k]=SPI_Stack.InputMessage[k+1];
		SPI_Stack.OutputMessage[k]=SPI_Stack.OutputMessage[k+1];
		SPI_Stack.ProcessNeedsAnswer[k]=SPI_Stack.ProcessNeedsAnswer[k+1];
		SPI_Stack.Receiver[k]=SPI_Stack.Receiver[k+1];
		SPI_Stack.Status[k]=SPI_Stack.Status[k+1];
	
	}
	
	if (k<SPI_Stack.size)
	{
		PORT_CS |= (1<<SPI_Stack.Receiver[k]);
		
		SPI_Stack.Status[k]=2;
		SPDR=SPI_Stack.OutputMessage[k];		
	}
	else
	{
		SPCR &=(~(1<<SPIE));
	}
	
}
void SPI_add_Stack (uint8_t Receiver, uint8_t data , uint8_t CS_state, uint8_t delay, Process process)
{
	cli();
	if (SPI_Stack.size==SPIMessagesCount) return;
	
	SPI_Stack.Status[SPI_Stack.size]=1;
	SPI_Stack.Receiver[SPI_Stack.size]=Receiver;
	SPI_Stack.OutputMessage[SPI_Stack.size]=data;
	SPI_Stack.DelayAfterSend[SPI_Stack.size]=delay;
	SPI_Stack.CS_State[SPI_Stack.size]=CS_state;
	SPI_Stack.ProcessNeedsAnswer[SPI_Stack.size]=process;
	SPI_Stack.size++;
	
	if (!(SPCR&(1<<SPIE)))
	{
		
		SPI_Stack.Status[0]=2;
		PORT_CS |= (1<<SPI_Stack.Receiver[0]);
		SPDR=SPI_Stack.OutputMessage[0];
		SPCR |=(1<<SPIE);
		
	}
	
	sei();
}


void mcp2515_request_register (uint8_t address)
{
	SPI_add_Stack(P_CS,SPI_READ,1,0,NULL);
	
	
}

void mcp2515_get_received_register (uint8_t data)
{
	char result [30];
	sprintf(result,"Rv: %d",data);
	UART_Add_Message(result);
	
}
*/

