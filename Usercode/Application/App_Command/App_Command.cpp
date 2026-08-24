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
#include "Application/App_Remote/App_Remote.h"

//============================== 运行状态 ==============================//

//默认无力，只有遥控在线且左拨杆稳定处于中档才允许运动
static Enum_App_Command_Mode_e App_Command_Current_Mode = App_Command_Mode_No_Power;

//============================== 对外接口 ==============================//

/**
 * @brief 初始化遥控命令状态
 */
void App_Command_Init(void)
{
    App_Command_Current_Mode = App_Command_Mode_No_Power;
}

/**
 * @brief 更新遥控命令状态并刷新底盘目标
 */
void App_Command_Update(void)
{
    Enum_App_Command_Mode_e Target_Mode = App_Command_Mode_No_Power;

    if (App_Remote_Get_Online_State())
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
            //TODO: 后续在此接入导航控制目标，当前保持无力。
            break;
        }

        case App_Command_Mode_No_Power:
        default:
        {
            break;
        }
    }
}
