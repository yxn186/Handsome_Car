/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App_Light.cpp
  * @brief   App层巡检灯控
  ******************************************************************************
  */
/* USER CODE END Header */
#include "App_Light.h"
#include "App_Light_Config.h"

#include "FreeRTOS.h"
#include "task.h"
#include "Relay.h"

static Class_Relay Photo_Light_1;
static Class_Relay Photo_Light_2;
static bool Navigation_Capture_Active = false;
static bool Navigation_Command_Was_Fresh = false;
static bool Manual_Light_Enabled = false;
static bool Dial_Armed = true;
static bool Capture_Done = false;

static void App_Light_Set_All(bool Enable)
{
    Photo_Light_1.SetState(Enable);
    Photo_Light_2.SetState(Enable);
}

static void App_Light_Update_Manual(bool Remote_Online, float Dial_Wheel)
{
    if (!Remote_Online)
    {
        Manual_Light_Enabled = false;
        Dial_Armed = true;
        App_Light_Set_All(false);
        return;
    }

    if (Dial_Armed)
    {
        if (Dial_Wheel <= -App_Light_Dial_Trigger_Threshold)
        {
            Manual_Light_Enabled = true;
            Dial_Armed = false;
        }
        else if (Dial_Wheel >= App_Light_Dial_Trigger_Threshold)
        {
            Manual_Light_Enabled = false;
            Dial_Armed = false;
        }
    }
    else if ((Dial_Wheel > -App_Light_Dial_Release_Threshold) &&
             (Dial_Wheel < App_Light_Dial_Release_Threshold))
    {
        Dial_Armed = true;
    }

    App_Light_Set_All(Manual_Light_Enabled);
}

void App_Light_Init(void)
{
    Photo_Light_1.Init(PHOTO_LIGHT_1_GPIO_Port, PHOTO_LIGHT_1_Pin, GPIO_PIN_SET);
    Photo_Light_2.Init(PHOTO_LIGHT_2_GPIO_Port, PHOTO_LIGHT_2_Pin, GPIO_PIN_SET);

    Navigation_Capture_Active = false;
    Navigation_Command_Was_Fresh = false;
    Manual_Light_Enabled = false;
    Dial_Armed = true;
    Capture_Done = false;
    App_Light_Set_All(false);
}

void App_Light_Update(bool Navigation_Command_Fresh,
                      bool Capture_Enable,
                      bool Remote_Online,
                      float Dial_Wheel)
{
    if (Navigation_Command_Fresh && Capture_Enable)
    {
        Navigation_Capture_Active = true;
        Navigation_Command_Was_Fresh = true;
        App_Light_Set_All(true);
        Capture_Done = true;
        return;
    }

    if (Navigation_Capture_Active)
    {
        Navigation_Capture_Active = false;
        Manual_Light_Enabled = false;
        Dial_Armed = true;
        App_Light_Set_All(false);
    }

    if (!Navigation_Command_Fresh && Navigation_Command_Was_Fresh)
    {
        Manual_Light_Enabled = false;
        Dial_Armed = true;
        App_Light_Set_All(false);
    }

    Navigation_Command_Was_Fresh = Navigation_Command_Fresh;
    Capture_Done = false;
    App_Light_Update_Manual(Remote_Online, Dial_Wheel);
}

bool App_Light_Get_Capture_Done(void)
{
    bool Capture_Done_Snapshot = false;

    taskENTER_CRITICAL();
    Capture_Done_Snapshot = Capture_Done;
    taskEXIT_CRITICAL();

    return Capture_Done_Snapshot;
}
