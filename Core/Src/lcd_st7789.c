#include "lcd_st7789.h"

#include "gt30.h"
#include "main.h"

/* ---- 底层 SPI 发送 ---- */
static void LCD_SPI_Send(uint8_t *buf, uint16_t len)
{
    HAL_SPI_Transmit(&hspi1, buf, len, HAL_MAX_DELAY);
}

static void LCD_WriteCmd(uint8_t cmd)
{
    LCD_CS_LOW();
    LCD_DC_LOW();
    LCD_SPI_Send(&cmd, 1);
    LCD_CS_HIGH();
}

static void LCD_WriteData8(uint8_t dat)
{
    LCD_CS_LOW();
    LCD_DC_HIGH();
    LCD_SPI_Send(&dat, 1);
    LCD_CS_HIGH();
}

static void LCD_WriteData16(uint16_t dat)
{
    uint8_t buf[2] = { dat >> 8, dat & 0xFF };
    LCD_CS_LOW();
    LCD_DC_HIGH();
    LCD_SPI_Send(buf, 2);
    LCD_CS_HIGH();
}

/* ---- 硬件复位 ---- */
static void LCD_HardReset(void)
{
    LCD_RST_HIGH();
    HAL_Delay(10);
    LCD_RST_LOW();
    HAL_Delay(20);
    LCD_RST_HIGH();
    HAL_Delay(120);
}

/* ---- 初始化指令序列 ---- */
void LCD_Init(void)
{
    LCD_BLK_ON();
    LCD_HardReset();

    LCD_WriteCmd(0x11);          // Sleep Out
    HAL_Delay(120);

    LCD_WriteCmd(0x36);          // MADCTL: 扫描方向
    LCD_WriteData8(0x00);        // 正常方向，RGB 顺序

    LCD_WriteCmd(0x3A);          // COLMOD: 像素格式
    LCD_WriteData8(0x55);        // 16位 RGB565

    LCD_WriteCmd(0xB2);          // Porch Setting
    LCD_WriteData8(0x0C);
    LCD_WriteData8(0x0C);
    LCD_WriteData8(0x00);
    LCD_WriteData8(0x33);
    LCD_WriteData8(0x33);

    LCD_WriteCmd(0xB7);          // Gate Control
    LCD_WriteData8(0x35);

    LCD_WriteCmd(0xBB);          // VCOM
    LCD_WriteData8(0x19);

    LCD_WriteCmd(0xC0);          // LCM Control
    LCD_WriteData8(0x2C);

    LCD_WriteCmd(0xC2);          // VDV/VRH Enable
    LCD_WriteData8(0x01);

    LCD_WriteCmd(0xC3);          // VRH Set
    LCD_WriteData8(0x12);

    LCD_WriteCmd(0xC4);          // VDV Set
    LCD_WriteData8(0x20);

    LCD_WriteCmd(0xC6);          // Frame Rate 60Hz
    LCD_WriteData8(0x0F);

    LCD_WriteCmd(0xD0);          // Power Control
    LCD_WriteData8(0xA4);
    LCD_WriteData8(0xA1);

    /* Gamma 正极 */
    LCD_WriteCmd(0xE0);
    uint8_t gp[] = {0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23};
    for(int i=0;i<14;i++) LCD_WriteData8(gp[i]);

    /* Gamma 负极 */
    LCD_WriteCmd(0xE1);
    uint8_t gn[] = {0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23};
    for(int i=0;i<14;i++) LCD_WriteData8(gn[i]);

    LCD_WriteCmd(0x21);          // Display Inversion On（ST7789 通常需要）
    LCD_WriteCmd(0x29);          // Display ON

    LCD_Fill(0, 0, LCD_W-1, LCD_H-1, BLACK);  // 清屏
}

/* ---- 设置显示窗口 ---- */
void LCD_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    LCD_WriteCmd(0x2A);          // CASET（列地址）
    LCD_WriteData16(x0);
    LCD_WriteData16(x1);

    LCD_WriteCmd(0x2B);          // RASET（行地址）
    LCD_WriteData16(y0);
    LCD_WriteData16(y1);

    LCD_WriteCmd(0x2C);          // RAMWR（开始写像素）
}

/* ---- 区域填充 ---- */
void LCD_Fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    uint32_t pxCount = (uint32_t)(x1 - x0 + 1) * (y1 - y0 + 1);
    uint8_t buf[2] = { color >> 8, color & 0xFF };

    LCD_SetWindow(x0, y0, x1, y1);
    LCD_CS_LOW();
    LCD_DC_HIGH();
    for (uint32_t i = 0; i < pxCount; i++) {
        LCD_SPI_Send(buf, 2);
    }
    LCD_CS_HIGH();
}

/* ---- 画点 ---- */
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    LCD_SetWindow(x, y, x, y);
    LCD_WriteData16(color);
}

// void LCD_WriteChar_GT30(uint16_t x, uint16_t y, uint16_t charCode,
//                         uint16_t fgColor, uint16_t bgColor)
// {
//     uint8_t dotBuf[32];
//     GT30_GetDotMatrix(charCode, dotBuf);  // ← 从芯片真实读取
//
//     LCD_SetWindow(x, y, x + 15, y + 15);
//     LCD_CS_LOW();
//     LCD_DC_HIGH();
//     for (int row = 0; row < 16; row++) {
//         for (int byte = 0; byte < 2; byte++) {
//             uint8_t b = dotBuf[row * 2 + byte];
//             for (int bit = 7; bit >= 0; bit--) {
//                 uint16_t px = (b >> bit) & 0x01 ? fgColor : bgColor;
//                 uint8_t  buf[2] = { px >> 8, px & 0xFF };
//                 HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
//             }
//         }
//     }
//     LCD_CS_HIGH();
// }


void LCD_WriteChar_GT30(uint16_t x, uint16_t y, uint16_t charCode,
                        uint16_t fgColor, uint16_t bgColor)
{
    uint8_t dotBuf[24];
    GT30_GetDotMatrix(charCode, dotBuf);

    LCD_SetWindow(x, y, x + 11, y + 11);

    LCD_CS_LOW();
    LCD_DC_HIGH();

    for (int row = 0; row < 12; row++)
    {
        uint16_t rowData = (dotBuf[row*2] << 8) | dotBuf[row*2 + 1];

        for (int col = 0; col < 12; col++)
        {
            uint16_t px = (rowData & (0x8000 >> col)) ? fgColor : bgColor;
            uint8_t buf[2] = { px >> 8, px & 0xFF };
            HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
        }
    }

    LCD_CS_HIGH();
}

void LCD_PrintStr(uint16_t x, uint16_t y, const char *str,
                  uint16_t fgColor, uint16_t bgColor)
{
    uint16_t curX = x;
    while (*str)
    {
        uint8_t high = (uint8_t)*str++;
        uint8_t low  = (uint8_t)*str++;
        uint16_t gbCode = ((uint16_t)high << 8) | low;
        LCD_WriteChar_GT30(curX, y, gbCode, fgColor, bgColor);
        curX += 13;  // 12像素字宽 + 1像素间距
    }
}
