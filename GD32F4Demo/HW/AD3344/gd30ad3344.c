/************************************************************
 * Copyright: 2025 CIMC.
 * File: SPI2.h
 * Platform: 2025CIMC IHD-V04
 * Version: CIMC     2026/5/10     V0.01    original
 ************************************************************/

#include "gd30ad3344.h"
#include "SPI2.h"
#include "UART0.h"

GD30AD3344 GD30AD3344_InitStruct;

/************************* ??????????? *************************/
static void ConfigAD3344CS(void);
static u16  AD3344Transfer16(u16 half_word);
static void ad3344_ExtRef(void);

/************************* ?????????? *************************/
static void ConfigAD3344CS(void)
{
    rcu_periph_clock_enable(GD30AD3344_CS_RCU);
    gpio_mode_set(GD30AD3344_CS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GD30AD3344_CS_PIN);
    gpio_output_options_set(GD30AD3344_CS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GD30AD3344_CS_PIN);
    GD30AD3344_CS_HIGH();
}

static u16 AD3344Transfer16(u16 half_word)
{
    GD30AD3344_CS_LOW();
    half_word = SPI2_ReadWriteHalfWord(half_word);
    GD30AD3344_CS_HIGH();

    return half_word;
}

//配置AIN3作为外部参考
void ad3344_ExtRef(void)
{
    uint16_t addr,val,rdval;
    uint16_t tx_data;
    
    addr = 0x10 + 0x4;
    
    SPI_CLR_CS();
    delay_us(1000);
    
    tx_data = 0x8106;
    ad3344_spi_txrx16bit(tx_data);
    delay_us(1000);
    
    tx_data = addr;
    ad3344_spi_txrx16bit(tx_data);
    delay_us(1000);
    
    rdval = ad3344_spi_txrx16bit(0x00);
    delay_us(1000);
    
    SPI_SET_CS();
    delay_us(1000);
    
    val = rdval | 0x40;
    
    SPI_CLR_CS();
    delay_us(1000);
    
    tx_data = 0x8100;
    ad3344_spi_txrx16bit(tx_data);
    
    tx_data = addr;
    ad3344_spi_txrx16bit(tx_data);
    
    tx_data = val;
    ad3344_spi_txrx16bit(tx_data);
    delay_us(1000);
    
    SPI_SET_CS();
    delay_us(1000);
}

/************************* API??????? *************************/
void GD30AD3344_Init(void)
{
    SPI2_Init(SPI_CK_PL_LOW_PH_2EDGE, SPI_PSC_64);
    ConfigAD3344CS();

    GD30AD3344_InitStruct.SS         = 0;
    GD30AD3344_InitStruct.MUX        = GD30AD3344_MUX_AIN0_GND;
    GD30AD3344_InitStruct.PGA        = GD30AD3344_PGA_4V096;
    GD30AD3344_InitStruct.MODE       = GD30AD3344_MODE_CONTINUOUS;
    GD30AD3344_InitStruct.DR         = GD30AD3344_DR_12_5SPS;
    GD30AD3344_InitStruct.RESERVED_1 = 0;
    GD30AD3344_InitStruct.PULL_UP_EN = 1;
    GD30AD3344_InitStruct.NOP        = 1;
    GD30AD3344_InitStruct.RESERVED   = 1;

    (void)AD3344Transfer16(GD30AD3344_InitStruct_Value);

    printf("AD3344 init cfg=0x%04X\r\n", GD30AD3344_InitStruct_Value);
}

float ADS118_PGA_SET(GD30AD3344_PGA_TypeDef PGA)
{
    switch (PGA) {
    case GD30AD3344_PGA_6V144:
        return 6.144f;
    case GD30AD3344_PGA_4V096:
        return 4.096f;
    case GD30AD3344_PGA_2V048:
        return 2.048f;
    case GD30AD3344_PGA_1V024:
        return 1.024f;
    case GD30AD3344_PGA_0V512:
        return 0.512f;
    case GD30AD3344_PGA_0V256:
        return 0.256f;
    case GD30AD3344_PGA_0V064:
        return 0.064f;
    default:
        return 2.048f;
    }
}

float GD30AD3344_AD_Read(GD30AD3344_Channel_TypeDef CH, GD30AD3344_PGA_TypeDef Ref)
{
    uint16_t raw_data;
    int16_t signed_data;
    float result;

    GD30AD3344_InitStruct.MUX = CH;
    GD30AD3344_InitStruct.PGA = Ref;

    raw_data = AD3344Transfer16(GD30AD3344_InitStruct_Value);
    signed_data = (int16_t)raw_data;
    result = (float)signed_data * ADS118_PGA_SET(Ref) / 32768.0f;

    printf("AD3344 raw=0x%04X, val=%f\r\n", raw_data, result);

    return result;
}








