/************************************************************
 * Copyright: 2025 CIMC.
 * File: SPI2.h
 * Platform: 2025CIMC IHD-V04
 * Version: CIMC     2026/5/10     V0.01    original
 ************************************************************/

#include "gd30ad3344.h"
#include "SPI2.h"
#include "UART1.h"
#include "OLED.h"
#include "SysTick.h"

//用电压表标定
#define TEMP_K  121.134991
#define TEMP_B  -257.603098

//用V1ADC采集标定
//#define TEMP_K  121.808203
//#define TEMP_B  -260.882515

#define VOLT_LEN 5

GD30AD3344 GD30AD3344_InitStruct;   //AD3344配置结构体
static float s_Voltage[VOLT_LEN] = {0};     //ADC采样电压缓存数组

/************************* 内部函数声明 *************************/
static void ConfigAD3344CS(void);
static u16  AD3344Transfer16(u16 half_word);
static void ad3344_ExtRef(void);
static float GetMedianFloat(float *buf, u8 len);

/************************* 内部函数定义 *************************/
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
   
   GD30AD3344_CS_LOW();
   DelayNms(1);
   
   tx_data = 0x8106;
   AD3344Transfer16(tx_data);
   DelayNms(1);
   
   tx_data = addr;
   AD3344Transfer16(tx_data);
   DelayNms(1);
   
   rdval = AD3344Transfer16(0x00);
   DelayNms(1);
   
   GD30AD3344_CS_HIGH();
   DelayNms(1);
   
   val = rdval | 0x40;
   
   GD30AD3344_CS_LOW();
   DelayNms(1);
   
   tx_data = 0x8100;
   AD3344Transfer16(tx_data);
   
   tx_data = addr;
   AD3344Transfer16(tx_data);
   
   tx_data = val;
   AD3344Transfer16(tx_data);
   DelayNms(1);
   
   GD30AD3344_CS_HIGH();
   DelayNms(1);
}

//计算某个数组中的中位数
static float GetMedianFloat(float *buf, u8 len)
{
    float vol[VOLT_LEN];
    float t;
    u8 i, j;
    if (len == 0) {
        return 0.0f;
    }

    for (i = 0; i < len; i++) {
        vol[i] = buf[i];
    }

		//冒泡排序
    for (i = 0; i < len - 1; i++) {
        for (j = i + 1; j < len; j++) {
            if (vol[i] > vol[j]) {
                t = vol[i];
                vol[i] = vol[j];
                vol[j] = t;
            }
        }
    }
		
    return vol[len / 2];
}


/************************* API??????? *************************/
void GD30AD3344_Init(void)
{
    SPI2_Init(SPI_CK_PL_LOW_PH_2EDGE, SPI_PSC_64);
    ConfigAD3344CS();
    // ad3344_ExtRef();    //配置AIN3作为外部基准源

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

//读取电压AD值
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

    return result;
}

//温度传感器采样100ms调用一次
void  GD30AD3344Task(void)
{
    static u8 s_cnt = 0;
    float vol = 0;
    vol = GD30AD3344_AD_Read(GD30AD3344_MUX_AIN0_GND, GD30AD3344_PGA_4V096);
    s_Voltage[s_cnt] = vol;
    s_cnt = (s_cnt + 1) % VOLT_LEN;
}

//获取温度
//获取温度
float GD30AD3344_GetTemperature(void)
{
    float vol = 0;
    float temp = 0;
    float R = 0;

    vol = GetMedianFloat(s_Voltage, VOLT_LEN); //取中位数电压(中值滤波)

    //电压转电阻
    R = 47.823236f * vol + -0.010013f;

    //PT100电阻转温度
    temp = (R - 100.0f) / 0.385055f;

    OLED_ShowString(0,0,(u8*)"R:",16);
    OLED_ShowNum(32,0,(u32)R,3,16);
    OLED_ShowChar(64,0,'.',16);
    OLED_ShowNum(80,0,(u32)(R * 100) % 100,2,16);
    printf("R: %f, vol: %f, temp: %f\r\n", R, vol, temp);

    return temp;
}


