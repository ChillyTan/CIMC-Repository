/************************************************************
 * 版权：2025CIMC Copyright。 
 * 文件：Function.c
 * 作者: Lingyu Meng
 * 平台: 2025CIMC IHD-V04
 * 版本: Lingyu Meng     2025/2/16     V0.01    original
************************************************************/


/************************* 头文件 *************************/

#include "Function.h"

/************************* 宏定义 *************************/


/************************ 变量定义 ************************/


/************************ 内部函数声明 ********************/
static void Proc2msTask(void);
static void Proc1sTask(void);

/************************ 内部函数定义 ********************/

//2ms任务处理
static void Proc2msTask(void)
{
	if(Get2msFlag())
	{
		LEDFlicker(250);
		Clr2msFlag();
	}
}

//1s任务处理
static void Proc1sTask(void)
{
	if(Get1secFlag())
	{
		
		Clr1secFlag();
	}
}

/************************ 外部函数定义 ************************/
/************************************************************ 
 * Function :       System_Init
 * Comment  :       用于初始化MCU
 * Parameter:       null
 * Return   :       null
 * Author   :       Lingyu Meng
 * Date     :       2025-02-30 V0.1 original
************************************************************/
void System_Init(void)
{
	InitSysTick();
	LED_Init();
}


/************************************************************ 
 * Function :       UsrFunction
 * Comment  :       用户程序主循环
 * Parameter:       null
 * Return   :       null
 * Author   :       Lingyu Meng
 * Date     :       2025-02-30 V0.1 original
************************************************************/
void UsrFunction(void)
{
	while(1)
	{
		Proc2msTask();
		Proc1sTask();
	}
}


/****************************End*****************************/

