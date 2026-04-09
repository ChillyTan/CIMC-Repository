/*********************************************************************************************************
* 模块名称：ReadWriteSDCard.c
* 摘    要：读写SD卡模块
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
#include "ReadWriteSDCard.h"
#include "SDCard.h"
#include "gd32f470x_conf.h"
#include "GUITop.h"
#include "stdio.h"
#include "SysTick.h"
#include "LED.h"
#include "TLILCD.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define MAX_STRING_LEN 64 //显示字符最大长度

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructGUIDev s_structGUIDev;                      //GUI设备结构体
static u8   s_arrSDBuffer[2048];                         //SD卡读写缓冲区
static char s_arrStringBuff[MAX_STRING_LEN];             //字符串转换缓冲区
static sd_card_info_struct s_structSDCardInfo;           //SD卡信息
static sd_error_enum       s_enumSDCardStatus;           //SD卡在线检测

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static u8*  Read(u32 addr, u32 len);                     //按字节读取SD卡
static void Write(u32 addr, u8 data);                    //按字节写入SD卡
static void ReadSDCard(u32 addr, u32 len);               //读取SD卡
static void WriteSDCard(u32 addr, u8 data);              //写入SD卡
static void PrintSDInfo(sd_card_info_struct *pcardinfo); //打印SD卡信息
static void InitSDCard(void);                            //初始化SD卡

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：Read
* 函数功能：按字节读取SD卡
* 输入参数：addr：读取地址，len：读取长度（字节）
* 输出参数：void
* 返 回 值：读取缓冲区首地址
* 创建日期：2021年07月01日
* 注    意：SD卡读写以数据块的形式进行
*********************************************************************************************************/
static u8* Read(u32 addr, u32 len)
{
  u32 addrOffset; //目标地址与实际写入地址偏移量
  u8* buff;       //读取缓冲区
  u8* result;     //返回地址

  //查找数据块首地址
  addrOffset = 0;
  while(0 != (addr % s_structSDCardInfo.card_blocksize))
  {
    addr = addr - 1;
    addrOffset = addrOffset + 1;
  }

  //按数据块读入数据保存到数据缓冲区
  buff = s_arrSDBuffer;
  len = len + addrOffset;
  while(len > 0)
  {
    //计算剩余读取数据量
    if(len >= s_structSDCardInfo.card_blocksize)
    {
      len = len - s_structSDCardInfo.card_blocksize;
    }
    else
    {
      len = 0;
    }

    //读入一整个数据块
    sd_block_read((u32*)buff, addr, s_structSDCardInfo.card_blocksize);

    //设置读取地址为下一个数据块
    addr = addr + s_structSDCardInfo.card_blocksize;

    //设置下一个读取缓冲区地址
    buff = buff + s_structSDCardInfo.card_blocksize;
  }

  //计算返回地址
  result = s_arrSDBuffer + addrOffset;

  return result;
}

/*********************************************************************************************************
* 函数名称：WriteSDCard
* 函数功能：按字节写入SD卡
* 输入参数：addr：写入地址，data：写入数据
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：SD卡读写以数据块的形式进行
*********************************************************************************************************/
static void Write(u32 addr, u8 data)
{
  u32 addrOffset; //目标地址与实际写入地址偏移量

  //查找数据块首地址
  addrOffset = 0;
  while(0 != (addr % s_structSDCardInfo.card_blocksize))
  {
    addr = addr - 1;
    addrOffset = addrOffset + 1;
  }

  //读取一整个数据块
  sd_block_read((u32*)s_arrSDBuffer, addr, s_structSDCardInfo.card_blocksize);

  //修改数据块，将要写入的数据储存到数据块指定位置中
  s_arrSDBuffer[addrOffset] = data;

  //写入修改后的数据块
  sd_block_write((u32*)s_arrSDBuffer, addr, s_structSDCardInfo.card_blocksize);
}

