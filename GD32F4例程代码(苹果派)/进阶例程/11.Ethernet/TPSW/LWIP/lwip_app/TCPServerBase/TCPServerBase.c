/*********************************************************************************************************
* 模块名称：TCPServerBase.c
* 摘    要：TCP服务器驱动模块
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
#include "TCPServerBase.h"
#include "SysTick.h"
#include "Malloc.h"
#include "stdio.h"
#include "string.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/
//客户端连接状态
typedef enum
{
  ES_TCPSERVER_NONE = 0,  //没有连接
  ES_TCPSERVER_ACCEPTED,  //客户端连接上了
  ES_TCPSERVER_CLOSING,   //即将关闭连接
}EnumTCPServerStates;

//客户端设备结构体（LWIP回调函数使用的结构体）
typedef struct
{
  EnumTCPServerStates state;                        //当前连接状
  struct tcp_pcb*     pcb;                          //指向当前的pcb
  struct pbuf*        p;                            //指向接收/或传输的pbuf
  StructCirQue        recvCirQue;                   //接收循环队列
  unsigned char       recvBuf[TCP_SERVER_BUF_SIZE]; //接收循环队列的缓冲区
  u32                 sendNum;                      //发送缓冲区中的数据量
  unsigned char       sendBuf[TCP_SERVER_BUF_SIZE]; //发送缓冲区
}StructTCPClientDev;

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static struct tcp_pcb*    s_structTCPServerPcb;  //定义一个TCP服务器控制块
static struct tcp_pcb*    s_structTCPServerConn; //定义一个TCP服务器控制块
static StructTCPClientDev s_arrClientDev[TCP_SERVER_CLIENT_NUM]; //客户端设备

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static err_t TCPServerAcceptCallback(void *arg, struct tcp_pcb *newpcb, err_t err);             //客户端接入回调函数
static err_t TCPServerRecvCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err); //接收数据回调函数
static void  TCPServerErrorCallback(void *arg,err_t err);                                       //出错回调函数
static err_t TCPServerPollCallback(void *arg, struct tcp_pcb *tpcb);                            //轮询回调函数
static err_t TCPServerSendCallback(void *arg, struct tcp_pcb *tpcb, u16_t len);                 //发送回调函数
static void  TCPServerSendData(struct tcp_pcb *tpcb, StructTCPClientDev *es);                   //发送数据
static void  TCPServerConnectionClose(struct tcp_pcb *tpcb, StructTCPClientDev *es);            //关闭tcp连接
static void  ClearClientDev(StructTCPClientDev *client);                                        //清空某一客户端设备数据

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：TCPServerAcceptCallback
* 函数功能：客户端接入回调函数，有新的客户端接入时将会调用此函数
* 输入参数：arg：回调参数
*          newpcb：新接入的PCB
*          err：错误码
* 输出参数：void
* 返 回 值：错误码
* 创建日期：2021年07月01日
* 注    意：tcp_abort函数通过向远程主机发送一个RST(复位)段来中止连接，pcb结构将会被释放，
*          该函数是不会失败的，它一定能完成中止的目的。
*********************************************************************************************************/
static err_t TCPServerAcceptCallback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  err_t ret_err;
  StructTCPClientDev *es; 
  u32 i;
  u8  ip[4];

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);

  //查找客户端结构体数组空位置
  es = NULL;
  for(i = 0; i < TCP_SERVER_CLIENT_NUM; i++)
  {
    if(ES_TCPSERVER_NONE == s_arrClientDev[i].state)
    {
      es = &s_arrClientDev[i];
      break;
    }
  }

  //成功查找到空位置
  if(es != NULL)
  {
    ClearClientDev(es);                 //清空当前客户端设备结构体数据
    es->state = ES_TCPSERVER_ACCEPTED;  //接收连接
    es->pcb = newpcb;                   //标记PCB

    tcp_setprio(newpcb, TCP_PRIO_MIN);          //设置新创建的pcb优先级
    tcp_arg(newpcb, es);                        //设置回调参数
    tcp_recv(newpcb, TCPServerRecvCallback);    //设置TCP服务器接收到数据回调函数
    tcp_err( newpcb, TCPServerErrorCallback);   //设置TCP服务器出现错误时回调函数
    tcp_poll(newpcb, TCPServerPollCallback, 1); //设置TCP服务器轮询回调函数
    tcp_sent(newpcb, TCPServerSendCallback);    //设置TCP服务器发送回调函数

    //打印客户端IP
    ip[0] = (newpcb->remote_ip.addr >> 0 ) & 0xff; //IADDR4
    ip[1] = (newpcb->remote_ip.addr >> 8 ) & 0xff; //IADDR3
    ip[2] = (newpcb->remote_ip.addr >> 16) & 0xff; //IADDR2
    ip[3] = (newpcb->remote_ip.addr >> 24) & 0xff; //IADDR1
    printf("Client %d.%d.%d.%d connected\r\n", ip[0], ip[1], ip[2], ip[3]);

    ret_err = ERR_OK;
  }

  //接入的客户端数量已满
  else
  {
    //强行终止连接
    tcp_abort(newpcb);
    ret_err = ERR_MEM;
  }
  return ret_err;
}

