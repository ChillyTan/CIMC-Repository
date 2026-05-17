/************************************************************
 * ?????2025CIMC Copyright??
 * ?????gd30ad3344.h
 * ????: CIMC
 * ??: 2025CIMC IHD-V04
 * ???: CIMC     2026/5/10     V0.01    original
 ************************************************************/

#ifndef __GD30AD3344_H
#define __GD30AD3344_H

/************************* ???? *************************/
#include "gd32f470x_conf.h"
#include "DataType.h"

/************************* ???? *************************/
#define GD30AD3344_CS_RCU               RCU_GPIOB
#define GD30AD3344_CS_PORT              GPIOB
#define GD30AD3344_CS_PIN               GPIO_PIN_6

#define GD30AD3344_CS_LOW()             gpio_bit_reset(GD30AD3344_CS_PORT, GD30AD3344_CS_PIN)
#define GD30AD3344_CS_HIGH()            gpio_bit_set(GD30AD3344_CS_PORT, GD30AD3344_CS_PIN)

/*=========================================================================
 * Auxiliary Definition
 */
    #define AD3344_DUAL_END                                 (0)
    #define AD3344_SINGLE_END                               (1)
/*========================================================================= */

/*=========================================================================
    Reset Value for Module GD30AD3344
    -----------------------------------------------------------------------*/
#define AD3344_CONVERSION_RESET                             ((uint32_t)0x0)
#define AD3344_CONFIG_RESET                                 ((uint32_t)0x58b)
/*=========================================================================*/

/******************  Bit Definition for Register CONVERSION  *********************/
#define AD3344_CONVERSION_CNVDATA_Msk               ((uint32_t)0xffff)      /* bit mask, */
#define AD3344_CONVERSION_CNVDATA_Pos               ((uint32_t)0)           /*bit position, */

/******************  Bit Definition for Register CONFIG  *********************/
#define AD3344_CONFIG_NOP_Msk                       ((uint32_t)0x6)         /* bit mask, GD30AD3344??NOP??*/
#define AD3344_CONFIG_NOP_Pos                       ((uint32_t)1)           /*bit position, GD30AD3344??NOP??*/
#define AD3344_CONFIG_PULL_UP_EN_Msk                ((uint32_t)0x8)         /* bit mask, DOUT???????*/
#define AD3344_CONFIG_PULL_UP_EN_Pos                ((uint32_t)3)           /*bit position, DOUT???????*/
#define AD3344_CONFIG_DR_Msk                        ((uint32_t)0xe0)        /* bit mask, Data rate*/
#define AD3344_CONFIG_DR_Pos                        ((uint32_t)5)           /*bit position, Data rate*/
#define AD3344_CONFIG_MODE_Msk                      ((uint32_t)0x100)       /* bit mask, Device operating mode*/
#define AD3344_CONFIG_MODE_Pos                      ((uint32_t)8)           /*bit position, Device operating mode*/
#define AD3344_CONFIG_PGA_Msk                       ((uint32_t)0xe00)       /* bit mask, Programmable gain amplifier configuration*/
#define AD3344_CONFIG_PGA_Pos                       ((uint32_t)9)           /*bit position, Programmable gain amplifier configuration*/
#define AD3344_CONFIG_MUX_Msk                       ((uint32_t)0x7000)      /* bit mask, input multiplexer configuration*/
#define AD3344_CONFIG_MUX_Pos                       ((uint32_t)12)          /*bit position, input multiplexer configuration*/
#define AD3344_CONFIG_OS_Msk                        ((uint32_t)0x8000)      /* bit mask, This bit is used to start a single conversion..OS can only be written when in
                                                                               power-down state and has no effect when a conversion is ongoing.Always reads back 0 (default).*/
#define AD3344_CONFIG_OS_Pos                        ((uint32_t)15)          /*bit position, This bit is used to start a single conversion..OS can only be written when in
                                                                               power-down state and has no effect when a conversion is ongoing.Always reads back 0 (default).*/

/* Register 0x00 (CONVERSION) definition
 * ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |  Bit 15  |  Bit 14  |  Bit 13  |  Bit 12  |  Bit 11  |  Bit 10  |   Bit 9  |   Bit 8  |   Bit 7  |   Bit 6  |   Bit 5  |   Bit 4  |   Bit 3  |   Bit 2  |   Bit 1  |   Bit 0  |
 * ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |                                                                                    CONV[15:0]                                                                                   |
 * ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    /* CONVERSION register address */
    #define AD3344_CONVERSION_ADDRESS                                              ((uint8_t) 0x00)

    /* CONVERSION default (reset) value */
    #define AD3344_CONVERSION_DEFAULT                                              ((uint16_t) 0x0000)

    /* CONVERSION register field masks */
    #define AD3344_CONVERSION_CONV_MASK                                            ((uint16_t) 0xFFFF)


