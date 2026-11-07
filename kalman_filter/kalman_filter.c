/*
 * FileName : kalman_filter.c
 * Author   : xiahouzuoxin @163.com
 * Version  : v1.0
 * Date     : 2014/9/24 20:36:51
 * Brief    : 一维/二维卡尔曼滤波器的最小实现。
 * 
 * Copyright (C) MICL,USTB
 *
 * 来源：https://github.com/xiahouzuoxin/kalman_filter
 * 本工程只增加接口与迁移说明，没有修改原有可执行逻辑。
 *
 * 通用形式：
 *   预测：x = A*x
 *         P = A*P*A^T + Q
 *   更新：K = P*H^T*(H*P*H^T + R)^-1
 *         x = x + K*(z - H*x)
 *         P = (I - K*H)*P
 *
 * 一维实现把大写 P/H/Q/R 对应为 state->p/H/q/r。q 和 r 都是
 * “方差”，不是标准差，也不是示例 README 中可能出现的 3σ 门限。
 */
 
#include "kalman_filter.h"

/*
 * @brief   
 *   Init fields of structure @kalman1_state.
 *   I make some defaults in this init function:
 *     A = 1;
 *     H = 1; 
 *   and @q,@r are valued after prior tests.
 *
 *   NOTES: Please change A,H,q,r according to your application.
 *
 * @inputs  
 *   state - Klaman filter structure
 *   init_x - initial x state value   
 *   init_p - initial estimated error convariance
 * @outputs 
 * @retval  
 *
 * 中文说明：
 *   一维模型默认 A=1、H=1，意思是“状态在没有新信息时保持不变”，
 *   并且一次测量直接观测这个状态。它适合作为轮速等标量数据的
 *   起步实现，但原作者的 q=200、r=500 来自其示例数据，不能
 *   直接假定也适合本项目。
 *
 *   init_p 表示对 init_x 的不确定程度：p 越大，第一帧测量对状态
 *   的影响越强。init_x、init_p、q、r 使用与物理量一致的尺度。
 */
void kalman1_init(kalman1_state *state, float init_x, float init_p)
{
    state->x = init_x;
    state->p = init_p;
    state->A = 1;
    state->H = 1;
    // state->q = 2e2;//10e-6;  /* 过程噪声方差；仅示例默认值，迁移后通常要覆盖 */
    // state->r = 5e2;//10e-5;  /* 测量噪声方差；仅示例默认值，迁移后通常要覆盖 */
    state->q = 0.1f;
    state->r = 0.1f;
}

/*
 * @brief   
 *   1 Dimension Kalman filter
 * @inputs  
 *   state - Klaman filter structure
 *   z_measure - Measure value
 * @outputs 
 * @retval  
 *   Estimated result
 *
 * 中文说明：
 *   每来一帧新测量调用一次。函数先把旧状态预测到当前时刻，再用
 *   “测量值 - 预测测量值”形成 innovation，并按 gain 修正状态。
 *
 *   q 相对越大：更愿意接受状态变化，跟随快，但残余抖动更多。
 *   r 相对越大：更不相信测量，曲线更平滑，但响应更慢。
 *   p 与 gain 不是给人固定整定的参数；它们由滤波器每帧计算。
 *
 * 编码器使用示例（只说明调用关系，不在此处执行）：
 *   kalman1_state left_filter;   // 左右轮各一个独立状态
 *   kalman1_init(&left_filter, first_speed_mps, first_p);
 *   left_filter.q = q_speed_variance; // 在 init 之后覆盖默认 200
 *   left_filter.r = r_speed_variance; // 在 init 之后覆盖默认 500
 *   filtered_mps = kalman1_filter(&left_filter, raw_speed_mps);
 *
 * 原始速度应为：CNT 差值 -> 结合真实 dt 和每转脉冲数换算 m/s。
 * 滤波不能修正错 PPR、时间戳跳动、漏脉冲或 0/1 个脉冲造成的量化。
 */
