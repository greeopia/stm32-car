/*
 * MPU6050 support consolidated into IMU.c / IMU.h for STM32F103 SPL.
 *
 * The frame conversion and Kalman update below are adapted from the
 * MPU6050 driver by Bulanov Konstantin (2019), copyright 2021.
 * Contact: leech001@gmail.com
 * Original Kalman algorithm: https://github.com/TKJElectronics/KalmanFilter
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "IMU.h"
#include <math.h>
#include <string.h>
#include "stm32f10x.h"                  // Device header

#define IMU_I2C_ERROR_BITS (I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_OVR)

typedef enum
{
    IMU_STAGE_IDLE = 0,
    IMU_STAGE_START_WRITE,
    IMU_STAGE_ADDRESS_WRITE,
    IMU_STAGE_REGISTER_SENT,
    IMU_STAGE_START_READ,
    IMU_STAGE_ADDRESS_READ,
    IMU_STAGE_DMA,
    IMU_STAGE_STOP
} IMU_Stage_t;

static volatile uint8_t imubuf[IMU_FRAME_LENGTH];
static volatile IMU_Status_t imu_status = IMU_STATUS_UNINITIALIZED;
static volatile IMU_Error_t imu_error = IMU_ERROR_NONE;
static volatile IMU_Stage_t imu_stage = IMU_STAGE_IDLE;
static uint32_t frame_start_ms;
static uint32_t previous_frame_ms;
static uint8_t filter_initialized;
static Kalman_t KalmanX;
static Kalman_t KalmanY;
MPU6050_t imudata;

static void MPU6050_ClearADDR(void)
{
    volatile uint32_t discard;
    discard = I2C1->SR1;
    discard = I2C1->SR2;
    (void)discard;
}

static IMU_Error_t MPU6050_CheckError(void)
{
    uint32_t flags = I2C1->SR1;
    if ((flags & I2C_SR1_ARLO) != 0U)
        return IMU_ERROR_ARBITRATION;
    if ((flags & I2C_SR1_AF) != 0U)
        return IMU_ERROR_NACK;
    if ((flags & (I2C_SR1_BERR | I2C_SR1_OVR)) != 0U)
        return IMU_ERROR_BUS;
    return IMU_ERROR_NONE;
}

static void MPU6050_Abort(IMU_Error_t error)
{
    uint32_t irq_state = __get_PRIMASK();
    __disable_irq();
    I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, DISABLE);
    I2C_DMACmd(I2C1, DISABLE);
    DMA_Cmd(DMA1_Channel7, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_GL7);
    /* After arbitration loss we no longer own the bus: do not send STOP. */
    if (error != IMU_ERROR_ARBITRATION && (I2C1->SR2 & I2C_SR2_MSL) != 0U)
        I2C_GenerateSTOP(I2C1, ENABLE);
    I2C_ClearFlag(I2C1, I2C_FLAG_AF | I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_OVR);
    I2C_DMALastTransferCmd(I2C1, DISABLE);
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    imu_error = error;
    imu_stage = IMU_STAGE_IDLE;
    imu_status = IMU_STATUS_ERROR;
    __set_PRIMASK(irq_state);
}

/* These bounded waits are used only during initialization, never by DMA reads. */
static ErrorStatus MPU6050_WaitFlag_Init(uint32_t flag)
{
    uint32_t polls = IMU_INIT_POLL_LIMIT;
    while (polls-- != 0U)
    {
        imu_error = MPU6050_CheckError();
        if (imu_error != IMU_ERROR_NONE)
            return ERROR;
        if (I2C_GetFlagStatus(I2C1, flag) == SET)
            return SUCCESS;
    }
    imu_error = IMU_ERROR_TIMEOUT;
    return ERROR;
}

static ErrorStatus MPU6050_WaitIdle_Init(void)
{
    uint32_t polls = IMU_INIT_POLL_LIMIT;
    while (polls-- != 0U)
    {
        if (((I2C1->SR2 & I2C_SR2_BUSY) == 0U) &&
            ((I2C1->CR1 & I2C_CR1_STOP) == 0U))
            return SUCCESS;
    }
    imu_error = IMU_ERROR_TIMEOUT;
    return ERROR;
}

