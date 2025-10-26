#include <stdio.h>
#include <string.h>
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_lpi2c.h"
#include "app.h"

#define MPU9250_ADDR        0x68
#define WHO_AM_I_REG        0x75
#define PWR_MGMT_1_REG      0x6B
#define ACCEL_XOUT_H        0x3B
#define I2C_BAUDRATE        100000U
#define LPI2C_MASTER_BASE   ((LPI2C_Type *)EXAMPLE_I2C_MASTER_BASE) // Adjust based on your board
#define CALIB_SAMPLE		1000U

lpi2c_master_handle_t g_m_handle;
volatile bool g_MasterCompletionFlag = false;
volatile bool g_MasterNackFlag = false;

volatile int32_t ax_sum = 0, ay_sum = 0, az_sum = 0, gx_sum = 0, gy_sum = 0, gz_sum = 0, temp_sum = 0;

/* Callback */
static void lpi2c_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_LPI2C_Nak)
        g_MasterNackFlag = true;
    else
        g_MasterCompletionFlag = true;
}

/* Write single register */
void MPU_WriteReg(uint8_t reg, uint8_t value)
{
    lpi2c_master_transfer_t masterXfer = {0};
    uint8_t data[2] = {reg, value};

    masterXfer.slaveAddress = MPU9250_ADDR;
    masterXfer.direction = kLPI2C_Write;
    masterXfer.data = data;
    masterXfer.dataSize = 2;
    masterXfer.flags = kLPI2C_TransferDefaultFlag;

    LPI2C_MasterTransferBlocking(LPI2C_MASTER_BASE, &masterXfer);
}

/* Read bytes from MPU */
void MPU_ReadRegs(uint8_t reg, uint8_t *data, uint8_t length)
{
    lpi2c_master_transfer_t masterXfer = {0};

    masterXfer.slaveAddress = MPU9250_ADDR;
    masterXfer.direction = kLPI2C_Read;
    masterXfer.subaddress = reg;
    masterXfer.subaddressSize = 1;
    masterXfer.data = data;
    masterXfer.dataSize = length;
    masterXfer.flags = kLPI2C_TransferDefaultFlag;

    LPI2C_MasterTransferBlocking(LPI2C_MASTER_BASE, &masterXfer);
}

int main(void)
{
    BOARD_InitHardware();
    LPI2C_MasterGetDefaultConfig(&(lpi2c_master_config_t){0});
    lpi2c_master_config_t masterConfig;
    LPI2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz = I2C_BAUDRATE;
    LPI2C_MasterInit(LPI2C_MASTER_BASE, &masterConfig, CLOCK_GetFreq(LPI2C_MASTER_CLOCK_FREQUENCY));

    PRINTF("Initializing MPU9250...\r\n");

    /* Reset and wake up MPU */
    MPU_WriteReg(PWR_MGMT_1_REG, 0x80);
    SDK_DelayAtLeastUs(100000, CLOCK_GetFreq(kCLOCK_CoreSysClk));
    MPU_WriteReg(PWR_MGMT_1_REG, 0x00);
    SDK_DelayAtLeastUs(100000, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    /* Verify connection */
    uint8_t whoami = 0;
    MPU_ReadRegs(WHO_AM_I_REG, &whoami, 1);
    PRINTF("WHO_AM_I = 0x%X\r\n", whoami);

    uint8_t sensorData[14];
    int16_t ax, ay, az, gx, gy, gz, temp;
    PRINTF("Calibrating MPU...\r\n");
    for (int i = 0; i < CALIB_SAMPLE; i++) {
    	MPU_ReadRegs(ACCEL_XOUT_H, sensorData, 14);
        ax = (sensorData[0] << 8) | sensorData[1];
        ay = (sensorData[2] << 8) | sensorData[3];
        az = (sensorData[4] << 8) | sensorData[5];
        temp = (sensorData[6] << 8) | sensorData[7];
        gx = (sensorData[8] << 8) | sensorData[9];
        gy = (sensorData[10] << 8) | sensorData[11];
        gz = (sensorData[12] << 8) | sensorData[13];

        ax_sum += ax;
        ay_sum += ay;
        az_sum += az;
        temp_sum += temp;
        gx_sum += gx;
        gy_sum += gy;
        gz_sum += gz;
	}

    int32_t ax_offset =((ax_sum-(ax_sum*0.05))/CALIB_SAMPLE);
    int32_t ay_offset =((ay_sum-(ay_sum*0.05))/CALIB_SAMPLE);
    int32_t az_offset =((az_sum-(az_sum*0.05))/CALIB_SAMPLE);
    int32_t temp_offset =((temp_sum-(temp_sum*0.05))/CALIB_SAMPLE);
    int32_t gx_offset =((gx_sum-(gx_sum*0.05))/CALIB_SAMPLE);
    int32_t gy_offset =((gy_sum-(gy_sum*0.05))/CALIB_SAMPLE);
    int32_t gz_offset =((gz_sum-(gz_sum*0.05))/CALIB_SAMPLE);

    PRINTF("\n\nOFFSETs\nACCEL: X=%d Y=%d Z=%d | GYRO: X=%d Y=%d Z=%d | TEMP=%d\r\n\n\n", ax_offset, ay_offset,
    		az_offset, gx_offset, gy_offset, gz_offset, temp_offset);
    while (1)
    {
        MPU_ReadRegs(ACCEL_XOUT_H, sensorData, 14);

        ax = ((sensorData[0] << 8) | sensorData[1]) - ax_offset;
        ay = ((sensorData[2] << 8) | sensorData[3]) - ay_offset;
        az = ((sensorData[4] << 8) | sensorData[5]) - az_offset;
        temp = ((sensorData[6] << 8) | sensorData[7]) - temp_offset;
        gx = ((sensorData[8] << 8) | sensorData[9]) - gx_offset;
        gy = ((sensorData[10] << 8) | sensorData[11]) - gy_offset;
        gz = ((sensorData[12] << 8) | sensorData[13]) - gz_offset;

        PRINTF("ACCEL: X=%d Y=%d Z=%d | GYRO: X=%d Y=%d Z=%d | TEMP=%d\r\n", ax, ay, az, gx, gy, gz, temp);

        SDK_DelayAtLeastUs(100000, CLOCK_GetFreq(kCLOCK_CoreSysClk)); // 100ms
    }
}
