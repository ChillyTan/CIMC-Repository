/************************************************************
 * Copyright: 2025 CIMC.
 * File: SPI2.h
 * Platform: 2025CIMC IHD-V04
 * Version: CIMC     2026/5/10     V0.01    original
 ************************************************************/
#include "SPI2.h"

/************************* 外部函数定义 *************************/
void SPI2_Init(uint32_t clock_polarity_phase, uint32_t prescale)
{
    spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(SPI2_GPIO_RCU);
    rcu_periph_clock_enable(SPI2_RCU);

    gpio_af_set(SPI2_GPIO_PORT, SPI2_GPIO_AF, SPI2_SCK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN);
    gpio_mode_set(SPI2_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_SCK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN);
    gpio_output_options_set(SPI2_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI2_SCK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN);

    spi_i2s_deinit(SPI2_PORT);
    spi_struct_para_init(&spi_init_struct);
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = clock_polarity_phase;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = prescale;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI2_PORT, &spi_init_struct);

    spi_enable(SPI2_PORT);
}

u8 SPI2_ReadWriteByte(u8 data)
{
    u32 timeout;
    u8  rx_data;

    timeout = 0xFFFF;
    while(RESET == spi_i2s_flag_get(SPI2_PORT, SPI_FLAG_TBE)) {
        if(timeout-- == 0) {
            return 0xFF;
        }
    }

    spi_i2s_data_transmit(SPI2_PORT, data);

    timeout = 0xFFFF;
    while(SET == spi_i2s_flag_get(SPI2_PORT, SPI_FLAG_TRANS)) {
        if(timeout-- == 0) {
            return 0xFF;
        }
    }

    timeout = 0xFFFF;
    while(RESET == spi_i2s_flag_get(SPI2_PORT, SPI_FLAG_RBNE)) {
        if(timeout-- == 0) {
            return 0xFF;
        }
    }

    rx_data = (u8)spi_i2s_data_receive(SPI2_PORT);

    return rx_data;
}

u16 SPI2_ReadWriteHalfWord(u16 data)
{
    u8 rx_h;
    u8 rx_l;

    rx_h = SPI2_ReadWriteByte((u8)(data >> 8));
    rx_l = SPI2_ReadWriteByte((u8)(data & 0xFF));

    return ((u16)rx_h << 8) | rx_l;
}
