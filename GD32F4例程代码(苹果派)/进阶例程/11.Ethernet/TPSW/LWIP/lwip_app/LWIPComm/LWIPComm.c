/*********************************************************************************************************
* 模块名称：LWIPComm.c
* 摘    要：LWIP公用模块
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
#include "LWIPComm.h"
#include "lwipopts.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "ethernetif.h"
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h"
#include "Malloc.h"
#include "SysTick.h"
#include "LAN8720.h"
#include "Timer.h"
#include "stdio.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructLWIPDev s_structLWIPDev;          //lwip控制结构体
static struct netif  s_structLWIPNetif;        //定义一个全局的网络接口
static u64           s_iTCPTimer          = 0; //TCP查询计时器
static u64           s_iARPTimer          = 0; //ARP查询计时器
static u64           s_iLWIPCommLocaltime = 0; //lwip本地时间计数器,单位:ms

#if LWIP_DHCP
  static u64 s_iDHCPFineTimer   = 0; //DHCP精细处理计时器
  static u64 s_iDHCPCoarseTimer = 0; //DHCP粗糙处理计时器
#endif

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static u8   LWIPCommMemMalloc(void);                    //lwip中mem和memp内存申请
static void LWIPCommMemFree(void);                      //lwip中mem和memp内存释放
static void LWIPCommDefaultIpSet(StructLWIPDev *lwipx); //lwip 默认IP设置
static void LWIPDhcpProcessHandle(void);                //DHCP处理任务

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：LWIPCommMemMalloc
* 函数功能：lwip中mem和memp内存申请
* 输入参数：void
* 输出参数：void
* 返 回 值：0-成功，其他-失败
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static u8 LWIPCommMemMalloc(void)
{
  extern u32  memp_get_memorysize(void); //在memp.c里面定义
  extern u8_t *memp_memory;              //在memp.c里面定义
  extern u8_t *ram_heap;                 //在mem.c里面定义

  u32 mempsize;
  u32 ramheapsize;

  //得到memp_memory数组大小
  mempsize = memp_get_memorysize();

  //为memp_memory申请内存
  memp_memory = MyMalloc(SRAMIN, mempsize);

  //得到ram heap大小
  ramheapsize = LWIP_MEM_ALIGN_SIZE(MEM_SIZE) + 2 * LWIP_MEM_ALIGN_SIZE(4 * 3) + MEM_ALIGNMENT;

  //为ram_heap申请内存
  ram_heap = MyMalloc(SRAMIN,ramheapsize);

  //有申请失败的
  if(!memp_memory || (!ram_heap))
  {
    LWIPCommMemFree();
    return 1;
  }
  return 0;
}

/*********************************************************************************************************
* 函数名称：LWIPCommMemFree
* 函数功能：lwip中mem和memp内存释放
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void LWIPCommMemFree(void)
{
  extern u8_t *memp_memory;             //在memp.c里面定义
  extern u8_t *ram_heap;                //在mem.c里面定义
  
  MyFree(SRAMIN, memp_memory);
  MyFree(SRAMIN, ram_heap);
}

/*********************************************************************************************************
* 函数名称：LWIPCommDefaultIpSet
* 函数功能：lwip 默认IP设置
* 输入参数：lwipx:lwip控制结构体指针
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void LWIPCommDefaultIpSet(StructLWIPDev *lwipx)
{
  u8* id;

  //MAC地址设置SGD32唯一ID
  id = (u8*)0x1FFF7A10;
  lwipx->mac[0] = id[0];
  lwipx->mac[1] = id[1];
  lwipx->mac[2] = id[2];
  lwipx->mac[3] = id[3];
  lwipx->mac[4] = id[4];
  lwipx->mac[5] = id[5];

  //默认本地IP为:192.168.1.31
  lwipx->ip[0] = 192;
  lwipx->ip[1] = 168;
  lwipx->ip[2] = 1;
  lwipx->ip[3] = 31;

  //默认子网掩码:255.255.255.0
  lwipx->netmask[0] = 255;
  lwipx->netmask[1] = 255;
  lwipx->netmask[2] = 255;
  lwipx->netmask[3] = 0;

  //默认网关:192.168.1.1
  lwipx->gateway[0] = 192;
  lwipx->gateway[1] = 168;
  lwipx->gateway[2] = 1;
  lwipx->gateway[3] = 1;

  //没有DHCP
  lwipx->dhcpstatus=0;
}

/*********************************************************************************************************
* 函数名称：LWIPDhcpProcessHandle
* 函数功能：DHCP处理任务
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
#if LWIP_DHCP //如果使能了DHCP
static void LWIPDhcpProcessHandle(void)
{
  u32 ip = 0, netmask = 0, gw = 0;
  u8 *pIP, *pMask, *pMAC, *pGW;

  switch(s_structLWIPDev.dhcpstatus)
  {
    //开启DHCP
    case 0:
      dhcp_start(&s_structLWIPNetif);
      s_structLWIPDev.dhcpstatus = 1;  //等待通过DHCP获取到的地址
      printf("正在查找DHCP服务器,请稍等...........\r\n");
      break;

    //等待获取到IP地址
    case 1:
    {
      ip      = s_structLWIPNetif.ip_addr.addr; //读取新IP地址
      netmask = s_structLWIPNetif.netmask.addr; //读取子网掩码
      gw      = s_structLWIPNetif.gw.addr;      //读取默认网关
      
      //正确获取到IP地址的时候
      if(ip != 0)
      {
        s_structLWIPDev.dhcpstatus = 2; //DHCP成功
        pMAC = s_structLWIPDev.mac;
        printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n", pMAC[0], pMAC[1], pMAC[2], pMAC[3], pMAC[4], pMAC[5]);
        
        //解析出通过DHCP获取到的IP地址
        s_structLWIPDev.ip[3] = (uint8_t)(ip >> 24); 
        s_structLWIPDev.ip[2] = (uint8_t)(ip >> 16);
        s_structLWIPDev.ip[1] = (uint8_t)(ip >> 8);
        s_structLWIPDev.ip[0] = (uint8_t)(ip);
        pIP = s_structLWIPDev.ip;
        printf("通过DHCP获取到IP地址..............%d.%d.%d.%d\r\n", pIP[0], pIP[1], pIP[2], pIP[3]);
        
        //解析通过DHCP获取到的子网掩码地址
        s_structLWIPDev.netmask[3] = (uint8_t)(netmask >> 24);
        s_structLWIPDev.netmask[2] = (uint8_t)(netmask >> 16);
        s_structLWIPDev.netmask[1] = (uint8_t)(netmask >> 8);
        s_structLWIPDev.netmask[0] = (uint8_t)(netmask);
        pMask = s_structLWIPDev.netmask;
        printf("通过DHCP获取到子网掩码............%d.%d.%d.%d\r\n", pMask[0], pMask[1], pMask[2], pMask[3]);
        
        //解析出通过DHCP获取到的默认网关
        s_structLWIPDev.gateway[3] = (uint8_t)(gw >> 24);
        s_structLWIPDev.gateway[2] = (uint8_t)(gw >> 16);
        s_structLWIPDev.gateway[1] = (uint8_t)(gw >> 8);
        s_structLWIPDev.gateway[0] = (uint8_t)(gw);
        pGW = s_structLWIPDev.gateway;
        printf("通过DHCP获取到的默认网关..........%d.%d.%d.%d\r\n", pGW[0], pGW[1], pGW[2], pGW[3]);
      }
      
      //通过DHCP服务获取IP地址失败,且超过最大尝试次数
      else if(s_structLWIPNetif.dhcp->tries > LWIP_MAX_DHCP_TRIES)
      {
        s_structLWIPDev.dhcpstatus = 0XFF; //DHCP超时失败
        
        //使用静态IP地址
        pMAC  = s_structLWIPDev.mac;
        pIP   = s_structLWIPDev.ip;
        pMask = s_structLWIPDev.netmask;
        pGW   = s_structLWIPDev.gateway;
        IP4_ADDR(&(s_structLWIPNetif.ip_addr), pIP[0], pIP[1], pIP[2], pIP[3]);
        IP4_ADDR(&(s_structLWIPNetif.netmask), pMask[0], pMask[1], pMask[2], pMask[3]);
        IP4_ADDR(&(s_structLWIPNetif.gw)     , pGW[0], pGW[1], pGW[2], pGW[3]);
        printf("DHCP服务超时,使用静态IP地址!\r\n");
        printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n", pMAC[0], pMAC[1], pMAC[2], pMAC[3], pMAC[4], pMAC[5]);
        printf("静态IP地址........................%d.%d.%d.%d\r\n", pIP[0], pIP[1], pIP[2], pIP[3]);
        printf("子网掩码..........................%d.%d.%d.%d\r\n", pMask[0], pMask[1], pMask[2], pMask[3]);
        printf("默认网关..........................%d.%d.%d.%d\r\n", pGW[0], pGW[1], pGW[2], pGW[3]);
      }
    }
    break;
    default : break;
  }
}
#endif

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：LWIPCommInit
* 函数功能：LWIP初始化(LWIP启动的时候使用)
* 输入参数：void
* 输出参数：void
* 返 回 值：0,成功
*          1,内存错误
*          2,DM9000初始化失败
*          3,网卡添加失败
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
u8 LWIPCommInit(void)
{
  struct netif*  Netif_Init_Flag; //调用netif_add()函数时的返回值,用于判断网络初始化是否成功
  struct ip_addr ipaddr;          //ip地址
  struct ip_addr netmask;         //子网掩码
  struct ip_addr gw;              //默认网关

#if 0 == LWIP_DHCP
  u8 *pIP, *pMask, *pMAC, *pGW;
#endif

  //内存申请
  if(LWIPCommMemMalloc())
  {
    return 1;
  }

  //初始化LAN8720
  InitLAN8720();

  //初始化LWIP内核
  lwip_init();

  //设置默认IP等信息
  LWIPCommDefaultIpSet(&s_structLWIPDev); 

  //使用动态IP
#if LWIP_DHCP
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;

//使用静态IP
#else
  pMAC  = s_structLWIPDev.mac;
  pIP   = s_structLWIPDev.ip;
  pMask = s_structLWIPDev.netmask;
  pGW   = s_structLWIPDev.gateway;
  IP4_ADDR(&ipaddr, pIP[0], pIP[1], pIP[2], pIP[3]);
  IP4_ADDR(&netmask, pMask[0], pMask[1], pMask[2], pMask[3]);
  IP4_ADDR(&gw, pGW[0], pGW[1], pGW[2], pGW[3]);
  printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",pMAC[0], pMAC[1], pMAC[2], pMAC[3], pMAC[4], pMAC[5]);
  printf("静态IP地址........................%d.%d.%d.%d\r\n",pIP[0], pIP[1], pIP[2], pIP[3]);
  printf("子网掩码..........................%d.%d.%d.%d\r\n",pMask[0], pMask[1], pMask[2], pMask[3]);
  printf("默认网关..........................%d.%d.%d.%d\r\n",pGW[0], pGW[1], pGW[2], pGW[3]);
#endif

  //向网卡列表中添加一个网口
  Netif_Init_Flag = netif_add(&s_structLWIPNetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  //如果使用DHCP的话
#if LWIP_DHCP
  s_structLWIPDev.dhcpstatus = 0; //DHCP标记为0
  dhcp_start(&s_structLWIPNetif); //开启DHCP服务
#endif

  //网卡添加失败
  if(Netif_Init_Flag == NULL)
  {
    return 3;
  }

  //网口添加成功后,设置netif为默认值,并且打开netif网口
  else
  {
    netif_set_default(&s_structLWIPNetif); //设置netif为默认网口
    netif_set_up(&s_structLWIPNetif);      //打开netif网口
  }
  return 0;//操作OK
}

/*********************************************************************************************************
* 函数名称：lwip_pkt_handle
* 函数功能：LWIP接收数据包轮询任务
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void LWIPReadPackHandle(void)
{
  //从网络缓冲区中读取接收到的数据包并将其发送给LWIP处理
  if(enet_rxframe_size_get())
  {
    ethernetif_input(&s_structLWIPNetif);
  }
}

/*********************************************************************************************************
* 函数名称：lwip_periodic_handle
* 函数功能：LWIP轮询任务
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void LWIPPeriodicHandle()
{
  //获取系统时间
  s_iLWIPCommLocaltime = GetSysTime();

#if LWIP_TCP
  //每250ms调用一次tcp_tmr()函数
  if(s_iLWIPCommLocaltime - s_iTCPTimer >= TCP_TMR_INTERVAL)
  {
    s_iTCPTimer =  s_iLWIPCommLocaltime;
    tcp_tmr();
  }
#endif

  //ARP每5s周期性调用一次
  if ((s_iLWIPCommLocaltime - s_iARPTimer) >= ARP_TMR_INTERVAL)
  {
    s_iARPTimer = s_iLWIPCommLocaltime;
    etharp_tmr();
  }

#if LWIP_DHCP //如果使用DHCP的话
  //每500ms调用一次dhcp_fine_tmr()
  if (s_iLWIPCommLocaltime - s_iDHCPFineTimer >= DHCP_FINE_TIMER_MSECS)
  {
    s_iDHCPFineTimer = s_iLWIPCommLocaltime;
    dhcp_fine_tmr();
    if((s_structLWIPDev.dhcpstatus != 2) && (s_structLWIPDev.dhcpstatus != 0XFF))
    { 
      LWIPDhcpProcessHandle();  //DHCP处理
    }
  }

  //每60s执行一次DHCP粗糙处理
  if (s_iLWIPCommLocaltime - s_iDHCPCoarseTimer >= DHCP_COARSE_TIMER_MSECS)
  {
    s_iDHCPCoarseTimer = s_iLWIPCommLocaltime;
    dhcp_coarse_tmr();
  }
#endif
}

/*********************************************************************************************************
* 函数名称：GetLWIPCommDev
* 函数功能：获取LWIP设备结构体
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
StructLWIPDev* GetLWIPCommDev(void)
{
  return &s_structLWIPDev;
}


