/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Chassis.cpp
  * @brief   App层差速底盘控制库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "App_Chassis.h"
#include "App_Chassis_Config.h"
#include "DifferentialWheel_Chassis_Calculation.h"
#include "DJI_Motor.h"
#include "PID.h"
#include "bsp_can.h"
#include "bxcan_adapter.h"
#include "can.h"

//============================== Debug变量 ==============================//

//默认进入正常控制模式，所有调试目标均为0
volatile App_Chassis_Debug_t App_Chassis_Debug = {};

//============================== 底盘对象 ==============================//

//CAN2适配器和M3508电机组
static Class_BxCAN_Adapter Wheel_Motor_CAN_Adapter;
static Class_DJI_Motor_Group Wheel_Motor_Group;
static Class_DJI_Motor Wheel_Motor[App_Chassis_Wheel_Motor_Count];

//差速底盘解算和四路轮速PID
static Class_DifferentialWheel_Chassis_Calculation DifferentialWheel_Chassis_Calculation;
static Class_PID Wheel_Motor_PID[App_Chassis_Wheel_Motor_Count];

//电机ID和方向与App_Chassis_Config.h一一对应
static const uint8_t Wheel_Motor_ID[App_Chassis_Wheel_Motor_Count] =
{
    Wheel_Motor_0_ID,
    Wheel_Motor_1_ID,
    Wheel_Motor_2_ID,
    Wheel_Motor_3_ID
};

static const int8_t Wheel_Motor_Direction[App_Chassis_Wheel_Motor_Count] =
{
    Wheel_Motor_0_Direction,
    Wheel_Motor_1_Direction,
    Wheel_Motor_2_Direction,
    Wheel_Motor_3_Direction
};

//============================== 运行状态 ==============================//

//反馈时间由CAN中断更新，因此需要使用volatile
static volatile uint32_t Wheel_Motor_Last_Feedback_Time[App_Chassis_Wheel_Motor_Count] = {0U};
static volatile uint8_t Wheel_Motor_Feedback_Initialized[App_Chassis_Wheel_Motor_Count] = {0U};

//每个电机和全部电机的在线状态
static uint8_t Wheel_Motor_Online[App_Chassis_Wheel_Motor_Count] = {0U};
static volatile uint8_t All_Motors_Online = 0U;

//正常模式目标和最后一次目标刷新时间
static volatile float Chassis_Target_Speed_X = 0.0f;
static volatile float Chassis_Target_W_Z = 0.0f;
static volatile uint32_t Chassis_Target_Last_Update_Time = 0U;
static volatile uint8_t Chassis_Target_Initialized = 0U;

//用于检测Debug模式切换并清除上一模式的PID状态
static uint8_t Last_Debug_Mode_Flag = App_Chassis_Debug_Mode_Normal;

//============================== 内部函数 ==============================//

/**
 * @brief 判断CAN标准ID对应的轮电机索引
 *
 * @param CAN_ID 接收到的CAN标准ID
 * @return int8_t 0~3为对应电机索引，-1表示不是本底盘电机反馈
 */
static int8_t App_Chassis_Find_Wheel_Motor_Index(uint16_t CAN_ID)
{
    for (uint8_t i = 0U; i < App_Chassis_Wheel_Motor_Count; i++)
    {
        if (CAN_ID == (0x200U + Wheel_Motor_ID[i]))
        {
            return static_cast<int8_t>(i);
        }
    }

    return -1;
}

/**
 * @brief CAN2 FIFO0接收回调
 *
 * 只处理合法的8字节标准数据帧，并刷新对应M3508的反馈时间。
 *
 * @param Rx_Buffer CAN接收缓冲区
 */
static void App_Chassis_CAN_Rx_Callback(CAN_Rx_Buffer_t *Rx_Buffer)
{
    if (Rx_Buffer == nullptr)
    {
        return;
    }

    if (Rx_Buffer->Header.IDE != CAN_ID_STD ||
        Rx_Buffer->Header.RTR != CAN_RTR_DATA ||
        Rx_Buffer->Header.DLC != 8U)
    {
        return;
    }

    int8_t Motor_Index = App_Chassis_Find_Wheel_Motor_Index(
        static_cast<uint16_t>(Rx_Buffer->Header.StdId));
    if (Motor_Index < 0)
    {
        return;
    }

    Wheel_Motor_Group.Process_Rx_Data(static_cast<uint16_t>(Rx_Buffer->Header.StdId),
                                      Rx_Buffer->Data,
                                      static_cast<uint8_t>(Rx_Buffer->Header.DLC));

    Wheel_Motor_Last_Feedback_Time[Motor_Index] = HAL_GetTick();
    Wheel_Motor_Feedback_Initialized[Motor_Index] = 1U;
}

