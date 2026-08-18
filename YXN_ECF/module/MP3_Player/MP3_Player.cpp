/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    MP3_Player.cpp
  * @brief   MP3-TF-16P / Mini MP3 Player 串口控制库
  * @author  yxn
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "MP3_Player.h"
#include "cmsis_os.h"
#include <string.h>

/* Private macros ------------------------------------------------------------*/

// 如果你非常确定想用bsp_usart的DMA发送，可以改成1。
// 默认用HAL_UART_Transmit阻塞发送，因为MP3指令只有10字节，9600bps下约10ms，可靠性更高。
#ifndef MP3_PLAYER_USE_BSP_USART_DMA_TX
#define MP3_PLAYER_USE_BSP_USART_DMA_TX 1
#endif

/* MP3-TF-16P 命令字 */
#define MP3_CMD_NEXT                    0x01U
#define MP3_CMD_PREVIOUS                0x02U
#define MP3_CMD_PLAY_ROOT_TRACK         0x03U
#define MP3_CMD_VOLUME_UP               0x04U
#define MP3_CMD_VOLUME_DOWN             0x05U
#define MP3_CMD_SET_VOLUME              0x06U
#define MP3_CMD_SET_EQ                  0x07U
#define MP3_CMD_LOOP_SPECIFIED_TRACK    0x08U
#define MP3_CMD_SELECT_DEVICE           0x09U
#define MP3_CMD_SLEEP                   0x0AU
#define MP3_CMD_RESET                   0x0CU
#define MP3_CMD_PLAY                    0x0DU
#define MP3_CMD_PAUSE                   0x0EU
#define MP3_CMD_PLAY_FOLDER_TRACK       0x0FU
#define MP3_CMD_LOOP_ALL                0x11U
#define MP3_CMD_PLAY_MP3_TRACK          0x12U
#define MP3_CMD_INSERT_ADVERT           0x13U
#define MP3_CMD_PLAY_FOLDER_TRACK_1000  0x14U
#define MP3_CMD_STOP_ADVERT             0x15U
#define MP3_CMD_STOP                    0x16U
#define MP3_CMD_LOOP_FOLDER             0x17U
#define MP3_CMD_RANDOM_PLAY             0x18U
#define MP3_CMD_LOOP_CURRENT            0x19U
#define MP3_CMD_SET_DAC                 0x1AU

#define MP3_CMD_DEVICE_INSERT           0x3AU
#define MP3_CMD_DEVICE_REMOVE           0x3BU
#define MP3_CMD_U_DISK_FINISHED         0x3CU
#define MP3_CMD_TF_FINISHED             0x3DU
#define MP3_CMD_FLASH_FINISHED          0x3EU
#define MP3_CMD_INIT_RESULT             0x3FU
#define MP3_CMD_ERROR                   0x40U
#define MP3_CMD_ACK                     0x41U
#define MP3_CMD_QUERY_STATUS            0x42U
#define MP3_CMD_QUERY_VOLUME            0x43U
#define MP3_CMD_QUERY_EQ                0x44U
#define MP3_CMD_QUERY_U_DISK_FILES      0x47U
#define MP3_CMD_QUERY_TF_FILES          0x48U
#define MP3_CMD_QUERY_FLASH_FILES       0x49U
#define MP3_CMD_QUERY_U_DISK_TRACK      0x4BU
#define MP3_CMD_QUERY_TF_TRACK          0x4CU
#define MP3_CMD_QUERY_FLASH_TRACK       0x4DU
#define MP3_CMD_QUERY_FOLDER_FILES      0x4EU
#define MP3_CMD_QUERY_TOTAL_FOLDERS     0x4FU

static void MP3_Player_Delay(uint32_t delay_ms)
{
    if (delay_ms == 0U)
    {
        return;
    }

    if (osKernelGetState() == osKernelRunning)
    {
        osDelay(delay_ms);
    }
    else
    {
        HAL_Delay(delay_ms);
    }
}

