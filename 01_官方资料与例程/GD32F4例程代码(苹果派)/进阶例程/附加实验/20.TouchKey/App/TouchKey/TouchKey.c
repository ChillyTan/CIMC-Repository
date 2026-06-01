/*********************************************************************************************************
* 模块名称：TouchKey.c
* 摘    要：触摸按键驱动模块
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
#include "TouchKey.h"
#include "gd32f470x_conf.h"
#include "SysTick.h"
#include "Timer.h"
#include "stdio.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define TOUCH_GATE_VAL 20 //触摸的门限值，也就是必须大于s_iTouchDefaultValue + TOUCH_GATE_VAL才认为是有效触摸

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static u16 s_iTouchDefaultValue = 0; //按键未按下时的输入捕获值
static u16 s_iKeyValue          = 0; //当前按键输入捕获值

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigTouchKey(void);  //配置触摸按键

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigTouchKey
* 函数功能：配置触摸按键
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*          1、触摸按键使用了TIM11的CH1
*          2、仅使能定时器更新中断
*********************************************************************************************************/
static void ConfigTouchKey(void)
{
  timer_parameter_struct    timer_initpara;
  timer_ic_parameter_struct timer_icintpara;

  //使能RCC相关时钟
  rcu_periph_clock_enable(RCU_GPIOC);   //使能GPIOC的时钟
  rcu_periph_clock_enable(RCU_TIMER11); //使能TIM11时钟

  //配置PB15为复用推挽
  gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_15);
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_15);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_15);

  //复位TIM11
  timer_deinit(TIMER11);

  //配置TIM11
  timer_struct_para_init(&timer_initpara);               //设置默认参数
  timer_initpara.period            = 999;                //设置自动重装载值（1ms）
  timer_initpara.prescaler         = 199;                //设置预分频器值（1MHz）
  timer_initpara.alignedmode       = TIMER_COUNTER_EDGE; //设置边沿对齐
  timer_initpara.counterdirection  = TIMER_COUNTER_UP;   //设置向上计数模式
  timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;   //设置时钟分割
  timer_initpara.repetitioncounter = 0;                  //设置重复计数
  timer_init(TIMER11, &timer_initpara);                  //根据参数初始化定时器

  //配置TIM11 CH1输入捕获
  timer_channel_input_struct_para_init(&timer_icintpara);           //设置默认参数
  timer_icintpara.icfilter    = 0;                                  //配置滤波器
  timer_icintpara.icpolarity  = TIMER_IC_POLARITY_RISING;           //上升沿捕获
  timer_icintpara.icprescaler = TIMER_IC_PSC_DIV1;                  //设置预分频
  timer_icintpara.icselection = TIMER_IC_SELECTION_DIRECTTI;        //ic0映射到CI0
  timer_input_capture_config(TIMER11, TIMER_CH_1, &timer_icintpara); //根据参数配置TIM11 CH1

  //使能影子寄存器自动重装载
  timer_auto_reload_shadow_enable(TIMER11);

  //使能定时器的更新中断
  timer_interrupt_enable(TIMER11, TIMER_INT_UP);

  //定时器中断NVIC使能
  nvic_irq_enable(TIMER7_BRK_TIMER11_IRQn, 0, 0);

  //使能TIM11
  timer_enable(TIMER11);
}

