/*********************************************************************************************************
* 模块名称：FatFsTest.c
* 摘    要：FatFs与读写SD卡实验模块
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
#include "FatFsTest.h"
#include "ReadBookByte.h"
#include "GUITop.h"
#include "GUIPlatform.h"
#include "stdio.h"
#include "ff.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define BOOK_X0     (6)    //显示区域x0
#define BOOK_Y0     (62)   //显示区域y0
#define BOOK_X1     (474)  //显示区域x1
#define BOOK_Y1     (734)  //显示区域y1
#define FONT_WIDTH  (12)   //显示字体宽度
#define FONT_HEIGHT (30)   //显示字体高度
#define MAX_LINE_NUM ((BOOK_Y1 - BOOK_Y0) / FONT_HEIGHT) //最大行数

//最大上一页数量
#define MAX_PREV_PAGE (64)

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructGUIDev s_structGUIDev;  //GUI设备结构体
static u32          s_iBackColor;    //背景颜色
static char         s_iEndFlag = 0;  //文本显示完全标志位
static char         s_iLastChar = 0; //上一个未显示出来的字符
static u32          s_arrPrevPosition[MAX_PREV_PAGE]; //上一页起始位置(0xFFFFFFFF表示无意义)

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
static void NewPage(void);      //显示新的一页
static void PreviousPage(void); //显示上一页
static void NextPage(void);     //下一页

/*********************************************************************************************************
* 函数名称：NewPage
* 函数功能：显示新的一页
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void NewPage(void)
{
  char newchar;                //当前字节
  u32  lineCnt;                //当前行位置
  u32  rowCnt;                 //当前列位置
  u32  x;                      //字符显示横坐标
  u32  y;                      //字符显示纵坐标
  u32  visibleLen;             //当前字符开始往后有多少个可视字符
  u32  newParaFlag;            //新段标志位（用于非段落开头空行不显示）

  //清除显示
  GUIFillColor(BOOK_X0, BOOK_Y0, BOOK_X1, BOOK_Y1, s_iBackColor);

  //显示上一个未打印出来的字符
  if((s_iLastChar >= ' ') && (s_iLastChar <= '~'))
  {
    rowCnt = 0;
    lineCnt = 0;
    x = BOOK_X0 + FONT_WIDTH * rowCnt;
    y = BOOK_Y0 + FONT_HEIGHT * lineCnt;
    GUIDrawChar(x, y, s_iLastChar, GUI_FONT_ASCII_24, NULL, GUI_COLOR_BLACK, 1);
    rowCnt = 1;
  }
  else
  {
    rowCnt = 0;
    lineCnt = 0;
  }

  //显示一整页内容
  newchar = 0;
  newParaFlag = 0;
  s_iLastChar = 0;
  while(1)
  {
    //从缓冲区中读取1字节数据
    if(0 == ReadBookByte(&newchar, &visibleLen))
    {
      s_iEndFlag = 1;
      return;
    }

    //回车换行符号
    if('\r' == newchar)
    {
      rowCnt = 0;
    }

    //换行
    else if('\n' == newchar)
    {
      rowCnt = 0;
      lineCnt = lineCnt + 1;
      if(lineCnt >= MAX_LINE_NUM)
      {
        return;
      }
      newParaFlag = 1;
    }

    //正常显示
    if((newchar >= ' ') && (newchar <= '~'))
    {
      //检查当前行是否足够显示整个单词
      if((newchar != ' ') && ((BOOK_X0 + FONT_WIDTH * (rowCnt + visibleLen)) > (BOOK_X1 - FONT_WIDTH)))
      {
        rowCnt = 0;
        lineCnt = lineCnt + 1;
        if(lineCnt >= MAX_LINE_NUM)
        {
          s_iLastChar = newchar;
          return;
        }
      }

      //非新段首行空格不显示
      if((0 == rowCnt) && (0 == newParaFlag) && (' ' == newchar))
      {
        continue;
      }

      x = BOOK_X0 + FONT_WIDTH * rowCnt;
      y = BOOK_Y0 + FONT_HEIGHT * lineCnt;
      GUIDrawChar(x, y, newchar, GUI_FONT_ASCII_24, NULL, GUI_COLOR_BLACK, 1);
    }

    //更新列计数
    rowCnt = rowCnt + 1;
    x = BOOK_X0 + FONT_WIDTH * rowCnt;
    if(x > (BOOK_X1 - FONT_WIDTH))
    {
      rowCnt = 0;
    }

    //更新行计数
    if(0 == rowCnt)
    {
      lineCnt = lineCnt + 1;
      if(lineCnt >= MAX_LINE_NUM)
      {
        return;
      }
    }

    //清除新段标志位
    newParaFlag = 0;
  }
}

/*********************************************************************************************************
* 函数名称：PreviousPage
* 函数功能：上一页
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void PreviousPage(void)
{
  u32  i; //循环变量

  //刷新上一页位置数据
  if(0xFFFFFFFF != s_arrPrevPosition[MAX_PREV_PAGE - 2])
  {
    for(i = (MAX_PREV_PAGE - 1); i > 0; i--)
    {
      s_arrPrevPosition[i] = s_arrPrevPosition[i - 1];
    }
    s_arrPrevPosition[0] = 0xFFFFFFFF;
  }

  //上一页有意义
  if(0xFFFFFFFF != s_arrPrevPosition[MAX_PREV_PAGE - 1])
  {
    //成功设置读写位置
    if(1 == SetPosition(s_arrPrevPosition[MAX_PREV_PAGE - 1]))
    {
      s_iEndFlag = 0;
      s_iLastChar = 0;

      //刷新新的一页
      NewPage();
    }
  }
}

/*********************************************************************************************************
* 函数名称：NextPage
* 函数功能：下一页
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void NextPage(void)
{
  u32  i; //循环变量

  //文本已全部显示，直接退出
  if(1 == s_iEndFlag)
  {
    return;
  }

  //保存上一页位置
  if(0xFFFFFFFF == s_arrPrevPosition[MAX_PREV_PAGE - 1])
  {
    s_arrPrevPosition[MAX_PREV_PAGE - 1] = GetBytePosition();
    s_arrPrevPosition[MAX_PREV_PAGE - 2] = GetBytePosition();
  }
  else
  {
    for(i = 0; i < (MAX_PREV_PAGE - 1); i++)
    {
      s_arrPrevPosition[i] = s_arrPrevPosition[i + 1];
    }
    s_arrPrevPosition[MAX_PREV_PAGE - 1] = GetBytePosition();
  }

  //刷新新的一页
  NewPage();
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitFatFsTest
* 函数功能：初始化FatFs与读写SD卡实验模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitFatFsTest(void)
{
  static FATFS   fs[2]; //文件系统设备结构体
  FRESULT result;       //文件操作返回变量
  DIR     direct;       //路径
  FILINFO fileInfo;     //文件信息
  u32     i;            //循环变量

  //初始化前一页位置储存区
  for(i = 0; i < MAX_PREV_PAGE; i++)
  {
    s_arrPrevPosition[i] = 0xFFFFFFFF;
  }

  //挂载文件系统
  result = f_mount(&fs[0], FS_VOLUME_SD, 1);
  if(result != FR_OK)
  {
    printf("挂载文件系统失败 (%d)\r\n", result);
  }
  else
  {
    printf("挂载文件系统成功\r\n");
  }

  //打开特定路径
  result = f_opendir(&direct, "0:/book");
  if(result != FR_OK)
  {
    printf("路径不存在\r\n");
  }
  else
  {
    printf("成功打开指定目录\r\n");
  }

  //查询目录下所有文件，并将文件名字打印输出到串口
  while(1)
  {
    result = f_readdir(&direct, &fileInfo);
    if((result != FR_OK) || (0 == fileInfo.fname[0]))
    {
      break;
    }
    else
    {
      printf("*/%s\r\n", fileInfo.fname);
    }
  }

  //关闭目录
  result = f_closedir(&direct);
  if(result != FR_OK)
  {
    printf("关闭目录失败\r\n");
  }
  
  //设置书名
  s_structGUIDev.bookName = "Sherlock Holmes";

  //设置前一页回调函数
  s_structGUIDev.previousPageCallback = PreviousPage;

  //设置下一页回调函数
  s_structGUIDev.nextPageCallback = NextPage;

  //初始化UI界面设计，绘制背景、创建按键和显示书名
  InitGUI(&s_structGUIDev);

  //获取阅读背景颜色（使用纯色背景）
  s_iBackColor = GUIReadPoint((BOOK_X0 + BOOK_X1) / 2, (BOOK_Y0 + BOOK_Y1) / 2);

  //先刷新一页出来
  NextPage();
}

