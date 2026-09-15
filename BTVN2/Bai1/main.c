#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "misc.h"

#define RX_BUFFER_SIZE 128

// Cấu hình Mã lớp và Mã nhóm của bạn tại đây
#define MA_LOP   "HTN"   // Thay bằng Mã lớp thực tế của bạn
#define MA_NHOM  "N03"         // Thay bằng Mã nhóm thực tế của bạn

char rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_index = 0;

// Hàm gửi 1 ký tự qua USART1
void USART1_SendChar(char c)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, (uint16_t)c);
}

// Hàm gửi một chuỗi ký tự qua USART1
void USART1_SendString(const char *str)
{
    while (*str)
    {
        USART1_SendChar(*str);
        str++;
    }
}

// Cấu hình USART1 ở chế độ cả TX và RX
void USART1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 1. Bật clock cho GPIOA và USART1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // 2. PA9 - USART1_TX (Alternate Function Push-Pull)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. PA10 - USART1_RX (Input Floating)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 4. Cấu hình thông số USART1: 115200 bps, 8 data bits, 1 stop bit, no parity
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // Chế độ cả TX và RX
    USART_Init(USART1, &USART_InitStructure);

    // 5. Bật ngắt khi nhận dữ liệu (RXNE)
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 6. Cấu hình ngắt NVIC
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 7. Cho phép USART1 hoạt động
    USART_Cmd(USART1, ENABLE);
}

// Trình xử lý ngắt USART1
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        char rx_char = (char)USART_ReceiveData(USART1);

        // Kiểm tra ký tự đánh dấu kết thúc '!'
        if (rx_char == '!')
        {
            if (rx_index > 0)
            {
                rx_buffer[rx_index] = '\0'; // Thêm ký tự kết thúc chuỗi C

                // Gửi lại PC định dạng: <Mã lớp><Mã nhóm>: <Bản tin đã nhận từ PC>\n\r
                USART1_SendString(MA_LOP);
                USART1_SendString(MA_NHOM);
                USART1_SendString(": ");
                USART1_SendString(rx_buffer);
                USART1_SendString("\n\r");

                // Reset chỉ số bộ đệm để nhận bản tin tiếp theo
                rx_index = 0;
            }
        }
        else
        {
            // Lưu ký tự vào bộ đệm nếu chưa tràn bộ đệm
            if (rx_index < RX_BUFFER_SIZE - 1)
            {
                rx_buffer[rx_index++] = rx_char;
            }
        }

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

int main(void)
{
    USART1_Init();

    while (1)
    {
        // Luồng chính rảnh rỗi, việc nhận và phản hồi xử lý hoàn toàn trong Ngắt
    }
}
