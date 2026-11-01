#include "IR.h"
#include "stm32f10x.h"                  // Device header
#include "PID.h"
#include "motor.h"
#include <SpeedDecision.h>
#include "SysTick.h"

static uint8_t IR_MASKS = 0x00; // 低位对应左边传感器，高位对应右边传感器 0b00000000

static enum {
	IR_PB11 = 0,
	IR_PB10 = 1,
	IR_PB1 = 2,
	IR_PB5 = 3,
	IR_PB0 = 4,
	IR_PA5 = 5,
	IR_PA4 = 6,
} IR_Index;

IR_State_t IR_State = IR_State_Normal;

IR_GPIO_t IR_GPIO[7] = {
	{GPIOB, GPIO_Pin_11, "PB11", 0x01 << 6, -3},
	{GPIOB, GPIO_Pin_10, "PB10", 0x01 << 5, -2},
	{GPIOB, GPIO_Pin_1, "PB1", 0x01 << 4, -1},
	{GPIOB, GPIO_Pin_5, "PB5", 0x01 << 3, 0},
	{GPIOB, GPIO_Pin_0, "PB0", 0x01 << 2, 1},
	{GPIOA, GPIO_Pin_5, "PA5", 0x01 << 1, 2},
	{GPIOA, GPIO_Pin_4, "PA4", 0x01 << 0, 3}
};

extern PID IR_PID;

static void IR_Task_Init(void) {
	// 左 PB11 PB10 PB1 PB5 PB0 PA5 PA4 右
	// err > 0 -> 向左转 --> Ltarget = base - pid(err); Rtarget = base + pid(err) 
	LmotorTarget = 0.0f;
	RmotorTarget = 0.0f;

	Positional_PID_Init(&IR_PID, 0.1f, 0.0f, 0.00f, 999.9f, -0.4f, 0.4f);
}

void IR_init(void) {
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

float bias; // 滤波后的线偏移，供 IR_PID 和 SpeedDecision 读取

// static uint8_t IR_Counts(uint8_t mask) {
//     uint8_t count = 0;
//     mask &= 0x7F;  // 七路红外，只统计低7位
//     while (mask != 0) {
//         mask &= mask - 1;
//         count++;
//     }
//     return count;
// }

static void IR_Mask_Update() {
	static uint8_t IR_GPIO_NUM = sizeof(IR_GPIO)/sizeof(IR_GPIO[0]); int8_t temps = 0;
	for (uint8_t i = 0; i < IR_GPIO_NUM; i++) {
		if (GPIO_ReadInputDataBit(IR_GPIO[i].port, IR_GPIO[i].pin) == SET) {
			IR_MASKS |= IR_GPIO[i].mask;
			temps += IR_GPIO[i].weight;
		}
		else IR_MASKS &= ~IR_GPIO[i].mask;
	}
	if (__builtin_popcount(IR_MASKS) == 0) IR_State = IR_State_LostLine;
	else if (IR_MASKS == 0b01111111) IR_State = IR_State_FullCheck;
	
	else {
		float raw = 1.0f*temps/__builtin_popcount(IR_MASKS);
		IR_State = IR_State_Normal;
		bias += 0.5f*(raw - bias); // 一阶低通，代替原来的3点平均
	}
}

float base_speed = 0.0f, base_speed_max = 0.8f;
// 3s电池速度最高大概1.25m/s

static uint32_t check = 0;
static uint8_t check_first = 1;
void IR_Task(void) { // 我想应该用位置式pd控制器
	IR_Mask_Update();
	switch (IR_State) {
		case IR_State_LostLine: {
			bias = 0;

			if (check_first) {
				check = GetTick();
				check_first = 0;
			}
			if (GetTick() - check >= 2000) {
				__disable_irq();
				SetDutyl(0.0f); SetDutyr(0.0f);
				while(1);
			}
			break;
		}
		case IR_State_FullCheck: {
			bias = 0.0f; 
			LmotorTarget = base_speed; RmotorTarget = base_speed;

			break;
		}
		case IR_State_Normal: { // 这里也许可以改一下，改成用前馈
			check_first = 1;

			Positional_PID_Cal(&IR_PID, 0, bias);

			LmotorTarget = base_speed - IR_PID.output; 
			RmotorTarget = base_speed + IR_PID.output;
			break;
		}
		/*
		1.固定基础速度，别的控制环输出差速量
		2.根据视觉的元素决定基础速度
		3.根据某一个环的误差量决定基础速度（也许可以加一点滤波啥啥的）
		*/
	}
}
