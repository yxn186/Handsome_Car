/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    WS2812.cpp
  * @brief   WS2812库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "WS2812.h"
#include "bsp_spi.h"
#include "cmsis_os2.h"

Class_WS2812 *Class_WS2812::WS2812_Object_List[WS2812_MAX_OBJECT_NUM] = {nullptr};

void Class_WS2812::WS2812_Register_Object(Class_WS2812 *WS2812_Object)
{
    if (WS2812_Object == nullptr)
    {
        return;
    }

    for (uint8_t i = 0; i < WS2812_MAX_OBJECT_NUM; i++)
    {
        if (WS2812_Object_List[i] == WS2812_Object)
        {
            return;
        }
    }

    for (uint8_t i = 0; i < WS2812_MAX_OBJECT_NUM; i++)
    {
        if (WS2812_Object_List[i] == nullptr)
        {
            WS2812_Object_List[i] = WS2812_Object;
            return;
        }
    }
}



/**
 * @brief WS2812 SPI回调函数
 * 
 * @param Tx_Buffer 发送缓冲区
 * @param Rx_Buffer 接收缓冲区
 * @param Tx_Length 发送长度
 * @param Rx_Length 接收长度
 */
void Class_WS2812::WS2812_SPI_Callback(uint8_t *Tx_Buffer, uint8_t *Rx_Buffer, uint16_t Tx_Length, uint16_t Rx_Length)
{
    (void)Rx_Buffer;
    (void)Tx_Length;
    (void)Rx_Length;

    for (uint8_t i = 0; i < WS2812_MAX_OBJECT_NUM; i++)
    {
        Class_WS2812 *WS2812 = WS2812_Object_List[i];

        if (WS2812 != nullptr && WS2812->WS2812_Tx_Buffer == Tx_Buffer)
        {
            WS2812->WS2812_Busy_Flag = false;
            return;
        }
    }
}

/**
 * @brief 写入SPI位
 * 
 * @param bit_pos 位位置
 * @param bit 位值
 */
void Class_WS2812::WS2812_WriteSpiBit(uint32_t *bit_pos, uint8_t bit)
{
    //负责写 1 个 SPI 位
    //WS2812 的每个 LED 需要 24 位数据
    //每位数据通过 SPI 转换成 3 个 SPI 位
    if (bit)
    {
        //ws2812_tx_buf[*bit_pos / 8]:它表示当前要写第几个字节
        //1 << (7 - (*bit_pos % 8)):它表示当前要写这个字节里面的哪一位
        WS2812_Tx_Buffer[*bit_pos / 8] |= (1 << (7 - (*bit_pos % 8)));
    }

    (*bit_pos)++;
}

/**
 * @brief 写入WS2812位
 * 
 * @param bit_pos 位位置
 * @param bit 位值
 */
void Class_WS2812::WS2812_WriteWsBit(uint32_t *bit_pos, uint8_t bit)
{
    //写入规则：（WS2812自身要求）
    //WS2812 bit = 0  ->  SPI bit = 100
    //WS2812 bit = 1  ->  SPI bit = 110

    //1
    if (bit)
    {
        WS2812_WriteSpiBit(bit_pos, 1);
        WS2812_WriteSpiBit(bit_pos, 1);
        WS2812_WriteSpiBit(bit_pos, 0);
    }
    else//0
    {
        WS2812_WriteSpiBit(bit_pos, 1);
        WS2812_WriteSpiBit(bit_pos, 0);
        WS2812_WriteSpiBit(bit_pos, 0);
    }
}

/**
 * @brief 写入WS2812字节
 * 
 * @param bit_pos 位位置
 * @param data 字节数据
 */
void Class_WS2812::WS2812_WriteByte(uint32_t *bit_pos, uint8_t data)
{
    //1 个字节有 8 位。
    for (int8_t i = 7; i >= 0; i--)
    {
        WS2812_WriteWsBit(bit_pos, (data >> i) & 0x01);
    }
}

/**
 * @brief 初始化WS2812
 * 
 * @param hspi SPI句柄
 * @param LED_Num 实际使用的LED数量
 */
void Class_WS2812::WS2812_Init(SPI_HandleTypeDef *hspi, uint16_t LED_Num)
{
    if (hspi == nullptr)
    {
        return;
    }

    WS2812_hspi = hspi;
    WS2812_Register_Object(this);

    if (LED_Num > WS2812_LED_NUM)
    {
        LED_Num = WS2812_LED_NUM;
    }

    WS2812_LedNum = LED_Num;
    WS2812_TxBuffer_Size = (WS2812_RESET_BYTES + WS2812_LedNum * WS2812_SPI_BYTES_PER_LED + WS2812_RESET_BYTES);
    
    WS2812_Busy_Flag = 0;
    Flow_R = 0;
    Flow_G = 0;
    Flow_B = 0;
    Flow_LED_Time_ms = 1;
    Flow_Timer_ms = 0;
    Flow_LED_Index = 0;
    Flow_Enable_Flag = false;

    WS2812_Clear();

    SPI_Init(WS2812_hspi, Class_WS2812::WS2812_SPI_Callback);
}

