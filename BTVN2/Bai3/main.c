#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_usart.h"

static __IO uint32_t msTicks = 0;

void SysTick_Handler(void) {
    msTicks++;
}

void Delay_ms(uint32_t ms) {
    uint32_t start = msTicks;
    while ((msTicks - start) < ms);
}

void USART1_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // PA9 -> TX (Alternate function push-pull)
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio);

    // PA10 -> RX (Input floating)
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    USART_InitTypeDef usart;
    usart.USART_BaudRate = 9600;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &usart);

    USART_Cmd(USART1, ENABLE);
}

void UART1_SendChar(char c) {
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, c);
}

void UART1_SendString(const char *str) {
    while (*str) {
        UART1_SendChar(*str++);
    }
}

void ADC1_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

    // PA0 -> ADC Channel 0 (Analog Input)
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_0;
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &gpio);

    ADC_InitTypeDef adc;
    adc.ADC_Mode = ADC_Mode_Independent;
    adc.ADC_ScanConvMode = DISABLE;
    adc.ADC_ContinuousConvMode = DISABLE;
    adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc.ADC_DataAlign = ADC_DataAlign_Right;
    adc.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &adc);

    ADC_Cmd(ADC1, ENABLE);

    // Hiệu chỉnh ADC
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

uint16_t ADC1_Read(void) {
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    return ADC_GetConversionValue(ADC1);
}

// Hàm chuyển số nguyên thành chuỗi
void uint_to_str(uint32_t val, char *str) {
    char temp[11];
    int i = 0, j = 0;

    if (val == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    while (val > 0) {
        temp[i++] = (val % 10) + '0';
        val /= 10;
    }

    while (i > 0) {
        str[j++] = temp[--i];
    }
    str[j] = '\0';
}

int main(void) {
    SystemInit();
    SysTick_Config(SystemCoreClock / 1000);
    USART1_Init();
    ADC1_Init();

    char buf[12];

    while (1) {
        uint16_t raw_adc = ADC1_Read();
        
        // Tính toán điện áp dạng mV (VD: 3.3V = 3300mV)
        uint32_t mv = ((uint32_t)raw_adc * 3300) / 4095;
        uint32_t volt_int = mv / 1000;
        uint32_t volt_dec = (mv % 1000) / 10; // Lấy 2 chữ số thập phân

        // Gửi duy nhất kết quả điện áp qua UART
        UART1_SendString("Voltage: ");
        uint_to_str(volt_int, buf);
        UART1_SendString(buf);
        UART1_SendString(".");

        // Hiển thị chuẩn 2 chữ số thập phân
        UART1_SendChar((volt_dec / 10) + '0');
        UART1_SendChar((volt_dec % 10) + '0');
        UART1_SendString("V\r\n");

        Delay_ms(1000);
    }
}
