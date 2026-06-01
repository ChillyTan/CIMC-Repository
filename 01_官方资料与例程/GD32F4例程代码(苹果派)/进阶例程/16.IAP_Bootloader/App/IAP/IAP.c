/*********************************************************************************************************
* 模块名称：IAP.c
* 摘    要：IAP升级模块
* 当前版本：1.0.0
* 作    者：Leyutek(COPYRIGHT 2018 - 2021 Leyutek. All rights reserved.)
* 完成日期：2021年07月01日
* 内    容：
* 注    意：使用SD卡在线升级，bin文件修改日期相当于版本号
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
#include "IAP.h"
#include "gd32f470x_conf.h"
#include "ff.h"
#include "Flash.h"
#include "SerialString.h"

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
__asm static void SetMSP(u32 addr);                                    //设置主栈指针
static       u8   IsBinType(char* name);                               //判断是否为Bin文件
static       void CombiPathAndName(char* buf, char* path, char* name); //将路径和名字组合在一起

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：SetMSP
* 函数功能：设置主栈指针
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：addr参数存储在R0中
*********************************************************************************************************/
__asm static void SetMSP(u32 addr)
{
  MSR MSP, r0
  BX r14
}

/*********************************************************************************************************
* 函数名称：IsBinType
* 函数功能：判断是否为Bin文件
* 输入参数：void
* 输出参数：void
* 返 回 值：1-是，0-否
* 创建日期：2021年07月01日
* 注    意：支持BIN、Bin和bin结尾三种格式
*********************************************************************************************************/
static u8 IsBinType(char* name)
{
  u32 i, flag;

  i = 0;
  flag = 0;
  while((0 != name[i]) && (i < MAX_BIN_NAME_LEN))
  {
    if('.' == name[i])
    {
      if(('B' == name[i + 1]) || ('I' == name[i + 2]) || ('N' == name[i + 3]))
      {
        flag = 1;
      }
      else if(('B' == name[i + 1]) || ('i' == name[i + 2]) || ('n' == name[i + 3]))
      {
        flag = 1;
      }
      else if(('b' == name[i + 1]) || ('i' == name[i + 2]) || ('n' == name[i + 3]))
      {
        flag = 1;
      }
      else
      {
        flag = 0;
      }
    }
    i++;
  }

  return flag;
}

