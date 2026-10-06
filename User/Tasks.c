#include "stm32f10x.h"                  // Device header
#include <stddef.h>
#include "Tasks.h"
#include "SysTick.h"

#include "IR.h"
#include "motor.h"
#include "IMU.h"

Task_t task[3] = {
    {.flag = 0, .TimCount = 0, .TimReload = 10, .pTaskFunc = motor_Task},
    {.flag = 0, .TimCount = 0, .TimReload = 20, .pTaskFunc = IMU_Task},
    {.flag = 0, .TimCount = 0, .TimReload = 40, .pTaskFunc = IR_Task},
}; // 顺序应该也代表简易仲裁优先级了

/* 每1ms调用一次，只负责计时 */
void Tasks_Tick(void)
{
    for (uint8_t i = 0; i < sizeof(task)/sizeof(task[0]); i++)
    {
        if (task[i].pTaskFunc == NULL) continue; // 空槽位不参与计时：TimReload=0会导致flag被立即置位
            
        task[i].TimCount++;

        if (task[i].TimCount >= task[i].TimReload) {
            task[i].TimCount = 0;
            task[i].flag = 1;
        }
    }
}

/* 在主循环中不断调用，负责执行到期任务 */
void Tasks_Run(void) {
    for (uint8_t i = 0; i < sizeof(task)/sizeof(task[0]); i++)
    {
        if (task[i].flag && task[i].pTaskFunc != NULL)
        {
            task[i].flag = 0;
            task[i].pTaskFunc();
        }
    }
}


