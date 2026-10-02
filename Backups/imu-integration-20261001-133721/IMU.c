#include "IMU.h"
#include "arm_math.h"                   // CMSIS:DSP
#include "stm32f10x.h"                  // Device header
#include "mpu6050.h"
#include "SysTick.h"

// mpu6050一次返回12位数据
/*
	*AccX  = (int16_t)((Buf[0]  << 8) | Buf[1]);			//0x3B/0x3C
	*AccY  = (int16_t)((Buf[2]  << 8) | Buf[3]);			//0x3D/0x3E
	*AccZ  = (int16_t)((Buf[4]  << 8) | Buf[5]);			//0x3F/0x40
	                                                       //Buf[6]和Buf[7]是温度，跳过
	*GyroX = (int16_t)((Buf[8]  << 8) | Buf[9]);			//0x43/0x44
	*GyroY = (int16_t)((Buf[10] << 8) | Buf[11]);			//0x45/0x46
	*GyroZ = (int16_t)((Buf[12] << 8) | Buf[13]);			//0x47/0x48
*/
static uint8_t imubuf[14];

void IMU_init() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	I2C_InitTypeDef I2C_InitStruct;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStruct.I2C_ClockSpeed = 400000;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_OwnAddress1 = 0; // 现在无效
	I2C_Init(I2C1, &I2C_InitStruct);
	
	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&I2C1->DR;				//外设基地址，给定形参AddrA
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//外设数据宽度，选择字节，对应8位的I2C数据寄存器
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//外设地址自增，选择失能，始终以I2C数据寄存器为源
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)imubuf;					//存储器基地址，给定存放AD转换结果的全局数组AD_Value
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;				//存储器数据宽度，选择字节，与源数据宽度对应
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//存储器地址自增，选择使能，每次转运后，数组移到下一个位置
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;							//数据传输方向，选择由外设到存储器，I2C数据寄存器转到数组
	DMA_InitStructure.DMA_BufferSize = 14;										//转运的数据大小（转运次数），与ADC通道数一致
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;								//模式，选择循环模式，与ADC的连续转换一致
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//存储器到存储器，选择失能，数据由ADC外设触发转运到存储器
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;						//优先级，选择中等
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);								//将结构体变量交给DMA_Init，配置DMA1的通道1
	
	DMA_Cmd(DMA1_Channel7, ENABLE);	
	I2C_DMACmd(I2C1, ENABLE);
	I2C_Cmd(I2C1, ENABLE);
	
	I2C_GenerateSTART(I2C1, ENABLE);
}

static void MPU6050_WriteReg(I2C_TypeDef* I2Cx, uint8_t RegAddress, uint8_t Data)
{
	I2C_GenerateSTART(I2Cx, ENABLE);										//硬件I2C生成起始条件
					//等待EV5
	
	I2C_Send7bitAddress(I2Cx, MPU6050_ADDR, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
		//等待EV6
	
	I2C_SendData(I2Cx, RegAddress);											//硬件I2C发送寄存器地址
			//等待EV8
	
	I2C_SendData(I2Cx, Data);												//硬件I2C发送数据
				//等待EV8_2
	
	I2C_GenerateSTOP(I2Cx, ENABLE);											//硬件I2C生成终止条件
}

/**
  * 函    数：MPU6050读寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考MPU6050手册的寄存器描述
  * 返 回 值：读取寄存器的数据，范围：0x00~0xFF
  */
static uint8_t MPU6050_ReadReg(I2C_TypeDef* I2Cx, uint8_t RegAddress)
{
	uint8_t Data;
	
	I2C_GenerateSTART(I2Cx, ENABLE);										//硬件I2C生成起始条件
					//等待EV5
	
	I2C_Send7bitAddress(I2Cx, MPU6050_ADDR, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
			//等待EV6
	
	I2C_SendData(I2Cx, RegAddress);											//硬件I2C发送寄存器地址
			//等待EV8_2
	
	I2C_GenerateSTART(I2Cx, ENABLE);										//硬件I2C生成重复起始条件
					//等待EV5
	
	I2C_Send7bitAddress(I2Cx, MPU6050_ADDR, I2C_Direction_Receiver);		//硬件I2C发送从机地址，方向为接收
		//等待EV6
	
	I2C_AcknowledgeConfig(I2Cx, DISABLE);									//在接收最后一个字节之前提前将应答失能
	I2C_GenerateSTOP(I2Cx, ENABLE);											//在接收最后一个字节之前提前申请停止条件
	
					//等待EV7
	Data = I2C_ReceiveData(I2Cx);											//接收数据寄存器
	
	I2C_AcknowledgeConfig(I2Cx, ENABLE);									//将应答恢复为使能，为了不影响后续可能产生的读取多字节操作
	
	return Data;
}

static void MPU6050_WriteReg_Init(I2C_TypeDef* I2Cx, uint8_t RegAddress, uint8_t Data) {
	static void MPU6050_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT){
	uint32_t Timeout;
	Timeout = 10000;									//给定超时计数时间
	while (I2C_CheckEvent(I2Cx, I2C_EVENT) != SUCCESS)	//循环等待指定事件
	{
		Timeout --;										//等待时，计数值自减
		if (Timeout == 0)								//自减到0后，等待超时
		{
			/*超时的错误处理代码，可以添加到此处*/
			break;										//跳出等待，不等了
		}
	}
	}


	I2C_GenerateSTART(I2Cx, ENABLE);										//硬件I2C生成起始条件
	MPU6050_WaitEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2Cx, MPU6050_ADDR, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
	MPU6050_WaitEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
	I2C_SendData(I2Cx, RegAddress);											//硬件I2C发送寄存器地址
	MPU6050_WaitEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING);			//等待EV8
	
	I2C_SendData(I2Cx, Data);												//硬件I2C发送数据
	MPU6050_WaitEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED);				//等待EV8_2
	
	I2C_GenerateSTOP(I2Cx, ENABLE);											//硬件I2C生成终止条件
}



MPU6050_t imudata;
void IMU_ReadData(I2C_TypeDef* I2Cx, MPU6050_t* imuData) {
	I2C_GenerateSTART(I2Cx, ENABLE);
	I2C_Send7bitAddress(I2Cx, MPU6050_ADDR, I2C_Direction_Transmitter);

}