/**
 * @brief 重置四路轮速PID
 */
static void App_Chassis_Reset_PID(void)
{
    for (uint8_t i = 0U; i < App_Chassis_Wheel_Motor_Count; i++)
    {
        Wheel_Motor_PID[i].Reset();
    }
}

/**
 * @brief 清零四个电机输出并发送零控制帧
 *
 * 电机掉线或目标超时时每周期调用，防止电调保持上一次非零命令。
 */
static void App_Chassis_Force_Zero_Output(void)
{
    App_Chassis_Reset_PID();

    for (uint8_t i = 0U; i < App_Chassis_Wheel_Motor_Count; i++)
    {
        Wheel_Motor[i].Set_Out(0);
    }

    Wheel_Motor_Group.Push_Data();
}

/**
 * @brief 刷新电机在线状态、PID反馈和差速底盘当前轮速
 *
 * @param Now_ms 当前系统时间，单位ms
 */
static void App_Chassis_Update_State(uint32_t Now_ms)
{
    All_Motors_Online = 1U;

    for (uint8_t i = 0U; i < App_Chassis_Wheel_Motor_Count; i++)
    {
        if (Wheel_Motor_Feedback_Initialized[i] != 0U &&
            (Now_ms - Wheel_Motor_Last_Feedback_Time[i]) <= App_Chassis_Feedback_Timeout_ms)
        {
            Wheel_Motor_Online[i] = 1U;
        }
        else
        {
            Wheel_Motor_Online[i] = 0U;
            All_Motors_Online = 0U;
        }

        //反馈和输出使用同一个方向修正，PID内部统一使用底盘正方向
        float Wheel_Current_Angular_Speed =
            Wheel_Motor[i].Get_AngleSpeed() * Wheel_Motor_Direction[i];
        Wheel_Motor_PID[i].Set_Current_Speed(Wheel_Current_Angular_Speed);
        DifferentialWheel_Chassis_Calculation.Set_Current_Wheel_Motor_Data(
            i,
            Wheel_Current_Angular_Speed);
    }
}

/**
 * @brief 判断正常模式目标是否仍在有效时间内
 *
 * @param Now_ms 当前系统时间，单位ms
 * @return 1 目标有效
 * @return 0 尚未设置目标或目标已经超时
 */
static uint8_t App_Chassis_Normal_Target_Is_Fresh(uint32_t Now_ms)
{
    if (Chassis_Target_Initialized == 0U)
    {
        return 0U;
    }

    return ((Now_ms - Chassis_Target_Last_Update_Time) <= App_Chassis_Command_Timeout_ms) ? 1U : 0U;
}

/**
 * @brief 将差速解算结果作为四路PID目标并发送控制帧
 */
static void App_Chassis_Control_Differential_Target(void)
{
    for (uint8_t i = 0U; i < App_Chassis_Wheel_Motor_Count; i++)
    {
        Wheel_Motor_PID[i].Set_Speed_Target(DifferentialWheel_Chassis_Calculation.Get_Target_Wheel_Angular_Speed(i));
        Wheel_Motor_PID[i].Control_Speed_To_Out();

        int16_t Motor_Output = static_cast<int16_t>(Wheel_Motor_PID[i].Get_Out() * Wheel_Motor_Direction[i]);
        Wheel_Motor[i].Set_Out(Motor_Output);
    }

    Wheel_Motor_Group.Push_Data();
}

/**
 * @brief 使用四个Debug轮速目标进行PID控制
 */
static void App_Chassis_Control_Debug_Wheel_Target(void)
{
    for (uint8_t i = 0U; i < App_Chassis_Wheel_Motor_Count; i++)
    {
        Wheel_Motor_PID[i].Set_Speed_Target(App_Chassis_Debug.Wheel_Target[i]);
        Wheel_Motor_PID[i].Control_Speed_To_Out();

        int16_t Motor_Output = static_cast<int16_t>(
            Wheel_Motor_PID[i].Get_Out() * Wheel_Motor_Direction[i]);
        Wheel_Motor[i].Set_Out(Motor_Output);
    }

    Wheel_Motor_Group.Push_Data();
}

