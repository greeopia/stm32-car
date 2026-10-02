#ifndef IMU_H
#define IMU_H

#include <stdint.h>
#include "stm32f10x.h"

/*
 * 硬件资源：I2C1重映射，PB8=SCL、PB9=SDA；接收使用DMA1_Channel7。
 * 初始化读ID、写寄存器使用有限次数轮询；运行时读取由I2C中断和DMA完成。
 * 接口在主循环中按顺序调用，不要从多个中断或任务中同时调用。
 *
 * 一帧流程：
 * IDLE -> IMU_StartRead() -> BUSY -> 中断配合IMU_Task() -> READY
 * READY -> IMU_ReadData() -> IDLE
 */

/*MPU6050地址和寄存器*/
#define MPU6050_ADDR             0xD0U     // AD0接低电平；7位地址0x68左移一位，供SPL使用
#define WHO_AM_I_REG             0x75U     // 器件ID，初始化期望读到0x68
#define PWR_MGMT_1_REG           0x6BU     // 电源管理：睡眠、时钟源等
#define SMPLRT_DIV_REG           0x19U     // 采样率分频
#define CONFIG_REG               0x1AU     // 数字低通滤波等配置
#define ACCEL_CONFIG_REG         0x1CU     // 加速度量程
#define ACCEL_XOUT_H_REG         0x3BU     // 14字节数据帧的起始寄存器
#define TEMP_OUT_H_REG           0x41U     // 温度数据高字节
#define GYRO_CONFIG_REG          0x1BU     // 角速度量程
#define GYRO_XOUT_H_REG           0x43U     // 角速度X轴高字节
#define RAD_TO_DEG               57.29577951308232f  // 弧度转角度

/*数据长度、超时和换算参数*/
#define IMU_FRAME_LENGTH         14U       // 加速度6字节 + 温度2字节 + 角速度6字节
#define IMU_TIMEOUT_MS           20U       // 一次运行时读取的超时阈值，单位ms
#define IMU_INIT_POLL_LIMIT      100000U   // 初始化每次等待的轮询次数上限，不是毫秒
#define IMU_ACCEL_SCALE          16384.0f  // 正负2g量程，X/Y轴换算：LSB/g
#define IMU_GYRO_SCALE           131.0f    // 正负250度/秒量程，换算：LSB/(度/秒)
/*
 * 待校准：14418.0f沿用旧驱动的Z轴修正值，不是正负2g量程的标准灵敏度。
 * 未做校准时，标准换算系数为16384.0f；请根据自己的模块实测决定。
 * 此系数目前仅影响Az；姿态计算仍使用原始加速度值。
 */
#define IMU_ACCEL_Z_SCALE        14418.0f

/*一帧解析后的数据*/
typedef struct
{
    int16_t Accel_X_RAW;          // X轴原始加速度，有符号16位
    int16_t Accel_Y_RAW;          // Y轴原始加速度
    int16_t Accel_Z_RAW;          // Z轴原始加速度
    float Ax;                    // X轴加速度，单位g
    float Ay;                    // Y轴加速度，单位g
    float Az;                    // Z轴加速度，单位g，使用上面的Z轴修正系数
    int16_t Gyro_X_RAW;           // X轴原始角速度，有符号16位
    int16_t Gyro_Y_RAW;           // Y轴原始角速度
    int16_t Gyro_Z_RAW;           // Z轴原始角速度
    float Gx;                    // X轴角速度，单位度/秒
    float Gy;                    // Y轴角速度，单位度/秒
    float Gz;                    // Z轴角速度，单位度/秒
    float Temperature;           // 芯片温度，单位摄氏度
    float KalmanAngleX;          // 滤波后的X轴倾角，单位度
    float KalmanAngleY;          // 滤波后的Y轴倾角，单位度；当前没有航向角输出
} MPU6050_t;

/*卡尔曼滤波器内部参数，实际参数在IMU.c的IMU_ResetFilters()中设置*/
typedef struct
{
    float Q_angle;               // 角度过程噪声参数
    float Q_bias;                // 陀螺仪零偏过程噪声参数
    float R_measure;             // 加速度倾角测量噪声参数
    float angle;                 // 估计角度
    float bias;                  // 估计陀螺仪零偏
    float P[2][2];               // 估计误差协方差矩阵
} Kalman_t;

/*模块状态*/
typedef enum
{
    IMU_STATUS_UNINITIALIZED = 0, // 尚未初始化
    IMU_STATUS_IDLE,              // 空闲，可以启动下一帧
    IMU_STATUS_BUSY,              // 正在通信，包含DMA结束后等待STOP完成的阶段
    IMU_STATUS_READY,             // 一帧就绪，等待IMU_ReadData()取出
    IMU_STATUS_ERROR              // 通信或初始化失败，处理原因后按需重新初始化
} IMU_Status_t;

