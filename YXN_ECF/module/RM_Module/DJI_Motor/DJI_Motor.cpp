/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    DJI_Motor.c
  * @brief   大疆系电机库（目前只支持单种电机多个使用）（3508 6020）
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "DJI_Motor.h"
#include <cstring>

#define motor_gear_ratio_inv 0.063432835820f //减速比倒数 268：17

/**
 * @brief 大疆电机组初始化
 * 
 * @param hcan hcanx
 * @param Motor_Type 电机型号 DJI_Motor_6020 / DJI_Motor_3508
 */
void Class_DJI_Motor_Group::Init(Class_CAN_Interface *CAN_Interface, DJI_Motor_Type_Typedef Motor_Type)
{
    this->CAN_Interface = CAN_Interface;
    this->Type = Motor_Type;

    memset(Motor_List, 0, sizeof(Motor_List));
    memset(TxData_Low, 0, sizeof(TxData_Low));
    memset(TxData_High, 0, sizeof(TxData_High));

    // Group_FIFO0 = this;

    // uint8_t CAN_FIlter_BANK = (hcan->Instance == CAN1) ? 0 : 14;

    // CAN_Register_RxCallBack_FIFO0_Function(CAN_RxCallback_Entry);
    // CAN_Filter_Mask_Config(hcan, CAN_FILTER(CAN_FIlter_BANK) | CAN_FIFO_0 | CAN_STDID | CAN_DATA_TYPE, 0x200, 0x7E0);
    // CAN_Init(hcan);
}

/**
 * @brief 大疆电机初始化函数
 * 
 * @param Motor_Type 电机型号 DJI_Motor_6020 / DJI_Motor_3508
 * @param Motor_ID 电机ID
 * @param group 电机组
 */
void Class_DJI_Motor::Init(DJI_Motor_Type_Typedef Motor_Type, uint8_t Motor_ID, Class_DJI_Motor_Group *group)
{
    Type = Motor_Type;
    ID = Motor_ID;
    Group = group;

    RawAngle = 0;
    Speed_Rpm = 0;
    Angle_Speed = 0;
    Torque_Current = 0;
    Temperature = 0;
    Out = 0;
    Angle = 0;
    Last_Angle = 0;
    Continuous_Angle = 0;

    if (Group != nullptr)
    {
        Group->Register_Motor(this);
    }
}

/**
 * @brief 大疆电机获取电机对应CAN-ID的起始ID
 * 
 * @return uint16_t 0x201--3508 0x205--6020
 */
uint16_t Class_DJI_Motor_Group::Get_Rx_Start_ID(void) const
{
    return (Type == DJI_Motor_3508) ? 0x201 : 0x205;
}

/**
 * @brief 大疆电机获取低位ID发送数据CAN标识符
 * 
 * @return uint16_t 0x200--3508 0x1FF--6020
 */
uint16_t Class_DJI_Motor_Group::Get_Tx_Low_ID(void) const
{
    return (Type == DJI_Motor_3508) ? 0x200 : 0x1FF;
}

/**
 * @brief 大疆电机获取高位ID发送数据CAN标识符
 * 
 * @return uint16_t 0x200--3508 0x1FF--6020
 */
uint16_t Class_DJI_Motor_Group::Get_Tx_High_ID(void) const
{
    return (Type == DJI_Motor_3508) ? 0x1FF : 0x2FF;
}

/**
 * @brief 处理接收到的CAN数据进程函数
 *
 * @param id CAN标准帧ID
 * @param data 数据地址
 * @param length 数据长度
 */
void Class_DJI_Motor_Group::Process_Rx_Data(uint16_t id,const uint8_t *data,uint8_t length)
{
    //防空
    if (data == nullptr)
    {
        return;
    }

    // 大疆电机反馈报文固定为8字节
    if (length != 8)
    {
        return;
    }

    uint16_t start_id = Get_Rx_Start_ID();

    if (id < start_id || id >= start_id + 8)
    {
        return;
    }

    uint8_t index = static_cast<uint8_t>(id - start_id);

    if (Motor_List[index] != nullptr)
    {
        Motor_List[index]->FeedBack_Data(data);
    }
}

/**
 * @brief 大疆电机注册电机函数
 * 
 * @param motor Class_DJI_Motor类地址
 */