float kalman1_filter(kalman1_state *state, float z_measure)
{
    /* 预测：A=1 时状态保持，p 增加过程噪声 q。 */
    state->x = state->A * state->x;
    state->p = state->A * state->A * state->p + state->q;  /* p(n|n-1)=A^2*p(n-1|n-1)+q */

    /*
     * 更新：gain 权衡“预测可信度 p”和“测量可信度 r”；
     * innovation = z - H*x；随后同时修正状态和误差方差 p。
     */
    state->gain = state->p * state->H / (state->p * state->H * state->H + state->r);
    state->x = state->x + state->gain * (z_measure - state->H * state->x);
    state->p = (1 - state->gain * state->H) * state->p;

    return state->x;
}

/*
 * @brief   
 *   Init fields of structure @kalman1_state.
 *   I make some defaults in this init function:
 *     A = {{1, 0.1}, {0, 1}};
 *     H = {1,0}; 
 *   and @q,@r are valued after prior tests. 
 *
 *   NOTES: Please change A,H,q,r according to your application.
 *
 * @inputs  
 * @outputs 
 * @retval  
 *
 * 中文说明：
 *   二维状态通常理解为 x[0]=被测量、x[1]=被测量的变化率。
 *   当前 A[0][1]=0.1 相当于每帧 x[0] 增加 0.1*x[1]；从原始
 *   库迁移时，它只是隐含的采样周期，不是要照搬的物理常数。
 *
 *   init_x 和 init_p 由调用者提供，而 A/H/q/r 会被本函数覆盖。
 *   若 x[0] 是角度（rad）、x[1] 是角速度（rad/s），A[0][1]
 *   就应是秒；若改角度单位为度，H、q、r 等也必须同步换算。
 *
 *   缺陷警告：本实现随后用逐元素公式近似 A*P*A^T 和
 *   (I-K*H)*P，并不满足通用矩阵乘法。修改 A/H 后尤其不能
 *   假设二维协方差仍然正确。
 */
void kalman2_init(kalman2_state *state, float *init_x, float (*init_p)[2])
{
    state->x[0]    = init_x[0];
    state->x[1]    = init_x[1];
    state->p[0][0] = init_p[0][0];
    state->p[0][1] = init_p[0][1];
    state->p[1][0] = init_p[1][0];
    state->p[1][1] = init_p[1][1];
    //state->A       = {{1, 0.1}, {0, 1}};
    state->A[0][0] = 1;
    state->A[0][1] = 0.1;             /* 等价于隐含 dt=0.1 s；迁移时改成真实采样周期 */
    state->A[1][0] = 0;
    state->A[1][1] = 1;
    //state->H       = {1,0};
    state->H[0]    = 1;
    state->H[1]    = 0;
    //state->q       = {{10e-6,0}, {0,10e-6}};  /* measure noise convariance */
    state->q[0]    = 10e-7;           /* 1e-6；示例默认值，单位应与 x[0]^2 一致 */
    state->q[1]    = 10e-7;           /* 1e-6；单位应与 x[1]^2 一致 */
    state->r       = 10e-7;           /* 测量噪声方差；单位应与 x[0]^2 一致 */
}

