/*********************************************************************************************************
* 模块名称：WavPlayer.h
* 摘    要：Wav解码模块
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
#ifndef _WAV_PLAYER_H_
#define _WAV_PLAYER_H_

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "DataType.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/
//Wav播放状态机
typedef enum
{
  WAV_PLAYER_STATE_IDLE,    //空闲
  WAV_PLAYER_STATE_RESTART, //重新播放，用于切歌
  WAV_PLAYER_STATE_START,   //开始播放
  WAV_PLAYER_STATE_PLAY,    //正在播放
  WAV_PLAYER_STATE_PAUSE,   //暂停
  WAV_PLAYER_STATE_FINISH,  //完成
}EnumWavPlayerState;

//RIFF块
typedef __packed struct
{
  u32 chunkID;   //chunk id;这里固定为"RIFF",即0x46464952
  u32 chunkSize; //集合大小;文件总大小-8
  u32 format;    //格式;WAVE,即0X45564157
}StructChunkRIFF;

//Format块
typedef __packed struct
{
  u32 chunkID;       //chunk id;这里固定为"fmt ",即0x20746D66
  u32 chunkSize;     //子集合大小(不包括ID和Size);这里为:20.
  u16 audioFormat;   //音频格式;0X10,表示线性PCM;0X11表示IMA ADPCM
  u16 numOfChannels; //通道数量;1,表示单声道;2,表示双声道;
  u32 sampleRate;    //采样率;0X1F40,表示8Khz
  u32 byteRate;      //字节速率; 
  u16 blockAlign;    //块对齐(字节); 
  u16 bitsPerSample; //单个采样数据大小;4位ADPCM,设置为4
  // u16 byteExtraData; //附加的数据字节;2个; 线性PCM,没有这个参数
  // u16 extraData;     //附加的数据,单个采样数据块大小;0X1F9:505字节  线性PCM,没有这个参数
}StructChunkFMT;

//Fact块
typedef __packed struct
{
  u32 chunkID;      //chunk id;这里固定为"fact",即0X74636166;
  u32 chunkSize;    //子集合大小(不包括ID和Size);这里为:4.
  u32 numOfSamples; //采样的数量; 
}StructChunkFACT;

//Data块
typedef __packed struct 
{
  u32 chunkID;   //chunk id;这里固定为"data",即0x61746164
  u32 chunkSize; //子集合大小(不包括ID和Size);文件大小-60.
}StructChunkDATA;

//Wav头
typedef __packed struct
{
  StructChunkRIFF riff; //riff块
  StructChunkFMT fmt;   //fmt块
  // StructChunkFACT fact; //fact块 线性PCM,没有这个结构体
  StructChunkDATA data; //data块
}StructWavHeader;

/*********************************************************************************************************
*                                              API函数声明
*********************************************************************************************************/
void InitWavPlayer(void);        //初始化Wav播放模块
void WavPlayerPoll(void);        //Wav播放轮询任务
void SetWavFileName(char* name); //设置音频文件名
void StartWavPlay(void);         //开始/继续播放
void PauseWavPlay(void);         //暂停播放
u8   IsWavPlayerIdle(void);      //判断Wav播放器是否为空闲
void GetWavTime(u32* currentTime, u32* allTime); //获取歌曲播放时间
EnumWavPlayerState GetWavPlayerState(void);      //获取Wav播放器状态

#endif
