/*********************************************************************************************************
* 模块名称：SHT20.c
* 摘    要：外部温湿度驱动模块
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
#include "SHT20.h"
#include "gd32f470x_conf.h"
#include "IICCommon.h"
#include "SysTick.h"
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
static StructIICCommonDev s_structIICDev; //IIC设备

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void   ConfigSCLGPIO(void);     //配置SCL的GPIO
static void   ConfigSDAMode(u8 mode);  //配置SDA输入输出
static void   SetSCL(u8 state);        //时钟信号线SCL控制
static void   SetSDA(u8 state);        //数据信号线SDA控制
static u8     GetSDA(void);            //获取SDA输入电平
static void   Delay(u8 time);          //延时函数

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigSCLGPIO
* 函数功能：配置SCL的GPIO 
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：IIC闲时SCL拉高
*********************************************************************************************************/
static void ConfigSCLGPIO(void)
{
  //使能RCU相关时钟
  rcu_periph_clock_enable(RCU_GPIOB);  //使能GPIOB的时钟
  
  //SCL
  gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_6);         //上拉输出
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX,GPIO_PIN_6); //推挽输出
  gpio_bit_set(GPIOB, GPIO_PIN_6);                                              //拉高SCL
  
  //SDA
  gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_7);         //上拉输出
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX,GPIO_PIN_7); //推挽输出
  gpio_bit_set(GPIOB, GPIO_PIN_7);                                              //拉高SCL
}

/*********************************************************************************************************
* 函数名称：ConfigSDAMode
* 函数功能：配置SDA输入输出
* 输入参数：mode：IIC_COMMON_INPUT、IIC_COMMON_OUTPUT 
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ConfigSDAMode(u8 mode)
{
  rcu_periph_clock_enable(RCU_GPIOB);  //使能GPIOB的时钟
  
  //配置成输出
  if(IIC_COMMON_OUTPUT == mode)
  {
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_7);         //上拉输出
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX,GPIO_PIN_7); //推挽输出
  }
  
  //配置成输入
  else if(IIC_COMMON_INPUT == mode)
  {
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_7);          //上拉输入
  }
}

/*********************************************************************************************************
* 函数名称：SetSCL
* 函数功能：时钟信号线SCL控制
* 输入参数：state：0-输入低电平，1-输出高电平
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void SetSCL(u8 state)
{
  gpio_bit_write(GPIOB, GPIO_PIN_6, (bit_status)state);
}


/*********************************************************************************************************
* 函数名称：SetSDA
* 函数功能：数据信号线SDA控制
* 输入参数：state：0-输入低电平，1-输出高电平
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void SetSDA(u8 state)
{
  gpio_bit_write(GPIOB, GPIO_PIN_7, (bit_status)state);
}

/*********************************************************************************************************
* 函数名称：GetSDA
* 函数功能：获取SDA输入电平
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static u8 GetSDA(void)
{
  return gpio_input_bit_get(GPIOB, GPIO_PIN_7);
}

/*********************************************************************************************************
* 函数名称：Delay
* 函数功能：延时函数
* 输入参数：time：延时时钟周期
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void Delay(u8 time)
{
  DelayNus(10 * time);
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitSHT20
* 函数功能：初始化外部温湿度驱动模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitSHT20(void)
{
  //配置IIC引脚
  ConfigSCLGPIO();                  //配置SCL的GPIO
  ConfigSDAMode(IIC_COMMON_INPUT);  //配置SDA为输入模式
  
  //配置IIC设备
  s_structIICDev.deviceID      = 0x80;
  s_structIICDev.SetSCL        = SetSCL;
  s_structIICDev.SetSDA        = SetSDA;
  s_structIICDev.GetSDA        = GetSDA;
  s_structIICDev.ConfigSDAMode = ConfigSDAMode;
  s_structIICDev.Delay         = Delay;
  
  //模块上电后需要至少15ms才能进入空闲状态
  DelayNms(100);
}

/*********************************************************************************************************
* 函数名称：GetSHT20Temp
* 函数功能：获取SHT20温度测量结果
* 输入参数：void 
* 输出参数：void
* 返 回 值：摄氏度℃
* 创建日期：2021年07月01日
* 注    意：默认状态下温度数据默认14bit格式
*********************************************************************************************************/
double GetSHT20Temp(void)
{
  u16 byte = 0; //从IIC中接收到的字节

  //发送起始信号
  IICCommonStart(&s_structIICDev);

  //发送写地址信息
  if(1 == IICCommonSendOneByte(&s_structIICDev, 0x80))
  {
    printf("SHT20: Fail to send write addr while get temp\r\n");
    return 0;
  }

  //发送读取温度命令，使用“no hold master”模式
  if(1 == IICCommonSendOneByte(&s_structIICDev, 0xF3))
  {
    printf("SHT20: Fail to send cmd while get temp\r\n");
    return 0;
  }

  //结束传输
  IICCommonEnd(&s_structIICDev);

  //等待温度测量完成，在14bit格式下最多需要85ms
  DelayNms(100);

  //发送读取测量结果命令
  IICCommonStart(&s_structIICDev);
  while(1 == IICCommonSendOneByte(&s_structIICDev, 0x81))
  {
    printf("SHT20: Fail to read date while get temp\r\n");
    printf("SHT20: Try again\r\n");
    IICCommonEnd(&s_structIICDev);
    DelayNms(100);
    IICCommonStart(&s_structIICDev);
  }

  //读取温度高字节原始数据
  byte = 0;
  byte = byte | IICCommonReadOneByte(&s_structIICDev, IIC_COMMON_ACK);

  //读取温度低字节原始数据
  byte = (byte << 8) | IICCommonReadOneByte(&s_structIICDev, IIC_COMMON_ACK);

  //结束接收温度数据
  IICCommonEnd(&s_structIICDev);

  //校验是否为温度数据
  if(byte & (1 << 1))
  {
    printf("SHT20: this is not a temp data\r\n");
    return 0;
  }
  else
  {
    //计算温度值
    byte = byte & 0xFFFC;
    return (-46.85 + 175.72 * ((double)byte) / 65536.0);
  }
}