/*错误原因*/
typedef enum
{
    IMU_ERROR_NONE = 0,           // 当前没有记录到硬件/通信错误
    IMU_ERROR_TIMEOUT,            // 初始化轮询或运行时读取超时
    IMU_ERROR_NACK,               // 从机未应答，优先检查接线、电源和地址
    IMU_ERROR_BUS,                // I2C总线错误或接收溢出
    IMU_ERROR_ARBITRATION,        // I2C仲裁丢失
    IMU_ERROR_DMA,                // DMA传输错误
    IMU_ERROR_DEVICE,             // WHO_AM_I读到的ID不符合预期
    IMU_ERROR_BUS_BUSY            // 启动读取时总线BUSY或上次STOP尚未结束
} IMU_Error_t;

/*
 * 最近一次成功解析的数据；初始化后为0。
 * IMU_ReadData()成功时更新，DMA中断不会直接更新它。
 * 本轮尚未成功解析时，这里保留上一帧，不能当作新数据使用。
 */
extern MPU6050_t imudata;

/*接口说明*/

/*
 * 功能：初始化I2C1、GPIO、DMA和NVIC，检查ID，配置MPU6050并重置滤波器。
 * 前提：先在应用中设置NVIC优先级分组，当前配置适合NVIC_PriorityGroup_2。
 * 默认：I2C 400kHz，加速度正负2g，角速度正负250度/秒，传感器采样率1kHz。
 * 返回：SUCCESS表示初始化成功，状态变为IDLE；失败返回ERROR。
 * 注意：此函数有有限次数轮询，适合上电初始化或故障恢复，不要每帧调用。
 *       BUSY期间调用会直接返回ERROR；重新初始化会清空旧数据和滤波状态。
 */
ErrorStatus IMU_init(void);

/*
 * 功能：启动一帧14字节读取，立即返回，后续I2C协议和接收由中断、DMA推进。
 * 参数：now_ms为当前累计毫秒数，应与IMU_Task()使用同一个计时源。
 * 返回：SUCCESS只表示已启动，状态变为BUSY，不代表数据已经读完。
 *       非IDLE或总线忙时返回ERROR；READY未取出的帧不会被覆盖。
 * 注意：总线忙时错误码为BUS_BUSY，但状态仍为IDLE，可稍后重试。
 *       此函数相当于每帧的启动入口，需要应用按采样周期主动调用。
 */
ErrorStatus IMU_StartRead(uint32_t now_ms);

/*
 * 功能：检查STOP是否结束，并检查读取是否超时；不循环等待。
 * 参数：now_ms为当前累计毫秒数，允许uint32_t计数正常回绕。
 * 调用：主循环每轮调用，即使当前处于BUSY也要调用，调用间隔应远小于20ms。
 * 注意：DMA中断只发出STOP；此函数确认STOP和总线BUSY清除后才置READY。
 *       不调用会影响完成状态和超时处理；不要在这里使用阻塞延时调度。
 */
void IMU_Task(uint32_t now_ms);

/*
 * 功能：取出READY状态的一帧，完成单位换算和X/Y倾角滤波。
 * 参数：I2Cx目前只能传I2C1；imuData为有效输出指针，可传&imudata。
 * 返回：成功返回SUCCESS，同时更新imudata和*imuData，状态变为IDLE。
 *       未READY、指针为空或I2Cx不是I2C1时返回ERROR，不消费该帧。
 * 注意：此函数不启动I2C通信、不等待DMA；一帧只能成功取出一次。
 *       换算和滤波在调用处执行，建议放在主循环，不放在中断中。
 */
ErrorStatus IMU_ReadData(I2C_TypeDef *I2Cx, MPU6050_t *imuData);

/*功能：查询当前状态；READY后调用IMU_ReadData()，IDLE时才启动下一帧。*/
IMU_Status_t IMU_GetStatus(void);

/*
 * 功能：查询最近记录的硬件/通信错误原因。
 * 注意：接口因状态不合适或参数错误而返回ERROR时，不一定设置错误码。
 *       因此ERROR返回值不能只靠本函数判断；还应检查调用状态和参数。
 *       重新初始化、成功启动读取或成功解析数据后，错误码会清为NONE。
 */
IMU_Error_t IMU_GetError(void);

