/*********************************************************************************************************
* 模块名称：JPEG.h
* 摘    要：JPEG显示模块
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
#ifndef _JPEG_H_
#define _JPEG_H_

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "DataType.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体定义
*********************************************************************************************************/
typedef struct
{
  unsigned char* image;   //图片首地址
  unsigned int   readCnt; //读取位置
  unsigned int   size;    //图片大小
  unsigned int   x0;      //起始横坐标
  unsigned int   y0;      //起始纵坐标
}StructJpegImage;

//主屏幕背景图片
extern const unsigned char s_arrJpegBackgroundImage[37484]; 

//按键按下背景图片（45x45）
extern const unsigned char s_arrJpegkeyPressImage45x45x24[8492];

//按键释放背景图片（45x45）
extern const unsigned char s_arrJpegkeyReleaseImage45x45x24[8491];

//按键按下背景图片（70x45）
extern const unsigned char s_arrJpegkeyPressImage70x45x24[8659];

//按键释放背景图片（70x45）
extern const unsigned char s_arrJpegkeyReleaseImage70x45x24[8732];

//按键按下背景图片（96x45）
extern const unsigned char s_arrJpegkeyPressImage96x45x24[8845];

//按键释放背景图片（96x45）
extern const unsigned char s_arrJpegkeyReleaseImage96x45x24[8926];

//按键按下背景图片（123x45）
extern const unsigned char s_arrJpegkeyPressImage123x45x24[9099];

//按键释放背景图片（123x45）
extern const unsigned char s_arrJpegkeyReleaseImage123x45x24[9219];

//按键按下背景图片（416x45）
extern const unsigned char s_arrJpegkeyPressImage416x45x24[10811];

//按键释放背景图片（416x45）
extern const unsigned char s_arrJpegkeyReleaseImage416x45x24[11162];

/*********************************************************************************************************
*                                              API函数声明
*********************************************************************************************************/
void InitJPEG(void);                                                               //初始化JPEG模块
void DisplayJPEGInFlash(StructJpegImage* image, unsigned int x0, unsigned int y0); //指定位置绘制JPEG图片

#endif
