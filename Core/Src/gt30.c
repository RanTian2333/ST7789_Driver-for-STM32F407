/* gt30.c */
#include "gt30.h"
#include <string.h>
#include "main.h"

extern SPI_HandleTypeDef hspi1;

#define GT30_CS_LOW()   HAL_GPIO_WritePin(GT30_CS_GPIO_Port, GT30_CS_Pin, GPIO_PIN_RESET)
#define GT30_CS_HIGH()  HAL_GPIO_WritePin(GT30_CS_GPIO_Port, GT30_CS_Pin, GPIO_PIN_SET)

void GT30_Init(void)
{
    uint8_t dummy[24];
    GT30_SwitchMode();
    GT30_CS_LOW();
    GT30_ReadBytes(0x00000, dummy, 24);
    GT30_CS_HIGH();
    HAL_Delay(1);
    LCD_SwitchMode();
}

void GT30_SwitchMode(void)
{
    while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY));
    __HAL_SPI_DISABLE(&hspi1);
    hspi1.Instance->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA);
    __HAL_SPI_ENABLE(&hspi1);
}

void LCD_SwitchMode(void)
{
    while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY));
    __HAL_SPI_DISABLE(&hspi1);
    hspi1.Instance->CR1 |= (SPI_CR1_CPOL | SPI_CR1_CPHA);
    __HAL_SPI_ENABLE(&hspi1);
}

void GT30_ReadBytes(uint32_t addr, uint8_t *buf, uint16_t len)
{
    uint8_t cmd[4] = {
        0x03,
        (addr >> 16) & 0xFF,
        (addr >>  8) & 0xFF,
         addr        & 0xFF
    };
    GT30_SwitchMode();
    GT30_CS_LOW();
    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, buf, len, HAL_MAX_DELAY);
    GT30_CS_HIGH();
    LCD_SwitchMode();
}

void GT30_ReadBytes_Fast(uint32_t addr, uint8_t *buf, uint16_t len)
{
    uint8_t cmd[5] = {
        0x0B,
        (addr >> 16) & 0xFF,
        (addr >>  8) & 0xFF,
         addr        & 0xFF,
        0x00
    };
    GT30_SwitchMode();
    GT30_CS_LOW();
    HAL_SPI_Transmit(&hspi1, cmd, 5, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, buf, len, HAL_MAX_DELAY);
    GT30_CS_HIGH();
    LCD_SwitchMode();
}

// ===================== 统一字库获取 =====================
uint8_t GT30_GetMatrix(FontSize_t font, uint16_t code, uint8_t *dotBuf)
{
    uint32_t addr = 0;
    uint16_t size  = 0;
    uint16_t index  = 0;
    uint8_t  msb  = (code >> 8) & 0xFF;
    uint8_t  lsb  = code & 0xFF;

//    memset(dotBuf, 0, 128); // 最大32×32=128字节

    switch (font) {
        case FONT_12X12:
            size = 24;
            // 二级汉字区 (A1-A9)
            if (msb >= 0xA1 && msb <= 0xA9 && lsb >= 0xA1 && lsb <= 0xFE)
            {
                index = (msb - 0xA1) * 94 + (lsb - 0xA1);
            }
            // 一级汉字区 (B0-F7)
            else if (msb >= 0xB0 && msb <= 0xF7 && lsb >= 0xA1 && lsb <= 0xFE)
            {
                index = (msb - 0xB0) * 94 + (lsb - 0xA1) + 846;
            }
            else
            {
                // 不支持的字符，返回 1
                memset(dotBuf, 0, size);
                return 1;
            }
            addr = 0x00000 + index * size;
            GT30_ReadBytes_Fast(addr, dotBuf, size);
            break;

        case FONT_16X16:
            size = 32;

            if(msb >=0xA1 && msb <= 0XA9 && lsb >=0xA1) {
                index = (msb - 0xA1) * 94 + (lsb - 0xA1);
            }
            else if(msb >=0xB0 && msb <= 0xF7 && lsb >=0xA1) {
                index = (msb - 0xB0) * 94 + (lsb - 0xA1) + 846;
            }
            else {
                memset(dotBuf, 0, size);
                return 1;
            }
            addr = 0x2C9D0 + index * size;
            GT30_ReadBytes_Fast(addr, dotBuf, size);
            break;
        case FONT_24X24:
            size = 72;
            if(msb >=0xA1 && msb <= 0XA9 && lsb >=0xA1) {
                index = (msb - 0xA1) * 94 + (lsb - 0xA1);
            }
            else if(msb >=0xB0 && msb <= 0xF7 && lsb >=0xA1) {
                index = (msb - 0xB0) * 94 + (lsb - 0xA1) + 846;
            }
            else {
                memset(dotBuf, 0, size);
                return 1;
            }
            addr = 0x68190 + index * size;
            GT30_ReadBytes_Fast(addr, dotBuf, size);
            break;
        case FONT_32X32:
            size = 128;
            if(msb >=0xA1 && msb <= 0XA9 && lsb >=0xA1)
                index = (msb - 0xA1) * 94 + (lsb - 0xA1);
            else if(msb >=0xB0 && msb <= 0xF7 && lsb >=0xA1)
                index = ((msb - 0xB0) * 94 + (lsb - 0xA1)+ 846);
            else {
                memset(dotBuf, 0, size);
                return 1;
            }
            addr = 0xEDF00 + index * size;
            GT30_ReadBytes_Fast(addr, dotBuf, size);
            break;
        default:
            //  无效字号，返回 1
            size = 0;
            memset(dotBuf, 0, size);
            return 1;
    }
    return 0;
}