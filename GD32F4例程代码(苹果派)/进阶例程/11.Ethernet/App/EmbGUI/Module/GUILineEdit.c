/*********************************************************************************************************
* 模块名称：GUILineEdit.c
* 摘    要：行编辑模块
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
#include "GUILineEdit.h"
#include "TLILCD.h"
#include "LineEditWidget.h"
#include "JPEG.h"
#include "stdio.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define TEXT_LEN_MAX 64 //显示字符最大长度

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructLineEditWidget s_arrLineEdit[GUI_LINE_EDIT_MAX];
static char                 s_arrText[GUI_LINE_EDIT_MAX][TEXT_LEN_MAX];
static u8                   s_arrCharCnt[GUI_LINE_EDIT_MAX];
static const u8             s_arrCharOff[GUI_LINE_EDIT_MAX] = {3, 5};

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void LineEditPressCallback(void* lineEdit);  //行编辑回调函数

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：LineEditPressCallback
* 函数功能：行编辑回调函数
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：有且仅有一个行编辑控件能获取焦点
*********************************************************************************************************/
static void LineEditPressCallback(void* lineEdit)
{
  u8 i;
  for(i = 0; i < GUI_LINE_EDIT_MAX; i++)
  {
    if((StructLineEditWidget*)lineEdit != &s_arrLineEdit[i])
    {
      //取消控件焦点状态
      ClearLineEditWidgetFocus(&s_arrLineEdit[i]);
    }
  }
}

