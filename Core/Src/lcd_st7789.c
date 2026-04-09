#include "lcd_st7789.h"

#include "gt30.h"
#include "main.h"


// 屏幕宽度宏定义（根据你的屏幕修改）
#define LCD_WIDTH   240
// 换行后下移的行间距（根据字体大小调整，建议 = 字体高度）
#define LINE_SPACE  2


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


//  ======= 字体绘制函数 =======

// 12×12 汉字
void LCD_WriteChar_12(uint16_t x, uint16_t y, uint16_t gbCode, uint16_t fg, uint16_t bg)
{
    uint8_t buf[24];
    if(GT30_GetGB2312_Matrix(FONT_12X12, gbCode, buf)) return;

    LCD_SetWindow(x, y, x+11, y+11);
    LCD_CS_LOW(); LCD_DC_HIGH();
    for(int r=0;r<12;r++) {
        uint16_t d = (buf[r*2]<<8) | buf[r*2+1];
        for(int c=0;c<12;c++) {
            uint16_t clr = (d & (0x8000>>c)) ? fg : bg;
            uint8_t b[2]={clr>>8, clr&0xFF};
            HAL_SPI_Transmit(&hspi1, b,2,HAL_MAX_DELAY);
        }
    }
    LCD_CS_HIGH();
}

// 16×16 汉字
void LCD_WriteChar_16(uint16_t x, uint16_t y, uint16_t gbCode, uint16_t fg, uint16_t bg)
{
    uint8_t buf[32];
    if(GT30_GetGB2312_Matrix(FONT_16X16, gbCode, buf)) return;

    LCD_SetWindow(x,y,x+15,y+15);
    LCD_CS_LOW(); LCD_DC_HIGH();
    for(int r=0;r<16;r++) {
        uint16_t d = (buf[r*2]<<8)|buf[r*2+1];
        for(int c=0;c<16;c++) {
            uint16_t clr = (d & (0x8000>>c)) ? fg : bg;
            uint8_t b[2]={clr>>8, clr&0xFF};
            HAL_SPI_Transmit(&hspi1,b,2,HAL_MAX_DELAY);
        }
    }
    LCD_CS_HIGH();
}

// 24×24 汉字
void LCD_WriteChar_24(uint16_t x, uint16_t y, uint16_t gbCode, uint16_t fg, uint16_t bg)
{
    uint8_t buf[72];
    if(GT30_GetGB2312_Matrix(FONT_24X24, gbCode, buf)) return;

    LCD_SetWindow(x,y,x+23,y+23);
    LCD_CS_LOW(); LCD_DC_HIGH();
    for(int r=0;r<24;r++) {
        uint32_t d = (buf[r*3]<<16)|(buf[r*3+1]<<8)|buf[r*3+2];
        for(int c=0;c<24;c++) {
            uint16_t clr = (d & (0x800000>>c)) ? fg : bg;
            uint8_t b[2]={clr>>8, clr&0xFF};
            HAL_SPI_Transmit(&hspi1,b,2,HAL_MAX_DELAY);
        }
    }
    LCD_CS_HIGH();
}

// 32×32 汉字
void LCD_WriteChar_32(uint16_t x, uint16_t y, uint16_t gbCode, uint16_t fg, uint16_t bg)
{
    uint8_t buf[128];
    if(GT30_GetGB2312_Matrix(FONT_32X32, gbCode, buf)) return;

    LCD_SetWindow(x,y,x+31,y+31);
    LCD_CS_LOW(); LCD_DC_HIGH();
    for(int r=0;r<32;r++) {
        uint32_t d = (buf[r*4]<<24)|(buf[r*4+1]<<16)|(buf[r*4+2]<<8)|buf[r*4+3];
        for(int c=0;c<32;c++) {
            uint16_t clr = (d & (0x80000000>>c)) ? fg : bg;
            uint8_t b[2]={clr>>8, clr&0xFF};
            HAL_SPI_Transmit(&hspi1,b,2,HAL_MAX_DELAY);
        }
    }
    LCD_CS_HIGH();
}

// ==================== ASCII 显示 ====================

