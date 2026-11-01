#include <SpeedDecision.h>
#include <stm32f10x.h>
#include <PID.h>
#include <SysTick.h>
#include <IR.h>
#include <math.h>

extern PID speed_decision;

#define SPEED_ACC   0.30f   // 升速加速度上限 m/s²
#define SPEED_DEC   1.20f   // 降速加速度上限 m/s²（降速快 4 倍）
#define SPEED_KP    6.0f    // 速度环增益 1/s
#define SPEED_MIN   0.20f   // 基础速度下限 m/s
#define SPEED_K_SLOW 0.12f  // 每 1 单位 |bias| 降多少 m/s

/**
 * ************************************************************************
 * @brief 速度决策模块初始化
 * @note 用位置式PID初始化速度决策控制器 speed_decision：
 * @note 
 * ************************************************************************
 */
void soft_speeddecision_init(PID* des) {
    Positional_PID_Init(des, SPEED_KP, 0, 0, 0, -SPEED_DEC, SPEED_ACC);
    PID_Reset(des); 
}

/**
 * ************************************************************************
 * @brief 速度决策周期任务
 * @note 由调度器按固定周期调用，注意调度表有固定顺序
 * ************************************************************************
 */
void Decision_Task() {
    static uint32_t LastTime = 0; // ms
    uint32_t now = GetTick();
    float dt = (uint32_t)(now - LastTime) * 0.001f;
    if (dt > 0.1f) dt = 0.1f;

    float target = base_speed_max - SPEED_K_SLOW * fabsf(bias);
    if (target < SPEED_MIN) target = SPEED_MIN;

    Positional_PID_Cal(&speed_decision, target, base_speed);   // output = m/s²
    base_speed += speed_decision.output * dt;                  // ← 关键，不能少
    if (base_speed < 0.0f) base_speed = 0.0f;
    if (base_speed > base_speed_max) base_speed = base_speed_max;

    LastTime = now;
}