/*********************************************************************************************************
* 函数名称：CharToU32
* 函数功能：字符转int
* 输入参数：c：需要转换的字符
* 输出参数：void
* 返 回 值：字符对应的数值，返回16表示出错
* 创建日期：2021年07月01日
* 注    意：有且仅有一个行编辑控件能获取焦点
*********************************************************************************************************/
static u32 CharToU32(char c)
{
  u32 result;
  switch(c)
  {
    case '0': result = 0 ; break;
    case '1': result = 1 ; break;
    case '2': result = 2 ; break;
    case '3': result = 3 ; break;
    case '4': result = 4 ; break;
    case '5': result = 5 ; break;
    case '6': result = 6 ; break;
    case '7': result = 7 ; break;
    case '8': result = 8 ; break;
    case '9': result = 9 ; break;
    case 'A': result = 10; break;
    case 'a': result = 10; break;
    case 'B': result = 11; break;
    case 'b': result = 11; break;
    case 'C': result = 12; break;
    case 'c': result = 12; break;
    case 'D': result = 13; break;
    case 'd': result = 13; break;
    case 'E': result = 14; break;
    case 'e': result = 14; break;
    case 'F': result = 15; break;
    case 'f': result = 15; break;
    default : result = 16; break;
  }

  return result;
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitGUILineEdit
* 函数功能：初始化行编辑模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitGUILineEdit(void)
{
  static StructJpegImage s_structHasFocusImage;
  static StructJpegImage s_structNoFocusImage;

  u8 i, j;

  //初始化字符串缓冲区
  for(i = 0; i < GUI_LINE_EDIT_MAX; i++)
  {
    s_arrCharCnt[i] = 0;
    for(j = 0; j < TEXT_LEN_MAX; j++)
    {
      s_arrText[i][j] = 0;
    }
  }

  s_structHasFocusImage.image = (unsigned char*)s_arrJpegLineEditHasFocusImage220x45x24;
  s_structHasFocusImage.size = sizeof(s_arrJpegLineEditHasFocusImage220x45x24) / sizeof(unsigned char);
  s_structNoFocusImage.image = (unsigned char*)s_arrJpegLineEditNoFocusImage220x45x24;
  s_structNoFocusImage.size = sizeof(s_arrJpegLineEditNoFocusImage220x45x24) / sizeof(unsigned char);


  //发送ID
  s_arrText[GUI_LINE_EDIT_CLIENT_IP][0] = 'I';
  s_arrText[GUI_LINE_EDIT_CLIENT_IP][1] = 'P';
  s_arrText[GUI_LINE_EDIT_CLIENT_IP][2] = ':';
  InitLineEditWidgetStruct(&s_arrLineEdit[GUI_LINE_EDIT_CLIENT_IP]);
  s_arrLineEdit[GUI_LINE_EDIT_CLIENT_IP].x0 = 560;
  s_arrLineEdit[GUI_LINE_EDIT_CLIENT_IP].y0 = 105;
  s_arrLineEdit[GUI_LINE_EDIT_CLIENT_IP].width = 220;
  s_arrLineEdit[GUI_LINE_EDIT_CLIENT_IP].height = 45;
  s_arrLineEdit[GUI_LINE_EDIT_CLIENT_IP].textX0 = 2;
  s_arrLineEdit[GUI_LINE_EDIT_CLIENT_IP].textY0 = 10;
  s_arrLineEdit[GUI_LINE_EDIT_CLIENT_IP].text = (char*)(&s_arrText[GUI_LINE_EDIT_CLIENT_IP]);
  s_arrLineEdit[GUI_LINE_EDIT_CLIENT_IP].hasFocusBackgroundImage = (void*)(&s_structHasFocusImage);
  s_arrLineEdit[GUI_LINE_EDIT_CLIENT_IP].noFocusBackgroundImage = (void*)(&s_structNoFocusImage);
  s_arrLineEdit[GUI_LINE_EDIT_CLIENT_IP].pressCallback = LineEditPressCallback;
  CreateLineEditWidget(&s_arrLineEdit[GUI_LINE_EDIT_CLIENT_IP]);

  //发送数据
  s_arrText[GUI_LINE_EDIT_SEND_DATA][0] = 'D';
  s_arrText[GUI_LINE_EDIT_SEND_DATA][1] = 'A';
  s_arrText[GUI_LINE_EDIT_SEND_DATA][2] = 'T';
  s_arrText[GUI_LINE_EDIT_SEND_DATA][3] = 'A';
  s_arrText[GUI_LINE_EDIT_SEND_DATA][4] = ':';
  InitLineEditWidgetStruct(&s_arrLineEdit[GUI_LINE_EDIT_SEND_DATA]);
  s_arrLineEdit[GUI_LINE_EDIT_SEND_DATA].x0 = 560;
  s_arrLineEdit[GUI_LINE_EDIT_SEND_DATA].y0 = 160;
  s_arrLineEdit[GUI_LINE_EDIT_SEND_DATA].width = 220;
  s_arrLineEdit[GUI_LINE_EDIT_SEND_DATA].height = 45;
  s_arrLineEdit[GUI_LINE_EDIT_SEND_DATA].textX0 = 3;
  s_arrLineEdit[GUI_LINE_EDIT_SEND_DATA].textY0 = 10;
  s_arrLineEdit[GUI_LINE_EDIT_SEND_DATA].text = (char*)(&s_arrText[GUI_LINE_EDIT_SEND_DATA]);
  s_arrLineEdit[GUI_LINE_EDIT_SEND_DATA].hasFocusBackgroundImage = (void*)(&s_structHasFocusImage);
  s_arrLineEdit[GUI_LINE_EDIT_SEND_DATA].noFocusBackgroundImage = (void*)(&s_structNoFocusImage);
  s_arrLineEdit[GUI_LINE_EDIT_SEND_DATA].pressCallback = LineEditPressCallback;
  CreateLineEditWidget(&s_arrLineEdit[GUI_LINE_EDIT_SEND_DATA]);
}

/*********************************************************************************************************
* 函数名称：ScanGUILineEdit
* 函数功能：行编辑模块扫描
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void ScanGUILineEdit(void)
{
  u8 i;
  for(i = 0; i < GUI_LINE_EDIT_MAX; i++)
  {
    ScanLineEditWidget(&s_arrLineEdit[i]);
  }
}

/*********************************************************************************************************
* 函数名称：AddGUILineEditText
* 函数功能：给获得焦点的行编辑添加一个字符
* 输入参数：text：字符
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void AddGUILineEditText(char text)
{
  //显示最大字符数量
  static const u8 s_arrTextMaxLen[GUI_LINE_EDIT_MAX] = {15, 13};

  u8 i;
  u8 cnt;
  EnumGUILineEdit focus;

  //查找具有焦点的行编辑控件
  focus = GUI_LINE_EDIT_NONE;
  for(i = 0; i < GUI_LINE_EDIT_MAX; i++)
  {
    if(EDIT_LINE_HAS_FOCUS == s_arrLineEdit[i].hasFocus)
    {
      focus = (EnumGUILineEdit)i;
      break;
    }
  }

  //找到了具有焦点的行编辑控件
  if(GUI_LINE_EDIT_NONE != focus)
  {
    cnt = s_arrCharCnt[focus];
    if(cnt < s_arrTextMaxLen[focus])
    {
      s_arrText[focus][cnt + s_arrCharOff[focus]] = text;
      s_arrCharCnt[focus] = s_arrCharCnt[focus] + 1;
      SetLineEditWidgetText(&s_arrLineEdit[focus], (char*)(&s_arrText[focus]));
    }
    else
    {
      s_arrCharCnt[focus] = s_arrTextMaxLen[focus];
    }
  }
}

/*********************************************************************************************************
* 函数名称：DelectGUILineEditTextChar
* 函数功能：给获得焦点的行编辑删除一个字符
* 输入参数：text：字符
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void DelectGUILineEditTextChar(void)
{
  u8 i;
  u8 cnt;
  EnumGUILineEdit focus;

  //查找具有焦点的行编辑控件
  focus = GUI_LINE_EDIT_NONE;
  for(i = 0; i < GUI_LINE_EDIT_MAX; i++)
  {
    if(EDIT_LINE_HAS_FOCUS == s_arrLineEdit[i].hasFocus)
    {
      focus = (EnumGUILineEdit)i;
      break;
    }
  }

  //找到了具有焦点的行编辑控件
  if(GUI_LINE_EDIT_NONE != focus)
  {
    cnt = s_arrCharCnt[focus];
    if(cnt > 0)
    {
      s_arrText[focus][cnt + s_arrCharOff[focus] - 1] = 0;
      s_arrCharCnt[focus] = s_arrCharCnt[focus] - 1;
      SetLineEditWidgetText(&s_arrLineEdit[focus], (char*)(s_arrText[focus]));
    }
  }
}

/*********************************************************************************************************
* 函数名称：GetGUILineEditRusult
* 函数功能：获取行编辑控件输入结果
* 输入参数：lineEdit：控件枚举，result：用于返回输入结果
* 输出参数：void
* 返 回 值：0-获取成功，其它：获取失败
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
u8 GetGUILineEditRusult(EnumGUILineEdit lineEdit, char** result)
{
  //控件不再许可范围
  if(lineEdit >= GUI_LINE_EDIT_MAX)
  {
    return 1;
  }

  //行编辑中有数据
  if(s_arrCharCnt[lineEdit] > 0)
  {
    *result = s_arrText[lineEdit] + s_arrCharOff[lineEdit];
    return 0;
  }
  
  //无数据
  else
  {
    *result = NULL;
    return 1;
  }
}
