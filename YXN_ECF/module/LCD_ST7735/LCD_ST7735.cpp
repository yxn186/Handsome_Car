/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    LCD_ST7735.cpp
  * @brief   LCD_ST7735库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "LCD_ST7735.h"
#include "cmsis_os2.h"

#define ST7735_SWRESET  0x01    //Software Reset，软件复位
#define ST7735_SLPOUT   0x11    //Sleep Out，退出睡眠模式
#define ST7735_NORON    0x13    //Normal Display Mode On，进入正常显示模式
#define ST7735_INVOFF   0x20    //Display Inversion Off，关闭反色显示
#define ST7735_INVON    0x21    //Display Inversion On，开启反色显示
#define ST7735_DISPON   0x29    //Display On，打开显示输出
#define ST7735_CASET    0x2A    //Column Address Set，设置列地址范围，也就是 X 方向范围
#define ST7735_RASET    0x2B    //Row Address Set，设置行地址范围，也就是 Y 方向范围
#define ST7735_RAMWR    0x2C    //Memory Data Write，写显存
#define ST7735_MADCTL   0x36    //Memory Data Access Control，显存访问方向控制 1. 屏幕横向/纵向扫描方向 2. x/y 是否交换 3. 图像是否上下/左右翻转 4. RGB/BGR 颜色顺序
#define ST7735_COLMOD   0x3A    //Color Mode，设置像素颜色格式

/**
 * @brief 延时函数，根据当前系统状态选择合适的延时方式
 * 
 * @param delay_ms 延时时间，单位为毫秒
 */
static void LCD_Delay(uint32_t delay_ms)
{
    if (delay_ms == 0U)
    {
        return;
    }

    if (osKernelGetState() == osKernelRunning)
    {
        osDelay(delay_ms);
    }
    else
    {
        HAL_Delay(delay_ms);
    }
}

static Class_ST7735_LCD *LCD_Active_Object = nullptr;

/**
 * @brief LCD SPI 传输完成回调函数
 * 
 * @param Tx_Buffer 发送缓冲区
 * @param Rx_Buffer 接收缓冲区
 * @param Tx_Length 发送数据长度
 * @param Rx_Length 接收数据长度
 */
static void LCD_SPI_TxCpltCallback(uint8_t *Tx_Buffer,
                                   uint8_t *Rx_Buffer,
                                   uint16_t Tx_Length,
                                   uint16_t Rx_Length)
{
    (void)Tx_Buffer;
    (void)Rx_Buffer;
    (void)Tx_Length;
    (void)Rx_Length;

    if (LCD_Active_Object != nullptr)
    {
        LCD_Active_Object->TxCpltCallback();
    }
}

/**
 * @brief 硬件复位 LCD 屏幕
 * 
 */
void Class_ST7735_LCD::Reset(void)
{
    //RES = 1，先保持正常状态
    //延时 20ms
    HAL_GPIO_WritePin(res_gpio_, res_pin_, GPIO_PIN_SET);
    LCD_Delay(20);

    //RES = 0，拉低复位
    //延时 20ms
    HAL_GPIO_WritePin(res_gpio_, res_pin_, GPIO_PIN_RESET);
    LCD_Delay(20);

    //RES = 1，释放复位
    //延时 120ms
    HAL_GPIO_WritePin(res_gpio_, res_pin_, GPIO_PIN_SET);
    LCD_Delay(120);
}

/**
 * @brief 控制 LCD 背光开关
 * 
 * @param enable 是否开启背光，true 为开启，false 为关闭
 */
