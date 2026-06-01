/*********************************************************************************************************
* 模块名称：GUIGraph.c
* 摘    要：动态内存波形显示模块
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
#include "GUIGraph.h"
#include "GUIPlatform.h"
#include "TLILCD.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructGUIGraphDev s_structGraph;      //内存波形显示设备结构体
static u8                s_arrWave1Buf[800]; //波形1显示波形
static u8                s_arrWave2Buf[800]; //波形2显示波形

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：DrawBackground
* 函数功能：刷新背景
* 输入参数：widget：控件
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void DrawBackground(StructGUIGraphDev* widget)
{
  //起点、终点坐标
  u32 x0, y0, x1, y1;

  x0 = widget->x0;
  y0 = widget->y0;
  x1 = widget->x0 + widget->width;
  y1 = widget->y0 + widget->height;
  GUIFillColor(x0, y0, x1, y1, widget->backColor);
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：CreateGUIGraph
* 函数功能：创建动态内存波形显示模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void CreateGUIGraph(void)
{
  //设置波形显示各项参数
  s_structGraph.x0 = 25;
  s_structGraph.y0 = 140;
  s_structGraph.width = 750;
  s_structGraph.height = 230;
  s_structGraph.xOff = 10;
  s_structGraph.yOff = 10;
  s_structGraph.backColor = GUI_COLOR_BLACK;
  s_structGraph.line1Color = GUI_COLOR_GREEN;
  s_structGraph.line2Color = GUI_COLOR_ORANGE;
  s_structGraph.linePointNum = 0;
  s_structGraph.waveSpeed = 10;
  s_structGraph.line1Buf = (u8*)s_arrWave1Buf;
  s_structGraph.line2Buf = (u8*)s_arrWave2Buf;

  //绘制背景
  DrawBackground(&s_structGraph);
}

/*********************************************************************************************************
* 函数名称：GUIGraphAddrPoint
* 函数功能：波形增加点
* 输入参数：wave1,wave2：波形1、波形2数据
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void GUIGraphAddrPoint(u8 wave1, u8 wave2)
{
  //绘制波形区域横坐标起点、终点坐标、显示宽度以及循环变量
  u32 x0, x1, width, i;

  //数据范围超标
  if((wave1 > 100) || (wave2 > 100))
  {
    return;
  }

  //计算横坐标起点、终点坐标以及宽度
  x0 = s_structGraph.x0 + s_structGraph.xOff;
  x1 = s_structGraph.x0 + s_structGraph.width - s_structGraph.xOff;
  width = x1 - x0 + 1;
  width = width / s_structGraph.waveSpeed;

  //缓冲区未满情况
  if(s_structGraph.linePointNum < width)
  {
    s_structGraph.line1Buf[s_structGraph.linePointNum] = wave1;
    s_structGraph.line2Buf[s_structGraph.linePointNum] = wave2;
    s_structGraph.linePointNum++;
  }

  //缓冲区已满，需要做数据移动
  else
  {
    //数据往前移动，舍弃最旧的点
    for(i = 0; i < (width - 1); i++)
    {
      s_structGraph.line1Buf[i] = s_structGraph.line1Buf[i + 1];
      s_structGraph.line2Buf[i] = s_structGraph.line2Buf[i + 1];
    }

    //将最新的数据保存到数组末尾
    s_structGraph.line1Buf[width - 1] = wave1;
    s_structGraph.line2Buf[width - 1] = wave2;
  }
}

/*********************************************************************************************************
* 函数名称：GUIGraphRefresh
* 函数功能：刷新显示
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void GUIGraphRefresh(void)
{
  //绘制波形区域起点、终点坐标、显示宽度、显示高度以及循环变量
  u32 x0, y0, y1, width, height, i;

  //线条起点、终点坐标
  u32 lineX0, lineY0, lineX1, lineY1;

  //计算起点、终点坐标以及宽度
  x0 = s_structGraph.x0 + s_structGraph.xOff;
  y0 = s_structGraph.y0 + s_structGraph.yOff;
  y1 = s_structGraph.y0 + s_structGraph.height - s_structGraph.yOff;
  width = s_structGraph.linePointNum;
  height = y1 - y0 + 1;

  //画曲线
  for(i = 0; i < (width - 1); i++)
  {
    //计算线条横坐标起点、终点
    lineX0 = x0 + i * s_structGraph.waveSpeed;
    lineX1 = lineX0 + s_structGraph.waveSpeed;

    //画背景，顺便清除之前显示的点
    GUIFillColor(lineX0, y0, lineX1, y1 + 1, s_structGraph.backColor);

    //绘制线条1
    //计算第一个点纵坐标
    lineY0 = s_structGraph.line1Buf[i];
    lineY0 = 100 - lineY0;
    lineY0 = height * lineY0 / 100;
    lineY0 = y0 + lineY0;

    //计算第二个点纵坐标
    lineY1 = s_structGraph.line1Buf[i + 1];
    lineY1 = 100 - lineY1;
    lineY1 = height * lineY1 / 100;
    lineY1 = y0 + lineY1;

    //画线
    LCDDrawLine(lineX0, lineY0, lineX1, lineY1, s_structGraph.line1Color);

    //显示线条2
    //计算第一个点纵坐标
    lineY0 = s_structGraph.line2Buf[i];
    lineY0 = 100 - lineY0;
    lineY0 = height * lineY0 / 100;
    lineY0 = y0 + lineY0;

    //计算第二个点纵坐标
    lineY1 = s_structGraph.line2Buf[i + 1];
    lineY1 = 100 - lineY1;
    lineY1 = height * lineY1 / 100;
    lineY1 = y0 + lineY1;

    //画线
    LCDDrawLine(lineX0, lineY0, lineX1, lineY1, s_structGraph.line2Color);
  }
}