static ErrorStatus MPU6050_SelectReg_Init(uint8_t reg)
{
    if (MPU6050_WaitIdle_Init() != SUCCESS)
        return ERROR;
    I2C_GenerateSTART(I2C1, ENABLE);
    if (MPU6050_WaitFlag_Init(I2C_FLAG_SB) != SUCCESS)
        return ERROR;
    I2C_Send7bitAddress(I2C1, MPU6050_ADDR, I2C_Direction_Transmitter);
    if (MPU6050_WaitFlag_Init(I2C_FLAG_ADDR) != SUCCESS)
        return ERROR;
    MPU6050_ClearADDR();
    I2C_SendData(I2C1, reg);
    return MPU6050_WaitFlag_Init(I2C_FLAG_BTF);
}

static ErrorStatus MPU6050_WriteReg_Init(uint8_t reg, uint8_t data)
{
    if (MPU6050_SelectReg_Init(reg) != SUCCESS)
        return ERROR;
    I2C_SendData(I2C1, data);
    if (MPU6050_WaitFlag_Init(I2C_FLAG_BTF) != SUCCESS)
        return ERROR;
    I2C_GenerateSTOP(I2C1, ENABLE);
    return MPU6050_WaitIdle_Init();
}

static ErrorStatus MPU6050_ReadReg_Init(uint8_t reg, uint8_t *data)
{
    uint32_t irq_state;
    if (MPU6050_SelectReg_Init(reg) != SUCCESS)
        return ERROR;
    I2C_GenerateSTART(I2C1, ENABLE);
    if (MPU6050_WaitFlag_Init(I2C_FLAG_SB) != SUCCESS)
        return ERROR;
    I2C_Send7bitAddress(I2C1, MPU6050_ADDR, I2C_Direction_Receiver);
    if (MPU6050_WaitFlag_Init(I2C_FLAG_ADDR) != SUCCESS)
        return ERROR;

    /* Single-byte reception: ACK=0 BEFORE clearing ADDR (RM0008 EV6_3). */
    irq_state = __get_PRIMASK();
    __disable_irq();
    I2C_AcknowledgeConfig(I2C1, DISABLE);
    MPU6050_ClearADDR();
    I2C_GenerateSTOP(I2C1, ENABLE);
    __set_PRIMASK(irq_state);

    if (MPU6050_WaitFlag_Init(I2C_FLAG_RXNE) != SUCCESS)
        return ERROR;
    *data = I2C_ReceiveData(I2C1);
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    return MPU6050_WaitIdle_Init();
}

static void IMU_ResetFilters(void)
{
    memset(&KalmanX, 0, sizeof(KalmanX));
    memset(&KalmanY, 0, sizeof(KalmanY));
    KalmanX.Q_angle = KalmanY.Q_angle = 0.001f;
    KalmanX.Q_bias = KalmanY.Q_bias = 0.003f;
    KalmanX.R_measure = KalmanY.R_measure = 0.03f;
    filter_initialized = 0U;
    previous_frame_ms = 0U;
}

