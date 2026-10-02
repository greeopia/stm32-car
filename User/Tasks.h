#pragma once

#include <stdint.h>

//typedef enum {
//	TASK_FINISHED,
//	TASK_READY,
//	TASK_RUNNING,
//	TASK_SUSPEND
//} TasksState_t;

typedef struct {
//	TasksState_t state;
	volatile uint8_t flag;
	volatile uint16_t TimCount; // (every tick, TimCount++, when TimCount == TimReload, state = TASK_READY)
	uint16_t TimReload;
	// 和CNT ARR 的关系类似 
	void (*pTaskFunc)();
} Task_t;
