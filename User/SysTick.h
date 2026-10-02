#pragma once

#include <stdint.h>

extern volatile uint32_t uwTick;

uint32_t GetTick();
void SysTickInit();