#pragma once

// #include "stm32f10x.h"                  // Device header
// #include <stdbool.h>

// #define CH_COUNT 6
// /* CH0/1：左右正转速度m/s（或诊断时每100ms原始脉冲数）；CH2/3：左右目标0~3m/s；CH4/5：左右占空比0~1。 */
// typedef struct Frame{
//     float fdata[CH_COUNT];
//     uint8_t tail[4];
// }TxFrame;

// extern TxFrame txFrame;
// // extern uint8_t RxFrame[2][128]; // RX先不用

// typedef struct {
// 	uint8_t fdata[2][128];
// 	uint8_t head, tail;

//     bool (*push)(struct RxFrame*, uint8_t);
//     bool (*pop)(struct RxFrame*, uint8_t*);
// } RxFrame;

// extern RxFrame rxFrame;

void vofa_init();
void SerialVofa_Task();

