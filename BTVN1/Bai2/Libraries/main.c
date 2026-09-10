#include "stm32f10x.h"

/* 
 * Hàm delay bằng vòng lặp phần mềm (Software Delay).
 * Tùy chỉnh số vòng lặp để thay đổi tốc độ chạy của LED.
 */
void delay_ms(volatile uint32_t ms) {
    while (ms--) {
        for (volatile uint32_t i = 0; i < 1500; i++) {
            __asm__ volatile ("nop");
        }
    }
}

int main(void) {
    /* 
     * 1. Cấp xung clock cho ngoại vi GPIOA:
     * Dùng hằng số RCC_APB2ENR_IOPAEN (bit 2 của thanh ghi RCC->APB2ENR)
     */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    /* 
     * 2. Cấu hình 8 chân PA0 -> PA7 ở chế độ General Purpose Output Push-Pull 2MHz:
     * 8 chân này được quản lý toàn bộ bởi thanh ghi CRL (32 bit):
     * - Mỗi chân chiếm 4 bit: CNF = 00 (Push-Pull), MODE = 10 (Output 2MHz) => 0b0010 = 0x2
     * - Cấu hình đồng loạt cả 8 chân: ghi giá trị 0x22222222 vào GPIOA->CRL
     */
    GPIOA->CRL = 0x22222222;

    /* Tắt toàn bộ 8 LED ban đầu */
    GPIOA->ODR &= ~0x00FF;

    /* Thời gian sáng của mỗi bước chạy LED (mili-giây) */
    uint32_t step_delay_ms = 150;

    while (1) {
        /* ==========================================================
         * 1. Chạy từ trái sang phải: PA0 -> PA7
         * Tại mỗi bước chỉ bật duy nhất 1 LED (1 << i)
         * ========================================================== */
        for (int i = 0; i < 8; i++) {
            /* Xóa 8 bit thấp [7:0] và bật bit thứ i */
            GPIOA->ODR = (GPIOA->ODR & ~0x00FF) | (1 << i);
            delay_ms(step_delay_ms);
        }

        /* ==========================================================
         * 2. Đảo chiều chạy từ phải sang trái: PA6 -> PA1
         * (Bỏ qua PA7 và PA0 để không bị khựng/sáng đúp tại 2 đầu mút)
         * ========================================================== */
        for (int i = 6; i > 0; i--) {
            GPIOA->ODR = (GPIOA->ODR & ~0x00FF) | (1 << i);
            delay_ms(step_delay_ms);
        }
    }

    return 0;
}
