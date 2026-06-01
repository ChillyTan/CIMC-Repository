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
#include "GUIButton.h"
#include "GUIGraph.h"
#include "GUIPlatform.h"
#include "TextWidget.h"
#include "stdio.h"

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
static StructTextWidget s_structInUsage;
static StructTextWidget s_structInFree;
static StructTextWidget s_structExUsase;
static StructTextWidget s_structExFree;
static u16 s_arrInUsageBackgound[1728];
static u16 s_arrInFreeBackgound[1728];
static u16 s_arrExUsageBackgound[1728];
static u16 s_arrExFreeBackgound[1728];

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void DisplayBackground(void);               //绘制背景
static void UpdataInSRAMText(u32 usage, u32 free); //更新内部内存池使用信息显示
static void UpdataExSDRAMText(u32 usage, u32 free); //更新外部内存池使用信息显示

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
* 函数名称：UpdataInSRAMText
* 函数功能：更新内部内存池使用信息显示
* 输入参数：usage：使用量（字节），free：剩余量（字节）
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：最终显示的是千字节
*********************************************************************************************************/
static void UpdataInSRAMText(u32 usage, u32 free)
{
  //字符串转换缓冲区
  char string[16];

  //使用量显示
  sprintf(string, "%dKB", usage / 1024);
  s_structInUsage.setText(&s_structInUsage, string);

  //剩余量显示
  sprintf(string, "%dKB", free / 1024);
  s_structInFree.setText(&s_structInFree, string);
}

/*********************************************************************************************************
* 函数名称：UpdataExSRAMText
* 函数功能：更新外部内存池使用信息显示
* 输入参数：usage：使用量（字节），free：剩余量（字节）
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：最终显示的是千字节
*********************************************************************************************************/
static void UpdataExSDRAMText(u32 usage, u32 free)
{
  //字符串转换缓冲区
  char string[16];

  //使用量显示
  sprintf(string, "%dMB", usage / 1024 / 1024);
  s_structExUsase.setText(&s_structExUsase, string);

  //剩余量显示
  sprintf(string, "%dMB", free / 1024 / 1024);
  s_structExFree.setText(&s_structExFree, string);
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

  //LCD横屏显示
  LCDDisplayDir(LCD_SCREEN_HORIZONTAL);
  LCDClear(LCD_COLOR_WHITE);

  //绘制背景
  DisplayBackground();

  //创建按键
  InitGUIButton();

  //创建动态内存波形显示模块
  CreateGUIGraph();

  //连接函数指针
  dev->addMemoryWave = GUIGraphAddrPoint;
  dev->updateInSRAMInfo = UpdataInSRAMText;
  dev->updateExSDRAMInfo = UpdataExSDRAMText;

  /*
  *InSRAM
  */
  //标题
  GUIDrawTextLine(50, 40 , (u32)"InSRAM", GUI_FONT_ASCII_24, NULL, GUI_COLOR_WHITE, 1, 15);

  //使用量
  GUIDrawTextLine(50, 64 , (u32)"Usage: ", GUI_FONT_ASCII_24, NULL, GUI_COLOR_WHITE, 1, 15);
  InitTextWidgetStruct(&s_structInUsage);
  s_structInUsage.x0 = 134;
  s_structInUsage.y0 = 64;
  s_structInUsage.width = 72;
  s_structInUsage.background = (u32)s_arrInUsageBackgound;
  CreateTextWidget(&s_structInUsage);
  s_structInUsage.setText(&s_structInUsage, "0KB");

  //剩余量
  GUIDrawTextLine(50, 88 , (u32)"Free : ", GUI_FONT_ASCII_24, NULL, GUI_COLOR_WHITE, 1, 15);
  InitTextWidgetStruct(&s_structInFree);
  s_structInFree.x0 = s_structInUsage.x0;
  s_structInFree.y0 = 88;
  s_structInFree.width = s_structInUsage.width;
  s_structInFree.background = (u32)s_arrInFreeBackgound;
  CreateTextWidget(&s_structInFree);
  sprintf(string, "%dKB", dev->inMallocSize / 1024);
  s_structInFree.setText(&s_structInFree, string);

  //总量
  sprintf(string, "Total: %dKB", dev->inMallocSize / 1024);
  GUIDrawTextLine(50, 112, (u32)string, GUI_FONT_ASCII_24, NULL, GUI_COLOR_WHITE, 1, 15);

  /*
  *ExSRAM
  */
 //标题
  GUIDrawTextLine(425, 40 , (u32)"ExSDRAM", GUI_FONT_ASCII_24, NULL, GUI_COLOR_WHITE, 1, 15);

  //使用量
  GUIDrawTextLine(425, 64 , (u32)"Usage: ", GUI_FONT_ASCII_24, NULL, GUI_COLOR_WHITE, 1, 15);
  InitTextWidgetStruct(&s_structExUsase);
  s_structExUsase.x0 = 509;
  s_structExUsase.y0 = 64;
  s_structExUsase.width = 84;
  s_structExUsase.background = (u32)s_arrExUsageBackgound;
  CreateTextWidget(&s_structExUsase);
  s_structExUsase.setText(&s_structExUsase, "0MB");

  //剩余量
  GUIDrawTextLine(425, 88 , (u32)"Free : ", GUI_FONT_ASCII_24, NULL, GUI_COLOR_WHITE, 1, 15);
  InitTextWidgetStruct(&s_structExFree);
  s_structExFree.x0 = s_structExUsase.x0;
  s_structExFree.y0 = 88;
  s_structExFree.width = 84;
  s_structExFree.background = (u32)s_arrExFreeBackgound;
  CreateTextWidget(&s_structExFree);
  sprintf(string, "%dMB", dev->exMallocSize / 1024 / 1024);
  s_structExFree.setText(&s_structExFree, string);

  //总量
  sprintf(string, "Total: %dMB", dev->exMallocSize / 1024 / 1024);
  GUIDrawTextLine(425, 112, (u32)string, GUI_FONT_ASCII_24, NULL, GUI_COLOR_WHITE, 1, 15);

  //保存设备指针
  s_pGUIDev = dev;
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
  static u64 s_iLastTime    = 0; //上次波形刷新时间（ms）
  static u64 s_iCurrentTime = 0; //当前时间（ms）
  EnumGUIButtons button;

  //按键扫描
  button = ScanGUIButton();
  if(GUI_BUTTON_NONE != button)
  {
    if(GUI_BUTTON_MALLOC_INSRAM == button)
    {
      if(NULL != s_pGUIDev->mallocInButtonCallback)
      {
        s_pGUIDev->mallocInButtonCallback();
      }
    }
    else if(GUI_BUTTON_MALLOC_EXSRAM == button)
    {
      if(NULL != s_pGUIDev->mallocExButtonCallback)
      {
        s_pGUIDev->mallocExButtonCallback();
      }
    }
    else if(GUI_BUTTON_FREE == button)
    {
      if(NULL != s_pGUIDev->freeButtonCallback)
      {
        s_pGUIDev->freeButtonCallback();
      }
    }
  }

  //波形刷新
  s_iCurrentTime = GUIGetSysTime();
  if((s_iCurrentTime - s_iLastTime) > 1000)
  {
    s_iLastTime = s_iCurrentTime;
    GUIGraphRefresh();
  }
}
