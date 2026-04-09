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
#include "JPEG.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructButtonWidget    s_arrButton[GUI_BUTTON_MAX]; //触摸按键
static StructGUIButtonResult s_structButtonPress;         //按键扫描结果

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

  //获取按键按下结果
  for(i = (EnumGUIButtons)0; i < GUI_BUTTON_MAX; i++)
  {
    if((StructButtonWidget*)button == &s_arrButton[i])
    {
      s_structButtonPress.button[s_structButtonPress.num++] = i;
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
  static StructJpegImage s_structPressImage45x45;
  static StructJpegImage s_structReleaseImage45x45;
  static StructJpegImage s_structPressImage70x45;
  static StructJpegImage s_structReleaseImage70x45;
  static StructJpegImage s_structPressImage96x45;
  static StructJpegImage s_structReleaseImage96x45;
  static StructJpegImage s_structPressImage123x45;
  static StructJpegImage s_structReleaseImage123x45;
  static StructJpegImage s_structPressImage416x45;
  static StructJpegImage s_structReleaseImage416x45;

  //显示字符
  static const char* s_arrButtonText[GUI_BUTTON_MAX] = 
  {"ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "- -", "= +", "Back",   //第一行
   "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[ {", "] }", "\\ |",   //第二行
   "CapsLock", "A", "S", "D", "F", "G", "H", "J", "K", "L", "; :", "' \"", "Enter", //第三行
   "Shift", "Z", "X", "C", "V", "B", "N", "M", ", <", ". >", "/ ?", "Shift",        //第四行
   "Ctrl", "Win", "Alt", "", "Alt", "Ctrl"};                                        //第五行

  //控件宽度
  static const u16 s_arrButtonWidth[GUI_BUTTON_MAX] = 
  {
    45 , 45, 45, 45 , 45, 45, 45, 45, 45, 45, 45, 45 , 45, 96, //第一行
    70 , 45, 45, 45 , 45, 45, 45, 45, 45, 45, 45, 45 , 45, 70, //第二行
    96 , 45, 45, 45 , 45, 45, 45, 45, 45, 45, 45, 45 , 96,     //第三行
    123, 45, 45, 45 , 45, 45, 45, 45, 45, 45, 45, 123,         //第四行
    96 , 45, 45, 416, 45, 96};                                 //第五行
  
  //字符横坐标相对于控件的偏移量
  static const u8 s_arrStringX0[GUI_BUTTON_MAX] = 
  {
    4  , 16, 16, 16 , 16, 16, 16, 16, 16, 16, 16, 4 , 4 , 24, //第一行
    17 , 16, 16, 16 , 16, 16, 16, 16, 16, 16, 16, 4 , 4 , 17, //第二行
    0  , 16, 16, 16 , 16, 16, 16, 16, 16, 16, 4 , 4 , 18,     //第三行
    31 , 16, 16, 16 , 16, 16, 16, 16, 4 , 4 , 4 , 31,         //第四行
    24 , 4 , 4 , 0  , 4 , 24};                                //第五行

  EnumGUIButtons i;
  u32 x, y;

  s_structPressImage45x45.image = (unsigned char*)s_arrJpegkeyPressImage45x45x24;
  s_structPressImage45x45.size = sizeof(s_arrJpegkeyPressImage45x45x24) / sizeof(unsigned char);
  s_structReleaseImage45x45.image = (unsigned char*)s_arrJpegkeyReleaseImage45x45x24;
  s_structReleaseImage45x45.size = sizeof(s_arrJpegkeyReleaseImage45x45x24) / sizeof(unsigned char);

  s_structPressImage70x45.image = (unsigned char*)s_arrJpegkeyPressImage70x45x24;
  s_structPressImage70x45.size = sizeof(s_arrJpegkeyPressImage70x45x24) / sizeof(unsigned char);
  s_structReleaseImage70x45.image = (unsigned char*)s_arrJpegkeyReleaseImage70x45x24;
  s_structReleaseImage70x45.size = sizeof(s_arrJpegkeyReleaseImage70x45x24) / sizeof(unsigned char);

  s_structPressImage96x45.image = (unsigned char*)s_arrJpegkeyPressImage96x45x24;
  s_structPressImage96x45.size = sizeof(s_arrJpegkeyPressImage96x45x24) / sizeof(unsigned char);
  s_structReleaseImage96x45.image = (unsigned char*)s_arrJpegkeyReleaseImage96x45x24;
  s_structReleaseImage96x45.size = sizeof(s_arrJpegkeyReleaseImage96x45x24) / sizeof(unsigned char);

  s_structPressImage123x45.image = (unsigned char*)s_arrJpegkeyPressImage123x45x24;
  s_structPressImage123x45.size = sizeof(s_arrJpegkeyPressImage123x45x24) / sizeof(unsigned char);
  s_structReleaseImage123x45.image = (unsigned char*)s_arrJpegkeyReleaseImage123x45x24;
  s_structReleaseImage123x45.size = sizeof(s_arrJpegkeyReleaseImage123x45x24) / sizeof(unsigned char);

  s_structPressImage416x45.image = (unsigned char*)s_arrJpegkeyPressImage416x45x24;
  s_structPressImage416x45.size = sizeof(s_arrJpegkeyPressImage416x45x24) / sizeof(unsigned char);
  s_structReleaseImage416x45.image = (unsigned char*)s_arrJpegkeyReleaseImage416x45x24;
  s_structReleaseImage416x45.size = sizeof(s_arrJpegkeyReleaseImage416x45x24) / sizeof(unsigned char);

  //标记无按键按下
  s_structButtonPress.num = 0;
  for(i = (EnumGUIButtons)0; i < GUI_BUTTON_MAX; i++)
  {
    s_structButtonPress.button[i] = GUI_BUTTON_NONE;
  }

  //创建按键
  x = 6;
  y = 130;
  for(i = (EnumGUIButtons)0; i < GUI_BUTTON_MAX; i++)
  {
    InitButtonWidgetStruct(&s_arrButton[i]);
    s_arrButton[i].x0 = x;
    s_arrButton[i].y0 = y;
    s_arrButton[i].width = s_arrButtonWidth[i];
    s_arrButton[i].height = 45;
    s_arrButton[i].textX0 = s_arrStringX0[i];
    s_arrButton[i].textY0 = 10;
    //s_arrButton[i].textColor = GUI_COLOR_ORANGE;
    s_arrButton[i].text = s_arrButtonText[i];
    s_arrButton[i].type = BUTTON_WIDGET_TYPE_LEVEL;
    s_arrButton[i].pressCallback = ButtonPressCallback;

    //设置背景图片
    s_arrButton[i].pressImageType = GUI_IMAGE_TYPE_JPEG;
    s_arrButton[i].releaseImageType = GUI_IMAGE_TYPE_JPEG;
    if(45 == s_arrButton[i].width)
    {
      s_arrButton[i].pressBackgroudImage = (void*)(&s_structPressImage45x45);
      s_arrButton[i].releaseBackgroudImage = (void*)(&s_structReleaseImage45x45);
    }
    else if(70 == s_arrButton[i].width)
    {
      s_arrButton[i].pressBackgroudImage = (void*)(&s_structPressImage70x45);
      s_arrButton[i].releaseBackgroudImage = (void*)(&s_structReleaseImage70x45);
    }
    else if(96 == s_arrButton[i].width)
    {
      s_arrButton[i].pressBackgroudImage = (void*)(&s_structPressImage96x45);
      s_arrButton[i].releaseBackgroudImage = (void*)(&s_structReleaseImage96x45);
    }
    else if(123 == s_arrButton[i].width)
    {
      s_arrButton[i].pressBackgroudImage = (void*)(&s_structPressImage123x45);
      s_arrButton[i].releaseBackgroudImage = (void*)(&s_structReleaseImage123x45);
    }
    else if(416 == s_arrButton[i].width)
    {
      s_arrButton[i].pressBackgroudImage = (void*)(&s_structPressImage416x45);
      s_arrButton[i].releaseBackgroudImage = (void*)(&s_structReleaseImage416x45);
    }

    //创建控件
    CreateButtonWidget(&s_arrButton[i]);

    //横坐标递增
    x = x + s_arrButton[i].width + 8;

    //切换到下一行
    if((GUI_BUTTON_BACK == i) || (GUI_BUTTON_SLASH == i) || (GUI_BUTTON_ENTER == i) || (GUI_BUTTON_RSHIFT == i) || (GUI_BUTTON_RCTRL == i))
    {
      x = 6;
      y = y + s_arrButton[0].height + 10;
    }
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
StructGUIButtonResult* ScanGUIButton(void)
{
  EnumGUIButtons i;

  //标记无按键按下
  s_structButtonPress.num = 0;
  for(i = (EnumGUIButtons)0; i < GUI_BUTTON_MAX; i++)
  {
    s_structButtonPress.button[i] = GUI_BUTTON_NONE;
  }

  //依次扫描所有按键，并将按键扫描结果放在s_structButtonPress中
  for(i = (EnumGUIButtons)0; i < GUI_BUTTON_MAX; i++)
  {
    ScanButtonWidget(&s_arrButton[i]);
  }

  return &s_structButtonPress;
}
