/* gt30.c */
#include "gt30.h"

#include <string.h>


// 初始化 GT30 芯片
void GT30_Init(void)
{
    uint8_t dummy[24];

    // 1. 确保 SPI 已初始化
    // 假设 hspi1 已经初始化为默认 Mode3，后续切换模式
    // GT30 默认要求 SPI Mode0 (CPOL=0, CPHA=0)

    // 2. 切换到 GT30 模式
    GT30_SwitchMode();   // 切换 SPI 到 Mode0

    // 3. 拉低 CS，做一次 Dummy 读取以预热内部 SRAM
    GT30_CS_LOW();
    // 读取第一个汉字（GB2312 起始地址 0x0000）
    GT30_ReadBytes(0x00000, dummy, 24);
    GT30_CS_HIGH();

    // 4. 可选延时，保证内部 SRAM 数据加载完
    HAL_Delay(1);  // 1ms 足够

    // 5. 切回 LCD SPI 模式
    LCD_SwitchMode();

    // 6. 初始化完成，第一次汉字输出即可正常显示
}


/* 切换 SPI1 到 Mode0（CPOL=0, CPHA=0），给 GT30 用 */
void GT30_SwitchMode(void)
{
    /* 等待 SPI 空闲 */
    while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY));
    __HAL_SPI_DISABLE(&hspi1);
    hspi1.Instance->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA);  // CPOL=0, CPHA=0
    __HAL_SPI_ENABLE(&hspi1);
}

/* 切换 SPI1 到 Mode3（CPOL=1, CPHA=1），给 ST7789 用 */
void LCD_SwitchMode(void)
{
    while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY));
    __HAL_SPI_DISABLE(&hspi1);
    hspi1.Instance->CR1 |= (SPI_CR1_CPOL | SPI_CR1_CPHA);   // CPOL=1, CPHA=1
    __HAL_SPI_ENABLE(&hspi1);
}

/*
 * GT30 读字库数据
 * 协议：发送 0x03（Read命令）+ 3字节地址，然后读 len 字节
 */
void GT30_ReadBytes(uint32_t addr, uint8_t *buf, uint16_t len)
{
    uint8_t cmd[4] = {
        0x03,                        // Read 指令
        (addr >> 16) & 0xFF,
        (addr >>  8) & 0xFF,
         addr        & 0xFF
    };

    GT30_SwitchMode();
    GT30_CS_LOW();

    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, buf, len, HAL_MAX_DELAY);

    GT30_CS_HIGH();
    LCD_SwitchMode();               // 切回 Mode3 给 LCD
}


void GT30_ReadBytes_Fast(uint32_t addr, uint8_t *buf, uint16_t len)
{
    uint8_t cmd[5] = {
        0x0B,
        (addr >> 16) & 0xFF,
        (addr >>  8) & 0xFF,
         addr        & 0xFF,
        0x00  // dummy byte
    };

    GT30_SwitchMode();
    GT30_CS_LOW();
    HAL_SPI_Transmit(&hspi1, cmd, 5, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, buf, len, HAL_MAX_DELAY);
    GT30_CS_HIGH();
    LCD_SwitchMode();
}




// /*
//  * 根据 GB2312 编码获取 12×12 点阵（24字节）
//  * GT30 汉字区起始地址：（具体以你的字库手册为准）
//  */
// void GT30_GetDotMatrix(uint16_t gbCode, uint8_t *dotBuf)
// {
//     uint8_t  high   = (gbCode >> 8) & 0xFF;
//     uint8_t  low    = gbCode & 0xFF;
//     uint32_t offset = ((uint32_t)(high - 0xA1) * 94 + (low - 0xA1)) * 32;
//     uint32_t addr   = 0x2B000 + offset;   // 替换为GT30 汉字起始地址
//
//     GT30_ReadBytes(addr, dotBuf, 32);
// }


// /*
//  * 根据 GB2312 编码获取 12×12 点阵（24字节）
//  * GT30 汉字区起始地址：（具体以你的字库手册为准）
//  */
// void GT30_GetDotMatrix(uint16_t gbCode, uint8_t *dotBuf)
// {
//     uint8_t  high = (gbCode >> 8) & 0xFF;
//     uint8_t  low  = gbCode & 0xFF;
//     uint32_t index;
//     uint32_t addr;
//
//     // 二级汉字区 (A1-A9)
//     if (high >= 0xA1 && high <= 0xA9 && low >= 0xA1 && low <= 0xFE)
//     {
//         index = (high - 0xA1) * 94 + (low - 0xA1);
//     }
//     // 一级汉字区 (B0-F7)
//     else if (high >= 0xB0 && high <= 0xF7 && low >= 0xA1 && low <= 0xFE)
//     {
//         index = (high - 0xB0) * 94 + (low - 0xA1) + 846;
//     }
//     else
//     {
//         // 非法编码，清空输出
//         memset(dotBuf, 0, 24);
//         return;
//     }
//
//     // 每个字 24 字节
//     addr = 0x00000 + index * 24;
//
//     GT30_ReadBytes(addr, dotBuf, 24);
// }


void GT30_GetDotMatrix(uint16_t gbCode, uint8_t *dotBuf)
{
    uint8_t  high = (gbCode >> 8) & 0xFF;
    uint8_t  low  = gbCode & 0xFF;
    uint32_t index;
    uint32_t addr;

    // 二级汉字区 (A1-A9)
    if (high >= 0xA1 && high <= 0xA9 && low >= 0xA1 && low <= 0xFE)
    {
        index = (high - 0xA1) * 94 + (low - 0xA1);
    }
    // 一级汉字区 (B0-F7)
    else if (high >= 0xB0 && high <= 0xF7 && low >= 0xA1 && low <= 0xFE)
    {
        index = (high - 0xB0) * 94 + (low - 0xA1) + 846;
    }
    else
    {
        memset(dotBuf, 0, 24);
        return;
    }

    addr = 0x00000 + index * 24;

   // GT30_ReadBytes(addr, dotBuf, 24);

    GT30_ReadBytes_Fast(addr, dotBuf, 24);


    // ⭐ 添加调试:检查读到的数据
    // 如果全是0x00或全是0xFF,说明读取有问题
    uint8_t allZero = 1, allFF = 1;
    for(int i = 0; i < 24; i++) {
        if(dotBuf[i] != 0x00) allZero = 0;
        if(dotBuf[i] != 0xFF) allFF = 0;
    }

    // 在这里打个断点,或者用LED闪烁次数表示状态
    if(allZero) {
        // 全是0,说明没读到数据或地址错误
        // 让LED快闪表示错误
    }
    if(allFF) {
        // 全是FF,可能SPI通讯有问题
    }
}