ErrorStatus IMU_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    I2C_InitTypeDef I2C_InitStruct;
    DMA_InitTypeDef DMA_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    uint8_t id;

    if (imu_status == IMU_STATUS_BUSY)
        return ERROR;
    NVIC_DisableIRQ(I2C1_EV_IRQn);
    NVIC_DisableIRQ(I2C1_ER_IRQn);
    NVIC_DisableIRQ(DMA1_Channel7_IRQn);
    imu_status = IMU_STATUS_UNINITIALIZED;
    imu_error = IMU_ERROR_NONE;
    imu_stage = IMU_STAGE_IDLE;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    I2C_DeInit(I2C1);
    I2C_StructInit(&I2C_InitStruct);
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_ClockSpeed = 400000U;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStruct.I2C_OwnAddress1 = 0U;
    I2C_Init(I2C1, &I2C_InitStruct);
    I2C_Cmd(I2C1, ENABLE);

    DMA_DeInit(DMA1_Channel7);
    DMA_StructInit(&DMA_InitStruct);
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&I2C1->DR;
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)imubuf;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStruct.DMA_BufferSize = IMU_FRAME_LENGTH;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_Init(DMA1_Channel7, &DMA_InitStruct);
    DMA_ITConfig(DMA1_Channel7, DMA_IT_TC | DMA_IT_TE, ENABLE);

    if (MPU6050_ReadReg_Init(WHO_AM_I_REG, &id) != SUCCESS)
        goto init_failed;
    if (id != 0x68U)
    {
        imu_error = IMU_ERROR_DEVICE;
        goto init_failed;
    }
    /* Match the imported driver's ranges and 1 kHz sensor sample rate. */
    if (MPU6050_WriteReg_Init(PWR_MGMT_1_REG, 0x00U) != SUCCESS ||
        MPU6050_WriteReg_Init(CONFIG_REG, 0x00U) != SUCCESS ||
        MPU6050_WriteReg_Init(SMPLRT_DIV_REG, 0x07U) != SUCCESS ||
        MPU6050_WriteReg_Init(ACCEL_CONFIG_REG, 0x00U) != SUCCESS ||
        MPU6050_WriteReg_Init(GYRO_CONFIG_REG, 0x00U) != SUCCESS)
        goto init_failed;

    memset(&imudata, 0, sizeof(imudata));
    IMU_ResetFilters();
    DMA_ClearFlag(DMA1_FLAG_GL7);
    NVIC_ClearPendingIRQ(I2C1_EV_IRQn);
    NVIC_ClearPendingIRQ(I2C1_ER_IRQn);
    NVIC_ClearPendingIRQ(DMA1_Channel7_IRQn);
    NVIC_StructInit(&NVIC_InitStruct);
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1U;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannel = I2C1_ER_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0U;
    NVIC_Init(&NVIC_InitStruct);
    NVIC_InitStruct.NVIC_IRQChannel = I2C1_EV_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1U;
    NVIC_Init(&NVIC_InitStruct);
    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel7_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2U;
    NVIC_Init(&NVIC_InitStruct);
    imu_error = IMU_ERROR_NONE;
    imu_status = IMU_STATUS_IDLE;
    return SUCCESS;

init_failed:
    MPU6050_Abort(imu_error);
    return ERROR;
}

ErrorStatus IMU_StartRead(uint32_t now_ms)
{
    uint32_t irq_state = __get_PRIMASK();
    __disable_irq();
    if (imu_status != IMU_STATUS_IDLE)
    {
        __set_PRIMASK(irq_state);
        return ERROR;
    }
    if ((I2C1->SR2 & I2C_SR2_BUSY) != 0U ||
        (I2C1->CR1 & I2C_CR1_STOP) != 0U)
    {
        imu_error = IMU_ERROR_BUS_BUSY;
        __set_PRIMASK(irq_state);
        return ERROR;
    }

    DMA_Cmd(DMA1_Channel7, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_GL7);
    DMA_SetCurrDataCounter(DMA1_Channel7, IMU_FRAME_LENGTH);
    I2C_DMACmd(I2C1, DISABLE);
    I2C_DMALastTransferCmd(I2C1, DISABLE);
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    I2C_ClearFlag(I2C1, I2C_FLAG_AF | I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_OVR);
    frame_start_ms = now_ms;
    imu_error = IMU_ERROR_NONE;
    imu_stage = IMU_STAGE_START_WRITE;
    imu_status = IMU_STATUS_BUSY;
    I2C_ITConfig(I2C1, I2C_IT_BUF, DISABLE);
    I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_ERR, ENABLE);
    I2C_GenerateSTART(I2C1, ENABLE);
    __set_PRIMASK(irq_state);
    return SUCCESS;
}

