#ifndef __RTC_H
#define __RTC_H

#include "DataType.h"

void RTC_Init(void);
void rtc_setup(void);
void rtc_show_time(void);
void rtc_show_alarm(void);
u8 usart_input_threshold(u32 value);
void rtc_pre_config(void);

#endif
