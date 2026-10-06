#include "vofa.h"
#include "BlueTooth.h"
#include "stm32f10x.h"                  // Device header

Frame TxFrame = {
    .fdata = {0.0f},
    .tail = {0x00, 0x00, 0x80, 0x7f}
};

void vofa_init(Frame* frame) {
    frame->fdata[0] = GetSpeedl();
    frame->fdata[1] = GetDutyl();
    frame->fdata[2] = GetSpeedr();
    frame->fdata[3] = GetDutyr();
}

