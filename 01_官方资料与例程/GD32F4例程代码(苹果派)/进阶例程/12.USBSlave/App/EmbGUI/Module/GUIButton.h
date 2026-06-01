/*********************************************************************************************************
* 模块名称：GUIButton.h
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
#ifndef _GUI_BUTTON_H_
#define _GUI_BUTTON_H_

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "DataType.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/
//按键枚举
typedef enum
{
  GUI_BUTTON_ESC = 0,   // ESC，必须是从0开始
  GUI_BUTTON_K1,        // 1
  GUI_BUTTON_K2,        // 2
  GUI_BUTTON_K3,        // 3
  GUI_BUTTON_K4,        // 4
  GUI_BUTTON_K5,        // 5
  GUI_BUTTON_K6,        // 6
  GUI_BUTTON_K7,        // 7
  GUI_BUTTON_K8,        // 8
  GUI_BUTTON_K9,        // 9
  GUI_BUTTON_K0,        // 0
  GUI_BUTTON_MINUS,     // - _
  GUI_BUTTON_EQUAL,     // = +
  GUI_BUTTON_BACK,      // Backspace
  GUI_BUTTON_TAB,       // Tab
  GUI_BUTTON_Q,         // Q
  GUI_BUTTON_W,         // W
  GUI_BUTTON_E,         // E
  GUI_BUTTON_R,         // R
  GUI_BUTTON_T,         // T
  GUI_BUTTON_Y,         // Y
  GUI_BUTTON_U,         // U
  GUI_BUTTON_I,         // I
  GUI_BUTTON_O,         // O
  GUI_BUTTON_P,         // P
  GUI_BUTTON_LPARAN,    // [ {
  GUI_BUTTON_RPARAN,    // ] }
  GUI_BUTTON_SLASH,     // \ |
  GUI_BUTTON_CAPSLOCK,  // CapLock
  GUI_BUTTON_A,         // A
  GUI_BUTTON_S,         // S
  GUI_BUTTON_D,         // D
  GUI_BUTTON_F,         // F
  GUI_BUTTON_G,         // G
  GUI_BUTTON_H,         // H
  GUI_BUTTON_J,         // J
  GUI_BUTTON_K,         // K
  GUI_BUTTON_L,         // L
  GUI_BUTTON_SEMICOLON, // ; :semicolon
  GUI_BUTTON_QUOTATION, // ' "
  GUI_BUTTON_ENTER,     // Enter
  GUI_BUTTON_LSHIFT,    // left Shift
  GUI_BUTTON_Z,         // Z
  GUI_BUTTON_X,         // X
  GUI_BUTTON_C,         // C
  GUI_BUTTON_V,         // V
  GUI_BUTTON_B,         // B
  GUI_BUTTON_N,         // N
  GUI_BUTTON_M,         // M
  GUI_BUTTON_COMMA,     // , <
  GUI_BUTTON_PERIOD,    // . >
  GUI_BUTTON_QUESTION,  // / ?
  GUI_BUTTON_RSHIFT,    // right Shift
  GUI_BUTTON_LCTRL,     // left Ctrl
  GUI_BUTTON_WIN,       // Windows
  GUI_BUTTON_LALT,      // left Alt
  GUI_BUTTON_SPASEBAR,  // Spacebar
  GUI_BUTTON_RALT,      // Right Alt
  GUI_BUTTON_RCTRL,     // right Ctrl
  GUI_BUTTON_MAX,       // 按键数量
  GUI_BUTTON_NONE,      // 无效按键
}EnumGUIButtons;

//按键扫描结果
typedef struct
{
  u32 num;                               //按键按下数量计数
  EnumGUIButtons button[GUI_BUTTON_MAX]; //按键扫描结果储存的是第一个、第二个、第n个按键按下的键值
}StructGUIButtonResult;

/*********************************************************************************************************
*                                              API函数声明
*********************************************************************************************************/
void InitGUIButton(void);                    //初始化GUI按键模块
StructGUIButtonResult* ScanGUIButton(void);  //按键扫描

#endif
