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

#include "cmsis_os2.h"
#include "Application/App_Chassis.h"

/**
 * @brief 工程初始化任务
 *
 * 本文件中的强定义覆盖CubeMX生成的弱定义。底盘初始化完成后退出本任务。
 *
 * @param argument FreeRTOS任务参数，本任务不使用
 */
void InitTaskFunction(void *argument)
{
    (void)argument;

    //初始化差速底盘、四个M3508、PID和CAN2接收
    App_Chassis_Init();

    osThreadExit();
}

/**
 * @brief 工程主周期任务
 *
 * 每1ms刷新一次底盘反馈、Debug模式和电机控制输出。
 *
 * @param argument FreeRTOS任务参数，本任务不使用
 */
void MainTaskFunction(void *argument)
{
    (void)argument;

    for (;;)
    {
        App_Chassis_Update();
        osDelay(1U);
    }
}