/*********************************************************************************************************
* 函数名称：FatFsTask
* 函数功能：FatFs与读写SD卡实验模块任务
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：每隔20毫秒执行一次
*********************************************************************************************************/
void FatFsTask(void)
{
  GUITask(); //GUI任务
}

/*********************************************************************************************************
* 函数名称：CreatReadProgressFile
* 函数功能：创建保存阅读进度文件
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*          1、打开文件时文件变量FIL一定要是静态变量或动态内存分配的，否则会卡死，原因未知
*          1、FA_READ         ：指定对文件的读取权限。可以从文件中读取数据
*          2、FA_WRITE        ：指定对文件的写入访问权限。可以将数据写入该文件。与FA_READ组合以进行读写访问
*          3、FA_OPEN_EXISTING：打开一个文件。如果文件不存在，该函数将失败。（默认）
*          4、FA_CREATE_NEW   ：创建一个新文件。如果文件已存在，那么函数将失败并返回FR_EXIST
*          5、FA_CREATE_ALWAYS：创建一个新文件。如果文件已存在，那么它将被截断和覆盖
*          6、FA_OPEN_ALWAYS  ：打开文件（如果存在）。否则，将创建一个新文件
*          7、FA_OPEN_APPEND  ：与FA_OPEN_ALWAYS相同，只是读/写指针设置在文件末尾
*********************************************************************************************************/
void CreatReadProgressFile(void)
{
  static FIL s_fileProgressFile; //进度缓存文件
  DIR        progressDir;        //目标路径
  FRESULT    result;             //文件操作返回变量
  
  //校验进度缓存路径是否存在，若不存在则创建该路径
  result = f_opendir(&progressDir,"0:/book/progress");
  if(FR_NO_PATH == result)
  {
    f_mkdir("0:/book/progress");
  }
  else
  {
    f_closedir(&progressDir);
  }

  //检查文件是否存在，如不存在则创建
  result = f_open(&s_fileProgressFile, "0:/book/progress/progress.txt", FA_CREATE_NEW | FA_READ);
  if(FR_OK != result)
  {
    printf("CreatReadProgressFile：文件已存在\r\n");
  }
  else
  {
    printf("CreatReadProgressFile：创建文件成功\r\n");
    f_close(&s_fileProgressFile);
  }
}