/**
 * @brief 初始化MP3模块串口
 *
 * @param huart MP3模块连接的UART句柄
 */
void Class_MP3_Player::Init(UART_HandleTypeDef *huart)
{
    Init(huart, nullptr, 0);
}

/**
 * @brief 初始化MP3模块串口，并绑定Busy播放状态引脚
 *
 * @param huart MP3模块连接的UART句柄
 * @param busy_gpio_port Busy引脚GPIO端口；不接Busy可传nullptr
 * @param busy_gpio_pin Busy引脚GPIO Pin
 */
void Class_MP3_Player::Init(UART_HandleTypeDef *huart, GPIO_TypeDef *busy_gpio_port, uint16_t busy_gpio_pin)
{
    this->huart = huart;
    Set_Busy_Pin(busy_gpio_port, busy_gpio_pin);

    memset(Tx_Frame, 0, sizeof(Tx_Frame));
    memset(Rx_Frame, 0, sizeof(Rx_Frame));
    Rx_Frame_Index = 0;

    Module_Online = false;
    Last_Frame_Valid = false;
    Local_Checksum_Error = false;
    Device_Online_Mask = 0;
    Last_Status = MP3_Player_Status_Unknown;
    Last_Error = MP3_Player_Error_None;
    Last_Rx_Command = 0;
    Last_Rx_Parameter = 0;
    Last_Finished_Track = 0;
    Last_Finished_Device_Command = 0;
    Last_Volume = 0;
    Last_EQ = 0;
    Last_Query_Result = 0;
    Last_Rx_Tick = 0;
    Last_Ack_Tick = 0;
    Last_Send_Tick = 0;

    if (this->huart == nullptr)
    {
        return;
    }

    // 使用你的bsp_usart库启动 ReceiveToIdle DMA 接收，并把this作为回调上下文
    UART_Init(this->huart, nullptr, UART_RxCallback_Entry, MP3_PLAYER_FRAME_LENGTH * 2U, this);
}

/**
 * @brief 单独设置Busy播放状态引脚
 *
 * @param busy_gpio_port Busy引脚GPIO端口；传nullptr表示不使用Busy引脚
 * @param busy_gpio_pin Busy引脚GPIO Pin
 */
void Class_MP3_Player::Set_Busy_Pin(GPIO_TypeDef *busy_gpio_port, uint16_t busy_gpio_pin)
{
    Busy_GPIO_Port = busy_gpio_port;
    Busy_GPIO_Pin = busy_gpio_pin;
}

/**
 * @brief 设置两条串口命令之间的最小间隔
 *
 * @param interval_ms 间隔时间，单位ms；设置为0表示不主动延时
 */
void Class_MP3_Player::Set_Send_Interval(uint32_t interval_ms)
{
    Min_Send_Interval_ms = interval_ms;
}

/**
 * @brief 上电后快速进入TF卡播放准备状态
 *
 * @param volume 音量，范围0~30
 * @param power_on_delay_ms 上电初始化等待时间，建议1500~3000ms
 * @return HAL_StatusTypeDef HAL_OK表示最后一条命令发送成功
 */
HAL_StatusTypeDef Class_MP3_Player::Start_TF_Card(uint8_t volume, uint32_t power_on_delay_ms)
{
    HAL_StatusTypeDef status = HAL_OK;

    // 模块上电后需要初始化TF卡/文件系统，不建议立即发播放命令
    if (power_on_delay_ms > 0U)
    {
        MP3_Player_Delay(power_on_delay_ms);
    }

    // 指定播放设备为TF卡。指定设备后，模块还需要约200ms初始化文件系统
    status = Select_Device(MP3_Player_Device_TF);
    MP3_Player_Delay(200);

    if (status != HAL_OK)
    {
        return status;
    }

    status = Set_Volume(volume);
    return status;
}

/**
 * @brief 计算MP3协议校验
 *
 * @param cmd 命令字节
 * @param feedback 是否需要反馈
 * @param param 16位参数
 * @return uint16_t 校验值
 */
