/*********************************************************************************************************
* 模块名称：LAN8720.c
* 摘    要：LAN8720以太网网卡驱动模块
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
#include "LAN8720.h"
#include "gd32f470x_conf.h"
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

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigRCU(void);					 //配置RCU时钟
static void ConfigGPIO(void);					 //配置GPIO
static void Reset(void);							 //复位LAN8720
	
/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigRCU
* 函数功能：配置RCU时钟
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ConfigRCU(void)
{
  rcu_periph_clock_enable(RCU_SYSCFG);
  rcu_periph_clock_enable(RCU_ENET);
  rcu_periph_clock_enable(RCU_ENETTX);
  rcu_periph_clock_enable(RCU_ENETRX);
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOB);
  rcu_periph_clock_enable(RCU_GPIOC);
  rcu_periph_clock_enable(RCU_GPIOG);
}

/*********************************************************************************************************
* 函数名称：ConfigGPIO
* 函数功能：配置GPIO
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ConfigGPIO(void)
{
  //ETH_MDIO
  gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_2);
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_2);

  //ETH_MDC
  gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_1);
  gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
  gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_1);

  //ETH_RMII_TXD0
  gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_13);
  gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
  gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_13);

  //ETH_RMII_TXD1
  gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_14);
  gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_14);
  gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_14);

  //ETH_RMII_TX_EN
  gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_11);
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_11);

  //ETH_RMII_RXD0
  gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_4);
  gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
  gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_4);

  //ETH_RMII_RXD1
  gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_5);
  gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
  gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_5);

  //ETH_RMII_CRS_DV
  gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_7);
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_7);

  //ETH_RMII_REF_CLK
  gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_1);
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_1);

  //ETH_RST
  gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_3);
  gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
  gpio_bit_set(GPIOC, GPIO_PIN_3);
}

/*********************************************************************************************************
* 函数名称：Reset
* 函数功能：复位LAN8720
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void Reset(void)
{  
  gpio_bit_reset(GPIOC, GPIO_PIN_3);
  DelayNms(100);
  gpio_bit_set(GPIOC, GPIO_PIN_3);
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitLAN8720
* 函数功能：初始化以太网网卡驱动模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitLAN8720(void)
{
  ErrStatus status = ERROR;

  //使能RCU
  ConfigRCU();

  //配置GPIO
  ConfigGPIO();

  //硬复位LAN8720
  Reset();

  //配置使用RMII
  syscfg_enet_phy_interface_config(SYSCFG_ENET_PHY_RMII);

  //复位以太网模块
  enet_deinit();

  //软复位
  status = enet_software_reset();
  if(ERROR == status)
  {
    while(1){}
  }

  //配置以太网
  // enet_initpara_config(HALFDUPLEX_OPTION, ENET_CARRIERSENSE_ENABLE|ENET_RECEIVEOWN_ENABLE|ENET_RETRYTRANSMISSION_DISABLE|ENET_BACKOFFLIMIT_10|ENET_DEFERRALCHECK_DISABLE);
  // enet_initpara_config(DMA_OPTION, ENET_FLUSH_RXFRAME_ENABLE | ENET_SECONDFRAME_OPT_ENABLE | ENET_NORMAL_DESCRIPTOR);
  status = enet_init(ENET_AUTO_NEGOTIATION, ENET_AUTOCHECKSUM_DROP_FAILFRAMES, ENET_BROADCAST_FRAMES_PASS);
  if(ERROR == status)
  {
    while(1){}
  }
}
