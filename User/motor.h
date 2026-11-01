#pragma once

#include <stdint.h>

#define MOTER_HAVE_ENCODERS 1

/* 两轮仅正转；占空比范围为0~1，负值按0处理。 */
void motor_init(void);
float GetDutyl(void);
float GetDutyr(void);
void SetDutyl(float duty);
void SetDutyr(float duty);

void Encoder_init(void);

#if MOTER_HAVE_ENCODERS
float GetMotorSpeedl(uint32_t T);
float GetMotorSpeedr(uint32_t T);
#else
float GetMotorCountl(void);
float GetMotorCountr(void);
#endif

extern float speedl, speedr; // 正转速度，m/s
// extern volatile uint16_t pulse_l_100ms, pulse_r_100ms; // 每100ms的原始脉冲增量，用于VOFA诊断
extern volatile float LmotorTarget, RmotorTarget; // 正转速度目标，m/s


void motor_Task(void);

extern volatile uint16_t AD_Value;

void Target_AD_Init(void);
float GetTarget(void); // 电位器全量程对应0~3 m/s，m/s


// TIM1->CCR1/2控制PWM；TIM2/3->CNT提供单路脉冲计数反馈。
