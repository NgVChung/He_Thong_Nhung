#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "misc.h"

// Hàm gửi 1 ký tự qua USART1
void USART1_SendChar(char c)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, (uint16_t)c);
}

// Hàm gửi một chuỗi ký tự qua USART1
void USART1_SendString(char *str)
{
    while (*str)
    {
        USART1_SendChar(*str);
        str++;
    }
}

// Hàm khởi tạo GPIO và USART1
void USART1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 1. Bật clock cho GPIOA và USART1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // 2. Cấu hình chân PA9 làm TX (Alternate Function Push-Pull)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. Cấu hình chân PA10 làm RX (Input Floating)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 4. Cấu hình thông số USART1: 115200 bps, 8 bit data, 1 stop bit, no parity
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    // 5. Cho phép ngắt khi nhận dữ liệu (RXNE)
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 6. Cấu hình bộ điều khiển ngắt NVIC
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 7. Bật USART1
    USART_Cmd(USART1, ENABLE);
}

// Trình xử lý ngắt USART1
void USART1_IRQHandler(void)
{
    // Kiểm tra cờ ngắt RXNE (Receive Data Register Not Empty)
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        // Đọc dữ liệu nhận được từ PC
        char rx_char = (char)USART_ReceiveData(USART1);

        // Gửi chuỗi "Hello " phản hồi lại PC
        USART1_SendString("03-08: ");

        // Gửi ký tự PC vừa truyền tới (để xác nhận) và thêm ký tự xuống dòng
        USART1_SendChar(rx_char);
        USART1_SendString("\r\n");

        // Xóa cờ ngắt RXNE
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

int main(void)
{
    // Khởi tạo ngoại vi
    USART1_Init();

    // Thông báo khởi động lên PC
    USART1_SendString("STM32 Ready! Send any key...\r\n");

    while (1)
    {
        // Vòng lặp chính rảnh rỗi, việc truyền nhận được xử lý tự động trong Ngắt
    }
}
