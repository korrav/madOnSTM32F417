#include "main.h"

#define SPI1_ADDR 0x4001300C
#define SPI2_ADDR 0x4000380C
//private variable



void SPI_Config(void);
void initAnalog(void);

int main(void) {
	int temp = 5;
	//configures the priority grouping
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	//start time
	start_timer();
	//configure spi
	SPI_Config();
	//ETHERNET INITIALIZATION
  ETH_BSP_Config();
	/* Initilaize the LwIP stack */
  LwIP_Init();
	//udp initialization
	if (!udp_echoserver_init())
		return 1;
  while (1)
  {  
		//PROCESSING OF PEREODIC TIMERS FOR LWIP
    LwIP_Periodic_Handle(gettime());
		//PROCESSING OF INCOMING PACKET
    if (ETH_CheckFrameReceived())
    { 
      /* process received ethernet packet */
      LwIP_Pkt_Handle();
    }
		trans_Control((void*)&temp, sizeof(temp));
  }   
}

void SPI_Config(void) {
	GPIO_InitTypeDef structGPIO;
	SPI_InitTypeDef structSPI;
	DMA_InitTypeDef structDMA;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);
	//configure GPIO
	structGPIO.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6; //spi1
  structGPIO.GPIO_Mode = GPIO_Mode_AF;
	structGPIO.GPIO_Speed = GPIO_Speed_50MHz;
	structGPIO.GPIO_OType = GPIO_OType_PP;
	structGPIO.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &structGPIO);	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	structGPIO.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; //spi2
	GPIO_Init(GPIOB, &structGPIO);	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);
	//init spi 
	SPI_StructInit(&structSPI);
	structSPI.SPI_Direction = SPI_Direction_1Line_Rx;
	structSPI.SPI_Mode = SPI_Mode_Slave;
	structSPI.SPI_DataSize = SPI_DataSize_16b;
	SPI_Init(SPI1, &structSPI);
	SPI_Init(SPI2, &structSPI);
	SPI_Cmd(SPI1, ENABLE);
	SPI_Cmd(SPI2, ENABLE);
	//init DMA
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);
	structDMA.DMA_Channel = DMA_Channel_3;
	structDMA.DMA_PeripheralBaseAddr = SPI1_ADDR;
	structDMA.DMA_Memory0BaseAddr = (uint32_t)getAnalogBuf(0);
	structDMA.DMA_DIR = DMA_DIR_PeripheralToMemory;
	structDMA.DMA_BufferSize = SIZE_BUF * 2;
	structDMA.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	structDMA.DMA_MemoryInc = DMA_MemoryInc_Enable;
	structDMA.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	structDMA.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	structDMA.DMA_Mode = DMA_Mode_Circular;
	structDMA.DMA_Priority = DMA_Priority_VeryHigh;
	structDMA.DMA_FIFOMode = DMA_FIFOMode_Enable;
	structDMA.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	structDMA.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &structDMA);
	structDMA.DMA_Channel = DMA_Channel_0;
	structDMA.DMA_PeripheralBaseAddr = SPI2_ADDR;
	structDMA.DMA_Memory0BaseAddr = (uint32_t)getAnalogBuf(1);
	DMA_Init(DMA1_Stream3, &structDMA);
	//init DMA interrupt
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	//enable DMA
	DMA_Cmd(DMA2_Stream0, ENABLE);
	DMA_Cmd(DMA1_Stream3, ENABLE);
	//initialize the analog part
	initAnalog();
}

void initAnalog(void) {
	void initialAnalogHandler(void);
	DMA_SetCurrDataCounter(DMA2_Stream0, SIZE_BUF);
	DMA_SetCurrDataCounter(DMA2_Stream3, SIZE_BUF);
}
