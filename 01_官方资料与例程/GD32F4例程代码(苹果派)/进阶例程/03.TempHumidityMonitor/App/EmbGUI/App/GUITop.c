/*********************************************************************************************************
* 模块名称：GUITop.c
* 摘    要：顶层界面设计
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
#include "GUITop.h"
#include "TLILCD.h"
#include "JPEG.h"
#include "TextWidget.h"
#include "GUIPlatform.h"
#include "stdio.h"
#include "math.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructGUIDev* s_pGUIDev = NULL;
static StructTextWidget s_structInTemp;
static StructTextWidget s_structExTemp;
static StructTextWidget s_structHumidity;
static u16 s_arrInTempBackgound[1440];
static u16 s_arrExTempBackgound[1440];
static u16 s_arrHumidityBackgound[1440];

static u32 s_iTempColor;
static u32 s_iHumidityColor;
static u16 s_iHumidityBackground[106 * 106];

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void DisplayBackground(void);               //绘制背景
static void UpdataInTempShow(double temp);         //更新内部温度值显示
static void UpdataExTempShow(double temp);         //更新外部温度值显示
static void UpdateExHumidityShow(double humidity); //更新外部湿度度值显示

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
  backgroundImage.image = (u8*)s_arrJpegBackgroundImage;
  backgroundImage.size  = sizeof(s_arrJpegBackgroundImage) / sizeof(u8);

  //解码并显示图片
  DisplayJPEGInFlash(&backgroundImage, 0, 0);
}

/*********************************************************************************************************
* 函数名称：UpdataInTempShow
* 函数功能：更新内部温度值显示
* 输入参数：temp：温度值
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void UpdataInTempShow(double temp)
{
  //填充区域
  u32 x0, y0, x1, y1, yTemp;

  //字符串转换缓冲区
  char string[16];

  //温度值显示
  sprintf(string, "%.1f", temp);
  s_structInTemp.setText(&s_structInTemp, string);

  //设置填充区域
  x0 = 115;
  y0 = 105;
  x1 = 128;
  y1 = 280;
  
  //温度大于100摄氏度强制显示满格
  if(temp >= 100)
  {
    temp = 100;
  }

  //0-100℃
  if(0 == s_pGUIDev->ShowFlag)
  {
    if((temp >= 0) && (temp <= 100))
    {
      yTemp = y1 - (y1 - y0) * temp / 100.0;
      GUIFillColor(x0, yTemp, x1, y1, s_iTempColor);
      GUIFillColor(x0, y0, x1, yTemp, GUI_COLOR_WHITE);
    }
    else
    {
      GUIFillColor(x0, y0, x1, y1, GUI_COLOR_WHITE);
    }
  }

  //±50℃
  if(1 == s_pGUIDev->ShowFlag)
  {
    if((temp >= -50) && (temp <= 50))
    {
      yTemp = y1 - (y1 - y0) * (temp + 50.0) / 100.0;
      GUIFillColor(x0, yTemp, x1, y1, s_iTempColor);
      GUIFillColor(x0, y0, x1, yTemp, GUI_COLOR_WHITE);
    }
    else
    {
      GUIFillColor(x0, y0, x1, y1, GUI_COLOR_WHITE);
    }
  }
}

/*********************************************************************************************************
* 函数名称：UpdataExTempShow
* 函数功能：更新外部温度值显示
* 输入参数：temp：温度值
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void UpdataExTempShow(double temp)
{
  //填充区域
  u32 x0, y0, x1, y1, yTemp;

  //字符串转换缓冲区
  char string[16];

  //温度值显示
  sprintf(string, "%.1f", temp);
  s_structExTemp.setText(&s_structExTemp, string);

  //设置填充区域
  x0 = 386;
  y0 = 105;
  x1 = 399;
  y1 = 280;

  //温度大于100摄氏度强制显示满格
  if(temp >= 100)
  {
    temp = 100;
  }
  
  //0-100℃
  if(0 == s_pGUIDev->ShowFlag)
  {
    if((temp >= 0) && (temp <= 100))
    {
      yTemp = y1 - (y1 - y0) * temp / 100.0;
      GUIFillColor(x0, yTemp, x1, y1, s_iTempColor);
      GUIFillColor(x0, y0, x1, yTemp, GUI_COLOR_WHITE);
    }
    else
    {
      GUIFillColor(x0, y0, x1, y1, GUI_COLOR_WHITE);
    }
  }

  //±50℃
  if(1 == s_pGUIDev->ShowFlag)
  {
    if((temp >= -50) && (temp <= 50))
    {
      yTemp = y1 - (y1 - y0) * (temp + 50.0) / 100.0;
      GUIFillColor(x0, yTemp, x1, y1, s_iTempColor);
      GUIFillColor(x0, y0, x1, yTemp, GUI_COLOR_WHITE);
    }
    else
    {
      GUIFillColor(x0, y0, x1, y1, GUI_COLOR_WHITE);
    }
  }
}

/*********************************************************************************************************
* 函数名称：UpdateExHumidityShow
* 函数功能：更新外部湿度度值显示
* 输入参数：humidity：湿度值
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void UpdateExHumidityShow(double humidity)
{
  int x, y, x0, y0, x1, y1;
  double pi, angle0, angle1, angle;

  //字符串转换缓冲区
  char string[16];

  //温度值显示
  if(humidity >= 100.0)
  {
    sprintf(string, "100");
  }
  else
  {
    sprintf(string, "%.1f", humidity);
  }
  s_structHumidity.setText(&s_structHumidity, string);

  //重绘背景
  GUIDrawBackground(616, 167, 106, 106, (u32)s_iHumidityBackground);

  //显示范围限定
  if(humidity < 0)
  {
    humidity = 0;
  }
  else if(humidity > 100)
  {
    humidity = 100;
  }

  //温湿度翻转
  humidity = 100.0 - humidity;

  //求直线在直角坐标系中的角度
  pi = 3.1415926535;
  angle0 = -pi / 3.0;      //100%对应的角度（弧度）
  angle1 = pi * 4.0 / 3.0; //0%对应的角度（弧度）
  angle = angle0 + (angle1 - angle0) * humidity / 100.0;

  //画线
  x = 50.0 * cos(angle);
  y = 50.0 * sin(angle);
  y = -y;
  x0 = 669;
  y0 = 220;
  x1 = x0 + x;
  y1 = y0 + y;
  GUIDrawLine(x0, y0, x1, y1, 3, s_iHumidityColor, GUI_LINE_SQUARE);
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitGUI
* 函数功能：初始化GUI
* 输入参数：dev：GUI设备结构体
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：书名使用12x24字体
*********************************************************************************************************/
void InitGUI(StructGUIDev* dev)
{
  //字符串转换缓冲区
  char string[16];

  //保存设备指针
  s_pGUIDev = dev;

  //LCD横屏显示
  LCDDisplayDir(LCD_SCREEN_HORIZONTAL);
  LCDClear(LCD_COLOR_CYAN);

  //绘制背景
  DisplayBackground();

  //初始化显示模块
  s_iTempColor = GUIReadPoint(122, 290);
  GUIFillColor(115, 105, 128, 280, GUI_COLOR_WHITE);
  GUIFillColor(386, 105, 399, 280, GUI_COLOR_WHITE);

  s_iHumidityColor = GUIReadPoint(669, 220);
  GUISaveBackground(616, 167, 106, 106, (u32)s_iHumidityBackground);

  //连接函数指针
  s_pGUIDev->setInTemp = UpdataInTempShow;
  s_pGUIDev->setExtemp = UpdataExTempShow;
  s_pGUIDev->setHumidity = UpdateExHumidityShow;

  //内部温度
  InitTextWidgetStruct(&s_structInTemp);
  s_structInTemp.x0 = 124;
  s_structInTemp.y0 = 366;
  s_structInTemp.width = 60;
  s_structInTemp.background = (u32)s_arrInTempBackgound;
  CreateTextWidget(&s_structInTemp);
  sprintf(string, "--");
  s_structInTemp.setText(&s_structInTemp, string);

  //外部温度
  InitTextWidgetStruct(&s_structExTemp);
  s_structExTemp.x0 = 394;
  s_structExTemp.y0 = 366;
  s_structExTemp.width = 60;
  s_structExTemp.background = (u32)s_arrExTempBackgound;
  CreateTextWidget(&s_structExTemp);
  sprintf(string, "--");
  s_structExTemp.setText(&s_structExTemp, string);

  //外部湿度
  InitTextWidgetStruct(&s_structHumidity);
  s_structHumidity.x0 = 665;
  s_structHumidity.y0 = 366;
  s_structHumidity.width = 60;
  s_structHumidity.background = (u32)s_arrHumidityBackgound;
  CreateTextWidget(&s_structHumidity);
  sprintf(string, "--");
  s_structHumidity.setText(&s_structHumidity, string);
}

/*********************************************************************************************************
* 函数名称：GUITask
* 函数功能：GUI任务
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：每隔20毫秒执行一次
*********************************************************************************************************/
void GUITask(void)
{
  
}
