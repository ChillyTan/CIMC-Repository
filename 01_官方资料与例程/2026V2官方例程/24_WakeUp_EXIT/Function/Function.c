/************************************************************
 * 版权：2025CIMC Copyright。
 * 文件：Function.c
 * 作者: Jialei Zhao
 * 平台: 2025CIMC IHD-V04
 * 版本: Jialei Zhao     2026/1/30     V0.01    original
************************************************************/


/************************* 头文件 *************************/

#include "Function.h"
#include "LED.h"
#include "KEY.h"
#include "usart.h"



/************************* 宏定义 *************************/

#define RCU_MODIFY_4(__delay)   do{                                     \
                                    volatile uint32_t i, reg;           \
                                    if(0 != __delay){                   \
                                        /* Insert a software delay */   \
                                        for(i=0; i<__delay; i++){       \
                                        }                               \
                                        reg = RCU_CFG0;                 \
                                        reg &= ~(RCU_CFG0_AHBPSC);      \
                                        reg |= RCU_AHB_CKSYS_DIV2;      \
                                        /* AHB = SYSCLK/2 */            \
                                        RCU_CFG0 = reg;                 \
                                        /* Insert a software delay */   \
                                        for(i=0; i<__delay; i++){       \
                                        }                               \
                                        reg = RCU_CFG0;                 \
                                        reg &= ~(RCU_CFG0_AHBPSC);      \
                                        reg |= RCU_AHB_CKSYS_DIV4;      \
                                        /* AHB = SYSCLK/4 */            \
                                        RCU_CFG0 = reg;                 \
                                        /* Insert a software delay */   \
                                        for(i=0; i<__delay; i++){       \
                                        }                               \
                                        reg = RCU_CFG0;                 \
                                        reg &= ~(RCU_CFG0_AHBPSC);      \
                                        reg |= RCU_AHB_CKSYS_DIV8;      \
                                        /* AHB = SYSCLK/8 */            \
                                        RCU_CFG0 = reg;                 \
                                        /* Insert a software delay */   \
                                        for(i=0; i<__delay; i++){       \
                                        }                               \
                                        reg = RCU_CFG0;                 \
                                        reg &= ~(RCU_CFG0_AHBPSC);      \
                                        reg |= RCU_AHB_CKSYS_DIV16;     \
                                        /* AHB = SYSCLK/16 */           \
                                        RCU_CFG0 = reg;                 \
                                        /* Insert a software delay */   \
                                        for(i=0; i<__delay; i++){       \
                                        }                               \
                                    }                                   \
                                }while(0)

/************************ 变量定义 ************************/

static void rcu_config(void);

/************************ 函数定义 ************************/



/************************************************************
 * Function :       System_Init
 * Comment  :       用于初始化MCU
 * Parameter:       null
 * Return   :       null
 * Author   :      	Jialei Zhao
 * Date     :       2026-01-30 V0.1 original
************************************************************/

void System_Init(void)
{
	systick_config();     // 时钟配置

	LED_Init();        // LED初始化
	KEY_Init();        // 按键初始化
	usart_init();

}
/************************************************************
 * Function :       UsrFunction
 * Comment  :       用户程序功能: LED1闪烁
 * Parameter:       null
 * Return   :       null
 * Author   :      	Jialei Zhao
 * Date     :       2026-01-30 V0.1 original
************************************************************/
void UsrFunction(void)
{
	rcu_periph_clock_enable(RCU_PMU);

	LED1_ON();

	delay_1ms(2000);

	LED1_OFF();

	pmu_to_deepsleepmode(PMU_LDO_LOWPOWER , PMU_LOWDRIVER_ENABLE , WFI_CMD);

	//!进入深度睡眠模式唤醒后， 此时 他会从这里开始执行 ， 然后时钟会被 自动切换到 IRC16M（内部 16MHz RC 振荡器）作为系统时钟，
	//!如果睡眠前使用的是 HXTAL（外部晶振）或 PLL（锁相环），这些时钟源在深度睡眠期间已被关闭，唤醒后需要你重新使能并配置它们。
	rcu_config();	//!重新配置时钟到240MHZ

	while (1)
	{
		usart_recv_buf();

		LED_Toggle(GPIOA , GPIO_PIN_5);

		delay_1ms(500);
	}
}


