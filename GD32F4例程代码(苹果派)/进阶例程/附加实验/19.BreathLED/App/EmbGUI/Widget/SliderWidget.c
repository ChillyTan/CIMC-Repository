/*********************************************************************************************************
* 模块名称：SliderWidget.c
* 摘    要：滑条控件模块
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
#include "SliderWidget.h"
#include "gd32f470x_conf.h"
#include "Touch.h"
#include "TLILCD.h"
#include "BMP.h"
#include "Timer.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void DrawPoint(u16 x0,u16 y0, u16 r, u16 color);                    //绘制实心圆
static void Repaint(StructSliderWidget* widget);                           //控件重绘


/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：DrawPoint
* 函数功能：绘制实心圆
* 输入参数：（x0、y0）：圆心坐标，r：半径，color：颜色
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void DrawPoint(u16 x0,u16 y0, u16 r, u16 color)
{
  int a, b;
  int di;
  a = 0; b = r;	  
  di = 3 - (r << 1);             //判断下个点位置的标志
  while(a <= b)
  {
    int i = a, p = b;
    while(i > 0)
    {
      LCDDrawPoint(x0+b,y0-i, color);
      LCDDrawPoint(x0-i,y0+b, color);
      i--;
    }
    while( p > 0)
    {
      LCDDrawPoint(x0 - a, y0 - p, color);
      LCDDrawPoint(x0 - p, y0 - a, color);
      LCDDrawPoint(x0 + a, y0 - p, color);
      LCDDrawPoint(x0 - p, y0 + a, color);
      LCDDrawPoint(x0 + a, y0 + p, color);
      LCDDrawPoint(x0 + p, y0 + a, color);
      p--;
    }
    a++;

    //Bresenham算法画圆
    if(di < 0)
    {
      di += 4 * a + 6;
    }
    else
    {
      di += 10 + 4 * (a - b);
      b--;
    }
  }
  LCDDrawPoint(x0, y0, color); //圆心坐标
}

/*********************************************************************************************************
* 函数名称：Repaint
* 函数功能：控件重绘
* 输入参数：widget：需要重绘的控件
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：圆点直径默认为控件高度
*********************************************************************************************************/
static void Repaint(StructSliderWidget* widget)
{
  static u64 s_arrlastTime = 0;
  u64 newTime;
  u16 x0, y0, x1, y1;
  u16 lineY, lineX0, lineX1, lineY0, lineY1;
  u16 circleR, circleX0, circleX1, circleX, circleY;

  newTime = GetSysTime();

  if(newTime - s_arrlastTime >= 100)
  {
    s_arrlastTime = newTime;

    //计算起点终点
    x0 = widget->x0;
    y0 = widget->y0;
    x1 = widget->x0 + widget->width;
    y1 = widget->y0 + widget->height;

    //绘制背景
    LCDColorFillPixel(x0, y0, x1 - 1, y1 - 1, (u16*)widget->background);

    //绘制横线
    if(NULL != widget->lineImage)
    {
      BMPDisplayInFlash(widget->lineImage, x0, y0);
    }
    else
    {
      lineY = (y0 + y1) / 2;
      lineX0 = x0 + widget->lineSize;
      lineX1 = x1 - widget->lineSize;
      lineY0 = lineY - widget->lineSize;
      lineY1 = lineY + widget->lineSize;
      LCDFillPixel(lineX0, lineY0, lineX1, lineY1, widget->lineColor);
      DrawPoint(lineX0, lineY, widget->lineSize, widget->lineColor);
      DrawPoint(lineX1, lineY, widget->lineSize, widget->lineColor);
    }


    //绘制圆点
    if(NULL != widget->circleImage)
    {
      circleR  = (widget->height / 2) - 1; //半径
      circleX0 = x0 + 3 + circleR;         //最小横坐标
      circleX1 = x1 - 3 - circleR;         //最大横坐标
      circleX0 = circleX0 + (circleX1 - circleX0) * widget->value / 100 - circleR;
      BMPDisplayInFlash(widget->circleImage, circleX0, y0);
    }
    else
    {
      circleR  = (widget->height / 2) - 1; //半径
      circleX0 = x0 + 1 + circleR;         //最小横坐标
      circleX1 = x1 - 1 - circleR;         //最大横坐标
      circleX  = circleX0 + (circleX1 - circleX0) * widget->value / 100; //圆心横坐标
      circleY  = (y0 + y1) / 2;      //圆心纵坐标
      DrawPoint(circleX, circleY, circleR, widget->circleColor);
    }
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：CreateSliderWidget
* 函数功能：创建滑条控件
* 输入参数：widget：滑条控件
* 输出参数：void
* 返 回 值：0-成功创建，其它-创建失败
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
u8 CreateSliderWidget(StructSliderWidget* widget)
{
  u16 x, y, i;
  u16* background;

  //背景缓冲区不存在
  if(NULL == widget->background)
  {
    return 1;
  }

  //将背景储存到缓冲区
  i = 0;
  background = (u16*)widget->background;
  for(y = widget->y0; y < (widget->y0 + widget->height); y++)
  {
    for(x = widget->x0; x < (widget->x0 + widget->width); x++)
    {
      background[i] = LCDReadPoint(x, y);
      i++;
    }
  }

  //绘制控件
  Repaint(widget);
  
  return 0;
}

/*********************************************************************************************************
* 函数名称：ScanSliderWidget
* 函数功能：扫描滑条控件
* 输入参数：widget：滑条控件
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void ScanSliderWidget(StructSliderWidget* widget)
{
  StructTouchPoint point; //触点位置信息
  u8  pointState;         //触点触摸状态
  u16 x0, y0, x1, y1;     //起点、终点坐标位置
  u8  value;              //滑条数值

  //获取触点1信息
  pointState = GetTouch1Result(&point);

  //触点被按下
  if(1 == pointState)
  {
    //计算起点终点
    x0 = widget->x0;
    y0 = widget->y0;
    x1 = widget->x0 + widget->width;
    y1 = widget->y0 + widget->height;

    //判断控件所在区域是否被按下
    if((point.x >= x0) && (point.x <= x1) && (point.y >= y0) && (point.y <= y1))
    {
      value = 100 * (point.x - x0) / (x1 - x0);

      //重绘
      if(value != widget->value)
      {
        widget->value = value;
        Repaint(widget);
      }
    }
  }
}