/*********************************************************************************************************
* 函数名称：CombiPathAndName
* 函数功能：将路径和名字组合在一起，并储存在buf中
* 输入参数：buf：字符串缓冲区，path：路径，name：文件名
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void CombiPathAndName(char* buf, char* path, char* name)
{
  u32 i, j;

  //保存路径到buf中
  i = 0;
  j = 0;
  while((0 != path[i]) && (j < MAX_BIN_NAME_LEN))
  {
    buf[j++] = path[i++];
  }
  buf[j++] = '/';

  //将名字保存到buf中
  i = 0;
  while((0 != name[i]) && (j < MAX_BIN_NAME_LEN))
  {
    buf[j++] = name[i++];
  }
  buf[j] = 0;
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：GotoApp
* 函数功能：跳转到App
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void GotoApp(u32 appAddr)
{
  //App复位中断服务函数
  void (*appResetHandler)(void); 

  //延时变量
  u32 delay;

  //检查栈顶地址是否合法.
  if(0x20000000 == ((*(u32*)appAddr) & 0x2FF80000)) 
  {
    //输出提示正在跳转中
    PutString(" 跳转到App...\r\n");
    PutString("----Leyutek(COPYRIGHT 2018 - 2021 Leyutek. All rights reserved.)-----\r\n");
    PutString("\r\n");
    PutString("\r\n");
    
    //延时等待字符串打印完成
    delay = 10000;
    while(delay--);

    //获取App复位中断服务函数地址，用户代码区第二个字为程序开始地址(复位地址)
    appResetHandler = (void (*)(void))(*(u32*)(appAddr + 4));

    //设置App主栈指针，用户代码区的第一个字用于存放栈顶地址
    SetMSP(*(u32*)(appAddr));

    //跳转到App
    appResetHandler();
  }
  else
  {
    PutString(" 非法栈顶地址\r\n");
    PutString(" 跳转到App失败!!!\r\n");
    PutString("----Leyutek(COPYRIGHT 2018 - 2021 Leyutek. All rights reserved.)-----\r\n");
    PutString("\r\n");
    PutString("\r\n");
  }
}

/*********************************************************************************************************
* 函数名称：CheckAppVersion
* 函数功能：指定目录下App版本校验，若发现App版本有更新则自动更新
* 输入参数：path：搜索路径
* 输出参数：void
* 返 回 值：0-版本校验成功，无需更新，1-版本校验失败，需要更新App程序
* 创建日期：2021年07月01日
* 注    意：
*          1、校验前要先挂载文件系统
*          2、以Bin文件修改日期为版本号，若发现指定目录下Bin文件与本地保存的Bin文件修改日期不一致则更新App程序
*********************************************************************************************************/
void CheckAppVersion(char* path)
{
  static FIL  s_fileBin;                   //Bin文件
  static u8   s_arrBuf[FILE_BUF_SIZE];     //数据缓冲区
  static u32  s_iReadNum;                  //成功读取量
  static char s_arrName[MAX_BIN_NAME_LEN]; //Bin文件名（含路径）
  static u32  s_iLastProcess;              //上次显示的进度
  static u32  s_iCurrentProcess;           //当前进度
  FRESULT     result;                      //文件操作返回变量
  DIR         direct;                      //路径
  FILINFO     fileInfo;                    //文件信息
  u32         appVersion;                  //APP版本号
  u32         localVersion;                //本地版本号
  u32         flashWriteAddr;              //Flash写入地址
  u32         year, month, data, hour, minute, second;

  PutString(" 开始搜索Bin文件并校验App版本\r\n");
  PutString(" --Bin文件目录："); PutString(path); PutString("\r\n\r\n");

  //打开指定路径
  result = f_opendir(&direct, path);
  if(result != FR_OK)
  {
    PutString(" 路径："); PutString(path); PutString(" 不存在\r\n");
    PutString(" 校验结束\r\n\r\n");
    return;
  }

  //在指定目录下搜索App Bin文件
  while(1)
  {
    result = f_readdir(&direct, &fileInfo);
    if((result != FR_OK) || (0 == fileInfo.fname[0]))
    {
      PutString(" 没有查找到Bin文件\r\n");
      PutString(" 请检查Bin文件是否已经放入指定目录\r\n\r\n");
      return;
    }
    else if(1 == IsBinType(fileInfo.fname))
    {
      year   = ((fileInfo.fdate & 0xFE00) >> 9 ) + 1980;
      month  = ((fileInfo.fdate & 0x01E0) >> 5 );
      data   = ((fileInfo.fdate & 0x001F) >> 0 );
      hour   = ((fileInfo.ftime & 0xF800) >> 11);
      minute = ((fileInfo.ftime & 0x07E0) >> 5 );
      second = ((fileInfo.ftime & 0x001F) >> 0 );
      PutString(" 成功查找到Bin文件："); PutString(path); PutString("/"); PutString(fileInfo.fname);
      PutString("  ");
      PutDecUint(year, 1); PutString("/"); PutDecUint(month, 1); PutString("/"); PutDecUint(data, 1);
      PutString(" ");
      PutDecUint(hour, 2); PutString(":"); PutDecUint(minute, 2); PutString(":"); PutDecUint(second, 2);
      PutString("  ");
      PutDouble(fileInfo.fsize / 1024.0, 2); PutString(" KB\r\n\r\n");
      break;
    }
  }

  //校验App版本
  appVersion = ((u32)fileInfo.fdate << 16) | fileInfo.ftime;
  localVersion = *(u32*)APP_VERSION_BEGIN_ADDR;
  if(appVersion == localVersion)
  {
    PutString(" 当前App为版本与本地App版本一致，无需更新\r\n\r\n");
    return;
  }
  else if(appVersion < localVersion)
  {
    PutString(" 请注意，当前App并非最新版本\r\n\r\n");
  }
  else
  {
    PutString(" 当前App为最新版本，需要更新\r\n\r\n");
  }

  //将路径和Bin文件名组合到一起
  CombiPathAndName(s_arrName, path, fileInfo.fname);

  //开始更新
  PutString(" 开始更新\r\n");

  //打开文件
  result = f_open(&s_fileBin, s_arrName, FA_OPEN_EXISTING | FA_READ);
  if(result != FR_OK)
  {
    PutString(" 打开Bin文件失败\r\n");
    PutString(" 更新失败\r\n\r\n");
    return;
  }

  //读取Bin文件数据并写入到Flash指定位置
  flashWriteAddr = (u32)APP_BEGIN_ADDR;
  s_iLastProcess = 0;
  s_iCurrentProcess = 0;
  while(1)
  {
    //输出更新进度
    s_iCurrentProcess = 100 * s_fileBin.fptr / s_fileBin.fsize;
    if((s_iCurrentProcess - s_iLastProcess) >= 5)
    {
      s_iLastProcess = s_iCurrentProcess;
      PutString(" 更新进度："); PutDecUint(s_iCurrentProcess, 1); PutString("\r\n");
    }
    
    //读取Bin文件数据到数据缓冲区
    result = f_read(&s_fileBin, s_arrBuf, FILE_BUF_SIZE, &s_iReadNum);
    if(result !=  FR_OK)
    {
      PutString(" 读取Bin文件数据失败\r\n");
      PutString(" 更新失败\r\n\r\n");
      return;
    }

    //将读取到的数据写入Flash中
    if(s_iReadNum > 0)
    {
      FlashWriteWord(flashWriteAddr, (u32*)s_arrBuf, s_iReadNum / 4);
    }

    //更新Flash写入位置
    flashWriteAddr = flashWriteAddr + s_iReadNum;

    //判断文件是否读完
    if((s_fileBin.fptr >= s_fileBin.fsize) || (0 == s_iReadNum))
    {
      PutString(" 更新进度：%%100\r\n");
      PutString(" 更新完成\r\n\r\n");
      break;
    }
  }
  
  //保存App版本到Flash指定位置
  FlashWriteWord(APP_VERSION_BEGIN_ADDR, &appVersion, 1);

  //关闭文件
  f_close(&s_fileBin);

  //关闭目录
  f_closedir(&direct);
}

/*********************************************************************************************************
* 函数名称：SystemReset
* 函数功能：系统复位
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void SystemReset(void)
{
	__set_FAULTMASK(1);//关闭所有中断
	NVIC_SystemReset();//系统复位
}