/*********************************************************************************************************
* 函数名称：SaveReadProgress
* 函数功能：保存阅读进度
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*          1、打开文件时文件变量FIL一定要是静态变量或动态内存分配的，否则会卡死，原因未知
*          1、FA_READ         ：指定对文件的读取权限。可以从文件中读取数据
*          2、FA_WRITE        ：指定对文件的写入访问权限。可以将数据写入该文件。与FA_READ组合以进行读写访问
*          3、FA_OPEN_EXISTING：打开一个文件。如果文件不存在，该函数将失败。（默认）
*          4、FA_CREATE_NEW   ：创建一个新文件。如果文件已存在，那么函数将失败并返回FR_EXIST
*          5、FA_CREATE_ALWAYS：创建一个新文件。如果文件已存在，那么它将被截断和覆盖
*          6、FA_OPEN_ALWAYS  ：打开文件（如果存在）。否则，将创建一个新文件
*          7、FA_OPEN_APPEND  ：与FA_OPEN_ALWAYS相同，只是读/写指针设置在文件末尾
*********************************************************************************************************/
void SaveReadProgress(void)
{
  static FIL  s_fileProgressFile;  //进度缓存文件
  static char s_arrStringBuf[256]; //字符串转换缓冲区
  FRESULT     result;              //文件操作返回变量
  u32         len;                 //字符串长度
  u32         writeNum;            //成功写入的数量

  //打开文件
  result = f_open(&s_fileProgressFile, "0:/book/progress/progress.txt", FA_OPEN_EXISTING | FA_WRITE);
  if(FR_OK != result)
  {
    printf("SaveReadProgress：打开文件失败\r\n");
    return;
  }

  //字符串转换
  sprintf(s_arrStringBuf, "阅读进度:%d/%d", GetBytePosition(), GetBookSize());

  //统计字符串长度
  len = 0;
  while(0 != s_arrStringBuf[len])
  {
    len++;
  }

  //将进度写入指定文件
  result = f_write(&s_fileProgressFile, s_arrStringBuf, len, &writeNum);
  if((FR_OK == result) && (writeNum == len))
  {
    printf("SaveReadProgress：保存进度成功\r\n");
  }
  else
  {
    printf("SaveReadProgress：保存进度失败\r\n");
  }

  //关闭文件
  f_close(&s_fileProgressFile);
}

/*********************************************************************************************************
* 函数名称：DeleteReadProgress
* 函数功能：删除阅读进度
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void DeleteReadProgress(void)
{
  f_unlink("0:/book/progress/progress.txt");
  printf("DeleteReadProgress：删除成功\r\n");
}