/* Register 0x01 (CONFIG) definition
 * ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |  Bit 15  |  Bit 14  |  Bit 13  |  Bit 12  |  Bit 11  |  Bit 10  |   Bit 9  |   Bit 8  |   Bit 7  |   Bit 6  |   Bit 5  |   Bit 4  |   Bit 3  |   Bit 2  |   Bit 1  |   Bit 0  |
 * ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * |    SS    |            MUX[2:0]            |            PGA[2:0]            |   MODE   |             DR[2:0]            | RESERVED |PULL_UP_EN|       NOP[1:0]      | RESERVED |
 * ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */

    #define AD3344_CONFIG_ADDRESS                                                  ((uint8_t) 0x01)

    /* CONFIG default (reset) value */
    #define AD3344_CONFIG_DEFAULT                                                  ((uint16_t) 0x058b)

    #define AD3344_REG_CONFIG_OS_MASK                       (0x8000)
    #define AD3344_REG_CONFIG_OS_SINGLE                     (0x8000)        // Write: Set to start a single-conversion
    #define AD3344_REG_CONFIG_OS_BUSY                       (0x0000)        // Read: Bit = 0 when conversion is in progress
    #define AD3344_REG_CONFIG_OS_NOTBUSY                    (0x8000)        // Read: Bit = 1 when device is not performing a conversion

    #define AD3344_REG_CONFIG_MUX_MASK                      (0x7000)
    #define AD3344_REG_CONFIG_MUX_DIFF_0_1                  (0x0000)        // Differential P = AIN0, N = AIN1 (default)
    #define AD3344_REG_CONFIG_MUX_DIFF_0_3                  (0x1000)        // Differential P = AIN0, N = AIN3
    #define AD3344_REG_CONFIG_MUX_DIFF_1_3                  (0x2000)        // Differential P = AIN1, N = AIN3
    #define AD3344_REG_CONFIG_MUX_DIFF_2_3                  (0x3000)        // Differential P = AIN2, N = AIN3
    #define AD3344_REG_CONFIG_MUX_SINGLE_0                  (0x4000)        // Single-ended AIN0
    #define AD3344_REG_CONFIG_MUX_SINGLE_1                  (0x5000)        // Single-ended AIN1
    #define AD3344_REG_CONFIG_MUX_SINGLE_2                  (0x6000)        // Single-ended AIN2
    #define AD3344_REG_CONFIG_MUX_SINGLE_3                  (0x7000)        // Single-ended AIN3

    #define AD3344_REG_CONFIG_PGA_MASK                      (0x0E00)
    #define AD3344_REG_CONFIG_PGA_6_144V                    (0x0000)        // +/-6.144V range = Gain 2/3
    #define AD3344_REG_CONFIG_PGA_4_096V                    (0x0200)        // +/-4.096V range = Gain 1
    #define AD3344_REG_CONFIG_PGA_2_048V                    (0x0400)        // +/-2.048V range = Gain 2 (default)
    #define AD3344_REG_CONFIG_PGA_1_024V                    (0x0600)        // +/-1.024V range = Gain 4
    #define AD3344_REG_CONFIG_PGA_0_512V                    (0x0800)        // +/-0.512V range = Gain 8
    #define AD3344_REG_CONFIG_PGA_0_256V                    (0x0A00)        // +/-0.256V range = Gain 16
    #define AD3344_REG_CONFIG_PGA_0_064V                    (0x0C00)        // +/-0.064V range = Gain 32

    #define AD3344_REG_CONFIG_MODE_MASK                     (0x0100)
    #define AD3344_REG_CONFIG_MODE_CONTIN                   (0x0000)        // Continuous conversion mode
    #define AD3344_REG_CONFIG_MODE_SINGLE                   (0x0100)        // Power-down single-shot mode (default)

    #define AD3344_REG_CONFIG_DR_MASK                       (0x00E0)
    #define AD3344_REG_CONFIG_DR_6_25SPS                    (0x0000)        // 6.25 samples per second
    #define AD3344_REG_CONFIG_DR_12_5SPS                    (0x0020)        // 12.5 samples per second
    #define AD3344_REG_CONFIG_DR_25SPS                      (0x0040)        // 25 samples per second
    #define AD3344_REG_CONFIG_DR_50SPS                      (0x0060)        // 50 samples per second
    #define AD3344_REG_CONFIG_DR_100SPS                     (0x0080)        // 100 samples per second (default)
    #define AD3344_REG_CONFIG_DR_250SPS                     (0x00A0)        // 250 samples per second
    #define AD3344_REG_CONFIG_DR_500SPS                     (0x00C0)        // 500 samples per second
    #define AD3344_REG_CONFIG_DR_1000SPS                    (0x00E0)        // 1000 samples per second

    #define AD3344_REG_CONFIG_PULL_UP_EN_MASK               (0x0008)
    #define AD3344_REG_CONFIG_PULL_UP_DIS                   (0x0000)        // Pullup resistor disabled on DOUT/DRDY pin
    #define AD3344_REG_CONFIG_PULL_UP_EN                    (0x0008)        // Pullup resistor enabled on DOUT/DRDY pin (default)

    #define AD3344_REG_CONFIG_NOP_MASK                      (0x0006)
    #define AD3344_REG_CONFIG_NOP_INV_0                     (0x0000)        // Invalid data, do not update the contents of the Config register
    #define AD3344_REG_CONFIG_NOP_VALID                     (0x0002)        // Valid data, update the Config register (default)
    #define AD3344_REG_CONFIG_NOP_INV_1                     (0x0004)        // Invalid data, do not update the contents of the Config register
    #define AD3344_REG_CONFIG_NOP_INV_2                     (0x0006)        // Invalid data, do not update the contents of the Config register
    
    #define AD3344_CONFIG_RESERVED_MASK                            ((uint16_t) 0x0001)
    #define AD3344_RESERVED_VALUE                                  ((uint16_t) 0x0001)

