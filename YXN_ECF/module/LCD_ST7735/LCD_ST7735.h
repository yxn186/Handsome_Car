/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    LCD_ST7735.h
  * @brief   This file contains all the function prototypes for
  *          the LCD_ST7735.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD_ST7735_H__
#define __LCD_ST7735_H__



/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bsp_spi.h"
#include <stdint.h>

/*YOUR CODE*/

//屏幕逻辑分辨率
#define ST7735_WIDTH   128
#define ST7735_HEIGHT  160

/*颜色是 RGB565 格式
RGB565 的意思是：
R：5 bit
G：6 bit
B：5 bit
总共 16 bit，也就是 2 字节表示一个像素
*/
#define ST7735_RGB565(r, g, b) \
    (uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

#define ST7735_BLACK   0x0000
#define ST7735_WHITE   0xFFFF
#define ST7735_RED     0xF800
#define ST7735_GREEN   0x07E0
#define ST7735_BLUE    0x001F
#define ST7735_YELLOW  0xFFE0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F

//每次 DMA 最多发送的像素量
//一个像素 2 字节，所以一次 DMA 发送的字节数 = ST7735_DMA_CHUNK_PIXELS * 2
#define ST7735_DMA_CHUNK_PIXELS 256

class Class_ST7735_LCD
{
public:
    // 初始化：允许阻塞
    void Init(
        SPI_HandleTypeDef *hspi,
        GPIO_TypeDef *cs_gpio,  uint16_t cs_pin,
        GPIO_TypeDef *dc_gpio,  uint16_t dc_pin,
        GPIO_TypeDef *res_gpio, uint16_t res_pin,
        GPIO_TypeDef *blk_gpio, uint16_t blk_pin,
        uint8_t x_offset = 0,
        uint8_t y_offset = 0
    );

    // LCD_Task 里周期调用，用来推进 DMA 状态机
    void Update(void);

    // 非阻塞启动绘制
    uint8_t StartFillScreen(uint16_t color);
    uint8_t StartFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

    uint8_t StartDrawImage(uint16_t x,uint16_t y,uint16_t w,uint16_t h,const uint8_t *image);

    bool IsBusy(void) const;

    void Backlight(bool enable);

    // 给 BSP SPI 回调用
    void TxCpltCallback(void);

private:
    enum class LCD_State : uint8_t
    {
        IDLE = 0,

        SEND_CASET_CMD,
        SEND_CASET_DATA,

        SEND_RASET_CMD,
        SEND_RASET_DATA,

        SEND_RAMWR_CMD,

        SEND_COLOR_DATA,
        SEND_IMAGE_DATA,
    };

    enum class LCD_Draw_Mode : uint8_t
    {
        FILL_COLOR = 0,
        IMAGE,
    };

private:
    SPI_HandleTypeDef *hspi_;

    GPIO_TypeDef *cs_gpio_;
    uint16_t cs_pin_;

    GPIO_TypeDef *dc_gpio_;
    uint16_t dc_pin_;

    GPIO_TypeDef *res_gpio_;
    uint16_t res_pin_;

    GPIO_TypeDef *blk_gpio_;
    uint16_t blk_pin_;

    uint8_t x_offset_;
    uint8_t y_offset_;

    volatile bool dma_done_;
    bool busy_;
    LCD_State state_;

    uint16_t x0_;
    uint16_t y0_;
    uint16_t x1_;
    uint16_t y1_;

    uint16_t color_;
    uint32_t remain_pixels_;

    uint8_t cmd_buffer_;
    uint8_t data_buffer_[4];
    uint8_t color_buffer_[ST7735_DMA_CHUNK_PIXELS * 2];

    LCD_Draw_Mode draw_mode_;

    const uint8_t *image_data_;
    uint32_t image_offset_;
    uint32_t remain_bytes_;

private:
    void Reset(void);

    // 初始化阶段用阻塞
    void WriteCommand_Blocking(uint8_t cmd);
    void WriteData_Blocking(const uint8_t *data, uint16_t len);
    void WriteDataByte_Blocking(uint8_t data);
    void FillScreen_Blocking(uint16_t color);

    // 运行阶段用 DMA
    uint8_t StartDmaCommand(uint8_t cmd);
    uint8_t StartDmaData(uint8_t *data, uint16_t len);

    void ContinueDmaTransfer(void);
    void PrepareColorBuffer(uint16_t pixels);
};



#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __LCD_ST7735_H__ */