uint16_t Class_MP3_Player::Calculate_Checksum(uint8_t cmd, uint8_t feedback, uint16_t param)
{
    uint16_t sum = 0;

    sum += 0xFFU;                         // 版本
    sum += 0x06U;                         // 长度
    sum += cmd;                           // 命令
    sum += feedback;                      // 是否应答
    sum += (uint8_t)(param >> 8);         // 参数高字节
    sum += (uint8_t)(param & 0x00FFU);    // 参数低字节

    // 模块协议：0 - sum
    return (uint16_t)(0U - sum);
}

/**
 * @brief 发送原始命令
 *
 * @param cmd 命令字节
 * @param param 16位参数
 * @param need_feedback 是否需要应答，false=不需要，true=需要
 * @return HAL_StatusTypeDef 发送状态
 */
HAL_StatusTypeDef Class_MP3_Player::Send_Command(uint8_t cmd, uint16_t param, bool need_feedback)
{
    if (huart == nullptr)
    {
        return HAL_ERROR;
    }

    // 控制发包间隔，避免连续命令太快
    if (Min_Send_Interval_ms > 0U && Last_Send_Tick != 0U)
    {
        uint32_t now = HAL_GetTick();
        uint32_t elapsed = now - Last_Send_Tick;

        if (elapsed < Min_Send_Interval_ms)
        {
            MP3_Player_Delay(Min_Send_Interval_ms - elapsed);
        }
    }

    uint8_t feedback = need_feedback ? 0x01U : 0x00U;
    uint16_t checksum = Calculate_Checksum(cmd, feedback, param);

    Tx_Frame[0] = 0x7EU;
    Tx_Frame[1] = 0xFFU;
    Tx_Frame[2] = 0x06U;
    Tx_Frame[3] = cmd;
    Tx_Frame[4] = feedback;
    Tx_Frame[5] = (uint8_t)(param >> 8);
    Tx_Frame[6] = (uint8_t)(param & 0x00FFU);
    Tx_Frame[7] = (uint8_t)(checksum >> 8);
    Tx_Frame[8] = (uint8_t)(checksum & 0x00FFU);
    Tx_Frame[9] = 0xEFU;

#if MP3_PLAYER_USE_BSP_USART_DMA_TX
    HAL_StatusTypeDef status = (HAL_StatusTypeDef)UART_Transmit_Data(huart, Tx_Frame, MP3_PLAYER_FRAME_LENGTH);
#else
    // 10字节在9600bps下约10ms，阻塞发送简单可靠
    HAL_StatusTypeDef status = HAL_UART_Transmit(huart, Tx_Frame, MP3_PLAYER_FRAME_LENGTH, 30);
#endif

    if (status == HAL_OK)
    {
        Last_Send_Tick = HAL_GetTick();
    }

    return status;
}

/* -------------------- 常用播放控制 -------------------- */

HAL_StatusTypeDef Class_MP3_Player::Next(void)
{
    return Send_Command(MP3_CMD_NEXT, 0);
}

HAL_StatusTypeDef Class_MP3_Player::Previous(void)
{
    return Send_Command(MP3_CMD_PREVIOUS, 0);
}

HAL_StatusTypeDef Class_MP3_Player::Play(void)
{
    return Send_Command(MP3_CMD_PLAY, 0);
}

HAL_StatusTypeDef Class_MP3_Player::Pause(void)
{
    return Send_Command(MP3_CMD_PAUSE, 0);
}

HAL_StatusTypeDef Class_MP3_Player::Stop(void)
{
    return Send_Command(MP3_CMD_STOP, 0);
}

HAL_StatusTypeDef Class_MP3_Player::Reset(void)
{
    return Send_Command(MP3_CMD_RESET, 0);
}

HAL_StatusTypeDef Class_MP3_Player::Sleep(void)
{
    return Send_Command(MP3_CMD_SLEEP, 0);
}

