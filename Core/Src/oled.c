#include "oled.h"
#include "i2c.h"
#include <math.h>
#include <stdlib.h>
#include "cmsis_os2.h"
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_i2c.h"


extern osSemaphoreId_t oled_dma_semHandle;


// OLED器件地址
#define OLED_ADDRESS 0x7A

// OLED参数
#define OLED_PAGE 8            // OLED页数
#define OLED_ROW 8 * OLED_PAGE // OLED行数
#define OLED_COLUMN 128        // OLED列数
#define OLED_SIZE   (OLED_PAGE * OLED_COLUMN)  // 总字节数 = 8 × 128 = 1024
#define OLED_WIDTH  128
#define OLED_HEIGHT 64

// 显存
static uint8_t send_buffer[OLED_COLUMN + 1];  // 临时发送缓冲区
static uint8_t buffer1[OLED_SIZE];
static uint8_t buffer2[OLED_SIZE];
static uint8_t *back_buffer = buffer1;
static uint8_t *front_buffer = buffer2;
static volatile uint8_t dma_busy = 0;
// 0 = DMA空闲，可以开始新传输
// 1 = DMA正在传输，不能修改缓冲区


// 阻塞发送（用于命令和小数据）
static void OLED_SendBlocking(uint8_t *data, uint8_t len) {
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDRESS, data, len, HAL_MAX_DELAY);
}

void OLED_SendCmd(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    OLED_SendBlocking(buf, 2);
}

// 阻塞式清屏（初始化时用）
void OLED_ClearScreen(void) {
    uint8_t buf[OLED_COLUMN + 1];  // 栈上分配，用完即释放
    buf[0] = 0x40;
    memset(buf + 1, 0, OLED_COLUMN);
    
    for (uint8_t page = 0; page < OLED_PAGE; page++) {
        OLED_SendCmd(0xB0 + page);
        OLED_SendCmd(0x02);
        OLED_SendCmd(0x10);
        OLED_SendBlocking(buf, OLED_COLUMN + 1);
    }
}

// ========================== OLED驱动函数 ==========================


void OLED_Init() {
  OLED_SendCmd(0xAE); /*关闭显示 display off*/

  OLED_SendCmd(0x02); /*设置列起始地址 set lower column address*/
  OLED_SendCmd(0x10); /*设置列结束地址 set higher column address*/

  OLED_SendCmd(0x40); /*设置起始行 set display start line*/

  OLED_SendCmd(0xB0); /*设置页地址 set page address*/

  OLED_SendCmd(0x81); /*设置对比度 contract control*/
  OLED_SendCmd(0xCF); /*128*/

  OLED_SendCmd(0xA1); /*设置分段重映射 从右到左 set segment remap*/

  OLED_SendCmd(0xA6); /*正向显示 normal / reverse*/

  OLED_SendCmd(0xA8); /*多路复用率 multiplex ratio*/
  OLED_SendCmd(0x3F); /*duty = 1/64*/

  OLED_SendCmd(0xAD); /*设置启动电荷泵 set charge pump enable*/
  OLED_SendCmd(0x8B); /*启动DC-DC */

  OLED_SendCmd(0x33); /*设置泵电压 set VPP 10V */

  OLED_SendCmd(0xC8); /*设置输出扫描方向 COM[N-1]到COM[0] Com scan direction*/

  OLED_SendCmd(0xD3); /*设置显示偏移 set display offset*/
  OLED_SendCmd(0x00); /* 0x00 */

  OLED_SendCmd(0xD5); /*设置内部时钟频率 set osc frequency*/
  OLED_SendCmd(0xC0);

  OLED_SendCmd(0xD9); /*设置放电/预充电时间 set pre-charge period*/
  OLED_SendCmd(0x1F); /*0x22*/

  OLED_SendCmd(0xDA); /*设置引脚布局 set COM pins*/
  OLED_SendCmd(0x12);

  OLED_SendCmd(0xDB); /*设置电平 set vcomh*/
  OLED_SendCmd(0x40);

  // 清空缓冲区
    memset(back_buffer, 0, OLED_SIZE);
    memset(front_buffer, 0, OLED_SIZE);
    
    // 清屏（阻塞方式，确保初始化完成）
    OLED_ClearScreen();
    
    OLED_SendCmd(0xAF); // 开显示
}



/**
 * @brief 开启OLED显示
 */