void Class_ST7735_LCD::Backlight(bool enable)
{
    (void)enable;

    if (blk_gpio_ == nullptr)
    {
        return;
    }

    HAL_GPIO_WritePin(blk_gpio_, blk_pin_, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief LCD 阻塞发送命令
 * 
 * @param cmd 要发送的命令
 */
void Class_ST7735_LCD::WriteCommand_Blocking(uint8_t cmd)
{
    //DC = 0，告诉屏幕这次 SPI 发的是命令
    HAL_GPIO_WritePin(dc_gpio_, dc_pin_, GPIO_PIN_RESET);

    SPI_Transmit_Data_Blocking(
        hspi_,
        cs_gpio_,
        cs_pin_,
        GPIO_PIN_RESET,
        &cmd,
        1,
        100
    );
}

/**
 * @brief LCD 阻塞发送数据
 * 
 * @param data 要发送的数据
 * @param len 数据长度
 */
void Class_ST7735_LCD::WriteData_Blocking(const uint8_t *data, uint16_t len)
{
    if (data == nullptr || len == 0) return;

    //DC = 1，告诉屏幕这次 SPI 发的是数据
    HAL_GPIO_WritePin(dc_gpio_, dc_pin_, GPIO_PIN_SET);

    SPI_Transmit_Data_Blocking(
        hspi_,
        cs_gpio_,
        cs_pin_,
        GPIO_PIN_RESET,
        (uint8_t *)data,
        len,
        100
    );
}

/**
 * @brief LCD 阻塞发送单字节数据
 * 
 * @param data 要发送的数据
 */
void Class_ST7735_LCD::WriteDataByte_Blocking(uint8_t data)
{
    WriteData_Blocking(&data, 1);
}

/**
 * @brief 阻塞版 某个颜色填充整个屏幕
 * 
 * @param color 要填充的颜色
 */
void Class_ST7735_LCD::FillScreen_Blocking(uint16_t color)
{
    uint16_t x0 = 0 + x_offset_;
    uint16_t y0 = 0 + y_offset_;
    uint16_t x1 = ST7735_WIDTH - 1 + x_offset_;
    uint16_t y1 = ST7735_HEIGHT - 1 + y_offset_;

    uint8_t data[4];

    WriteCommand_Blocking(ST7735_CASET);
    data[0] = x0 >> 8;
    data[1] = x0 & 0xFF;
    data[2] = x1 >> 8;
    data[3] = x1 & 0xFF;
    WriteData_Blocking(data, 4);

    WriteCommand_Blocking(ST7735_RASET);
    data[0] = y0 >> 8;
    data[1] = y0 & 0xFF;
    data[2] = y1 >> 8;
    data[3] = y1 & 0xFF;
    WriteData_Blocking(data, 4);

    WriteCommand_Blocking(ST7735_RAMWR);

    uint8_t buffer[128];

    for (uint16_t i = 0; i < sizeof(buffer) / 2; i++)
    {
        buffer[i * 2] = color >> 8;
        buffer[i * 2 + 1] = color & 0xFF;
    }

    uint32_t total_pixels = (uint32_t)ST7735_WIDTH * ST7735_HEIGHT;

    while (total_pixels > 0)
    {
        uint16_t send_pixels;

        if (total_pixels > sizeof(buffer) / 2)
        {
            send_pixels = sizeof(buffer) / 2;
        }
        else
        {
            send_pixels = total_pixels;
        }

        WriteData_Blocking(buffer, send_pixels * 2);

        total_pixels -= send_pixels;
    }
}

/**
 * @brief 初始化 LCD
 * 
 * @param hspi SPI 句柄
 * @param cs_gpio 片选引脚 GPIO
 * @param cs_pin 片选引脚编号
 * @param dc_gpio 数据/命令引脚 GPIO
 * @param dc_pin 数据/命令引脚编号
 * @param res_gpio 复位引脚 GPIO
 * @param res_pin 复位引脚编号
 * @param blk_gpio 背光引脚 GPIO
 * @param blk_pin 背光引脚编号
 * @param x_offset X 偏移量
 * @param y_offset Y 偏移量
 */
void Class_ST7735_LCD::Init(SPI_HandleTypeDef *hspi,
                            GPIO_TypeDef *cs_gpio,  uint16_t cs_pin,
                            GPIO_TypeDef *dc_gpio,  uint16_t dc_pin,
                            GPIO_TypeDef *res_gpio, uint16_t res_pin,
                            GPIO_TypeDef *blk_gpio, uint16_t blk_pin,
                            uint8_t x_offset,uint8_t y_offset)
{
    hspi_ = hspi;

    cs_gpio_ = cs_gpio;
    cs_pin_ = cs_pin;

    dc_gpio_ = dc_gpio;
    dc_pin_ = dc_pin;

    res_gpio_ = res_gpio;
    res_pin_ = res_pin;

    blk_gpio_ = blk_gpio;
    blk_pin_ = blk_pin;

    x_offset_ = x_offset;
    y_offset_ = y_offset;

    dma_done_ = true;
    busy_ = false;
    state_ = LCD_State::IDLE;

    x0_ = 0;
    y0_ = 0;
    x1_ = 0;
    y1_ = 0;

    color_ = ST7735_BLACK;
    remain_pixels_ = 0;

    draw_mode_ = LCD_Draw_Mode::FILL_COLOR;

    image_data_ = nullptr;
    image_offset_ = 0;
    remain_bytes_ = 0;

    // 注册 SPI DMA 完成回调
    SPI_Init(hspi_, LCD_SPI_TxCpltCallback);

    LCD_Active_Object = this;

    //先关背光
    Backlight(false);

    //硬件复位屏幕
    Reset();

    //软件复位
    WriteCommand_Blocking(ST7735_SWRESET);
    LCD_Delay(150);//等待 150ms 来完成软件复位

    //退出睡眠
    WriteCommand_Blocking(ST7735_SLPOUT);
    LCD_Delay(120);//等待 120ms 来完成退出睡眠

    //设置颜色格式：RGB565
    //1.进入设置颜色模式
    WriteCommand_Blocking(ST7735_COLMOD);
    //2.发送0x05数据 告诉LCD是使用 16-bit/pixel，也就是 RGB565
    WriteDataByte_Blocking(0x05);
    LCD_Delay(10);//等待 10ms 来完成设置颜色模式

    //设置显示方向和 RGB 顺序
    //设置显存访问控制
    WriteCommand_Blocking(ST7735_MADCTL);
    WriteDataByte_Blocking(0xC0);

    //关闭显示反转
    WriteCommand_Blocking(ST7735_INVOFF);

    //进入正常显示模式
    WriteCommand_Blocking(ST7735_NORON);
    LCD_Delay(10);//等待 10ms 来完成进入正常显示模式

    //让 LCD 控制器真正开始显示显存内容
    WriteCommand_Blocking(ST7735_DISPON);
    LCD_Delay(120);

    //打开背光
    Backlight(true);

    //阻塞清屏 初始化最后把整屏写成黑色
    FillScreen_Blocking(ST7735_BLACK);

    //为运行阶段的 DMA 非阻塞状态机准备
    busy_ = false;
    state_ = LCD_State::IDLE;
    dma_done_ = true;
}

/**
 * @brief 判断 LCD 是否忙碌
 * 
 * @return true 如果 LCD 正在忙碌
 * @return false 如果 LCD 空闲
 */
bool Class_ST7735_LCD::IsBusy(void) const
{
    return busy_;
}

/**
 * @brief LCD DMA 传输完成回调函数
 * 
 */
void Class_ST7735_LCD::TxCpltCallback(void)
{
    //DMA发送完成 置为 true
    dma_done_ = true;
}

/**
 * @brief 非阻塞版 发送命令
 * 
 * @param cmd 要发送的命令
 * @return uint8_t HAL 状态
 */
uint8_t Class_ST7735_LCD::StartDmaCommand(uint8_t cmd)
{
    if (hspi_->State != HAL_SPI_STATE_READY)
    {
        return HAL_BUSY;
    }

    LCD_Active_Object = this;

    cmd_buffer_ = cmd;
    dma_done_ = false;

    //告诉屏幕这次 SPI 发的是命令
    HAL_GPIO_WritePin(dc_gpio_, dc_pin_, GPIO_PIN_RESET);

    return SPI_Transmit_Data(
        hspi_,
        cs_gpio_,
        cs_pin_,
        GPIO_PIN_RESET,
        &cmd_buffer_,
        1
    );
}

/**
 * @brief 非阻塞版 发送数据
 * 
 * @param data 要发送的数据
 * @param len 数据长度
 * @return uint8_t HAL 状态
 */
uint8_t Class_ST7735_LCD::StartDmaData(uint8_t *data, uint16_t len)
{
    if (data == nullptr || len == 0)
    {
        return HAL_ERROR;
    }

    if (hspi_->State != HAL_SPI_STATE_READY)
    {
        return HAL_BUSY;
    }

    LCD_Active_Object = this;

    //DMA发送未完成 完成后会在回调函数里把 dma_done_ 置为 true
    dma_done_ = false;

    //告诉屏幕这次 SPI 发的是数据
    HAL_GPIO_WritePin(dc_gpio_, dc_pin_, GPIO_PIN_SET);

    return SPI_Transmit_Data(
        hspi_,
        cs_gpio_,
        cs_pin_,
        GPIO_PIN_RESET,
        data,
        len
    );
}

/**
 * @brief 准备颜色缓冲区
 * 
 * @param pixels 要准备的像素数量
 */
void Class_ST7735_LCD::PrepareColorBuffer(uint16_t pixels)
{
    for (uint16_t i = 0; i < pixels; i++)
    {
        color_buffer_[i * 2] = color_ >> 8;
        color_buffer_[i * 2 + 1] = color_ & 0xFF;
    }
}

/**
 * @brief 非阻塞版 全屏填充
 * 
 * @param color 要填充的颜色，RGB565 格式
 * @return uint8_t HAL 状态
 */
uint8_t Class_ST7735_LCD::StartFillScreen(uint16_t color)
{
    return StartFillRect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, color);
}

/**
 * @brief 非阻塞版 填充矩形区域
 * 
 * @param x 矩形区域的起始 X 坐标
 * @param y 矩形区域的起始 Y 坐标
 * @param w 矩形区域的宽度
 * @param h 矩形区域的高度
 * @param color 要填充的颜色，RGB565 格式
 * @return uint8_t HAL 状态
 */
uint8_t Class_ST7735_LCD::StartFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    //判断 LCD 忙不忙
    if (busy_)
    {
        return HAL_BUSY;
    }

    //判断起点坐标是否合法
    if (x >= ST7735_WIDTH || y >= ST7735_HEIGHT)
    {
        return HAL_ERROR;
    }

    //自动裁剪宽度
    if ((x + w) > ST7735_WIDTH)
    {
        w = ST7735_WIDTH - x;
    }

    //自动裁剪高度
    if ((y + h) > ST7735_HEIGHT)
    {
        h = ST7735_HEIGHT - y;
    }

    //检查宽高是否为 0
    if (w == 0 || h == 0)
    {
        return HAL_ERROR;
    }

    //计算屏幕窗口坐标
    //ST7735 画图前，需要告诉屏幕：我要在哪个区域写像素
    x0_ = x + x_offset_;            //起始列
    y0_ = y + y_offset_;            //起始行
    x1_ = x + w - 1 + x_offset_;    //结束列
    y1_ = y + h - 1 + y_offset_;    //结束行
    //加入了offset，是因为 ST7735 的显存地址和实际屏幕显示区域的坐标不完全一致，具体偏移量取决于具体的 LCD 模块。

    //保存颜色到成员变量 color_，真正发送颜色数据是在后面，所以颜色必须保存起来，后面每次填充 DMA buffer 的时候都要用
    color_ = color;

    //计算剩余像素数
    remain_pixels_ = (uint32_t)w * h;

    //设置绘图模式为填充颜色
    draw_mode_ = LCD_Draw_Mode::FILL_COLOR;

    //设置 LCD 进入忙状态
    busy_ = true;

    //设置状态机起点
    state_ = LCD_State::SEND_CASET_CMD;

    //设置DMA完成 看作当前是上一包 DMA 已经完成
    dma_done_ = true;

    // 启动第一包 DMA
    Update();

    return HAL_OK;//说明已经成功创建并启动了一个 LCD 绘制任务。
}

