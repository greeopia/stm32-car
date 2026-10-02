#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "../../../User/IMU.c"

static void reset_logs(void)
{
    mock_address_count=0; mock_data_count=0;
    mock_start_count=0; mock_stop_count=0;
    mock_defer_stop=1; mock_dma_pending=0;
}
static void complete_stop(void)
{
    I2C1->CR1 &= ~I2C_CR1_STOP;
    I2C1->SR2 = 0;
}
static void initialize_ok(void)
{
    mock_defer_stop=0; mock_stall_start=0; mock_id=0x68;
    mock_address_count=mock_data_count=0;
    assert(IMU_init()==SUCCESS);
    assert(IMU_GetStatus()==IMU_STATUS_IDLE);
    assert(IMU_GetError()==IMU_ERROR_NONE);
    assert(!mock_dma_enabled && !mock_dma_request);
    assert(mock_gpio_config.GPIO_Pin==(GPIO_Pin_8|GPIO_Pin_9));
    assert(mock_i2c_config.I2C_ClockSpeed==400000);
    assert(mock_i2c_config.I2C_AcknowledgedAddress==I2C_AcknowledgedAddress_7bit);
    assert(mock_dma_config.DMA_PeripheralBaseAddr==(uint32_t)(uintptr_t)&I2C1->DR);
    assert(mock_dma_config.DMA_PeripheralDataSize==DMA_PeripheralDataSize_Byte);
    assert(mock_dma_config.DMA_MemoryDataSize==DMA_MemoryDataSize_Byte);
    assert(mock_dma_config.DMA_Mode==DMA_Mode_Normal);
    assert(mock_dma_config.DMA_BufferSize==14);
    reset_logs();
}
static void drive_read_address(void)
{
    I2C1_EV_IRQHandler(); // SB -> write address
    I2C1_EV_IRQHandler(); // ADDR -> register address
    I2C1_EV_IRQHandler(); // BTF -> repeated START
    I2C1_EV_IRQHandler(); // SB -> read address
    I2C1_EV_IRQHandler(); // ADDR -> DMA, with LAST set
    assert(mock_start_count==2);
    assert(mock_address_count==2);
    assert(mock_addresses[0]==0xD0 && mock_addresses[1]==0xD0);
    assert(mock_directions[0]==I2C_Direction_Transmitter);
    assert(mock_directions[1]==I2C_Direction_Receiver);
    assert(mock_data_count==1 && mock_data[0]==0x3B);
    assert(mock_dma_enabled && mock_dma_request && mock_last);
    assert(!(mock_i2c_it & (I2C_IT_EVT|I2C_IT_BUF)));
    assert(imu_stage==IMU_STAGE_DMA);
}
static void dma_complete(uint32_t now)
{
    mock_dma.CNDTR=0;
    mock_dma_pending=DMA1_IT_TC7;
    DMA1_Channel7_IRQHandler();
    assert(IMU_GetStatus()==IMU_STATUS_BUSY);
    assert(!mock_dma_enabled && !mock_dma_request);
    assert(mock_last); // retained while the final STOP is still in progress
    IMU_Task(now);
    assert(IMU_GetStatus()==IMU_STATUS_BUSY);
    complete_stop();
    IMU_Task(now+1);
    assert(IMU_GetStatus()==IMU_STATUS_READY);
    assert(!mock_last);
}
static void test_normal_frame(void)
{
    MPU6050_t output;
    uint8_t input[14]={0x80,0x00, 0x7F,0xFF, 0x40,0x00,
                       0x01,0x54, 0x00,0x83, 0xFE,0xFA, 0x00,0x00};
    unsigned before;
    initialize_ok();
    before=mock_flag_reads;
    assert(IMU_StartRead(100)==SUCCESS);
    assert(mock_flag_reads==before); // no polling event waits in the runtime API
    assert(IMU_StartRead(101)==ERROR); // an active transaction cannot be overwritten
    drive_read_address();
    memcpy((void *)imubuf,input,sizeof(input));
    dma_complete(101);
    assert(IMU_StartRead(102)==ERROR); // preserve an unconsumed frame
    assert(IMU_ReadData(I2C1,NULL)==ERROR);
    assert(IMU_ReadData(I2C2,&output)==ERROR);
    assert(IMU_GetStatus()==IMU_STATUS_READY);
    assert(IMU_ReadData(I2C1,&output)==SUCCESS);
    assert(output.Accel_X_RAW==-32768 && output.Accel_Y_RAW==32767);
    assert(output.Accel_Z_RAW==16384);
    assert(output.Gyro_X_RAW==131 && output.Gyro_Y_RAW==-262);
    assert(fabsf(output.Gx-1.0f)<0.0001f && fabsf(output.Gy+2.0f)<0.0001f);
    assert(fabsf(output.Temperature-37.53f)<0.0001f);
    assert(isfinite(output.KalmanAngleX) && isfinite(output.KalmanAngleY));
    assert(imudata.Accel_X_RAW==output.Accel_X_RAW && imudata.Gx==output.Gx);
    assert(imudata.KalmanAngleX==output.KalmanAngleX);
    assert(IMU_GetStatus()==IMU_STATUS_IDLE);
    assert(IMU_ReadData(I2C1,&output)==ERROR);

    reset_logs();
    assert(IMU_StartRead(110)==SUCCESS);
    assert(mock_dma.CNDTR==14); // normal-mode DMA count is reloaded for the next frame
    drive_read_address();
    memset((void *)imubuf,0,sizeof(imubuf)); imubuf[4]=0x40;
    dma_complete(111);
    assert(IMU_ReadData(I2C1,&output)==SUCCESS);
    assert(isfinite(output.KalmanAngleX) && isfinite(output.KalmanAngleY));
    puts("PASS: frame lifecycle, signed data, units, filter and DMA rearm");
}
static void test_timeout_wrap(void)
{
    initialize_ok();
    assert(IMU_StartRead(UINT32_MAX-10U)==SUCCESS);
    IMU_Task(8); // 19 ms elapsed across wraparound
    assert(IMU_GetStatus()==IMU_STATUS_BUSY);
    IMU_Task(9); // 20 ms elapsed
    assert(IMU_GetStatus()==IMU_STATUS_ERROR);
    assert(IMU_GetError()==IMU_ERROR_TIMEOUT);
    assert(!mock_dma_enabled && !mock_dma_request);
    assert(mock_primask==0);
    puts("PASS: nonblocking timeout and millisecond counter wraparound");
}
static void test_error_paths(void)
{
    unsigned stops;
    initialize_ok();
    assert(IMU_StartRead(200)==SUCCESS);
    I2C1->SR1=I2C_SR1_AF;
    I2C1_ER_IRQHandler();
    assert(IMU_GetStatus()==IMU_STATUS_ERROR && IMU_GetError()==IMU_ERROR_NACK);
    assert(!mock_i2c_it && !mock_dma_enabled && !mock_dma_request);

    initialize_ok();
    assert(IMU_StartRead(300)==SUCCESS);
    stops=mock_stop_count;
    I2C1->SR1=I2C_SR1_ARLO; I2C1->SR2=I2C_SR2_MSL|I2C_SR2_BUSY;
    I2C1_ER_IRQHandler();
    assert(IMU_GetError()==IMU_ERROR_ARBITRATION);
    assert(mock_stop_count==stops); // no STOP when bus ownership has been lost

    initialize_ok();
    assert(IMU_StartRead(400)==SUCCESS);
    drive_read_address();
    mock_dma_pending=DMA1_IT_TE7|DMA1_IT_TC7;
    DMA1_Channel7_IRQHandler();
    assert(IMU_GetStatus()==IMU_STATUS_ERROR && IMU_GetError()==IMU_ERROR_DMA);
    assert(!mock_dma_enabled && !mock_dma_request);
    assert(mock_dma_pending==0);

    initialize_ok();
    I2C1->SR2=I2C_SR2_BUSY;
    assert(IMU_StartRead(500)==ERROR);
    assert(IMU_GetStatus()==IMU_STATUS_IDLE && IMU_GetError()==IMU_ERROR_BUS_BUSY);
    assert(mock_start_count==0);
    complete_stop();
    assert(IMU_StartRead(501)==SUCCESS);
    assert(IMU_GetError()==IMU_ERROR_NONE);
    assert(IMU_init()==ERROR); // initialization cannot reset an active read
    IMU_Task(521);
    assert(IMU_GetStatus()==IMU_STATUS_ERROR);
    puts("PASS: NACK, arbitration loss, DMA errors and busy-bus rejection");
}
static void test_initialization_failure(void)
{
    initialize_ok();
    mock_defer_stop=0; mock_address_count=mock_data_count=0; mock_id=0x69;
    assert(IMU_init()==ERROR);
    assert(IMU_GetError()==IMU_ERROR_DEVICE);
    mock_id=0x68; mock_stall_start=1; mock_address_count=mock_data_count=0;
    assert(IMU_init()==ERROR);
    assert(IMU_GetError()==IMU_ERROR_TIMEOUT);
    assert(mock_primask==0);
    puts("PASS: incorrect sensor ID and bounded initialization timeout");
}
int main(void)
{
    assert(IMU_GetStatus()==IMU_STATUS_UNINITIALIZED);
    assert(IMU_StartRead(0)==ERROR);
    test_normal_frame();
    test_timeout_wrap();
    test_error_paths();
    test_initialization_failure();
    puts("All host simulations passed. Hardware bus timing is not simulated.");
    return 0;
}
