/*********************************************************************************************************
* 模块名称：I2S.c
* 摘    要：I2S音频驱动模块
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
#include "I2S.h"
#include "gd32f470x_conf.h"
#include "stdio.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/
typedef struct
{
  u32 fs;     //采样率，单位是Hz
  u32 pllN;   //PLL I2S倍频系数（N，50-500）
  u32 pllR;   //PLL I2S分频系数（R，2-7）
  u32 i2sDIV; //I2S预分频器的分频系数（0-1）
  u32 i2sOF;  //I2S预分频器的奇系数（1 - 255）
}StructISCPscInfo;

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
//I2S分频系数表
//PLLI2S时钟输入固定为1MHz
//默认MCK打开
//音频采样率公式：FS = I2SCLK / (256 * (DIV * 2 + OF))
static const StructISCPscInfo s_arrI2SPscTable[]=
{
  {8000,   256, 5, 12, 1}, //8Khz采样率
  {11025,  429, 2, 38, 0}, //11.025Khz采样率
  {16000,  213, 2, 13, 0}, //16Khz采样率
  {22050,  429, 2, 19, 0}, //22.05Khz采样率
  {32000,  213, 2,  6, 1}, //32Khz采样率
  {44100,  429, 2,  9, 1}, //44.1Khz采样率
  {48000,  467, 2,  9, 1}, //48Khz采样率
  {88200,  271, 2,  3, 0}, //88.2Khz采样率
  {96000,  344, 2,  3, 1}, //96Khz采样率
  {176400, 271, 2,  1, 1}, //176.4Khz采样率
  {192000, 295, 2,  1, 1}, //192Khz采样率
};

static u32 s_iDMAFlagStartFlag = 0;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigI2SGPIO(void);  //配置I2S的GPIO

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigI2SGPIO
* 函数功能：配置I2S的GPIO
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigI2SGPIO(void)
{
  //使能RCC相关时钟
  rcu_periph_clock_enable(RCU_GPIOB);  //使能GPIOB的时钟
  rcu_periph_clock_enable(RCU_GPIOC);  //使能GPIOC的时钟
  rcu_periph_clock_enable(RCU_GPIOI);  //使能GPIOI的时钟

  //I2S1_WS
  gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_12);
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_12);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_12);

  //I2S1_CK
  gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_13);
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_13);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_13);

  //I2S1_ADD_SD
  gpio_af_set(GPIOB, GPIO_AF_6, GPIO_PIN_14);
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_14);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_14);

  //I2S1_SD
  gpio_af_set(GPIOI, GPIO_AF_5, GPIO_PIN_3);
  gpio_mode_set(GPIOI, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_3);
  gpio_output_options_set(GPIOI, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_3);

  //I2S1_MCK
  gpio_af_set(GPIOC, GPIO_AF_5, GPIO_PIN_6);
  gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_6);
  gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_6);
}

/*********************************************************************************************************
* 函数名称：GetPscInfo
* 函数功能：根据采样率获取分频系数
* 输入参数：void
* 输出参数：void
* 返 回 值：分频系数结构体首地址
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
static StructISCPscInfo* GetPscInfo(u32 fs)
{
  StructISCPscInfo* info;
  u32 i;

  //查表获取分频信息
  info = NULL;
  for(i = 0; i < sizeof(s_arrI2SPscTable) / sizeof(StructISCPscInfo); i++)
  {
    if(s_arrI2SPscTable[i].fs == fs)
    {
      info = (StructISCPscInfo*)(&s_arrI2SPscTable[i]);
      break;
    }
  }

  return info;
}

/*********************************************************************************************************
* 函数名称：ConfigI2SClock
* 函数功能：配置I2S时钟
* 输入参数：void
* 输出参数：void
* 返 回 值：0-配置成功，1-配置失败
* 创建日期：2018年01月01日
* 注    意：调用前要先开启I2S1时钟
*********************************************************************************************************/
static u32 ConfigI2SClock(u32 fs)
{
  StructISCPscInfo* info;

  //获取分频信息
  info = GetPscInfo(fs);

  //获取分频信息失败
  if(NULL == info)
  {
    printf("ConfigI2SCLK: fail to get psc info!!!\r\n");
    return 1;
  }

  //关闭PLLI2S
  rcu_osci_off(RCU_PLLI2S_CK);

  //配置PLLI2S
  if(ERROR == rcu_plli2s_config(info->pllN, info->pllR))
  {
    while(1);
  }

  //开启PLLI2S
  rcu_osci_on(RCU_PLLI2S_CK);
  if(ERROR == rcu_osci_stab_wait(RCU_PLLI2S_CK))
  {
    while(1);
  }

  //配置PLLI2S为I2S时钟输入
  rcu_i2s_clock_config(RCU_I2SSRC_PLLI2S);

  return 0;
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitI2S
* 函数功能：初始化I2S音频驱动
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void InitI2S(void)
{
  s_iDMAFlagStartFlag = 0;
}

/*********************************************************************************************************
* 函数名称：I2SConfig
* 函数功能：配置I2S
* 输入参数：fs：采样率，单位是Hz
* 输出参数：void
* 返 回 值：0-配置成功，1-配置失败
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
u32 I2SConfig(u32 fs, EnumI2SDataLen dataLen, EnumI2SChLen chLen)
{
  StructISCPscInfo* info;

  //获取分频信息
  info = GetPscInfo(fs);

  //获取分频信息失败
  if(NULL == info)
  {
    printf("ConfigI2SCLK: fail to get psc info!!!\r\n");
    return 1;
  }

  //使能I2S1时钟
  rcu_periph_clock_enable(RCU_SPI1);

  //复位I2S1
  spi_i2s_deinit(SPI1);

  //配置I2S的GPIO
  ConfigI2SGPIO();

  //配置时钟
  if(0 != ConfigI2SClock(fs))
  {
    printf("I2SConfig: Fail to config I2S clock\r\n");
    return 1;
  }

  /*
   *I2S配置
   */

  //设置为I2S模式
  SPI_I2SCTL(SPI1) |= (1 << 11);

  //I2S_MCK输出使能
  SPI_I2SPSC(SPI1) |= (1 << 9);

  //配置预分频器的奇系数
  SPI_I2SPSC(SPI1) &= ~(0x01 << 8);
  SPI_I2SPSC(SPI1) |= ((info->i2sOF & 0x01) << 8);

  //配置预分频器的分频系数
  SPI_I2SPSC(SPI1) &= ~(0xFF << 0);
  SPI_I2SPSC(SPI1) |= ((info->i2sDIV & 0xFF) << 0);

  //I2S_CK空闲状态为高电平
  SPI_I2SCTL(SPI1) |= (1 << 3);

  //设置I2S为主机发送模式
  SPI_I2SCTL(SPI1) |= (1 << 9);
  SPI_I2SCTL(SPI1) &= ~(1 << 8);

  //选用飞利浦标准
  SPI_I2SCTL(SPI1) &= ~(1 << 5);
  SPI_I2SCTL(SPI1) &= ~(1 << 4);

  //配置数据长度
  if(DATA_LEN_16 == dataLen)
  {
    SPI_I2SCTL(SPI1) &= ~(1 << 2);
    SPI_I2SCTL(SPI1) &= ~(1 << 1);
  }
  else if(DATA_LEN_24 == dataLen)
  {
    SPI_I2SCTL(SPI1) &= ~(1 << 2);
    SPI_I2SCTL(SPI1) |= (1 << 1);
  }
  else if(DATA_LEN_32 == dataLen)
  {
    SPI_I2SCTL(SPI1) |= (1 << 2);
    SPI_I2SCTL(SPI1) &= ~(1 << 1);
  }
  else
  {
    return 1;
  }

  //配置通道长度
  if(CH_LEN_16 == chLen)
  {
    SPI_I2SCTL(SPI1) &= ~(1 << 0);
  }
  else if(CH_LEN_32 == chLen)
  {
    SPI_I2SCTL(SPI1) |= (1 << 0);
  }
  else
  {
    return 1;
  }

  //发送缓冲区DMA使能
  SPI_CTL1(SPI1) |= (1 << 1);

  //I2S使能
  SPI_I2SCTL(SPI1) |= (1 << 10);

  /*
   *I2S ADD配置
   */

  //设置为I2S ADD模式
  I2S_ADD_I2SCTL(I2S1_ADD) |= (1 << 11);

  //I2S_CK空闲状态为高电平
  I2S_ADD_I2SCTL(I2S1_ADD) |= (1 << 3);

  //设置I2S ADD为从机接收模式
  I2S_ADD_I2SCTL(I2S1_ADD) &= ~(1 << 9);
  I2S_ADD_I2SCTL(I2S1_ADD) |= (1 << 8);

  //选用飞利浦标准
  I2S_ADD_I2SCTL(I2S1_ADD) &= ~(1 << 5);
  I2S_ADD_I2SCTL(I2S1_ADD) &= ~(1 << 4);

  //配置数据长度
  if(DATA_LEN_16 == dataLen)
  {
    I2S_ADD_I2SCTL(I2S1_ADD) &= ~(1 << 2);
    I2S_ADD_I2SCTL(I2S1_ADD) &= ~(1 << 1);
  }
  else if(DATA_LEN_24 == dataLen)
  {
    I2S_ADD_I2SCTL(I2S1_ADD) &= ~(1 << 2);
    I2S_ADD_I2SCTL(I2S1_ADD) |= (1 << 1);
  }
  else if(DATA_LEN_32 == dataLen)
  {
    I2S_ADD_I2SCTL(I2S1_ADD) |= (1 << 2);
    I2S_ADD_I2SCTL(I2S1_ADD) &= ~(1 << 1);
  }
  else
  {
    return 1;
  }

  //配置通道长度
  if(CH_LEN_16 == chLen)
  {
    I2S_ADD_I2SCTL(I2S1_ADD) &= ~(1 << 0);
  }
  else if(CH_LEN_32 == chLen)
  {
    I2S_ADD_I2SCTL(I2S1_ADD) |= (1 << 0);
  }
  else
  {
    return 1;
  }

  //接收缓冲区DMA使能
  I2S_ADD_CTL1(I2S1_ADD) |= (1 << 0);

  //I2S ADD使能
  I2S_ADD_I2SCTL(I2S1_ADD) |= (1 << 10);

  return 0;
}

