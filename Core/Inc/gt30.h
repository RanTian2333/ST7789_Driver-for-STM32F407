/* gt30.h */
#ifndef GT30_H
#define GT30_H

#include "stm32f4xx_hal.h"
#include <stdint.h>


// 字号枚举
typedef enum {
    FONT_12X12  = 0,
    FONT_16X16  = 1,
    FONT_24X24  = 2,
    FONT_32X32  = 3,
    ASCII_6X12  = 4,
    ASCII_8X16  = 5,
    ASCII_12X24 = 6,
    ASCII_16X32 = 7,
} FontSize_t;


extern SPI_HandleTypeDef hspi1;


void GT30_Init(void);  // 初始化 GT30 芯片
void     GT30_SwitchMode(void);   // 切换 SPI 到 Mode0 给字库用
void     LCD_SwitchMode(void);    // 切换 SPI 到 Mode3 给 LCD 用
void     GT30_ReadBytes(uint32_t addr, uint8_t *buf, uint16_t len);
void     GT30_GetDotMatrix(uint16_t gbCode, uint8_t *dotBuf);

// 统一获取点阵接口（支持所有字号）
uint8_t GT30_GetMatrix(FontSize_t font, uint16_t code, uint8_t *dotBuf);

#endif