/*********************************************************************************************************
* 模块名称：ADC.c
* 摘    要：ADC模块
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
#include "ADC.h"
#include "gd32f470x_conf.h"
#include "U16Queue.h"
#include "SysTick.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static unsigned short   s_arrADCData;                //存放ADC转换结果数据
static StructU16CirQue  s_structADCCirQue;           //ADC循环队列
static unsigned short   s_arrADCBuf[ADC_BUF_SIZE];   //ADC循环队列的缓冲区

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigRCU(void);                                       //使能设备时钟
static void ConfigDMA1CH0ForADC0(void);                            //配置DMA1的通道0
static void ConfigTIMER0(unsigned short arr, unsigned short psc);  //配置TIMER0
static void ConfigADC0(void);                                      //配置ADC0

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigRCU
* 函数功能：配置时钟
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注  意：
**********************************************************************************************************/
static void ConfigRCU(void)
{
  //使能GPIOC\ADC0\DMA1\TIMER0时钟
  rcu_periph_clock_enable(RCU_GPIOC);
  adc_clock_config(ADC_ADCCK_HCLK_DIV10);//配置ADC时钟24MHz
  rcu_periph_clock_enable(RCU_ADC0);
  rcu_periph_clock_enable(RCU_DMA1);
  rcu_periph_clock_enable(RCU_TIMER0);
}

/*********************************************************************************************************
* 函数名称：ConfigDMA0CH0ForADC0
* 函数功能：配置DMA
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注  意：
**********************************************************************************************************/
static void ConfigDMA1CH0ForADC0(void)
{
  //DMA初始化结构体
  dma_single_data_parameter_struct dma_init_struct;
  
  dma_deinit(DMA1, DMA_CH0);                                       //初始化结构体设置默认值
  dma_init_struct.direction  = DMA_PERIPH_TO_MEMORY;               //设置DMA数据传输方向
  dma_init_struct.memory0_addr  = (uint32_t)&s_arrADCData;         //内存地址设置
  dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_DISABLE;      //内存增长使能
  dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_16BIT;    //传输位宽16位
  dma_init_struct.number     = 1;                                  //内存数据量设置
  dma_init_struct.periph_addr  = (uint32_t)&(ADC_RDATA(ADC0));     //外设地址设置
  dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;      //外设地址增长失能
  dma_init_struct.priority   = DMA_PRIORITY_ULTRA_HIGH;            //优先级设置
	dma_init_struct.circular_mode       = DMA_CIRCULAR_MODE_ENABLE;  //使能循环模式
  dma_single_data_mode_init(DMA1, DMA_CH0, &dma_init_struct);      //初始化结构体
	dma_channel_subperipheral_select(DMA1, DMA_CH0, DMA_SUBPERI0);   //DMA通道外设选择
	
  nvic_irq_enable(DMA1_Channel0_IRQn, 1, 1);                       //中断线配置
  dma_interrupt_enable(DMA1, DMA_CH0, DMA_CHXCTL_FTFIE);           //使能DMA中断
      
  //使能DMA
  dma_channel_enable(DMA1, DMA_CH0);
}

/*********************************************************************************************************
* 函数名称：ConfigTIMER0
* 函数功能：初始化定时器
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注  意：
**********************************************************************************************************/
static void ConfigTIMER0(unsigned short arr, unsigned short psc)
{
  //初始化结构体
  timer_oc_parameter_struct timer_ocintpara;
  timer_parameter_struct timer_initpara;
  
  //初始化结构体设置默认值
  timer_deinit(TIMER0);

  timer_initpara.prescaler         = psc;                                           //设置分频
  timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;                            //设置对齐模式
  timer_initpara.counterdirection  = TIMER_COUNTER_UP;                              //设置计数模式
  timer_initpara.period            = arr;                                           //设置重装载值
  timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;                              //设置时钟分割
  timer_initpara.repetitioncounter = 0;   
  timer_init(TIMER0, &timer_initpara);                                              //初始化结构体

  timer_ocintpara.ocpolarity  = TIMER_OC_POLARITY_LOW;                              //通道输出极性设置
  timer_ocintpara.outputstate = TIMER_CCX_ENABLE;                                   //通道输出状态设置
  timer_channel_output_config(TIMER0, TIMER_CH_0, &timer_ocintpara);                //通道输出初始化

  timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, 100);                 //通道选择
  timer_channel_output_mode_config(TIMER0, TIMER_CH_0, TIMER_OC_MODE_PWM1);         //通道输出模式配置
  timer_channel_output_shadow_config(TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);  //失能比较影子寄存器

  timer_auto_reload_shadow_enable(TIMER0);                                          //自动重载影子使能
  timer_primary_output_config(TIMER0, ENABLE);                                      //TIMER0通道输出使能
  
  timer_enable(TIMER0);                                                             //使能TIMER0
}