/**
 * @brief 设置单个LED颜色
 * 
 * @param index LED索引
 * @param R 红色分量
 * @param G 绿色分量
 * @param B 蓝色分量
 */
void Class_WS2812::WS2812_SetPixel(uint16_t index, uint8_t R, uint8_t G, uint8_t B)
{
    if (index >= WS2812_LedNum)
    {
        return;
    }

    WS2812_LEDs[index].R = R;
    WS2812_LEDs[index].G = G;
    WS2812_LEDs[index].B = B;
}

/**
 * @brief 设置所有LED颜色
 * 
 * @param R 红色分量
 * @param G 绿色分量
 * @param B 蓝色分量
 */
void Class_WS2812::WS2812_SetAll(uint8_t R, uint8_t G, uint8_t B)
{
    for (uint16_t i = 0; i < WS2812_LedNum; i++)
    {
        WS2812_SetPixel(i, R, G, B);
    }
}

/**
 * @brief 清除所有LED颜色
 * 
 */
void Class_WS2812::WS2812_Clear(void)
{
    WS2812_SetAll(0, 0, 0);
}

/**
 * @brief 检查WS2812是否忙碌
 * 
 * @return uint8_t 忙碌状态
 */
uint8_t Class_WS2812::WS2812_IsBusy(void)
{
    return WS2812_Busy_Flag;
}

/**
 * @brief 等待WS2812空闲
 * 
 */
void Class_WS2812::WS2812_Wait(void)
{
    while (WS2812_Busy_Flag)
    {
        if (osKernelGetState() == osKernelRunning)
        {
            osDelay(1);
        }
    }
}

void Class_WS2812::WS2812_Flow_Config(uint8_t R, uint8_t G, uint8_t B, uint16_t LED_Time_ms)
{
    Flow_R = R;
    Flow_G = G;
    Flow_B = B;
    Flow_LED_Time_ms = (LED_Time_ms == 0) ? 1 : LED_Time_ms;
}

void Class_WS2812::WS2812_Flow_Start(void)
{
    Flow_Enable_Flag = true;
    Flow_LED_Index = 0;
    Flow_Timer_ms = Flow_LED_Time_ms;
}

void Class_WS2812::WS2812_Flow_Stop(void)
{
    Flow_Enable_Flag = false;

    if (!WS2812_Busy_Flag)
    {
        WS2812_Clear();
        WS2812_Show();
    }
}

void Class_WS2812::WS2812_Flow_Tick_1ms(void)
{
    if (!Flow_Enable_Flag || WS2812_LedNum == 0)
    {
        return;
    }

    if (Flow_Timer_ms < Flow_LED_Time_ms)
    {
        Flow_Timer_ms++;
        return;
    }

    if (WS2812_Busy_Flag)
    {
        return;
    }

    WS2812_Clear();
    WS2812_SetPixel(Flow_LED_Index, Flow_R, Flow_G, Flow_B);

    if (WS2812_Show() == HAL_OK)
    {
        Flow_Timer_ms = 0;
        Flow_LED_Index++;

        if (Flow_LED_Index >= WS2812_LedNum)
        {
            Flow_LED_Index = 0;
        }
    }
}

/**
 * @brief 推送/显示WS2812 LED颜色
 * 
 * @return uint8_t 状态
 */
uint8_t Class_WS2812::WS2812_Show(void)
{
    if (WS2812_Busy_Flag || WS2812_hspi == nullptr || WS2812_LedNum == 0)
    {
        return HAL_BUSY;
    }

    memset(WS2812_Tx_Buffer, 0, WS2812_TxBuffer_Size);

    uint32_t bit_pos = WS2812_RESET_BYTES * 8U;

    for (uint16_t i = 0; i < WS2812_LedNum; i++)
    {
        // 大多数 WS2812 是 GRB 顺序
        WS2812_WriteByte(&bit_pos, WS2812_LEDs[i].G);
        WS2812_WriteByte(&bit_pos, WS2812_LEDs[i].R);
        WS2812_WriteByte(&bit_pos, WS2812_LEDs[i].B);
    }

    WS2812_Busy_Flag = 1;

    uint8_t state = SPI_Transmit_Data(WS2812_hspi,
                                      nullptr,
                                      0,
                                      GPIO_PIN_RESET,
                                      WS2812_Tx_Buffer,
                                      WS2812_TxBuffer_Size);

    if (state != HAL_OK)
    {
        WS2812_Busy_Flag = 0;
    }

    return state;
}