/**
 * @brief 非阻塞版 绘制图片
 * 
 * @param x 矩形区域的起始 X 坐标
 * @param y 矩形区域的起始 Y 坐标
 * @param w 矩形区域的宽度
 * @param h 矩形区域的高度
 * @param image 图片数据指针，RGB565 格式
 * @return uint8_t HAL 状态
 */
uint8_t Class_ST7735_LCD::StartDrawImage(uint16_t x,uint16_t y,uint16_t w,uint16_t h,const uint8_t *image)
{
    if (busy_)
    {
        return HAL_BUSY;
    }

    if (image == nullptr)
    {
        return HAL_ERROR;
    }

    if (x >= ST7735_WIDTH || y >= ST7735_HEIGHT)
    {
        return HAL_ERROR;
    }

    /*
     * 第一版图片显示先不做裁剪。
     * 要求图片完整落在屏幕范围内。
     * 因为图片数组是一整块连续数据，如果裁剪右边/下边，
     * 还需要考虑每一行的 stride，代码会复杂不少。
     */
    if ((x + w) > ST7735_WIDTH || (y + h) > ST7735_HEIGHT)
    {
        return HAL_ERROR;
    }

    if (w == 0 || h == 0)
    {
        return HAL_ERROR;
    }

    x0_ = x + x_offset_;
    y0_ = y + y_offset_;
    x1_ = x + w - 1 + x_offset_;
    y1_ = y + h - 1 + y_offset_;

    draw_mode_ = LCD_Draw_Mode::IMAGE;

    image_data_ = image;
    image_offset_ = 0;
    remain_bytes_ = (uint32_t)w * h * 2;

    busy_ = true;
    state_ = LCD_State::SEND_CASET_CMD;
    dma_done_ = true;

    Update();

    return HAL_OK;
}

