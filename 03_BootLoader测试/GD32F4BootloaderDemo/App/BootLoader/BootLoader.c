/*********************************************************************************************************
* 模块名称：BootLoader.c
* 摘    要：BootLoader模块，实现基本的BootLoader功能
* 当前版本：1.0.0
* 作    者：SZLY(COPYRIGHT 2018 - 2020 SZLY. All rights reserved.)
* 完成日期：2020年01月01日 
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
#include "BootLoader.h"
#include "gd32f470x_conf.h"
#include "DataType.h"
#include "UART0.h"
#include "RCU.h"
#include "NVIC.h"
#include "SysTick.h"
#include "Timer.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define BOOTLOADER_TIMEOUT    5000    // Bootloader等待时间(ms)，若超时则跳转到用户程序
#define APPLICATION_START_ADDR 0x08004000  // 用户应用程序起始地址
#define FLASH_PAGE_SIZE       0x400   // Flash页大小(1KB)

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/
// Bootloader命令枚举
typedef enum
{
  CMD_SYNC = 0x00,          // 同步命令
  CMD_GET_VERSION = 0x01,   // 获取版本信息
  CMD_READ_MEMORY = 0x11,   // 读内存
  CMD_GO = 0x21,            // 跳转命令
  CMD_WRITE_MEMORY = 0x31,  // 写内存
  CMD_ERASE = 0x43,         // 擦除Flash
  CMD_GET_ID = 0x02,        // 获取芯片ID
  CMD_ERROR = 0xFF          // 错误命令
} EnumBootCmd;

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static u32 s_u32BootTimeoutCount = 0;    // Bootloader超时计数
static u8 s_u8BootloaderActive = TRUE;   // Bootloader活动标志

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void BootloaderWaitForSync(void);     // 等待同步信号
static u8 BootloaderProcessCommand(void);    // 处理Bootloader命令
static void BootloaderJumpToApp(void);       // 跳转到应用程序
static void BootloaderSendAck(u8 ack);       // 发送应答
static void BootloaderSendByte(u8 byte);     // 发送一个字节

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：BootloaderWaitForSync
* 函数功能：等待同步信号 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void BootloaderWaitForSync(void)
{
  u8 syncByte = 0;
  
  // 发送同步字符
  BootloaderSendByte(0x79);  // 正确同步响应
  
  // 等待主机发送同步请求
  s_u32BootTimeoutCount = 0;
  while(s_u32BootTimeoutCount < BOOTLOADER_TIMEOUT)
  {
    if(Get1SecFlag())
    {
      s_u32BootTimeoutCount += 1000;  // 每秒增加1000ms
      Clr1SecFlag();
    }
    
    // 检查是否有数据到达
    if(ReadUART1(&syncByte, 1) > 0)
    {
      if(syncByte == 0x7F || syncByte == 0x00)  // 同步字符
      {
        BootloaderSendByte(0x79);  // 发送正确同步响应
        return;
      }
      else
      {
        BootloaderSendByte(0x1F);  // 发送错误同步响应
      }
    }
  }
  
  // 超时，尝试跳转到应用程序
  s_u8BootloaderActive = FALSE;
}

/*********************************************************************************************************
* 函数名称：BootloaderProcessCommand
* 函数功能：处理Bootloader命令 
* 输入参数：void
* 输出参数：void
* 返 回 值：u8 - TRUE表示继续运行Bootloader，FALSE表示退出Bootloader
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static u8 BootloaderProcessCommand(void)
{
  u8 cmd = 0;
  u8 checksum = 0;
  u8 receivedChecksum = 0;
  u8 numBytes = 0;
  u8 addrBytes[4];
  u8 len;
  u8 readChecksum = 0;
  u32 readAddr;
  u8 dataLen;
  u8 dataWithChecksum[256 + 1];
  u32 targetAddr;
  FLASH_Status flashStatus = FLASH_COMPLETE;
  u8 eraseData[256];
  u8 eraseChecksum = 0;
  u8 pageNum = 0;
  u8 goChecksum = 0;
  u32 wordData;
  u32 pageAddr;
  u8 i;
  u8 dataByte;
  
  // 接收命令字节
  if(ReadUART1(&cmd, 1) == 0)
  {
    return TRUE;  // 没有接收到命令，继续等待
  }
  
  // 计算校验和（命令与其反码的异或应该为0xFF）
  if(ReadUART1(&receivedChecksum, 1) == 0)
  {
    BootloaderSendAck(0x1F);  // 发送错误应答
    return TRUE;
  }
  
  checksum = cmd ^ receivedChecksum;
  if(checksum != 0xFF)
  {
    BootloaderSendAck(0x1F);  // 校验和错误
    return TRUE;
  }
  
  switch(cmd)
  {
    case CMD_SYNC:
      BootloaderSendAck(0x79);  // 正确应答
      break;
      
    case CMD_GET_VERSION:
      // 发送版本信息: 版本号 + 选项字节支持标志 + 结束标志
      BootloaderSendByte(0x01);  // 版本号
      BootloaderSendByte(0x00);  // 选项字节不支持
      BootloaderSendByte(0x00);  // 结束标志
      BootloaderSendAck(0x79);   // 正确应答
      break;
      
    case CMD_GET_ID:
      // 发送芯片ID (STM32F103xx: 0x414)
      BootloaderSendByte(0x02);  // 返回2个字节
      BootloaderSendByte(0x41);  // 高字节
      BootloaderSendByte(0x44);  // 低字节 (STM32F103RCT6)
      BootloaderSendAck(0x79);   // 正确应答
      break;
      
    case CMD_READ_MEMORY:
      // 读取地址（4字节）
      if(ReadUART1(addrBytes, 4) != 4)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 读取地址校验和
      if(ReadUART1(&readChecksum, 1) != 1)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 验证地址校验和
      readChecksum ^= addrBytes[0] ^ addrBytes[1] ^ addrBytes[2] ^ addrBytes[3];
      if(readChecksum != 0xFF)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 读取长度
      if(ReadUART1(&len, 1) != 1)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 读取长度校验和
      if(ReadUART1(&readChecksum, 1) != 1)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 验证长度校验和
      readChecksum ^= len;
      if(readChecksum != 0xFF)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 构建读取地址
      readAddr = (addrBytes[0] << 24) | (addrBytes[1] << 16) | (addrBytes[2] << 8) | addrBytes[3];
      
      // 发送长度
      WriteUART1(&len, 1);
      
      // 发送数据
      for(i = 0; i < len; i++)
      {
        dataByte = *((u8*)(readAddr + i));
        WriteUART1(&dataByte, 1);
      }
      
      BootloaderSendAck(0x79);  // 正确应答
      break;
      
    case CMD_GO:
      // 接收地址和校验和
      if(ReadUART1(addrBytes, 4) != 4) 
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      if(ReadUART1(&goChecksum, 1) != 1)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 验证校验和
      goChecksum ^= addrBytes[0];
      goChecksum ^= addrBytes[1];
      goChecksum ^= addrBytes[2];
      goChecksum ^= addrBytes[3];
      
      if(goChecksum != 0xFF)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 跳转到指定地址
      BootloaderSendAck(0x79);  // 正确应答
      // 这里只是模拟，实际跳转会在命令处理结束后进行
      return FALSE;  // 退出Bootloader
      
    case CMD_WRITE_MEMORY:
      // 读取目标地址（4字节）
      if(ReadUART1(addrBytes, 4) != 4)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 读取地址校验和
      if(ReadUART1(&checksum, 1) != 1)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 验证地址校验和
      checksum ^= addrBytes[0] ^ addrBytes[1] ^ addrBytes[2] ^ addrBytes[3];
      if(checksum != 0xFF)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 构建目标地址
      targetAddr = (addrBytes[0] << 24) | (addrBytes[1] << 16) | (addrBytes[2] << 8) | addrBytes[3];
      
      // 验证目标地址是否在应用程序区域内
      if(targetAddr < APPLICATION_START_ADDR || targetAddr >= 0x08020000) // 假设最大128KB应用区域
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 读取数据长度（减1，因为长度字节本身也要计算）
      if(ReadUART1(&dataLen, 1) != 1)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 读取数据和校验和
      if(ReadUART1(dataWithChecksum, dataLen + 1) != (dataLen + 1))
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 验证数据校验和
      checksum = dataLen;
      for(i = 0; i <= dataLen; i++)
      {
        checksum ^= dataWithChecksum[i];
      }
      
      if(checksum != 0xFF)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 解锁Flash
      FLASH_Unlock();
      
      // 编程数据到Flash - 按字（4字节）写入
      for(i = 0; i <= dataLen; i += 4)
      {
        // 组装32位字 - 处理不足4字节的情况
        if((i + 3) <= dataLen)
        {
          wordData = (dataWithChecksum[i] << 24) | (dataWithChecksum[i+1] << 16) | 
                    (dataWithChecksum[i+2] << 8) | dataWithChecksum[i+3];
        }
        else
        {
          wordData = 0xFFFFFFFF; // 默认值
          if(i <= dataLen) wordData = (wordData & 0x00FFFFFF) | ((u32)dataWithChecksum[i] << 24);
          if((i+1) <= dataLen) wordData = (wordData & 0xFF00FFFF) | ((u32)dataWithChecksum[i+1] << 16);
          if((i+2) <= dataLen) wordData = (wordData & 0xFFFF00FF) | ((u32)dataWithChecksum[i+2] << 8);
        }
        
        // 写入Flash
        flashStatus = FLASH_ProgramWord(targetAddr + i, wordData);
        if(flashStatus != FLASH_COMPLETE)
        {
          FLASH_Lock();
          BootloaderSendAck(0x1F);
          return TRUE;
        }
      }
      
      // 上锁Flash
      FLASH_Lock();
      
      BootloaderSendAck(0x79);  // 正确应答
      break;
      
    case CMD_ERASE:
      // 读取页数（减1，因为页数本身也要计算）
      if(ReadUART1(&numBytes, 1) != 1)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 读取页地址和校验和
      if(ReadUART1(eraseData, numBytes + 1) != (numBytes + 1))
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 验证校验和
      for(i = 0; i <= numBytes; i++)
      {
        eraseChecksum ^= eraseData[i];
      }
      
      if(eraseChecksum != 0xFF)
      {
        BootloaderSendAck(0x1F);
        return TRUE;
      }
      
      // 执行Flash擦除
      FLASH_Unlock(); // 解锁Flash
      
      // 擦除所有指定的页
      for(pageNum = 0; pageNum < numBytes; pageNum++)
      {
        pageAddr = APPLICATION_START_ADDR + (eraseData[pageNum] * FLASH_PAGE_SIZE);
        
        // 验证页地址是否在应用程序区域
        if(pageAddr >= APPLICATION_START_ADDR && 
           pageAddr < (APPLICATION_START_ADDR + 0x10000)) // 假设应用程序不超过64KB
        {
          flashStatus = FLASH_ErasePage(pageAddr);
          if(flashStatus != FLASH_COMPLETE)
          {
            FLASH_Lock(); // 上锁Flash
            BootloaderSendAck(0x1F);
            return TRUE;
          }
        }
        else
        {
          FLASH_Lock(); // 上锁Flash
          BootloaderSendAck(0x1F);
          return TRUE;
        }
      }
      
      FLASH_Lock(); // 上锁Flash
      BootloaderSendAck(0x79);  // 正确应答
      break;
      
    default:
      BootloaderSendAck(0x1F);  // 不支持的命令
      break;
  }
  
  return TRUE;  // 继续运行Bootloader
}

/*********************************************************************************************************
* 函数名称：BootloaderJumpToApp
* 函数功能：跳转到应用程序 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void BootloaderJumpToApp(void)
{
  typedef void (*pFunction)(void);
  pFunction Jump_To_Application;
  u32 JumpAddress;
  
  // 检查应用程序是否有效（检查栈顶地址是否有效）
  if (((*(__IO u32*)APPLICATION_START_ADDR) & 0x2FFE0000) == 0x20000000)
  {
    // 获取应用程序复位向量地址
    JumpAddress = *(__IO u32*)(APPLICATION_START_ADDR + 4);
    Jump_To_Application = (pFunction)JumpAddress;
    
    // 设置主堆栈指针
    __set_MSP(*(__IO u32*)APPLICATION_START_ADDR);
    
    // 跳转到应用程序
    Jump_To_Application();
  }
}

/*********************************************************************************************************
* 函数名称：BootloaderSendAck
* 函数功能：发送应答 
* 输入参数：ack - 应答字节
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void BootloaderSendAck(u8 ack)
{
  WriteUART1(&ack, 1);
}



/*********************************************************************************************************
* 函数名称：BootloaderSendByte
* 函数功能：发送一个字节 
* 输入参数：byte - 要发送的字节
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void BootloaderSendByte(u8 byte)
{
  WriteUART1(&byte, 1);
}



/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitBootLoader
* 函数功能：初始化BootLoader模块 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void InitBootLoader(void)
{
  s_u8BootloaderActive = TRUE;
  s_u32BootTimeoutCount = 0;
}

/*********************************************************************************************************
* 函数名称：BootLoaderTask
* 函数功能：BootLoader任务处理 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void BootLoaderTask(void)
{
  if(s_u8BootloaderActive)
  {
    BootloaderWaitForSync();
    
    // 如果仍在Bootloader模式，开始处理命令
    while(s_u8BootloaderActive)
    {
      if(!BootloaderProcessCommand())
      {
        // 命令处理完成，准备跳转到应用程序
        break;
      }
    }
  }
  
  // 尝试跳转到应用程序
  BootloaderJumpToApp();
}

/*********************************************************************************************************
* 函数名称：IsBootloaderActive
* 函数功能：检查Bootloader是否处于活动状态 
* 输入参数：void
* 输出参数：void
* 返 回 值：u8 - TRUE表示Bootloader活动，FALSE表示非活动
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
u8 IsBootloaderActive(void)
{
  return s_u8BootloaderActive;
}
