/*********************************************************************************************************
* 模块名称：ethernetif.c
* 摘    要：网卡硬件层驱动模块
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
#include "netif/ethernetif.h"
#include "lwip/mem.h"
#include "netif/etharp.h"
#include "gd32f4xx_enet.h"
#include "LWIPComm.h"
#include "malloc.h"
#include "string.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
//网卡的名字
#define IFNAME0 'G'
#define IFNAME1 'D'

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
/* ENET RxDMA/TxDMA descriptor */
extern enet_descriptors_struct  rxdesc_tab[ENET_RXBUF_NUM], txdesc_tab[ENET_TXBUF_NUM];

/* ENET receive buffer  */
extern uint8_t rx_buff[ENET_RXBUF_NUM][ENET_RXBUF_SIZE]; 

/* ENET transmit buffer */
extern uint8_t tx_buff[ENET_TXBUF_NUM][ENET_TXBUF_SIZE]; 

/*global transmit and receive descriptors pointers */
extern enet_descriptors_struct  *dma_current_txdesc;
extern enet_descriptors_struct  *dma_current_rxdesc;

/* preserve another ENET RxDMA/TxDMA ptp descriptor for normal mode */
enet_descriptors_struct  ptp_txstructure[ENET_TXBUF_NUM];
enet_descriptors_struct  ptp_rxstructure[ENET_RXBUF_NUM];

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static err_t low_level_init(struct netif *netif);                   //初始化硬件
static err_t low_level_output(struct netif *netif, struct pbuf *p); //用于发送数据包的最底层函数
static struct pbuf * low_level_input(struct netif *netif);          //用于接收数据包的最底层函数

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：low_level_init
* 函数功能：由ethernetif_init()调用用于初始化硬件
* 输入参数：netif:网卡结构体指针
* 输出参数：void
* 返 回 值：ERR_OK-正常，其他-失败
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static err_t low_level_init(struct netif *netif)
{
  int i;
  StructLWIPDev* lwipDev = NULL;
  
  lwipDev = GetLWIPCommDev();

  //设置MAC地址长度,为6个字节
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  //初始化MAC地址,设置什么地址由用户自己设置,但是不能与网络中其他设备MAC地址重复
  netif->hwaddr[0] = lwipDev->mac[0];
  netif->hwaddr[1] = lwipDev->mac[1];
  netif->hwaddr[2] = lwipDev->mac[2];
  netif->hwaddr[3] = lwipDev->mac[3];
  netif->hwaddr[4] = lwipDev->mac[4];
  netif->hwaddr[5] = lwipDev->mac[5];

  /* initialize MAC address in ethernet MAC */
  enet_mac_address_set(ENET_MAC_ADDRESS0, netif->hwaddr);

  //最大允许传输单元,允许该网卡广播和ARP功能
  netif->mtu = 1500;
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

  /* initialize descriptors list: chain/ring mode */
#ifdef SELECT_DESCRIPTORS_ENHANCED_MODE
    enet_ptp_enhanced_descriptors_chain_init(ENET_DMA_TX);
    enet_ptp_enhanced_descriptors_chain_init(ENET_DMA_RX);
#else

    enet_descriptors_chain_init(ENET_DMA_TX);
    enet_descriptors_chain_init(ENET_DMA_RX);
    
//    enet_descriptors_ring_init(ENET_DMA_TX);
//    enet_descriptors_ring_init(ENET_DMA_RX);

#endif /* SELECT_DESCRIPTORS_ENHANCED_MODE */

  /* enable ethernet Rx interrrupt */
  for(i = 0; i < ENET_RXBUF_NUM; i++)
  { 
    enet_rx_desc_immediate_receive_complete_interrupt(&rxdesc_tab[i]);
  }

  /* enable the TCP, UDP and ICMP checksum insertion for the Tx frames */
  for(i=0; i < ENET_TXBUF_NUM; i++)
  {
    enet_transmit_checksum_config(&txdesc_tab[i], ENET_CHECKSUM_TCPUDPICMP_FULL);
  }

  /* note: TCP, UDP, ICMP checksum checking for received frame are enabled in DMA config */

  /* enable MAC and DMA transmission and reception */
  enet_enable();

  return ERR_OK;
}