/*********************************************************************************************************
* 函数名称：I2SReadWrite16
* 函数功能：I2S读写16位数据
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
u16 I2SReadWrite16(u16 data)
{
  //等待发送缓冲区空闲（等待上一批数据发送完成）
  while(RESET == spi_i2s_flag_get(SPI1, I2S_FLAG_TBE));

  //发送数据
  spi_i2s_data_transmit(SPI1, data);

  //等待接收缓冲区非空（接收数据）
  //while(RESET == spi_i2s_flag_get(SPI1, I2S_FLAG_RBNE));

  //获取I2S数据输入
  return(spi_i2s_data_receive(SPI1));
}

/*********************************************************************************************************
* 函数名称：I2SReadWrite24
* 函数功能：I2S读写24位数据
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
u32 I2SReadWrite24(u32 data)
{
  u16 writeData;
  u16 readData;
  u32 result;

  //发送高16位，D[23:8]
  writeData = data >> 8;
  readData = I2SReadWrite16(writeData);
  result = 0;
  result = result | ((u32)readData << 8);

  //发送低16位， D[7:0]
  writeData = data << 8;
  readData = I2SReadWrite16(writeData);
  result = result | (readData >> 8);

  return result;
}

/*********************************************************************************************************
* 函数名称：I2SReadWrite32
* 函数功能：I2S读写32位数据
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
u32 I2SReadWrite32(u32 data)
{
  u16 writeData;
  u16 readData;
  u32 result;

  //发送高16位
  writeData = data >> 16;
  readData = I2SReadWrite16(writeData);
  result = 0;
  result = result | ((u32)readData << 16);

  //发送低16位
  writeData = data & 0xFFFF;
  readData = I2SReadWrite16(writeData);
  result = result | readData;

  return result;
}

/*********************************************************************************************************
* 函数名称：I2SStartDMATransmitWidth16
* 函数功能：I2S使用DMA发送16位数据
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void I2SSendData16ByDMA(u16* dataBuf, u32 len)
{
  dma_multi_data_parameter_struct dma_multi_initpara;

  //等待上次传输完成
  while(0 != I2STransmitState()){}

  //开启DMA0时钟
  rcu_periph_clock_enable(RCU_DMA0);

  //禁用DMA传输
  dma_channel_disable(DMA0, DMA_CH4);

  //复位DMA0通道
  dma_deinit(DMA0, DMA_CH4);

  //配置DMA参数
  dma_multi_data_para_struct_init(&dma_multi_initpara);
  dma_multi_initpara.periph_addr        = (u32)(&SPI_DATA(SPI1));
  dma_multi_initpara.periph_width       = DMA_PERIPH_WIDTH_16BIT;
  dma_multi_initpara.periph_inc         = DMA_PERIPH_INCREASE_DISABLE;
  dma_multi_initpara.memory0_addr       = (u32)dataBuf;
  dma_multi_initpara.memory_width       = DMA_MEMORY_WIDTH_16BIT;
  dma_multi_initpara.memory_inc         = DMA_MEMORY_INCREASE_ENABLE;
  dma_multi_initpara.memory_burst_width = DMA_MEMORY_BURST_SINGLE;
  dma_multi_initpara.periph_burst_width = DMA_PERIPH_BURST_SINGLE;
  dma_multi_initpara.critical_value     = DMA_FIFO_4_WORD;
  dma_multi_initpara.circular_mode      = DMA_CIRCULAR_MODE_DISABLE;
  dma_multi_initpara.direction          = DMA_MEMORY_TO_PERIPH;
  dma_multi_initpara.number             = len;
  dma_multi_initpara.priority           = DMA_PRIORITY_LOW;
  dma_multi_data_mode_init(DMA0, DMA_CH4, &dma_multi_initpara);
  dma_channel_subperipheral_select(DMA0, DMA_CH4, DMA_SUBPERI0);
  dma_flow_controller_config(DMA0, DMA_CH4, DMA_FLOW_CONTROLLER_DMA);

  //开启DMA传输
  dma_channel_enable(DMA0, DMA_CH4);

  //标记传输开始
  s_iDMAFlagStartFlag = 1;
}

/*********************************************************************************************************
* 函数名称：I2SReadData16ByDMA
* 函数功能：I2S使用DMA接收16位数据
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void I2SReadData16ByDMA(u16* dataBuf, u32 len)
{
  dma_multi_data_parameter_struct dma_multi_initpara;

  //开启DMA0时钟
  rcu_periph_clock_enable(RCU_DMA0);

  //禁用DMA传输
  dma_channel_disable(DMA0, DMA_CH3);

  //复位DMA0通道
  dma_deinit(DMA0, DMA_CH3);

  //配置DMA参数
  dma_multi_data_para_struct_init(&dma_multi_initpara);
  dma_multi_initpara.periph_addr        = (u32)(&I2S_ADD_DATA(I2S1_ADD));
  dma_multi_initpara.periph_width       = DMA_PERIPH_WIDTH_16BIT;
  dma_multi_initpara.periph_inc         = DMA_PERIPH_INCREASE_DISABLE;
  dma_multi_initpara.memory0_addr       = (u32)dataBuf;
  dma_multi_initpara.memory_width       = DMA_MEMORY_WIDTH_16BIT;
  dma_multi_initpara.memory_inc         = DMA_MEMORY_INCREASE_ENABLE;
  dma_multi_initpara.memory_burst_width = DMA_MEMORY_BURST_SINGLE;
  dma_multi_initpara.periph_burst_width = DMA_PERIPH_BURST_SINGLE;
  dma_multi_initpara.critical_value     = DMA_FIFO_4_WORD;
  dma_multi_initpara.circular_mode      = DMA_CIRCULAR_MODE_DISABLE;
  dma_multi_initpara.direction          = DMA_PERIPH_TO_MEMORY;
  dma_multi_initpara.number             = len;
  dma_multi_initpara.priority           = DMA_PRIORITY_LOW;
  dma_multi_data_mode_init(DMA0, DMA_CH3, &dma_multi_initpara);
  dma_channel_subperipheral_select(DMA0, DMA_CH3, DMA_SUBPERI3);
  dma_flow_controller_config(DMA0, DMA_CH3, DMA_FLOW_CONTROLLER_DMA);

  //开启DMA传输
  dma_channel_enable(DMA0, DMA_CH3);

  //等待DMA传输完成
  while(SET != dma_flag_get(DMA0, DMA_CH3, DMA_FLAG_FTF)){}
}

/*********************************************************************************************************
* 函数名称：I2STransmitState
* 函数功能：获取发送状态
* 输入参数：void
* 输出参数：void
* 返 回 值：0-发送完成，1-忙碌
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
u32 I2STransmitState(void)
{
  if(0 == s_iDMAFlagStartFlag)
  {
    return 0;
  }
  else if(SET == dma_flag_get(DMA0, DMA_CH4, DMA_FLAG_FTF))
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

/*********************************************************************************************************
* 函数名称：I2SDualDMATransmit16
* 函数功能：I2S使用DMA发送16位数据
* 输入参数：dataBuf1，dataBuf2：双缓冲区首地址，len：缓冲区长度（按照16位数据计算）
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：使用DMA双缓冲区发送数据，首先发送的是缓冲区1中的数据，然后是缓冲区2的数据，交替循环
*********************************************************************************************************/
void I2SDualDMATransmit16(u16* dataBuf1, u16* dataBuf2, u32 len)
{
  dma_multi_data_parameter_struct dma_multi_initpara;

  //开启DMA0时钟
  rcu_periph_clock_enable(RCU_DMA0);

  //禁用DMA传输
  dma_channel_disable(DMA0, DMA_CH4);

  //复位DMA0通道
  dma_deinit(DMA0, DMA_CH4);

  //配置DMA参数
  dma_multi_data_para_struct_init(&dma_multi_initpara);
  dma_multi_initpara.periph_addr        = (u32)(&SPI_DATA(SPI1));      //外设地址
  dma_multi_initpara.periph_width       = DMA_PERIPH_WIDTH_16BIT;      //外设传输宽度
  dma_multi_initpara.periph_inc         = DMA_PERIPH_INCREASE_DISABLE; //外设地址固定
  dma_multi_initpara.memory0_addr       = (u32)dataBuf1;               //存储器0基地址
  dma_multi_initpara.memory_width       = DMA_MEMORY_WIDTH_16BIT;      //存储器传输宽度
  dma_multi_initpara.memory_inc         = DMA_MEMORY_INCREASE_ENABLE;  //使能储存器地址增长
  dma_multi_initpara.memory_burst_width = DMA_MEMORY_BURST_SINGLE;     //存储器突发类型
  dma_multi_initpara.periph_burst_width = DMA_PERIPH_BURST_SINGLE;     //外设突发类型
  dma_multi_initpara.critical_value     = DMA_FIFO_4_WORD;             //FIFO深度
  dma_multi_initpara.circular_mode      = DMA_CIRCULAR_MODE_ENABLE;    //循环模式
  dma_multi_initpara.direction          = DMA_MEMORY_TO_PERIPH;        //读存储器写外设
  dma_multi_initpara.number             = len;                         //传输数据量
  dma_multi_initpara.priority           = DMA_PRIORITY_LOW;            //软件优先级
  dma_multi_data_mode_init(DMA0, DMA_CH4, &dma_multi_initpara);        //根据参数配置DMA通道
  dma_channel_subperipheral_select(DMA0, DMA_CH4, DMA_SUBPERI0);       //外设使能
  dma_flow_controller_config(DMA0, DMA_CH4, DMA_FLOW_CONTROLLER_DMA);  //DMA作为传输控制器

  //开启DMA双缓冲区
  dma_switch_buffer_mode_config(DMA0, DMA_CH4, (u32)dataBuf2, DMA_MEMORY_0);
  dma_switch_buffer_mode_enable(DMA0, DMA_CH4, ENABLE);

  //开启DMA传输
  dma_channel_enable(DMA0, DMA_CH4);
}