/*********************************************************************************************************
* 函数名称：TCPServerRecvCallback
* 函数功能：接收数据回调函数，TCP服务器接收到客户端数据后将会调用此函数
* 输入参数：arg：回调参数，用于传递TCP服务器设备结构体（StructTCPClientDev）地址
*          tpcb：客户端PCB
*          p：数据包首地址
*          err：错误码
* 输出参数：void
* 返 回 值：错误码
* 创建日期：2021年07月01日
* 注    意：
*          1、处理完数据后要手动释放数据包内存
*          2、客户端断开后将会执行此回调函数，并且传进来的参数p为NULL
*********************************************************************************************************/
static err_t TCPServerRecvCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  err_t ret_err;
  struct pbuf* q;
  StructTCPClientDev* es;
  u32 i;

  //校验arg参数是否为空
  LWIP_ASSERT("arg != NULL", arg != NULL);

  //获取传递TCP服务器设备结构体地址
  es = (StructTCPClientDev*)arg;

  //p为NULL表示客户端已断开
  if(p == NULL)
  {
    //标记需要关闭TCP连接
    es->state = ES_TCPSERVER_CLOSING;
    es->p = p; 
    ret_err = ERR_OK;
  }

  //从客户端接收到一个非空数据,但是由于某种原因err!=ERR_OK
  else if(err != ERR_OK)
  {
    if(p)
    {
      pbuf_free(p); //释放接收pbuf
    }
    ret_err = err;
  }
  
  //当处于连接状态并且接收到的数据不为空
  else if(es->state == ES_TCPSERVER_ACCEPTED)
  {
    //遍历完整个pbuf链表
    for(q = p; q != NULL; q = q->next)
    {
      //将数据保存到队列中
      EnQueue(&es->recvCirQue, (DATA_TYPE*)q->payload, q->len);
    }

    //更新tcp窗口大小
    tcp_recved(tpcb, p->tot_len);

    //释放内存
    pbuf_free(p);

    //数据接收正常
    ret_err = ERR_OK;
  }

  //服务器关闭了
  else
  {
    //更新tcp窗口大小
    tcp_recved(tpcb, p->tot_len);

    //清空所有客户端设备结构体数据
    for(i = 0; i < TCP_SERVER_CLIENT_NUM; i++)
    {
      ClearClientDev(&s_arrClientDev[i]);
    }

    //释放内存
    pbuf_free(p);

    ret_err = ERR_OK;
  }
  return ret_err;
}

