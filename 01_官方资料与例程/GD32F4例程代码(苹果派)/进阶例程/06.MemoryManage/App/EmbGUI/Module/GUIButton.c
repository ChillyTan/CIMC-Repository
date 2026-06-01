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
  static StructBmpImage s_structKeyPressImage;
  static StructBmpImage s_structKeyReleaseImage;

  s_structKeyPressImage.alphaType   = BMP_GATE_ALPHA;
  s_structKeyPressImage.alphaGate   = 200;
  s_structKeyPressImage.storageType = BMP_IN_FLASH;
  s_structKeyPressImage.addr        = (void*)s_arrBmpButtonPressImage130x50x32;

  s_structKeyReleaseImage.alphaType   = BMP_GATE_ALPHA;
  s_structKeyReleaseImage.alphaGate   = 200;
  s_structKeyReleaseImage.storageType = BMP_IN_FLASH;
  s_structKeyReleaseImage.addr        = (void*)s_arrBmpButtonRleaseImage130x50x32;

  //标记无按键按下
  s_enumButtonPress = GUI_BUTTON_NONE;

  //创建申请INSRAM内存按钮
  InitButtonWidgetStruct(&s_arrButton[GUI_BUTTON_MALLOC_INSRAM]);
  s_arrButton[GUI_BUTTON_MALLOC_INSRAM].x0 = 100;
  s_arrButton[GUI_BUTTON_MALLOC_INSRAM].y0 = 400;
  s_arrButton[GUI_BUTTON_MALLOC_INSRAM].width = 130;
  s_arrButton[GUI_BUTTON_MALLOC_INSRAM].height = 50;
  s_arrButton[GUI_BUTTON_MALLOC_INSRAM].textX0 = 29;
  s_arrButton[GUI_BUTTON_MALLOC_INSRAM].textY0 = 13;
  s_arrButton[GUI_BUTTON_MALLOC_INSRAM].text = "InSRAM";
  s_arrButton[GUI_BUTTON_MALLOC_INSRAM].pressBackgroudImage = (void*)(&s_structKeyPressImage);
  s_arrButton[GUI_BUTTON_MALLOC_INSRAM].releaseBackgroudImage = (void*)(&s_structKeyReleaseImage);
  s_arrButton[GUI_BUTTON_MALLOC_INSRAM].pressCallback = ButtonPressCallback;
  CreateButtonWidget(&s_arrButton[GUI_BUTTON_MALLOC_INSRAM]);

  //创建申请EXSRAM内存按钮
  InitButtonWidgetStruct(&s_arrButton[GUI_BUTTON_MALLOC_EXSRAM]);
  s_arrButton[GUI_BUTTON_MALLOC_EXSRAM].x0 = 335;
  s_arrButton[GUI_BUTTON_MALLOC_EXSRAM].y0 = s_arrButton[0].y0;
  s_arrButton[GUI_BUTTON_MALLOC_EXSRAM].width = 130;
  s_arrButton[GUI_BUTTON_MALLOC_EXSRAM].height = 50;
  s_arrButton[GUI_BUTTON_MALLOC_EXSRAM].textX0 = 25;
  s_arrButton[GUI_BUTTON_MALLOC_EXSRAM].textY0 = 13;
  s_arrButton[GUI_BUTTON_MALLOC_EXSRAM].text = "ExSDRAM";
  s_arrButton[GUI_BUTTON_MALLOC_EXSRAM].pressBackgroudImage = (void*)(&s_structKeyPressImage);
  s_arrButton[GUI_BUTTON_MALLOC_EXSRAM].releaseBackgroudImage = (void*)(&s_structKeyReleaseImage);
  s_arrButton[GUI_BUTTON_MALLOC_EXSRAM].pressCallback = ButtonPressCallback;
  CreateButtonWidget(&s_arrButton[GUI_BUTTON_MALLOC_EXSRAM]);

  //创建释放内存按钮
  InitButtonWidgetStruct(&s_arrButton[GUI_BUTTON_FREE]);
  s_arrButton[GUI_BUTTON_FREE].x0 = 570;
  s_arrButton[GUI_BUTTON_FREE].y0 = s_arrButton[0].y0;
  s_arrButton[GUI_BUTTON_FREE].width = 130;
  s_arrButton[GUI_BUTTON_FREE].height = 50;
  s_arrButton[GUI_BUTTON_FREE].textX0 = 41;
  s_arrButton[GUI_BUTTON_FREE].textY0 = 13;
  s_arrButton[GUI_BUTTON_FREE].text = "Free";
  s_arrButton[GUI_BUTTON_FREE].pressBackgroudImage = (void*)(&s_structKeyPressImage);
  s_arrButton[GUI_BUTTON_FREE].releaseBackgroudImage = (void*)(&s_structKeyReleaseImage);
  s_arrButton[GUI_BUTTON_FREE].pressCallback = ButtonPressCallback;
  CreateButtonWidget(&s_arrButton[GUI_BUTTON_FREE]);
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

