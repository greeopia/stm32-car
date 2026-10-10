// #include "stm32f10x.h"                  // Device header
// #include "BlueTooth.h"
// #include "cJSON.h"
// #include "cJSON_Utils.h"
// #include "vofa.h"

// void BlueTooth_init() {
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);
// 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
// 	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
	
// 	GPIO_InitTypeDef GPIO_InitStruct;
// 	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
// 	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7; // RX
// 	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOB, &GPIO_InitStruct);
// 	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
// 	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6; // TX
// 	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
// 	USART_InitTypeDef USART_InitStruct;
// 	USART_InitStruct.USART_BaudRate = 115200;
// 	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
// 	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
// 	USART_InitStruct.USART_Parity = USART_Parity_No;
// 	USART_InitStruct.USART_StopBits = USART_StopBits_1;
// 	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
// 	USART_Init(USART1, &USART_InitStruct);	

// 	DMA_InitTypeDef DMA_InitStruct; 

// 	DMA_DeInit(DMA1_Channel4); // TX: USART_TC + DMA_TC (justfloat) (DMA普通模式)
// 	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
// 	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)&TxFrame;
// 	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
// 	DMA_InitStruct.DMA_BufferSize = sizeof(TxFrame);
// 	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
// 	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
// 	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // USART_WordLength_8b
// 	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
// 	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
// 	DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
// 	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
// 	DMA_Init(DMA1_Channel4, &DMA_InitStruct);
// 	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
// 	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);

// 	DMA_DeInit(DMA1_Channel5); // RX: USART_IDLE + DMA_HT/TC -> cJSON/指令 (这里没有连续转运嗷) (DMA循环模式)
// 	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
// 	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)RxFrame[0]; // DMAy_Channelx->CMAR
// 	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
// 	DMA_InitStruct.DMA_BufferSize = sizeof(RxFrame[0]); // DMAy_Channelx->CNDTR
// 	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
// 	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Disable;
// 	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // USART_WordLength_8b
// 	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
// 	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
// 	DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh;
// 	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
// 	DMA_Init(DMA1_Channel5, &DMA_InitStruct);
// 	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
// 	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
// 	DMA_ITConfig(DMA1_Channel5, DMA_IT_HT | DMA_IT_TC, ENABLE);
	
// 	USART_ITConfig(USART1, USART_IT_TC, DISABLE);
// 	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);

// 	NVIC_InitTypeDef NVIC_InitStruct;
// 	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
// 	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
// 	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
// 	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
// 	NVIC_Init(&NVIC_InitStruct);

// 	NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel4_IRQn;
// 	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
// 	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
// 	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
// 	NVIC_Init(&NVIC_InitStruct);

// 	NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel5_IRQn;
// 	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
// 	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
// 	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
// 	NVIC_Init(&NVIC_InitStruct);	

// 	DMA_Cmd(DMA1_Channel5, ENABLE); // RX DMA循环模式使能，TX DMA普通模式每次有数据时手动使能控制频率防止解析错误等等。
// 	USART_Cmd(USART1, ENABLE);
// }

// void USART1_IRQHandler() {
// 	if (USART_GetITStatus(USART1, USART_IT_IDLE) == SET) { // 接收
		
// 	}
	
// }

// void DMA1_Channel4_IRQHandler() { // DMA TC发送
	
// }

// void DMA1_Channel5_IRQHandler() { // DMA HT/TC接收
		
// }
