/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Chassis_Config.h
  * @brief   App层差速底盘配置文件
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_CHASSIS_CONFIG_H__
#define __APP_CHASSIS_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*YOUR CODE*/

//============================== 电机配置 ==============================//

//底盘使用4个M3508，电机索引0、1为左侧，2、3为右侧
#define App_Chassis_Wheel_Motor_Count                                     4U

//M3508电机ID配置，范围1~8
#define Wheel_Motor_0_ID                                                  3U
#define Wheel_Motor_1_ID                                                  2U
#define Wheel_Motor_2_ID                                                  4U
#define Wheel_Motor_3_ID                                                  1U

//电机方向配置，1为正向，-1为反向
//方向会同时作用于速度反馈和PID输出，使PID内部始终使用底盘统一方向
#define Wheel_Motor_0_Direction                                           1
#define Wheel_Motor_1_Direction                                           1
#define Wheel_Motor_2_Direction                                           -1
#define Wheel_Motor_3_Direction                                           -1

//============================== 底盘参数 ==============================//

#define App_Chassis_b                                                     0.26146f  //旋转中心到左、右轮中心的横向距离，单位m
#define App_Chassis_Wheel_Radius                                          0.12f     //车轮半径，单位m
#define App_Chassis_Max_Wheel_Motor_Linear_Speed                          7.0f     //单轮最大线速度，单位m/s

//============================== CAN配置 ==============================//

//M3508使用CAN2、FIFO0，CAN2过滤器编号从14开始
#define App_Chassis_CAN_Filter_Bank                                       14U
#define App_Chassis_CAN_Filter_ID                                         0x200U
#define App_Chassis_CAN_Filter_Mask                                       0x7F0U

//任一电机超过该时间未收到反馈，闭环模式进入零输出保护
#define App_Chassis_Feedback_Timeout_ms                                   100U

//正常模式下，上层超过该时间未刷新目标，底盘进入零输出保护
#define App_Chassis_Command_Timeout_ms                                    100U

//============================== PID配置 ==============================//

//M3508轮电机速度环PID，初始值全部为0，实车调试前按机械参数填写
#define Wheel_Motor_PID_Kp_s                                              600.0f
#define Wheel_Motor_PID_Ki_s                                              0.70f
#define Wheel_Motor_PID_Kd_s                                              0.0f
#define Wheel_Motor_PID_ErrorInt_High_s                                   3000.0f
#define Wheel_Motor_PID_ErrorInt_Low_s                                    -3000.0f
#define Wheel_Motor_PID_Integral_Stop_Near_Zero_Enable_s                  0U
#define Wheel_Motor_PID_Integral_Stop_Target_Abs_Threshold_s              0.0f
#define Wheel_Motor_PID_Integral_Stop_Error_Abs_Threshold_s               0.0f
#define Wheel_Motor_PID_Out_High                                          10000.0f
#define Wheel_Motor_PID_Out_Low                                           -10000.0f

#ifdef __cplusplus
}
#endif

#endif /* __APP_CHASSIS_CONFIG_H__ */
