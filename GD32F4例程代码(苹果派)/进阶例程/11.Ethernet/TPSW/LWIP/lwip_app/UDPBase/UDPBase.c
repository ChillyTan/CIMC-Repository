/*********************************************************************************************************
* 模块名称：UDPBase.c
* 摘    要：UDP驱动模块
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
#include "UDPBase.h"
#include "SysTick.h"
#include "stdio.h"
#include "string.h"
#include "Queue.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/
//UDP连接状态
typedef enum
{
  ES_UDP_NONE = 0,  //没有连接
  ES_UDP_CONNECTED, //连接上了
}EnumUDPStates;

//客户端设备结构体
typedef struct
{
  EnumUDPStates    state;                   //当前连接状
  struct udp_pcb*  pcb;                     //指向当前的pcb
  StructCirQue     recvCirQue;              //接收循环队列
  unsigned char    recvBuf[UDP_RX_BUFSIZE]; //接收循环队列的缓冲区
  u32              sendNum;                 //发送缓冲区中的数据量
  unsigned char    sendBuf[UDP_TX_BUFSIZE]; //发送缓冲区
}StructUDPDev;

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructUDPDev s_structUDPDev; //UDP设备结构体

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：UDPRecvCallback
* 函数功能：UDP数据接收回调函数
* 输入参数：arg：用户参数
*          upcb：UDP的PCB
*          p：数据包首地址
*          addr：远程IP地址
*          port：远程端口号
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void UDPRecvCallback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
  struct pbuf *q;

  //接收到不为空的数据时
  if(p != NULL)
  {
    //遍历完整个pbuf链表
    for(q = p; q != NULL; q = q->next)
    {
      //将数据保存到队列中
      EnQueue(&s_structUDPDev.recvCirQue, (DATA_TYPE*)q->payload, q->len);
    }

    pbuf_free(p);//释放内存
  }
  
  //UDP连接断开
  else
  {
    udp_disconnect(upcb);
    s_structUDPDev.state = ES_UDP_NONE; //标记连接断开
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：CreateUDPConnect
* 函数功能：创建UDP连接
* 输入参数：ip0-ip3：服务器IP地址，例如192, 168, 1, 104，port：TCP服务器端口
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void CreateUDPConnect(u8 ServerIP0, u8 ServerIP1, u8 ServerIP2, u8 ServerIP3, u16 port)
{
  err_t err;
  struct ip_addr rmtipaddr;  //远端ip地址
  StructLWIPDev*  lwipDev;

  //获取本地IP地址
  lwipDev = GetLWIPCommDev();

  //打印各项参数
  printf("Local IP:%d.%d.%d.%d\r\n", lwipDev->ip[0], lwipDev->ip[1], lwipDev->ip[2], lwipDev->ip[3]); //服务器IP
  printf("Remote IP:%d.%d.%d.%d\r\n", ServerIP0, ServerIP1, ServerIP2, ServerIP3);                    //远端IP
  printf("Remotewo Port:%d\r\n", port);                                                               //客户端端口号

  //标记连接断开
  s_structUDPDev.state = ES_UDP_NONE;

  //创建一个新的pcb
  s_structUDPDev.pcb = udp_new();
  if(s_structUDPDev.pcb)
  { 
    //生成IPV4地址
    IP4_ADDR(&rmtipaddr, ServerIP0, ServerIP1, ServerIP2, ServerIP3);

    //UDP客户端连接到指定IP地址和端口号的服务器
    err = udp_connect(s_structUDPDev.pcb, &rmtipaddr, port);
    if(err == ERR_OK)
    {
      //绑定本地IP地址与端口号
      err = udp_bind(s_structUDPDev.pcb, IP_ADDR_ANY, port);

      //绑定完成
      if(err == ERR_OK)
      {
        //注册接收回调函数
        udp_recv(s_structUDPDev.pcb, UDPRecvCallback, NULL);

        //标记已连接
        s_structUDPDev.state = ES_UDP_CONNECTED;

        //初始化接收队列
        InitQueue(&s_structUDPDev.recvCirQue, s_structUDPDev.recvBuf, UDP_RX_BUFSIZE);

        //清空发送缓冲区
        s_structUDPDev.sendNum = 0;
      }
    }
  }

  //打印输出连接成功
  if(ES_UDP_CONNECTED == s_structUDPDev.state)
  {
    printf("UDP已连接\r\n");
  }
}

/*********************************************************************************************************
* 函数名称：UDPConnectionClose
* 函数功能：关闭UDP连接
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void UDPConnectionClose(void)
{
  udp_disconnect(s_structUDPDev.pcb); //断开UDP连接
  udp_remove(s_structUDPDev.pcb);     //移除UDP
  s_structUDPDev.state = ES_UDP_NONE; //标记连接断开
}

/*********************************************************************************************************
* 函数名称：ReadUDP
* 函数功能：读UDP接收数据缓冲区
* 输入参数：buf-读出数据储存地址，len-读取长度
* 输出参数：void
* 返 回 值：成功读取到的数据量
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
u32 ReadUDP(u8* buf, u32 len)
{
  //校验是否已连接
  if(ES_UDP_CONNECTED != s_structUDPDev.state)
  {
    return 0;
  }

  return DeQueue(&s_structUDPDev.recvCirQue, buf, len);
}

/*********************************************************************************************************
* 函数名称：WriteUDP
* 函数功能：UDP发送数据
* 输入参数：buf-发送数据储存地址，len-发送长度
* 输出参数：void
* 返 回 值：成功发送的数据量
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
u32 WriteUDP(u8* buf, u32 len)
{
  u32 sendCnt;
  struct pbuf *ptr;

  //校验是否已连接
  if(ES_UDP_CONNECTED != s_structUDPDev.state)
  {
    return 0;
  }

  //将发送数据拷贝到发送缓冲区
  sendCnt = 0;
  while((s_structUDPDev.sendNum < UDP_TX_BUFSIZE) && (sendCnt < len))
  {
    s_structUDPDev.sendBuf[s_structUDPDev.sendNum] = buf[sendCnt];
    s_structUDPDev.sendNum++;
    sendCnt++;
  }

  //发送数据
  ptr = pbuf_alloc(PBUF_TRANSPORT, s_structUDPDev.sendNum, PBUF_POOL); //申请内存
  if(ptr)
  {
    //设置发送缓冲区
    ptr->payload = (void*)s_structUDPDev.sendBuf;

    //udp发送数据
    udp_send(s_structUDPDev.pcb, ptr);

    //释放内存
    pbuf_free(ptr);
  }
  s_structUDPDev.sendNum = 0;

  return sendCnt;
}
