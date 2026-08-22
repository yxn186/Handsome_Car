/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Handsome_Car_Task.cpp
  * @brief   Handsome_Car任务层
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "Handsome_Car_Task.h"

#include "cmsis_os2.h"
#include "usb_device.h"
#include "Application/App_Chassis/App_Chassis.h"
#include "Application/App_Command/App_Command.h"
#include "Application/App_Remote/App_Remote.h"
#include "Application/App_Vision/App_Vision.h"

//USART和USB接收回调仅在全部App初始化完成后才向上层分发数据
extern "C"
{
bool Global_Init_Finished = false;
}

/**
 * @brief 工程初始化任务
 *
 * 本文件中的强定义覆盖CubeMX生成的弱定义。全部App初始化完成后退出本任务。
 *
 * @param argument FreeRTOS任务参数，本任务不使用
 */
extern "C" void InitTaskFunction(void *argument)
{
    (void)argument;

    //先注册Vision回调并准备BSP USB接收缓冲区，再启动USB CDC设备
    Vision.Init();
    MX_USB_DEVICE_Init();

    //初始化差速底盘、四个M3508、PID和CAN2接收
    App_Chassis_Init();

    //初始化DR16和USART3 DMA接收
    App_Remote_Init();

    //初始化遥控命令状态，默认底盘无力
    App_Command_Init();

    //允许USART和USB接收回调向上层分发数据
    Global_Init_Finished = true;

    osThreadExit();
}

/**
 * @brief 工程主周期任务
 *
 * 初始化完成后，每1ms刷新一次DR16、底盘控制和Vision在线状态。
 *
 * @param argument FreeRTOS任务参数，本任务不使用
 */
extern "C" void MainTaskFunction(void *argument)
{
    (void)argument;

    for (;;)
    {
        //MainTask优先级高于InitTask，初始化完成前只延时等待
        if (!Global_Init_Finished)
        {
            osDelay(1U);
            continue;
        }

        //先更新遥控数据，便于后续控制层直接读取当前周期状态
        App_Remote_Update();

        //根据遥控器在线状态、左拨杆和左摇杆刷新底盘目标
        App_Command_Update();

        //刷新底盘反馈、Debug模式和电机控制输出
        App_Chassis_Update();

        //Vision接收频率统计和离线检测固定按1ms周期运行
        Vision.USB_Offline_Detection_1ms(1U);

        osDelay(1U);
    }
}

/**
 * @brief USB数据发送任务
 *
 * 初始化完成后，每1ms将Temp自增一次并通过Vision发送。
 *
 * @param argument FreeRTOS任务参数，本任务不使用
 */
extern "C" void USBTaskFunction(void *argument)
{
    uint32_t Temp = 0U;

    (void)argument;

    for (;;)
    {
        if (!Global_Init_Finished)
        {
            osDelay(1U);
            continue;
        }

        ++Temp;
        Vision.Set_Transmit_Temp(Temp);
        Vision.USB_Transmit();
        osDelay(1U);
    }
}
