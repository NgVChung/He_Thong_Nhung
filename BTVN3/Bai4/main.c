#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include <stdio.h>

#define SAMPLES_PER_SECOND   100
#define BUFFER_SIZE          (SAMPLES_PER_SECOND * 2) // 200 mẫu (2 nửa x 100 mẫu = 1s dữ liệu)

// Bộ đệm lưu dữ liệu ADC từ DMA
volatile uint16_t adc_buffer[BUFFER_SIZE];

// Cờ báo hiệu xử lý dữ liệu cho main loop
volatile uint8_t flag_send_half1 = 0; // Cờ gửi nửa đầu (Index 0 -> 99)
volatile uint8_t flag_send_half2 = 0; // Cờ gửi nửa sau (Index 100 -> 199)

/* --- 1. Cấu hình UART1 (PA9 - TX) --- */
void USART1_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    // PA9: USART1_TX (Alternate Function Push-Pull)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA10: USART1_RX (Input Floating)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

/* --- Hàm trợ giúp gửi chuỗi qua UART1 --- */
void UART_SendString(const char *str) {
    while (*str) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, *str++);
    }
}

/* --- 2. Cấu hình GPIO PA0 làm Kênh đầu vào Analog (ADC1 Channel 0) --- */
void GPIO_ADC_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; // PA0
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // Analog Input
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/* --- 3. Cấu hình Timer 3 tần số 100Hz phát xung Trigger TRGO --- */
void TIM2_Init(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    // Clock TIM2 = 72 MHz. Prescaler = 720-1 -> Clock đếm = 100 kHz
    TIM_TimeBaseStructure.TIM_Prescaler = 720 - 1;
    // Period (ARR) = 1000-1 -> Tần số tràn = 100 kHz / 1000 = 100 Hz
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // Phát sự kiện Update Event làm tín hiệu TRGO kích hoạt ADC
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);

    TIM_Cmd(TIM3, ENABLE);
}

/* --- 4. Cấu hình DMA1 Channel 1 lưu dữ liệu ADC vào RAM --- */
void DMA1_Init(void) {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA1_Channel1);
    DMA_InitTypeDef DMA_InitStructure;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR); // Địa chỉ thanh ghi ADC_DR
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_buffer;       // Địa chỉ mảng trong RAM
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                 // Đọc từ ngoại vi ghi vào RAM
    DMA_InitStructure.DMA_BufferSize = BUFFER_SIZE;                    // 200 mẫu
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;    // Cố định địa chỉ ADC
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;            // Tăng địa chỉ RAM
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // 16-bit
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         // 16-bit
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                    // Tự động ghi vòng tròn
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    // Bật ngắt Half-Transfer (HT) và Transfer-Complete (TC)
    DMA_ITConfig(DMA1_Channel1, DMA_IT_HT | DMA_IT_TC, ENABLE);

    // Cấu hình NVIC cho ngắt DMA1 Channel 1
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DMA_Cmd(DMA1_Channel1, ENABLE);
}

/* --- 5. Cấu hình ADC1 chạy Trigger ngoài từ TIM2 --- */
void ADC1_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); // ADCCLK = 72MHz / 6 = 12MHz (< 14MHz)

    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // Không chạy liên tục, chờ Trigger từ TIM3
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO; // Kích hoạt bằng TIM3 TRGO
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    // Cấu hình Kênh 0 (PA0), thời gian lấy mẫu 55.5 cycles
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);

    // Bật DMA cho ADC1
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);

    // Hiệu chuẩn ADC (Calibration)
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));

    // Bật kích hoạt từ bên ngoài cho ADC1
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);
}

/* --- 6. Trình phục vụ ngắt DMA1 Channel 1 (Half-Transfer & Transfer-Complete) --- */
void DMA1_Channel1_IRQHandler(void) {
    // Kiểm tra ngắt Half-Transfer Complete (HT) -> Đã nạp đủ 100 mẫu nửa đầu
    if (DMA_GetITStatus(DMA1_IT_HT1) != RESET) {
        flag_send_half1 = 1; // Bật cờ xử lý nửa đầu (Index 0 -> 99)
        DMA_ClearITPendingBit(DMA1_IT_HT1);
    }

    // Kiểm tra ngắt Transfer-Complete (TC) -> Đã nạp đủ 100 mẫu nửa sau
    if (DMA_GetITStatus(DMA1_IT_TC1) != RESET) {
        flag_send_half2 = 1; // Bật cờ xử lý nửa sau (Index 100 -> 199)
        DMA_ClearITPendingBit(DMA1_IT_TC1);
    }
}

/* --- Hàm gửi dữ liệu từ Buffer lên PC qua UART --- */
void Send_ADC_Data_To_PC(uint16_t start_index, uint16_t length) {
    char str_buf[32];
    for (uint16_t i = start_index; i < (start_index + length); i++) {
        // Định dạng dữ liệu dạng số, ngắt nhau bởi \n\r theo đúng yêu cầu
        sprintf(str_buf, "%d\n\r", adc_buffer[i]);
        UART_SendString(str_buf);
    }
}

/* --- 7. Hàm chính (Main Loop) --- */
int main(void) {
    SystemInit();
    
    USART1_Init();
    GPIO_ADC_Init();
    DMA1_Init();
    ADC1_Init();
    TIM2_Init(); // Bật Timer2 bắt đầu đếm và bắn Trigger

    while (1) {
        // Xử lý an toàn nửa đầu bộ đệm khi nhận ngắt HT (0.5 giây đầu)
        if (flag_send_half1) {
            Send_ADC_Data_To_PC(0, SAMPLES_PER_SECOND);
            flag_send_half1 = 0;
        }

        // Xử lý an toàn nửa sau bộ đệm khi nhận ngắt TC (0.5 giây sau)
        if (flag_send_half2) {
            Send_ADC_Data_To_PC(SAMPLES_PER_SECOND, SAMPLES_PER_SECOND);
            flag_send_half2 = 0;
        }
    }
}