void I2C1_EV_IRQHandler(void)
{
    uint32_t flags = I2C1->SR1;
    if (imu_status != IMU_STATUS_BUSY)
    {
        I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_BUF, DISABLE);
        return;
    }
    if ((flags & IMU_I2C_ERROR_BITS) != 0U)
    {
        MPU6050_Abort(MPU6050_CheckError());
        return;
    }

    switch (imu_stage)
    {
    case IMU_STAGE_START_WRITE:
        if ((flags & I2C_SR1_SB) != 0U)
        {
            imu_stage = IMU_STAGE_ADDRESS_WRITE;
            I2C_Send7bitAddress(I2C1, MPU6050_ADDR, I2C_Direction_Transmitter);
        }
        break;
    case IMU_STAGE_ADDRESS_WRITE:
        if ((flags & I2C_SR1_ADDR) != 0U)
        {
            MPU6050_ClearADDR();
            imu_stage = IMU_STAGE_REGISTER_SENT;
            I2C_SendData(I2C1, ACCEL_XOUT_H_REG);
        }
        break;
    case IMU_STAGE_REGISTER_SENT:
        if ((flags & I2C_SR1_BTF) != 0U)
        {
            imu_stage = IMU_STAGE_START_READ;
            I2C_GenerateSTART(I2C1, ENABLE);
        }
        break;
    case IMU_STAGE_START_READ:
        if ((flags & I2C_SR1_SB) != 0U)
        {
            imu_stage = IMU_STAGE_ADDRESS_READ;
            I2C_Send7bitAddress(I2C1, MPU6050_ADDR, I2C_Direction_Receiver);
        }
        break;
    case IMU_STAGE_ADDRESS_READ:
        if ((flags & I2C_SR1_ADDR) != 0U)
        {
            /* Configure DMA and LAST before clearing ADDR/releasing SCL. */
            I2C_AcknowledgeConfig(I2C1, ENABLE);
            I2C_DMALastTransferCmd(I2C1, ENABLE);
            DMA_Cmd(DMA1_Channel7, ENABLE);
            I2C_DMACmd(I2C1, ENABLE);
            I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_BUF, DISABLE);
            imu_stage = IMU_STAGE_DMA;
            MPU6050_ClearADDR();
        }
        break;
    default:
        break;
    }
}

void I2C1_ER_IRQHandler(void)
{
    IMU_Error_t error = MPU6050_CheckError();
    if (error != IMU_ERROR_NONE)
        MPU6050_Abort(error);
}

void DMA1_Channel7_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TE7) == SET)
    {
        DMA_ClearFlag(DMA1_FLAG_GL7);
        MPU6050_Abort(IMU_ERROR_DMA);
        return;
    }
    if (DMA_GetITStatus(DMA1_IT_TC7) == SET)
    {
        DMA_ClearFlag(DMA1_FLAG_GL7);
        if (imu_status == IMU_STATUS_BUSY && imu_stage == IMU_STAGE_DMA)
        {
            I2C_DMACmd(I2C1, DISABLE);
            DMA_Cmd(DMA1_Channel7, DISABLE);
            I2C_GenerateSTOP(I2C1, ENABLE);
            /* Retain LAST until STOP finishes, rather than changing the final ACK. */
            imu_stage = IMU_STAGE_STOP;
        }
    }
}

void IMU_Task(uint32_t now_ms)
{
    uint32_t irq_state = __get_PRIMASK();
    __disable_irq();
    if (imu_status == IMU_STATUS_BUSY)
    {
        if (imu_stage == IMU_STAGE_STOP &&
            (I2C1->CR1 & I2C_CR1_STOP) == 0U &&
            (I2C1->SR2 & I2C_SR2_BUSY) == 0U)
        {
            I2C_ITConfig(I2C1, I2C_IT_ERR, DISABLE);
            I2C_DMALastTransferCmd(I2C1, DISABLE);
            I2C_AcknowledgeConfig(I2C1, ENABLE);
            __DMB();
            imu_stage = IMU_STAGE_IDLE;
            imu_status = IMU_STATUS_READY;
        }
        else if ((uint32_t)(now_ms - frame_start_ms) >= IMU_TIMEOUT_MS)
        {
            MPU6050_Abort(IMU_ERROR_TIMEOUT);
        }
    }
    __set_PRIMASK(irq_state);
}

static float Kalman_getAngle(Kalman_t *kalman, float angle, float rate, float dt)
{
    float innovation;
    float covariance;
    float gain0;
    float gain1;
    float p00;
    float p01;

    kalman->angle += dt * (rate - kalman->bias);
    kalman->P[0][0] += dt * (dt * kalman->P[1][1] - kalman->P[0][1] -
                            kalman->P[1][0] + kalman->Q_angle);
    kalman->P[0][1] -= dt * kalman->P[1][1];
    kalman->P[1][0] -= dt * kalman->P[1][1];
    kalman->P[1][1] += kalman->Q_bias * dt;
    covariance = kalman->P[0][0] + kalman->R_measure;
    gain0 = kalman->P[0][0] / covariance;
    gain1 = kalman->P[1][0] / covariance;
    innovation = angle - kalman->angle;
    kalman->angle += gain0 * innovation;
    kalman->bias += gain1 * innovation;
    p00 = kalman->P[0][0];
    p01 = kalman->P[0][1];
    kalman->P[0][0] -= gain0 * p00;
    kalman->P[0][1] -= gain0 * p01;
    kalman->P[1][0] -= gain1 * p00;
    kalman->P[1][1] -= gain1 * p01;
    return kalman->angle;
}

