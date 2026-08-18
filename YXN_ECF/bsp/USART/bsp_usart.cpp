/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_usart.cpp
  * @brief   usart底层库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "bsp_usart.h"

/*YOUR CODE*/

// 每个端口独立的收发缓冲区（尺寸在 bsp_usart.h 中按端口配置）
static uint8_t UART1_Tx_Buf[UART1_TX_BUFFER_SIZE];
static uint8_t UART1_Rx_Buf[UART1_RX_BUFFER_SIZE];
static uint8_t UART2_Tx_Buf[UART2_TX_BUFFER_SIZE];
static uint8_t UART2_Rx_Buf[UART2_RX_BUFFER_SIZE];
static uint8_t UART3_Tx_Buf[UART3_TX_BUFFER_SIZE];
static uint8_t UART3_Rx_Buf[UART3_RX_BUFFER_SIZE];
static uint8_t UART6_Tx_Buf[UART6_TX_BUFFER_SIZE];
static uint8_t UART6_Rx_Buf[UART6_RX_BUFFER_SIZE];

// 管理对象，通过指针引用各自的静态缓冲区
Struct_UART_Manage_Object UART1_Manage_Object = { nullptr, UART1_Tx_Buf, UART1_Rx_Buf, 0, nullptr, nullptr, nullptr };
Struct_UART_Manage_Object UART2_Manage_Object = { nullptr, UART2_Tx_Buf, UART2_Rx_Buf, 0, nullptr, nullptr, nullptr };
Struct_UART_Manage_Object UART3_Manage_Object = { nullptr, UART3_Tx_Buf, UART3_Rx_Buf, 0, nullptr, nullptr, nullptr };
Struct_UART_Manage_Object UART4_Manage_Object = { nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr };
Struct_UART_Manage_Object UART5_Manage_Object = { nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr };
Struct_UART_Manage_Object UART6_Manage_Object = { nullptr, UART6_Tx_Buf, UART6_Rx_Buf, 0, nullptr, nullptr, nullptr };


/**
 * @brief 钳制UART接收长度，防止DMA越界
 *
 * @param instance UART实例
 * @param requested 请求的接收长度
 * @return uint16_t 实际使用的接收长度
 * @details 将调用方请求的 RX 长度钳制到该端口实际缓冲区大小以内，防止 DMA 越界写入
 */
static uint16_t UART_Clamp_Rx_Length(USART_TypeDef *instance, uint16_t requested)
{
    uint16_t capacity;
    if(instance == USART1)
    {
        capacity = UART1_RX_BUFFER_SIZE;
    }
    else if (instance == USART2)
    {
        capacity = UART2_RX_BUFFER_SIZE;
    }
    else if (instance == USART3)
    {
        capacity = UART3_RX_BUFFER_SIZE;
    }
    else if (instance == USART6)
    {
        capacity = UART6_RX_BUFFER_SIZE;
    }
    else
    {
        return requested;
    }
    return (requested == 0 || requested > capacity) ? capacity : requested;
}

static HAL_StatusTypeDef UART_Start_Receive_To_Idle_DMA(UART_HandleTypeDef *huart, uint8_t *rx_buffer, uint16_t rx_buffer_length)
{
    HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle_DMA(huart, rx_buffer, rx_buffer_length);

    if ((status == HAL_OK) && (huart->hdmarx != nullptr))
    {
        __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
    }

    return status;
}


/**
 * @brief 初始化UART
 *
 * @param huart UART编号
 * @param Rx_Callback_Function 处理回调函数
 * @param Rx_Buffer_Length 接收缓冲区长度
 */
