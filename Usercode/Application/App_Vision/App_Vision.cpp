/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Vision.cpp
  * @brief   App层视觉USB通信
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "App_Vision.h"

#include <string.h>
#include "bsp_usb.h"

/* Private constants ---------------------------------------------------------*/

static constexpr uint8_t App_Vision_Frame_Header = 0xAAU;
static constexpr uint8_t App_Vision_Frame_Tail = 0x55U;

/* Private types -------------------------------------------------------------*/

#pragma pack(push, 1)
typedef struct
{
    uint8_t Frame_Header;
    uint8_t Frame_Tail;
} App_Vision_RX_Frame_t;

typedef struct
{
    uint8_t Frame_Header;
    uint8_t Frame_Tail;
} App_Vision_TX_Frame_t;
#pragma pack(pop)

static_assert(sizeof(App_Vision_RX_Frame_t) == 2U, "App_Vision RX frame size must be 2 bytes");
static_assert(sizeof(App_Vision_TX_Frame_t) == 2U, "App_Vision TX frame size must be 2 bytes");

typedef union
{
    App_Vision_RX_Frame_t Data;
    uint8_t Raw[sizeof(App_Vision_RX_Frame_t)];
} App_Vision_RX_Frame_u;

typedef union
{
    App_Vision_TX_Frame_t Data;
    uint8_t Raw[sizeof(App_Vision_TX_Frame_t)];
} App_Vision_TX_Frame_u;

/* Private variables ---------------------------------------------------------*/

static App_Vision_RX_Frame_u App_Vision_Receive_Union = {};
static App_Vision_TX_Frame_u App_Vision_Transmit_Union = {};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief BSPUSB接收回调
 *
 * 当前协议只校验帧头和帧尾，暂不解析业务数据。
 */
static void App_Vision_USB_Callback(uint8_t *Buffer, uint16_t Length)
{
    if ((Buffer == nullptr) || (Length != sizeof(App_Vision_Receive_Union.Raw)))
    {
        return;
    }

    memcpy(App_Vision_Receive_Union.Raw, Buffer, sizeof(App_Vision_Receive_Union.Raw));

    if ((App_Vision_Receive_Union.Data.Frame_Header != App_Vision_Frame_Header) ||
        (App_Vision_Receive_Union.Data.Frame_Tail != App_Vision_Frame_Tail))
    {
        return;
    }
}

//============================== 对外接口 ==============================//

/**
 * @brief 初始化视觉USB通信并注册接收回调
 */
void App_Vision_Init(void)
{
    memset(&App_Vision_Receive_Union, 0, sizeof(App_Vision_Receive_Union));
    memset(&App_Vision_Transmit_Union, 0, sizeof(App_Vision_Transmit_Union));

    USB_Init(App_Vision_USB_Callback);
}

/**
 * @brief 发送仅包含帧头和帧尾的视觉数据帧
 */
void App_Vision_Transmit(void)
{
    App_Vision_Transmit_Union.Data.Frame_Header = App_Vision_Frame_Header;
    App_Vision_Transmit_Union.Data.Frame_Tail = App_Vision_Frame_Tail;

    (void)USB_Transmit_Data(App_Vision_Transmit_Union.Raw,
                            sizeof(App_Vision_Transmit_Union.Raw));
}
