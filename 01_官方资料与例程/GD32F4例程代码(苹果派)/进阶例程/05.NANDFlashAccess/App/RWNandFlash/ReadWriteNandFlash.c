/*********************************************************************************************************
* 模块名称：ReadWriteNandFlash.c
* 摘    要：读写NandFlash模块
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
#include "ReadWriteNandFlash.h"
#include "gd32f470x_conf.h"
#include "GUITop.h"
#include "stdio.h"
#include "FTL.h"

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
static StructGUIDev s_structGUIDev;          //GUI设备结构体
static u8   s_arrWRBuff[2048];                //读写缓冲区
static char s_arrStringBuff[MAX_STRING_LEN]; //字符串转换缓冲区

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ReadProc(u32 addr, u32 len);   //读取操作处理
static void WriteProc(u32 addr, u8 data);  //写操作处理

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ReadProc
* 函数功能：读取操作处理
* 输入参数：addr：读取地址，len：读取长度（字节）
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ReadProc(u32 addr, u32 len)
{
  u32 i;    //循环变量
  u32 data; //读取到的数据

  //校验地址范围
  if((addr >= s_structGUIDev.beginAddr) && ((addr + len - 1) <= s_structGUIDev.endAddr))
  {
    //输出读取信息到终端和串口
    sprintf(s_arrStringBuff, "Read : 0x%08X - 0x%02X\r\n", addr, len);
    s_structGUIDev.showLine(s_arrStringBuff);
    printf("%s", s_arrStringBuff);

    //从NandFlash中读取数据
    FTLReadSectors(s_arrWRBuff, addr / FTL_SECTOR_SIZE, FTL_SECTOR_SIZE, 1);

    //打印到终端和串口上
    for(i = 0; i < len; i++)
    {
      //防止数组下标溢出
      if(((addr % FTL_SECTOR_SIZE) + i) > sizeof(s_arrWRBuff))
      {
        return;
      }

      //读取
      data = s_arrWRBuff[(addr % FTL_SECTOR_SIZE) + i];

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

/*********************************************************************************************************
* 函数名称：WriteProc
* 函数功能：写操作处理
* 输入参数：addr：写入地址，data：写入数据
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void WriteProc(u32 addr, u8 data)
{
  //校验地址范围
  if((addr >= s_structGUIDev.beginAddr) && (addr <= s_structGUIDev.endAddr))
  {
    //输出信息到终端和串口
    sprintf(s_arrStringBuff, "Write: 0x%08X - 0x%02X\r\n", addr, data);
    s_structGUIDev.showLine(s_arrStringBuff);
    printf("%s", s_arrStringBuff);

    //读入NandFlash数据
    FTLReadSectors(s_arrWRBuff, addr / FTL_SECTOR_SIZE, FTL_SECTOR_SIZE, 1);

    //修改
    s_arrWRBuff[addr % FTL_SECTOR_SIZE] = data;

    //写入NandFlash
    FTLWriteSectors(s_arrWRBuff, addr / FTL_SECTOR_SIZE, FTL_SECTOR_SIZE, 1);
  }
  else
  {
    //无效地址
    s_structGUIDev.showLine("Write: Invalid address\r\n");
    printf("Write: Invalid address\r\n");
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitReadWriteNandFlash
* 函数功能：初始化读写NandFlash模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitReadWriteNandFlash(void)
{
  //读写首地址
  s_structGUIDev.beginAddr = 0;

  //读写结束地址
  s_structGUIDev.endAddr = 0 + NAND_MAX_ADDRESS - 1;

  //设置写入回调函数
  s_structGUIDev.writeCallback = WriteProc;

  //设置读取回调函数
  s_structGUIDev.readCallback = ReadProc;

  //初始化UI界面设计
  InitGUI(&s_structGUIDev);

  //打印地址范围到终端和串口
  sprintf(s_arrStringBuff, "Addr: 0x%08X - 0x%08X\r\n", s_structGUIDev.beginAddr, s_structGUIDev.endAddr);
  s_structGUIDev.showLine(s_arrStringBuff);
  printf("%s", s_arrStringBuff);
}

/*********************************************************************************************************
* 函数名称：ReadWriteNandFlashTask
* 函数功能：读写NandFlash模块任务
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：每隔40毫秒执行一次
*********************************************************************************************************/
void ReadWriteNandFlashTask(void)
{
  GUITask(); //GUI任务
}
