#include "stm32f10x.h"                  // Device header
#include "IR.h"
#include "PID.h"
#include "motor.h"
#include "IMU.h"
#include "SysTick.h"


void motor_init() {
	// PWM引脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, & GPIO_InitStruct);
	
	TIM_InternalClockConfig(TIM1);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
	TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStruct.TIM_Period = 7200 - 1;
	TIM_TimeBaseStruct.TIM_Prescaler = 10-1; // 72MHz/10/7200 = 1000Hz
	TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStruct);
	
	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_OCStructInit(&TIM_OCInitStruct);
//	TIM_OCInitStruct.TIM_OCIdleState
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = (uint16_t)(7200 * 0.38f);
	TIM_OC1Init(TIM1, &TIM_OCInitStruct);
	TIM_OC2Init(TIM1, &TIM_OCInitStruct);
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	TIM_Cmd(TIM1, ENABLE);
	// CCR1/2: 0~7200
	
	
	// 方向引脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//	GPIO_InitTypeDef GPIO_InitStruct2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, & GPIO_InitStruct);

	GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET); // OUT1
	GPIO_WriteBit(GPIOB, GPIO_Pin_13, Bit_SET); // OUT2
	GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_RESET); // OUT3
	GPIO_WriteBit(GPIOB, GPIO_Pin_15, Bit_SET); // OUT4
	// 这样才是正转嘛(笑)
} 

extern PID Lmotor_PID;
extern PID Rmotor_PID;

float GetDutyl() {
	return 1.0f*TIM1->CCR1/(TIM1->ARR+1);
}

float GetDutyr() {
	return 1.0f*TIM1->CCR2/(TIM1->ARR+1);
}

void Encoder_init() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/*外部时钟配置，两路都走TI1，即外部时钟模式1*/
	TIM_TIxExternalClockConfig(TIM2, TIM_TIxExternalCLK1Source_TI1, TIM_ICPolarity_Rising, 0xF);												
	TIM_TIxExternalClockConfig(TIM3, TIM_TIxExternalCLK1Source_TI1, TIM_ICPolarity_Rising, 0xF);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = 65536 - 1;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 1-1;
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
	
	TIM2->CNT = 0; TIM3->CNT = 0;

	TIM_Cmd(TIM2, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}

static const float WHEEL_RADIUS = 0.033f; // m

#ifndef PI
#define PI          3.14159265f
#endif
float GetMotorCountl() { // m/20ms
	uint16_t cnt = TIM2->CNT;
	// TIM2->CNT = 0;

	float Countl = (1.0f*cnt/20)*2*PI*WHEEL_RADIUS;
	return Countl;
}

float GetMotorCountr() {
	uint16_t cnt = TIM3->CNT;
	// TIM3->CNT = 0;

	float Countr = (1.0f*cnt/20)*2*PI*WHEEL_RADIUS;
	return Countr;
}

float speedl = 0.0f, speedr = 0.0f; // m/s 实际值

void motor_Task() {
	static float LastCountl = 0, LastCountr = 0;
	static uint32_t LastTick = 0;
	float Countl = GetMotorCountl(), Countr = GetMotorCountr();
	if (LastCountl > Countl || LastCountr > Countr) { // 计数器溢出
		LastCountl = Countl;
		LastCountr = Countr;
		LastTick = GetTick();
		return;
	}
	uint32_t NowTick = GetTick();
	speedl = (Countl - LastCountl) / (NowTick - LastTick) * 1000.0f;
	speedr = (Countr - LastCountr) / (NowTick - LastTick) * 1000.0f;
	LastTick = NowTick; LastCountl = Countl; LastCountr = Countr;
	
	
}
//    // 计算转向误差（头文件已定义ImageStatus）
//     float turn_error = ImageStatus.Det_True - (float)ImageStatus.MiddleLine;
//     // 误差死区：±2内视为无偏差
//     if (turn_error > -1.0f && turn_error < 1.0f)
//     {
//         turn_error = 0;
//     }
