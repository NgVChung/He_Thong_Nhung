#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

/* Cập nhật thông tin Lớp và Nhóm của bạn theo yêu cầu đề bài */
#define ID_LOP    "HTN"   // Thay bằng mã lớp của bạn
#define ID_NHOM   "N03"      // Thay bằng mã nhóm của bạn

uint32_t btn_counter = 0;
char tx_buffer[64];

/* 1. Khởi tạo nút nhấn tại chân PA0 (Pull-up) */
void Button_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // Mức cao khi nhả, mức 0 khi nhấn
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/* 2. Khởi tạo USART1 (chân TX PA9, 115200 baud) */
void USART1_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // Cấu hình chân PA9 làm chân USART1_TX (Alternate Function Push-Pull)
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Cấu hình thông số UART
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    // Kích hoạt đường phát DMA trên USART1
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    USART_Cmd(USART1, ENABLE);
}

/* 3. Truyền chuỗi qua DMA1 Channel 4 (không dùng hàm chờ CPU) */
void DMA_Send_Packet(const char *data, uint16_t length)
{
    // Cấp clock cho DMA1
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // Vô hiệu hóa kênh trước khi cấu hình gói mới
    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_DeInit(DMA1_Channel4);

    DMA_InitTypeDef DMA_InitStructure;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)data;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;          // Memory -> Peripheral
    DMA_InitStructure.DMA_BufferSize = length;                  // Số lượng byte truyền
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;     // Địa chỉ RAM tự tăng
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;               // Truyền 1 lần mỗi lần gọi
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);

    // Kích hoạt DMA: Phần cứng tự đẩy data ra TX, CPU tiếp tục chạy ngay không bị block
    DMA_Cmd(DMA1_Channel4, ENABLE);
}

/* Hàm trễ mềm khử rung phím */
void Simple_Delay(volatile uint32_t count)
{
    while (count--) {
        __NOP();
    }
}

int main(void)
{
    Button_Init();
    USART1_Init();

    while (1)
    {
        // Khi nhấn nút PA0 (mức logic 0 do kéo lên Pull-up)
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
        {
            Simple_Delay(72000 * 20); // Debounce ~20ms

            if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
            {
                // Chờ thả nút ra để tránh nhảy số liên tục khi giữ phím
                while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET);

                // 1. Tăng biến đếm nút nhấn
                btn_counter++;

                // 2. Tạo chuỗi bản tin theo định dạng: <ID-Lớp><ID-Nhóm>:BTN:<Giá trị nút nhấn>\n\r
                int len = snprintf(tx_buffer, sizeof(tx_buffer), "%s%s:BTN:%lu\n\r", 
                                   ID_LOP, ID_NHOM, btn_counter);

                // 3. Đẩy sang DMA truyền lên PC (không dùng vòng lặp chờ cờ UART)
                DMA_Send_Packet(tx_buffer, len);
            }
        }
    }
}

/* Hàm hỗ trợ cấp phát động newlib nano (khắc phục lỗi undefined reference to 'end') */
caddr_t _sbrk(int incr)
{
    extern char _ebss;
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_ebss;
    }
    prev_heap_end = heap_end;
    heap_end += incr;

    return (caddr_t) prev_heap_end;
}
