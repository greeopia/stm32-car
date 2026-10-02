#define DMA_DIR_PeripheralSRC ((uint32_t)0x00000000)
#define DMA_IT_TC ((uint32_t)0x00000002)
#define DMA_IT_TE ((uint32_t)0x00000008)
#define DMA_M2M_Disable ((uint32_t)0x00000000)
#define DMA_MemoryDataSize_Byte ((uint32_t)0x00000000)
#define DMA_MemoryInc_Enable ((uint32_t)0x00000080)
#define DMA_Mode_Normal ((uint32_t)0x00000000)
#define DMA_PeripheralDataSize_Byte ((uint32_t)0x00000000)
#define DMA_PeripheralInc_Disable ((uint32_t)0x00000000)
#define DMA_Priority_VeryHigh ((uint32_t)0x00003000)
#define DMA1_FLAG_GL7 ((uint32_t)0x01000000)
#define DMA1_IT_TC7 ((uint32_t)0x02000000)
#define DMA1_IT_TE7 ((uint32_t)0x08000000)
#define GPIO_Pin_8 ((uint16_t)0x0100)  /*!< Pin 8 selected */
#define GPIO_Pin_9 ((uint16_t)0x0200)  /*!< Pin 9 selected */
#define GPIO_Remap_I2C1 ((uint32_t)0x00000002)  /*!< I2C1 Alternate Function mapping */
#define I2C_Ack_Enable ((uint16_t)0x0400)
#define I2C_AcknowledgedAddress_7bit ((uint16_t)0x4000)
#define I2C_CR1_ACK ((uint16_t)0x0400)            /*!< Acknowledge Enable */
#define I2C_CR1_STOP ((uint16_t)0x0200)            /*!< Stop Generation */
#define I2C_Direction_Receiver ((uint8_t)0x01)
#define I2C_Direction_Transmitter ((uint8_t)0x00)
#define I2C_DutyCycle_2 ((uint16_t)0xBFFF) /*!< I2C fast mode Tlow/Thigh = 2 */
#define I2C_FLAG_ADDR ((uint32_t)0x10000002)
#define I2C_FLAG_AF ((uint32_t)0x10000400)
#define I2C_FLAG_ARLO ((uint32_t)0x10000200)
#define I2C_FLAG_BERR ((uint32_t)0x10000100)
#define I2C_FLAG_BTF ((uint32_t)0x10000004)
#define I2C_FLAG_OVR ((uint32_t)0x10000800)
#define I2C_FLAG_RXNE ((uint32_t)0x10000040)
#define I2C_FLAG_SB ((uint32_t)0x10000001)
#define I2C_IT_BUF ((uint16_t)0x0400)
#define I2C_IT_ERR ((uint16_t)0x0100)
#define I2C_IT_EVT ((uint16_t)0x0200)
#define I2C_Mode_I2C ((uint16_t)0x0000)
#define I2C_SR1_ADDR ((uint16_t)0x0002)            /*!< Address sent (master mode)/matched (slave mode) */
#define I2C_SR1_AF ((uint16_t)0x0400)            /*!< Acknowledge Failure */
#define I2C_SR1_ARLO ((uint16_t)0x0200)            /*!< Arbitration Lost (master mode) */
#define I2C_SR1_BERR ((uint16_t)0x0100)            /*!< Bus Error */
#define I2C_SR1_BTF ((uint16_t)0x0004)            /*!< Byte Transfer Finished */
#define I2C_SR1_OVR ((uint16_t)0x0800)            /*!< Overrun/Underrun */
#define I2C_SR1_RXNE ((uint16_t)0x0040)            /*!< Data Register not Empty (receivers) */
#define I2C_SR1_SB ((uint16_t)0x0001)            /*!< Start Bit (Master mode) */
#define I2C_SR2_BUSY ((uint16_t)0x0002)            /*!< Bus Busy */
#define I2C_SR2_MSL ((uint16_t)0x0001)            /*!< Master/Slave */
#define RCC_AHBPeriph_DMA1 ((uint32_t)0x00000001)
#define RCC_APB1Periph_I2C1 ((uint32_t)0x00200000)
#define RCC_APB2Periph_AFIO ((uint32_t)0x00000001)
#define RCC_APB2Periph_GPIOB ((uint32_t)0x00000008)