void Class_DJI_Motor_Group::Register_Motor(Class_DJI_Motor *motor)
{
    if (motor == nullptr)
    {
        return;
    }

    if (motor->Type != Type)
    {
        return;
    }

    uint8_t max_id = (Type == DJI_Motor_3508) ? 8 : 7;

    if (motor->ID == 0 || motor->ID > max_id)
    {
        return;
    }

    Motor_List[motor->ID - 1] = motor;
    motor->Group = this;
}

/**
 * @brief 大疆电机更新反馈数据
 * 
 * @param data 8字节反馈数据 
 */
void Class_DJI_Motor::FeedBack_Data(const uint8_t *data)
{
    if (data == nullptr) return;
    
    RawAngle = (static_cast<uint16_t>(data[0]) << 8) | data[1];

    Angle = RawAngle * 0.0439453125f;// 360.0f / 8192.0f

    float delta_angle = Angle - Last_Angle;

    if (delta_angle > 180.0f)
    {
        delta_angle -= 360.0f;
    }
    else if (delta_angle < -180.0f)
    {
        delta_angle += 360.0f;
    }

    Continuous_Angle += delta_angle;
    Last_Angle = Angle;

    Speed_Rpm = static_cast<int16_t>((static_cast<uint16_t>(data[2]) << 8) | data[3]);

    if (Type == DJI_Motor_3508)
    {
        Angle_Speed = Speed_Rpm * 0.1047197533333f * motor_gear_ratio_inv; // rad/s, gearbox output shaft
    }
    else
    {
        Angle_Speed = Speed_Rpm * 0.1047197533333f; // rad/s
    }

    Torque_Current =static_cast<int16_t>((static_cast<uint16_t>(data[4]) << 8) | data[5]);

    Temperature = data[6];
}

/**
 * @brief 大疆系电机限幅函数
 * 
 * @param out 
 * @return int16_t 
 */
int16_t Class_DJI_Motor::Limit_Out(int16_t out) const
{
    if (Type == DJI_Motor_6020)
    {
        if (out > 25000)
        {
            return 25000;
        }
        if (out < -25000)
        {
            return -25000;
        }
    }
    else if (Type == DJI_Motor_3508)
    {
        if (out > 16384)
        {
            return 16384;
        }
        if (out < -16384)
        {
            return -16384;
        }
    }

    return out;
}

/**
 * @brief 大疆电机设置输出值
 * 
 * @param out 
 */
void Class_DJI_Motor::Set_Out(int16_t out)
{
    Out = Limit_Out(out);
}

float Class_DJI_Motor::Get_AngleSpeed(void)
{
    return Angle_Speed;
}

/**
 * @brief 大疆电机上传数据给电机进行控制
 * 
 */
void Class_DJI_Motor_Group::Push_Data(void)
{
    memset(TxData_Low,  0, 8);
    memset(TxData_High, 0, 8);

    uint8_t need_send_low = 0;
    uint8_t need_send_high = 0;

    uint8_t max_id = (Type == DJI_Motor_3508) ? 8 : 7;

    for (uint8_t i = 0; i < max_id; i++)
    {
        if (Motor_List[i] == nullptr)
        {
            continue;
        }

        uint8_t ID = i + 1;
        int16_t out = Motor_List[i]->Get_Out();

        uint8_t *TxData = nullptr;
        uint8_t Temp_ID = 0;

        if (ID <= 4)
        {
            TxData = TxData_Low;
            Temp_ID = ID - 1;
            need_send_low = 1;
        }
        else
        {
            TxData = TxData_High;
            Temp_ID = ID - 5;
            need_send_high = 1;
        }

        TxData[2 * Temp_ID]     = (uint8_t)((uint16_t)out >> 8);
        TxData[2 * Temp_ID + 1] = (uint8_t)((uint16_t)out);
    }

    if (CAN_Interface == nullptr)
    {
        return;
    }

    if (need_send_low)
    {
        CAN_Interface->Transmit(Get_Tx_Low_ID(),TxData_Low,8);
    }

    if (need_send_high)
    {
        CAN_Interface->Transmit(Get_Tx_High_ID(), TxData_High, 8);
    }
}