/*********************************************************************************************************
* 函数名称：TCPServerErrorCallback
* 函数功能：TCP服务器出错回调函数
* 输入参数：arg：回调参数，用于传递TCP服务器设备结构体（StructTCPClientDev）地址
*          err：错误码
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void TCPServerErrorCallback(void *arg, err_t err)
{
  LWIP_UNUSED_ARG(err);
  printf("tcp error:%x\r\n", (u32)arg);
}

/*********************************************************************************************************
* 函数名称：TCPServerPollCallback
* 函数功能：TCP服务器轮询回调函数
* 输入参数：arg：回调参数，用于传递TCP服务器设备结构体（StructTCPClientDev）地址
*          tpcb：客户端PCB
* 输出参数：void
* 返 回 值：错误码
* 创建日期：2021年07月01日
* 注    意：tcp_abort函数通过向远程主机发送一个RST(复位)段来中止连接，pcb结构将会被释放，
*          该函数是不会失败的，它一定能完成中止的目的。
*********************************************************************************************************/
static err_t TCPServerPollCallback(void *arg, struct tcp_pcb *tpcb)
{
  err_t ret_err;
  StructTCPClientDev* es;

  //获取TCP服务器设备结构体首地址
  es = (StructTCPClientDev*)arg; 

  if(es != NULL)
  {
    //关闭连接
    if(es->state == ES_TCPSERVER_CLOSING)
    {
      TCPServerConnectionClose(tpcb, es);
    }

    //有数据要发送
    else if(0 != es->sendNum)
    {
      //申请内存
      es->p = pbuf_alloc(PBUF_TRANSPORT, es->sendNum, PBUF_POOL);

      //复制数据到数据包
      pbuf_take(es->p, (const void*)es->sendBuf, es->sendNum);

      //发送数据
      TCPServerSendData(tpcb, es);

      //清空发送数据量
      es->sendNum = 0;
      
      //释放内存
      if(es->p != NULL)
      {
        pbuf_free(es->p);
      }
      es->p = NULL;
    }

    ret_err = ERR_OK;
  }
  else
  {
    //终止连接,删除pcb控制块
    tcp_abort(tpcb);
    ret_err = ERR_ABRT;
  }
  return ret_err;
}

/*********************************************************************************************************
* 函数名称：TCPServerSendCallback
* 函数功能：发送回调函数(当从远端主机接收到ACK信号后发送数据)
* 输入参数：arg：回调参数，用于传递TCP服务器设备结构体（StructTCPClientDev）地址
*          tpcb：客户端PCB
*          len：数据量
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static err_t TCPServerSendCallback(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  StructTCPClientDev* es;
  LWIP_UNUSED_ARG(len); 

  //获取TCP服务器设备结构体首地址
  es = (StructTCPClientDev*) arg;
  if(es->p)
  {
    //发送数据
    TCPServerSendData(tpcb, es);
  }
  return ERR_OK;
}

/*********************************************************************************************************
* 函数名称：TCPServerSendData
* 函数功能：发送数据
* 输入参数：tpcb：客户端PCB
*          es：TCP服务器设备结构体（StructTCPClientDev）
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void TCPServerSendData(struct tcp_pcb *tpcb, StructTCPClientDev* es)
{
  struct pbuf *ptr;
  u16 plen;
  err_t wr_err = ERR_OK;

  //循环发送数据
  while((wr_err == ERR_OK) && es->p && (es->p->len <= tcp_sndbuf(tpcb)))
  {
    //获取数据包首地址
    ptr = es->p;

    //发送数据（数据将被拷贝到LWIP内存当中等待发送，而并非立即发送）
    wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);

    //成功发送一个数据包
    if(wr_err == ERR_OK)
    { 
      plen = ptr->len;
      es->p = ptr->next; //指向下一个pbuf

      //pbuf的引用次数加一
      if(es->p)
      {
        pbuf_ref(es->p);
      }

      //释放前一个pbuf内存
      pbuf_free(ptr);

      //更新tcp窗口大小
      tcp_recved(tpcb, plen);
    }
    else if(wr_err == ERR_MEM)
    {
      es->p = ptr;
    }
  }
}

/*********************************************************************************************************
* 函数名称：TCPServerConnectionClose
* 函数功能：关闭tcp连接
* 输入参数：tpcb：客户端PCB
*          es：TCP服务器设备结构体（StructTCPClientDev）
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：调用tcp_close将关闭一个指定的TCP连接，并自动释放(删除)pcb结构
*********************************************************************************************************/
static void TCPServerConnectionClose(struct tcp_pcb *tpcb, StructTCPClientDev* es)
{
  u8 ip[4];

  if(NULL != es)
  {
    //打印客户端IP
    ip[0] = (es->pcb->remote_ip.addr >> 0 ) & 0xff; //IADDR4
    ip[1] = (es->pcb->remote_ip.addr >> 8 ) & 0xff; //IADDR3
    ip[2] = (es->pcb->remote_ip.addr >> 16) & 0xff; //IADDR2
    ip[3] = (es->pcb->remote_ip.addr >> 24) & 0xff; //IADDR1
    printf("Client %d.%d.%d.%d closed\r\n", ip[0], ip[1], ip[2], ip[3]);
    ClearClientDev(es);
  }

  tcp_arg(tpcb, NULL);
  tcp_sent(tpcb, NULL);
  tcp_recv(tpcb, NULL);
  tcp_err(tpcb, NULL);
  tcp_poll(tpcb, NULL, 0);
  tcp_close(tpcb);
}

