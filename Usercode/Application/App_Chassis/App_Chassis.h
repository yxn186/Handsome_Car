/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Chassis.h
  * @brief   App层差速底盘控制接口
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_CHASSIS_H__
#define __APP_CHASSIS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/

/**
 * @brief App_Chassis调试模式
 *
 * 调试时直接修改App_Chassis_Debug.Mode_Flag即可切换控制路径：
 * 0：正常控制；1：底盘X/Wz调试；2：四轮目标转速调试；3：四轮原始输出调试。
 */
typedef enum
{
    App_Chassis_Debug_Mode_Normal = 0,
    App_Chassis_Debug_Mode_Chassis_Target,
    App_Chassis_Debug_Mode_Wheel_Target,
    App_Chassis_Debug_Mode_Wheel_Output
} Enum_App_Chassis_Debug_Mode_e;

//============================== Debug变量 ==============================//

/**
 * @brief App_Chassis调试数据
 *
 * 在Ozone中展开App_Chassis_Debug即可集中修改和观察全部调试变量。
 */
typedef struct
{
    uint8_t Mode_Flag;       //调试模式Flag，默认0为正常控制
    float Speed_X;           //模式1：底盘前后速度，单位m/s
    float W_Z;               //模式1：底盘旋转角速度，单位rad/s
    float Wheel_Target[4];   //模式2：四个车轮的目标角速度，单位rad/s
    int16_t Wheel_Output[4]; //模式3：四个M3508的原始输出
} App_Chassis_Debug_t;

//使用volatile保证Ozone能够实时修改和观察结构体内的数据
extern volatile App_Chassis_Debug_t App_Chassis_Debug;

//============================== 对外接口 ==============================//

/**
 * @brief 初始化差速底盘、电机、PID和CAN2接收
 */
void App_Chassis_Init(void);

/**
 * @brief 设置正常模式的底盘目标
 *
 * 正常模式下应持续调用本函数刷新目标，否则超过配置时间后自动零输出。
 *
 * @param Speed_X 底盘前后速度，前进为正，单位m/s
 * @param W_Z 底盘旋转角速度，逆时针为正，单位rad/s
 */
void App_Chassis_Set_Target(float Speed_X, float W_Z);

/**
 * @brief 底盘周期更新函数
 *
 * 由MainTask每1ms调用，根据Debug模式选择正常控制、底盘目标调试、
 * 四轮转速调试或四轮原始输出调试。
 */
void App_Chassis_Update(void);

/**
 * @brief 底盘进入无力状态
 *
 * 清除正常目标、Debug变量和PID状态，并立即发送零输出帧。
 */
void App_Chassis_No_Power(void);

/**
 * @brief 获取四个轮电机是否全部在线
 *
 * @return 1 四个电机均在反馈超时时间内
 * @return 0 至少一个电机尚未反馈或反馈已超时
 */
uint8_t App_Chassis_Get_All_Motors_Online(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_CHASSIS_H__ */
