/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Command.cpp
  * @brief   App层遥控命令处理
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "App_Command.h"
#include "App_Command_Config.h"

#include "Application/App_Chassis/App_Chassis.h"
#include "Application/App_Light/App_Light.h"
#include "Application/App_Remote/App_Remote.h"
#include "Application/App_Vision/App_Vision.h"

//============================== 运行状态 ==============================//

//遥控离线时默认使用导航控制。
static Enum_App_Command_Mode_e App_Command_Current_Mode = App_Command_Mode_Navigation_Reserved;

//============================== 对外接口 ==============================//

/**
 * @brief 初始化遥控命令状态
 */
void App_Command_Init(void)
{
    App_Command_Current_Mode = App_Command_Mode_Navigation_Reserved;
}

/**
 * @brief 更新遥控命令状态并刷新底盘目标
 */
void App_Command_Update(void)
{
    float Navigation_Vx = 0.0f;
    float Navigation_Wz = 0.0f;
    bool Capture_Enable = false;
    bool Navigation_Command_Fresh = Vision.Get_Navigation_Command(&Navigation_Vx,
                                                                    &Navigation_Wz,
                                                                    &Capture_Enable);
    bool Remote_Online = App_Remote_Get_Online_State();
    Enum_App_Command_Mode_e Target_Mode = App_Command_Mode_Navigation_Reserved;

    if (Remote_Online)
    {
        switch (App_Remote_Get_Left_Switch())
        {
            case App_Remote_Switch_Status_Down:
            {
                Target_Mode = App_Command_Mode_No_Power;
                break;
            }

            case App_Remote_Switch_Status_Middle:
            {
                Target_Mode = App_Command_Mode_Remote;
                break;
            }

            case App_Remote_Switch_Status_Up:
            {
                Target_Mode = App_Command_Mode_Navigation_Reserved;
                break;
            }

            default:
            {
                //拨杆切换瞬间保持无力，避免状态未稳定时误动作。
                Target_Mode = App_Command_Mode_No_Power;
                break;
            }
        }
    }

    App_Light_Update(Navigation_Command_Fresh,
                     Capture_Enable,
                     Remote_Online,
                     App_Remote_Get_Dial_Wheel());

    if (Target_Mode != App_Command_Current_Mode)
    {
        App_Command_Current_Mode = Target_Mode;

        if (App_Command_Current_Mode != App_Command_Mode_Remote)
        {
            App_Chassis_No_Power();
        }
    }

    switch (App_Command_Current_Mode)
    {
        case App_Command_Mode_Remote:
        {
            float Speed_X_mps =
                App_Remote_Get_Left_Y() * App_Command_Remote_Max_Speed_X_mps;
            float W_Z_radps =
                -App_Remote_Get_Right_X() * App_Command_Remote_Max_W_Z_radps;

            App_Chassis_Set_Target(Speed_X_mps, W_Z_radps);
            break;
        }

        case App_Command_Mode_Navigation_Reserved:
        {
            if (Navigation_Command_Fresh)
            {
                App_Chassis_Set_Target(Navigation_Vx, Navigation_Wz);
            }
            break;
        }

        case App_Command_Mode_No_Power:
        default:
        {
            break;
        }
    }
}
