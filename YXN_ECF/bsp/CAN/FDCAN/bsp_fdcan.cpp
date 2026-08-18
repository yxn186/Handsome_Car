/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_fdcan.cpp
  * @brief   FDCAN库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "bsp_fdcan.h"

Struct_CAN_Manage_Object CAN1_Manage_Object = {nullptr};
Struct_CAN_Manage_Object CAN2_Manage_Object = {nullptr};
Struct_CAN_Manage_Object CAN3_Manage_Object = {nullptr};

//TxData 不同ID 好像没啥用
// CAN1
// uint8_t CAN1_0x1fe_Tx_Data[8];
// uint8_t CAN1_0x1ff_Tx_Data[8];
// uint8_t CAN1_0x200_Tx_Data[8];
// uint8_t CAN1_0x2fe_Tx_Data[8];
// uint8_t CAN1_0x2ff_Tx_Data[8];
// uint8_t CAN1_0x3fe_Tx_Data[8];
// uint8_t CAN1_0x4fe_Tx_Data[8];

// // CAN2
// uint8_t CAN2_0x1fe_Tx_Data[8];
// uint8_t CAN2_0x1ff_Tx_Data[8];
// uint8_t CAN2_0x200_Tx_Data[8];
// uint8_t CAN2_0x2fe_Tx_Data[8];
// uint8_t CAN2_0x2ff_Tx_Data[8];
// uint8_t CAN2_0x3fe_Tx_Data[8];
// uint8_t CAN2_0x4fe_Tx_Data[8];

// // CAN3
// uint8_t CAN3_0x1fe_Tx_Data[8];
// uint8_t CAN3_0x1ff_Tx_Data[8];
// uint8_t CAN3_0x200_Tx_Data[8];
// uint8_t CAN3_0x2fe_Tx_Data[8];
// uint8_t CAN3_0x2ff_Tx_Data[8];
// uint8_t CAN3_0x3fe_Tx_Data[8];
// uint8_t CAN3_0x4fe_Tx_Data[8];

/**
 * @brief 配置CAN的过滤器
 * 默认开了fifo0和fifo1的全通滤波器, 但由于fifo0和fifo1匹配规则一致, 因此fifo1理论上不会被触发, 即使fifo0满
 * 如若出现接收满的情况, 可配置掩码选择性接收, 合理分担总线带宽
 * 此外, Cortex-M7内核的滤波器配置在每个CAN实例都是独立的, 且滤波器编号也都是独立的
 * 如, F4系列芯片的CAN1和CAN2的滤波器编号分别是0-13和14-27, 但H7系列芯片的FDCAN1和FDCAN2的滤波器编号都可以从0开始
 *
 * @param hfdcan CAN编号
 */
void CAN_Filter_Mask_Config(FDCAN_HandleTypeDef *hfdcan)
{
    FDCAN_FilterTypeDef CAN_Filter_Init_Structure{};

    // 配置fifo0全通滤波器
    CAN_Filter_Init_Structure.IdType = FDCAN_STANDARD_ID;//标准帧
    CAN_Filter_Init_Structure.FilterIndex = 0;//第0号滤波器
    CAN_Filter_Init_Structure.FilterType = FDCAN_FILTER_MASK;//掩码模式
    CAN_Filter_Init_Structure.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;//匹配后放进FIFO0 因此 CubeMX 中必须保证：Rx Fifo0 Elmts Nbr > 0
    CAN_Filter_Init_Structure.FilterID1 = 0x00000000;
    CAN_Filter_Init_Structure.FilterID2 = 0x00000000;
    //掩码匹配关系可以理解为：
    //收到的ID & Mask == FilterID1 & Mask
    //Mask某一位为1：这一位必须匹配
    //Mask某一位为0：这一位不检查

    //上述掩码所有位都是0，意味着没有任何一位需要比较，因此：所有标准ID都匹配

    //把滤波器配置写进当前实例对应的 Message RAM
    HAL_FDCAN_ConfigFilter(hfdcan, &CAN_Filter_Init_Structure);
    
    //HAL_FDCAN_ConfigGlobalFilter
    //函数的四个参数依次表示：
    // 不匹配标准滤波器的标准数据帧怎么处理
    // 不匹配扩展滤波器的扩展数据帧怎么处理
    // 标准遥控帧怎么处理
    // 扩展遥控帧怎么处理
    // 启动CAN中断与总线

    // 全局滤波器, 直接拒绝不符合规则的标准数据帧, 扩展数据帧, 标准遥控帧, 扩展遥控帧
    HAL_FDCAN_ConfigGlobalFilter(hfdcan, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);
    
    //开启中断通知（这里的“开启通知”只是在 FDCAN 外设内部允许这些事件产生中断）
    HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_BUS_OFF | FDCAN_IT_ERROR_PASSIVE | FDCAN_IT_ARB_PROTOCOL_ERROR | FDCAN_IT_DATA_PROTOCOL_ERROR, 0);
    // 上述第二个参数是位组合
    // 上述表示开启了：
    // RX FIFO0有新报文
    // 总线进入Bus-Off
    // 进入错误被动状态
    // 仲裁阶段协议错误
    // 数据阶段协议错误
}

