/*********************************************************************************************************
* 模块名称：LineEditWidget.c
* 摘    要：行编辑控件模块
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
#include "LineEditWidget.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void DrawBackground(StructLineEditWidget* widget); //绘制背景
static void DrawText(StructLineEditWidget* widget);       //显示控件字符
static void CallBackProc(StructLineEditWidget* widget);   //回调处理
static u8   IsPress(StructLineEditWidget* widget);        //判断控件是否被按下

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：DrawBackground
* 函数功能：绘制背景
* 输入参数：widget：行编辑控件
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void DrawBackground(StructLineEditWidget* widget)
{
  u32 x0, y0, x1, y1;

  //计算起点终点坐标
  x0 = widget->x0;
  y0 = widget->y0;
  x1 = widget->x0 + widget->width;
  y1 = widget->y0 + widget->height;

  //控件获得焦点
  if(EDIT_LINE_HAS_FOCUS == widget->hasFocus)
  {
    //绘制纯色背景
    if(NULL == widget->hasFocusBackgroundImage)
    {
      GUIFillColor(x0, y0, x1, y1, widget->hasFocusColor);
    }

    //绘制背景图片
    else
    {
      GUIDrawImage(x0, y0, (u32)widget->hasFocusBackgroundImage, widget->hasFocusImageType);
    }
  }

  //控件无焦点
  else if(EDIT_LINE_NO_FOCUS == widget->hasFocus)
  {
    //绘制纯色背景
    if(NULL == widget->noFocusBackgroundImage)
    {
      GUIFillColor(x0, y0, x1, y1, widget->noFocusColor);
    }

    //绘制背景图片
    else
    {
      GUIDrawImage(x0, y0, (u32)widget->noFocusBackgroundImage, widget->noFocusImageType);
    }
  }
}

/*********************************************************************************************************
* 函数名称：DrawText
* 函数功能：显示控件字符
* 输入参数：widget：行编辑控件
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void DrawText(StructLineEditWidget* widget)
{
  u32 x0, y0, len;

  //计算起点终点坐标
  x0 = widget->x0 + widget->textX0;
  y0 = widget->y0 + widget->textY0;

  //计算控件最大显示字符数量
  len = widget->width / GetFontWidth(widget->textFont);

  GUIDrawTextLine(x0, y0, (u32)widget->text, widget->textFont, NULL, widget->textColor, 1, len);
}

/*********************************************************************************************************
* 函数名称：CallBackProc
* 函数功能：回调处理
* 输入参数：widget：行编辑控件
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void CallBackProc(StructLineEditWidget* widget)
{
  if(NULL != widget->pressCallback)
  {
    widget->pressCallback(widget);
  }
}

/*********************************************************************************************************
* 函数名称：IsPress
* 函数功能：判断控件是否按下
* 输入参数：widget：行编辑控件
* 输出参数：void
* 返 回 值：1-控件被按下，0-控件未被按下
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static u8 IsPress(StructLineEditWidget* widget)
{
  u32 tx, ty;         //触点坐标信息
  u8  pointState;     //触点触摸状态
  u32 x0, y0, x1, y1; //按键起点、终点坐标位置

  //获取屏幕触点信息
  pointState = GUIGetTouch(&tx, &ty, 0);

  //触点1并未按下
  if(0 == pointState)
  {
    return 0;
  }

  //触点1按下
  else if(1 == pointState)
  {
    //计算起点终点坐标
    x0 = widget->x0;
    y0 = widget->y0;
    x1 = widget->x0 + widget->width;
    y1 = widget->y0 + widget->height;

    //判断是否按下
    if((tx >= x0) && (tx <= x1) && (ty >= y0) && (ty <= y1))
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }
  
  return 0;
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitLineEditWidgetStruct
* 函数功能：设置行编辑控件结构体默认值
* 输入参数：widget：行编辑控件
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitLineEditWidgetStruct(StructLineEditWidget* widget)
{
  widget->x0                      = 0;                   //起点横坐标默认左上角
  widget->y0                      = 0;                   //起点纵坐标默认左上角
  widget->width                   = 100;                 //宽度
  widget->height                  = 50;                  //高度
  widget->textX0                  = 75;                  //字符与控件原点横坐标偏移量
  widget->textY0                  = 10;                  //字符与控件原点纵坐标偏移量
  widget->textFont                = GUI_FONT_ASCII_24;   //12x24 ASCII码字体
  widget->textColor               = GUI_COLOR_WHITE;     //字体颜色默认为白色
  widget->text                    = NULL;                //默认不显示字符
  widget->hasFocusBackgroundImage = NULL;                //有焦点时不使用背景图片
  widget->noFocusBackgroundImage  = NULL;                //无焦点时不使用背景图片
  widget->hasFocusImageType       = GUI_IMAGE_TYPE_JPEG; //有焦点时背景图片格式默认为JPEG
  widget->noFocusImageType        = GUI_IMAGE_TYPE_JPEG; //无焦点时背景图片格式默认为JPEG
  widget->hasFocusColor           = GUI_COLOR_GREEN;     //不使用背景图片时有焦点用绿色填充
  widget->noFocusColor            = GUI_COLOR_BLUE;      //不使用背景图片时无焦点用蓝色填充
  widget->pressCallback           = NULL;                //不使用回调
}

/*********************************************************************************************************
* 函数名称：CreateLineEditWidget
* 函数功能：初始化行编辑控件模块
* 输入参数：widget：行编辑控件
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void CreateLineEditWidget(StructLineEditWidget* widget)
{
  //控件默认无焦点
  widget->hasFocus = EDIT_LINE_NO_FOCUS;

  //刷新背景
  DrawBackground(widget);

  //绘制显示字符
  DrawText(widget);
}

/*********************************************************************************************************
* 函数名称：ScanLineEditWidget
* 函数功能：行编辑控件扫描
* 输入参数：widget：行编辑控件
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void ScanLineEditWidget(StructLineEditWidget* widget)
{
  //控件被按下
  if(1 == IsPress(widget))
  {
    //自动获取控件焦点
    if(EDIT_LINE_NO_FOCUS == widget->hasFocus)
    {
      //回调处理
      CallBackProc(widget);

      //控件获得焦点
      widget->hasFocus = EDIT_LINE_HAS_FOCUS;

      //背景重绘
      DrawBackground(widget);

      //字符重绘
      DrawText(widget);
    }
  }
}

/*********************************************************************************************************
* 函数名称：ClearLineEditWidgetFocus
* 函数功能：清除行编辑控件焦点
* 输入参数：widget：行编辑控件
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void ClearLineEditWidgetFocus(StructLineEditWidget* widget)
{
  if(EDIT_LINE_HAS_FOCUS == widget->hasFocus)
  {
    //清除行编辑控件焦点
    widget->hasFocus = EDIT_LINE_NO_FOCUS;

    //背景重绘
    DrawBackground(widget);

    //字符重绘
    DrawText(widget);
  }
}

/*********************************************************************************************************
* 函数名称：SetLineEditWidgetText
* 函数功能：设置显示字符
* 输入参数：widget：行编辑控件
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：仅在控件获得焦点的情况下更新字符显示
*********************************************************************************************************/
void SetLineEditWidgetText(StructLineEditWidget* widget, char* text)
{
  //仅在控件获得焦点的情况下更新字符显示
  if(EDIT_LINE_HAS_FOCUS == widget->hasFocus)
  {
    //设置显示内容
    widget->text = text;

    //背景重绘
    DrawBackground(widget);

    //字符重绘
    DrawText(widget);
  }
}