HAL_StatusTypeDef Class_MP3_Player::Set_Volume(uint8_t volume)
{
    if (volume > 30U)
    {
        volume = 30U;
    }

    Last_Volume = volume;
    return Send_Command(MP3_CMD_SET_VOLUME, volume);
}

HAL_StatusTypeDef Class_MP3_Player::Volume_Up(void)
{
    return Send_Command(MP3_CMD_VOLUME_UP, 0);
}

HAL_StatusTypeDef Class_MP3_Player::Volume_Down(void)
{
    return Send_Command(MP3_CMD_VOLUME_DOWN, 0);
}

HAL_StatusTypeDef Class_MP3_Player::Set_EQ(MP3_Player_EQ_Typedef eq)
{
    uint16_t eq_value = (uint16_t)eq;

    if (eq_value > 5U)
    {
        eq_value = 0U;
    }

    Last_EQ = (uint8_t)eq_value;
    return Send_Command(MP3_CMD_SET_EQ, eq_value);
}

HAL_StatusTypeDef Class_MP3_Player::Set_DAC(bool enable)
{
    // 资料中：0x0000 = 开DAC，0x0001 = 关DAC高阻
    return Send_Command(MP3_CMD_SET_DAC, enable ? 0x0000U : 0x0001U);
}

HAL_StatusTypeDef Class_MP3_Player::Select_Device(MP3_Player_Device_Typedef device)
{
    return Send_Command(MP3_CMD_SELECT_DEVICE, (uint16_t)device);
}

HAL_StatusTypeDef Class_MP3_Player::Play_Root_Track(uint16_t track)
{
    if (track == 0U)
    {
        return HAL_ERROR;
    }

    return Send_Command(MP3_CMD_PLAY_ROOT_TRACK, track);
}

HAL_StatusTypeDef Class_MP3_Player::Play_MP3_Track(uint16_t track)
{
    if (track == 0U)
    {
        return HAL_ERROR;
    }

    return Send_Command(MP3_CMD_PLAY_MP3_TRACK, track);
}

HAL_StatusTypeDef Class_MP3_Player::Play_Folder_Track(uint8_t folder, uint8_t track)
{
    if (folder == 0U || folder > 99U || track == 0U)
    {
        return HAL_ERROR;
    }

    uint16_t param = ((uint16_t)folder << 8) | track;
    return Send_Command(MP3_CMD_PLAY_FOLDER_TRACK, param);
}

HAL_StatusTypeDef Class_MP3_Player::Play_Folder_Track_1000(uint8_t folder, uint16_t track)
{
    if (folder == 0U || folder > 15U || track == 0U || track > 0x0FFFU)
    {
        return HAL_ERROR;
    }

    // 高4位为文件夹号，低12位为曲目号
    uint16_t param = (uint16_t)(((uint16_t)folder << 12) | (track & 0x0FFFU));
    return Send_Command(MP3_CMD_PLAY_FOLDER_TRACK_1000, param);
}

HAL_StatusTypeDef Class_MP3_Player::Loop_All(bool enable)
{
    return Send_Command(MP3_CMD_LOOP_ALL, enable ? 0x0001U : 0x0000U);
}

HAL_StatusTypeDef Class_MP3_Player::Loop_Specified_Track(uint16_t track)
{
    if (track == 0U)
    {
        return HAL_ERROR;
    }

    return Send_Command(MP3_CMD_LOOP_SPECIFIED_TRACK, track);
}

HAL_StatusTypeDef Class_MP3_Player::Loop_Current(bool enable)
{
    // 资料中：0x0000 = 当前曲目循环开启，0x0001 = 关闭
    return Send_Command(MP3_CMD_LOOP_CURRENT, enable ? 0x0000U : 0x0001U);
}

HAL_StatusTypeDef Class_MP3_Player::Loop_Folder(uint8_t folder)
{
    if (folder == 0U || folder > 99U)
    {
        return HAL_ERROR;
    }

    return Send_Command(MP3_CMD_LOOP_FOLDER, folder);
}

