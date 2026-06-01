/*********************************************************************************************************
* 模块名称：KeyboardTop.c
* 摘    要：USB键盘驱动模块
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
#include "KeyboardTop.h"
#include "Keyboard.h"
#include "GUITop.h"
#include "stdio.h"
#include "SysTick.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructGUIDev s_structGUIDev; //GUI设备结构体

//键值转换，GUI传回来的键值与HID键值不一致，需要做转换
//GUI键值编号是从左往右，从上至下，从0开始编号
static u8 s_arrKeyTable[] = 
{
  //第一行
  KEYBOARD_ESC, KEYBOARD_1, KEYBOARD_2, KEYBOARD_3, KEYBOARD_4, KEYBOARD_5, KEYBOARD_6, KEYBOARD_7,
  KEYBOARD_8, KEYBOARD_9, KEYBOARD_0, KEYBOARD_MINUS, KEYBOARD_EQUAL, KEYBOARD_BACKSPACE,

  //第二行
  KEYBOARD_TAB, KEYBOARD_Q, KEYBOARD_W, KEYBOARD_E, KEYBOARD_R, KEYBOARD_T, KEYBOARD_Y, KEYBOARD_U, 
  KEYBOARD_I, KEYBOARD_O, KEYBOARD_P, KEYBOARD_LPARAN, KEYBOARD_RPARAN, KEYBOARD_SLASH,

  //第三行
  KEYBOARD_CAPSLOCK, KEYBOARD_A, KEYBOARD_S, KEYBOARD_D, KEYBOARD_F, KEYBOARD_G, KEYBOARD_H, KEYBOARD_J,
  KEYBOARD_K, KEYBOARD_L, KEYBOARD_SEMICOLON, KEYBOARD_QUOTATION, KEYBOARD_ENTER,

  //第四行
  KEYBOARD_LEFT_SHIFT, KEYBOARD_Z, KEYBOARD_X, KEYBOARD_C, KEYBOARD_V, KEYBOARD_B, KEYBOARD_N, KEYBOARD_M,
  KEYBOARD_COMMA, KEYBOARD_PERIOD, KEYBOARD_QUESTION, KEYBOARD_RIGHT_SHIFT,

  //第五行
  KEYBOARD_LEFT_CTRL, KEYBOARD_LEFT_WIN, KEYBOARD_LEFT_ALT, KEYBOARD_SPACEBAR, KEYBOARD_RIGHT_ALT, 
  KEYBOARD_RIGHT_CTRL,
};

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void KeyCallback(StructGUIButtonResult* result); //按键回调函数

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：KeyCallback
* 函数功能：按键回调函数
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*          1、按下控制键（shift、ctrl和win）后记得发送空按键指令给电脑以取消控制键状态，否则电脑会误以为控制键一直
*             处于按下状态，进而严重干扰电脑正常键盘输入
*          2、GUI按键扫描检测到没有按键按下时也会调用一次此回调函数，用以方便用户清除控制键状态
*********************************************************************************************************/
static void KeyCallback(StructGUIButtonResult* result)
{
  u8 i, funcKey, key[6];

  //初始化键值
  funcKey = 0;
  for(i = 0; i < 6; i++)
  {
    key[i] = 0;
  }

  //获取键值填入临时缓冲区
  for(i = 0; i < result->num; i++)
  {
    //左Ctrl键
    if(GUI_BUTTON_LCTRL == result->button[i])
    {
      funcKey = funcKey | SET_LEFT_CTRL;
    }

    //右Ctrl键
    else if(GUI_BUTTON_RCTRL == result->button[i])
    {
      funcKey = funcKey | SET_RIGHT_CTRL;
    }

    //左Shift键
    else if(GUI_BUTTON_LSHIFT == result->button[i])
    {
      funcKey = funcKey | SET_LEFT_SHIFT;
    }

    //右Shift键
    else if(GUI_BUTTON_RSHIFT == result->button[i])
    {
      funcKey = funcKey | SET_RIGHT_SHIFT;
    }

    //左Alt键
    else if(GUI_BUTTON_LALT == result->button[i])
    {
      funcKey = funcKey | SET_LEFT_ALT;
    }

    //右Alt键
    else if(GUI_BUTTON_RALT == result->button[i])
    {
      funcKey = funcKey | SET_RIGHT_ALT;
    }

    //Windows键
    else if(GUI_BUTTON_WIN == result->button[i])
    {
      funcKey = funcKey | SET_LEFT_WINDOWS;
    }

    //键值转换并保存到键值缓冲区
    if(GUI_BUTTON_NONE != result->button[i])
    {
      key[i] = s_arrKeyTable[result->button[i]];
    }

    //只获取前6个键值
    if(i >= 5)
    {
      break;
    }
  }
  
  //上报键值
  while(0 != SendKeyVal(funcKey, key[0], key[1], key[2], key[3], key[4], key[5]));
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitKeyboardTop
* 函数功能：初始化USB键盘顶层模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitKeyboardTop(void)
{
  //初始化USB键盘驱动
  InitKeyboard();
  
  //设置按键扫描频率
  s_structGUIDev.scanTime = 100;

  //设置回调函数
  s_structGUIDev.scanCallback = KeyCallback;

  //初始化UI界面设计
  InitGUI(&s_structGUIDev);
}

/*********************************************************************************************************
* 函数名称：KeyboardTopTask
* 函数功能：USB键盘顶层模块任务
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：执行的时间越快，按键扫描频率越准确
*********************************************************************************************************/
void KeyboardTopTask(void)
{
  GUITask(); //GUI任务
}
