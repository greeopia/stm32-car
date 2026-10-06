#pragma once

#include "stm32f10x.h"                  // Device header

#define CH_COUNT 6
typedef struct Frame{
    float fdata[CH_COUNT];
    uint8_t tail[4];
}Frame;

extern Frame TxFrame;

void vofa_init(Frame* frame);

