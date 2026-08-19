/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Remote.cpp
  * @brief   App层DR16遥控器接收
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "App_Remote.h"

#include "DR16.h"

/* Private defines -----------------------------------------------------------*/

//每100ms检查一次DR16是否持续发送数据
#define App_Remote_DR16_Alive_Detection_Period_ms 100U

/* Private variables ---------------------------------------------------------*/

//DR16对象只在App_Remote内部使用，不向C任务层暴露C++类型
static Class_DR16 Remote_DR16;
static uint32_t Remote_Last_Alive_Detection_Time = 0U;

//============================== 对外接口 ==============================//

/**
 * @brief 初始化DR16遥控器和USART3 DMA接收
 */
void App_Remote_Init(void)
{
    Remote_Last_Alive_Detection_Time = HAL_GetTick();

    //DR16通过USART3接收固定18字节遥控数据
    Remote_DR16.Init(&huart3);
}

/**
 * @brief 更新DR16数据和在线状态
 *
 * 由MainTask每1ms调用，内部每100ms执行一次遥控器在线检测。
 */
void App_Remote_Update(void)
{
    uint32_t Now_ms = HAL_GetTick();

    //刷新拨杆和按键的稳定状态、触发状态
    Remote_DR16.Task_1ms_Data_Calculate();

    if ((Now_ms - Remote_Last_Alive_Detection_Time) >= App_Remote_DR16_Alive_Detection_Period_ms)
    {
        Remote_Last_Alive_Detection_Time = Now_ms;
        Remote_DR16.Task_100ms_Alive_Detection();
    }
}

/**
 * @brief 获取DR16是否在线
 *
 * @return true DR16在线
 * @return false DR16离线
 */
bool App_Remote_Get_Online_State(void)
{
    return (Remote_DR16.Get_Status() == DR16_Status_ENABLE);
}

/**
 * @brief 获取右摇杆X轴数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Right_X(void)
{
    return Remote_DR16.Get_Right_X();
}

/**
 * @brief 获取右摇杆Y轴数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Right_Y(void)
{
    return Remote_DR16.Get_Right_Y();
}

/**
 * @brief 获取左摇杆X轴数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Left_X(void)
{
    return Remote_DR16.Get_Left_X();
}

/**
 * @brief 获取左摇杆Y轴数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Left_Y(void)
{
    return Remote_DR16.Get_Left_Y();
}

/**
 * @brief 获取DR16拨轮数据
 *
 * @return float 归一化到-1~1的拨轮数据
 */
float App_Remote_Get_Dial_Wheel(void)
{
    return Remote_DR16.Get_Yaw();
}

/**
 * @brief 获取左侧拨杆状态
 *
 * @return Enum_App_Remote_Switch_Status_e 左侧拨杆状态
 */
Enum_App_Remote_Switch_Status_e App_Remote_Get_Left_Switch(void)
{
    return static_cast<Enum_App_Remote_Switch_Status_e>(Remote_DR16.Get_Left_Switch());
}

/**
 * @brief 获取右侧拨杆状态
 *
 * @return Enum_App_Remote_Switch_Status_e 右侧拨杆状态
 */
Enum_App_Remote_Switch_Status_e App_Remote_Get_Right_Switch(void)
{
    return static_cast<Enum_App_Remote_Switch_Status_e>(Remote_DR16.Get_Right_Switch());
}
