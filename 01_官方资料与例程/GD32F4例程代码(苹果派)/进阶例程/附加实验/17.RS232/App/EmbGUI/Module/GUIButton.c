/*********************************************************************************************************
* 模块名称：GUIButton.c
* 摘    要：GUI按键模块
* 当前版本：1.0.0
* 作    者：Leyutek(COPYRIGHT 2018 - 2021 Leyutek. All rights reserved.)
* 完成日期：2021年07月01日
* 内    容：
* 注    意：                                                                  
**********************************************************************************************************
* 取代版本：
* 作    者：
* 完成日期：
* 修改内容：
* 修改文件：
*********************************************************************************************************/

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "GUIButton.h"
#include "TLILCD.h"
#include "ButtonWidget.h"
#include "BMP.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructButtonWidget s_arrButton[GUI_BUTTON_MAX]; //触摸按键
static EnumGUIButtons     s_enumButtonPress;           //用来标记那个按键按下

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ButtonPressCallback(void* button);  //按键回调函数

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ButtonPressCallback
* 函数功能：按键回调函数
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ButtonPressCallback(void* button)
{
  EnumGUIButtons i;

  for(i = (EnumGUIButtons)0; i < GUI_BUTTON_MAX; i++)
  {
    if((StructButtonWidget*)button == &s_arrButton[i])
    {
      s_enumButtonPress = i;
      return;
    }
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitGUIButton
* 函数功能：初始化GUI按键模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitGUIButton(void)
{
  static const u16 s_arrButtonX[GUI_BUTTON_MAX] = {560, 640, 720,  //K7-K9
                                                   560, 640, 720,  //K4-K5
                                                   560, 640, 720,  //K1-K3
                                                   560, 640, 720}; //SEND-BACK

  static const u16 s_arrButtonY[GUI_BUTTON_MAX] = {220, 220, 220,  //K7-K9
                                                   285, 285, 285,  //K4-K5
                                                   350, 350, 350,  //K1-K3
                                                   415, 415, 415}; //SEND-BACK

  static const char* s_arrButtonText[GUI_BUTTON_MAX] = {"7","8", "9", "4", "5", "6", "1", "2", "3", "Send", "0", "Back"};
  
  static StructBmpImage s_structKeyPressImage;
  static StructBmpImage s_structKeyReleaseImage;

  u32 i;

  s_structKeyPressImage.alphaType   = BMP_GATE_ALPHA;
  s_structKeyPressImage.alphaGate   = 200;
  s_structKeyPressImage.storageType = BMP_IN_FLASH;
  s_structKeyPressImage.addr        = (void*)s_arrBmpButton0PressImage60x45x32;

  s_structKeyReleaseImage.alphaType   = BMP_GATE_ALPHA;
  s_structKeyReleaseImage.alphaGate   = 200;
  s_structKeyReleaseImage.storageType = BMP_IN_FLASH;
  s_structKeyReleaseImage.addr        = (void*)s_arrBmpButton0ReleaseImage60x45x32;

  //标记无按键按下
  s_enumButtonPress = GUI_BUTTON_NONE;

  //创建K0-KF
  for(i = 0; i < GUI_BUTTON_MAX; i++)
  {
    InitButtonWidgetStruct(&s_arrButton[i]);
    s_arrButton[i].x0 = s_arrButtonX[i];
    s_arrButton[i].y0 = s_arrButtonY[i];
    s_arrButton[i].width = 60;
    s_arrButton[i].height = 45;

    if((GUI_BUTTON_SEND == i) || (GUI_BUTTON_BACK == i))
    {
      s_arrButton[i].textX0 = 6;
    }
    else
    {
      s_arrButton[i].textX0 = 23;
    }
    
    s_arrButton[i].textY0 = 10;
    s_arrButton[i].text = s_arrButtonText[i];
    s_arrButton[i].pressBackgroudImage = (void*)(&s_structKeyPressImage);
    s_arrButton[i].releaseBackgroudImage = (void*)(&s_structKeyReleaseImage);
    s_arrButton[i].pressCallback = ButtonPressCallback;
    CreateButtonWidget(&s_arrButton[i]);
  }
}

/*********************************************************************************************************
* 函数名称：ScanGUIButton
* 函数功能：按键扫描
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：每隔20毫秒执行一次
*********************************************************************************************************/
EnumGUIButtons ScanGUIButton(void)
{
  EnumGUIButtons i;

  for(i = (EnumGUIButtons)0; i < GUI_BUTTON_MAX; i++)
  {
    s_enumButtonPress = GUI_BUTTON_NONE;
    ScanButtonWidget(&s_arrButton[i]);

    if(GUI_BUTTON_NONE != s_enumButtonPress)
    {
      return s_enumButtonPress;
    }
  }

  return GUI_BUTTON_NONE;
}

