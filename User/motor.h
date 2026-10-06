#pragma once

void motor_init();
float GetDutyl();
float GetDutyr();

void Encoder_init();
float GetMotorCountl();
float GetMotorCountr();

extern float speedl, speedr; // m/s

void motor_Task();
