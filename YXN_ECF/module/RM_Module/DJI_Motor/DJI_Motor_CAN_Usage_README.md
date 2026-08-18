# DJI Motor CAN / FDCAN 调用说明

当前 `DJI_Motor` 通过 `Class_CAN_Interface` 统一发送接口，可在 STM32 bxCAN 与 FDCAN 之间切换。

## 1. 公共对象

例如创建以下公共对象

```cpp
#include "DJI_Motor.h"

Class_DJI_Motor_Group Chassis_Motor_Group;
Class_DJI_Motor Chassis_Motor_1;
Class_DJI_Motor Chassis_Motor_2;
Class_DJI_Motor Chassis_Motor_3;
Class_DJI_Motor Chassis_Motor_4;
```

电机组、适配器和电机对象应为全局或静态对象，不能在初始化函数内创建为局部对象。

## 2. 使用普通 bxCAN案例

```cpp
#include "bxcan_adapter.h"
#include "bsp_can.h"

//创建适应层对象
Class_BxCAN_Adapter Motor_CAN_Adapter;

//创建接收回调函数
static void Motor_CAN_Rx_Callback(CAN_Rx_Buffer_t *rx_buffer)
{
    if (rx_buffer == nullptr)
    {
        return;
    }

    //进入接收进程函数
    Chassis_Motor_Group.Process_Rx_Data(rx_buffer->Header.StdId,rx_buffer->Data,rx_buffer->Header.DLC);
}

void Motor_Init(void)
{
    //初始化CAN适应层
    Motor_CAN_Adapter.Init(&hcan2);

    //注册电机组
    Chassis_Motor_Group.Init(&Motor_CAN_Adapter,DJI_Motor_3508);

    //初始化电机
    Chassis_Motor_1.Init(DJI_Motor_3508, 1, &Chassis_Motor_Group);
    Chassis_Motor_2.Init(DJI_Motor_3508, 2, &Chassis_Motor_Group);
    Chassis_Motor_3.Init(DJI_Motor_3508, 3, &Chassis_Motor_Group);
    Chassis_Motor_4.Init(DJI_Motor_3508, 4, &Chassis_Motor_Group);

    //注册回调函数
    CAN_Register_RxCallBack_FIFO0_Function(Motor_CAN_Rx_Callback);

    //初始化过滤
    CAN_Filter_Mask_Config(&hcan2,CAN_FILTER(14) | CAN_FIFO_0 | CAN_STDID | CAN_DATA_TYPE,0x200,0x7E0);

    //初始化CAN
    CAN_Init(&hcan2);
}
```

使用 CAN1 时，滤波器编号一般从 `0` 开始；使用 CAN2 时，一般从 `14` 开始。

## 3. 使用 FDCAN

```cpp
#include "fdcan_adapter.h"
#include "bsp_fdcan.h"

//创建适应层对象
Class_FDCAN_Adapter Motor_FDCAN_Adapter;

//创建接收回调函数
static void Motor_FDCAN_Rx_Callback(FDCAN_RxHeaderTypeDef &header,uint8_t *buffer)
{
    if (buffer == nullptr)
    {
        return;
    }

    if (header.IdType != FDCAN_STANDARD_ID ||
        header.RxFrameType != FDCAN_DATA_FRAME ||
        header.DataLength != FDCAN_DLC_BYTES_8)
    {
        return;
    }

    //进入接收进程函数
    Chassis_Motor_Group.Process_Rx_Data(static_cast<uint16_t>(header.Identifier),buffer,8);
}

void Motor_Init(void)
{
    //适应层初始化
    Motor_FDCAN_Adapter.Init(&hfdcan1);

    //电机组初始化
    Chassis_Motor_Group.Init(&Motor_FDCAN_Adapter,DJI_Motor_3508);

    //电机初始化
    Chassis_Motor_1.Init(DJI_Motor_3508, 1, &Chassis_Motor_Group);
    Chassis_Motor_2.Init(DJI_Motor_3508, 2, &Chassis_Motor_Group);
    Chassis_Motor_3.Init(DJI_Motor_3508, 3, &Chassis_Motor_Group);
    Chassis_Motor_4.Init(DJI_Motor_3508, 4, &Chassis_Motor_Group);

    //CAN初始化+注册回调函数
    CAN_Init(&hfdcan1, Motor_FDCAN_Rx_Callback);
}
```

## 4. 周期控制

在控制任务中设置输出，并周期调用 `Push_Data()`：

```cpp
void Motor_Control_1ms(void)
{
    Chassis_Motor_1.Set_Out(3000);
    Chassis_Motor_2.Set_Out(3000);
    Chassis_Motor_3.Set_Out(3000);
    Chassis_Motor_4.Set_Out(3000);

    Chassis_Motor_Group.Push_Data();
}
```

读取反馈：

```cpp
float angle = Chassis_Motor_1.Get_Angle();
float continuous_angle = Chassis_Motor_1.Get_Continuous_Angle();
float angular_speed = Chassis_Motor_1.Get_AngleSpeed();
int16_t current = Chassis_Motor_1.Get_Torque_Current();
uint8_t temperature = Chassis_Motor_1.Get_Temperature();
```

切换 bxCAN 与 FDCAN 时，只需要更换适配器和底层初始化方式，`DJI_Motor` 的控制代码无需修改。