void OLED_DisPlay_On() {
  OLED_SendCmd(0x8D); // 电荷泵使能
  OLED_SendCmd(0x14); // 开启电荷泵
  OLED_SendCmd(0xAF); // 点亮屏幕
}

/**
 * @brief 关闭OLED显示
 */
void OLED_DisPlay_Off() {
  OLED_SendCmd(0x8D); // 电荷泵使能
  OLED_SendCmd(0x10); // 关闭电荷泵
  OLED_SendCmd(0xAE); // 关闭屏幕
}

/**
 * @brief 设置颜色模式 黑底白字或白底黑字
 * @param ColorMode 颜色模式COLOR_NORMAL/COLOR_REVERSED
 * @note 此函数直接设置屏幕的颜色模式
 */
void OLED_SetColorMode(OLED_ColorMode mode) {
  if (mode == OLED_COLOR_NORMAL) {
    OLED_SendCmd(0xA6); // 正常显示
  }
  if (mode == OLED_COLOR_REVERSED) {
    OLED_SendCmd(0xA7); // 反色显示
  }
}

// ========================== 显存操作函数 ==========================

/**
 * @brief 清空显存 绘制新的一帧
 */
void OLED_NewFrame() {
    memset(back_buffer, 0, OLED_SIZE);
}


/**
 * @brief 将当前显存显示到屏幕上
 * @note 此函数是移植本驱动时的重要函数 将本驱动库移植到其他驱动芯片时应根据实际情况修改此函数
 */
void OLED_ShowFrame() {

    while(dma_busy) {
        return;
    }

    // 交换缓冲区指针,让 DMA 把 front_buffer 发到屏幕
    uint8_t *tmp = back_buffer;
    back_buffer = front_buffer;
    front_buffer = tmp;
    send_buffer[0] = 0x40;  // 数据命令
    OLED_SendCmd(0xB0 + 0);
    OLED_SendCmd(0x02);
    OLED_SendCmd(0x10);
    memcpy(send_buffer + 1, front_buffer , OLED_COLUMN);

    dma_busy = 1;
    HAL_I2C_Master_Transmit_DMA(&hi2c1, OLED_ADDRESS, send_buffer, OLED_COLUMN + 1);

}

static uint8_t current_page = 0;

//dma中断
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C1) {
    current_page++;
    if (current_page < OLED_PAGE) {
            // 发送下一页
            send_buffer[0] = 0x40;
            OLED_SendCmd(0xB0 + current_page);
            OLED_SendCmd(0x02);
            OLED_SendCmd(0x10);
            memcpy(send_buffer + 1, front_buffer + current_page * OLED_COLUMN, OLED_COLUMN);
            HAL_I2C_Master_Transmit_DMA(&hi2c1, OLED_ADDRESS, send_buffer, OLED_COLUMN + 1);
        } else {
            // 所有页发送完毕
            current_page = 0;
            dma_busy = 0;
            if (oled_dma_semHandle != NULL) {
                osSemaphoreRelease(oled_dma_semHandle);
            }
        }
  }
}

/**
 * @brief 设置一个像素点
 * @param x 横坐标
 * @param y 纵坐标
 * @param color 颜色
 */
void OLED_SetPixel(uint8_t x, uint8_t y, OLED_ColorMode color) {
  if (x >= OLED_COLUMN || y >= OLED_ROW) return;
    uint8_t page = y / 8;
    uint8_t bit = y % 8;
  if (!color) {
    back_buffer[page * OLED_WIDTH + x] |= (1 << bit);
  } else {
    back_buffer[page * OLED_WIDTH + x] &= ~(1 << bit);
  }
}


void OLED_SetBlock(uint8_t x, uint8_t y, const uint8_t *data, uint8_t w, uint8_t h, OLED_ColorMode color) {
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    if (x + w > OLED_WIDTH) w = OLED_WIDTH - x;
    if (y + h > OLED_HEIGHT) h = OLED_HEIGHT - y;
    
    uint8_t start_page = y / 8;
    uint8_t start_bit = y % 8;
    uint16_t base_idx = start_page * OLED_WIDTH + x;
    uint8_t data_height_byte = (h + 7) / 8;  // 数据占几字节高度
    
    for (uint8_t col = 0; col < w; col++) {
        uint16_t buf_idx = base_idx + col;      // back_buffer 索引
        
        // 情况1：从起始位开始，不跨页
        if (start_bit == 0 && h >= 8) {
            // 整页覆盖，直接复制整字节
            for (uint8_t byte = 0; byte < data_height_byte; byte++) {
                uint8_t val = data[byte * w + col];
                if (color == OLED_COLOR_REVERSED) val = ~val;
                back_buffer[buf_idx + byte * OLED_WIDTH] = val;
            }
        } else {
            // 跨页或部分覆盖，逐位处理
            for (uint8_t row = 0; row < h; row++) {
                uint8_t page = (y + row) / 8;
                uint8_t bit = (y + row) % 8;
                uint16_t idx = page * OLED_WIDTH + x + col;
                
                uint8_t data_byte = row / 8;
                uint8_t data_bit = row % 8;
                uint8_t val = (data[data_byte * w + col] >> data_bit) & 0x01;
                if (color == OLED_COLOR_REVERSED) val = !val;
                
                if (val) {
                    back_buffer[idx] |= (1 << bit);
                } else {
                    back_buffer[idx] &= ~(1 << bit);
                }
            }
        }
    }
}

