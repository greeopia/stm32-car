#pragma once

#include "kalman_filter/kalman_filter.h"

extern float L_filter_speed, R_filter_speed;
extern kalman1_state Lmotor_Filter, Rmotor_Filter;

// 调 Q R 矩阵，初始 P 矩阵有影响但不多
