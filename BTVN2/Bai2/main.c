#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

/* Các biến đếm ngắt (phải có volatile để tránh tối ưu hóa bộ nhớ) */
volatile uint32_t count_led01hz = 0;
volatile uint32_t count_led1hz  = 0;
volatile uint32_t count_led10hz = 0;

/* Trình xử lý ngắt SysTick - Tự động gọi mỗi 1ms */
void SysTick_Handler(void) {
    // LED 1 (0.1Hz) - Chân PA0: Đảo trạng thái mỗi 5000ms (5s sáng, 5s tắt)
    if (++count_led01hz >= 5000) {
        GPIO_WriteBit(GPIOA, GPIO_Pin_0, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_0)));
        count_led01hz = 0;
    }

    // LED 2 (1Hz) - Chân PA1: Đảo trạng thái mỗi 500ms (0.5s sáng, 0.5s tắt)
    if (++count_led1hz >= 500) {
        GPIO_WriteBit(GPIOA, GPIO_Pin_1, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_1)));
        count_led1hz = 0;
    }

    // LED 3 (10Hz) - Chân PA2: Đảo trạng thái mỗi 50ms (50ms sáng, 50ms tắt)
    if (++count_led10hz >= 50) {
        GPIO_WriteBit(GPIOA, GPIO_Pin_2, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2)));
        count_led10hz = 0;
    }
}

/* Khởi tạo GPIO cho 3 LED */
static void GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    // 1. Cấp xung Clock cho Port A
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 2. Cấu hình PA0, PA1, PA2 làm Output Push-Pull tốc độ 10MHz
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

int main(void) {
    // Cấu hình các chân GPIO
    GPIO_Config();

    // Cấu hình SysTick tạo ngắt mỗi 1ms (Sử dụng xung SystemCoreClock)
    if (SysTick_Config(SystemCoreClock / 1000)) {
        while (1); // Lỗi cấu hình SysTick
    }

    // Vòng lặp chính chờ ngắt
    while (1) {
        __NOP();
    }
}