/************************************************************
 * Function :       _soft_delay_
 * Comment  :       软件延时函数
 * Parameter:       time: 延时时间，单位：ms
 * Return   :       null
 * Author   :      	Jialei Zhao
 * Date     :       2026-01-30 V0.1 original
************************************************************/
static void _soft_delay_(uint32_t time)
{
	__IO uint32_t i;
	for (i = 0; i < time * 10; i++)
	{
	}
}

/************************************************************
 * Function :       rcu_config
 * Comment  :       用于配置时钟到240MHZ
 * Parameter:       null
 * Return   :       null
 * Author   :      	Jialei Zhao
 * Date     :       2026-01-30 V0.1 original
************************************************************/
static void rcu_config(void)
{
	uint32_t timeout = 0U;
	uint32_t stab_flag = 0U;

	//!时钟切换demo里面的
	/* It is strongly recommended to include it to avoid issues caused by self-removal. */
	RCU_MODIFY_4(0x50);
	/* select HXTAL as system clock source, deinitialize the RCU */
	rcu_system_clock_source_config(RCU_CKSYSSRC_HXTAL);
	/* It is strongly recommended to include it to avoid issues caused by self-removal. */
	_soft_delay_(200);
	rcu_deinit();

	/* enable HXTAL */
	RCU_CTL |= RCU_CTL_HXTALEN;

	/* wait until HXTAL is stable or the startup time is longer than HXTAL_STARTUP_TIMEOUT */
	do
	{
		timeout++;
		stab_flag = (RCU_CTL & RCU_CTL_HXTALSTB);
	} while ((0U == stab_flag) && (HXTAL_STARTUP_TIMEOUT != timeout));

	/* if fail */
	if (0U == (RCU_CTL & RCU_CTL_HXTALSTB))
	{
		while (0U == (RCU_CTL & RCU_CTL_HXTALSTB))
		{
		}
	}

	RCU_APB1EN |= RCU_APB1EN_PMUEN;
	PMU_CTL |= PMU_CTL_LDOVS;

	/* HXTAL is stable */
	/* AHB = SYSCLK */
	RCU_CFG0 |= RCU_AHB_CKSYS_DIV1;
	/* APB2 = AHB/2 */
	RCU_CFG0 |= RCU_APB2_CKAHB_DIV2;
	/* APB1 = AHB/4 */
	RCU_CFG0 |= RCU_APB1_CKAHB_DIV4;

	/* Configure the main PLL, PSC = 25, PLL_N = 480, PLL_P = 2, PLL_Q = 10 */
	RCU_PLL = (25U | (480U << 6U) | (((2U >> 1U) - 1U) << 16U) |
		(RCU_PLLSRC_HXTAL) | (10U << 24U));

	/* enable PLL */
	RCU_CTL |= RCU_CTL_PLLEN;

	/* wait until PLL is stable */
	while (0U == (RCU_CTL & RCU_CTL_PLLSTB))
	{
	}

	/* Enable the high-drive to extend the clock frequency to 240 Mhz */
	PMU_CTL |= PMU_CTL_HDEN;
	while (0U == (PMU_CS & PMU_CS_HDRF))
	{
	}

	/* select the high-drive mode */
	PMU_CTL |= PMU_CTL_HDS;
	while (0U == (PMU_CS & PMU_CS_HDSRF))
	{
	}

	/* select PLL as system clock */
	RCU_CFG0 &= ~RCU_CFG0_SCS;
	RCU_CFG0 |= RCU_CKSYSSRC_PLLP;

	/* wait until PLL is selected as system clock */
	while (0U == (RCU_CFG0 & RCU_SCSS_PLLP))
	{
	}
}


/****************************End*****************************/