// 6×12 ASCII
void LCD_WriteASCII_6x12(uint16_t x, uint16_t y, uint8_t ascii, uint16_t fg, uint16_t bg) {
    uint8_t buf[12] = {0};
    if(GT30_GetASCII_Matrix(ASCII_6X12, ascii, buf)) return;

    LCD_SetWindow(x, y, x+5, y+11);
    LCD_CS_LOW();
    LCD_DC_HIGH();

    for(int r=0; r<12; r++) {
        uint8_t d = buf[r];
        for(int c=0; c<6; c++) {
            uint16_t clr = (d & (0x80 >> c)) ? fg : bg;
            uint8_t b[2] = {clr >> 8, clr & 0xFF};
            HAL_SPI_Transmit(&hspi1, b, 2, HAL_MAX_DELAY);
        }
    }
    LCD_CS_HIGH();
}

// 8×16 ASCII
void LCD_WriteASCII_8x16(uint16_t x,uint16_t y,uint8_t ascii,uint16_t fg,uint16_t bg)
{
    uint8_t buf[16] = {0};
    if(GT30_GetASCII_Matrix(ASCII_8X16, ascii, buf)) return;

    LCD_SetWindow(x,y,x+7,y+15);
    LCD_CS_LOW();
    LCD_DC_HIGH();

    for(int r=0; r<16; r++) {
        uint8_t d = buf[r];
        for(int c=0; c<8; c++) {
            uint16_t clr = (d & (0x80 >> c)) ? fg : bg;
            uint8_t b[2] = {clr >> 8, clr & 0xFF};
            HAL_SPI_Transmit(&hspi1, b, 2, HAL_MAX_DELAY);
        }
    }
    LCD_CS_HIGH();
}

// 12×24 ASCII
void LCD_WriteASCII_12x24(uint16_t x, uint16_t y, uint8_t ascii, uint16_t fg, uint16_t bg) {
    uint8_t buf[48] = {0};
    if(GT30_GetASCII_Matrix(ASCII_12X24, ascii, buf)) return;

    LCD_SetWindow(x, y, x+11, y+23);
    LCD_CS_LOW();
    LCD_DC_HIGH();

    for(int r=0; r<24; r++) {
        uint16_t d = (buf[r*2] << 8) | buf[r*2+1];
        for(int c=0; c<12; c++) {
            uint16_t clr = (d & (0x8000 >> c)) ? fg : bg;
            uint8_t b[2] = {clr >> 8, clr & 0xFF};
            HAL_SPI_Transmit(&hspi1, b, 2, HAL_MAX_DELAY);
        }
    }
    LCD_CS_HIGH();
}

// 16×32 ASCII
void LCD_WriteASCII_16x32(uint16_t x, uint16_t y, uint8_t ascii, uint16_t fg, uint16_t bg) {
    uint8_t buf[64] = {0};
    if(GT30_GetASCII_Matrix(ASCII_16X32, ascii, buf)) return;

    LCD_SetWindow(x, y, x+15, y+31);
    LCD_CS_LOW();
    LCD_DC_HIGH();

    for(int r=0; r<32; r++) {
        uint16_t d = (buf[r*2] << 8) | buf[r*2+1];
        for(int c=0; c<16; c++) {
            uint16_t clr = (d & (0x8000 >> c)) ? fg : bg;
            uint8_t b[2] = {clr >> 8, clr & 0xFF};
            HAL_SPI_Transmit(&hspi1, b, 2, HAL_MAX_DELAY);
        }
    }
    LCD_CS_HIGH();
}

