#pragma once

void BlueTooth_init();
void BlueToothVofa_Task();

#include "stm32f10x.h"                  // Device header
#include <stdbool.h>

#define CH_COUNT 8
/* CH0/1：左右正转速度m/s（或诊断时每100ms原始脉冲数）；CH2/3：左右目标0~3m/s；CH4/5：左右占空比0~1。 */
typedef struct Frame{
    float fdata[CH_COUNT];
    uint8_t tail[4];
}TxFrame;

extern TxFrame txFrame;
// extern uint8_t RxFrame[2][128]; // RX先不用

#define U1_FRAME_SIZE 128
typedef struct RxFrame{
	uint8_t fdata[2][U1_FRAME_SIZE];
	uint8_t head, tail;
    volatile bool CMAR;

    bool (*push)(struct RxFrame* self, const uint8_t data);
    // uint8_t (*pop)(struct RxFrame* self, uint8_t*);
    uint8_t (*pop)(struct RxFrame* self);
} RxFrame;

extern RxFrame rxFrame;
