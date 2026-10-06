#include "stm32f10x.h"                  // Device header
#include "Tasks.h"

#include "IR.h"
#include "motor.h"
#include "IMU.h"

Task_t task[3] = {
    {.flag = 0, .TimCount = 0, .TimReload = 10, .pTaskFunc = motor_Task},
    {.flag = 0, .TimCount = 0, .TimReload = 20, .pTaskFunc = IMU_Task},
    {.flag = 0, .TimCount = 0, .TimReload = 40, .pTaskFunc = IR_Task},
};
