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
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructLineEditWidget s_arrLineEdit[GUI_LINE_EDIT_MAX];
static char s_arrText[GUI_LINE_EDIT_MAX][TEXT_LEN_MAX];
static u8   s_arrCharCnt[GUI_LINE_EDIT_MAX];

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void LineEditPressCallback(void* lineEdit);  //行编辑回调函数
static u32  CharToU32(char c);                      //字符转int

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
  static const u16  s_arrX0[GUI_LINE_EDIT_MAX]    = {485, 485, 25 , 245}; //横坐标
  static const u16  s_arrY0[GUI_LINE_EDIT_MAX]    = {55 , 120, 340, 340}; //纵坐标
  static const u16  s_arrWidth[GUI_LINE_EDIT_MAX] = {290, 290, 210, 210}; //控件宽度
  static StructJpegImage s_structWriteAddrHasFocusImage;
  static StructJpegImage s_structWriteAddrNoFocusImage;
  static StructJpegImage s_structWriteDataHasFocusImage;
  static StructJpegImage s_structWriteDataNoFocusImage;
  static StructJpegImage s_structReadAddrHasFocusImage;
  static StructJpegImage s_structReadAddrNoFocusImage;
  static StructJpegImage s_structReadLenHasFocusImage;
  static StructJpegImage s_structReadLenNoFocusImage;

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

  //写入地址
  InitLineEditWidgetStruct(&s_arrLineEdit[GUI_LINE_EDIT_WRITE_ADDR]);
  s_structWriteAddrHasFocusImage.image = (unsigned char*)s_arrJpegWriteAddrHasFocusImage290x50x24;
  s_structWriteAddrHasFocusImage.size = sizeof(s_arrJpegWriteAddrHasFocusImage290x50x24) / sizeof(unsigned char);
  s_structWriteAddrNoFocusImage.image = (unsigned char*)s_arrJpegWriteAddrNoFocusImage290x50x24;
  s_structWriteAddrNoFocusImage.size = sizeof(s_arrJpegWriteAddrNoFocusImage290x50x24) / sizeof(unsigned char);
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_ADDR].x0 = s_arrX0[GUI_LINE_EDIT_WRITE_ADDR];
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_ADDR].y0 = s_arrY0[GUI_LINE_EDIT_WRITE_ADDR];
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_ADDR].width = s_arrWidth[GUI_LINE_EDIT_WRITE_ADDR];
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_ADDR].height = 50;
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_ADDR].textX0 = 75;
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_ADDR].textY0 = 10;
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_ADDR].text = (char*)(&s_arrText[GUI_LINE_EDIT_WRITE_ADDR]);
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_ADDR].hasFocusBackgroundImage = (void*)(&s_structWriteAddrHasFocusImage);
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_ADDR].noFocusBackgroundImage = (void*)(&s_structWriteAddrNoFocusImage);
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_ADDR].pressCallback = LineEditPressCallback;
  CreateLineEditWidget(&s_arrLineEdit[GUI_LINE_EDIT_WRITE_ADDR]);

  //要写入的数据
  InitLineEditWidgetStruct(&s_arrLineEdit[GUI_LINE_EDIT_WRITE_DATA]);
  s_structWriteDataHasFocusImage.image = (unsigned char*)s_arrJpegWriteDataHasFocusImage290x50x24;
  s_structWriteDataHasFocusImage.size = sizeof(s_arrJpegWriteDataHasFocusImage290x50x24) / sizeof(unsigned char);
  s_structWriteDataNoFocusImage.image = (unsigned char*)s_arrJpegWriteDataNoFocusImage290x50x24;
  s_structWriteDataNoFocusImage.size = sizeof(s_arrJpegWriteDataNoFocusImage290x50x24) / sizeof(unsigned char);
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_DATA].x0 = s_arrX0[GUI_LINE_EDIT_WRITE_DATA];
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_DATA].y0 = s_arrY0[GUI_LINE_EDIT_WRITE_DATA];
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_DATA].width = s_arrWidth[GUI_LINE_EDIT_WRITE_DATA];
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_DATA].height = 50;
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_DATA].textX0 = 75;
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_DATA].textY0 = 10;
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_DATA].text = (char*)(&s_arrText[GUI_LINE_EDIT_WRITE_DATA]);
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_DATA].hasFocusBackgroundImage = (void*)(&s_structWriteDataHasFocusImage);
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_DATA].noFocusBackgroundImage = (void*)(&s_structWriteDataNoFocusImage);
  s_arrLineEdit[GUI_LINE_EDIT_WRITE_DATA].pressCallback = LineEditPressCallback;
  CreateLineEditWidget(&s_arrLineEdit[GUI_LINE_EDIT_WRITE_DATA]);

  //读取地址
  InitLineEditWidgetStruct(&s_arrLineEdit[GUI_LINE_EDIT_READ_ADDR]);
  s_structReadAddrHasFocusImage.image = (unsigned char*)s_arrJpegReadAddrHasFocusImage210x50x24;
  s_structReadAddrHasFocusImage.size = sizeof(s_arrJpegReadAddrHasFocusImage210x50x24) / sizeof(unsigned char);
  s_structReadAddrNoFocusImage.image = (unsigned char*)s_arrJpegReadAddrNoFocusImage210x50x24;
  s_structReadAddrNoFocusImage.size = sizeof(s_arrJpegReadAddrNoFocusImage210x50x24) / sizeof(unsigned char);
  s_arrLineEdit[GUI_LINE_EDIT_READ_ADDR].x0 = s_arrX0[GUI_LINE_EDIT_READ_ADDR];
  s_arrLineEdit[GUI_LINE_EDIT_READ_ADDR].y0 = s_arrY0[GUI_LINE_EDIT_READ_ADDR];
  s_arrLineEdit[GUI_LINE_EDIT_READ_ADDR].width = s_arrWidth[GUI_LINE_EDIT_READ_ADDR];
  s_arrLineEdit[GUI_LINE_EDIT_READ_ADDR].height = 50;
  s_arrLineEdit[GUI_LINE_EDIT_READ_ADDR].textX0 = 85;
  s_arrLineEdit[GUI_LINE_EDIT_READ_ADDR].textY0 = 10;
  s_arrLineEdit[GUI_LINE_EDIT_READ_ADDR].text = (char*)(&s_arrText[GUI_LINE_EDIT_READ_ADDR]);
  s_arrLineEdit[GUI_LINE_EDIT_READ_ADDR].hasFocusBackgroundImage = (void*)(&s_structReadAddrHasFocusImage);
  s_arrLineEdit[GUI_LINE_EDIT_READ_ADDR].noFocusBackgroundImage = (void*)(&s_structReadAddrNoFocusImage);
  s_arrLineEdit[GUI_LINE_EDIT_READ_ADDR].pressCallback = LineEditPressCallback;
  CreateLineEditWidget(&s_arrLineEdit[GUI_LINE_EDIT_READ_ADDR]);

  //读取长度
  InitLineEditWidgetStruct(&s_arrLineEdit[GUI_LINE_EDIT_READ_LEN]);
  s_structReadLenHasFocusImage.image = (unsigned char*)s_arrJpegReadLenHasFocusImage210x50x24;
  s_structReadLenHasFocusImage.size = sizeof(s_arrJpegReadLenHasFocusImage210x50x24) / sizeof(unsigned char);
  s_structReadLenNoFocusImage.image = (unsigned char*)s_arrJpegReadLenNoFocusImage210x50x24;
  s_structReadLenNoFocusImage.size = sizeof(s_arrJpegReadLenNoFocusImage210x50x24) / sizeof(unsigned char);
  s_arrLineEdit[GUI_LINE_EDIT_READ_LEN].x0 = s_arrX0[GUI_LINE_EDIT_READ_LEN];
  s_arrLineEdit[GUI_LINE_EDIT_READ_LEN].y0 = s_arrY0[GUI_LINE_EDIT_READ_LEN];
  s_arrLineEdit[GUI_LINE_EDIT_READ_LEN].width = s_arrWidth[GUI_LINE_EDIT_READ_LEN];
  s_arrLineEdit[GUI_LINE_EDIT_READ_LEN].height = 50;
  s_arrLineEdit[GUI_LINE_EDIT_READ_LEN].textX0 = 85;
  s_arrLineEdit[GUI_LINE_EDIT_READ_LEN].textY0 = 10;
  s_arrLineEdit[GUI_LINE_EDIT_READ_LEN].text = (char*)(&s_arrText[GUI_LINE_EDIT_READ_LEN]);
  s_arrLineEdit[GUI_LINE_EDIT_READ_LEN].hasFocusBackgroundImage = (void*)(&s_structReadLenHasFocusImage);
  s_arrLineEdit[GUI_LINE_EDIT_READ_LEN].noFocusBackgroundImage = (void*)(&s_structReadLenNoFocusImage);
  s_arrLineEdit[GUI_LINE_EDIT_READ_LEN].pressCallback = LineEditPressCallback;
  CreateLineEditWidget(&s_arrLineEdit[GUI_LINE_EDIT_READ_LEN]);
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
  static const u8 s_arrTextMaxLen[GUI_LINE_EDIT_MAX] = {8, 8, 8, 2};

  u8 i;
  u8 cnt;
  EnumGUILineEdit focus;

  //查找具有焦点的行编辑控件
  focus = GUI_LINE_EDIT_NONE;
  for(i = GUI_LINE_EDIT_WRITE_ADDR; i < GUI_LINE_EDIT_MAX; i++)
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
      s_arrText[focus][cnt] = text;
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
  for(i = GUI_LINE_EDIT_WRITE_ADDR; i < GUI_LINE_EDIT_MAX; i++)
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
      s_arrText[focus][cnt - 1] = 0;
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
u8 GetGUILineEditRusult(EnumGUILineEdit lineEdit, u32* result)
{
  u32 sum;       //最终结果
  u32 charValue; //当前字符对应的数值
  u8 i = 0;

  //控件不再许可范围
  if(lineEdit >= GUI_LINE_EDIT_MAX)
  {
    return 1;
  }

  //计算行编辑输入值
  i = 0;
  sum = 0;
  while(1)
  {
    charValue = CharToU32(s_arrText[lineEdit][i]);
    if(charValue < 16)
    {
      sum = sum * 16 + charValue;
    }
    else
    {
      break;
    }

    i = i + 1;
  }

  //读取第一个字符对应的数值时失败，表示当前行编辑输入为空
  if(0 == i)
  {
    return 1;
  }

  //返回输入值
  else
  {
    *result = sum;
    return 0;
  }
}
