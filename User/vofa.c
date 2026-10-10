#include "vofa.h"
#include "BlueTooth.h"
#include "stm32f10x.h"                  // Device header
#include "string.h"

#include "motor.h"

static volatile uint8_t tx_busy = 0;
#define U2_TX_SIZE 128
static uint8_t tx_buf[U2_TX_SIZE];

Frame TxFrame = {
    .fdata = {0.0f},
    .tail = {0x00, 0x00, 0x80, 0x7f}
};

uint8_t RxFrame[2][128];

extern float speedl, speedr;
void vofa_init() {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2; // TX
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3; // RX
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

	USART_InitTypeDef USART_InitStruct; // 应该只是有线串口发送vofa数据
	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &USART_InitStruct);

    DMA_DeInit(DMA1_Channel7);
    DMA_InitTypeDef DMA_InitStruct;
    DMA_InitStruct.DMA_BufferSize = sizeof(tx_buf);
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
    // DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)&TxFrame;
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)&tx_buf;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_Init(DMA1_Channel7, &DMA_InitStruct);

    USART_ITConfig(USART2, USART_IT_TC, ENABLE); // 发送用TC中断，DMA发送完毕后触发
    // USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
    // USART_ITConfig(USART2, USART_IT_IDLE, ENABLE); // 接收用IDLE中断，DMA接收完毕后触发
    DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct; // 分别配置DMATC和USARTTC中断
    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel7_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_Init(&NVIC_InitStruct);

    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
    DMA_Cmd(DMA1_Channel7, DISABLE);
    USART_Cmd(USART2, ENABLE);
}

// void DMA1_Channel7_IRQHandler() {

// }

void USART2_IRQHandler() {
    if (USART_GetITStatus(USART2, USART_IT_TC) == SET)
    {
        /* 清除发送完成标志，避免反复进入中断 */
        USART_ClearITPendingBit(USART2, USART_IT_TC);
    }
}

void DMA1_Channel7_IRQHandler() {
    if (DMA_GetITStatus(DMA1_IT_TC7) != SET) {
        return;
    }
    DMA_Cmd(DMA1_Channel7, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_GL7);

    tx_busy = 0;
}

static uint8_t DMA_Send(const void *data, uint16_t len) {
    if (data == NULL || len == 0 || len > U2_TX_SIZE) return 0;

    if (tx_busy) return 0;

    /* 先置忙，再准备和启动发送 */
    tx_busy = 1;

    memcpy(tx_buf, data, len);

    DMA_Cmd(DMA1_Channel7, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_GL7);
    DMA_SetCurrDataCounter(DMA1_Channel7, len);

    /* 便于之后查询USART TC，确认线路已发完 */
    USART_ClearFlag(USART2, USART_FLAG_TC);

    /* 确保缓冲区写入完成，再交给DMA */
    __DMB();
    DMA_Cmd(DMA1_Channel7, ENABLE);

    return 1;
}

void SerialVofa_Task() {
	float target = GetTarget();
    TxFrame.fdata[0] = speedl;
    TxFrame.fdata[1] = speedr; // 实际速度
    TxFrame.fdata[2] = target; // 左轮正转目标，0~3m/s
    TxFrame.fdata[3] = target; // 右轮使用同一次ADC读数
    TxFrame.fdata[4] = GetDutyl(); // 左轮占空比，0~1
    TxFrame.fdata[5] = GetDutyr(); // 右轮占空比，0~1

    DMA_Send(&TxFrame, sizeof(TxFrame));
}