/*********************************************************************************************************
* 函数名称：ReadSDCard
* 函数功能：读取SD卡
* 输入参数：addr：读取地址，len：读取长度（字节）
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ReadSDCard(u32 addr, u32 len)
{
  u32 i;    //循环变量
  u8* buff; //读取缓冲区
  u8  data; //读取到的数据

  //检查SD卡是否插入
  if(SD_OK == s_enumSDCardStatus)
  {
    //校验地址范围
    if((addr >= s_structGUIDev.beginAddr) && (addr + len - 1 <= s_structGUIDev.endAddr))
    {
      //输出读取信息到终端和串口
      sprintf(s_arrStringBuff, "Read : 0x%08X - 0x%02X\r\n", addr, len);
      s_structGUIDev.showLine(s_arrStringBuff);
      printf("%s", s_arrStringBuff);

      //从SD卡中读取数据
      buff = Read(addr, len);

      //打印到终端和串口上
      for(i = 0; i < len; i++)
      {
        //读取
        data = buff[i];

        //输出
        sprintf(s_arrStringBuff, "0x%08X: 0x%02X\r\n", addr + i, data);
        s_structGUIDev.showLine(s_arrStringBuff);
        printf("%s", s_arrStringBuff);
      }
    }
    else
    {
      //无效地址
      s_structGUIDev.showLine("Read: Invalid address\r\n");
      printf("Read: Invalid address\r\n");
    }
  }
  else
  {
    s_structGUIDev.showLine("Read: SD card not inserted\r\n");
    printf("Read: SD card not inserted\r\n");
  }
}

/*********************************************************************************************************
* 函数名称：WriteSDCard
* 函数功能：写入SD卡
* 输入参数：addr：写入地址，data：写入数据
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void WriteSDCard(u32 addr, u8 data)
{
  //检查SD卡是否插入
  if(SD_OK == s_enumSDCardStatus)
  {
    //校验地址范围
    if((addr >= s_structGUIDev.beginAddr) && (addr <= s_structGUIDev.endAddr))
    {
      //输出信息到终端和串口
      sprintf(s_arrStringBuff, "Write: 0x%08X - 0x%02X\r\n", addr, data);
      s_structGUIDev.showLine(s_arrStringBuff);
      printf("%s", s_arrStringBuff);

      //写入SD卡
      Write(addr, data);
    }
    else
    {
      //无效地址
      s_structGUIDev.showLine("Write: Invalid address\r\n");
      printf("Write: Invalid address\r\n");
    }
  }
  else
  {
    s_structGUIDev.showLine("Write: SD card not inserted\r\n");
    printf("Write: SD card not inserted\r\n");
  }
}

/*********************************************************************************************************
* 函数名称：PrintSDInfo
* 函数功能：打印SD卡信息
* 输入参数：pcardinfo：SD卡信息结构体首地址
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void PrintSDInfo(sd_card_info_struct *pcardinfo)
{
  switch(pcardinfo->card_type)
  {
  case SDIO_STD_CAPACITY_SD_CARD_V1_1: 
    printf("SD Card Type: standard capacity SD card version 1.1\r\n");
    break;
  case SDIO_STD_CAPACITY_SD_CARD_V2_0: 
    printf("SD Card Type: standard capacity SD card version 2.0\r\n");
    break;
  case SDIO_HIGH_CAPACITY_SD_CARD: 
    printf("SD Card Type: high capacity SD card\r\n");
    break;
  case SDIO_SECURE_DIGITAL_IO_CARD: 
    printf("SD Card Type: secure digital IO card\r\n");
    break;
  case SDIO_SECURE_DIGITAL_IO_COMBO_CARD: 
    printf("SD Card Type: secure digital IO combo card\r\n");
    break;
  case SDIO_MULTIMEDIA_CARD: 
    printf("SD Card Type: multimedia card\r\n");
    break;
  case SDIO_HIGH_CAPACITY_MULTIMEDIA_CARD: 
    printf("SD Card Type: high capacity multimedia card\r\n");
    break;
  case SDIO_HIGH_SPEED_MULTIMEDIA_CARD: 
    printf("SD Card Type: high speed multimedia card\r\n");
    break;
  default:
    break;
  }
  printf("SD Card RCA: %d\r\n",pcardinfo->card_rca);                           //卡相对地址
  printf("SD Card Capacity: %d MB\r\n",(u32)(pcardinfo->card_capacity >> 20)); //显示容量
  printf("SD Card BlockSize: %d\r\n",pcardinfo->card_blocksize);               //显示块大小
}

/*********************************************************************************************************
* 函数名称：InitSDCard
* 函数功能：初始化SD卡
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：要使能SDIO中断，否则DMA写入SD卡时会卡死
*********************************************************************************************************/
static void InitSDCard(void)
{
  u8  firstShowFlag;
  u32 cardstate;
  
  //初始化SD卡
  firstShowFlag = 1;
  while(SD_OK != sd_init())
  {
    //LCD输出提示信息
    if(1 == firstShowFlag)
    {
      firstShowFlag = 0;
      LCDDisplayDir(LCD_SCREEN_HORIZONTAL);
      LCDClear(LCD_COLOR_WHITE);
      LCDShowString(250, 200, 300, 30, LCD_FONT_24, LCD_TEXT_NORMAL, LCD_COLOR_MAGENTA, LCD_COLOR_WHITE, "Please insert SD card");
    }

    //串口输出提示信息
    printf("Failed to initialize SD card\r\n"); 

    //LED灯闪烁
    LEDCtrl(0, 0);
    DelayNms(100);
    LEDCtrl(0, 1);
    DelayNms(100);

    LEDCtrl(0, 0);
    DelayNms(100);
    LEDCtrl(0, 1);
    DelayNms(100);

    LEDCtrl(0, 0);
    DelayNms(1000);
  }
  printf("Initialize SD card successfully\r\n");
  
  //获取SD卡信息
  sd_card_information_get(&s_structSDCardInfo);

  //打印输出SD卡信息
  PrintSDInfo(&s_structSDCardInfo);

  //选中SD卡
  sd_card_select_deselect(s_structSDCardInfo.card_rca);

  //查看SD卡是否锁死
  sd_cardstatus_get(&cardstate);
  if(cardstate & 0x02000000)
  {
    LCDClear(LCD_COLOR_WHITE);
    LCDShowString(250, 200, 300, 30, LCD_FONT_24, LCD_TEXT_NORMAL, LCD_COLOR_MAGENTA, LCD_COLOR_WHITE, "SD card is locked!");
    printf("SD card is locked!\r\n");
    while(1)
    {
      //LED灯闪烁
      LEDCtrl(0, 0);
      DelayNms(100);
      LEDCtrl(0, 1);
      DelayNms(100);

      LEDCtrl(0, 0);
      DelayNms(100);
      LEDCtrl(0, 1);
      DelayNms(100);

      LEDCtrl(0, 0);
      DelayNms(1000);
    }
  }

  //切换SD卡到4线模式
  sd_bus_mode_config(SDIO_BUSMODE_4BIT);

  //使用DMA传输
  sd_transfer_mode_config(SD_DMA_MODE);

  //使能SDIO中断
  nvic_irq_enable(SDIO_IRQn, 0, 0);
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitReadWriteSDCard
* 函数功能：初始化读写SD卡模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitReadWriteSDCard(void)
{
  //初始化SD卡
  InitSDCard();

  //SD卡首地址
  s_structGUIDev.beginAddr = 0;

  //SD卡结束地址
  s_structGUIDev.endAddr = s_structGUIDev.beginAddr + s_structSDCardInfo.card_capacity - 1;

  //设置写入回调函数
  s_structGUIDev.writeCallback = WriteSDCard;

  //设置读取回调函数
  s_structGUIDev.readCallback = ReadSDCard;

  //初始化UI界面设计
  InitGUI(&s_structGUIDev);

  //打印地址范围到终端和串口
  sprintf(s_arrStringBuff, "Addr: 0x%08X - 0x%08X\r\n", s_structGUIDev.beginAddr, s_structGUIDev.endAddr);
  s_structGUIDev.showLine(s_arrStringBuff);
  printf("%s", s_arrStringBuff);
}

/*********************************************************************************************************
* 函数名称：ReadWriteSDCardTask
* 函数功能：读写SD卡模块任务
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：每隔40毫秒执行一次
*********************************************************************************************************/
void ReadWriteSDCardTask(void)
{
  static u8 s_iTimeCnt = 0;
  sd_error_enum status;

  //每隔1s检查一遍SD卡是否存在
  s_iTimeCnt++;
  if(s_iTimeCnt >= 25)
  {
    //计时器清零
    s_iTimeCnt = 0;

    //SD卡插入检测
    status = sd_detect();

    //检测到SD卡插入则重新初始化SD卡
    if((SD_ERROR == s_enumSDCardStatus) && (SD_OK == status))
    {
      //等待SD卡插入稳定
      DelayNms(1000);

      //输出提示信息到串口和终端
      printf("Reinitialize the SD card\r\n");
      s_structGUIDev.showLine("Reinitialize the SD card\r\n");

      //重新初始化
      InitSDCard();
    }
    s_enumSDCardStatus = status;
  }

  GUITask(); //GUI任务
}

