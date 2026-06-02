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
  SystemInit();        //系统初始化
  InitRCU();           //初始化RCU模块
  InitNVIC();          //初始化NVIC模块
  InitTimer();         //初始化Timer模块
  InitSysTick();       //初始化SysTick模块
	InitLED();           //初始化LED模块
  InitUART0(115200);   //初始化UART模块
  InitKeyOne();        //初始化按键模块
  InitProcKeyOne();    //初始化按键处理模块
	OLED_Init();         //初始化OLED模块
	RTC_Init();          //初始化RTC模块
	spi_flash_init();    //初始化SPI_FLASH模块
	GD30AD3344_Init();	 //初始化AD3344模块
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
	System_Init();
	


	while(1)
	{
		Proc2msTask();
		Proc1sTask();
	}
}


/****************************End*****************************/

