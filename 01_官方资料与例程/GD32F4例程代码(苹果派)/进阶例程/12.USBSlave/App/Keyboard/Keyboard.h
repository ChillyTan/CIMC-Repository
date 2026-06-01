/*********************************************************************************************************
* 模块名称：KEYBOARD.h
* 摘    要：USB键盘驱动模块
* 当前版本：1.0.0
* 作    者：Leyutek(COPYRIGHT 2018 - 2021 Leyutek. All rights reserved.)
* 完成日期：2021年07月01日
* 内    容：
* 注    意：记得勾选“C99”模式，否则编译会报错
**********************************************************************************************************
* 取代版本：
* 作    者：
* 完成日期：
* 修改内容：
* 修改文件：
*********************************************************************************************************/
#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "DataType.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
//键盘HID码表
#define KEYBOARD_NULL             0   // no event indicated
#define KEYBOARD_ERROR_ROLL_OVER  1   // Error Roll Over
#define KEYBOARD_POST_Fail        2   // POST Fail
#define KEYBOARD_Error_Undefined  3   // Error Undefined
#define KEYBOARD_A                4   // KEYBOARD a and A
#define KEYBOARD_B                5   // KEYBOARD b and B
#define KEYBOARD_C                6   // KEYBOARD c and C
#define KEYBOARD_D                7   // KEYBOARD d and D
#define KEYBOARD_E                8   // KEYBOARD e and E
#define KEYBOARD_F                9   // KEYBOARD f and F
#define KEYBOARD_G                10  // KEYBOARD g and G
#define KEYBOARD_H                11  // KEYBOARD h and H
#define KEYBOARD_I                12  // KEYBOARD i and I
#define KEYBOARD_J                13  // KEYBOARD j and J
#define KEYBOARD_K                14  // KEYBOARD k and K
#define KEYBOARD_L                15  // KEYBOARD l and L
#define KEYBOARD_M                16  // KEYBOARD m and M
#define KEYBOARD_N                17  // KEYBOARD n and N
#define KEYBOARD_O                18  // KEYBOARD o and O
#define KEYBOARD_P                19  // KEYBOARD p and P
#define KEYBOARD_Q                20  // KEYBOARD q and Q
#define KEYBOARD_R                21  // KEYBOARD r and R
#define KEYBOARD_S                22  // KEYBOARD s and S
#define KEYBOARD_T                23  // KEYBOARD t and T
#define KEYBOARD_U                24  // KEYBOARD u and U
#define KEYBOARD_V                25  // KEYBOARD v and V
#define KEYBOARD_W                26  // KEYBOARD w and W
#define KEYBOARD_X                27  // KEYBOARD x and X
#define KEYBOARD_Y                28  // KEYBOARD y and Y
#define KEYBOARD_Z                29  // KEYBOARD z and Z
#define KEYBOARD_1                30  // KEYBOARD 1 and !
#define KEYBOARD_2                31  // KEYBOARD 2 and @
#define KEYBOARD_3                32  // KEYBOARD 3 and #
#define KEYBOARD_4                33  // KEYBOARD 4 and $
#define KEYBOARD_5                34  // KEYBOARD 5 and %
#define KEYBOARD_6                35  // KEYBOARD 6 and ^
#define KEYBOARD_7                36  // KEYBOARD 7 and &
#define KEYBOARD_8                37  // KEYBOARD 8 and *
#define KEYBOARD_9                38  // KEYBOARD 9 and (
#define KEYBOARD_0                39  // KEYBOARD 0 and )
#define KEYBOARD_ENTER            40  // KEYBOARD ENTER
#define KEYBOARD_ESC              41  // KEYBOARD ESCAPE
#define KEYBOARD_BACKSPACE        42  // KEYBOARD Backspace
#define KEYBOARD_TAB              43  // KEYBOARD Tab
#define KEYBOARD_SPACEBAR         44  // KEYBOARD Spacebar
#define KEYBOARD_MINUS            45  // KEYBOARD - and _(underscore)
#define KEYBOARD_EQUAL            46  // KEYBOARD = and +
#define KEYBOARD_LPARAN           47  // KEYBOARD [ and {
#define KEYBOARD_RPARAN           48  // KEYBOARD ] and }
#define KEYBOARD_SLASH            49  // KEYBOARD \ and |
#define KEYBOARD_SEMICOLON        51  // KEYBOARD ; and :
#define KEYBOARD_QUOTATION        52  // KEYBOARD ‘ and “
#define KEYBOARD_TILDE            53  // KEYBOARD `(Grave Accent) and ~(Tilde)
#define KEYBOARD_COMMA            54  // KEYBOARD, and <
#define KEYBOARD_PERIOD           55  // KEYBOARD . and >
#define KEYBOARD_QUESTION         56  // KEYBOARD / and ?
#define KEYBOARD_CAPSLOCK         57  // KEYBOARD Caps Lock
#define KEYBOARD_F1               58  // KEYBOARD F1
#define KEYBOARD_F2               59  // KEYBOARD F2
#define KEYBOARD_F3               60  // KEYBOARD F3
#define KEYBOARD_F4               61  // KEYBOARD F4
#define KEYBOARD_F5               62  // KEYBOARD F5
#define KEYBOARD_F6               63  // KEYBOARD F6
#define KEYBOARD_F7               64  // KEYBOARD F7
#define KEYBOARD_F8               65  // KEYBOARD F8
#define KEYBOARD_F9               66  // KEYBOARD F9
#define KEYBOARD_F10              67  // KEYBOARD F10
#define KEYBOARD_F11              68  // KEYBOARD F11
#define KEYBOARD_F12              69  // KEYBOARD F12
#define KEYBOARD_PRINTSCREEN      70  // KEYBOARD PrintScreen
#define KEYBOARD_SCROLLLOCK       71  // KEYBOARD Scroll Lock
#define KEYBOARD_PAUSE            72  // KEYBOARD Pause
#define KEYBOARD_INSERT           73  // KEYBOARD Insert
#define KEYBOARD_HOME             74  // KEYBOARD Home
#define KEYBOARD_PAGEUP           75  // KEYBOARD PageUp
#define KEYBOARD_DELETE           76  // KEYBOARD Delete
#define KEYBOARD_END              77  // KEYBOARD End
#define KEYBOARD_PAGEDOWN         78  // KEYBOARD PageDown
#define KEYBOARD_RIGHTARROW       79  // KEYBOARD RightArrow
#define KEYBOARD_LEFTARROW        80  // KEYBOARD LeftArrow
#define KEYBOARD_DOWNARROW        81  // KEYBOARD DownArrow
#define KEYBOARD_UPARROW          82  // KEYBOARD UpArrow
#define KEYPAD_NUMLOCK            83  // KEYPAD Num Lock and Clear
#define KEYPAD_SLASH              84  // KEYPAD /
#define KEYPAD_MULTI              85  // KEYPAD *
#define KEYPAD_MINUS              86  // KEYPAD -
#define KEYPAD_PLUSE              87  // KEYPAD +
#define KEYPAD_ENTER              88  // KEYPAD ENTER
#define KEYPAD_1_AND_End          89  // KEYPAD 1 and End
#define KEYPAD_2_AND_DownArrow    90  // KEYPAD 2 and Down Arrow
#define KEYPAD_3_AND_PageDn       91  // KEYPAD 3 and PageDn
#define KEYPAD_4_AND_LeftArrow    92  // KEYPAD 4 and Left Arrow
#define KEYPAD_5                  93  // KEYPAD 5
#define KEYPAD_6_AND_RightArrow   94  // KEYPAD 6 and Right Arrow
#define KEYPAD_7_AND_Home         95  // KEYPAD 7 and Home
#define KEYPAD_8_AND_UpArrow      96  // KEYPAD 8 and Up Arrow
#define KEYPAD_9_AND_PageUp       97  // KEYPAD 9 and PageUp
#define KEYPAD_0_AND_Insert       98  // KEYPAD 0 and Insert
#define KEYPAD_DOT_and_DELETE     99  // KEYPAD . and Delete
#define KEYBOARD_APPLICATION      101 // KEYBOARD Application
#define KEYBOARD_LEFT_CTRL        224 // left ctrl
#define KEYBOARD_LEFT_SHIFT       225 // left shift
#define KEYBOARD_LEFT_ALT         226 // left alt
#define KEYBOARD_LEFT_WIN         227 // left windows
#define KEYBOARD_RIGHT_CTRL       228 // right ctrl
#define KEYBOARD_RIGHT_SHIFT      229 // right shift
#define KEYBOARD_RIGHT_ALT        230 // right alt
#define KEYBOARD_RIGHT_WIN        231 // right windows

//控制键
#define SET_LEFT_CTRL     ((u8)(1 << 0))
#define SET_LEFT_SHIFT    ((u8)(1 << 1))
#define SET_LEFT_ALT      ((u8)(1 << 2))
#define SET_LEFT_WINDOWS  ((u8)(1 << 3))
#define SET_RIGHT_CTRL    ((u8)(1 << 4))
#define SET_RIGHT_SHIFT   ((u8)(1 << 5))
#define SET_RIGHT_ALT     ((u8)(1 << 6))
#define SET_RIGHT_WINDOWS ((u8)(1 << 7))

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API函数声明
*********************************************************************************************************/
//初始化USB键盘驱动
void InitKeyboard(void);

//发送键值给电脑
u8 SendKeyVal(u8 keyFunc, u8 key0, u8 key1, u8 key2, u8 key3, u8 key4, u8 key5);

#endif