/**
 * @brief 推进当前 LCD 绘图任务更新函数
 * 
 */
void Class_ST7735_LCD::Update(void)
{
    //检查当前 LCD 有没有绘图任务
    if (!busy_)
    {
        return;
    }

    //检查上一包 DMA 发完没有
    if (!dma_done_)
    {
        return;
    }

    //以上都ok 就继续发下一包（真正推进状态机）
    ContinueDmaTransfer();
}

/**
 * @brief LCD 绘图状态机的执行函数
 * 
 */
void Class_ST7735_LCD::ContinueDmaTransfer(void)
{
    uint8_t spi_state = HAL_ERROR;//用来接收每次启动 DMA 的结果

    //spi_state ：这次启动 DMA 的返回值，HAL_OK / HAL_BUSY / HAL_ERROR
    //state_：LCD 状态机当前状态，SEND_CASET_CMD / SEND_COLOR_DATA 等

    //ST7735 绘图流程：
    //1. 设置列地址范围，也就是 X 范围
    //2. 设置行地址范围，也就是 Y 范围
    //3. 开始写显存
    //4. 连续发像素颜色
    switch (state_)
    {
        //设置 X 范围的命令
        case LCD_State::SEND_CASET_CMD:
        {
            state_ = LCD_State::SEND_CASET_DATA;
            spi_state = StartDmaCommand(ST7735_CASET);
        } break;

        //发送 X 范围数据
        case LCD_State::SEND_CASET_DATA:
        {
            //拆成高字节和低字节
            data_buffer_[0] = x0_ >> 8;
            data_buffer_[1] = x0_ & 0xFF;
            data_buffer_[2] = x1_ >> 8;
            data_buffer_[3] = x1_ & 0xFF;

            state_ = LCD_State::SEND_RASET_CMD;
            spi_state = StartDmaData(data_buffer_, 4);
        } break;

        //设置 Y 范围的命令
        case LCD_State::SEND_RASET_CMD:
        {
            state_ = LCD_State::SEND_RASET_DATA;
            spi_state = StartDmaCommand(ST7735_RASET);
        } break;

        //发送 Y 范围数据
        case LCD_State::SEND_RASET_DATA:
        {
            //拆成高字节和低字节
            data_buffer_[0] = y0_ >> 8;
            data_buffer_[1] = y0_ & 0xFF;
            data_buffer_[2] = y1_ >> 8;
            data_buffer_[3] = y1_ & 0xFF;

            state_ = LCD_State::SEND_RAMWR_CMD;
            spi_state = StartDmaData(data_buffer_, 4);
        } break;

        //往这个区域写像素数据的命令
        case LCD_State::SEND_RAMWR_CMD:
        {
            if (draw_mode_ == LCD_Draw_Mode::IMAGE)
            {
                state_ = LCD_State::SEND_IMAGE_DATA;
            }
            else
            {
                state_ = LCD_State::SEND_COLOR_DATA;
            }

            spi_state = StartDmaCommand(ST7735_RAMWR);
        } break;

        //发送像素数据
        case LCD_State::SEND_COLOR_DATA:
        {
            //判断像素是否发完
            if (remain_pixels_ == 0)//remain_pixels_表示当前矩形还有多少像素没有发送
            {
                busy_ = false;
                state_ = LCD_State::IDLE;
                dma_done_ = true;
                return;
            }

            //决定这次发多少像素
            uint16_t send_pixels;

            if (remain_pixels_ > ST7735_DMA_CHUNK_PIXELS)
            {
                //每次 DMA 最多发 256 个像素
                send_pixels = ST7735_DMA_CHUNK_PIXELS;
            }
            else
            {
                //本次发剩下的所有像素
                send_pixels = remain_pixels_;
            }

            //准备颜色 buffer
            //把当前颜色 color_ 填进 color_buffer_
            PrepareColorBuffer(send_pixels);

            //减少剩余像素数
            remain_pixels_ -= send_pixels;

            //状态继续保持
            state_ = LCD_State::SEND_COLOR_DATA;

            //启动颜色数据 DMA 发送
            //注意send_pixels * 2 因为一个 RGB565 像素是 2 字节
            spi_state = StartDmaData(color_buffer_, send_pixels * 2);
        } break;

        case LCD_State::SEND_IMAGE_DATA:
        {
            //判断图片数据是否发完
            if (remain_bytes_ == 0)
            {
                busy_ = false;
                state_ = LCD_State::IDLE;
                dma_done_ = true;

                image_data_ = nullptr;
                image_offset_ = 0;

                return;
            }

            uint16_t send_bytes;
            //本次最多发 512 字节
            if (remain_bytes_ > (ST7735_DMA_CHUNK_PIXELS * 2))
            {
                send_bytes = ST7735_DMA_CHUNK_PIXELS * 2;
            }
            else
            {
                send_bytes = (uint16_t)remain_bytes_;
            }

            state_ = LCD_State::SEND_IMAGE_DATA;

            //从 image_data_ + image_offset_ 开始发
            //offset 往后移动
            //remain_bytes_ 减少
            spi_state = StartDmaData((uint8_t *)(image_data_ + image_offset_), send_bytes);

            image_offset_ += send_bytes;
            remain_bytes_ -= send_bytes;
        } break;
        
        //如果状态机进入了 IDLE，或者出现了未知状态，就直接让 LCD 回到空闲
        //如果现在没有明确的绘图步骤，那就不要乱发 SPI。直接结束任务。
        case LCD_State::IDLE:
        default:
        {
            busy_ = false;
            dma_done_ = true;
            return;
        }
    }

    //如果这一步 DMA 没启动成功，就终止本次绘图任务，让 LCD 回到空闲状态。
    if (spi_state != HAL_OK)
    {
        busy_ = false;
        state_ = LCD_State::IDLE;
        dma_done_ = true;
    }
}

