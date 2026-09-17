#include "stm32f10x.h"

void PWM_TIM2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    /* Bật clock cho GPIOA */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* Bật clock cho TIM2 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* PA0, PA1, PA2, PA3:
       TIM2_CH1, CH2, CH3, CH4 */
    GPIO_InitStructure.GPIO_Pin =
        GPIO_Pin_0 |
        GPIO_Pin_1 |
        GPIO_Pin_2 |
        GPIO_Pin_3;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*
     * Timer clock = 72 MHz
     *
     * PSC = 71
     *
     * 72 MHz / (71 + 1)
     * = 1 MHz
     *
     * 1 count = 1 us
     */

    TIM_TimeBaseStructure.TIM_Prescaler = 71;
    TIM_TimeBaseStructure.TIM_CounterMode =
        TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 999;
    TIM_TimeBaseStructure.TIM_ClockDivision =
        TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(
        TIM2,
        &TIM_TimeBaseStructure
    );

    /* Cấu hình PWM mode 1 */
    TIM_OCInitStructure.TIM_OCMode =
        TIM_OCMode_PWM1;

    TIM_OCInitStructure.TIM_OutputState =
        TIM_OutputState_Enable;

    TIM_OCInitStructure.TIM_OCPolarity =
        TIM_OCPolarity_High;

    /* PA0 - 10% */
    TIM_OCInitStructure.TIM_Pulse = 100;

    TIM_OC1Init(
        TIM2,
        &TIM_OCInitStructure
    );

    TIM_OC1PreloadConfig(
        TIM2,
        TIM_OCPreload_Enable
    );

    /* PA1 - 30% */
    TIM_OCInitStructure.TIM_Pulse = 300;

    TIM_OC2Init(
        TIM2,
        &TIM_OCInitStructure
    );

    TIM_OC2PreloadConfig(
        TIM2,
        TIM_OCPreload_Enable
    );

    /* PA2 - 50% */
    TIM_OCInitStructure.TIM_Pulse = 500;

    TIM_OC3Init(
        TIM2,
        &TIM_OCInitStructure
    );

    TIM_OC3PreloadConfig(
        TIM2,
        TIM_OCPreload_Enable
    );

    /* PA3 - 70% */
    TIM_OCInitStructure.TIM_Pulse = 700;

    TIM_OC4Init(
        TIM2,
        &TIM_OCInitStructure
    );

    TIM_OC4PreloadConfig(
        TIM2,
        TIM_OCPreload_Enable
    );

    /* Cho phép preload ARR */
    TIM_ARRPreloadConfig(TIM2, ENABLE);

    /* Bắt đầu Timer */
    TIM_Cmd(TIM2, ENABLE);
}

int main(void)
{
    PWM_TIM2_Init();

    while (1)
    {
    }
}
