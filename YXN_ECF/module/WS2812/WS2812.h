/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    WS2812.h
  * @brief   This file contains all the function prototypes for
  *          the WS2812.c/.cpp file
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WS2812_H__
#define __WS2812_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/
#define WS2812_SPI_BYTES_PER_LED   9    //每个LED需要9个SPI字节（24位数据转换成72位SPI数据）
#define WS2812_RESET_BYTES         100  //复位信号需要至少 50 微秒的低电平，100 字节的 SPI 数据可以提供足够的时间

#define WS2812_LED_NUM  100  //最大支持LED数量（Init时可动态设置实际数量）

#define WS2812_TX_BUF_SIZE         (WS2812_RESET_BYTES + WS2812_LED_NUM * WS2812_SPI_BYTES_PER_LED + WS2812_RESET_BYTES)
#define WS2812_MAX_OBJECT_NUM      4
//按最大LED数量分配缓冲区，运行时按实际数量发送

typedef struct
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
} WS2812_Color_t;

class Class_WS2812
{
    public:
    /**
     * @brief 初始化WS2812
     * 
     * @param hspi SPI句柄
     * @param LED_Num 实际使用的LED数量
     */
    void WS2812_Init(SPI_HandleTypeDef *hspi, uint16_t LED_Num = WS2812_LED_NUM);

    /**
    * @brief 设置单个LED颜色
    * 
    * @param index LED索引
    * @param R 红色分量
    * @param G 绿色分量
    * @param B 蓝色分量
    */
    void WS2812_SetPixel(uint16_t index, uint8_t R, uint8_t G, uint8_t B);

    /**
    * @brief 设置所有LED颜色
    * 
    * @param R 红色分量
    * @param G 绿色分量
    * @param B 蓝色分量
    */
    void WS2812_SetAll(uint8_t R, uint8_t G, uint8_t B);

    /**
    * @brief 清除所有LED颜色
    * 
    */
    void WS2812_Clear(void);

    /**
    * @brief 推送/显示WS2812 LED颜色
    * 
    * @return uint8_t 状态
    */
    uint8_t WS2812_Show(void);

    /**
    * @brief 检查WS2812是否忙碌
    * 
    * @return uint8_t 忙碌状态
    */
    uint8_t WS2812_IsBusy(void);

    /**
    * @brief 等待WS2812空闲
    * 
    */
    void WS2812_Wait(void);

    void WS2812_Flow_Config(uint8_t R, uint8_t G, uint8_t B, uint16_t LED_Time_ms);

    void WS2812_Flow_Start(void);

    void WS2812_Flow_Stop(void);

    void WS2812_Flow_Tick_1ms(void);

    private:

    /**
    * @brief 写入SPI位
    * 
    * @param bit_pos 位位置
    * @param bit 位值
    */
    void WS2812_WriteSpiBit(uint32_t *bit_pos, uint8_t bit);

    /**
    * @brief 写入WS2812位
    * 
    * @param bit_pos 位位置
    * @param bit 位值
    */
    void WS2812_WriteWsBit(uint32_t *bit_pos, uint8_t bit);

    /**
    * @brief 写入WS2812字节
    * 
    * @param bit_pos 位位置
    * @param data 字节数据
    */
    void WS2812_WriteByte(uint32_t *bit_pos, uint8_t data);

    /**
    * @brief WS2812 SPI回调函数
    * 
    * @param Tx_Buffer 发送缓冲区
    * @param Rx_Buffer 接收缓冲区
    * @param Tx_Length 发送长度
    * @param Rx_Length 接收长度
    */
    static void WS2812_SPI_Callback(uint8_t *Tx_Buffer, uint8_t *Rx_Buffer, uint16_t Tx_Length, uint16_t Rx_Length);

    static void WS2812_Register_Object(Class_WS2812 *WS2812_Object);
    
    static Class_WS2812 *WS2812_Object_List[WS2812_MAX_OBJECT_NUM];

    WS2812_Color_t WS2812_LEDs[WS2812_LED_NUM];
    
    uint8_t WS2812_Tx_Buffer[WS2812_TX_BUF_SIZE];
    
    volatile bool WS2812_Busy_Flag = false;
    
    SPI_HandleTypeDef *WS2812_hspi = nullptr;

    //实际使用的LED数量和对应的SPI缓冲区大小
    uint16_t WS2812_LedNum = 0;
    uint16_t WS2812_TxBuffer_Size = 0;

    uint8_t Flow_R;
    uint8_t Flow_G;
    uint8_t Flow_B;
    uint16_t Flow_LED_Time_ms;
    uint16_t Flow_Timer_ms;
    uint16_t Flow_LED_Index;
    bool Flow_Enable_Flag;
};



#ifdef __cplusplus
}
#endif

#endif /* __WS2812_H__ */
