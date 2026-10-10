#include "stm32f10x.h"                  // Device header
#include <stddef.h>
#include "Tasks.h"
#include "SysTick.h"

#include "IR.h"
#include "motor.h"
#include "IMU.h"
#include "BlueTooth.h"
#include "vofa.h"

Task_t task[4] = {
    {.flag = 0, /*.TimCount = 0,*/.LastTime = 0, .TimReload = 100, .pTaskFunc = motor_Task},
    {.flag = 0, /*.TimCount = 0,*/.LastTime = 0, .TimReload = 0, .pTaskFunc = IMU_Task},
    {.flag = 0, /*.TimCount = 0,*/.LastTime = 0, .TimReload = 400, .pTaskFunc = IR_Task},
    {.flag = 0, /*.TimCount = 0,*/.LastTime = 0, .TimReload = 50, .pTaskFunc = SerialVofa_Task}
}; // 顺序应该也代表简易仲裁优先级了

/* 每1ms调用一次，只负责计时 */
// void Tasks_Tick(void)
// {
//     for (uint8_t i = 0; i < TaskNum; i++)
//     {
//         if (task[i].pTaskFunc == NULL) continue; // 空槽位不参与计时：TimReload=0会导致flag被立即置位
            
//         // task[i].TimCount++;

//         if (task[i].TimCount >= task[i].TimReload) {
//             task[i].TimCount = 0;
//             task[i].flag = 1;
//         }
//     }
// }

void Task_Init(Task_t task[], uint8_t tasksnum) {
    uint32_t now = GetTick();
    for (uint8_t i = 0; i < tasksnum; i++) {
        task[i].LastTime = now;
        task[i].flag = 0;
    }
}

/* 在主循环中不断调用，负责执行到期任务 */
static uint8_t TaskNum = sizeof(task)/sizeof(task[0]);
void Tasks_Run(void) {
    for (uint8_t i = 0; i < TaskNum; i++)
    {
        if (task[i].pTaskFunc == NULL || task[i].TimReload == 0) continue;

        if ((GetTick() - task[i].LastTime) >= task[i].TimReload) {
            task[i].LastTime = GetTick();
            task[i].flag = 1;
        }

        if (task[i].flag) {
            task[i].flag = 0;
            task[i].pTaskFunc();
        }
    }
}


