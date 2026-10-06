#include "stm32f10x.h"                  // Device header
#include "SysTick.h"

volatile uint32_t uwTick = 0; 

uint32_t GetTick() {
	return uwTick;
}

void SysTickInit() {
	if (SysTick_Config(SystemCoreClock / 1000)) { // 1ms一次
		while (1);
	}
}

/**
  * @brief  SysTick 中断服务函数
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    ++uwTick;
    Tasks_Tick();
}
