/************************************************************
 * 版权：2025CIMC Copyright。 
 * 文件：led.c
 * 作者: Lingyu Meng
 * 平台: 2025CIMC IHD-V04
 * 版本: Lingyu Meng     2025/2/16     V0.01    original
************************************************************/

/************************* 头文件 *************************/

#include "LED.h"

/************************ 全局变量定义 ************************/


/************************************************************ 
 * Function :       LED_Init
 * Comment  :       用于初始化LED端口
 * Parameter:       null
 * Return   :       null
 * Author   :       Lingyu Meng
 * Date     :       2025-02-30 V0.1 original
************************************************************/

void LED_Init(void)
{
	rcu_periph_clock_enable(RCU_GPIOA);    // 初始化GPIO_A总线时钟
	
	//初始化LED1~4端口（批量初始化端口）
	gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);   			// GPIO模式设置为输出
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);     // 输出参数设置
	gpio_bit_reset(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);  										     	// 引脚初始电平为低电平
}


//LED闪烁
void LEDFlicker(uint16_t cnt)
{
	static uint16_t s_iCnt = 0;
  s_iCnt++;                      //计数器的计数值加1
  
  if(s_iCnt >= cnt)              //计数器的计数值大于cnt
  {
    s_iCnt = 0;                  //重置计数器的计数值为0
	
		//LED状态取反，实现LED闪烁
		gpio_bit_write(GPIOA, GPIO_PIN_4, (bit_status)(1 - gpio_output_bit_get(GPIOA, GPIO_PIN_4)));
    gpio_bit_write(GPIOA, GPIO_PIN_5, (bit_status)(1 - gpio_output_bit_get(GPIOA, GPIO_PIN_5)));
    gpio_bit_write(GPIOA, GPIO_PIN_6, (bit_status)(1 - gpio_output_bit_get(GPIOA, GPIO_PIN_6)));
		gpio_bit_write(GPIOA, GPIO_PIN_7, (bit_status)(1 - gpio_output_bit_get(GPIOA, GPIO_PIN_7)));
  }
}


/****************************End*****************************/

