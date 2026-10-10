#include "stm32f10x.h"                  // Device header
#include "PID.h"
#include "motor.h"
#include "SysTick.h"

static void motor_pid_init(void);

void motor_init(void) {
	// PWM引脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, & GPIO_InitStruct);

	TIM_InternalClockConfig(TIM1);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
	TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStruct.TIM_Period = 7200 - 1;
	TIM_TimeBaseStruct.TIM_Prescaler = 10-1; // 72MHz/10/7200 = 1000Hz
	TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStruct);

	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_OCStructInit(&TIM_OCInitStruct);
//	TIM_OCInitStruct.TIM_OCIdleState
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = (uint16_t)(7200 * 0.0f);
	TIM_OC1Init(TIM1, &TIM_OCInitStruct);
	TIM_OC2Init(TIM1, &TIM_OCInitStruct);

	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	TIM_Cmd(TIM1, ENABLE);
	// CCR1/2: 0~7200


	// 方向引脚只在初始化时设置，固定为正转。
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//	GPIO_InitTypeDef GPIO_InitStruct2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, & GPIO_InitStruct);

	GPIO_ResetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_14); // IN1、IN3低电平
	GPIO_SetBits(GPIOB, GPIO_Pin_13 | GPIO_Pin_15); // IN2、IN4高电平

	motor_pid_init();
}

static void motor_pid_init(void) {
	Incremental_PID_Init(&Lmotor_PID, 0.5f, 0.1f, 0.0f, 0.0f, 1.0f);
	Incremental_PID_Init(&Rmotor_PID, 0.5f, 0.1f, 0.0f, 0.0f, 1.0f);
}

float GetDutyl(void) {
	return 1.0f*TIM1->CCR1/(TIM1->ARR+1);
}

float GetDutyr(void) {
	return 1.0f*TIM1->CCR2/(TIM1->ARR+1);
}

void SetDutyl(float duty) {
	if (duty < 0.0f) duty = 0.0f;
	if (duty > 1.0f) duty = 1.0f;
	TIM1->CCR1 = (uint16_t)(duty * (TIM1->ARR+1));
}

void SetDutyr(float duty) {
	if (duty < 0.0f) duty = 0.0f;
	if (duty > 1.0f) duty = 1.0f;
	TIM1->CCR2 = (uint16_t)(duty * (TIM1->ARR+1));
}

void Encoder_init(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*外部时钟配置，两路都走TI1，即外部时钟模式1*/
	TIM_TIxExternalClockConfig(TIM2, TIM_TIxExternalCLK1Source_TI1, TIM_ICPolarity_Rising, 0xF);
	TIM_TIxExternalClockConfig(TIM3, TIM_TIxExternalCLK1Source_TI1, TIM_ICPolarity_Rising, 0xF);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = 65536 - 1;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 1-1;
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);

	TIM2->CNT = 0; TIM3->CNT = 0;

	TIM_Cmd(TIM2, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}

static const float WHEEL_RADIUS = 0.033f; // m

#ifndef PI
#define PI          3.14159265f
#endif
float GetMotorCountl(void) { // 当前计数值对应的路程，m
	uint16_t cnt = TIM2->CNT;
	// TIM2->CNT = 0;

	float Countl = (1.0f*cnt/20)*2*PI*WHEEL_RADIUS;
	return Countl;
}

float GetMotorCountr(void) {
	uint16_t cnt = TIM3->CNT;
	// TIM3->CNT = 0;

	float Countr = (1.0f*cnt/20)*2*PI*WHEEL_RADIUS;
	return Countr;
}

float speedl = 0.0f, speedr = 0.0f; // 正转速度，m/s

void motor_Task(void) { // PI: 正转速度 -> 0~1占空比
	static float LastCountl = 0, LastCountr = 0;
	static uint32_t LastTick = 0;
	float Countl = GetMotorCountl(), Countr = GetMotorCountr();
	if (LastCountl > Countl || LastCountr > Countr) { // 计数器溢出
		LastCountl = Countl;
		LastCountr = Countr;
		LastTick = GetTick();
		return;
	}
	uint32_t NowTick = GetTick();
	speedl = (Countl - LastCountl) / (NowTick - LastTick) * 1000.0f;
	speedr = (Countr - LastCountr) / (NowTick - LastTick) * 1000.0f;
	LastTick = NowTick; LastCountl = Countl; LastCountr = Countr;


	float target = GetTarget();
	PID_Lmotor(target);
	PID_Rmotor(target);

	SetDutyl(Lmotor_PID.output);
	SetDutyr(Rmotor_PID.output);
}
//    // 计算转向误差（头文件已定义ImageStatus）
//     float turn_error = ImageStatus.Det_True - (float)ImageStatus.MiddleLine;
//     // 误差死区：±2内视为无偏差
//     if (turn_error > -1.0f && turn_error < 1.0f)
//     {
//         turn_error = 0;
//     }

volatile uint16_t AD_Value; // DMA传输的ADC值

void Target_AD_Init(void) { // PA1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/*设置ADC时钟*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);						//选择时钟6分频，ADCCLK = 72MHz / 6 = 12MHz

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*ADC初始化*/
	ADC_InitTypeDef ADC_InitStructure;						//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;      //模式，选择独立模式，即单独使用ADC2
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  //数据对齐，选择右对齐
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//外部触发，使用软件触发，不需要外部触发gConv_None;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;     //连续转换，失能，每转换一次规则组序列后停止
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;           //扫描模式，失能，只转换规则组的序列1这一个位置
	ADC_InitStructure.ADC_NbrOfChannel = 1;                 //通道数，为1，仅在扫描模式下，才需要指定大于1的数，在非扫描模式下，只能是1
	ADC_Init(ADC1, &ADC_InitStructure);                     //将结构体变量交给ADC_Init，配置ADC2

	/*DMA初始化*/
	DMA_InitTypeDef DMA_InitStructure;											//定义结构体变量
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;				//外设基地址，给定形参AddrA
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//外设数据宽度，选择半字，对应16为的ADC数据寄存器
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//外设地址自增，选择失能，始终以ADC数据寄存器为源
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&AD_Value;					//存储器基地址，给定存放AD转换结果的全局数组AD_Value
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//存储器数据宽度，选择半字，与源数据宽度对应
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//存储器地址自增，选择使能，每次转运后，数组移到下一个位置
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;							//数据传输方向，选择由外设到存储器，ADC数据寄存器转到数组
	DMA_InitStructure.DMA_BufferSize = 1;										//转运的数据大小（转运次数），与ADC通道数一致
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								//模式，选择循环模式，与ADC的连续转换一致
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//存储器到存储器，选择失能，数据由ADC外设触发转运到存储器
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						//优先级，选择中等
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);								//将结构体变量交给DMA_Init，配置DMA1的通道1

	/*DMA和ADC使能*/
	DMA_Cmd(DMA1_Channel1, ENABLE);							//DMA1的通道1使能
	ADC_DMACmd(ADC1, ENABLE);								//ADC1触发DMA1的信号使能
	ADC_Cmd(ADC1, ENABLE);									//ADC1使能

	/*ADC校准*/
	ADC_ResetCalibration(ADC1);								//固定流程，内部有电路会自动执行校准
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);

	/*ADC触发*/
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	//软件触发ADC开始工作，由于ADC处于连续转换模式，故触发一次后ADC就可以一直连续不断地工作
}

float GetTarget(void) { // 正转目标速度：电位器全量程对应0~3m/s。
	return (float)AD_Value / 4095.0f * MOTOR_TARGET_MAX_MPS;
}
