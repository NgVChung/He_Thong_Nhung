#include "stm32f10x.h"

void delay_ms(volatile uint32_t ms) {
    while (ms--) {
        for (volatile uint32_t i = 0; i < 1500; i++) {
            __asm__ volatile ("nop");
        }
    }
}

int main(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL = 0x22222222; // Cấu hình PA0-PA7 là Output Push-Pull 2MHz

    GPIOA->ODR &= ~0x00FF;   // Tắt 8 LED ban đầu
    
    uint32_t step_delay_ms = 1200; 

    while (1) {
        /* 1. Chạy từ PA0 -> PA7 */
        for (int i = 0; i < 8; i++) {
            GPIOA->ODR = (GPIOA->ODR & ~0x00FF) | (1 << i);
            delay_ms(step_delay_ms);
        }

        /* 2. Chạy ngược từ PA6 -> PA1 */
        for (int i = 6; i > 0; i--) {
            GPIOA->ODR = (GPIOA->ODR & ~0x00FF) | (1 << i);
            delay_ms(step_delay_ms);
        }
    }

    return 0;
}