/*
 * 主循环调用示例（这里只是说明，不会自动加入main.c）：
 * 使用工程现有的SysTickInit()/GetTick()，在main.c中包含SysTick.h、IMU.h。
 * GetTick()必须返回持续递增的毫秒计数，不是本次间隔或延时次数。
 *
 * NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
 * SysTickInit();
 * if (IMU_init() != SUCCESS) {
 *     // 用IMU_GetError()记录初始化错误，检查原因后再恢复。
 * }
 *
 * uint32_t last_read_ms = GetTick();
 * while (1) {
 *     uint32_t now_ms = GetTick();
 *     IMU_Task(now_ms);
 *
 *     if (IMU_GetStatus() == IMU_STATUS_READY) {
 *         if (IMU_ReadData(I2C1, &imudata) == SUCCESS) {
 *             // 此处才有新数据，可使用imudata.Gx、KalmanAngleX等。
 *         }
 *     }
 *
 *     if (IMU_GetStatus() == IMU_STATUS_IDLE &&
 *         (uint32_t)(now_ms - last_read_ms) >= 10U) {
 *         if (IMU_StartRead(now_ms) == SUCCESS) {
 *             last_read_ms = now_ms;    // 每10ms启动一帧，即应用读取频率100Hz
 *         }
 *     }
 *
 *     if (IMU_GetStatus() == IMU_STATUS_ERROR) {
 *         // 记录IMU_GetError()；处理故障后按需调用IMU_init()。
 *     }
 *     // 继续处理小车其他任务。
 * }
 */

/*
 * 需要上板调试/确认的地方：
 *
 * 1. 接线和地址
 *    PB8接SCL，PB9接SDA，电源正确、共地，确认SCL/SDA有合适的上拉。
 *    AD0低电平：7位地址0x68，MPU6050_ADDR用0xD0；高电平则用0xD2。
 *    WHO_AM_I仍应为0x68，不随AD0改变；不要把7位地址直接填进此宏。
 *    I2C频率在IMU_init()的I2C_ClockSpeed中；通信不稳定可先用100000排查。
 *
 * 2. 中断和DMA资源
 *    IMU.c已定义I2C1_EV_IRQHandler、I2C1_ER_IRQHandler、
 *    DMA1_Channel7_IRQHandler，工程中不能再有同名定义。
 *    检查DMA1_Channel7是否被其他模块占用，确认三个中断确实进入。
 *    当前DMA为Normal模式，每次读取重装14字节计数；不要只改成Circular。
 *
 * 3. 时基和完成状态
 *    先确认GetTick()确实每1ms递增，IMU_Task()在主循环持续执行。
 *    观察IDLE -> BUSY -> READY -> IDLE；一直BUSY时检查I2C/DMA中断、
 *    STOP是否结束及IMU_Task()是否执行，不能只等DMA完成标志。
 *    状态ERROR时先记录错误码，不要在每轮循环无条件重新初始化。
 *
 * 4. 超时参数
 *    IMU_TIMEOUT_MS是毫秒，按总线速度和应用调度评估，当前20ms。
 *    IMU_INIT_POLL_LIMIT只是轮询次数，实际耗时受CPU频率和优化等级影响。
 *
 * 5. 采样周期和低通滤波
 *    示例10ms是应用读取周期，不是传感器内部采样周期。
 *    IMU_init()当前CONFIG=0、SMPLRT_DIV=7，对应8kHz/(1+7)=1kHz。
 *    调整数字低通滤波后要重新计算分频；读取周期按小车控制需求调整。
 *    当前读取最新寄存器，没有FIFO缓存，未读取的中间采样不会保留。
 *
 * 6. 量程、零偏和比例系数
 *    修改ACCEL_CONFIG/GYRO_CONFIG量程时，要同步修改对应换算系数。
 *    优先确认上面的IMU_ACCEL_Z_SCALE；14418.0f不是通用校准结果。
 *    静止时角速度应接近0；加速度矢量模长应接近1g，单轴符号取决于姿态。
 *    当前没有单独进行陀螺仪静态零偏标定；抖动/漂移时先检查原始数据。
 *
 * 7. 安装方向和滤波
 *    核对传感器X/Y/Z方向与车体方向、角度正负，当前只输出X/Y倾角。
 *    在IMU_ResetFilters()中调Q_angle=0.001、Q_bias=0.003、R_measure=0.03。
 *    按实测噪声和响应速度调整；校准加速度时，姿态计算的原始值也需处理。
 *    滤波dt取相邻启动读取时间差；首帧或间隔超过0.5秒会重置滤波。
 *
 * 寄存器和换算依据：MPU-6000/MPU-6050 Register Map and Descriptions
 * https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf
 */

#endif
