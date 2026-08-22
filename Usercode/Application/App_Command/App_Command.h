/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Command.h
  * @brief   App层遥控命令接口
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_COMMAND_H__
#define __APP_COMMAND_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/

/**
 * @brief 底盘命令模式
 *
 * 下档无力；中档遥控；上档预留给后续导航控制。
 */
typedef enum
{
    App_Command_Mode_No_Power = 0,
    App_Command_Mode_Remote,
    App_Command_Mode_Navigation_Reserved
} Enum_App_Command_Mode_e;

//============================== 对外接口 ==============================//

/**
 * @brief 初始化遥控命令状态
 */
void App_Command_Init(void);

/**
 * @brief 更新遥控命令状态并刷新底盘目标
 *
 * 由MainTask每1ms调用，必须位于App_Remote_Update之后、
 * App_Chassis_Update之前。
 */
void App_Command_Update(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_COMMAND_H__ */
