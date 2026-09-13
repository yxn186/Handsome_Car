/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Vision.cpp
  * @brief   App层视觉USB通信
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "App_Vision.h"

#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_usb.h"

namespace
{
constexpr uint32_t Navigation_Command_Timeout_ms = 100U;
}

Class_Vision Vision;

/**
 * @brief BSPUSB接收回调
 *
 * 只接收完整的AA + 底盘速度 + 拍照使能 + 55数据帧。
 */
void Vision_USB_CallBack(uint8_t *Buffer, uint16_t Length)
{
    Class_Vision::Serial_RX_Frame_u Received_Frame = {};

    Vision.RX_Length = Length;
    Vision.RX_Should_Length = sizeof(Vision.Receive_Union.Raw);

    if ((Buffer == nullptr) || (Length == 0U))
    {
        return;
    }

    if (Length != sizeof(Vision.Receive_Union.Raw))
    {
        return;
    }

    memcpy(Received_Frame.Raw, Buffer, sizeof(Received_Frame.Raw));

    if ((Received_Frame.Data.Frame_Header != 0xAAU) ||
        (Received_Frame.Data.Frame_Tail != 0x55U) ||
        (Received_Frame.Data.Capture_Enable > 1U))
    {
        return;
    }

    Vision.Receive_Union = Received_Frame;

    //有效帧使Vision上线，并更新接收数据和频率计数
    Vision.Online_Time = HAL_GetTick();
    Vision.Serial_Rx_Flag = true;
    Vision.Online_State = true;
    Vision.Rx_Count++;
    Vision.Receive_Chassis_Vx = Vision.Receive_Union.Data.Chassis_Vx;
    Vision.Receive_Chassis_Wz = Vision.Receive_Union.Data.Chassis_Wz;
    Vision.Receive_Capture_Enable = (Vision.Receive_Union.Data.Capture_Enable != 0U);
    Vision.Navigation_Command_Last_Update_Time = Vision.Online_Time;
    Vision.Navigation_Command_Initialized = true;
}

/**
 * @brief 初始化视觉USB通信并注册接收回调
 */
void Class_Vision::Init(void)
{
    memset(&Receive_Union, 0, sizeof(Receive_Union));
    memset(&Transmit_Union, 0, sizeof(Transmit_Union));

    Online_Time = 0U;
    Serial_Rx_Flag = false;
    Online_State = false;
    Serial_Offline_Timer = 0U;
    Serial_Offline_Count = 0U;

    Receive_Chassis_Vx = 0.0f;
    Receive_Chassis_Wz = 0.0f;
    Receive_Capture_Enable = false;
    Navigation_Command_Last_Update_Time = 0U;
    Navigation_Command_Initialized = false;

    Rx_Count = 0U;
    Rx_Freq = 0.0f;
    Freq_Sample_Timer = 0U;
    RX_Length = 0U;
    RX_Should_Length = 0U;

    USB_Init(Vision_USB_CallBack);
}

/**
 * @brief 发送AA + capture_done + 实际底盘速度 + 55数据帧
 */
void Class_Vision::USB_Transmit(bool Capture_Done, float Chassis_Vx, float Chassis_Wz)
{
    Transmit_Union.Data.Frame_Header = 0xAAU;
    Transmit_Union.Data.Capture_Done = Capture_Done ? 1U : 0U;
    Transmit_Union.Data.Chassis_Vx = Chassis_Vx;
    Transmit_Union.Data.Chassis_Wz = Chassis_Wz;
    Transmit_Union.Data.Frame_Tail = 0x55U;

    (void)USB_Transmit_Data(Transmit_Union.Raw, sizeof(Transmit_Union.Raw));
}

/**
 * @brief 获取最新导航命令及其新鲜度
 */
bool Class_Vision::Get_Navigation_Command(float *Chassis_Vx,
                                          float *Chassis_Wz,
                                          bool *Capture_Enable) const
{
    if ((Chassis_Vx == nullptr) || (Chassis_Wz == nullptr) ||
        (Capture_Enable == nullptr))
    {
        return false;
    }

    bool Command_Fresh = false;
    uint32_t Now_ms = HAL_GetTick();

    taskENTER_CRITICAL();
    *Chassis_Vx = Receive_Chassis_Vx;
    *Chassis_Wz = Receive_Chassis_Wz;
    *Capture_Enable = Receive_Capture_Enable;
    Command_Fresh = Navigation_Command_Initialized &&
                    ((Now_ms - Navigation_Command_Last_Update_Time) <=
                     Navigation_Command_Timeout_ms);
    taskEXIT_CRITICAL();

    return Command_Fresh;
}

/**
 * @brief 视觉USB在线检测，由1ms任务周期调用
 */
void Class_Vision::USB_Offline_Detection_1ms(uint32_t Task_Time)
{
    (void)Task_Time;

    constexpr uint32_t Offline_Check_Period_ms = 100U;
    constexpr uint8_t Offline_Check_Threshold = 10U;

    //每1000ms统计一次有效帧接收频率
    Freq_Sample_Timer++;
    if (Freq_Sample_Timer >= 1000U)
    {
        Rx_Freq = static_cast<float>(Rx_Count);
        Rx_Count = 0U;
        Freq_Sample_Timer = 0U;
    }

    //本周期收到过有效数据时刷新在线状态
    if (Serial_Rx_Flag)
    {
        Serial_Rx_Flag = false;
        Serial_Offline_Timer = 0U;
        Serial_Offline_Count = 0U;
        Online_State = true;
        return;
    }

    //每100ms累计一次离线计数，连续10次后判定离线
    Serial_Offline_Timer++;
    if (Serial_Offline_Timer < Offline_Check_Period_ms)
    {
        return;
    }
    Serial_Offline_Timer = 0U;

    if (Serial_Offline_Count < Offline_Check_Threshold)
    {
        Serial_Offline_Count++;
    }

    if (Serial_Offline_Count < Offline_Check_Threshold)
    {
        return;
    }

    taskENTER_CRITICAL();
    RX_Length = 0U;
    Receive_Chassis_Vx = 0.0f;
    Receive_Chassis_Wz = 0.0f;
    Receive_Capture_Enable = false;
    Navigation_Command_Last_Update_Time = 0U;
    Navigation_Command_Initialized = false;
    taskEXIT_CRITICAL();
    Online_State = false;
}
