#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RX_BUFFER_SIZE 64

volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_index = 0;
volatile uint8_t command_ready = 0;

uint8_t led_status = 0;         // 0: OFF, 1: ON
uint16_t current_pwm_val = 0;   // Giá trị CCR1 (0 -> 1000)

void UART_SendString(const char *str) {
    while (*str) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, *str++);
    }
}

void TIM2_PWM_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;          
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;         
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;                    
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_Cmd(TIM2, ENABLE);
}

void USART1_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        char c = (char)USART_ReceiveData(USART1);
        
        if (c == '!') { 
            rx_buffer[rx_index] = '\0';
            command_ready = 1;
            rx_index = 0;
        } else if (rx_index < RX_BUFFER_SIZE - 1) {
            rx_buffer[rx_index++] = c;
        }
    }
}

void Process_Command(char *cmd) {
    char response[64];

    // Lệnh ON!
    if (strcmp(cmd, "ON") == 0) {
        led_status = 1;
        TIM_SetCompare1(TIM2, current_pwm_val); // Bật theo mức PWM cấu hình gần nhất
        UART_SendString("OK: ON!\r\n");
    } 
    // Lệnh OFF!
    else if (strcmp(cmd, "OFF") == 0) {
        led_status = 0;
        TIM_SetCompare1(TIM2, 0);               // Đưa đầu ra về 0
        UART_SendString("OK: OFF!\r\n");
    } 
    // Lệnh PWM:Percent%!
    else if (strncmp(cmd, "PWM:", 4) == 0) {
        int percent = atoi(cmd + 4);
        if (percent >= 0 && percent <= 100) {
            current_pwm_val = percent * 10;     // Lưu cấu hình gần nhất
            
            // Nếu đang ON thì thay đổi thực tế, nếu OFF chỉ đổi cấu hình
            if (led_status == 1) {
                TIM_SetCompare1(TIM2, current_pwm_val);
            }
            sprintf(response, "OK: PWM Set to %d%%!\r\n", percent);
            UART_SendString(response);
        } 
    } 
    // Lệnh Status!
    else if (strcmp(cmd, "Status") == 0) {
        int current_percent = current_pwm_val / 10;
        sprintf(response, "Status: LED=%s, PWM=%d%%!\r\n", (led_status ? "ON" : "OFF"), current_percent);
        UART_SendString(response);
    } 
}

int main(void) {
    SystemInit();
    TIM2_PWM_Init();
    USART1_Init();

    while (1) {
        if (command_ready) {
            Process_Command((char *)rx_buffer);
            command_ready = 0;
        }
    }
}