/************************* ??????G?? *************************/
typedef enum
{
    GD30AD3344_MUX_AIN0_AIN1 = 0,
    GD30AD3344_MUX_AIN0_AIN3 = 1,
    GD30AD3344_MUX_AIN1_AIN3 = 2,
    GD30AD3344_MUX_AIN2_AIN3 = 3,
    GD30AD3344_MUX_AIN0_GND  = 4,
    GD30AD3344_MUX_AIN1_GND  = 5,
    GD30AD3344_MUX_AIN2_GND  = 6,
    GD30AD3344_MUX_AIN3_GND  = 7
} GD30AD3344_Channel_TypeDef;

typedef enum
{
    GD30AD3344_PGA_6V144 = 0,
    GD30AD3344_PGA_4V096 = 1,
    GD30AD3344_PGA_2V048 = 2,
    GD30AD3344_PGA_1V024 = 3,
    GD30AD3344_PGA_0V512 = 4,
    GD30AD3344_PGA_0V256 = 5,
    GD30AD3344_PGA_0V064 = 6,
    GD30AD3344_PGA_RESERVED = 7
} GD30AD3344_PGA_TypeDef;

typedef enum
{
    GD30AD3344_MODE_CONTINUOUS = 0,
    GD30AD3344_MODE_SINGLE     = 1
} GD30AD3344_Mode_TypeDef;

typedef enum
{
    GD30AD3344_DR_6_25SPS = 0,
    GD30AD3344_DR_12_5SPS = 1,
    GD30AD3344_DR_25SPS   = 2,
    GD30AD3344_DR_50SPS   = 3,
    GD30AD3344_DR_100SPS  = 4,
    GD30AD3344_DR_250SPS  = 5,
    GD30AD3344_DR_500SPS  = 6,
    GD30AD3344_DR_1000SPS = 7
} GD30AD3344_DR_TypeDef;

typedef struct
{
    u16 SS;             /* bit15    OS/SS */
    u16 MUX;            /* bit14:12 */
    u16 PGA;            /* bit11:9  */
    u16 MODE;           /* bit8     */
    u16 DR;             /* bit7:5   */
    u16 RESERVED_1;     /* bit4     */
    u16 PULL_UP_EN;     /* bit3     */
    u16 NOP;            /* bit2:1   */
    u16 RESERVED;       /* bit0     */
} GD30AD3344;

extern GD30AD3344 GD30AD3344_InitStruct;

#define GD30AD3344_InitStruct_Value                                      \
    (u16)(                                                               \
        (((u16)GD30AD3344_InitStruct.SS         & 0x01U) << 15) |        \
        (((u16)GD30AD3344_InitStruct.MUX        & 0x07U) << 12) |        \
        (((u16)GD30AD3344_InitStruct.PGA        & 0x07U) << 9)  |        \
        (((u16)GD30AD3344_InitStruct.MODE       & 0x01U) << 8)  |        \
        (((u16)GD30AD3344_InitStruct.DR         & 0x07U) << 5)  |        \
        (((u16)GD30AD3344_InitStruct.RESERVED_1 & 0x01U) << 4)  |        \
        (((u16)GD30AD3344_InitStruct.PULL_UP_EN & 0x01U) << 3)  |        \
        (((u16)GD30AD3344_InitStruct.NOP        & 0x03U) << 1)  |        \
        (((u16)GD30AD3344_InitStruct.RESERVED   & 0x01U) << 0)           \
    )


/************************* API???????? *************************/
void  GD30AD3344_Init(void);
float GD30AD3344_AD_Read(GD30AD3344_Channel_TypeDef CH, GD30AD3344_PGA_TypeDef Ref);

#endif
