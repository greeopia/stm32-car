#include "IR.h"
#include "stm32f10x.h"                  // Device header
#include "PID.h"
#include "motor.h"

void IR_init() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_5 | GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	IR_Task_Init();
} // 正对着车子从左到右: PA4 PA5 PB0 PB5 PB1 PB10 PB11
// 黑线输出高电平
static void IR_Task_Init() {
	LmotorTarget = 0.0f, RmotorTarget = 0.0f;

	
}

// 3s电池速度最高大概1.25m/s
void IR_Task() { // 我想应该用位置式pd控制器
	
}