static int16_t MPU6050_DecodeWord(uint8_t offset)
{
    return (int16_t)(((uint16_t)imubuf[offset] << 8) | imubuf[offset + 1U]);
}

static void IMU_UpdateAngles(MPU6050_t *data)
{
    /* Convert before multiplying to avoid signed overflow at large raw values. */
    float ax = (float)data->Accel_X_RAW;
    float ay = (float)data->Accel_Y_RAW;
    float az = (float)data->Accel_Z_RAW;
    float roll = atan2f(ay, sqrtf(ax * ax + az * az)) * RAD_TO_DEG;
    float pitch = atan2f(-ax, az) * RAD_TO_DEG;
    float dt = (float)(uint32_t)(frame_start_ms - previous_frame_ms) * 0.001f;
    float roll_rate = data->Gx;

    if (filter_initialized == 0U || dt > 0.5f)
    {
        IMU_ResetFilters();
        KalmanX.angle = roll;
        KalmanY.angle = pitch;
        data->KalmanAngleX = roll;
        data->KalmanAngleY = pitch;
        filter_initialized = 1U;
    }
    else
    {
        if (dt == 0.0f)
            dt = 0.001f;
        if ((pitch < -90.0f && KalmanY.angle > 90.0f) ||
            (pitch > 90.0f && KalmanY.angle < -90.0f))
        {
            KalmanY.angle = pitch;
            data->KalmanAngleY = pitch;
        }
        else
        {
            data->KalmanAngleY = Kalman_getAngle(&KalmanY, pitch, data->Gy, dt);
        }
        if (fabsf(data->KalmanAngleY) > 90.0f)
            roll_rate = -roll_rate;
        data->KalmanAngleX = Kalman_getAngle(&KalmanX, roll, roll_rate, dt);
    }
    previous_frame_ms = frame_start_ms;
}

ErrorStatus IMU_ReadData(I2C_TypeDef *I2Cx, MPU6050_t *imuData)
{
    MPU6050_t sample;
    uint32_t irq_state;
    if (I2Cx != I2C1 || imuData == NULL || imu_status != IMU_STATUS_READY)
        return ERROR;

    sample.Accel_X_RAW = MPU6050_DecodeWord(0U);
    sample.Accel_Y_RAW = MPU6050_DecodeWord(2U);
    sample.Accel_Z_RAW = MPU6050_DecodeWord(4U);
    sample.Gyro_X_RAW = MPU6050_DecodeWord(8U);
    sample.Gyro_Y_RAW = MPU6050_DecodeWord(10U);
    sample.Gyro_Z_RAW = MPU6050_DecodeWord(12U);
    sample.Ax = (float)sample.Accel_X_RAW / IMU_ACCEL_SCALE;
    sample.Ay = (float)sample.Accel_Y_RAW / IMU_ACCEL_SCALE;
    sample.Az = (float)sample.Accel_Z_RAW / IMU_ACCEL_Z_SCALE;
    sample.Gx = (float)sample.Gyro_X_RAW / IMU_GYRO_SCALE;
    sample.Gy = (float)sample.Gyro_Y_RAW / IMU_GYRO_SCALE;
    sample.Gz = (float)sample.Gyro_Z_RAW / IMU_GYRO_SCALE;
    sample.Temperature = (float)MPU6050_DecodeWord(6U) / 340.0f + 36.53f;
    IMU_UpdateAngles(&sample);

    irq_state = __get_PRIMASK();
    __disable_irq();
    imudata = sample;
    *imuData = sample;
    imu_error = IMU_ERROR_NONE;
    imu_status = IMU_STATUS_IDLE;
    __set_PRIMASK(irq_state);
    return SUCCESS;
}

IMU_Status_t IMU_GetStatus(void)
{
    return imu_status;
}

IMU_Error_t IMU_GetError(void)
{
    return imu_error;
}