/*********************************************************************************************************
* 函数名称：ConfigADC0
* 函数功能：初始化ADC0设备
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注  意：
**********************************************************************************************************/
static void ConfigADC0(void)
{
  //配置GPIO模式为模拟输入
  gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_2);  
  
  //配置ADC0
  adc_dma_mode_enable(ADC0);                                        //使用DMA
  adc_dma_request_after_last_enable(ADC0);                          //每个规则转换结束时产生一个DMA请求
  adc_resolution_config(ADC0, ADC_RESOLUTION_12B);                  //规则组配置，12位分辨率
  adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);             //右对齐
  adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1);          //设置规则组长度
	
  //使能外部触发，
  adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
  
  //选择外部触发
  adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_T0_CH0);
  
  //ADC引脚输入
  adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_12, ADC_SAMPLETIME_480);

  //ADC0使能
  adc_enable(ADC0);

  //延时等待10ms
  DelayNms(10);

  //使能ADC0校准
  adc_calibration_enable(ADC0);
}

/*********************************************************************************************************
* 函数名称：DMA1_Channel0_IRQHandler
* 函数功能：DMA1中断
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注  意：
**********************************************************************************************************/
void DMA1_Channel0_IRQHandler(void)
{
  if(RESET != dma_interrupt_flag_get(DMA1, DMA_CH0, DMA_INT_FLAG_FTF))
  {
    WriteADCBuf(s_arrADCData);   //向ADC缓冲区写入数据
    
    //清除标志位
    dma_interrupt_flag_clear(DMA1, DMA_CH0, DMA_INT_FLAG_FTF);
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitADC
* 函数功能：初始化ADC模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注  意：
**********************************************************************************************************/
void InitADC(void)
{
  //时钟配置
  ConfigRCU();
  
  //DMA0CH0配置
  ConfigDMA1CH0ForADC0();
  
  //TIMER0配置
  ConfigTIMER0(7999, 239);
  
  //ADC0配置
  ConfigADC0();
  
  //初始化ADC缓冲区
  InitU16Queue(&s_structADCCirQue, s_arrADCBuf, ADC_BUF_SIZE);
}

/*********************************************************************************************************
* 函数名称：WriteADCBuf
* 函数功能：向ADC缓冲区写入数据
* 输入参数：d-待写入的数据
* 输出参数：void
* 返 回 值：成功标志位，1为成功，0为不成功
* 创建日期：2022年01月01日
* 注  意：
**********************************************************************************************************/
unsigned char WriteADCBuf(unsigned short d)
{
  unsigned char ok = 0;                       //将读取成功标志位的值设置为0

  ok = EnU16Queue(&s_structADCCirQue, &d, 1); //入队

  return ok;                                  //返回读取成功标志位的值
}

/*********************************************************************************************************
* 函数名称：ReadADCBuf
* 函数功能：从ADC缓冲区读取数据
* 输入参数：p-读取的数据存放的首地址
* 输出参数：void
* 返 回 值：成功标志位，1为成功，0为不成功
* 创建日期：2022年01月01日
* 注  意：
**********************************************************************************************************/
unsigned char ReadADCBuf(unsigned short* p)
{
  unsigned char ok = 0;                      //将读取成功标志位的值设置为0

  ok = DeU16Queue(&s_structADCCirQue, p, 1); //出队

  return ok;                                 //返回读取成功标志位的值
}

