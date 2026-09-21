#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_i2c.h"
#include <stdio.h>

#define BME280_I2C_ADDR         0xEC // Địa chỉ I2C BME280 (0x76 << 1), nếu SDO lên VCC thì đổi thành 0xEE (0x77 << 1)
#define BME280_REG_CALIB00      0x88
#define BME280_REG_ID           0xD0
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5
#define BME280_REG_PRESS_MSB    0xF7

// Các thông số hiệu chuẩn đọc từ BME280
uint16_t dig_T1;
int16_t  dig_T2, dig_T3;
uint16_t dig_P1;
int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
int32_t  t_fine;

/* --- Hàm Delay đơn giản --- */
void Delay_ms(volatile uint32_t ms) {
    ms *= 7200;
    while (ms--);
}

/* --- Khởi tạo USART1 (PA9-TX, PA10-RX) --- */
void USART1_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // GPIO PA9 (TX)
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // GPIO PA10 (RX)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // USART1: 9600-8-N-1
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

/* --- Gửi chuỗi qua UART --- */
void UART_SendString(const char *str) {
    while (*str) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, *str++);
    }
}

/* --- Khởi tạo I2C1 (PB6-SCL, PB7-SDA) --- */
void I2C1_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // PB6 (SCL), PB7 (SDA) -> Open-Drain Alternate Function
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    I2C_InitTypeDef I2C_InitStructure;
    I2C_InitStructure.I2C_ClockSpeed = 100000; // 100kHz Standard Mode
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitStructure);

    I2C_Cmd(I2C1, ENABLE);
}

/* --- Đọc 1 byte từ thanh ghi I2C --- */
uint8_t I2C_ReadReg(uint8_t devAddr, uint8_t regAddr) {
    uint8_t data;

    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C1, regAddr);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    I2C_AcknowledgeConfig(I2C1, DISABLE);
    I2C_GenerateSTOP(I2C1, ENABLE);

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    data = I2C_ReceiveData(I2C1);

    I2C_AcknowledgeConfig(I2C1, ENABLE);
    return data;
}

/* --- Đọc nhiều byte từ I2C --- */
void I2C_ReadMultiReg(uint8_t devAddr, uint8_t regAddr, uint8_t *pBuffer, uint16_t length) {
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C1, regAddr);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    while (length) {
        if (length == 1) {
            I2C_AcknowledgeConfig(I2C1, DISABLE);
            I2C_GenerateSTOP(I2C1, ENABLE);
        }
        if (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
            *pBuffer = I2C_ReceiveData(I2C1);
            pBuffer++;
            length--;
        }
    }
    I2C_AcknowledgeConfig(I2C1, ENABLE);
}

/* --- Ghi 1 byte vào thanh ghi I2C --- */
void I2C_WriteReg(uint8_t devAddr, uint8_t regAddr, uint8_t value) {
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C1, regAddr);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_SendData(I2C1, value);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTOP(I2C1, ENABLE);
}

/* --- Đọc các hệ số hiệu chuẩn của BME280 --- */
void BME280_ReadCalibration(void) {
    uint8_t calib[24];
    I2C_ReadMultiReg(BME280_I2C_ADDR, BME280_REG_CALIB00, calib, 24);

    dig_T1 = (uint16_t)(calib[1] << 8) | calib[0];
    dig_T2 = (int16_t)(calib[3] << 8) | calib[2];
    dig_T3 = (int16_t)(calib[5] << 8) | calib[4];

    dig_P1 = (uint16_t)(calib[7] << 8) | calib[6];
    dig_P2 = (int16_t)(calib[9] << 8) | calib[8];
    dig_P3 = (int16_t)(calib[11] << 8) | calib[10];
    dig_P4 = (int16_t)(calib[13] << 8) | calib[12];
    dig_P5 = (int16_t)(calib[15] << 8) | calib[14];
    dig_P6 = (int16_t)(calib[17] << 8) | calib[16];
    dig_P7 = (int16_t)(calib[19] << 8) | calib[18];
    dig_P8 = (int16_t)(calib[21] << 8) | calib[20];
    dig_P9 = (int16_t)(calib[23] << 8) | calib[22];
}

/* --- Khởi tạo BME280 --- */
uint8_t BME280_Init(void) {
    uint8_t id = I2C_ReadReg(BME280_I2C_ADDR, BME280_REG_ID);
    if (id != 0x60) { // ID mặc định của BME280 là 0x60 (BMP280 là 0x58)
        return 0; // Lỗi không tìm thấy cảm biến
    }

    BME280_ReadCalibration();

    // Cấu hình Oversampling: Temp x1, Press x1, Mode Normal
    // ctrl_meas (0xF4): osrs_t[7:5] | osrs_p[4:2] | mode[1:0]
    I2C_WriteReg(BME280_I2C_ADDR, BME280_REG_CTRL_MEAS, (1 << 5) | (1 << 2) | 3);
    I2C_WriteReg(BME280_I2C_ADDR, BME280_REG_CONFIG, 0x00); // Standby 0.5ms, Filter OFF

    return 1;
}

/* --- Thuật toán bù nhiệt độ theo Datasheet của Bosch --- */
float BME280_Compensate_Temperature(int32_t adc_T) {
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (float)((t_fine * 5 + 128) >> 8) / 100.0f;
}

/* --- Thuật toán bù áp suất theo Datasheet của Bosch --- */
float BME280_Compensate_Pressure(int32_t adc_P) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) return 0.0f; // Tránh lỗi chia cho 0

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

    return (float)p / 25600.0f; // Đơn vị: hPa (hPa = Pa / 100)
}

/* --- Đọc dữ liệu Nhiệt độ & Áp suất --- */
void BME280_ReadRaw(float *temp, float *press) {
    uint8_t data[6];
    I2C_ReadMultiReg(BME280_I2C_ADDR, BME280_REG_PRESS_MSB, data, 6);

    int32_t adc_P = (int32_t)(((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | (data[2] >> 4));
    int32_t adc_T = (int32_t)(((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | (data[5] >> 4));

    *temp = BME280_Compensate_Temperature(adc_T);
    *press = BME280_Compensate_Pressure(adc_P);
}

int main(void) {
    SystemInit();
    USART1_Init();
    I2C1_Init();

    Delay_ms(100);

    if (!BME280_Init()) {
        UART_SendString("ERR: BME280 NOT FOUND!\r\n");
        while (1);
    }

    UART_SendString("BME280 Ready!\r\n");

    float temperature = 0.0f;
    float pressure = 0.0f;
    char buffer[64];

    while (1) {
        BME280_ReadRaw(&temperature, &pressure);

        // Đóng gói chuỗi hiển thị
        int temp_int = (int)temperature;
        int temp_dec = (int)((temperature - temp_int) * 100);
        int press_int = (int)pressure;
        int press_dec = (int)((pressure - press_int) * 100);

        sprintf(buffer, "Temp: %d.%02d C | Press: %d.%02d hPa\r\n", 
                temp_int, (temp_dec < 0 ? -temp_dec : temp_dec), 
                press_int, (press_dec < 0 ? -press_dec : press_dec));

        UART_SendString(buffer);

        Delay_ms(1000); // Đọc và gửi mỗi 1 giây
    }
}
