/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Vision.h
  * @brief   App层视觉USB通信类
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_VISION_H__
#define __APP_VISION_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/

class Class_Vision
{
protected:
    friend void Vision_USB_CallBack(uint8_t *Buffer, uint16_t Length);

    //接收帧：帧头 + 底盘速度 + 拍照使能 + 帧尾
#pragma pack(push, 1)
    typedef struct
    {
        uint8_t Frame_Header;
        float Chassis_Vx;
        float Chassis_Wz;
        uint8_t Capture_Enable;
        uint8_t Frame_Tail;
    } Serial_RX_Frame_t;
#pragma pack(pop)

    static_assert(sizeof(Serial_RX_Frame_t) == 11U,
                  "Vision RX frame size must be 11 bytes");

    typedef union
    {
        Serial_RX_Frame_t Data;
        uint8_t Raw[sizeof(Serial_RX_Frame_t)];
    } Serial_RX_Frame_u;

    Serial_RX_Frame_u Receive_Union = {};

    //发送帧：帧头 + 拍照完成 + 实际底盘速度 + 帧尾
#pragma pack(push, 1)
    typedef struct
    {
        uint8_t Frame_Header;
        uint8_t Capture_Done;
        float Chassis_Vx;
        float Chassis_Wz;
        uint8_t Frame_Tail;
    } Serial_TX_Frame_t;
#pragma pack(pop)

    static_assert(sizeof(Serial_TX_Frame_t) == 11U,
                  "Vision TX frame size must be 11 bytes");

    typedef union
    {
        Serial_TX_Frame_t Data;
        uint8_t Raw[sizeof(Serial_TX_Frame_t)];
    } Serial_TX_Frame_u;

    Serial_TX_Frame_u Transmit_Union = {};

    //在线检测
    uint32_t Online_Time = 0U;
    bool Serial_Rx_Flag = false;
    bool Online_State = false;
    uint32_t Serial_Offline_Timer = 0U;
    uint8_t Serial_Offline_Count = 0U;

    //业务数据
    float Receive_Chassis_Vx = 0.0f;
    float Receive_Chassis_Wz = 0.0f;
    bool Receive_Capture_Enable = false;
    uint32_t Navigation_Command_Last_Update_Time = 0U;
    bool Navigation_Command_Initialized = false;

    //接收频率统计
    uint16_t Rx_Count = 0U;
    float Rx_Freq = 0.0f;
    uint32_t Freq_Sample_Timer = 0U;
    uint16_t RX_Length = 0U;
    uint16_t RX_Should_Length = 0U;

public:
    /**
     * @brief 初始化视觉USB通信并注册接收回调
     */
    void Init(void);

    /**
     * @brief 发送电控上行状态帧
     */
    void USB_Transmit(bool Capture_Done, float Chassis_Vx, float Chassis_Wz);

    /**
     * @brief 视觉USB在线检测，由1ms任务周期调用
     *
     * @param Task_Time 调用周期，接口与参考工程保持一致
     */
    void USB_Offline_Detection_1ms(uint32_t Task_Time);

    bool Get_Online_State(void) const { return Online_State; }
    float Get_Rx_Freq(void) const { return Rx_Freq; }

    /**
     * @brief 获取最新导航命令及其新鲜度
     *
     * @param Chassis_Vx 前进方向速度，单位m/s
     * @param Chassis_Wz 逆时针角速度，单位rad/s
     * @param Capture_Enable 巡检灯控使能
     * @return true 命令在底盘控制超时时间内有效
     */
    bool Get_Navigation_Command(float *Chassis_Vx,
                                float *Chassis_Wz,
                                bool *Capture_Enable) const;
};

extern Class_Vision Vision;

void Vision_USB_CallBack(uint8_t *Buffer, uint16_t Length);

#endif /* __APP_VISION_H__ */
