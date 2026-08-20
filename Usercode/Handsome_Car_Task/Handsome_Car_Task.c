/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Handsome_Car_Task.c
  * @brief   Handsome_Car任务层
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "Handsome_Car_Task.h"

#include <stdbool.h>
#include "cmsis_os2.h"
#include "usb_device.h"
#include "bsp_usb.h"
#include "Application/App_Chassis/App_Chassis.h"
#include "Application/App_Remote/App_Remote.h"

//USART接收回调仅在全部App初始化完成后才向上层分发数据
bool Global_Init_Finished = false;

/**
 * @brief 工程初始化任务
 *
 * 本文件中的强定义覆盖CubeMX生成的弱定义。全部App初始化完成后退出本任务。
 *
 * @param argument FreeRTOS任务参数，本任务不使用
 */
void InitTaskFunction(void *argument)
{
    (void)argument;

    //先准备BSP USB接收缓冲区，再启动CubeMX生成的USB CDC设备
    USB_Init(NULL);
    MX_USB_DEVICE_Init();

    //初始化差速底盘、四个M3508、PID和CAN2接收
    App_Chassis_Init();

    //初始化DR16和USART3 DMA接收
    App_Remote_Init();

    //允许USART接收回调向DR16对象分发数据
    Global_Init_Finished = true;

    osThreadExit();
}

/**
 * @brief 工程主周期任务
 *
 * 初始化完成后，每1ms刷新一次DR16和底盘控制状态。
 *
 * @param argument FreeRTOS任务参数，本任务不使用
 */
void MainTaskFunction(void *argument)
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

        //刷新底盘反馈、Debug模式和电机控制输出
        App_Chassis_Update();
        osDelay(1U);
    }
}
