/*********************************************************************************************************
* 模块名称：CAN.c
* 摘    要：CAN驱动模块
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
#include "CAN.h"
#include "gd32f470x_conf.h"
#include "Queue.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructCirQue  s_structCANRecCirQue;  //CAN接收循环队列
static unsigned char s_arrRecBuf[1024];   //CAN接收循环队列缓冲区

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static  void  ConfigCAN0(void);  //配置CAN0

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigCAN0
* 函数功能：配置CAN0
* 输入参数：void 
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：编号0-14为CAN0过滤器，15-29为CAN1过滤器
*********************************************************************************************************/
static  void  ConfigCAN0(void)
{
  can_parameter_struct        can_init_parameter;   //CAN初始化参数
  can_filter_parameter_struct can_filter_parameter; //CAN过滤器初始化参数

  //使能RCU相关时钟
  rcu_periph_clock_enable(RCU_CAN0);   //使能CAN时钟
  rcu_periph_clock_enable(RCU_GPIOA);  //使能GPIOA的时钟

  //配置CAN0 GPIO
  //TX
  gpio_af_set(GPIOA, GPIO_AF_9, GPIO_PIN_11);
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_11);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX,GPIO_PIN_11);

  //RX
  gpio_af_set(GPIOA, GPIO_AF_9, GPIO_PIN_12);
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,GPIO_PIN_12);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX,GPIO_PIN_12);

  //初始化参数结构体
  can_struct_para_init(CAN_INIT_STRUCT, &can_init_parameter);     //CAN参数设为默认值
  can_struct_para_init(CAN_FILTER_STRUCT, &can_filter_parameter); //过滤器参数设为默认值

  //初始化CAN0寄存器
  can_deinit(CAN0);
  
  //初始化CAN0参数
  can_init_parameter.time_triggered        = DISABLE;         //禁用时间触发通信
  can_init_parameter.auto_bus_off_recovery = DISABLE;         //通过软件手动地从离线状态恢复
  can_init_parameter.auto_wake_up          = DISABLE;         //通过软件手动的从睡眠工作模式唤醒
  can_init_parameter.auto_retrans          = DISABLE;         //使能自动重发
  can_init_parameter.rec_fifo_overwrite    = DISABLE;         //使能接收FIFO满时覆盖
  can_init_parameter.trans_fifo_order      = DISABLE;         //标识符（Identifier）较小的帧先发送
  can_init_parameter.working_mode          = CAN_NORMAL_MODE; //回环模式
  can_init_parameter.resync_jump_width     = CAN_BT_SJW_1TQ;  //再同步补偿宽度
  can_init_parameter.time_segment_1        = CAN_BT_BS1_5TQ;  //位段1
  can_init_parameter.time_segment_2        = CAN_BT_BS2_4TQ;  //位段2

  //1MBps
#if CAN_BAUDRATE == 1000
  can_init_parameter.prescaler = 6;
  //500KBps
#elif CAN_BAUDRATE == 500
  can_init_parameter.prescaler = 12;
  //250KBps
#elif CAN_BAUDRATE == 250
  can_init_parameter.prescaler = 24;
  //125KBps
#elif CAN_BAUDRATE == 125
  can_init_parameter.prescaler = 48;
  //100KBps
#elif  CAN_BAUDRATE == 100
  can_init_parameter.prescaler = 60;
  //50KBps
#elif  CAN_BAUDRATE == 50
  can_init_parameter.prescaler = 120;
  //20KBps
#elif  CAN_BAUDRATE == 20
  can_init_parameter.prescaler = 300;
#else
  #error "please select list can baudrate in private defines in main.c "
#endif

  //初始化CAN0
  can_init(CAN0, &can_init_parameter);

  //初始化过滤器参数
  can_filter_parameter.filter_number      = 0;                                        //过滤器单元编号
  can_filter_parameter.filter_mode        = CAN_FILTERMODE_MASK;                      //掩码模式
  can_filter_parameter.filter_bits        = CAN_FILTERBITS_32BIT;                     //32位过滤器
  can_filter_parameter.filter_list_high   = (((uint32_t)0x5A5<<21)&0xffff0000)>>16;   //标识符高位
  can_filter_parameter.filter_list_low    = (((uint32_t)0x5A5<<21)&0xffff);           //标识符低位
  can_filter_parameter.filter_mask_high   = 0xFFFF;                                   //掩码高位
  can_filter_parameter.filter_mask_low    = 0xFFFF;                                   //掩码低位
  can_filter_parameter.filter_fifo_number = CAN_FIFO0;                                //关联FIFO
  can_filter_parameter.filter_enable      = ENABLE;                                   //使能过滤器
  
  //初始化过滤器
  can_filter_init(&can_filter_parameter);

  //配置CAN0中断
  can_interrupt_enable(CAN0, CAN_INT_RFNE0);    //使能FIFO0非空中断CAN0_RX1_IRQn
  nvic_irq_enable(CAN0_RX0_IRQn, 0, 0);         //使能CAN中断
}

