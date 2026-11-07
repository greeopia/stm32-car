/*
 * FileName : kalman_filter.h
 * Author   : xiahouzuoxin @163.com
 * Version  : v1.0
 * Date     : 2014/9/24 20:37:01
 * Brief    : 一维/二维卡尔曼滤波器的公开数据类型和 API。
 * 
 * Copyright (C) MICL,USTB
 *
 * 模型与单位约定：
 *   状态预测：x(k) = A * x(k-1) + u(k)
 *   测量方程：z(k) = H * x(k) + w(k)
 *   u(k) 的协方差为 q，w(k) 的协方差为 r。
 *
 *   x、z、q、r、p 没有固定单位，但必须在同一次使用中保持物理量一致。
 *   例如 x 表示轮速 m/s 时，q、r、p 使用 (m/s)^2；编码器原始脉冲数
 *   不能直接和 m/s 混用。改变任务周期、速度单位或状态定义后，q/r
 *   通常都要重新整定。
 *
 *   本文件中的函数不检查空指针，调用者必须先提供有效的状态对象。
 */
#ifndef  _KALMAN_FILTER_H
#define  _KALMAN_FILTER_H

/*
 * NOTES: n Dimension means the state is n dimension, 
 * measurement always 1 dimension.
 *
 * 说明：n 维指状态 x 有 n 个分量；测量 z_measure 始终只有一个值。
 */

/*
 * 一维状态。
 *
 * 这里的一维是指状态 x 只有一个分量，不是“只能采样一次”。
 * 对轮速、温度、ADC 单通道等标量数据，通常每采样一次调用一次
 * kalman1_filter()。
 */
typedef struct {
    float x;  /* 状态估计值；对轮速通常是当前滤波后的速度 */
    float A;  /* 状态转移系数：x(k)=A*x(k-1)+u(k) */
    float H;  /* 测量系数：z(k)=H*x(k)+w(k)，直接测 x 时为 1 */
    float q;  /* 过程噪声方差；越大越相信新变化，跟踪更快但抖动更大 */
    float r;  /* 测量噪声方差；越大越不信任测量，曲线更平滑但响应更慢 */
    float p;  /* 状态估计误差方差；由滤波器持续更新，不是固定参数 */
    float gain; /* 本次更新计算出的卡尔曼增益 gain = P*H/(P*H^2+r) */
} kalman1_state;

/*
 * 二维常量速度模型，原用途接近“角度 + 角速度”。
 *
 * x[0] 是被测量/输出的状态，例如角度；x[1] 是它的变化率，例如
 * 角速度。两者单位必须与 A 中使用的采样时间相符。
 *
 * 重要限制：当前 kalman_filter.c 的二维协方差预测和更新公式并不
 * 等价于完整的 P=A*P*A^T+Q / P=(I-K*H)*P。它能编译，也可能在
 * 特定参数下看似工作，但不能当作经过验证的通用二维滤波器使用。
 */
typedef struct {
    float x[2];     /* 状态 [被测量, 变化率]，列向量 2x1 */
    float A[2][2];  /* 状态转移矩阵；A[0][1] 通常是采样周期 dt */
    float H[2];     /* 测量矩阵 1x2；只测 x[0] 时通常为 {1, 0} */
    float q[2];     /* 两个状态的过程噪声方差，只表示对角 Q */
    float r;        /* 单路测量的噪声方差，标量 */
    float p[2][2];  /* 状态估计误差协方差矩阵，应保持对称 */
    float gain[2];  /* 卡尔曼增益向量 2x1，每次更新时重新计算 */
} kalman2_state;                   

/*
 * 初始化一维滤波器。
 *
 * @param state   调用者分配的滤波器对象。
 * @param init_x  第一个可信状态值。对轮速可先填入第一帧有效速度；
 *                若暂时不知道，可填 0，但前几帧收敛会稍慢。
 * @param init_p  初始误差方差。表示对 init_x 有多不确定；越大表示
 *                越不相信初值。它不是测量方差 r。
 *
 * 函数会把 A、H 分别设为 1，并把 q、r 分别设为 200、500。
 * 这些默认值来自原作者示例，不是本项目轮速的推荐参数；初始化后
 * 通常必须按实际单位和采样周期覆盖 state->q 与 state->r。
 */
void kalman1_init(kalman1_state *state, float init_x, float init_p);

/*
 * 处理一维测量的一帧新数据，并返回更新后的状态估计。
 *
 * @param state      已初始化的滤波器对象。
 * @param z_measure  本次新测量值，单位必须与 state->x 相同。
 * @return           state->x，也就是本帧滤波结果。
 *
 * 调用约定：
 *   1. 每取得一个“新”速度值调用一次，不要对同一个旧值重复调用。
 *   2. 左右轮分别建立一个 kalman1_state，不能共用同一状态。
 *   3. 若采样周期变化，A=1 的模型仍可用，但 q/r 需要重新评估。
 */
float kalman1_filter(kalman1_state *state, float z_measure);

/*
 * 初始化二维滤波器。
 *
 * @param state   调用者分配的滤波器对象。
 * @param init_x  长度 2 的初值数组：{被测量初值, 变化率初值}。
 * @param init_p  2x2 初始协方差矩阵。
 *
 * 函数固定把 A 设为 {{1, 0.1}, {0, 1}}、H 设为 {1, 0}，
 * q[0]=q[1]=1e-6、r=1e-6。A[0][1]=0.1 实际隐含了 0.1 秒的
 * 采样周期；迁移到别的周期时必须让 A[0][1] 等于真实 dt，并让
 * x[1]、q、r 的单位体系自洽。
 *
 * 注意：本仓库这版二维协方差递推存在实现缺陷，详见 .c 文件。
 */
extern void kalman2_init(kalman2_state *state, float *init_x, float (*init_p)[2]);

/*
 * 处理二维状态模型的一帧单路测量，返回 state->x[0]。
 *
 * @param state      已初始化的二维滤波器对象。
 * @param z_measure  对 x[0] 的新测量值。
 * @return           更新后的 state->x[0]；x[1] 仍在 state 中更新。
 *
 * 由于当前二维实现的 P 递推不正确，除非先修复并验证矩阵运算，否则
 * 不建议把此 API 用于姿态、轨迹或需要可靠协方差估计的场合。
 */
extern float kalman2_filter(kalman2_state *state, float z_measure);

#endif  /*_KALMAN_FILTER_H*/