HAL_StatusTypeDef Class_MP3_Player::Random_Play(uint16_t param)
{
    // 不同版本资料中随机播放参数有0x0000/0x0002差异，所以保留参数入口，默认0
    return Send_Command(MP3_CMD_RANDOM_PLAY, param);
}

HAL_StatusTypeDef Class_MP3_Player::Insert_Advert(uint16_t track)
{
    if (track == 0U)
    {
        return HAL_ERROR;
    }

    return Send_Command(MP3_CMD_INSERT_ADVERT, track);
}

HAL_StatusTypeDef Class_MP3_Player::Stop_Advert(void)
{
    return Send_Command(MP3_CMD_STOP_ADVERT, 0);
}

/* -------------------- 查询指令 -------------------- */

HAL_StatusTypeDef Class_MP3_Player::Query_Status(void)
{
    return Send_Command(MP3_CMD_QUERY_STATUS, 0);
}

HAL_StatusTypeDef Class_MP3_Player::Query_Volume(void)
{
    return Send_Command(MP3_CMD_QUERY_VOLUME, 0);
}

HAL_StatusTypeDef Class_MP3_Player::Query_EQ(void)
{
    return Send_Command(MP3_CMD_QUERY_EQ, 0);
}

HAL_StatusTypeDef Class_MP3_Player::Query_TF_Total_Files(void)
{
    return Send_Command(MP3_CMD_QUERY_TF_FILES, 0);
}

HAL_StatusTypeDef Class_MP3_Player::Query_TF_Current_Track(void)
{
    return Send_Command(MP3_CMD_QUERY_TF_TRACK, 0);
}

HAL_StatusTypeDef Class_MP3_Player::Query_Folder_Total_Files(uint8_t folder)
{
    if (folder == 0U || folder > 99U)
    {
        return HAL_ERROR;
    }

    return Send_Command(MP3_CMD_QUERY_FOLDER_FILES, folder);
}

/* -------------------- 状态读取 -------------------- */

bool Class_MP3_Player::Is_Playing_By_BusyPin(void) const
{
    if (Busy_GPIO_Port == nullptr)
    {
        return false;
    }

    // MP3-TF-16P：有音频输出时Busy为低电平
    return (HAL_GPIO_ReadPin(Busy_GPIO_Port, Busy_GPIO_Pin) == GPIO_PIN_RESET);
}

bool Class_MP3_Player::Is_Playing(void) const
{
    if (Busy_GPIO_Port != nullptr)
    {
        return Is_Playing_By_BusyPin();
    }

    return (Last_Status == MP3_Player_Status_Playing);
}

/* -------------------- 接收解析 -------------------- */

void Class_MP3_Player::UART_RxCallback_Entry(void *context, uint8_t *Buffer, uint16_t Length)
{
    if (context == nullptr || Buffer == nullptr || Length == 0U)
    {
        return;
    }

    Class_MP3_Player *player = static_cast<Class_MP3_Player *>(context);
    player->UART_RxCallback(Buffer, Length);
}

void Class_MP3_Player::UART_RxCallback(uint8_t *Buffer, uint16_t Length)
{
    if (Buffer == nullptr || Length == 0U)
    {
        return;
    }

    for (uint16_t i = 0; i < Length; i++)
    {
        Parse_Byte(Buffer[i]);
    }
}

void Class_MP3_Player::Parse_Byte(uint8_t data)
{
    // 看到帧头就重新开始收一帧，避免前面乱字节影响解析
    if (data == 0x7EU)
    {
        Rx_Frame_Index = 0;
        Rx_Frame[Rx_Frame_Index++] = data;
        return;
    }

    if (Rx_Frame_Index == 0U)
    {
        return;
    }

    Rx_Frame[Rx_Frame_Index++] = data;

    if (Rx_Frame_Index >= MP3_PLAYER_FRAME_LENGTH)
    {
        if (Rx_Frame[MP3_PLAYER_FRAME_LENGTH - 1U] == 0xEFU && Check_Frame(Rx_Frame))
        {
            Process_Frame(Rx_Frame);
        }
        else
        {
            Local_Checksum_Error = true;
        }

        Rx_Frame_Index = 0;
    }
}

