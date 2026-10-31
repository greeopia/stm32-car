#pragma once

#include "stm32f10x.h"                  // Device header

// typedef enum {
    
// } IR_Index_t;

typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
    const char* label;

    uint8_t mask;
    int8_t weight;
}IR_GPIO_t; // PB11 PB10 PB1 PB5 PB0 PA5 PA4

extern IR_GPIO_t IR_GPIO[];

typedef enum {
    IR_State_Normal,
    IR_State_FullCheck,
    IR_State_LostLine
}IR_State_t;

extern IR_State_t IR_State;

void IR_init(void);

void IR_Task(void);

extern float bias_ave, base_speed, base_speed_max;