// ========================== 图形绘制函数 ==========================
/**
 * @brief 绘制一条线段
 * @param x1 起始点横坐标
 * @param y1 起始点纵坐标
 * @param x2 终止点横坐标
 * @param y2 终止点纵坐标
 * @param color 颜色
 * @note 此函数使用Bresenham算法绘制线段
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_ColorMode color) {
  static uint8_t temp = 0;
  if (x1 == x2) {
    if (y1 > y2) {
      temp = y1;
      y1 = y2;
      y2 = temp;
    }
    for (uint8_t y = y1; y <= y2; y++) {
      OLED_SetPixel(x1, y, color);
    }
  } else if (y1 == y2) {
    if (x1 > x2) {
      temp = x1;
      x1 = x2;
      x2 = temp;
    }
    for (uint8_t x = x1; x <= x2; x++) {
      OLED_SetPixel(x, y1, color);
    }
  }
}


/**
 * @brief 绘制一张图片
 * @param x 起始点横坐标
 * @param y 起始点纵坐标
 * @param img 图片
 * @param color 颜色
 */
void OLED_DrawImage(uint8_t x, uint8_t y, const Image *img, OLED_ColorMode color) {
  OLED_SetBlock(x, y, img->data, img->w, img->h, color);
}

// ================================ 文字绘制 ================================

/**
 * @brief 绘制一个ASCII字符
 * @param x 起始点横坐标
 * @param y 起始点纵坐标
 * @param ch 字符
 * @param font 字体
 * @param color 颜色
 */
void OLED_PrintASCIIChar(uint8_t x, uint8_t y, char ch, const ASCIIFont *font, OLED_ColorMode color) {
  OLED_SetBlock(x, y, font->chars + (ch - ' ') * (((font->h + 7) / 8) * font->w), font->w, font->h, color);
}

/**
 * @brief 绘制一个ASCII字符串
 * @param x 起始点横坐标
 * @param y 起始点纵坐标
 * @param str 字符串
 * @param font 字体
 * @param color 颜色
 */
void OLED_PrintASCIIString(uint8_t x, uint8_t y, char *str, const ASCIIFont *font, OLED_ColorMode color) {
  uint8_t x0 = x;
  while (*str) {
    OLED_PrintASCIIChar(x0, y, *str, font, color);
    x0 += font->w;
    str++;
  }
}

const int card_x2[3] = {0, 44, 88};

void DrawCornerDecorator(int slot) {
    int x = card_x2[slot];  // 卡片起始X坐标
    
    // 图标区域：x+10 到 x+34，y: 24 到 48
    int left = x + 10;
    int right = x + 34;
    int top = 24;
    int bottom = 48;
    int len = 6;  // 角落线长度
    
    // 左上角
    OLED_DrawLine(left, top, left + len, top, OLED_COLOR_NORMAL);
    OLED_DrawLine(left, top, left, top + len, OLED_COLOR_NORMAL);
    
    // 右上角
    OLED_DrawLine(right, top, right - len, top, OLED_COLOR_NORMAL);
    OLED_DrawLine(right, top, right, top + len, OLED_COLOR_NORMAL);
    
    // 左下角
    OLED_DrawLine(left, bottom, left + len, bottom, OLED_COLOR_NORMAL);
    OLED_DrawLine(left, bottom, left, bottom - len, OLED_COLOR_NORMAL);
    
    // 右下角
    OLED_DrawLine(right, bottom, right - len, bottom, OLED_COLOR_NORMAL);
    OLED_DrawLine(right, bottom, right, bottom - len, OLED_COLOR_NORMAL);
}
