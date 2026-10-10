#pragma once

#include <stdint.h>

#define MOTOR_TARGET_MAX_MPS 3.0f

/* 两轮仅正转；占空比范围为0~1，负值按0处理。 */
void motor_init(void);
float GetDutyl(void);
float GetDutyr(void);
void SetDutyl(float duty);
void SetDutyr(float duty);

void Encoder_init(void);
float GetMotorCountl(void);
float GetMotorCountr(void);

extern float speedl, speedr; // 正转速度，m/s

void motor_Task(void);

extern volatile uint16_t AD_Value;

void Target_AD_Init(void);
float GetTarget(void); // 电位器全量程对应0~MOTOR_TARGET_MAX_MPS，m/s


// TIM1->CCR1/2控制PWM；TIM2/3->CNT提供单路脉冲计数反馈。
