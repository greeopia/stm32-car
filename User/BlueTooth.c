#include "stm32f10x.h"                  // Device header
#include "BlueTooth.h"
#include "cJSON.h"
#include "cJSON_Utils.h"
#include "vofa.h"
#include "string.h"
#include "motor.h"

static volatile uint8_t tx_busy = 0, rx_busy = 0;
#define U1_TX_SIZE 128
static uint8_t tx_buf[U1_TX_SIZE];

Frame TxFrame = {
    .fdata = {0.0f},
    .tail = {0x00, 0x00, 0x80, 0x7f}
};

uint8_t RxFrame[2][128];
static uint8_t rx_cmar = 0; // 当前使用的DMA缓冲区索引，0或1

void BlueTooth_init() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7; // RX
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6; // TX
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStruct);	

	DMA_InitTypeDef DMA_InitStruct; 

	DMA_DeInit(DMA1_Channel4); // TX: USART_TC + DMA_TC (justfloat) (DMA普通模式)
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)tx_buf;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStruct.DMA_BufferSize = sizeof(tx_buf);
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // USART_WordLength_8b
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel4, &DMA_InitStruct);

	DMA_DeInit(DMA1_Channel5); // RX: USART_IDLE + DMA_HT/TC -> cJSON/指令 (这里没有连续转运嗷) (DMA循环模式)
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)RxFrame[rx_cmar]; // DMAy_Channelx->CMAR
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStruct.DMA_BufferSize = sizeof(RxFrame[rx_cmar]); // DMAy_Channelx->CNDTR
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // USART_WordLength_8b
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel5, &DMA_InitStruct);
	
    DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	USART_ITConfig(USART1, USART_IT_TC, ENABLE); // TX

	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
	DMA_ITConfig(DMA1_Channel5, DMA_IT_HT | DMA_IT_TC, ENABLE); // RX

	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&NVIC_InitStruct);

	NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&NVIC_InitStruct);

	NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);	

    USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
	DMA_Cmd(DMA1_Channel5, ENABLE); // RX DMA循环模式使能，TX DMA普通模式每次有数据时手动使能控制频率防止解析错误等等。
    DMA_Cmd(DMA1_Channel4, DISABLE); // TX DMA普通模式失能，等待手动使能
	USART_Cmd(USART1, ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7; 
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);    // STATE
}
// TX begin
void USART1_IRQHandler() {
    if (USART_GetITStatus(USART1, USART_IT_TC) == SET) // tx
    {
        /* 清除发送完成标志，避免反复进入中断 */
        USART_ClearITPendingBit(USART1, USART_IT_TC);
    }
    if (USART_GetITStatus(USART1, USART_IT_IDLE) == SET) // rx
    {
        /* 清除空闲中断标志 */
        // USART_ClearITPendingBit(USART1, USART_IT_IDLE); // ？这样好像不太行
        // 读取USART_SR和USART_DR寄存器以清除IDLE中断标志
		uint16_t temp = USART1->SR;
		temp = USART1->DR;
		(void)temp;
    }
}

void DMA1_Channel4_IRQHandler() {
    if (DMA_GetITStatus(DMA1_IT_TC4) != SET) {
        return;
    }
    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_GL4);

    tx_busy = 0;
}

static uint8_t DMA_Send(const void *data, uint16_t len) {
    if (data == NULL || len == 0 || len > U1_TX_SIZE) return 0;
    if (tx_busy) return 0;
    /* 先置忙，再准备和启动发送 */
    tx_busy = 1;

    memcpy(tx_buf, data, len);

    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_GL4);
    DMA_SetCurrDataCounter(DMA1_Channel4, len);

    /* 便于之后查询USART TC，确认线路已发完 */
    USART_ClearFlag(USART1, USART_FLAG_TC);

    /* 确保缓冲区写入完成，再交给DMA */
    __DMB();
    DMA_Cmd(DMA1_Channel4, ENABLE);

    return 1;
}
// TX end

// RX begin
void DMA1_Channel5_IRQHandler() {
    if (DMA_GetITStatus(DMA1_IT_HT5) == SET) {
        // 半传输中断处理
        DMA_ClearFlag(DMA1_FLAG_HT5);
        // 可以在这里处理半传输完成的逻辑



		DMA_Cmd(DMA1_Channel5, DISABLE);
        DMA1_Channel5->CMAR = (uint32_t)RxFrame[rx_cmar ^ 1]; // 切换到另一个缓冲区
        rx_cmar ^= 1; // 切换缓冲区索引
		DMA_Cmd(DMA1_Channel5, ENABLE);
    }
    if (DMA_GetITStatus(DMA1_IT_TC5) == SET) {
        // 传输完成中断处理
        DMA_ClearFlag(DMA1_FLAG_TC5);
        // 可以在这里处理传输完成的逻辑



		DMA_Cmd(DMA1_Channel5, DISABLE);
        DMA1_Channel5->CMAR = (uint32_t)RxFrame[rx_cmar ^ 1]; // 切换到另一个缓冲区
        rx_cmar ^= 1; // 切换缓冲区索引
		DMA_Cmd(DMA1_Channel5, ENABLE);
    }
}

/*

// 不带中断的接收，在while里Tasks外调用

uint8_t rx_buf[128];

uint16_t GetRxBuf(const void* data, uint16_t size) {
	uint16_t count, len;
	if (DMA1_Channel5->CNTTR < 128) {
		count = DMA1_Channel5->CNTTR;

		static uint32_t now = GetTick();
		if (GetTick() - now <= 30) return 0;

		if (DMA1_Channel5->CNTTR == count) {
			len = 128 - DMA1_Channel5->CNTTR;
			// for (uint16_t i = 0; i < len; i++) {

			// }
			memcpy(data, rx_buf, len);
			return len;
		}
	}
	return 0;
}

*/

void BlueToothVofa_Task() {
	float target = GetTarget();
    TxFrame.fdata[0] = speedl;
    TxFrame.fdata[1] = speedr; // 实际速度
    TxFrame.fdata[2] = target; // 左轮正转目标，0~3m/s
    TxFrame.fdata[3] = target; // 右轮使用同一次ADC读数
    TxFrame.fdata[4] = GetDutyl(); // 左轮占空比，0~1
    TxFrame.fdata[5] = GetDutyr(); // 右轮占空比，0~1

    DMA_Send(&TxFrame, sizeof(TxFrame));
}
