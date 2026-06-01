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
#include "GUIPlatform.h"
#include "JPEG.h"
#include "BMP.h"
#include "GUIButton.h"
#include "Malloc.h"
#include "Timer.h"
#include "stdio.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define TEXT_Y0       (605) //名字显示纵坐标起点
#define PROGRESS_Y0   (650) //进度条起始纵坐标
#define PROGRESS_X0   (80)  //进度条起始横坐标
#define PROGRESS_X1   (400) //进度条终止横坐标
#define PROGRESS_SIZE (6)   //进度条线条宽度（必须是双数）

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructGUIDev* s_pGUIDev = NULL;
static u16*          s_pTextBackground = NULL;
static u16*          s_pProgressBackground = NULL;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void DisplayBackground(void); //绘制背景

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
  //LCD竖屏显示
  LCDDisplayDir(LCD_SCREEN_VERTICAL);
  LCDClear(LCD_COLOR_CYAN);

  //绘制背景
  DisplayBackground();

  //创建按键
  InitGUIButton();

  //保存设备指针
  s_pGUIDev = dev;

  //为歌名/时间显示区域背景图片申请内存
  s_pTextBackground = MyMalloc(SDRAMEX, g_structTLILCDDev.width[LCD_LAYER_FOREGROUND] * 24 * 2);

  //保存歌名/时间显示区域背景
  if(NULL != s_pTextBackground)
  {
    GUISaveBackground(0, TEXT_Y0, g_structTLILCDDev.width[LCD_LAYER_FOREGROUND], 24, (u32)s_pTextBackground);
  }

  //为进度条背景图片申请内存
  s_pProgressBackground = MyMalloc(SDRAMEX, g_structTLILCDDev.width[LCD_LAYER_FOREGROUND] * (PROGRESS_SIZE + 2) * 2);

  //保存进度条背景
  if(NULL != s_pProgressBackground)
  {
    GUISaveBackground(0, PROGRESS_Y0 - PROGRESS_SIZE / 2, g_structTLILCDDev.width[LCD_LAYER_FOREGROUND], PROGRESS_SIZE + 2, (u32)s_pProgressBackground);
  }
}

/*********************************************************************************************************
* 函数名称：UpdataText
* 函数功能：更新名字/时间显示
* 输入参数：text：字符串
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void UpdataText(char* text)
{
  u32 len, max, total, begin;

  //计算名字
  len = 0;
  while((0 != text[len]) && ('\r' != text[len]) && ('\n' != text[len]))
  {
    len++;
  }

  //计算屏幕一行最多显示多少个字符
  max = g_structTLILCDDev.width[LCD_LAYER_FOREGROUND] / 12;

  //所需的像素点总数
  total = 12 * len;

  //计算起始显示像素点
  begin = (g_structTLILCDDev.width[LCD_LAYER_FOREGROUND] - total) / 2;

  //绘制背景图片
  if(NULL != s_pTextBackground)
  {
    GUIDrawBackground(0, TEXT_Y0, g_structTLILCDDev.width[LCD_LAYER_FOREGROUND], 24, (u32)s_pTextBackground);
  }
  
  //显示名字
  GUIDrawTextLine(begin, TEXT_Y0, (u32)text, GUI_FONT_ASCII_24, NULL, GUI_COLOR_BLACK, 1, max);
}

/*********************************************************************************************************
* 函数名称：ClearProgress
* 函数功能：清除进度条显示
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void ClearProgress(void)
{
  if(NULL != s_pProgressBackground)
  {
    GUIDrawBackground(0, PROGRESS_Y0 - PROGRESS_SIZE / 2, g_structTLILCDDev.width[LCD_LAYER_FOREGROUND], PROGRESS_SIZE + 2, (u32)s_pProgressBackground);
  }
}

/*********************************************************************************************************
* 函数名称：BeginShowProgress
* 函数功能：开始显示进度条
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void BeginShowProgress(void)
{
  GUIDrawLine(PROGRESS_X0, PROGRESS_Y0, PROGRESS_X1, PROGRESS_Y0, PROGRESS_SIZE, GUI_COLOR_WHITE, GUI_LINE_CIRCLE);
}

/*********************************************************************************************************
* 函数名称：UpdataSongProgress
* 函数功能：更新进度条显示
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：每隔100ms才会更新一次进度条显示
*********************************************************************************************************/
void UpdataProgress(u32 currentTime, u32 allTime)
{
  static u64 s_iLastTime    = 0;          //上一次更新的时间（ms）
  static u64 s_iCurrentTime = 0;          //当前时间（ms）
  u32 endPos;

  s_iCurrentTime = GetSysTime();
  if((s_iCurrentTime - s_iLastTime) > 100)
  {
    //保存系统时间
    s_iLastTime = s_iCurrentTime;

    //计算进度
    endPos = PROGRESS_X0 + (PROGRESS_X1 - PROGRESS_X0) * currentTime / allTime;

    //显示进度
    GUIDrawLine(PROGRESS_X0, PROGRESS_Y0, endPos, PROGRESS_Y0, PROGRESS_SIZE - 1, GUI_COLOR_ORANGE, GUI_LINE_CIRCLE);
  }
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
  EnumGUIButtons button;

  //按键扫描
  button = ScanGUIButton();
  if(GUI_BUTTON_NONE != button)
  {
    //播放
    if(GUI_BUTTON_PLAY == button)
    {
      if(NULL != s_pGUIDev->playCallback)
      {
        s_pGUIDev->playCallback();
      }
    }

    //录音
    if(GUI_BUTTON_RECORD == button)
    {
      if(NULL != s_pGUIDev->recordCallback)
      {
        s_pGUIDev->recordCallback();
      }
    }

    //暂停
    if(GUI_BUTTON_PAUSE == button)
    {
      if(NULL != s_pGUIDev->pauseCallback)
      {
        s_pGUIDev->pauseCallback();
      }
    }
  }
}