/*********************************************************************************************************
* 函数名称：CAN0_RX0_IRQHandler
* 函数功能：CAN0中断服务函数
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void CAN0_RX0_IRQHandler(void)
{
  //接收管理结构体
  static can_receive_message_struct s_structReceiveMessage;

  //获取CAN邮箱中的数据
  can_message_receive(CAN0, CAN_FIFO0, &s_structReceiveMessage);

  //校验是否为标准格式
  if(CAN_FF_STANDARD == s_structReceiveMessage.rx_ff)
  {
    //将数据保存到数据缓冲区
    EnQueue(&s_structCANRecCirQue, s_structReceiveMessage.rx_data, s_structReceiveMessage.rx_dlen);
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitCAN
* 函数功能：初始化CAN模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitCAN(void)
{
  u32 i;

  //初始化队列
  InitQueue(&s_structCANRecCirQue, s_arrRecBuf, sizeof(s_arrRecBuf) / sizeof(unsigned char));
  for(i = 0; i < sizeof(s_arrRecBuf) / sizeof(unsigned char); i++)
  {
    s_arrRecBuf[i] = 0;
  }

  //清空队列
  ClearQueue(&s_structCANRecCirQue);

  ConfigCAN0();  //配置CAN的GPIO
}

/*********************************************************************************************************
* 函数名称：WriteCAN0
* 函数功能：CAN0发送数据
* 输入参数：id：发送ID，buf：发送数据缓冲区，len：发送数据量
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：3个邮箱均为空时才可以通过CAN发送数据，否则有可能会因为邮箱优先级不同而造成发送数据顺序不对
*********************************************************************************************************/
void WriteCAN0(u32 id, u8* buf, u32 len)
{
  //发送管理结构体
  static can_trasnmit_message_struct s_structTransmitMessage;
  u32 sendCnt, sendLen, dataRemain, i;

  //初始化发送管理结构体
  s_structTransmitMessage.tx_sfid = id;
  s_structTransmitMessage.tx_efid = 0x00;
  s_structTransmitMessage.tx_ft   = CAN_FT_DATA;
  s_structTransmitMessage.tx_ff   = CAN_FF_STANDARD;

  s_structTransmitMessage.tx_dlen = 1;
  s_structTransmitMessage.tx_data[0] = buf[0];

  //发送数据
  sendCnt = 0;
  dataRemain = len;
  while(dataRemain)
  {
    //一次最多发送8个字节
    if(dataRemain >= 8)
    {
      sendLen = 8;
      dataRemain = dataRemain - 8;
    }
    else
    {
      sendLen = dataRemain;
      dataRemain = 0;
    }

    //从缓冲区中获取数据并保存到s_structTransmitMessage中
    for(i = 0; i < sendLen; i++)
    {
      s_structTransmitMessage.tx_data[i] = buf[sendCnt];
      sendCnt++;
    }

    //设置发送数据量
    s_structTransmitMessage.tx_dlen = sendLen;

    //等待3个邮箱均为空
    while(1)
    {
      if((CAN_TRANSMIT_PENDING != can_transmit_states(CAN0, 0)) &&
         (CAN_TRANSMIT_PENDING != can_transmit_states(CAN0, 1)) &&
         (CAN_TRANSMIT_PENDING != can_transmit_states(CAN0, 2)))
      {
        break;
      }
    }

    //发送
    can_message_transmit(CAN0, &s_structTransmitMessage);
  }
}

/*********************************************************************************************************
* 函数名称：ReadCAN0
* 函数功能：接收CAN0数据
* 输入参数：buf：发送数据缓冲区，len：发送数据量
* 输出参数：void
* 返 回 值：读到的数据量
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
u32 ReadCAN0(u8* buf, u32 len)
{
  u32 rLen, size;

  //获取队列中数据量
  size = QueueLength(&s_structCANRecCirQue);

  if(0 != size)
  {
    //从队列中读出数据
    rLen = DeQueue(&s_structCANRecCirQue, buf, len);
    return rLen;
  }
  else
  {
    //读取失败
    return 0;
  }
}

/*********************************************************************************************************
* 函数名称：DeInitCAN
* 函数功能：注销CAN模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void DeInitCAN(void)
{
  can_deinit(CAN0);
}
