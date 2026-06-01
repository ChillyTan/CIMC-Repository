/*********************************************************************************************************
* 模块名称：BreathLED.c
* 摘    要：呼吸灯模块
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
#include "BreathLED.h"
#include "gd32f470x_conf.h"
#include "math.h"
#include "Timer.h"
#include "LED.h"

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
static void ConfigBreathLEDGPIO(void); //配置呼吸灯的GPIO
static void ConfigBreathLedPWM(void);  //配置呼吸灯的PWM

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigBreathLEDGPIO
* 函数功能：配置呼吸灯的GPIO
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ConfigBreathLEDGPIO(void)
{
  //使能RCU相关时钟
  rcu_periph_clock_enable(RCU_TIMER2); //使能TIM2时钟
  rcu_periph_clock_enable(RCU_GPIOB);  //使能GPIOA
  
  gpio_af_set(GPIOB, GPIO_AF_2, GPIO_PIN_5);
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_5);
}

/*********************************************************************************************************
* 函数名称：ConfigBreathLedPWM
* 函数功能：配置呼吸灯的PWM
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ConfigBreathLedPWM(void)
{
  timer_oc_parameter_struct timer_ocintpara;
  timer_parameter_struct timer_initpara;

  //使能RCU相关时钟
  rcu_periph_clock_enable(RCU_TIMER2); //使能TIM0时钟

  //复位TIM2
  timer_deinit(TIMER2);

  //配置TIM2
  timer_initpara.prescaler         = 199;                //设置预分频值
  timer_initpara.alignedmode       = TIMER_COUNTER_EDGE; //设置边沿对齐模式
  timer_initpara.counterdirection  = TIMER_COUNTER_UP;   //设置向上计数模式
  timer_initpara.period            = 499;                //设置自动重装载值
  timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;   //设置时钟分割
  timer_initpara.repetitioncounter = 0;                  //设置重复计数器
  timer_init(TIMER2, &timer_initpara);                   //根据参数初始化定时器

  //配置TIM2 CH1
  timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;                   //PWM输出使能
  timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;                 //禁止比较输出
  timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;             //输出极性位高
  timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;            //比较输出极性为高
  timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;            //空闲状态通道输出极性
  timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;           //空闲状态比较通道输出极性
  timer_channel_output_config(TIMER2, TIMER_CH_1, &timer_ocintpara); //根据参数配置定时器通道

  //设定PWM值
  timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_1, 0);

  //PWM0模式
  timer_channel_output_mode_config(TIMER2, TIMER_CH_1, TIMER_OC_MODE_PWM0);

  //禁用通道影子寄存器
  timer_channel_output_shadow_config(TIMER2, TIMER_CH_1, TIMER_OC_SHADOW_DISABLE);

  //PWM输出使能
  timer_primary_output_config(TIMER2, ENABLE);

  //使能TIM2
  timer_enable(TIMER2);
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitBreathLED
* 函数功能：初始化呼吸灯模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitBreathLED(void)
{
  ConfigBreathLEDGPIO(); //配置呼吸灯的GPIO
  ConfigBreathLedPWM();  //配置呼吸灯的PWM
}

/*********************************************************************************************************
* 函数名称：BreathLEDTask
* 函数功能：呼吸灯任务
* 输入参数：period：呼吸灯周期（ms）
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*          1、调用频率越快呼吸灯周期越精确
*          2、以y=e^(-x^2)曲线做呼吸灯，x范围取-2到2，范围-3到3可以实现由亮至灭的效果
*********************************************************************************************************/
void BreathLEDTask(u32 period)
{
  static u64 s_iLastCycleBeginTime = 0; //上一次循环开始时间
  static u64 s_iFirstFlag = 1;          //第一次执行此函数标志位
  u64    currenTime;  //现在时间
  u64    elapsedTime; //一个循环中流经的时间
  double x;           //弧度
  double rate;        //比例
  u32    pwm;         //PWM值

  //初始化
  if(1 == s_iFirstFlag)
  {
    s_iFirstFlag = 0;
    s_iLastCycleBeginTime = GetSysTime();
    return;
  }

  //获取系统时间
  currenTime = GetSysTime();

  //计算一个周期内流经的时间
  elapsedTime = currenTime - s_iLastCycleBeginTime;

  //计算x值
  x = -2.0 + 4.0 * elapsedTime / period;

  //计算PWM值比例
  rate = exp(- x * x);

  //最终PWM数值
  pwm = TIMER_CAR(TIMER2) * rate;

  //配置PWM输出
  timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_1, pwm);

  //一个周期结束
  if(elapsedTime >= period)
  {
    s_iLastCycleBeginTime = currenTime;
  }
}

