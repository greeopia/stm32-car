#pragma once

#include "stm32f10x.h"                  // Device header

#define CH_COUNT 6
/* CH0/1：左右正转速度m/s；CH2/3：左右目标0~3m/s；CH4/5：左右占空比0~1。 */
typedef struct Frame{
    float fdata[CH_COUNT];
    uint8_t tail[4];
}Frame;

extern Frame TxFrame;
extern uint8_t RxFrame[2][128]; // RX先不用

void vofa_init();
void SerialVofa_Task();