/**
 * @brief FDCAN转换数据长度为DLC宏定义
 * 
 * @param Length 
 * @return uint32_t 
 */
static uint32_t CAN_Convert_Length_To_DLC(uint8_t Length)
{
    static const uint32_t DLC_Table[9] =
    {
        FDCAN_DLC_BYTES_0,
        FDCAN_DLC_BYTES_1,
        FDCAN_DLC_BYTES_2,
        FDCAN_DLC_BYTES_3,
        FDCAN_DLC_BYTES_4,
        FDCAN_DLC_BYTES_5,
        FDCAN_DLC_BYTES_6,
        FDCAN_DLC_BYTES_7,
        FDCAN_DLC_BYTES_8
    };

    if (Length > 8)
    {
        return FDCAN_DLC_BYTES_0;
    }

    return DLC_Table[Length];
}

/**
 * @brief 初始化CAN总线
 *
 * @param hfdcan CAN编号
 * @param Callback_Function 处理回调函数
 */
void CAN_Init(FDCAN_HandleTypeDef *hfdcan, CAN_Callback Callback_Function)
{
    //保险
    if (hfdcan == nullptr) return;

    //先判断是哪一个CAN实例, 然后把对应的HAL句柄和回调函数存入对应的管理结构体中
    if (hfdcan->Instance == FDCAN1)
    {
        CAN1_Manage_Object.CAN_Handler = hfdcan;
        CAN1_Manage_Object.Callback_Function = Callback_Function;
    }
    else if (hfdcan->Instance == FDCAN2)
    {
        CAN2_Manage_Object.CAN_Handler = hfdcan;
        CAN2_Manage_Object.Callback_Function = Callback_Function;
    }
    else if (hfdcan->Instance == FDCAN3)
    {
        CAN3_Manage_Object.CAN_Handler = hfdcan;
        CAN3_Manage_Object.Callback_Function = Callback_Function;
    }

    //配置滤波器和通知
    CAN_Filter_Mask_Config(hfdcan);

    //启动FDCAN
    if (HAL_FDCAN_Start(hfdcan) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief 发送数据帧
 *
 * @param hcan CAN编号
 * @param ID ID
 * @param Data 被发送的数据指针
 * @param Length 长度
 * @return HAL_StatusTypeDef  执行状态
 */
HAL_StatusTypeDef CAN_Transmit_Data(FDCAN_HandleTypeDef *hfdcan, uint16_t ID, uint8_t *Data, uint16_t Length)
{
    if (hfdcan == nullptr || Data == nullptr)
    {
        return HAL_ERROR;
    }

    if (ID > 0x7FF || Length > 8)
    {
        return HAL_ERROR;
    }

    FDCAN_TxHeaderTypeDef tx_header;    //构造报文头

    //把报文放进TX FIFO/Queue
    tx_header.Identifier = ID;  //标准ID 有效范围：0x000 ~ 0x7FF
    tx_header.IdType = FDCAN_STANDARD_ID;//标准帧
    tx_header.TxFrameType = FDCAN_DATA_FRAME;//数据帧
    tx_header.DataLength = CAN_Convert_Length_To_DLC(Length);//发送字节长度
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;//发送节点处于Error Active状态 不用重点关注
    tx_header.BitRateSwitch = FDCAN_BRS_OFF;//不切换数据波特率
    tx_header.FDFormat = FDCAN_CLASSIC_CAN;//使用经典CAN
    tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;//不记录发送事件 对应cubemx中 FDCAN1/2/3 -> Mode -> Tx Event FIFO Enable = 0
    tx_header.MessageMarker = 0;//不记录发送事件 对应cubemx中 FDCAN1/2/3 -> Mode -> Tx Event FIFO Enable = 0

    //加入发送FIFO
    return (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &tx_header, Data));
    //把头部和数据复制到Message RAM的TX FIFO，并提交一个发送请求
    // 之后硬件负责：
    // 等待总线空闲；
    // 参加仲裁；
    // 发送；
    // 自动重发。
    // 所以这是一个相对快速的非阻塞接口。
    // 但是TX FIFO满时，它会返回错误。因此周期发送时不能永远忽略返回值。
}

// /**
//  * @brief CAN的TIM定时器中断发送回调函数
//  *
//  */
// void TIM_100us_CAN_PeriodElapsedCallback()
// {
// }

// /**
//  * @brief CAN的TIM定时器中断发送回调函数
//  *
//  */
// void TIM_1ms_CAN_PeriodElapsedCallback()
// {
//     // DJI电机专属

//     static int mod2 = 0;
//     mod2++;
//     if (mod2 == 2)
//     {
//         mod2 = 0;

//         // 发送实例
//         // CAN_Transmit_Data(&hfdcan2, 0x1fe, CAN2_0x1fe_Tx_Data, 8);
//     }

//     CAN_Transmit_Data(&hfdcan1, 0x1fe, CAN1_0x1fe_Tx_Data, 8);
// }

// FDCAN收到数据
//       ↓
// 产生FDCAN中断
//       ↓
// FDCANx_IT0_IRQHandler()
//       ↓
// HAL_FDCAN_IRQHandler(&hfdcanx)
//       ↓
// HAL内部识别FIFO0新消息
//       ↓
// HAL_FDCAN_RxFifo0Callback()

/**
 * @brief HAL库CAN接收FIFO0中断（底层HAL库会在中断中调用此函数）
 *
 * @param hfdcan CAN编号
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    // 判断程序初始化完成
    if (!init_finished)
    {
        //如果未初始化完成
        // 也得接收, 防止FIFO满
        if (hfdcan->Instance == FDCAN1)
        {
            //为什么while？一次中断触发时，FIFO里可能已经积累了多帧。
            //如果只读取一帧，剩余报文可能继续留在FIFO中。
            //这样可以减小FIFO溢出的风险。
            //当FIFO为空后，HAL_FDCAN_GetRxMessage() 返回非 HAL_OK，循环结束。
            while (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &CAN1_Manage_Object.Rx_Header, CAN1_Manage_Object.Rx_Buffer) == HAL_OK)
            {

            }
        }
        else if (hfdcan->Instance == FDCAN2)
        {
            while (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &CAN2_Manage_Object.Rx_Header, CAN2_Manage_Object.Rx_Buffer) == HAL_OK)
            {

            }
        }
        else if (hfdcan->Instance == FDCAN3)
        {
            while (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &CAN3_Manage_Object.Rx_Header, CAN3_Manage_Object.Rx_Buffer) == HAL_OK)
            {

            }
        }
        return;
    }

    // 选择回调函数
    if (hfdcan->Instance == FDCAN1)
    {
        while (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &CAN1_Manage_Object.Rx_Header, CAN1_Manage_Object.Rx_Buffer) == HAL_OK)
        {
            //记录时间戳
            CAN1_Manage_Object.Rx_Timestamp = SYS_Timestamp.Get_Current_Timestamp();

            if (CAN1_Manage_Object.Callback_Function != nullptr)
            {
                //调用回调函数处理接收到的数据
                CAN1_Manage_Object.Callback_Function(CAN1_Manage_Object.Rx_Header, CAN1_Manage_Object.Rx_Buffer);
            }
        }
    }
    else if (hfdcan->Instance == FDCAN2)
    {
        while (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &CAN2_Manage_Object.Rx_Header, CAN2_Manage_Object.Rx_Buffer) == HAL_OK)
        {
            CAN2_Manage_Object.Rx_Timestamp = SYS_Timestamp.Get_Current_Timestamp();

            if (CAN2_Manage_Object.Callback_Function != nullptr)
            {
                CAN2_Manage_Object.Callback_Function(CAN2_Manage_Object.Rx_Header, CAN2_Manage_Object.Rx_Buffer);
            }
        }
    }
    else if (hfdcan->Instance == FDCAN3)
    {
        while (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &CAN3_Manage_Object.Rx_Header, CAN3_Manage_Object.Rx_Buffer) == HAL_OK)
        {
            CAN3_Manage_Object.Rx_Timestamp = SYS_Timestamp.Get_Current_Timestamp();

            if (CAN3_Manage_Object.Callback_Function != nullptr)
            {
                CAN3_Manage_Object.Callback_Function(CAN3_Manage_Object.Rx_Header, CAN3_Manage_Object.Rx_Buffer);
            }
        }
    }
}

/**
 * @brief HAL库CAN错误中断
 *
 * @param hfdcan CAN编号
 * @param ErrorStatusITs 错误状态
 */
void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
    if ((ErrorStatusITs & FDCAN_IT_BUS_OFF) != RESET)
    {
        // CAN总线离线, 重新启动CAN
        HAL_FDCAN_Stop(hfdcan);
        HAL_FDCAN_Start(hfdcan);
    }
}