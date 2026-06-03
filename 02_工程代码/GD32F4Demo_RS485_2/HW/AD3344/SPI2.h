/************************************************************
 * Copyright: 2025 CIMC.
 * File: SPI2.h
 * Platform: 2025CIMC IHD-V04
 * Version: CIMC     2026/5/10     V0.01    original
 ************************************************************/

#ifndef __SPI2_H
#define __SPI2_H

#include "gd32f470x_conf.h"
#include "DataType.h"

#define SPI2_PORT                       SPI2
#define SPI2_RCU                        RCU_SPI2

#define SPI2_GPIO_RCU                   RCU_GPIOB
#define SPI2_GPIO_PORT                  GPIOB
#define SPI2_SCK_PIN                    GPIO_PIN_3
#define SPI2_MISO_PIN                   GPIO_PIN_4
#define SPI2_MOSI_PIN                   GPIO_PIN_5
#define SPI2_GPIO_AF                    GPIO_AF_6

void SPI2_Init(uint32_t clock_polarity_phase, uint32_t prescale);
u8   SPI2_ReadWriteByte(u8 data);
u16  SPI2_ReadWriteHalfWord(u16 data);

#endif