/*
 * @brief   
 *   2 Dimension kalman filter
 * @inputs  
 *   state - Klaman filter structure
 *   z_measure - Measure value
 * @outputs 
 *   state->x[0] - Updated state value, Such as angle,velocity
 *   state->x[1] - Updated state value, Such as diffrence angle, acceleration
 *   state->p    - Updated estimated error convatiance matrix
 * @retval  
 *   Return value is equals to state->x[0], so maybe angle or velocity.
 *
 * 中文说明：
 *   z_measure 只测量 state->x[0]；x[1] 通过状态转移矩阵与 x[0]
 *   关联，并由卡尔曼增益间接修正。调用频率必须与 A 中的 dt 配套。
 *
 * 当前实现缺陷（阅读和维护 API 时必须知道）：
 *   1. 状态预测先覆盖 x[0]，随后计算 x[1]；当 A[1][0]!=0 时，
 *      这会偷偷使用新的 x[0]，与 x=A*x 的矩阵语义不一致。
 *   2. 协方差预测没有完整计算 A*P*A^T+Q，例如省略 A 的平方和
 *      P 的交叉项，并且可能破坏 P 的对称性。
 *   3. 协方差更新把 (I-K*H)*P 写成了逐个元素分别乘，实际也不等价，
 *      当 H={1,0} 时交叉项和 p[1][1] 的更新同样不符合矩阵公式。
 *
 * 因此这版二维接口只能作为学习原型的参考。若用于 MPU6050 姿态，
 * 工程中现有 IMU.c 的角度+零偏模型更贴合该用途；若最终仍要使用
 * 本文件二维实现，应先修正上述矩阵运算，并用解析用例验证 P 的
 * 数值与对称性，再上板整定 q/r。本次只补注释，没有悄悄改算法。
 */
float kalman2_filter(kalman2_state *state, float z_measure)
{
    float temp0 = 0.0f;
    float temp1 = 0.0f;
    float temp = 0.0f;

    /* 第 1 步：状态预测。注意 x[0] 被先覆盖，见函数头缺陷说明。 */
    state->x[0] = state->A[0][0] * state->x[0] + state->A[0][1] * state->x[1];
    state->x[1] = state->A[1][0] * state->x[0] + state->A[1][1] * state->x[1];
    /* 第 2 步：协方差预测。这里的逐项公式并不是通用的 A*P*A^T+Q。 */
    state->p[0][0] = state->A[0][0] * state->p[0][0] + state->A[0][1] * state->p[1][0] + state->q[0];
    state->p[0][1] = state->A[0][0] * state->p[0][1] + state->A[1][1] * state->p[1][1];
    state->p[1][0] = state->A[1][0] * state->p[0][0] + state->A[0][1] * state->p[1][0];
    state->p[1][1] = state->A[1][0] * state->p[0][1] + state->A[1][1] * state->p[1][1] + state->q[1];

    /*
     * 第 3 步：求卡尔曼增益。
     * 对单路测量，S=H*P*H^T+r；gain=P*H^T/S。
     */
    /* gain = p * H^T * [r + H * p * H^T]^(-1), H^T means transpose. */
    temp0 = state->p[0][0] * state->H[0] + state->p[0][1] * state->H[1];
    temp1 = state->p[1][0] * state->H[0] + state->p[1][1] * state->H[1];
    temp  = state->r + state->H[0] * temp0 + state->H[1] * temp1;
    state->gain[0] = temp0 / temp;
    state->gain[1] = temp1 / temp;
    /* 第 4 步：用 innovation=(z-H*x) 修正两个状态分量。 */
    /* x(n|n) = x(n|n-1) + gain(n) * [z_measure - H(n)*x(n|n-1)]*/
    temp = state->H[0] * state->x[0] + state->H[1] * state->x[1];
    state->x[0] = state->x[0] + state->gain[0] * (z_measure - temp); 
    state->x[1] = state->x[1] + state->gain[1] * (z_measure - temp);

    /*
     * 第 5 步：理论上应计算 P=(I-K*H)*P；下面逐元素写法并非该矩阵
     * 运算，仅保留原始行为并明确标注。
     */
    /* Update @p: p(n|n) = [I - gain * H] * p(n|n-1) */
    state->p[0][0] = (1 - state->gain[0] * state->H[0]) * state->p[0][0];
    state->p[0][1] = (1 - state->gain[0] * state->H[1]) * state->p[0][1];
    state->p[1][0] = (1 - state->gain[1] * state->H[0]) * state->p[1][0];
    state->p[1][1] = (1 - state->gain[1] * state->H[1]) * state->p[1][1];

    return state->x[0];
}