/*********************************************************************************************************
* 函数名称：GetSHT20RH
* 函数功能：获取SHT20湿度测量结果
* 输入参数：void
* 输出参数：void
* 返 回 值：温湿度
* 创建日期：2021年07月01日
* 注    意：默认状态下湿度数据默认12bit格式
*********************************************************************************************************/
double GetSHT20RH(void)
{
  u16 byte = 0; //从IIC中接收到的字节

  //发送起始信号
  IICCommonStart(&s_structIICDev);

  //发送写地址信息
  if(1 == IICCommonSendOneByte(&s_structIICDev, 0x80))
  {
    printf("SHT20: Fail to send write addr while get RH\r\n");
    return 0;
  }

  //发送读取湿度命令，使用“no hold master”模式
  if(1 == IICCommonSendOneByte(&s_structIICDev, 0xF5))
  {
    printf("SHT20: Fail to send cmd while get RH\r\n");
    return 0;
  }

  //结束传输
  IICCommonEnd(&s_structIICDev);

  //等待湿度测量完成，在12bit格式下最多需要29ms
  DelayNms(100);

  //发送读取测量结果命令
  IICCommonStart(&s_structIICDev);
  while(1 == IICCommonSendOneByte(&s_structIICDev, 0x81))
  {
    printf("SHT20: Fail to read date while get RH\r\n");
    printf("SHT20: Try again\r\n");
    IICCommonEnd(&s_structIICDev);
    DelayNms(100);
    IICCommonStart(&s_structIICDev);
  }

  //读取湿度高字节原始数据
  byte = 0;
  byte = byte | IICCommonReadOneByte(&s_structIICDev, IIC_COMMON_ACK);

  //读取湿度低字节原始数据
  byte = (byte << 8) | IICCommonReadOneByte(&s_structIICDev, IIC_COMMON_ACK);

  //结束接收湿度数据
  IICCommonEnd(&s_structIICDev);

  //校验是否为湿度数据
  if(byte & (1 << 1))
  {
    //计算湿度值
    byte = byte & 0xFFFC;
    return (-6.0 + 125.0 * ((double)byte) / 65536.0);
  }
  else
  {
    printf("SHT20: this is not a RH data\r\n");
    return 0;
  }
}