/*********************************************************************************************************
* 函数名称：I2SDualDMATransmit32
* 函数功能：I2S使用DMA发送32位数据
* 输入参数：dataBuf1，dataBuf2：双缓冲区首地址，len：缓冲区长度（按照32位数据计算）
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：使用DMA双缓冲区发送数据，首先发送的是缓冲区1中的数据，然后是缓冲区2的数据，交替循环
*********************************************************************************************************/
void I2SDualDMATransmit32(u32* dataBuf1, u32* dataBuf2, u32 len)
{
  dma_multi_data_parameter_struct dma_multi_initpara;

  //开启DMA0时钟
  rcu_periph_clock_enable(RCU_DMA0);

  //禁用DMA传输
  dma_channel_disable(DMA0, DMA_CH4);

  //复位DMA0通道
  dma_deinit(DMA0, DMA_CH4);

  //配置DMA参数
  dma_multi_data_para_struct_init(&dma_multi_initpara);
  dma_multi_initpara.periph_addr        = (u32)(&SPI_DATA(SPI1));      //外设地址
  dma_multi_initpara.periph_width       = DMA_PERIPH_WIDTH_16BIT;      //外设传输宽度
  dma_multi_initpara.periph_inc         = DMA_PERIPH_INCREASE_DISABLE; //外设地址固定
  dma_multi_initpara.memory0_addr       = (u32)dataBuf1;               //存储器0基地址
  dma_multi_initpara.memory_width       = DMA_MEMORY_WIDTH_32BIT;      //存储器传输宽度
  dma_multi_initpara.memory_inc         = DMA_MEMORY_INCREASE_ENABLE;  //使能储存器地址增长
  dma_multi_initpara.memory_burst_width = DMA_MEMORY_BURST_SINGLE;     //存储器突发类型
  dma_multi_initpara.periph_burst_width = DMA_PERIPH_BURST_SINGLE;     //外设突发类型
  dma_multi_initpara.critical_value     = DMA_FIFO_4_WORD;             //FIFO深度
  dma_multi_initpara.circular_mode      = DMA_CIRCULAR_MODE_ENABLE;    //循环模式
  dma_multi_initpara.direction          = DMA_MEMORY_TO_PERIPH;        //读存储器写外设
  dma_multi_initpara.number             = len;                         //传输数据量
  dma_multi_initpara.priority           = DMA_PRIORITY_LOW;            //软件优先级
  dma_multi_data_mode_init(DMA0, DMA_CH4, &dma_multi_initpara);        //根据参数配置DMA通道
  dma_channel_subperipheral_select(DMA0, DMA_CH4, DMA_SUBPERI0);       //外设使能
  dma_flow_controller_config(DMA0, DMA_CH4, DMA_FLOW_CONTROLLER_DMA);  //DMA作为传输控制器

  //开启DMA双缓冲区
  dma_switch_buffer_mode_config(DMA0, DMA_CH4, (u32)dataBuf2, DMA_MEMORY_0);
  dma_switch_buffer_mode_enable(DMA0, DMA_CH4, ENABLE);

  //开启DMA传输
  dma_channel_enable(DMA0, DMA_CH4);
}

