/*********************************************************************************************************
* 模块名称：EnternetTop.h
* 摘    要：以太网通信实验顶层模块
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
#include "EnternetTop.h"
#include "GUITop.h"
#include "TLILCD.h"
#include "SysTick.h"
#include "Timer.h"
#include "stdio.h"
#include "LWIPComm.h"
#include "TCPServerBase.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define BUFFER_SIZE    4096 //内部SRAM缓冲区大小
#define MAX_STRING_LEN 64   //显示字符最大长度

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructGUIDev s_structGUIDev;          //GUI设备结构体
static char s_arrStringBuff[MAX_STRING_LEN]; //字符串转换缓冲区

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ReadClientData(void);                   //读取客户端数据并打印输出到终端
static void SendDataToClient(char* ip, char* data); //按字节写入内部SRAM

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/

/*********************************************************************************************************
* 函数名称：ReadClientData
* 函数功能：读取客户端数据并打印输出到终端
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ReadClientData(void)
{
  static char s_arrReadBuf[64]; //读取缓冲区
  u32 readCnt;                  //读取到的数据量
  u32 i;                        //循环变量
  u32 ipAddr;                   //客户端IP地址
  u8  ip[4];                    //客户端IP地址拆解
  u64 sysTime;                  //系统运行时间（ms）
  u32 hour, minute, second;     //系统运行时间（小时、分钟、秒）

  //循环检查每个客户端是否有发送数据到服务器
  for(i = 0; i < TCP_SERVER_CLIENT_NUM; i++)
  {
    //获取客户端IP地址
    ipAddr = GetClientIP((EnumTCPClientList)i);

    //一次性读出客户端所有数据数据，最大读取量为缓冲区长度-1
    readCnt = ReadTCPServer(i, (u8*)s_arrReadBuf, (sizeof(s_arrReadBuf) / sizeof(u8)) - 1);

    //IP地址不为零，表示有客户端接上，readCnt大于零则表示成功读取到数据
    if(0 != ipAddr && readCnt > 0)
    {
      //获取系统时间
      sysTime = GetSysTime();

      //计算秒
      sysTime = sysTime / 1000;
      second = sysTime % 60;

      //计算分钟
      sysTime = sysTime / 60;
      minute = sysTime % 60;

      //计算小时
      sysTime = sysTime / 60;
      hour = sysTime;

      //标记字符串结尾
      s_arrReadBuf[readCnt] = 0;

      //终端输出一个空行
      s_structGUIDev.showLine("");

      //打印客户端IP地址到终端
      ip[0] = (ipAddr >> 0 ) & 0xff; //IADDR4
      ip[1] = (ipAddr >> 8 ) & 0xff; //IADDR3
      ip[2] = (ipAddr >> 16) & 0xff; //IADDR2
      ip[3] = (ipAddr >> 24) & 0xff; //IADDR1
      sprintf(s_arrStringBuff, "<receive ip: %d.%d.%d.%d  %02d:%02d:%02d>", ip[0], ip[1], ip[2], ip[3], hour, minute, second);
      s_structGUIDev.showLine(s_arrStringBuff);

      //打印客户端数据到终端
      s_structGUIDev.showLine(s_arrReadBuf);
    }
  }
}

/*********************************************************************************************************
* 函数名称：SendDataToClient
* 函数功能：发送数据给指定客户端
* 输入参数：ip：客户端IP地址，data：要发送的数据
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void SendDataToClient(char* ip, char* data)
{
  u8  sum;      //和，解析IP地址时使用
  u8  i;        //循环变量，解析IP地址时使用
  u8  strCnt;   //字符计数，解析IP地址时使用
  u8  ipBuf[4]; //IP地址（拆分形式）
  u32 ipAddr;   //IP地址（合并形式）
  u32 sendNum;  //需要发送的数据量
  u32 ret;      //发送数据返回值，表示成功发送的数据量
  u64 sysTime;  //系统运行时间（ms）
  u32 hour, minute, second; //系统运行时间（小时、分钟、秒）

  //获取系统时间
  sysTime = GetSysTime();

  //计算秒
  sysTime = sysTime / 1000;
  second = sysTime % 60;

  //计算分钟
  sysTime = sysTime / 60;
  minute = sysTime % 60;

  //计算小时
  sysTime = sysTime / 60;
  hour = sysTime;

  //解析出IP地址
  strCnt = 0;
  for(i = 0; i < 4; i++)
  {
    sum = 0;
    while((ip[strCnt] >= '0') && (ip[strCnt] <= '9'))
    {
      sum = sum * 10 + (ip[strCnt] - 48);
      strCnt++;
    }
    strCnt++;

    ipBuf[i] = sum;
  }
  ipAddr = (ipBuf[0] << 0) | (ipBuf[1] << 8) | (ipBuf[2] << 16) | (ipBuf[3] << 24);

  //统计发送数据量
  sendNum = 0;
  while(0 != data[sendNum])
  {
    sendNum++;
  }

  //将数据通过TCP服务器发送至客户端
  ret = WriteTCPServer(ipAddr, (u8*)data, sendNum);

  //输出一个空行
  s_structGUIDev.showLine("");

  //输出时间信息
  sprintf(s_arrStringBuff, "<send ip: %d.%d.%d.%d  %02d:%02d:%02d>", ipBuf[0], ipBuf[1], ipBuf[2], ipBuf[3], hour, minute, second);
  s_structGUIDev.showLine(s_arrStringBuff);

  if(0 != ret)
  {
    //发送成功
    s_structGUIDev.showLine(data);
  }
  else
  {
    //发送失败
    s_structGUIDev.showLine("Failed to send");
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitEnternet
* 函数功能：初始化以太网通信模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitEnternet(void)
{
  char s_arrServerIP[64];   //服务器IP字符串转换缓冲区
  char s_arrServerPort[64]; //服务器端口字符串转换缓冲区
  StructLWIPDev* lwipDev;   //LWIP设备结构体指针，用于获取服务器IP地址

  //LCD横屏显示
  LCDDisplayDir(LCD_SCREEN_HORIZONTAL);
  LCDClear(LCD_COLOR_WHITE);

  //获取LWIP设备结构体地址，用于打印TCP服务器IP地址
  lwipDev = GetLWIPCommDev();

  //LWIP初始化
	LCDShowString(250, 200, 300, 30, LCD_FONT_24, LCD_TEXT_TRANS, LCD_COLOR_RED, NULL, "Initializing LWIP");
  while(LWIPCommInit())
  {
    LCDClear(LCD_COLOR_WHITE);

		LCDShowString(250, 200, 300, 30, LCD_FONT_24, LCD_TEXT_TRANS, LCD_COLOR_RED, NULL, "LWIP Init Falied!");
    DelayNms(1200);
    LCDClear(LCD_COLOR_WHITE);
		LCDShowString(250, 200, 300, 30, LCD_FONT_24, LCD_TEXT_TRANS, LCD_COLOR_RED, NULL, "Retrying...");
  }
  LCDClear(LCD_COLOR_WHITE);
	LCDShowString(250, 200, 300, 30, LCD_FONT_24, LCD_TEXT_TRANS, LCD_COLOR_RED, NULL, "LWIP Init Success!");
  DelayNms(500);

#if LWIP_DHCP   //使用DHCP（自动获取本机IP地址）
  LCDClear(LCD_COLOR_WHITE);
	LCDShowString(250, 200, 300, 30, LCD_FONT_24, LCD_TEXT_TRANS, LCD_COLOR_RED, NULL, "DHCP IP configing...");

  //等待DHCP获取成功/超时溢出
  while((lwipDev->dhcpstatus!=2)&&(lwipDev->dhcpstatus!=0XFF))
  {
    LWIPPeriodicHandle();
    LWIPReadPackHandle();
  }
#endif

  //创建TCP服务器
  CreateTCPServer();

  //设置服务器IP地址显示
  sprintf(s_arrServerIP, "Server IP: %d.%d.%d.%d", lwipDev->ip[0], lwipDev->ip[1], lwipDev->ip[2], lwipDev->ip[3]);
  s_structGUIDev.serverIP = s_arrServerIP;

  //设置服务器端口号显示
  sprintf(s_arrServerPort, "Server Port: %d", TCP_SERVER_PORT);
  s_structGUIDev.serverPort = s_arrServerPort;

  //设置发送回调函数
  s_structGUIDev.sendCallback = SendDataToClient;

  //初始化UI界面设计
  InitGUI(&s_structGUIDev);
}

/*********************************************************************************************************
* 函数名称：EnternetTask
* 函数功能：以太网通信模块任务
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：每隔20毫秒执行一次
*********************************************************************************************************/
void EnternetTask(void)
{
  //GUI轮询任务
  GUITask();

  //LWIP轮询任务
  LWIPPeriodicHandle();

  //LWIP接收数据包轮询任务
  LWIPReadPackHandle();

  //接收客户端数据并输出打印到终端
  ReadClientData();
}