/*********************************************************************************************************
* 函数名称：ClearClientDev
* 函数功能：清空某一客户端设备数据
* 输入参数：client：客户端设备结构体地址
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ClearClientDev(StructTCPClientDev *client)
{
  u32 i;
  client->state = ES_TCPSERVER_NONE; //标记客户端没有连接
  client->pcb   = NULL;              //客户端PCB为空
  client->p     = NULL;              //接收/或传输的pbuf为空

  //清空接收队列
  for(i = 0; i < TCP_SERVER_BUF_SIZE; i++)
  {
    client->recvBuf[i] = 0;
  }
  InitQueue(&client->recvCirQue, client->recvBuf, TCP_SERVER_BUF_SIZE);

  //清空发送缓冲区
  client->sendNum = 0;
  for(i = 0; i < TCP_SERVER_BUF_SIZE; i++)
  {
    client->sendBuf[i] = 0;
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：CreateTCPServer
* 函数功能：创建TCP服务器
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void CreateTCPServer(void)
{
  static err_t    err;  
  u32             i;
  StructLWIPDev*  lwipDev = NULL;

  //获取LWIP设备结构体地址，用于打印TCP服务器IP地址
  lwipDev = GetLWIPCommDev();

  //初始化客户端设备
  for(i = 0; i < TCP_SERVER_CLIENT_NUM; i++)
  {
    ClearClientDev(&s_arrClientDev[i]);
  }
  
  //服务器IP
  printf("Server IP:%d.%d.%d.%d\r\n", lwipDev->ip[0], lwipDev->ip[1], lwipDev->ip[2], lwipDev->ip[3]);

  //服务器端口号
  printf("Server Port:%d\r\n",TCP_SERVER_PORT);
  
  s_structTCPServerPcb = tcp_new(); //创建一个新的pcb

  //创建成功
  if(s_structTCPServerPcb)
  {
    //将本地IP与指定的端口号绑定在一起,IP_ADDR_ANY为绑定本地所有的IP地址
    err = tcp_bind(s_structTCPServerPcb, IP_ADDR_ANY, TCP_SERVER_PORT);
    if(err == ERR_OK) //绑定完成
    {
      //设置tcppcb进入监听状态
      s_structTCPServerConn = tcp_listen(s_structTCPServerPcb);

      //设置客户端接入回调函数
      tcp_accept(s_structTCPServerConn, TCPServerAcceptCallback);
    }
    else
    {
      while(1);
    }
  }
  else
  {
    while(1);
  }
}

/*********************************************************************************************************
* 函数名称：CloseTCPServer
* 函数功能：关闭TCP服务器
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void CloseTCPServer(void)
{
  extern void tcp_pcb_purge(struct tcp_pcb *pcb); //在 tcp.c里面
  extern struct tcp_pcb *tcp_active_pcbs;         //在 tcp.c里面
  extern struct tcp_pcb *tcp_tw_pcbs;             //在 tcp.c里面
  struct tcp_pcb *pcb,*pcb2; 

  TCPServerConnectionClose(s_structTCPServerPcb, 0);  //关闭TCP Server连接
  TCPServerConnectionClose(s_structTCPServerConn, 0); //关闭TCP Server连接

  //强制删除TCP Server主动断开时的time wait
  while(tcp_active_pcbs != NULL)
  {
    LWIPPeriodicHandle(); //继续轮询
    LWIPReadPackHandle();
    DelayNms(10);         //等待tcp_active_pcbs为空
  }
  pcb = tcp_tw_pcbs;

  //如果有等待状态的pcbs
  while(pcb != NULL)
  {
    tcp_pcb_purge(pcb); 
    tcp_tw_pcbs = pcb->next;
    pcb2 = pcb;
    pcb = pcb->next;
    memp_free(MEMP_TCP_PCB, pcb2);
  }

  memset(s_structTCPServerPcb, 0, sizeof(struct tcp_pcb));
  memset(s_structTCPServerConn, 0, sizeof(struct tcp_pcb));
}

/*********************************************************************************************************
* 函数名称：ReadTCPServer
* 函数功能：读TCP服务器接收数据缓冲区
* 输入参数：client：客户端，可以是列表枚举，也可以是IP地址，buf-读出数据储存地址，len-读取长度
* 输出参数：void
* 返 回 值：成功读取到的数据量
* 创建日期：2021年07月01日
* 注    意：IP地址高低直接相反
*********************************************************************************************************/
u32 ReadTCPServer(u32 client, u8* buf, u32 len)
{
  u32 ret;
  u32 i;
  StructTCPClientDev *dev;

  dev = NULL;
  ret = 0;

  //IP小于TCP_SERVER_CLIEN_NUM，表示client以枚举列表的形式
  if(client < TCP_SERVER_CLIENT_NUM)
  {
    if(ES_TCPSERVER_ACCEPTED == s_arrClientDev[client].state)
    {
      dev = &s_arrClientDev[client];
    }
  }

  //使用真实IP地址形式
  else
  {
    for(i = 0; i < TCP_SERVER_CLIENT_NUM; i++)
    {
      if(ES_TCPSERVER_ACCEPTED == s_arrClientDev[i].state)
      {
        if(s_arrClientDev[i].pcb->remote_ip.addr == client)
        {
          dev = &s_arrClientDev[i];
          break;
        }
      }
    }
  }

  //从接收队列中将数据取出
  if(NULL != dev)
  {
    ret = DeQueue(&dev->recvCirQue, buf, len);
  }

  //返回成功读取到的数据量
  return ret;
}