/*********************************************************************************************************
* 函数名称：I2SGetTransmitNotUsedMemoryAddr
* 函数功能：获取发送DMA未在使用的内存地址
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：void
*********************************************************************************************************/
u32 I2SGetTransmitNotUsedMemoryAddr(void)
{
  if(DMA_MEMORY_0 == dma_using_memory_get(DMA0, DMA_CH4))
  {
    return DMA_CHM1ADDR(DMA0, DMA_CH4);
  }
  else
  {
    return DMA_CHM0ADDR(DMA0, DMA_CH4);
  }
}

/*********************************************************************************************************
* 函数名称：I2SEndDMATransmit
* 函数功能：结束DMA传输
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：该函数会等待最后一批数据传输完毕后再结束传输
*********************************************************************************************************/
void I2SEndDMATransmit(void)
{
  //清除发送完成标志位
  dma_flag_clear(DMA0, DMA_CH4, DMA_FLAG_FTF);

  //等待最后一批数据传输完成
  while(SET != dma_flag_get(DMA0, DMA_CH4, DMA_FLAG_FTF)){}

  //禁用DMA传输
  dma_channel_disable(DMA0, DMA_CH4);
}

/*********************************************************************************************************
* 函数名称：I2SDualDMAReceive16
* 函数功能：I2S使用DMA接收16位数据
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：使用DMA双缓冲区接收数据，首先填充的是缓冲区1中的数据，然后是缓冲区2的数据，交替循环
*********************************************************************************************************/
void I2SDualDMAReceive16(u16* dataBuf1, u16* dataBuf2, u32 len)
{
  dma_multi_data_parameter_struct dma_multi_initpara;

  //开启DMA0时钟
  rcu_periph_clock_enable(RCU_DMA0);

  //禁用DMA传输
  dma_channel_disable(DMA0, DMA_CH3);

  //复位DMA0通道
  dma_deinit(DMA0, DMA_CH3);

  //配置DMA参数
  dma_multi_data_para_struct_init(&dma_multi_initpara);
  dma_multi_initpara.periph_addr        = (u32)(&I2S_ADD_DATA(I2S1_ADD)); //外设地址
  dma_multi_initpara.periph_width       = DMA_PERIPH_WIDTH_16BIT;         //外设传输宽度
  dma_multi_initpara.periph_inc         = DMA_PERIPH_INCREASE_DISABLE;    //外设地址固定
  dma_multi_initpara.memory0_addr       = (u32)dataBuf1;                  //存储器0基地址
  dma_multi_initpara.memory_width       = DMA_MEMORY_WIDTH_16BIT;         //存储器传输宽度
  dma_multi_initpara.memory_inc         = DMA_MEMORY_INCREASE_ENABLE;     //使能储存器地址增长
  dma_multi_initpara.memory_burst_width = DMA_MEMORY_BURST_SINGLE;        //存储器突发类型
  dma_multi_initpara.periph_burst_width = DMA_PERIPH_BURST_SINGLE;        //外设突发类型
  dma_multi_initpara.critical_value     = DMA_FIFO_4_WORD;                //FIFO深度
  dma_multi_initpara.circular_mode      = DMA_CIRCULAR_MODE_ENABLE;       //循环模式
  dma_multi_initpara.direction          = DMA_PERIPH_TO_MEMORY;           //外设到存储器
  dma_multi_initpara.number             = len;                            //传输数据量
  dma_multi_initpara.priority           = DMA_PRIORITY_LOW;               //软件优先级
  dma_multi_data_mode_init(DMA0, DMA_CH3, &dma_multi_initpara);           //根据参数配置DMA通道
  dma_channel_subperipheral_select(DMA0, DMA_CH3, DMA_SUBPERI3);          //外设使能
  dma_flow_controller_config(DMA0, DMA_CH3, DMA_FLOW_CONTROLLER_DMA);     //DMA作为传输控制器

  //开启DMA双缓冲区
  dma_switch_buffer_mode_config(DMA0, DMA_CH3, (u32)dataBuf2, DMA_MEMORY_0);
  dma_switch_buffer_mode_enable(DMA0, DMA_CH3, ENABLE);

  //开启DMA传输
  dma_channel_enable(DMA0, DMA_CH3);
}

/*********************************************************************************************************
* 函数名称：I2SGetReceiveNotUsedMemoryAddr
* 函数功能：获取接收DMA未在使用的内存地址
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：void
*********************************************************************************************************/
u32 I2SGetReceiveNotUsedMemoryAddr(void)
{
  if(DMA_MEMORY_0 == dma_using_memory_get(DMA0, DMA_CH3))
  {
    return DMA_CHM1ADDR(DMA0, DMA_CH3);
  }
  else
  {
    return DMA_CHM0ADDR(DMA0, DMA_CH3);
  }
}

/*********************************************************************************************************
* 函数名称：I2SEndDMAReceive
* 函数功能：结束DMA接收
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：该函数会等待最后一批数据传输完毕后再结束传输
*********************************************************************************************************/
void I2SEndDMAReceive(void)
{
  //清除发送完成标志位
  dma_flag_clear(DMA0, DMA_CH3, DMA_FLAG_FTF);

  //等待最后一批数据传输完成
  while(SET != dma_flag_get(DMA0, DMA_CH3, DMA_FLAG_FTF)){}

  //禁用DMA传输
  dma_channel_disable(DMA0, DMA_CH3);
}