/*********************************************************************************************************
* 函数名称：TIMER7_BRK_TIMER11_IRQHandler
* 函数功能：TIM11中断服务函数
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*         1、每隔3ms获取一次电容触摸按键输入捕获结果
*         2、以25ms为一个周期，在这个周期内取输入捕获值最大值
*********************************************************************************************************/
void TIMER7_BRK_TIMER11_IRQHandler(void)
{
  static u8  s_iCapTimeCnt      = 0;
  static u8  s_iKeyTimeCnt      = 0;
  static u8  s_iCapture         = 0;
  static u16 s_iMaxCaptureValue = 0;

  if (timer_interrupt_flag_get(TIMER11, TIMER_INT_FLAG_UP) == SET)  //判断定时器更新中断是否发生
  {
    timer_interrupt_flag_clear(TIMER11, TIMER_INT_FLAG_UP);         //清除定时器更新中断标志

    //更新检测结果到s_iKeyValue，并将最大值清零
    if(0 == s_iKeyTimeCnt)
    {
      s_iKeyValue = s_iMaxCaptureValue;
      s_iMaxCaptureValue = 0;
    }
    s_iKeyTimeCnt = (s_iKeyTimeCnt + 1) % 25;

    //获取按键输入捕获值
    switch (s_iCapTimeCnt)
    {

    //电容触摸按键放电
    case 0:
      gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_15);            //上拉输出
      gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_15); //推挽输出
      gpio_bit_reset(GPIOB, GPIO_PIN_15);                                             //拉低
      break;

    //清空定时器输入捕获标志位，配置电容触摸按键引脚为浮空输入
    //此时定时器计数值为0，所以不用清空定时器计数值
    case 1:
      gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_15);
      gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_15);
      gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_15);
      break;
    
    //获取输入捕获检测结果，并保存最大值
    case 2:
      s_iCapture = timer_channel_capture_value_register_read(TIMER11, TIMER_CH_1);
      if(s_iCapture > s_iMaxCaptureValue)
      {
        s_iMaxCaptureValue = s_iCapture;
      }
      break;
    
    default:
      break;
    }
    s_iCapTimeCnt = (s_iCapTimeCnt + 1) % 3;
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitTouchKey
* 函数功能：初始化触摸按键驱动模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitTouchKey(void)
{
  ConfigTouchKey(); //配置触摸按键

  //获取按键未按下时的输入捕获值
  while(1)
  {
    s_iTouchDefaultValue = s_iKeyValue;
    if((s_iTouchDefaultValue > 0) && (s_iTouchDefaultValue < 100))
    {
      break;
    }
    else
    {
      printf("InitTouchKey: Falt to get default value\r\n");
      printf("InitTouchKey: value = %d\r\n", s_iTouchDefaultValue);
    }
    DelayNms(500);
  }

  printf("Touck key default value: %d\r\n", s_iTouchDefaultValue);
}

/*********************************************************************************************************
* 函数名称：ScanTouchKey
* 函数功能：触摸按键扫描
* 输入参数：keyDown：按下回调函数（不用请填入NULL），keyUp：抬起回调函数（不用请填入NULL）
* 输出参数：void
* 返 回 值：0-按键未按下，其它-按键按下时长（ms）
* 创建日期：2021年07月01日
* 注    意：keyDown将返回按键抬起时长（ms），keyUp将返回按键按下时长
*********************************************************************************************************/
u64 ScanTouchKey(void(*keyDown)(u64), void(*keyUp)(u64))
{
  static EnumTouchKeyState s_enumKeyLastState  = TOUCH_KEY_UP; //上一次检测结果
  static EnumTouchKeyState s_enumKeyNewState   = TOUCH_KEY_UP; //此次测量结果
  static u64               s_iKeyDownBeginTime = 0;            //按键按下时刻的系统时钟
  static u64               s_iKeyUpBeginTime   = 0;            //按键抬起时刻的系统时钟
  u64 currentTime;

  //获取系统时钟
  currentTime = GetSysTime();

  //判断按键状态
  if(s_iKeyValue > (s_iTouchDefaultValue + TOUCH_GATE_VAL))
  {
    s_enumKeyNewState = TOUCH_KEY_DOWN;
  }
  else
  {
    s_enumKeyNewState = TOUCH_KEY_UP;
  }

  //按下处理
  if((TOUCH_KEY_UP == s_enumKeyLastState) && (TOUCH_KEY_DOWN == s_enumKeyNewState))
  {
    //记录按键按下时刻系统时间
    s_iKeyDownBeginTime = currentTime;

    //回调处理
    if(NULL != keyDown)
    {
      if(0 == s_iKeyUpBeginTime)
      {
        keyDown(0);
      }
      else
      {
        keyDown(currentTime - s_iKeyUpBeginTime);
      }
    }
  }

  //抬起处理
  if((TOUCH_KEY_DOWN == s_enumKeyLastState) && (TOUCH_KEY_UP == s_enumKeyNewState))
  {
    //记录按键抬起时刻时钟
    s_iKeyUpBeginTime = currentTime;

    //回调处理
    if(NULL != keyUp)
    {
      if(0 == s_iKeyDownBeginTime)
      {
        keyUp(0);
      }
      else
      {
        keyUp(currentTime - s_iKeyDownBeginTime);
      }
    }
  }

  //保存按键状态
  s_enumKeyLastState = s_enumKeyNewState;

  //按键按下时返回按下时长
  if(TOUCH_KEY_DOWN == s_enumKeyNewState)
  {
    return currentTime - s_iKeyDownBeginTime;
  }

  //按键未按下
  else
  {
    return 0;
  }
}