void UART_Init(UART_HandleTypeDef *huart, UART_Tx_Call_Back Tx_Callback_Function, UART_Rx_Call_Back Rx_Callback_Function, uint16_t Rx_Buffer_Length, void *Rx_Callback_Context)
{
    // 钳制到该端口 RX 缓冲区实际大小，防止 DMA 越界（如 Serial 传 512 但 UART3 RX 仅 128）
    Rx_Buffer_Length = UART_Clamp_Rx_Length(huart->Instance, Rx_Buffer_Length);

    if (huart->Instance == USART1)
    {
        UART1_Manage_Object.UART_Handler = huart;
        UART1_Manage_Object.Tx_Callback_Function = Tx_Callback_Function;
        UART1_Manage_Object.Rx_Callback_Function = Rx_Callback_Function;
        UART1_Manage_Object.Rx_Buffer_Length = Rx_Buffer_Length;
        UART1_Manage_Object.Rx_Callback_Context = Rx_Callback_Context;
        UART_Start_Receive_To_Idle_DMA(huart, UART1_Manage_Object.Rx_Buffer, UART1_Manage_Object.Rx_Buffer_Length);
    }
    else if (huart->Instance == USART2)
    {
        UART2_Manage_Object.UART_Handler = huart;
        UART2_Manage_Object.Tx_Callback_Function = Tx_Callback_Function;
        UART2_Manage_Object.Rx_Callback_Function = Rx_Callback_Function;
        UART2_Manage_Object.Rx_Buffer_Length = Rx_Buffer_Length;
        UART2_Manage_Object.Rx_Callback_Context = Rx_Callback_Context;
        UART_Start_Receive_To_Idle_DMA(huart, UART2_Manage_Object.Rx_Buffer, UART2_Manage_Object.Rx_Buffer_Length);
    }
    else if (huart->Instance == USART3)
    {
        UART3_Manage_Object.UART_Handler = huart;
        UART3_Manage_Object.Tx_Callback_Function = Tx_Callback_Function;
        UART3_Manage_Object.Rx_Callback_Function = Rx_Callback_Function;
        UART3_Manage_Object.Rx_Buffer_Length = Rx_Buffer_Length;
        UART3_Manage_Object.Rx_Callback_Context = Rx_Callback_Context;
        UART_Start_Receive_To_Idle_DMA(huart, UART3_Manage_Object.Rx_Buffer, UART3_Manage_Object.Rx_Buffer_Length);
    }
    else if (huart->Instance == USART6)
    {
        UART6_Manage_Object.UART_Handler = huart;
        UART6_Manage_Object.Tx_Callback_Function = Tx_Callback_Function;
        UART6_Manage_Object.Rx_Callback_Function = Rx_Callback_Function;
        UART6_Manage_Object.Rx_Buffer_Length = Rx_Buffer_Length;
        UART6_Manage_Object.Rx_Callback_Context = Rx_Callback_Context;
        UART_Start_Receive_To_Idle_DMA(huart, UART6_Manage_Object.Rx_Buffer, UART6_Manage_Object.Rx_Buffer_Length);
    }

}

/**
 * @brief 掉线重新初始化UART
 *
 * @param huart UART编号
 */
void UART_Reinit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        UART_Start_Receive_To_Idle_DMA(huart, UART1_Manage_Object.Rx_Buffer, UART1_Manage_Object.Rx_Buffer_Length);
    }
    else if (huart->Instance == USART2)
    {
        UART_Start_Receive_To_Idle_DMA(huart, UART2_Manage_Object.Rx_Buffer, UART2_Manage_Object.Rx_Buffer_Length);
    }
    else if (huart->Instance == USART3)
    {
        UART_Start_Receive_To_Idle_DMA(huart, UART3_Manage_Object.Rx_Buffer, UART3_Manage_Object.Rx_Buffer_Length);
    }
    else if (huart->Instance == USART6)
    {
        UART_Start_Receive_To_Idle_DMA(huart, UART6_Manage_Object.Rx_Buffer, UART6_Manage_Object.Rx_Buffer_Length);
    }
}

/**
 * @brief 发送数据帧
 *
 * @param huart UART编号
 * @param Data 被发送的数据指针
 * @param Length 长度
 * @return uint8_t 执行状态
 */
uint8_t UART_Transmit_Data(UART_HandleTypeDef *huart, uint8_t *Data, uint16_t Length)
{
    return (HAL_UART_Transmit_DMA(huart, Data, Length));
}

/**
 * @brief UART的TIM定时器中断发送回调函数
 *
 */
void TIM_1ms_UART_PeriodElapsedCallback()
{

}

