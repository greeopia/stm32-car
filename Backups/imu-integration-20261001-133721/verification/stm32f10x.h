#ifndef MOCK_STM32F10X_H
#define MOCK_STM32F10X_H
#include <stdint.h>
#include <string.h>
#include <assert.h>

typedef enum { DISABLE = 0, ENABLE = 1 } FunctionalState;
typedef enum { RESET = 0, SET = 1 } FlagStatus, ITStatus;
typedef enum { ERROR = 0, SUCCESS = 1 } ErrorStatus;
typedef struct { volatile uint32_t CR1, CR2, DR, SR1, SR2; } I2C_TypeDef;
typedef struct { uint32_t CNDTR; } DMA_Channel_TypeDef;
typedef struct { uint32_t GPIO_Mode, GPIO_Pin, GPIO_Speed; } GPIO_InitTypeDef;
typedef struct {
    uint32_t I2C_Mode, I2C_ClockSpeed, I2C_DutyCycle, I2C_Ack;
    uint32_t I2C_AcknowledgedAddress, I2C_OwnAddress1;
} I2C_InitTypeDef;
typedef struct {
    uint32_t DMA_PeripheralBaseAddr, DMA_MemoryBaseAddr, DMA_DIR, DMA_BufferSize;
    uint32_t DMA_PeripheralInc, DMA_MemoryInc, DMA_PeripheralDataSize;
    uint32_t DMA_MemoryDataSize, DMA_Mode, DMA_Priority, DMA_M2M;
} DMA_InitTypeDef;
typedef struct {
    uint32_t NVIC_IRQChannel, NVIC_IRQChannelPreemptionPriority;
    uint32_t NVIC_IRQChannelSubPriority, NVIC_IRQChannelCmd;
} NVIC_InitTypeDef;
static I2C_TypeDef mock_i2c, mock_i2c2;
static DMA_Channel_TypeDef mock_dma;
#define I2C1 (&mock_i2c)
#define I2C2 (&mock_i2c2)
#define DMA1_Channel7 (&mock_dma)
#define GPIOB ((void *)0)
enum { I2C1_EV_IRQn=31, I2C1_ER_IRQn=32, DMA1_Channel7_IRQn=17 };
enum { GPIO_Mode_AF_OD=0x1C, GPIO_Speed_50MHz=3 };
#include "mock_constants.h"

static uint32_t mock_primask, mock_i2c_it, mock_dma_pending;
static unsigned mock_start_count, mock_stop_count, mock_flag_reads;
static unsigned mock_address_count, mock_data_count;
static uint8_t mock_addresses[64], mock_directions[64], mock_data[64];
static uint8_t mock_id = 0x68;
static int mock_stall_start, mock_defer_stop, mock_dma_enabled, mock_dma_request, mock_last;
static DMA_InitTypeDef mock_dma_config;
static GPIO_InitTypeDef mock_gpio_config;
static I2C_InitTypeDef mock_i2c_config;

