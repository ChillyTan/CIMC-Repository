/*********************************************************************************************************
* 模块名称：GUITerminal.c
* 摘    要：LED模块
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
#include "GUITerminal.h"
#include "LCD.h"
#include "JPEG.h"
#include "stdio.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define MAX_LINE_LEN 48 //一行最多显示48个字符
#define LINE_NUM     8  //显示8行

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructGUITherminal s_structTherminal;                   //终端设备结构体
static char               s_arrString[LINE_NUM][MAX_LINE_LEN]; //字符串缓冲区，最多显示8行
static u8                 s_iStringHead;                       //新行插入位置，从0开始
static u8                 s_iStringNum;                        //已显示行数

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void DisplayBackground(void);                  //绘制背景
static void ShowString(char *string, u16 x0, u16 y0); //显示字符串
static void UpdateStringShow(void);                   //更新字符显示

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：DisplayBackground
* 函数功能：绘制背景
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：调用之前要确保LCD方向为横屏
*********************************************************************************************************/
static void DisplayBackground(void)
{
  //背景图片控制结构体
  StructJpegImage backgroundImage;

  //初始化backgroundImage
  backgroundImage.image = (unsigned char*)s_arrJpegTerminalImage430x270x24;
  backgroundImage.size  = sizeof(s_arrJpegTerminalImage430x270x24) / sizeof(unsigned char);

  //解码并显示图片
  DisplayJPEGInFlash(&backgroundImage, s_structTherminal.x0, s_structTherminal.y0);
}

/*********************************************************************************************************
* 函数名称：ShowString
* 函数功能：显示一行字符串
* 输入参数：string：字符串首地址，x0,y0：相对于终端的起始坐标
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ShowString(char *string, u16 x0, u16 y0)
{
  u16 x, y, i, x1, size;

  //计算起始坐标和终点坐标
  x = s_structTherminal.x0 + x0;
  y = s_structTherminal.y0 + y0;
  x1 = s_structTherminal.x0 + s_structTherminal.width - 3;

  //清一行背景
  size = s_structTherminal.textSize;
  LCDFill(x, y, x1, y + size, s_structTherminal.textBackColor);

  //空串直接返回
  if(NULL == string)
  {
    return;
  }

  //判断是不是非法字符!
  i = 0;
  while((*(string + i) <= '~') && (*(string + i) >= ' '))
  {
    //判断回车换行
    if(('\r' == *(string + i)) || ('\n' == *(string + i)))
    {
      break;
    }

    //到了一行的尽头
    if(x > (x1 - s_structTherminal.textSize / 2))
    {
      break;
    }

    //打印输出到终端
    s_iLCDPointColor = s_structTherminal.textColor;
    LCDShowChar(x, y, *(string + i), s_structTherminal.textSize, 1);
    x += s_structTherminal.textSize / 2;
    i++;
  }
}

/*********************************************************************************************************
* 函数名称：UpdateStringShow
* 函数功能：更新字符显示
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void UpdateStringShow(void)
{
  u8 i, head, stringNum;
  u16 height;

  head = s_iStringHead;
  stringNum = s_iStringNum;
  height = s_structTherminal.textHeight;

  //行数记录为零，直接返回
  if(0 == stringNum)
  {
    return;
  }

  //行数不足LINE_NUM行，则从缓冲区0行开始显示
  else if(stringNum < LINE_NUM)
  {
    for(i = 0; i <= stringNum; i++)
    {
      ShowString(s_arrString[i], 3, 3 + height * (i + 1));
    }
  }

  //行数已满，需要考虑从哪行开始显示
  else
  {
    for(i = 0; i < LINE_NUM; i++)
    {
      ShowString(s_arrString[head], 3, 3 + height * (i + 1));
      head = (head + 1) % LINE_NUM;
    }
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitGUITerminal
* 函数功能：初始化GUI终端模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：调用之前要确保LCD方向为横屏
*********************************************************************************************************/
void InitGUITerminal(void)
{
  u16 x, y; //终端中心坐标位置

  s_structTherminal.x0 = 25;
  s_structTherminal.y0 = 55;
  s_structTherminal.width = 430;
  s_structTherminal.height = 270;
  s_structTherminal.textColor = WHITE;
  s_structTherminal.textSize = 24;
  s_structTherminal.textHeight = 30;
  DisplayBackground();

  //获取终端背景颜色
  x = s_structTherminal.x0 + s_structTherminal.width / 2;
  y = s_structTherminal.y0 + s_structTherminal.height / 2;
  s_structTherminal.textBackColor = LCDReadPoint(x, y);
}

/*********************************************************************************************************
* 函数名称：ShowAddrInTerminal
* 函数功能：终端显示地址范围
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void ShowAddrInTerminal(u32 begin, u32 end)
{
  char string[30];
  sprintf(string, "Addr: 0x%08X - 0x%08X", begin, end);
  ShowString(string, 60, 3);
}

/*********************************************************************************************************
* 函数名称：ClearGUITerminal
* 函数功能：清空终端显示
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：首行地址信息并不会被清除
*********************************************************************************************************/
void ClearGUITerminal(void)
{
  u8 i, j;

  for(i = 0; i < LINE_NUM; i++)
  {
    for(j = 0; j < MAX_LINE_LEN; j++)
    {
      s_arrString[i][j] = 0;
    }
  }

  //设置保存到的字符串数量为LINE_NUM，利用UpdateStringShow函数先清背景再绘制字符串的性质清空显示
  s_iStringNum = LINE_NUM;
  UpdateStringShow();

  //清空标记
  s_iStringHead = 0;
  s_iStringNum = 0;
}

/*********************************************************************************************************
* 函数名称：ShowStringLineInGUITerminal
* 函数功能：终端中显示一行字符串
* 输入参数：string：字符串
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void ShowStringLineInGUITerminal(char* string)
{
  u8 i;

  //保存字符串到缓冲区
  i = 0;
  while((*(string + i) <= '~') && (*(string + i) >= ' ') && (i < (MAX_LINE_LEN - 1)))
  {
    s_arrString[s_iStringHead][i] = *(string + i);
    i++;
  }
  s_arrString[s_iStringHead][i] = 0;

  //更新新行插入位置
  s_iStringHead = (s_iStringHead + 1) % LINE_NUM;

  //记录行数
  if(s_iStringNum < LINE_NUM)
  {
    s_iStringNum = s_iStringNum + 1;
  }
  else
  {
    s_iStringNum = LINE_NUM;
  }

  //更新终端显示
  UpdateStringShow();
}
