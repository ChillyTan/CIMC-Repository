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
*                                              枚举结构体定义
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
  static const u16 s_arrButtonX[GUI_BUTTON_MAX] = {485, 560, 635, 710, //K0-K3
                                                   485, 560, 635, 710, //K4-K7
                                                   485, 560, 635, 710, //K8-KB
                                                   485, 560, 635, 710, //KC-KF
                                                   25,  175, 325};     //Read, Write, Clear

  static const u16 s_arrButtonY[GUI_BUTTON_MAX] = {180, 180, 180, 180, //K0-K3
                                                   255, 255, 255, 255, //K4-K7
                                                   330, 330, 330, 330, //K8-KB
                                                   405, 405, 405, 405, //KC-KF
                                                   405, 405, 405};     //Read, Write, Clear
  static const char* s_arrButtonText[16] = {"0","1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", };
  static StructBmpImage s_structKeyNumPressImage;
  static StructBmpImage s_structKeyNumReleaseImage;
  static StructBmpImage s_structKeyFucPressImage;
  static StructBmpImage s_structKeyFucReleaseImage;

  EnumGUIButtons i;

  s_structKeyNumPressImage.alphaType   = BMP_GATE_ALPHA;
  s_structKeyNumPressImage.alphaGate   = 1;
  s_structKeyNumPressImage.storageType = BMP_IN_FLASH;
  s_structKeyNumPressImage.addr        = (void*)s_arrBmpButton0PressImage65x65x32;

  s_structKeyNumReleaseImage.alphaType   = BMP_GATE_ALPHA;
  s_structKeyNumReleaseImage.alphaGate   = 1;
  s_structKeyNumReleaseImage.storageType = BMP_IN_FLASH;
  s_structKeyNumReleaseImage.addr        = (void*)s_arrBmpButton0ReleaseImage65x65x32;

  s_structKeyFucPressImage.alphaType   = BMP_GATE_ALPHA;
  s_structKeyFucPressImage.alphaGate   = 1;
  s_structKeyFucPressImage.storageType = BMP_IN_FLASH;
  s_structKeyFucPressImage.addr        = (void*)s_arrBmpButton1PressImage130x65x32;

  s_structKeyFucReleaseImage.alphaType   = BMP_GATE_ALPHA;
  s_structKeyFucReleaseImage.alphaGate   = 1;
  s_structKeyFucReleaseImage.storageType = BMP_IN_FLASH;
  s_structKeyFucReleaseImage.addr        = (void*)s_arrBmpButton1ReleaseImage130x65x32;

  //标记无按键按下
  s_enumButtonPress = GUI_BUTTON_NONE;

  //创建K0-KF
  for(i = GUI_BUTTON_K0; i <= GUI_BUTTON_KF; i++)
  {
    InitButtonWidgetStruct(&s_arrButton[i]);
    s_arrButton[i].x0 = s_arrButtonX[i];
    s_arrButton[i].y0 = s_arrButtonY[i];
    s_arrButton[i].width = 65;
    s_arrButton[i].height = 65;
    s_arrButton[i].textX0 = 25;
    s_arrButton[i].textY0 = 20;
    s_arrButton[i].text = s_arrButtonText[i];
    s_arrButton[i].pressBackgroudImage = (void*)(&s_structKeyNumPressImage);
    s_arrButton[i].releaseBackgroudImage = (void*)(&s_structKeyNumReleaseImage);
    s_arrButton[i].pressCallback = ButtonPressCallback;
    CreateButtonWidget(&s_arrButton[i]);
  }

  //创建读取按钮
  InitButtonWidgetStruct(&s_arrButton[GUI_BUTTON_READ]);
  s_arrButton[GUI_BUTTON_READ].x0 = s_arrButtonX[GUI_BUTTON_READ];
  s_arrButton[GUI_BUTTON_READ].y0 = s_arrButtonY[GUI_BUTTON_READ];
  s_arrButton[GUI_BUTTON_READ].width = 130;
  s_arrButton[GUI_BUTTON_READ].height = 65;
  s_arrButton[GUI_BUTTON_READ].textX0 = 35;
  s_arrButton[GUI_BUTTON_READ].textY0 = 20;
  s_arrButton[GUI_BUTTON_READ].text = "READ";
  s_arrButton[GUI_BUTTON_READ].pressBackgroudImage = (void*)(&s_structKeyFucPressImage);
  s_arrButton[GUI_BUTTON_READ].releaseBackgroudImage = (void*)(&s_structKeyFucReleaseImage);
  s_arrButton[GUI_BUTTON_READ].pressCallback = ButtonPressCallback;
  CreateButtonWidget(&s_arrButton[GUI_BUTTON_READ]);

  //创建写入按钮
  InitButtonWidgetStruct(&s_arrButton[GUI_BUTTON_WRITE]);
  s_arrButton[GUI_BUTTON_WRITE].x0 = s_arrButtonX[GUI_BUTTON_WRITE];
  s_arrButton[GUI_BUTTON_WRITE].y0 = s_arrButtonY[GUI_BUTTON_WRITE];
  s_arrButton[GUI_BUTTON_WRITE].width = 130;
  s_arrButton[GUI_BUTTON_WRITE].height = 65;
  s_arrButton[GUI_BUTTON_WRITE].textX0 = 35;
  s_arrButton[GUI_BUTTON_WRITE].textY0 = 20;
  s_arrButton[GUI_BUTTON_WRITE].text = "WRITE";
  s_arrButton[GUI_BUTTON_WRITE].pressBackgroudImage = (void*)(&s_structKeyFucPressImage);
  s_arrButton[GUI_BUTTON_WRITE].releaseBackgroudImage = (void*)(&s_structKeyFucReleaseImage);
  s_arrButton[GUI_BUTTON_WRITE].pressCallback = ButtonPressCallback;
  CreateButtonWidget(&s_arrButton[GUI_BUTTON_WRITE]);

  //创建撤回按钮
  InitButtonWidgetStruct(&s_arrButton[GUI_BUTTON_BACK]);
  s_arrButton[GUI_BUTTON_BACK].x0 = s_arrButtonX[GUI_BUTTON_BACK];
  s_arrButton[GUI_BUTTON_BACK].y0 = s_arrButtonY[GUI_BUTTON_BACK];
  s_arrButton[GUI_BUTTON_BACK].width = 130;
  s_arrButton[GUI_BUTTON_BACK].height = 65;
  s_arrButton[GUI_BUTTON_BACK].textX0 = 35;
  s_arrButton[GUI_BUTTON_BACK].textY0 = 20;
  s_arrButton[GUI_BUTTON_BACK].text = "BACK";
  s_arrButton[GUI_BUTTON_BACK].pressBackgroudImage = (void*)(&s_structKeyFucPressImage);
  s_arrButton[GUI_BUTTON_BACK].releaseBackgroudImage = (void*)(&s_structKeyFucReleaseImage);
  s_arrButton[GUI_BUTTON_BACK].pressCallback = ButtonPressCallback;
  CreateButtonWidget(&s_arrButton[GUI_BUTTON_BACK]);
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