bool Class_MP3_Player::Check_Frame(const uint8_t frame[MP3_PLAYER_FRAME_LENGTH])
{
    if (frame == nullptr)
    {
        return false;
    }

    if (frame[0] != 0x7EU || frame[1] != 0xFFU || frame[2] != 0x06U || frame[9] != 0xEFU)
    {
        return false;
    }

    uint16_t param = ((uint16_t)frame[5] << 8) | frame[6];
    uint16_t expected_checksum = Calculate_Checksum(frame[3], frame[4], param);
    uint16_t received_checksum = ((uint16_t)frame[7] << 8) | frame[8];

    return (expected_checksum == received_checksum);
}

void Class_MP3_Player::Process_Frame(const uint8_t frame[MP3_PLAYER_FRAME_LENGTH])
{
    uint8_t cmd = frame[3];
    uint16_t param = ((uint16_t)frame[5] << 8) | frame[6];

    Last_Frame_Valid = true;
    Local_Checksum_Error = false;
    Last_Rx_Command = cmd;
    Last_Rx_Parameter = param;
    Last_Rx_Tick = HAL_GetTick();

    switch (cmd)
    {
        case MP3_CMD_INIT_RESULT:
        {
            // 参数低位表示在线设备：U盘=bit0，TF=bit1，PC=bit2，Flash=bit3
            Module_Online = true;
            Device_Online_Mask = param;
            Last_Error = MP3_Player_Error_None;
        } break;

        case MP3_CMD_DEVICE_INSERT:
        {
            Device_Online_Mask |= param;
            Module_Online = true;
        } break;

        case MP3_CMD_DEVICE_REMOVE:
        {
            Device_Online_Mask &= (uint16_t)(~param);
        } break;

        case MP3_CMD_U_DISK_FINISHED:
        case MP3_CMD_TF_FINISHED:
        case MP3_CMD_FLASH_FINISHED:
        {
            Last_Finished_Device_Command = cmd;
            Last_Finished_Track = param;
            Last_Status = MP3_Player_Status_Stopped;
        } break;

        case MP3_CMD_ERROR:
        {
            Last_Error = (MP3_Player_Error_Typedef)param;
        } break;

        case MP3_CMD_ACK:
        {
            Last_Ack_Tick = Last_Rx_Tick;
            Last_Error = MP3_Player_Error_None;
        } break;

        case MP3_CMD_QUERY_STATUS:
        {
            Last_Status = (MP3_Player_Status_Typedef)param;
            Last_Query_Result = param;
        } break;

        case MP3_CMD_QUERY_VOLUME:
        {
            Last_Volume = (uint8_t)(param & 0x00FFU);
            Last_Query_Result = param;
        } break;

        case MP3_CMD_QUERY_EQ:
        {
            Last_EQ = (uint8_t)(param & 0x00FFU);
            Last_Query_Result = param;
        } break;

        case MP3_CMD_QUERY_U_DISK_FILES:
        case MP3_CMD_QUERY_TF_FILES:
        case MP3_CMD_QUERY_FLASH_FILES:
        case MP3_CMD_QUERY_U_DISK_TRACK:
        case MP3_CMD_QUERY_TF_TRACK:
        case MP3_CMD_QUERY_FLASH_TRACK:
        case MP3_CMD_QUERY_FOLDER_FILES:
        case MP3_CMD_QUERY_TOTAL_FOLDERS:
        {
            Last_Query_Result = param;
        } break;

        default:
        {
            // 未专门处理的返回，也保留命令和参数，方便调试观察
            Last_Query_Result = param;
        } break;
    }
}
