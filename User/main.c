#include "stm32f10x.h"                  // Device header
#include "SysTick.h"
#include "Tasks.h"

#include "motor.h"
#include "OLED.h"
#include "BlueTooth.h"
#include "IR.h"
#include "vofa.h"
// #include "IMU.h"


int main(){
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SysTickInit();

	/* 诊断用：先初始化OLED并显示固定内容，以区分"程序卡死"和"I2C没通" */
	OLED_Init();
	motor_init();
	Encoder_init();
	Target_AD_Init();
	vofa_init();
	// IMU_init();
	// BlueTooth_init();

	Task_Init(task, sizeof(task)/sizeof(task[0]));
	while(1) {
		// IMU_Task();

		Tasks_Run();
		// OLED_ShowNum(3, 1, (uint32_t)TIM2->CNT, 4);
		// OLED_ShowNum(4, 1, (uint32_t)TIM3->CNT, 4);

	}
}
