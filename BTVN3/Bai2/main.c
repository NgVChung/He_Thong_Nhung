#include "stm32f10x.h"

#define CS_LOW()   GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define CS_HIGH()  GPIO_SetBits(GPIOA, GPIO_Pin_4)

void SystemInit(void)
{
}

/* =========================
   DELAY
   ========================= */
void Delay_ms(uint32_t ms)
{
    uint32_t i, j;

    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 8000; j++)
        {
            __NOP();
        }
    }
}


/* =========================
   SPI1
   ========================= */
void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    /* Bật clock GPIOA + AFIO + SPI1 */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_AFIO |
        RCC_APB2Periph_SPI1,
        ENABLE
    );

    /* PA5 = SCK
       PA7 = MOSI */
    GPIO_InitStructure.GPIO_Pin =
        GPIO_Pin_5 | GPIO_Pin_7;

    GPIO_InitStructure.GPIO_Mode =
        GPIO_Mode_AF_PP;

    GPIO_InitStructure.GPIO_Speed =
        GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PA4 = CS */
    GPIO_InitStructure.GPIO_Pin =
        GPIO_Pin_4;

    GPIO_InitStructure.GPIO_Mode =
        GPIO_Mode_Out_PP;

    GPIO_InitStructure.GPIO_Speed =
        GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    CS_HIGH();

    /* SPI1 Master */
    SPI_InitStructure.SPI_Direction =
        SPI_Direction_1Line_Tx;

    SPI_InitStructure.SPI_Mode =
        SPI_Mode_Master;

    SPI_InitStructure.SPI_DataSize =
        SPI_DataSize_8b;

    SPI_InitStructure.SPI_CPOL =
        SPI_CPOL_Low;

    SPI_InitStructure.SPI_CPHA =
        SPI_CPHA_1Edge;

    SPI_InitStructure.SPI_NSS =
        SPI_NSS_Soft;

    SPI_InitStructure.SPI_BaudRatePrescaler =
        SPI_BaudRatePrescaler_16;

    SPI_InitStructure.SPI_FirstBit =
        SPI_FirstBit_MSB;

    SPI_InitStructure.SPI_CRCPolynomial =
        7;

    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
}


/* =========================
   GỬI 1 BYTE
   ========================= */
void SPI1_SendByte(uint8_t data)
{
    while (
        SPI_I2S_GetFlagStatus(
            SPI1,
            SPI_I2S_FLAG_TXE
        ) == RESET
    );

    SPI_I2S_SendData(SPI1, data);

    while (
        SPI_I2S_GetFlagStatus(
            SPI1,
            SPI_I2S_FLAG_BSY
        ) == SET
    );
}


/* =========================
   GỬI MAX7219
   ========================= */
void MAX7219_Write(uint8_t address, uint8_t data)
{
    CS_LOW();

    SPI1_SendByte(address);
    SPI1_SendByte(data);

    CS_HIGH();
}


/* =========================
   INIT MAX7219
   ========================= */
void MAX7219_Init(void)
{
    MAX7219_Write(0x09, 0x00);  // Không Decode
    MAX7219_Write(0x0A, 0x08);  // Độ sáng
    MAX7219_Write(0x0B, 0x07);  // Scan 8 hàng
    MAX7219_Write(0x0C, 0x01);  // Bật
    MAX7219_Write(0x0F, 0x00);  // Tắt Test
}


/* =========================
   CLEAR
   ========================= */
void MAX7219_Clear(void)
{
    uint8_t i;

    for (i = 1; i <= 8; i++)
    {
        MAX7219_Write(i, 0x00);
    }
}


/* =================================================
   FONT GỐC
   ================================================= */

/* P */
const uint8_t FONT_P[8] =
{
    0x7C,
    0x66,
    0x66,
    0x7C,
    0x60,
    0x60,
    0x60,
    0x60
};


/* T */
const uint8_t FONT_T[8] =
{
    0x7E,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18
};


/* I */
const uint8_t FONT_I[8] =
{
    0x7E,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x18,
    0x7E
};


/* =================================================
   XOAY FONT 90° SANG TRÁI

   Hướng mong muốn:

             MA TRẬN LED
          ┌──────────────┐
          │      P       │
          │      P       │
          │      P       │
          │      P       │
          └──────────────┘
                 ↓
              CHÂN CẮM
                 ↓
              CON CHIP
   ================================================= */

void Display_Letter(const uint8_t *font)
{
    uint8_t row;
    uint8_t col;
    uint8_t data;

    for (row = 0; row < 8; row++)
    {
        data = 0;

        for (col = 0; col < 8; col++)
        {
            /*
             * Xoay toàn bộ ma trận
             * sang trái 90°
             */
            if (font[col] & (1 << (7 - row)))
            {
                data |= (1 << col);
            }
        }

        MAX7219_Write(row + 1, data);
    }
}


/* =========================
   MAIN
   ========================= */
int main(void)
{
    SPI1_Init();

    MAX7219_Init();

    MAX7219_Clear();

    while (1)
    {
        /* P */
        Display_Letter(FONT_P);
        Delay_ms(150);

        /* T */
        Display_Letter(FONT_T);
        Delay_ms(150);

        /* I */
        Display_Letter(FONT_I);
        Delay_ms(150);

        /* T */
        Display_Letter(FONT_T);
        Delay_ms(150);
    }
}