//  中英文字符串整合显示
void LCD_PrintStr(uint16_t x, uint16_t y, const char *str,
                  uint16_t fgColor, uint16_t bgColor,FontSize_t fontSize)
{
    uint16_t curX = x;    // 当前X坐标
    uint16_t curY = y;    // 当前Y坐标
    uint8_t width = 0;    // 单个字符宽度
    uint8_t charHeight = 0;// 字符高度（用于换行）


    // 先根据字体大小确定 宽度+高度
    if (fontSize == FONT_12X12) {
        width = 12;
        charHeight = 12;
    } else if (fontSize == FONT_16X16) {
        width = 16;
        charHeight = 16;
    } else if (fontSize == FONT_24X24) {
        width = 24;
        charHeight = 24;
    } else if (fontSize == FONT_32X32) {
        width = 32;
        charHeight = 32;
    }

    while (*str)
    {
        uint8_t high = (uint8_t)*str++;
        uint8_t low  = (uint8_t)*str++;
        uint16_t gbCode = ((uint16_t)high << 8) | low;

        // ============== 核心：超出240自动换行 ==============
        // 判断：当前X + 字符宽度 > 屏幕宽度 → 换行
        if (curX + width > LCD_WIDTH) {
            curX = x;                // X回到起始位置
            curY += charHeight + LINE_SPACE; // Y下移一行+间距
        }

        // 绘制字符
        if (fontSize == FONT_12X12) {
            LCD_WriteChar_12(curX, curY, gbCode, fgColor, bgColor);
        } else if (fontSize == FONT_16X16) {
            LCD_WriteChar_16(curX, curY, gbCode, fgColor, bgColor);
        } else if (fontSize == FONT_24X24) {
            LCD_WriteChar_24(curX, curY, gbCode, fgColor, bgColor);
        } else if (fontSize == FONT_32X32) {
            LCD_WriteChar_32(curX, curY, gbCode, fgColor, bgColor);
        }

        curX += width + 1;  // 字宽 + 1像素间距
    }
}


void LCD_Print(uint16_t x, uint16_t y, const char *str,
                    uint16_t fg, uint16_t bg,
                    FontSize_t asciiFont,   // ASCII字体
                    FontSize_t chineseFont) // 汉字字体
{
    uint16_t curX = x;
    uint16_t curY = y;

    uint8_t asciiW = 0, asciiH = 0;
    uint8_t hzW = 0, hzH = 0;

    /* ===== ASCII尺寸 ===== */
    switch(asciiFont)
    {
        case ASCII_6X12: asciiW=6; asciiH=12; break;
        case ASCII_8X16: asciiW=8; asciiH=16; break;
        case ASCII_12X24: asciiW=12; asciiH=24; break;
        case ASCII_16X32: asciiW=16; asciiH=32; break;
    }

    /* ===== 汉字尺寸 ===== */
    switch(chineseFont)
    {
        case FONT_12X12: hzW=12; hzH=12; break;
        case FONT_16X16: hzW=16; hzH=16; break;
        case FONT_24X24: hzW=24; hzH=24; break;
        case FONT_32X32: hzW=32; hzH=32; break;
    }

    while(*str)
    {
        /* ================= ASCII ================= */
        if(*str < 0x80)
        {
            char ch = *str++;

            // 换行符
            if(ch == '\n')
            {
                curX = x;
                curY += asciiH + LINE_SPACE;
                continue;
            }

            // 自动换行
            if(curX + asciiW > LCD_WIDTH)
            {
                curX = x;
                curY += asciiH + LINE_SPACE;
            }

            /* === 调用ASCII函数 === */
            switch(asciiFont)
            {
                case ASCII_6X12:
                    LCD_WriteASCII_6x12(curX, curY, ch, fg, bg);
                    break;

                case ASCII_8X16:
                    LCD_WriteASCII_8x16(curX, curY, ch, fg, bg);
                    break;

                case ASCII_12X24:
                    LCD_WriteASCII_12x24(curX, curY, ch, fg, bg);
                    break;

                case ASCII_16X32:
                    LCD_WriteASCII_16x32(curX, curY, ch, fg, bg);
                    break;
            }

            curX += asciiW + 1;
        }
        /* ================= 中文（GB2312） ================= */
        else
        {
            uint8_t high = (uint8_t)*str++;
            uint8_t low  = (uint8_t)*str++;
            uint16_t gb = (high << 8) | low;

            // 自动换行
            if(curX + hzW > LCD_WIDTH)
            {
                curX = x;
                curY += hzH + LINE_SPACE;
            }

            /* === 调用汉字函数 === */
            switch(chineseFont)
            {
                case FONT_12X12:
                    LCD_WriteChar_12(curX, curY, gb, fg, bg);
                    break;

                case FONT_16X16:
                    LCD_WriteChar_16(curX, curY, gb, fg, bg);
                    break;

                case FONT_24X24:
                    LCD_WriteChar_24(curX, curY, gb, fg, bg);
                    break;

                case FONT_32X32:
                    LCD_WriteChar_32(curX, curY, gb, fg, bg);
                    break;
            }

            curX += hzW + 1;
        }
    }
}
