#ifndef IMU_H
#define IMU_H

#include <stdint.h>
#include "stm32f10x.h"

/* I2C1: PB8=SCL, PB9=SDA; MPU6050 AD0=0. */
#define MPU6050_ADDR             0xD0U
#define WHO_AM_I_REG             0x75U
#define PWR_MGMT_1_REG           0x6BU
#define SMPLRT_DIV_REG           0x19U
#define CONFIG_REG               0x1AU
#define ACCEL_CONFIG_REG         0x1CU
#define ACCEL_XOUT_H_REG         0x3BU
#define TEMP_OUT_H_REG           0x41U
#define GYRO_CONFIG_REG          0x1BU
#define GYRO_XOUT_H_REG           0x43U
#define RAD_TO_DEG               57.29577951308232f

#define IMU_FRAME_LENGTH         14U
#define IMU_TIMEOUT_MS           20U
#define IMU_INIT_POLL_LIMIT      100000U
#define IMU_ACCEL_SCALE          16384.0f
#define IMU_GYRO_SCALE           131.0f
/* Preserve the imported driver's Z-axis correction; calibrate for your board. */
#define IMU_ACCEL_Z_SCALE        14418.0f

typedef struct
{
    int16_t Accel_X_RAW;
    int16_t Accel_Y_RAW;
    int16_t Accel_Z_RAW;
    float Ax;
    float Ay;
    float Az;
    int16_t Gyro_X_RAW;
    int16_t Gyro_Y_RAW;
    int16_t Gyro_Z_RAW;
    float Gx;
    float Gy;
    float Gz;
    float Temperature;
    float KalmanAngleX;
    float KalmanAngleY;
} MPU6050_t;

typedef struct
{
    float Q_angle;
    float Q_bias;
    float R_measure;
    float angle;
    float bias;
    float P[2][2];
} Kalman_t;

typedef enum
{
    IMU_STATUS_UNINITIALIZED = 0,
    IMU_STATUS_IDLE,
    IMU_STATUS_BUSY,
    IMU_STATUS_READY,
    IMU_STATUS_ERROR
} IMU_Status_t;

typedef enum
{
    IMU_ERROR_NONE = 0,
    IMU_ERROR_TIMEOUT,
    IMU_ERROR_NACK,
    IMU_ERROR_BUS,
    IMU_ERROR_ARBITRATION,
    IMU_ERROR_DMA,
    IMU_ERROR_DEVICE,
    IMU_ERROR_BUS_BUSY
} IMU_Error_t;

extern MPU6050_t imudata;

/*
 * Initialization alone uses bounded polling. Runtime reads never wait.
 * Configure the application's NVIC priority grouping before IMU_init().
 * Default ranges: +/-2 g, +/-250 degrees/s; sensor sample rate: 1 kHz.
 *
 * Main-loop sequence (now_ms is your existing monotonic millisecond tick):
 *   if (IMU_init() != SUCCESS) { handle_initialization_error(); }
 *   ...
 *   IMU_Task(now_ms);                 // call regularly, including while BUSY
 *   if (IMU_GetStatus() == IMU_STATUS_READY)
 *       IMU_ReadData(I2C1, &imudata);  // consumes a completed frame
 *   if (sample_due && IMU_GetStatus() == IMU_STATUS_IDLE)
 *       IMU_StartRead(now_ms);        // returns immediately
 *
 * ERROR requires caller-directed recovery with IMU_init().
 * One completed frame is retained until IMU_ReadData() consumes it.
 * The module defines I2C1_EV, I2C1_ER and DMA1_Channel7 interrupt handlers.
 * SysTick and application scheduling remain owned by the application.
 */
ErrorStatus IMU_init(void);
ErrorStatus IMU_StartRead(uint32_t now_ms);
void IMU_Task(uint32_t now_ms);
ErrorStatus IMU_ReadData(I2C_TypeDef *I2Cx, MPU6050_t *imuData);
IMU_Status_t IMU_GetStatus(void);
IMU_Error_t IMU_GetError(void);

#endif
