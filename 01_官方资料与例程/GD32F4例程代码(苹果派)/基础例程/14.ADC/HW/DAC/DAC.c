/*********************************************************************************************************
* 模块名称：DAC.c
* 摘    要：DAC模块
* 当前版本：1.0.0
* 作    者：Leyutek(COPYRIGHT 2018 - 2022 Leyutek. All rights reserved.)
* 完成日期：2022年01月01日
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
#include "DAC.h"
#include "gd32f470x_conf.h"
#include "Wave.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define DAC1_R12DH_ADDR ((unsigned int)0x40007414)   //DAC1的地址（12位右对齐）
	
/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructDACWave s_strDAC1WaveBuf;  //存储DAC1波形属性，包括波形地址和点数

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigTimer5(unsigned short arr, unsigned short psc);  //配置TIMER5
static void ConfigDAC1(void);                                      //配置DAC1
static void ConfigDMA0CH6ForDAC1(StructDACWave wave);              //配置DMA0的通道6

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigTimer5
* 函数功能：配置TIMER5
* 输入参数：arr-自动重装值，psc-预分频器值
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注    意：TIMER5时钟为240MHz
*********************************************************************************************************/
static  void ConfigTimer5(unsigned short arr, unsigned short psc)
{
  timer_parameter_struct timer_initpara;     //timer_initpara用于存放定时器的参数

  //使能RCU相关时钟
  rcu_periph_clock_enable(RCU_TIMER5);      //使能TIMER5的时钟

  timer_deinit(TIMER5);                     //设置TIMER5参数恢复默认值
  timer_struct_para_init(&timer_initpara);  //初始化timer_initpara
 
  //配置TIMER5
  timer_initpara.prescaler         = psc;               //设置预分频器值
  timer_initpara.counterdirection  = TIMER_COUNTER_UP;  //设置向上计数模式
  timer_initpara.period            = arr;               //设置自动重装载值
  timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;  //设置时钟分割
  timer_init(TIMER5, &timer_initpara);                  //根据参数初始化定时器

  //TIMER5触发源配置
  timer_master_output_trigger_source_select(TIMER5, TIMER_TRI_OUT_SRC_UPDATE);

  timer_enable(TIMER5);                                 //使能定时器
}

/*********************************************************************************************************
* 函数名称：ConfigDAC1
* 函数功能：配置DAC1
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigDAC1(void)
{
  //使能RCU相关时钟
  rcu_periph_clock_enable(RCU_GPIOA);  //使能GPIOA的时钟
  rcu_periph_clock_enable(RCU_DAC);    //使能DAC的时钟
  
  //设置GPIO
  gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_5);

  //DAC1配置
  dac_deinit();                                         //复位DAC模块
  dac_concurrent_disable();                             //禁用concurrent mode
  dac_output_buffer_enable(DAC1);                       //使能输出缓冲区
  dac_trigger_enable(DAC1);                             //使能外部触发源
  dac_trigger_source_config(DAC1, DAC_TRIGGER_T5_TRGO); //使用TIMER5做为触发源
  dac_wave_mode_config(DAC1, DAC_WAVE_DISABLE);         //禁用Wave mode
	
  //使能DAC1的DMA
  dac_dma_enable(DAC1);
  
  //使能DAC1
  dac_enable(DAC1);
}

/*********************************************************************************************************
* 函数名称：ConfigDMA0CH6ForDAC1
* 函数功能：配置DMA0_CH6
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigDMA0CH6ForDAC1(StructDACWave wave)
{
  //DMA配置结构体
  dma_single_data_parameter_struct dma_struct;

  //使能DMA0时钟
  rcu_periph_clock_enable(RCU_DMA0);

  //配置DMA0_CH6
  dma_deinit(DMA0, DMA_CH6);                                    //复位DMA
  dma_single_data_para_struct_init(&dma_struct);                //复位配置结构体
  dma_struct.periph_addr         = DAC1_R12DH_ADDR;             //外设地址
  dma_struct.periph_memory_width = DMA_PERIPH_WIDTH_16BIT;      //外设数据位宽为16位
  dma_struct.memory0_addr        = wave.waveBufAddr;            //内存地址
  dma_struct.number              = wave.waveBufSize;            //传输数据量
  dma_struct.priority            = DMA_PRIORITY_HIGH;           //高优先级
  dma_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE; //外设地址增长关闭
  dma_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;  //内存地址增长开启
  dma_struct.direction           = DMA_MEMORY_TO_PERIPH;        //传输方向为内存到地址
	dma_struct.circular_mode       = DMA_CIRCULAR_MODE_ENABLE;    //使能循环模式
  dma_single_data_mode_init(DMA0, DMA_CH6, &dma_struct);        //根据参数配置DMA0_CH6
  
	dma_channel_subperipheral_select(DMA0, DMA_CH6, DMA_SUBPERI7);//DMA通道外设选择
	
  //使能DMA0_CH6
  dma_channel_enable(DMA0, DMA_CH6); 
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitDAC
* 函数功能：初始化DAC模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注    意：
*********************************************************************************************************/
void InitDAC(void)
{
  s_strDAC1WaveBuf.waveBufAddr  = (unsigned int)GetSineWave100PointAddr(); //波形地址
  s_strDAC1WaveBuf.waveBufSize  = 100;                                     //波形点数
  
  ConfigTimer5(7999, 239);                 //配置定时器做为DAC1触发源
	ConfigDAC1();                            //配置DAC1
  ConfigDMA0CH6ForDAC1(s_strDAC1WaveBuf);  //配置DMA0的通道6
}
             
/*********************************************************************************************************
* 函数名称：SetDACWave
* 函数功能：设置DAC波形属性 
* 输入参数：wave，包括波形地址和点数
* 输出参数：void
* 返 回 值：void
* 创建日期：2018年01月01日
* 注    意：
*********************************************************************************************************/
void SetDACWave(StructDACWave wave)
{
	ConfigDMA0CH6ForDAC1(wave);//根据wave配置DMA0的通道6	
}