/*********************************************************************************************************
* 函数名称：low_level_output
* 函数功能：用于发送数据包的最底层函数(lwip通过netif->linkoutput指向该函数)
* 输入参数：netif:网卡结构体指针，p:pbuf数据结构体指针
* 输出参数：void
* 返 回 值：ERR_OK-正常，其他-失败
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    struct pbuf *q;
    int framelength = 0;
    uint8_t *buffer;

    while((uint32_t)RESET != (dma_current_txdesc->status & ENET_TDES0_DAV)){
    }
    
    buffer = (uint8_t *)(enet_desc_information_get(dma_current_txdesc, TXDESC_BUFFER_1_ADDR));
    
    /* copy frame from pbufs to driver buffers */
    for(q = p; q != NULL; q = q->next){ 
        memcpy((uint8_t *)&buffer[framelength], q->payload, q->len);
        framelength = framelength + q->len;
    }
    
    /* note: padding and CRC for transmitted frame 
       are automatically inserted by DMA */

    /* transmit descriptors to give to DMA */ 
#ifdef SELECT_DESCRIPTORS_ENHANCED_MODE
    ENET_NOCOPY_PTPFRAME_TRANSMIT_ENHANCED_MODE(framelength, NULL);
  
#else
    
    ENET_NOCOPY_FRAME_TRANSMIT(framelength);
#endif /* SELECT_DESCRIPTORS_ENHANCED_MODE */

    return ERR_OK;
}

/*********************************************************************************************************
* 函数名称：low_level_input
* 函数功能：用于接收数据包的最底层函数
* 输入参数：netif:网卡结构体指针
* 输出参数：void
* 返 回 值：pbuf数据结构体指针
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static struct pbuf * low_level_input(struct netif *netif)
{
    struct pbuf *p, *q;
    u16_t len;
    int l =0;
    uint8_t *buffer;
     
    p = NULL;
    
    /* obtain the size of the packet and put it into the "len" variable. */
    len = enet_desc_information_get(dma_current_rxdesc, RXDESC_FRAME_LENGTH);
    buffer = (uint8_t *)(enet_desc_information_get(dma_current_rxdesc, RXDESC_BUFFER_1_ADDR));
    
    /* we allocate a pbuf chain of pbufs from the Lwip buffer pool */
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    
    /* copy received frame to pbuf chain */
    if (p != NULL){
        for (q = p; q != NULL; q = q->next){ 
            memcpy((uint8_t *)q->payload, (u8_t*)&buffer[l], q->len);
            l = l + q->len;
        }    
    }
  
#ifdef SELECT_DESCRIPTORS_ENHANCED_MODE
    ENET_NOCOPY_PTPFRAME_RECEIVE_ENHANCED_MODE(NULL);
  
#else
    
    ENET_NOCOPY_FRAME_RECEIVE();
#endif /* SELECT_DESCRIPTORS_ENHANCED_MODE */

    return p;
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ethernetif_input
* 函数功能：网卡接收数据(lwip直接调用)
* 输入参数：netif:网卡结构体指针
* 输出参数：void
* 返 回 值：ERR_OK-发送正常，ERR_MEM-发送失败
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
err_t ethernetif_input(struct netif *netif)
{
  err_t err;
  struct pbuf *p;

  //调用low_level_input函数接收数据
  p = low_level_input(netif);
  if(p == NULL) 
  {
    return ERR_MEM;
  }

  //调用netif结构体中的input字段(一个函数)来处理数据包
  err = netif->input(p, netif); 
  if(err!=ERR_OK)
  {
    LWIP_DEBUGF(NETIF_DEBUG,("ethernetif_input: IP input error\n"));
    pbuf_free(p);
    p = NULL;
  }
  return err;
}

/*********************************************************************************************************
* 函数名称：ethernetif_init
* 函数功能：使用low_level_init()函数来初始化网络
* 输入参数：netif:网卡结构体指针
* 输出参数：void
* 返 回 值：ERR_OK-发送正常，其他-发送失败
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
err_t ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif!=NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME   //LWIP_NETIF_HOSTNAME
  netif->hostname="lwip"; //初始化名称
#endif

  netif->name[0]    = IFNAME0;          //初始化变量netif的name字段
  netif->name[1]    = IFNAME1;          //在文件外定义这里不用关心具体值
  netif->output     = etharp_output;    //IP层发送数据包函数
  netif->linkoutput = low_level_output; //ARP模块发送数据包函数
  low_level_init(netif);                //底层硬件初始化函数
  return ERR_OK;
}