/**
 * @brief 使用四个Debug原始输出直接控制M3508
 *
 * 本模式不经过差速解算、PID和方向修正。每个电机单独检查反馈状态，
 * 在线电机使用Debug输出，离线电机保持零输出。
 */
static void App_Chassis_Control_Debug_Wheel_Output(void)
{
    App_Chassis_Reset_PID();

    for (uint8_t i = 0U; i < App_Chassis_Wheel_Motor_Count; i++)
    {
        if (Wheel_Motor_Online[i] != 0U)
        {
            //不在App层额外限幅，最终限幅由Class_DJI_Motor::Set_Out完成
            Wheel_Motor[i].Set_Out(App_Chassis_Debug.Wheel_Output[i]);
        }
        else
        {
            Wheel_Motor[i].Set_Out(0);
        }
    }

    Wheel_Motor_Group.Push_Data();
}

//============================== 对外接口 ==============================//

/**
 * @brief 初始化差速底盘、电机、PID和CAN2接收
 */
void App_Chassis_Init(void)
{
    //初始化差速底盘解算
    DifferentialWheel_Chassis_Calculation.Init(App_Chassis_b,
                                               App_Chassis_Wheel_Radius,
                                               App_Chassis_Max_Wheel_Motor_Linear_Speed);

    //初始化CAN2适配器、M3508电机组和四个轮电机
    Wheel_Motor_CAN_Adapter.Init(&hcan2);
    Wheel_Motor_Group.Init(&Wheel_Motor_CAN_Adapter, DJI_Motor_3508);

    for (uint8_t i = 0U; i < App_Chassis_Wheel_Motor_Count; i++)
    {
        Wheel_Motor[i].Init(DJI_Motor_3508, Wheel_Motor_ID[i], &Wheel_Motor_Group);

        //配置四路相同的速度环PID参数
        Wheel_Motor_PID[i].Kp_s = Wheel_Motor_PID_Kp_s;
        Wheel_Motor_PID[i].Ki_s = Wheel_Motor_PID_Ki_s;
        Wheel_Motor_PID[i].Kd_s = Wheel_Motor_PID_Kd_s;
        Wheel_Motor_PID[i].ErrorInt_High_s = Wheel_Motor_PID_ErrorInt_High_s;
        Wheel_Motor_PID[i].ErrorInt_Low_s = Wheel_Motor_PID_ErrorInt_Low_s;
        Wheel_Motor_PID[i].Integral_Stop_Near_Zero_Enable_s =
            Wheel_Motor_PID_Integral_Stop_Near_Zero_Enable_s;
        Wheel_Motor_PID[i].Integral_Stop_Target_Abs_Threshold_s =
            Wheel_Motor_PID_Integral_Stop_Target_Abs_Threshold_s;
        Wheel_Motor_PID[i].Integral_Stop_Error_Abs_Threshold_s =
            Wheel_Motor_PID_Integral_Stop_Error_Abs_Threshold_s;
        Wheel_Motor_PID[i].Out_High = Wheel_Motor_PID_Out_High;
        Wheel_Motor_PID[i].Out_Low = Wheel_Motor_PID_Out_Low;
    }

    //注册CAN2 FIFO0回调并接收M3508反馈ID范围
    CAN_Register_RxCallBack_FIFO0_Function(App_Chassis_CAN_Rx_Callback);
    CAN_Filter_Mask_Config(&hcan2,
                           CAN_FILTER(App_Chassis_CAN_Filter_Bank) | CAN_FIFO_0 | CAN_STDID | CAN_DATA_TYPE,
                           App_Chassis_CAN_Filter_ID,
                           App_Chassis_CAN_Filter_Mask);
    CAN_Init(&hcan2);

    //初始化完成后先发送一次零输出帧
    App_Chassis_No_Power();
}

/**
 * @brief 设置正常模式的底盘目标
 *
 * @param Speed_X 底盘前后速度，前进为正，单位m/s
 * @param W_Z 底盘旋转角速度，逆时针为正，单位rad/s
 */
