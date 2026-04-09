#ifndef LCD_ST7789_H
#define LCD_ST7789_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#include "gt30.h"

// LCD 片选引脚：PA4（严格匹配接线表）
#define LCD_CS_LOW()    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define LCD_CS_HIGH()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)

// LCD 其他引脚（匹配接线表）
#define LCD_DC_LOW()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET)
#define LCD_DC_HIGH()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET)
#define LCD_RST_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)
#define LCD_RST_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)
#define LCD_BLK_ON()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET)
#define LCD_BLK_OFF()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET)

/* ---- 屏幕参数 ---- */
#define LCD_W  240
#define LCD_H  240

/* ---- 常用 RGB565 颜色 ---- */
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0

extern SPI_HandleTypeDef hspi1;

/* ---- 函数声明 ---- */
void LCD_Init(void);
void LCD_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void LCD_Fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void LCD_WriteChar_12(uint16_t x, uint16_t y, uint16_t gbCode, uint16_t fg, uint16_t bg);
void LCD_WriteChar_16(uint16_t x, uint16_t y, uint16_t gbCode, uint16_t fg, uint16_t bg);
void LCD_WriteChar_24(uint16_t x, uint16_t y, uint16_t gbCode, uint16_t fg, uint16_t bg);
void LCD_WriteChar_32(uint16_t x, uint16_t y, uint16_t gbCode, uint16_t fg, uint16_t bg);

void LCD_WriteASCII_6x12(uint16_t x,uint16_t y,uint8_t ascii,uint16_t fg,uint16_t bg);
void LCD_WriteASCII_8x16(uint16_t x,uint16_t y,uint8_t ascii,uint16_t fg,uint16_t bg);
void LCD_WriteASCII_12x24(uint16_t x,uint16_t y,uint8_t ascii,uint16_t fg,uint16_t bg);
void LCD_WriteASCII_16x32(uint16_t x,uint16_t y,uint8_t ascii,uint16_t fg,uint16_t bg);

void LCD_Print(uint16_t x, uint16_t y, const char *str,
                    uint16_t fg, uint16_t bg,
                    FontSize_t asciiFont,   // ASCII字体
                    FontSize_t chineseFont); // 汉字字体

#endif