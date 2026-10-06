#pragma once

void motor_init();
float GetDutyl();
float GetDutyr();

void Encoder_init();
float GetMotorCountl();
float GetMotorCountr();

extern float speedl, speedr; // m/s

void motor_Task();


// TIM1->CCR1/2: 目标值; TIM2/3->CNT: 实际值;
