/*********************************************************************************************************
* 模块名称：TCPClientBase.c
* 摘    要：TCP客户端驱动模块
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
#include "TCPClientBase.h" 
#include "SysTick.h"
#include "Malloc.h"
#include "stdio.h"
#include "string.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/
//客户端状态
typedef enum
{
  ES_TCPCLIENT_NONE = 0,   //没有连接
  ES_TCPCLIENT_CONNECTING, //正在连接
  ES_TCPCLIENT_CONNECTED,  //连接到服务器了
  ES_TCPCLIENT_CLOSING,    //关闭连接
}EnumTCPClientState;

//客户端设备结构体（LWIP回调函数使用的结构体）
typedef struct
{
  EnumTCPClientState state;                          //当前连接状
  struct tcp_pcb*    pcb;                            //指向当前的pcb
  struct pbuf*       p;                              //指向接收/或传输的pbuf
  StructCirQue       recvCirQue;                     //接收循环队列
  unsigned char      recvBuf[TCP_CLIENT_RX_BUFSIZE]; //接收循环队列的缓冲区
  u32                sendNum;                        //发送缓冲区中的数据量
  unsigned char      sendBuf[TCP_CLIENT_TX_BUFSIZE]; //发送缓冲区
}StructTCPClientDev;

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructTCPClientDev s_structTCPClientDev; //客户端设备

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static err_t TCPClientConnectedCallback(void *arg, struct tcp_pcb *tpcb, err_t err);           //连接回调函数
static err_t TCPClientRecvCallback(void *arg, struct tcp_pcb *tpcb,struct pbuf *p, err_t err); //接收数据回调函数
static void  TCPClientErrorCallback(void *arg, err_t err);                                     //出错回调函数
static err_t TCPClientPollCallback(void *arg, struct tcp_pcb *tpcb);                           //轮询回调函数
static err_t TCPClientSendCallback(void *arg, struct tcp_pcb *tpcb, u16_t len);                //发送回调函数
static void  TCPClientSendData(struct tcp_pcb *tpcb, StructTCPClientDev* es);                  //发送数据
static void  TCPClientConnectionClose(struct tcp_pcb *tpcb, StructTCPClientDev* es);           //关闭连接

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：TCPClientConnectedCallback
* 函数功能：连接到服务器回调函数，连接上服务器后LWIP会调用此函数
* 输入参数：arg：回调参数
*          tpcb：客户端PCB
*          err：错误码
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static err_t TCPClientConnectedCallback(void *arg, struct tcp_pcb *tpcb, err_t err)
{
  StructTCPClientDev* es = &s_structTCPClientDev;
  if(err == ERR_OK)
  {
    //初始化客户端设备
    InitQueue(&es->recvCirQue, es->recvBuf, TCP_CLIENT_RX_BUFSIZE);
    es->state = ES_TCPCLIENT_CONNECTED; //状态为连接成功
    es->pcb = tpcb;                     //记录客户端TCB
    es->p = NULL;                       //清空接收/或传输的pbuf
    es->sendNum = 0;                    //清空发送缓冲区
    
    //设置回调参数和回调函数
    tcp_arg(tpcb, es);                        //使用es更新tpcb的callback_arg
    tcp_recv(tpcb, TCPClientRecvCallback);    //初始化LwIP的tcp_recv回调功能
    tcp_err(tpcb, TCPClientErrorCallback);    //初始化tcp_err()回调函数
    tcp_sent(tpcb, TCPClientSendCallback);    //初始化LwIP的tcp_sent回调功能
    tcp_poll(tpcb, TCPClientPollCallback, 1); //初始化LwIP的tcp_poll回调功能
    err = ERR_OK;
  }
  
  //出错
  else
  {
    //关闭连接
    TCPClientConnectionClose(tpcb, 0);
  }
  return err;
}

/*********************************************************************************************************
* 函数名称：TCPClientRecvCallback
* 函数功能：接收数据回调函数，TCP客户端接收到服务器数据后将会调用此函数
* 输入参数：arg：回调参数，用于传递TCP客户端设备结构体（StructTCPClientDev）地址
*          tpcb：客户端PCB
*          p：数据包首地址
*          err：错误码
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static err_t TCPClientRecvCallback(void *arg, struct tcp_pcb *tpcb,struct pbuf *p, err_t err)
{
  struct pbuf *q;
  StructTCPClientDev* es;
  err_t ret_err;

  //校验arg参数是否为空
  LWIP_ASSERT("arg != NULL",arg != NULL);

  //获取传递TCP客户端设备结构体地址
  es = (StructTCPClientDev*)arg;

  //p为NULL表示服务器已断开
  if(p == NULL)
  {
    //需要关闭TCP 连接了
    es->state = ES_TCPCLIENT_CLOSING;
    es->p = p; 
    ret_err = ERR_OK;
  }
  
  //当接收到一个非空的数据帧,但是err!=ERR_OK
  else if(err!= ERR_OK)
  { 
    if(p)
    {
      //释放接收pbuf
      pbuf_free(p);
    }
    ret_err = err;
  }
  
  //当处于连接状态时
  else if(es->state == ES_TCPCLIENT_CONNECTED)
  {
    //当处于连接状态并且接收到的数据不为空时
    if(p != NULL)
    {
      //遍历完整个pbuf链表
      for(q = p; q != NULL; q = q->next)
      {
        //将数据保存到队列中
        EnQueue(&es->recvCirQue, (DATA_TYPE*)q->payload, q->len);
      }

      //用于获取接收数据,通知LWIP可以获取更多数据
      tcp_recved(tpcb, p->tot_len);

      //释放内存
      pbuf_free(p);

      //数据接收正常
      ret_err = ERR_OK;
    }
  }
  
  //接收到数据但是连接已经关闭
  else
  {
    //用于获取接收数据,通知LWIP可以获取更多数据
    tcp_recved(tpcb,p->tot_len);
    es->p = NULL;

    //释放内存
    pbuf_free(p);
    ret_err = ERR_OK;
  }
  return ret_err;
}

/*********************************************************************************************************
* 函数名称：TCPClientErrorCallback
* 函数功能：TCP客户端出错回调函数
* 输入参数：arg：回调参数，用于传递TCP客户端设备结构体（StructTCPClientDev）地址
*          err：错误码
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void TCPClientErrorCallback(void *arg, err_t err)
{
  LWIP_UNUSED_ARG(err);
  printf("tcp error:%x\r\n", (u32)arg);
}

/*********************************************************************************************************
* 函数名称：TCPClientPollCallback
* 函数功能：TCP客户端轮询回调函数
* 输入参数：arg：回调参数，用于传递TCP客户端设备结构体（StructTCPClientDev）地址
*          tpcb：客户端PCB
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static err_t TCPClientPollCallback(void *arg, struct tcp_pcb *tpcb)
{
  err_t ret_err;
  StructTCPClientDev* es; 

  //获取TCP客户端设备结构体首地址
  es = (StructTCPClientDev*)arg;

  //连接处于空闲可以发送数据
  if(es != NULL)
  {
    //关闭TCP连接
    if(es->state == ES_TCPCLIENT_CLOSING)
    {
      TCPClientConnectionClose(tpcb, es);
    }

    //有数据要发送
    else if(0 != es->sendNum)
    {
      //申请内存
      es->p = pbuf_alloc(PBUF_TRANSPORT, es->sendNum, PBUF_POOL);

      //复制数据到数据包
      pbuf_take(es->p, (const void*)es->sendBuf, es->sendNum);

      //发送数据
      TCPClientSendData(tpcb, es);

      //清空发送数据量
      es->sendNum = 0;

      //释放内存
      if(es->p)
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
* 函数名称：TCPClientSendCallback
* 函数功能：发送回调函数(当从远端主机接收到ACK信号后发送数据)
* 输入参数：arg：回调参数，用于传递TCP客户端设备结构体（StructTCPClientDev）地址
*          tpcb：客户端PCB
*          len：数据量
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static err_t TCPClientSendCallback(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  StructTCPClientDev* es;
  LWIP_UNUSED_ARG(len);

  //获取TCP客户端设备结构体首地址
  es = (StructTCPClientDev*)arg;
  if(es->p)
  {
    //发送数据
    TCPClientSendData(tpcb, es);
  }
  return ERR_OK;
}

/*********************************************************************************************************
* 函数名称：TCPClientSendData
* 函数功能：发送数据
* 输入参数：tpcb：客户端PCB
*          es：TCP客户端设备结构体（StructTCPClientDev）
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void TCPClientSendData(struct tcp_pcb *tpcb, StructTCPClientDev* es)
{
  struct pbuf *ptr; 
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
      //指向下一个pbuf
      es->p = ptr->next;

      //pbuf的ref加一
      if(es->p)
      {
        pbuf_ref(es->p);
      }

      //释放前一个pbuf内存
      pbuf_free(ptr);
    }
    else if(wr_err == ERR_MEM)
    {
      es->p = ptr;
    }

    //将发送缓冲队列中的数据立即发送出去
    tcp_output(tpcb);
  }
}

/*********************************************************************************************************
* 函数名称：TCPClientConnectionClose
* 函数功能：关闭与服务器的连接
* 输入参数：tpcb：客户端PCB
*          es：TCP客户端设备结构体（StructTCPClientDev）
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void TCPClientConnectionClose(struct tcp_pcb *tpcb, StructTCPClientDev* es)
{
  //标记断开连接
  es->state = ES_TCPCLIENT_NONE;

  //移除回调
  tcp_abort(tpcb);
  tcp_arg(tpcb, NULL);
  tcp_recv(tpcb, NULL);
  tcp_sent(tpcb, NULL);
  tcp_err(tpcb, NULL);
  tcp_poll(tpcb, NULL,0);
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：CreateTCPClient
* 函数功能：创建TCP客户端
* 输入参数：ip0-ip3：服务器IP地址，例如192, 168, 1, 104，port：TCP服务器端口
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void CreateTCPClient(u8 ServerIP0, u8 ServerIP1, u8 ServerIP2, u8 ServerIP3, u16 port)
{
  struct tcp_pcb* tcppcb;    //TCP客户端控制块
  struct ip_addr  rmtipaddr; //远端ip地址
  StructLWIPDev*  lwipDev;   //本地网络设备
  u32             retry;     //失败重试计数
  
  //获取本地IP地址
  lwipDev = GetLWIPCommDev();

  //打印各项参数
  printf("Local IP:%d.%d.%d.%d\r\n", lwipDev->ip[0], lwipDev->ip[1], lwipDev->ip[2], lwipDev->ip[3]); //客户端IP
  printf("Remote IP:%d.%d.%d.%d\r\n", ServerIP0, ServerIP1, ServerIP2, ServerIP3);                    //远端IP
  printf("Remotewo Port:%d\r\n", port);                                                               //客户端端口号

  //创建一个新的pcb
  tcppcb = tcp_new();
  if(tcppcb) //创建成功
  {
    //标记正在连接
    s_structTCPClientDev.state = ES_TCPCLIENT_CONNECTING;

    //组合成一个IPV4地址
    IP4_ADDR(&rmtipaddr, ServerIP0, ServerIP1, ServerIP2, ServerIP3);

    //连接到目的地址的指定端口上,当连接成功后回调TCPClientConnectedCallback()函数
    tcp_connect(tcppcb, &rmtipaddr, port, TCPClientConnectedCallback);

    //等待连接成功
    retry = 0;
    while(ES_TCPCLIENT_CONNECTED != s_structTCPClientDev.state)
    {
      //LWIP轮询任务
      LWIPPeriodicHandle();

      //LWIP接收数据包轮询任务
      LWIPReadPackHandle();
      
      //连接失败，重试
      retry++;
      if(retry >= 500)
      {
        retry = 0;
        
        //打印输出提示连接到服务器失败
        printf("连接到服务器失败\r\n");
        printf("重新连接...\r\n");
        
        //关闭连接
        TCPClientConnectionClose(tcppcb, 0);

        //创建一个新的pcb
        tcppcb = tcp_new();
        if(tcppcb)
        {
          //连接到目的地址的指定端口上,当连接成功后回调tcp_client_connected()函数
          tcp_connect(tcppcb, &rmtipaddr, port, TCPClientConnectedCallback);
        }
      }
      
      //延时2ms
      DelayNms(2);
    }

    //连接成功
    printf("连接服务器成功\r\n");
  }
  else
  {
    printf("创建TCP失败\r\n");
  }
}

/*********************************************************************************************************
* 函数名称：ReadTCPClient
* 函数功能：读TCP客户端接收数据缓冲区
* 输入参数：buf-读出数据储存地址，len-读取长度
* 输出参数：void
* 返 回 值：成功读取到的数据量
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
u32 ReadTCPClient(u8* buf, u32 len)
{
  return DeQueue(&s_structTCPClientDev.recvCirQue, buf, len);
}

/*********************************************************************************************************
* 函数名称：WriteTCPClient
* 函数功能：TCP客户端发送数据
* 输入参数：buf-发送数据储存地址，len-发送长度
* 输出参数：void
* 返 回 值：成功发送的数据量
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
u32 WriteTCPClient(u8* buf, u32 len)
{
  u32 sendCnt;

  //将发送数据拷贝到发送缓冲区
  sendCnt = 0;
  while((s_structTCPClientDev.sendNum < TCP_CLIENT_TX_BUFSIZE) && (sendCnt < len))
  {
    s_structTCPClientDev.sendBuf[s_structTCPClientDev.sendNum] = buf[sendCnt];
    s_structTCPClientDev.sendNum++;
    sendCnt++;
  }

  return sendCnt;
}
