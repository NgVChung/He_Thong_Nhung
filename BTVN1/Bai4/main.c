#include "stm32f10x.h"

int main(void)
{
    
    RCC->APB2ENR |= (1 << 2);

    
    GPIOA->CRL &= ~(0xF << 28);
    GPIOA->CRL |=  (0x4 << 28);

    GPIOA->CRH &= ~(0xF << 0);
    GPIOA->CRH |=  (0x2 << 0);

    GPIOA->BRR = (1 << 8);

    while (1)
    {
        
        if (GPIOA->IDR & (1 << 7))
        {
            
            for (volatile uint32_t i = 0; i < 50000; i++);

            if (GPIOA->IDR & (1 << 7))
            {
                while (GPIOA->IDR & (1 << 7));

                for (volatile uint32_t i = 0; i < 50000; i++);
                
                GPIOA->ODR ^= (1 << 8);
            }
        }
    }
}