/*********************************************************************************************************
* 函数名称：WriteTCPServer
* 函数功能：TCP服务器发送数据
* 输入参数：client：客户端，可以是列表枚举，也可以是IP地址，buf-发送数据储存地址，len-读取长度
* 输出参数：void
* 返 回 值：成功发送的数据量
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
u32 WriteTCPServer(u32 client, u8* buf, u32 len)
{
  u32 sendCnt;
  u32 i;
  StructTCPClientDev *dev;

  //向所有的客户端发送消息
  if(TCP_SERVER_CLIENT_ALL == client)
  {
    //遍历所有的客户端
    for(i = 0; i < TCP_SERVER_CLIENT_NUM; i++)
    {
      //当前客户端已连接
      if(ES_TCPSERVER_ACCEPTED == s_arrClientDev[i].state)
      {
        //将数据拷贝至发送缓冲区
        dev = &s_arrClientDev[i];
        sendCnt = 0;
        while(dev->sendNum < TCP_SERVER_BUF_SIZE && sendCnt < len)
        {
          dev->sendBuf[dev->sendNum] = buf[sendCnt];
          dev->sendNum++;
          sendCnt++;
        }
      }
    }

    //默认全部发送成功
    return len;
  }

  //向单个客户端发送数据
  else
  {
    dev = NULL;

    //IP小于TCP_SERVER_CLIEN_NUM，表示client以枚举列表的形式
    if(client < TCP_SERVER_CLIENT_NUM)
    {
      if(ES_TCPSERVER_ACCEPTED == s_arrClientDev[client].state)
      {
        dev = &s_arrClientDev[client];
      }
    }

    //使用真实IP地址形式
    else
    {
      for(i = 0; i < TCP_SERVER_CLIENT_NUM; i++)
      {
        if(ES_TCPSERVER_ACCEPTED == s_arrClientDev[i].state)
        {
          if(s_arrClientDev[i].pcb->remote_ip.addr == client)
          {
            dev = &s_arrClientDev[i];
            break;
          }
        }
      }
    }

    //将发送数据拷贝到发送缓冲区
    sendCnt = 0;
    if(NULL != dev)
    {
      while(dev->sendNum < TCP_SERVER_BUF_SIZE && sendCnt < len)
      {
        dev->sendBuf[dev->sendNum] = buf[sendCnt];
        dev->sendNum++;
        sendCnt++;
      }
    }

    return sendCnt;
  }
}

/*********************************************************************************************************
* 函数名称：GetClientIP
* 函数功能：通过客户端列表获取客户端IP
* 输入参数：client：客户端
* 输出参数：void
* 返 回 值：客户端IP，若客户端未连接则返回0
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
u32 GetClientIP(EnumTCPClientList client)
{
  //客户端处在客户端列表内
  if(client < TCP_SERVER_CLIENT_NUM)
  {
    //检查是否有客户端连上
    if(ES_TCPSERVER_ACCEPTED == s_arrClientDev[client].state)
    {
      return s_arrClientDev[client].pcb->remote_ip.addr;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return 0;
  }
}
