/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Remote.h
  * @brief   App层DR16遥控器接收接口
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_REMOTE_H__
#define __APP_REMOTE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdbool.h>

/*YOUR CODE*/

/**
 * @brief DR16拨杆状态
 *
 * 切换触发状态只保持一个更新周期，之后进入对应的稳定状态。
 */
typedef enum
{
    App_Remote_Switch_Status_Up = 0,
    App_Remote_Switch_Status_Trig_Up_Middle,
    App_Remote_Switch_Status_Trig_Middle_Up,
    App_Remote_Switch_Status_Middle,
    App_Remote_Switch_Status_Trig_Middle_Down,
    App_Remote_Switch_Status_Trig_Down_Middle,
    App_Remote_Switch_Status_Down
} Enum_App_Remote_Switch_Status_e;

//============================== 对外接口 ==============================//

/**
 * @brief 初始化DR16遥控器和USART3 DMA接收
 */
void App_Remote_Init(void);

/**
 * @brief 更新DR16数据和在线状态
 *
 * 由MainTask每1ms调用，内部每100ms执行一次遥控器在线检测。
 */
void App_Remote_Update(void);

/**
 * @brief 获取DR16是否在线
 *
 * @return true 100ms检测周期内接收到DR16数据
 * @return false 100ms检测周期内没有接收到数据
 */
bool App_Remote_Get_Online_State(void);

/**
 * @brief 获取右摇杆X轴数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Right_X(void);

/**
 * @brief 获取右摇杆Y轴数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Right_Y(void);

/**
 * @brief 获取左摇杆X轴数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Left_X(void);

/**
 * @brief 获取左摇杆Y轴数据
 *
 * @return float 归一化到-1~1的摇杆数据
 */
float App_Remote_Get_Left_Y(void);

/**
 * @brief 获取DR16拨轮数据
 *
 * @return float 归一化到-1~1的拨轮数据
 */
float App_Remote_Get_Dial_Wheel(void);

/**
 * @brief 获取左侧拨杆状态
 *
 * @return Enum_App_Remote_Switch_Status_e 左侧拨杆状态
 */
Enum_App_Remote_Switch_Status_e App_Remote_Get_Left_Switch(void);

/**
 * @brief 获取右侧拨杆状态
 *
 * @return Enum_App_Remote_Switch_Status_e 右侧拨杆状态
 */
Enum_App_Remote_Switch_Status_e App_Remote_Get_Right_Switch(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_REMOTE_H__ */
