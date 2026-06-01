#include "sleep.h"
#include "systick.h"
#include "usart.h"
#include <stdio.h>

void sleep_init(void)
{
    rcu_periph_clock_enable(RCU_PMU);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_SYSCFG);
    
    // 配置PA0为输入，上拉
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_0);
    
    // 配置PA0为外部中断
    syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN0);
    exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    nvic_irq_enable(EXTI0_IRQn, 1, 0);
    
    // 使能唤醒引脚
    pmu_wakeup_pin_enable();
}

void sleep_enter_deepsleep(void)
{
    // 休眠前打印
    printf("即将休眠\r\n");
    delay_1ms(100);  // 确保打印完成
    
    rcu_periph_clock_enable(RCU_PMU);
    
    // 进入深度睡眠
    pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, PMU_LOWDRIVER_ENABLE, WFI_CMD);
    
    // 唤醒后重新配置
    systick_config();
    usart0_init();  // 重新初始化串口
    
    // 唤醒后打印
    printf("成功苏醒\r\n");
}