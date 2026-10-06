#include "stm32f10x.h"                  // Device header
#include "SysTick.h"
#include "motor.h"
#include "OLED.h"
#include "BlueTooth.h"
#include "IR.h"
#include "IMU.h"


int main(){
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SysTickInit();
	motor_init();
	Encoder_init();
	IMU_init();
	BlueTooth_init();

	OLED_Init();


	while(1) {
		
	}
}