/**
 * @brief HAL库UART发送完成回调函数
 *
 * @param huart UART编号
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    // 选择回调函数
    if (huart->Instance == USART1)
    {
        if(UART1_Manage_Object.Tx_Callback_Function != nullptr)
        {
            UART1_Manage_Object.Tx_Callback_Function(UART1_Manage_Object.UART_Handler);
        }
    }
    else if (huart->Instance == USART2)
    {
        if(UART2_Manage_Object.Tx_Callback_Function != nullptr)
        {
            UART2_Manage_Object.Tx_Callback_Function(UART2_Manage_Object.UART_Handler);
        }
    }
    else if (huart->Instance == USART3)
    {
        if(UART3_Manage_Object.Tx_Callback_Function != nullptr)
        {
            UART3_Manage_Object.Tx_Callback_Function(UART3_Manage_Object.UART_Handler);
        }
    }
    else if (huart->Instance == USART6)
    {
        if(UART6_Manage_Object.Tx_Callback_Function != nullptr)
        {
            UART6_Manage_Object.Tx_Callback_Function(UART6_Manage_Object.UART_Handler);
        }
    }
}

/**
 * @brief HAL库UART接收DMA空闲中断回调函数
 *
 * @param huart UART编号
 * @param Size 长度
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // 判断程序初始化完成
    if (!Global_Init_Finished)
    {
        // 重启接收
        if (huart->Instance == USART1)
        {
            UART_Start_Receive_To_Idle_DMA(huart, UART1_Manage_Object.Rx_Buffer, UART1_Manage_Object.Rx_Buffer_Length);
        }
        else if (huart->Instance == USART2)
        {
            UART_Start_Receive_To_Idle_DMA(huart, UART2_Manage_Object.Rx_Buffer, UART2_Manage_Object.Rx_Buffer_Length);
        }
        else if (huart->Instance == USART3)
        {
            UART_Start_Receive_To_Idle_DMA(huart, UART3_Manage_Object.Rx_Buffer, UART3_Manage_Object.Rx_Buffer_Length);
        }
        else if (huart->Instance == USART6)
        {
            UART_Start_Receive_To_Idle_DMA(huart, UART6_Manage_Object.Rx_Buffer, UART6_Manage_Object.Rx_Buffer_Length);
        }

        return;
    }

    // 选择回调函数
    if (huart->Instance == USART1)
    {
        if(UART1_Manage_Object.Rx_Callback_Function != nullptr)
        {
            UART1_Manage_Object.Rx_Callback_Function(UART1_Manage_Object.Rx_Callback_Context, UART1_Manage_Object.Rx_Buffer, Size);
        }
        UART_Start_Receive_To_Idle_DMA(huart, UART1_Manage_Object.Rx_Buffer, UART1_Manage_Object.Rx_Buffer_Length);
    }
    else if (huart->Instance == USART2)
    {
        if(UART2_Manage_Object.Rx_Callback_Function != nullptr)
        {
            UART2_Manage_Object.Rx_Callback_Function(UART2_Manage_Object.Rx_Callback_Context, UART2_Manage_Object.Rx_Buffer, Size);
        }
        UART_Start_Receive_To_Idle_DMA(huart, UART2_Manage_Object.Rx_Buffer, UART2_Manage_Object.Rx_Buffer_Length);
    }
    else if (huart->Instance == USART3)
    {
        if(UART3_Manage_Object.Rx_Callback_Function != nullptr)
        {
            UART3_Manage_Object.Rx_Callback_Function(UART3_Manage_Object.Rx_Callback_Context, UART3_Manage_Object.Rx_Buffer, Size);
        }
        UART_Start_Receive_To_Idle_DMA(huart, UART3_Manage_Object.Rx_Buffer, UART3_Manage_Object.Rx_Buffer_Length);
    }
    else if (huart->Instance == USART6)
    {
        if(UART6_Manage_Object.Rx_Callback_Function != nullptr)
        {
            UART6_Manage_Object.Rx_Callback_Function(UART6_Manage_Object.Rx_Callback_Context, UART6_Manage_Object.Rx_Buffer, Size);
        }
        UART_Start_Receive_To_Idle_DMA(huart, UART6_Manage_Object.Rx_Buffer, UART6_Manage_Object.Rx_Buffer_Length);
    }
}

/**
 * @brief HAL库UART错误中断回调函数
 *
 * @param huart UART编号
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        UART_Start_Receive_To_Idle_DMA(huart, UART1_Manage_Object.Rx_Buffer, UART1_Manage_Object.Rx_Buffer_Length);
    }
    else if (huart->Instance == USART2)
    {
        UART_Start_Receive_To_Idle_DMA(huart, UART2_Manage_Object.Rx_Buffer, UART2_Manage_Object.Rx_Buffer_Length);
    }
    else if (huart->Instance == USART3)
    {
        UART_Start_Receive_To_Idle_DMA(huart, UART3_Manage_Object.Rx_Buffer, UART3_Manage_Object.Rx_Buffer_Length);
    }
    else if (huart->Instance == USART6)
    {
        UART_Start_Receive_To_Idle_DMA(huart, UART6_Manage_Object.Rx_Buffer, UART6_Manage_Object.Rx_Buffer_Length);
    }
}
