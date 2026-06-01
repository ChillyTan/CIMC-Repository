#ifndef __SLEEP_H
#define __SLEEP_H

#include "gd32f4xx.h"

void sleep_init(void);     // 初始化睡眠功能
void sleep_enter_deepsleep(void);  // 进入深度睡眠

#endif