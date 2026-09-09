#include "stm32f10x.h"

// Hàm delay đơn giản bằng vòng lặp vô hướng (dùng volatile để tránh bị GCC tối ưu hóa)
void delay_ms(volatile uint32_t delay) {
    while (delay--) {
        for (volatile int i = 0; i < 1000; i++) {
            __NOP(); // Lệnh No-Operation của ARM
        }
    }
}

int main(void) {
    RCC->APB2ENR |= (1 << 4);
  
    GPIOC->CRH &= ~(0xF << 20); // Clear 4 bit cấu hình của PC13
    GPIOC->CRH |=  (0x2 << 20); // Thiết lập MODE13 = 10 (Output 2MHz)

    while (1) {
        GPIOC->BSRR = (1 << 13);
        delay_ms(500);
        GPIOC->BRR = (1 << 13);
        delay_ms(500);
    }
    return 0;
}
