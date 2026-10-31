#include "stm32f10x.h"                  // Device header
#include "SysTick.h"
#include "Tasks.h"
#include "Delay.h"

#include "motor.h"
#include "OLED.h"
#include "BlueTooth.h"
#include "IR.h"
#include "vofa.h"
#include "PID.h"
#include "SpeedDecision.h"
// #include "IMU.h"

// emm 速度环是带滤波的增量式pi, 循迹是带速度前馈的位置式pd

int main(){
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SysTickInit();

	/* 诊断用：先初始化OLED并显示固定内容，以区分"程序卡死"和"I2C没通" */
	OLED_Init();
	motor_init();
	Encoder_init();
	Target_AD_Init();

	IR_init();
	soft_speeddecision_init(&speed_decision);
	// vofa_init();
	// IMU_init();
	BlueTooth_init();
	// Delay_s(2);

	Task_Init(task, sizeof(task)/sizeof(task[0]));

	OLED_ShowString(1, 1, "motor");
	OLED_ShowString(1, 7, "turn");
	// while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) == Bit_SET){
	while(1){
		// IMU_Task();
		Tasks_Run();
		// OLED_ShowNum(2, 1, (uint32_t)(Lmotor_PID.kp*100), 4);
		// OLED_ShowNum(3, 1, (uint32_t)(Lmotor_PID.ki*100), 4);
		// OLED_ShowNum(4, 1, (uint32_t)(Lmotor_PID.kd*100), 4);

		// OLED_ShowNum(2, 7, (uint32_t)(IR_PID.kp*100), 4);
		// OLED_ShowNum(3, 7, (uint32_t)(IR_PID.ki*100), 4);
		// OLED_ShowNum(4, 7, (uint32_t)(IR_PID.kd*100), 4);
	}
	SetDutyl(0.0f);
	SetDutyr(0.0f);	
}