void App_Chassis_Set_Target(float Speed_X, float W_Z)
{
    Chassis_Target_Speed_X = Speed_X;
    Chassis_Target_W_Z = W_Z;
    Chassis_Target_Last_Update_Time = HAL_GetTick();
    Chassis_Target_Initialized = 1U;
}

/**
 * @brief 底盘周期更新函数
 */
void App_Chassis_Update(void)
{
    uint32_t Now_ms = HAL_GetTick();
    uint8_t Debug_Mode = App_Chassis_Debug.Mode_Flag;

    //无论是否允许输出，都持续刷新反馈和底盘当前状态
    App_Chassis_Update_State(Now_ms);

    //模式切换时清除上一模式留下的积分和PID输出
    if (Debug_Mode != Last_Debug_Mode_Flag)
    {
        App_Chassis_Reset_PID();
        Last_Debug_Mode_Flag = Debug_Mode;
    }

    switch (Debug_Mode)
    {
        case App_Chassis_Debug_Mode_Normal:
        {
            //正常模式要求四台电机在线，并要求上层持续刷新控制目标
            if (All_Motors_Online == 0U ||
                App_Chassis_Normal_Target_Is_Fresh(Now_ms) == 0U)
            {
                App_Chassis_Force_Zero_Output();
                return;
            }

            DifferentialWheel_Chassis_Calculation.Set_Target_Chassis_Data(
                Chassis_Target_Speed_X,
                Chassis_Target_W_Z);
            DifferentialWheel_Chassis_Calculation.Update();
            App_Chassis_Control_Differential_Target();
            break;
        }

        case App_Chassis_Debug_Mode_Chassis_Target:
        {
            //模式1：Debug X/Wz经过差速解算和四路速度PID
            if (All_Motors_Online == 0U)
            {
                App_Chassis_Force_Zero_Output();
                return;
            }

            DifferentialWheel_Chassis_Calculation.Set_Target_Chassis_Data(
                App_Chassis_Debug.Speed_X,
                App_Chassis_Debug.W_Z);
            DifferentialWheel_Chassis_Calculation.Update();
            App_Chassis_Control_Differential_Target();
            break;
        }

        case App_Chassis_Debug_Mode_Wheel_Target:
        {
            //模式2：四个Debug轮速目标直接进入对应PID
            if (All_Motors_Online == 0U)
            {
                App_Chassis_Force_Zero_Output();
                return;
            }

            App_Chassis_Control_Debug_Wheel_Target();
            break;
        }

        case App_Chassis_Debug_Mode_Wheel_Output:
        {
            //模式3：每台在线电机使用对应原始输出，离线电机单独归零
            App_Chassis_Control_Debug_Wheel_Output();
            break;
        }

        default:
        {
            //未知模式一律进入安全零输出
            App_Chassis_Force_Zero_Output();
            break;
        }
    }
}

/**
 * @brief 底盘进入无力状态
 */
void App_Chassis_No_Power(void)
{
    Chassis_Target_Speed_X = 0.0f;
    Chassis_Target_W_Z = 0.0f;
    Chassis_Target_Last_Update_Time = 0U;
    Chassis_Target_Initialized = 0U;

    App_Chassis_Debug.Mode_Flag = App_Chassis_Debug_Mode_Normal;
    App_Chassis_Debug.Speed_X = 0.0f;
    App_Chassis_Debug.W_Z = 0.0f;
    Last_Debug_Mode_Flag = App_Chassis_Debug_Mode_Normal;

    for (uint8_t i = 0U; i < App_Chassis_Wheel_Motor_Count; i++)
    {
        App_Chassis_Debug.Wheel_Target[i] = 0.0f;
        App_Chassis_Debug.Wheel_Output[i] = 0;
    }

    DifferentialWheel_Chassis_Calculation.Set_Target_Chassis_Data(0.0f, 0.0f);
    DifferentialWheel_Chassis_Calculation.Update();
    App_Chassis_Force_Zero_Output();
}

/**
 * @brief 获取四个轮电机是否全部在线
 *
 * @return 1 四个电机均在线
 * @return 0 至少一个电机离线
 */
uint8_t App_Chassis_Get_All_Motors_Online(void)
{
    return All_Motors_Online;
}
