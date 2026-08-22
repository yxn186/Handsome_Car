/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Command_Config.h
  * @brief   App层遥控命令配置文件
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_COMMAND_CONFIG_H__
#define __APP_COMMAND_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/

//============================== 遥控配置 ==============================//

//左摇杆Y轴满量程对应的底盘前后速度，前推为正，单位m/s
#define App_Command_Remote_Max_Speed_X_mps                                 7.0f

//左摇杆X轴满量程对应的底盘旋转角速度，左推为正，单位rad/s
#define App_Command_Remote_Max_W_Z_radps                                   10.0f

#ifdef __cplusplus
}
#endif

#endif /* __APP_COMMAND_CONFIG_H__ */
