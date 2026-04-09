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
  static StructBmpImage s_structKeyPreviousPressImage;
  static StructBmpImage s_structKeyPreviousReleaseImage;
  static StructBmpImage s_structKeyNextPressImage;
  static StructBmpImage s_structKeyNextReleaseImage;

  s_structKeyPreviousPressImage.alphaType   = BMP_GATE_ALPHA;
  s_structKeyPreviousPressImage.alphaGate   = 200;
  s_structKeyPreviousPressImage.storageType = BMP_IN_FLASH;
  s_structKeyPreviousPressImage.addr        = (void*)s_arrBmpBookPreviousPageButtonPressImage130x45x32;

  s_structKeyPreviousReleaseImage.alphaType   = BMP_GATE_ALPHA;
  s_structKeyPreviousReleaseImage.alphaGate   = 200;
  s_structKeyPreviousReleaseImage.storageType = BMP_IN_FLASH;
  s_structKeyPreviousReleaseImage.addr        = (void*)s_arrBmpBookPreviousPageButtonReleaseImage130x45x32;

  s_structKeyNextPressImage.alphaType   = BMP_GATE_ALPHA;
  s_structKeyNextPressImage.alphaGate   = 200;
  s_structKeyNextPressImage.storageType = BMP_IN_FLASH;
  s_structKeyNextPressImage.addr        = (void*)s_arrBmpBookNextPageButtonPressImage130x45x32;

  s_structKeyNextReleaseImage.alphaType   = BMP_GATE_ALPHA;
  s_structKeyNextReleaseImage.alphaGate   = 200;
  s_structKeyNextReleaseImage.storageType = BMP_IN_FLASH;
  s_structKeyNextReleaseImage.addr        = (void*)s_arrBmpBookNextPageButtonReleaseImage130x45x32;

  //标记无按键按下
  s_enumButtonPress = GUI_BUTTON_NONE;

  //创建前一页按钮
  InitButtonWidgetStruct(&s_arrButton[GUI_BUTTON_PREVIOUS]);
  s_arrButton[GUI_BUTTON_PREVIOUS].x0 = 10;
  s_arrButton[GUI_BUTTON_PREVIOUS].y0 = 740;
  s_arrButton[GUI_BUTTON_PREVIOUS].width = 130;
  s_arrButton[GUI_BUTTON_PREVIOUS].height = 45;
  s_arrButton[GUI_BUTTON_PREVIOUS].textX0 = 35;
  s_arrButton[GUI_BUTTON_PREVIOUS].textY0 = 10;
  s_arrButton[GUI_BUTTON_PREVIOUS].text = "";
  s_arrButton[GUI_BUTTON_PREVIOUS].pressBackgroudImage = (void*)(&s_structKeyPreviousPressImage);
  s_arrButton[GUI_BUTTON_PREVIOUS].releaseBackgroudImage = (void*)(&s_structKeyPreviousReleaseImage);
  s_arrButton[GUI_BUTTON_PREVIOUS].pressCallback = ButtonPressCallback;
  CreateButtonWidget(&s_arrButton[GUI_BUTTON_PREVIOUS]);

  //创建后一页按钮
  InitButtonWidgetStruct(&s_arrButton[GUI_BUTTON_NEXT]);
  s_arrButton[GUI_BUTTON_NEXT].x0 = 340;
  s_arrButton[GUI_BUTTON_NEXT].y0 = 740;
  s_arrButton[GUI_BUTTON_NEXT].width = 130;
  s_arrButton[GUI_BUTTON_NEXT].height = 45;
  s_arrButton[GUI_BUTTON_NEXT].textX0 = 35;
  s_arrButton[GUI_BUTTON_NEXT].textY0 = 10;
  s_arrButton[GUI_BUTTON_NEXT].text = "";
  s_arrButton[GUI_BUTTON_NEXT].pressBackgroudImage = (void*)(&s_structKeyNextPressImage);
  s_arrButton[GUI_BUTTON_NEXT].releaseBackgroudImage = (void*)(&s_structKeyNextReleaseImage);
  s_arrButton[GUI_BUTTON_NEXT].pressCallback = ButtonPressCallback;
  CreateButtonWidget(&s_arrButton[GUI_BUTTON_NEXT]);
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

