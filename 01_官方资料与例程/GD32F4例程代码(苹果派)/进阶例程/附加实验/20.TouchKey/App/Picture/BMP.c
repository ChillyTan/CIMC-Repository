/*********************************************************************************************************
* 模块名称：LED.c
* 摘    要：LED模块
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
#include "BMP.h"
#include "BMPImage.h"
#include "TLILCD.h"
#include "stdio.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
//图象数据压缩的类型
#define BI_RGB        0  //没有压缩.RGB 5,5,5.
#define BI_RLE8       1  //每个象素8比特的RLE压缩编码，压缩格式由2字节组成(重复象素计数和颜色索引)；
#define BI_RLE4       2  //每个象素4比特的RLE压缩编码，压缩格式由2字节组成
#define BI_BITFIELDS 	3  //每个象素的比特由指定的掩码决定。

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/
//位图 = 文件头 + 信息头 + 调色板(可选) + 数据体

//BMP文件头
typedef struct
{
  u16 type;      //文件标志，只为'BM'，必须是BM，十六进制中则是0x424D；
  u32 size;      //文件大小，单位为字节
  u16 reserved1; //保留，必须为0
  u16 reserved2; //保留，必须为0
  u32 offBits;   //位图数据的起始位置相对于文件头的偏移量，单位为字节。
}StructBmpFileHeader;

//BMP信息头
typedef struct
{
  u32 infoSize;       //该结构体所需的字节数
  u32 width;          //位图的宽度（像素单位）
  u32 height;         //位图的高度（像素单位）
  u16 planes;         //目标设备的级别，必须为1
  u16 colorSize;      //每个像素所需的位数
  u32 compression;    //说明图象数据压缩的类型，位图压缩类型，必须是0（不压缩）
  u32 imageSize;      //位图的大小(其中包含了为了补齐行数是4的倍数而添加的空字节)，以字节为单位
  u32 xPelsPerMeter;  //位图水平分辨率，每米像素数
  u32 yPelsPerMeter;  //位图垂直分辨率，每米像素数
  u32 colorUsed;      //位图实际使用的颜色表中的颜色数
  u32 colorImportant; //位图显示过程中重要的颜色数，如果是 0，表示都重要
}StructBmpInfoHeader;

//颜色表
typedef struct
{
  u8 blue;     //指定蓝色强度
  u8 green;    //指定绿色强度
  u8 red;      //指定红色强度
  u8 reserved; //保留，设置为0
}StructRGB;

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static StructBmpFileHeader s_structFileHeader;
static StructBmpInfoHeader s_structInfoHeader;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static u8  GetHeaderInFlash(const unsigned char *image);         //获取文件头和信息头
static u8* GetLineInFlash(const unsigned char *image, u32 line); //获取一行数据

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：GetHeaderInFlash
* 函数功能：获取文件头和信息头
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static u8 GetHeaderInFlash(const unsigned char *image)
{ 
  u8 i;

  //获取文件标识
  i = 0;
  s_structFileHeader.type = image[i] | ((u32)(image[i + 1]) << 8);
  
  //获取文件大小
  i = 2;
  s_structFileHeader.size = image[i] | ((u32)(image[i + 1]) << 8) | ((u32)(image[i + 2]) << 16) | ((u32)(image[i + 3]) << 24);
  
  //保留数据1
  i = 6;
  s_structFileHeader.reserved1 = image[i] | ((u32)(image[i + 1]) << 8);
  
  //保留数据2
  i = 8;
  s_structFileHeader.reserved2 = image[i] | ((u32)(image[i + 1]) << 8);
  
  //偏移量
  i = 10;
  s_structFileHeader.offBits = image[i] | ((u32)(image[i + 1]) << 8) | ((u32)(image[i + 2]) << 16) | ((u32)(image[i + 3]) << 24);
  
  //获取Info结构体字节数
  i = 14;
  s_structInfoHeader.infoSize = image[i] | ((u32)(image[i + 1]) << 8) | ((u32)(image[i + 2]) << 16) | ((u32)(image[i + 3]) << 24);
  
  //获取图片宽度
  i = 18;
  s_structInfoHeader.width = image[i] | ((u32)(image[i + 1]) << 8) | ((u32)(image[i + 2]) << 16) | ((u32)(image[i + 3]) << 24);
  
  //获取图片高度
  i = 22;
  s_structInfoHeader.height = image[i] | ((u32)(image[i + 1]) << 8) | ((u32)(image[i + 2]) << 16) | ((u32)(image[i + 3]) << 24);
  
  //获取目标设备的级别
  i = 26;
  s_structInfoHeader.planes = image[i] | ((u32)(image[i + 1]) << 8);
  
  //获取每个像素所需的位数
  i = 28;
  s_structInfoHeader.colorSize = image[i] | ((u32)(image[i + 1]) << 8);
  
  //获取压缩的类型
  i = 30;
  s_structInfoHeader.compression = image[i] | ((u32)(image[i + 1]) << 8) | ((u32)(image[i + 2]) << 16) | ((u32)(image[i + 3]) << 24);
  
  //获取位图的大小
  i = 34;
  s_structInfoHeader.imageSize = image[i] | ((u32)(image[i + 1]) << 8) | ((u32)(image[i + 2]) << 16) | ((u32)(image[i + 3]) << 24);
  
  //获取水平分辨率
  i = 38;
  s_structInfoHeader.xPelsPerMeter = image[i] | ((u32)(image[i + 1]) << 8) | ((u32)(image[i + 2]) << 16) | ((u32)(image[i + 3]) << 24);
  
  //获取垂直分辨率
  i = 42;
  s_structInfoHeader.yPelsPerMeter = image[i] | ((u32)(image[i + 1]) << 8) | ((u32)(image[i + 2]) << 16) | ((u32)(image[i + 3]) << 24);
  
  //获取位图实际使用的颜色表中的颜色数
  i = 46;
  s_structInfoHeader.colorUsed = image[i] | ((u32)(image[i + 1]) << 8) | ((u32)(image[i + 2]) << 16) | ((u32)(image[i + 3]) << 24);
  
  //获取位图显示过程中重要的颜色数
  i = 50;
  s_structInfoHeader.colorImportant = image[i] | ((u32)(image[i + 1]) << 8) | ((u32)(image[i + 2]) << 16) | ((u32)(image[i + 3]) << 24);
  
  return 0;
}

/*********************************************************************************************************
* 函数名称：GetLineInFlash
* 函数功能：获取一行数据
* 输入参数：line：行号，从0开始
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：从SD卡中读取数据时读取地址需是4的整数倍，一行数据量不是4的倍数则补零
*********************************************************************************************************/
static u8* GetLineInFlash(const unsigned char *image, u32 line)
{

  u32  readAddr;  //读取地址相对于起始地址偏移量
  u32  lineSize;  //一行数据量，要4字节对齐
  u8*  p;         //返回地址
  
  //计算32位图片一行数据大小
  if(32 == s_structInfoHeader.colorSize)
  {
    lineSize = s_structInfoHeader.width * 4;
  }
  
  //计算24位图片一行数据大小
  else if(24 == s_structInfoHeader.colorSize)
  {
    //行数据量需是4的倍数
    lineSize = s_structInfoHeader.width * 3;
    while((lineSize % 4) != 0)
    {
      lineSize++;
    }
  }

  //计算16位图片一行数据大小
  else if(16 == s_structInfoHeader.colorSize)
  {
    //行数据量需是4的倍数
    lineSize = s_structInfoHeader.width * 2;
    while((lineSize % 4) != 0)
    {
      lineSize++;
    }
  }

  //计算行数对应的地址
  readAddr = s_structFileHeader.offBits + lineSize * line;

  //得到行首位置
  p = (u8*)image + readAddr;

  return p;
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitBMP
* 函数功能：初始化位图显示模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitBMP(void)
{
  
}

/*********************************************************************************************************
* 函数名称：BMPDisplayInFlash
* 函数功能：指定位置显示位图
* 输入参数：fileName：文件名，包含完整路径；x：横坐标；y：纵坐标
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*          1、起点在左上角，位图是从左至右从下到上的（从倒数第一行开始存的）
*          2、目前仅支持32位和24位格式位图显示
*          3、为了加快位图显示，透明度仅支持显示和不显示，透明度大于0则显示该点，透明度等于0则不显示
*********************************************************************************************************/
void BMPDisplayInFlash(const unsigned char *image, u32 x, u32 y)
{
  u32     x0, y0, x1, y1; //起点、终点
  u32     byteCnt;        //单次读取字节计数
  u8      red;            //红色通道数值
  u8      green;          //绿色通道数值
  u8      blue;           //蓝色通道数值
  u8      alpha;          //透明度
  u16     color;          //最终颜色值
  u32     lineCnt;        //读取行数计数
  u8*     line;           //行数据首地址
  u32     yn;             //实际写入纵坐标
  
  //读取BMP文件头和信息头
  GetHeaderInFlash(image);

  //校验“BM”
  if(s_structFileHeader.type != 0x4D42)
  {
    printf("BMPDisplay: this is not a BMP file\r\n");
    return;
  }

//图片信息输出
#if BMP_PRINT_INFO == 1
  printf("BMPDisplay: BMP file infomation\r\n");
  printf("name: %s\r\n", fileName);
  printf("size: %d\r\n", s_structFileHeader.size);
  printf("bit off: %d\r\n", s_structFileHeader.offBits);
  printf("width: %d\r\n", s_structInfoHeader.width);
  printf("height: %d\r\n", s_structInfoHeader.height);
  printf("color size: %d\r\n", s_structInfoHeader.colorSize);
  printf("image size: %d\r\n", s_structInfoHeader.imageSize);
#endif

  //仅支持32位、24位格式位图
  if(!((32 == s_structInfoHeader.colorSize) || (24 == s_structInfoHeader.colorSize)))
  {
    printf("BMPDisplay: unsupported format!!!\r\n");
    return;
  }

  //设定显示范围
  x0 = x;
  y0 = y;
  x1 = x0 + s_structInfoHeader.width;
  y1 = y0 + s_structInfoHeader.height;
  if(x1 >= g_structTLILCDDev.pWidth)
  {
    x1 = g_structTLILCDDev.pWidth;
  }
  if(y1 >= g_structTLILCDDev.pHeight)
  {
    y1 = g_structTLILCDDev.pHeight;
  }

  //位图是从左往右，从下至上储存的
  yn = y1 - 1;

  //行数计数清零
  lineCnt = 0;

  //纵坐标循环
  for(y = y0; y < y1; y++)
  {
    //读取一整行数据
    line = GetLineInFlash(image, lineCnt);

    //字节计数清零
    byteCnt = 0;

    //行数计数加一
    lineCnt++;

    //横坐标循环
    for(x = x0; x < x1; x++)
    {
      //32位像素点
      if(32 == s_structInfoHeader.colorSize)
      {
        //获取像素值
        blue  = line[byteCnt + 0] >> 3; //蓝色
        green = line[byteCnt + 1] >> 2; //绿色
        red   = line[byteCnt + 2] >> 3; //红色
        alpha = line[byteCnt + 3];      //透明度
        
        //24位RGB转16位RGB
        color = (((u32)red) << 11) | (((u32)green) << 5) | blue;
        
        //字节计数增加
        byteCnt = byteCnt + 4;
      }
      
      //24位像素点
      else if(24 == s_structInfoHeader.colorSize)
      {
        //获取像素值
        blue  = line[byteCnt + 0] >> 3; //蓝色
        green = line[byteCnt + 1] >> 2; //绿色
        red   = line[byteCnt + 2] >> 3; //红色
        
        //24位RGB转16位RGB
        color = (((u32)red) << 11) | (((u32)green) << 5) | blue;
        alpha = 1;
        
        //字节计数增加
        byteCnt = byteCnt + 3;
      }

      //画点，直接往屏幕写数据即可
      if(alpha >= 50)
      {
        LCDDrawPoint(x, yn, color);
      }
    }

    //从坐标从下往上扫描
    yn = yn - 1;
  }
}
