/* gt30.h */
#ifndef GT30_H
#define GT30_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

// GT30 片选引脚：PB2（严格匹配接线表）
#define GT30_CS_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET)
#define GT30_CS_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET)

extern SPI_HandleTypeDef hspi1;


void GT30_Init(void);  // 初始化 GT30 芯片
void     GT30_SwitchMode(void);   // 切换 SPI 到 Mode0 给字库用
void     LCD_SwitchMode(void);    // 切换 SPI 到 Mode3 给 LCD 用
void     GT30_ReadBytes(uint32_t addr, uint8_t *buf, uint16_t len);
void     GT30_GetDotMatrix(uint16_t gbCode, uint8_t *dotBuf);

#endif