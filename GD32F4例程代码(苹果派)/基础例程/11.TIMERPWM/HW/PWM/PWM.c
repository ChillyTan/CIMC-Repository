/*********************************************************************************************************
* 模块名称：PWM.c
* 摘    要：PWM模块
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
#include "PWM.h"
#include "gd32f470x_conf.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量定义
*********************************************************************************************************/
static  signed short s_iDutyCycle = 0;  //用于存放占空比

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigTIMER2ForPWMPB4(unsigned short arr, unsigned short psc);  //配置PWM

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigTIMER2ForPWMPB4
* 函数功能：配置TIMER2
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注    意：
*********************************************************************************************************/
static void ConfigTIMER2ForPWMPB4(unsigned short arr, unsigned short psc)
{
  //定义初始化结构体变量
  timer_oc_parameter_struct timer_ocinitpara;
  timer_parameter_struct timer_initpara;
  
  rcu_periph_clock_enable(RCU_GPIOB);    //使能GPIOB时钟
  rcu_periph_clock_enable(RCU_TIMER2);   //使能TIMER2时钟
   
  gpio_af_set(GPIOB, GPIO_AF_2, GPIO_PIN_4);
	gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);                //上拉输出
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_4);    //推挽输出

  timer_deinit(TIMER2);                                  //将TIMER2配置为默认值
  timer_struct_para_init(&timer_initpara);               //timer_initpara配置为默认值
    
  timer_initpara.prescaler         = psc;                //设置预分频值
  timer_initpara.alignedmode       = TIMER_COUNTER_EDGE; //设置对齐模式
  timer_initpara.counterdirection  = TIMER_COUNTER_UP;   //设置向上计数
  timer_initpara.period            = arr;                //设置重装载值
  timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;   //设置时钟分频因子
  timer_initpara.repetitioncounter = 0;                  //设置重复计数值
  timer_init(TIMER2, &timer_initpara);                   //初始化定时器
    
  //将结构体参数初始化为默认值
  timer_channel_output_struct_para_init(&timer_ocinitpara);  
    
  timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE;                    //设置通道输出状态
  timer_ocinitpara.outputnstate = TIMER_CCXN_DISABLE;                  //设置互补通道输出状态
  timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;              //设置通道输出极性
  timer_ocinitpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;             //设置互补通道输出极性
  timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;             //设置空闲状态下通道输出极性
  timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;            //设置空闲状态下互补通道输出极性
  timer_channel_output_config(TIMER2, TIMER_CH_0, &timer_ocinitpara);  //初始化结构体
     
  timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_0, 0);                   //设置占空比
  timer_channel_output_mode_config(TIMER2, TIMER_CH_0, TIMER_OC_MODE_PWM0);         //设置通道比较模式
  timer_channel_output_shadow_config(TIMER2, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);  //失能比较影子寄存器
  timer_auto_reload_shadow_enable(TIMER2);                                          //自动重载影子使能 

  timer_enable(TIMER2);  //使能定时器
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitPWM
* 函数功能：初始化PWM模块
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注    意：
*********************************************************************************************************/
void  InitPWM(void)
{
  ConfigTIMER2ForPWMPB4(999, 1199);  //配置TIMER2，240000000/(1199+1)/(999+1)=200Hz
}

/*********************************************************************************************************
* 函数名称：SetPWM
* 函数功能：设置占空比
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注    意：
*********************************************************************************************************/
void SetPWM(signed short val)
{
  s_iDutyCycle = val;                   //获取占空比的值
  
  timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_0, s_iDutyCycle);  //设置占空比
}

/*********************************************************************************************************
* 函数名称：IncPWMDutyCycle
* 函数功能：递增占空比，每次递增方波周期的1/10，直至高电平输出 
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注    意：
*********************************************************************************************************/
void IncPWMDutyCycle(void)
{
  if(s_iDutyCycle >= 1000)               //如果占空比不小于1000
  {                                     
    s_iDutyCycle = 1000;                 //保持占空比值为1000
  }                                     
  else                                  
  {                                     
    s_iDutyCycle += 100;                 //占空比递增方波周期的1/10
  }
  
  timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_0, s_iDutyCycle);  //设置占空比
}

/*********************************************************************************************************
* 函数名称：DecPWMDutyCycle
* 函数功能：递减占空比，每次递减方波周期的1/10，直至低电平输出 
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2022年01月01日
* 注    意：
*********************************************************************************************************/
void DecPWMDutyCycle(void)
{
  if(s_iDutyCycle <= 0)               //如果占空比不大于0
  {                                   
    s_iDutyCycle = 0;                 //保持占空比值为0
  }                                   
  else                                
  {                                   
    s_iDutyCycle -= 100;               //占空比递减方波周期的1/10
  }
  
  timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_0, s_iDutyCycle);//设置占空比
}