static inline uint32_t __get_PRIMASK(void) { return mock_primask; }
static inline void __disable_irq(void) { mock_primask = 1; }
static inline void __set_PRIMASK(uint32_t x) { mock_primask = x; }
static inline void __DMB(void) {}
static inline void NVIC_DisableIRQ(unsigned x) { (void)x; }
static inline void NVIC_ClearPendingIRQ(unsigned x) { (void)x; }
static inline void NVIC_Init(NVIC_InitTypeDef *x) { (void)x; }
static inline void RCC_APB2PeriphClockCmd(uint32_t x, FunctionalState s) { (void)x; (void)s; }
static inline void RCC_APB1PeriphClockCmd(uint32_t x, FunctionalState s) { (void)x; (void)s; }
static inline void RCC_AHBPeriphClockCmd(uint32_t x, FunctionalState s) { (void)x; (void)s; }
static inline void GPIO_PinRemapConfig(uint32_t x, FunctionalState s) { (void)x; (void)s; }
static inline void GPIO_StructInit(GPIO_InitTypeDef *x) { memset(x,0,sizeof(*x)); }
static inline void GPIO_Init(void *p, GPIO_InitTypeDef *x) { (void)p; mock_gpio_config=*x; }
static inline void I2C_DeInit(I2C_TypeDef *p) { memset(p,0,sizeof(*p)); mock_i2c_it=0; mock_dma_request=0; mock_last=0; }
static inline void I2C_StructInit(I2C_InitTypeDef *x) { memset(x,0,sizeof(*x)); }
static inline void I2C_Init(I2C_TypeDef *p, I2C_InitTypeDef *x) { (void)p; mock_i2c_config=*x; }
static inline void I2C_Cmd(I2C_TypeDef *p, FunctionalState s) { (void)p; (void)s; }
static inline void I2C_ITConfig(I2C_TypeDef *p, uint16_t bits, FunctionalState s) {
    (void)p; if(s==ENABLE) mock_i2c_it|=bits; else mock_i2c_it&=~bits;
}
static inline void I2C_DMACmd(I2C_TypeDef *p, FunctionalState s) {
    (void)p;
    if(s==ENABLE) { assert(mock_dma_enabled && mock_last); assert(mock_dma.CNDTR==14); }
    mock_dma_request=(s==ENABLE);
}
static inline void I2C_DMALastTransferCmd(I2C_TypeDef *p, FunctionalState s) { (void)p; mock_last=(s==ENABLE); }
static inline void I2C_AcknowledgeConfig(I2C_TypeDef *p, FunctionalState s) {
    if(s==ENABLE) p->CR1|=I2C_CR1_ACK; else p->CR1&=~I2C_CR1_ACK;
}
static inline void I2C_GenerateSTART(I2C_TypeDef *p, FunctionalState s) {
    assert(s==ENABLE); ++mock_start_count; p->SR1=mock_stall_start?0:I2C_SR1_SB;
    p->SR2=I2C_SR2_BUSY|I2C_SR2_MSL;
}
static inline void I2C_GenerateSTOP(I2C_TypeDef *p, FunctionalState s) {
    assert(s==ENABLE); ++mock_stop_count; p->CR1|=I2C_CR1_STOP;
    if(!mock_defer_stop) { p->CR1&=~I2C_CR1_STOP; p->SR2=0; }
}
static inline void I2C_Send7bitAddress(I2C_TypeDef *p,uint8_t addr,uint8_t dir) {
    assert(mock_address_count<64); mock_addresses[mock_address_count]=addr;
    mock_directions[mock_address_count++]=dir; p->SR1=I2C_SR1_ADDR;
    if(dir==I2C_Direction_Receiver) { p->SR1|=I2C_SR1_RXNE; p->DR=mock_id; }
}
static inline void I2C_SendData(I2C_TypeDef *p,uint8_t data) {
    assert(mock_data_count<64); mock_data[mock_data_count++]=data;
    p->SR1=I2C_SR1_BTF; p->DR=data;
}
static inline uint8_t I2C_ReceiveData(I2C_TypeDef *p) { p->SR1&=~I2C_SR1_RXNE; return (uint8_t)p->DR; }
static inline FlagStatus I2C_GetFlagStatus(I2C_TypeDef *p,uint32_t f) {
    ++mock_flag_reads; return (p->SR1 & (f&0xFFFF))?SET:RESET;
}
static inline void I2C_ClearFlag(I2C_TypeDef *p,uint32_t f) { p->SR1 &= ~(f&0xFFFF); }
static inline void DMA_DeInit(DMA_Channel_TypeDef *p) { p->CNDTR=0; mock_dma_enabled=0; }
static inline void DMA_StructInit(DMA_InitTypeDef *x) { memset(x,0,sizeof(*x)); }
static inline void DMA_Init(DMA_Channel_TypeDef *p,DMA_InitTypeDef *x) { mock_dma_config=*x; p->CNDTR=x->DMA_BufferSize; }
static inline void DMA_ITConfig(DMA_Channel_TypeDef *p,uint32_t b,FunctionalState s) { (void)p; (void)b; (void)s; }
static inline void DMA_Cmd(DMA_Channel_TypeDef *p,FunctionalState s) { assert(p==DMA1_Channel7); mock_dma_enabled=(s==ENABLE); }
static inline void DMA_ClearFlag(uint32_t f) { assert(f==DMA1_FLAG_GL7); mock_dma_pending=0; }
static inline void DMA_SetCurrDataCounter(DMA_Channel_TypeDef *p,uint16_t n) { assert(!mock_dma_enabled); p->CNDTR=n; }
static inline ITStatus DMA_GetITStatus(uint32_t f) { return (mock_dma_pending&f)?SET:RESET; }
#endif
