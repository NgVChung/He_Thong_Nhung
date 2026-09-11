#include "stm32f10x.h"
void delay_ms(uint16_t t){
		volatile unsigned long l = 0;
		for(uint16_t i=0; i<t; i++)
		   for(l=0; l<6000; l++)
			{}
	}
int main(void)
{
    uint8_t input_data;   // Dữ liệu đọc từ PA0-PA7
    uint8_t output_data;  // Dữ liệu sau khi đảo bit

    // Bật clock cho GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    // PA0-PA7: Input Pull-up
    // 0x8 = Input Pull-up/Pull-down
    GPIOA->CRL = 0x88888888;

    // Chọn Pull-up cho PA0-PA7
    GPIOA->ODR |= 0x000000FF;

    // PB8-PB15: Output Push-pull, 50 MHz
    // 0x3 = Output Push-pull
    GPIOB->CRH = 0x33333333;

    while (1)
    {
        // Đọc dữ liệu từ PA0-PA7
        input_data = GPIOA->IDR & 0x00FF;

        // Đảo dữ liệu: 0 -> 1, 1 -> 0
        output_data = (~input_data) & 0x00FF;

        // Ghi dữ liệu đã đảo ra PB8-PB15
        GPIOB->ODR = (GPIOB->ODR & 0x00FF) |
                     ((uint16_t)output_data << 8);
    }
} 
