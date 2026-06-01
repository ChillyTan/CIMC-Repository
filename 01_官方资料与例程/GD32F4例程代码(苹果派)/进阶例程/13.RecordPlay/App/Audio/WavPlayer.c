/*********************************************************************************************************
* 模块名称：WavPlayer.c
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

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "WavPlayer.h"
#include "WM8978.h"
#include "I2S.h"
#include "GUITop.h"
#include "stdio.h"
#include "ff.h"
#include "malloc.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define WAV_BUF_SIZE      (1024 * 10) //Wav文件数据缓冲区大小
#define WAV_NAME_LEN_MAX  (512)      //名字缓冲区最大长度

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/
/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
//最新的Wav文件名
static u8 s_arrWavFileName[WAV_NAME_LEN_MAX] = {0};

//Wav播放状态机
static EnumWavPlayerState s_enumPlayerState = WAV_PLAYER_STATE_IDLE;

//播放进度
static u32 s_iSongAllTime; //歌曲时长（s）
static u32 s_iPlayTime;    //播放时长（s）

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitWavPlayer
* 函数功能：初始化Wav播放模块
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitWavPlayer(void)
{
  s_enumPlayerState = WAV_PLAYER_STATE_IDLE;
}

/*********************************************************************************************************
* 函数名称：WavPlayerPoll
* 函数功能：Wav播放轮询任务
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void WavPlayerPoll(void)
{
  //静态变量
  static FIL*     s_pWavFile;       //Wav文件名
  static u8*      s_pAudioBuf1;     //音频数据缓冲区1
  static u8*      s_pAudioBuf2;     //音频数据缓冲区2
  static u32      s_iReadNum;       //成功读取到的数据量
  static u32      s_iBitRate;       //码率（bps）
  static u32      s_iNumOfChannels; //通道数量（1或2）
  static u32      s_iSampleRate;    //采样率
  static u32      s_iBitsPerSample; //通道长度（16、24或32）
  static u32      s_iReadPos;       //读取位置
  static u32      s_iTransmitFlag;  //开启传输标志位
  static u8*      s_iNotUseBuf;     //未在使用的缓冲区

  //临时变量
  FRESULT         result;           //文件操作返回变量
  u32             i;                //循环变量
  StructChunkFMT* fmt;              //FMT块
  u32             temp;             //临时变量
  u32             needReadFlag;     //需要读取新一批数据标志位

  //空闲状态或暂停状态
  if((WAV_PLAYER_STATE_IDLE == s_enumPlayerState) || (WAV_PLAYER_STATE_PAUSE == s_enumPlayerState))
  {
    //终止DMA传输
    if(0 != s_iTransmitFlag)
    {
      I2SEndDMATransmit();
      s_iTransmitFlag = 0;
    }
    return;
  }

  //重新播放
  if(WAV_PLAYER_STATE_RESTART == s_enumPlayerState)
  {
    //关闭文件
    f_close(s_pWavFile);

    //释放内存
    MyFree(SRAMIN, s_pWavFile);
    MyFree(SRAMIN, s_pAudioBuf1);
    MyFree(SRAMIN, s_pAudioBuf2);

    //切换到下一状态
    s_enumPlayerState = WAV_PLAYER_STATE_START;
  }

  //开始播放
  if(WAV_PLAYER_STATE_START == s_enumPlayerState)
  {
		printf("WavPlayerPoll：开始播放录音\r\n");
		
    //校验文件名
    if(0 == s_arrWavFileName[0])
    {
      s_enumPlayerState = WAV_PLAYER_STATE_IDLE;
      return;
    }

    //申请内存
    s_pWavFile   = MyMalloc(SRAMIN, sizeof(FIL));
    s_pAudioBuf1 = MyMalloc(SRAMIN, WAV_BUF_SIZE);
    s_pAudioBuf2 = MyMalloc(SRAMIN, WAV_BUF_SIZE);
    if((NULL == s_pWavFile) || (NULL == s_pAudioBuf1) || (NULL == s_pAudioBuf2))
    {
      printf("WavPlayerPoll：申请内存失败\r\n");
      MyFree(SRAMIN, s_pWavFile);
      MyFree(SRAMIN, s_pAudioBuf1);
      MyFree(SRAMIN, s_pAudioBuf2);
      s_enumPlayerState = WAV_PLAYER_STATE_IDLE;
      return;
    }

    //打开Wav文件
    result = f_open(s_pWavFile, (const TCHAR*)s_arrWavFileName, FA_OPEN_EXISTING | FA_READ);
    if (result !=  FR_OK)
    {
      printf("WavPlayerPoll：打开Wav文件失败\r\n");
      MyFree(SRAMIN, s_pWavFile);
      MyFree(SRAMIN, s_pAudioBuf1);
      MyFree(SRAMIN, s_pAudioBuf2);
      s_enumPlayerState = WAV_PLAYER_STATE_IDLE;
      return;
    }
    else
    {
      printf("WavPlayerPoll：播放Wav文件：%s\r\n", s_arrWavFileName);
    }
		
    //读取第一批数据
    f_read(s_pWavFile, s_pAudioBuf1, WAV_BUF_SIZE, &s_iReadNum);

    //获取FMT块
    fmt = (StructChunkFMT*)(s_pAudioBuf1 + sizeof(StructChunkRIFF));

    //获取歌曲信息
    s_iBitRate       = fmt->byteRate;      //码率
    s_iNumOfChannels = fmt->numOfChannels; //通道数量
    s_iSampleRate    = fmt->sampleRate;    //采样率
    s_iBitsPerSample = fmt->bitsPerSample; //通道长度（16、24或32）
    if(2 != s_iNumOfChannels)
    {
      printf("WavPlayerPoll：通道数量错误，只支持双通道Wav文件\r\n"); 
      s_enumPlayerState = WAV_PLAYER_STATE_FINISH; 
      return;
    }

    //计算歌曲总长
    s_iSongAllTime = s_pWavFile->fsize / fmt->byteRate;

    //清空录音时长记录
    s_iPlayTime = 0;

		//显示录音文件名和进度条背景
    UpdataText((char*)s_arrWavFileName);
    BeginShowProgress();
		
    //打印歌曲信息
    printf("WavPlayerPoll：bit rate: %dkB/s, song time: %ds\r\n", s_iBitRate / 1000, s_iSongAllTime);
    printf("WavPlayerPoll：sample rate: %dkHz, channels:%d, sample bit：%dbit\r\n", s_iSampleRate / 1000, s_iNumOfChannels, s_iBitsPerSample);

    //配置WM8978和I2S
    WM8978ADDACfg(1, 0);     //使能DAC输出，关闭ADC输入
    WM8978InputCfg(0, 0, 0); //关闭MIC、LineIn和AUX输入
    WM8978OutputCfg(1, 0);   //放音开启
    switch (s_iBitsPerSample)
    {
			case 16: WM8978I2SCfg(2, 0); I2SConfig(s_iSampleRate, DATA_LEN_16, CH_LEN_16); break;
      case 24: printf("WavPlayerPoll：格式错误，不支持24位的通道长度\r\n"); s_enumPlayerState = WAV_PLAYER_STATE_FINISH; return;
      case 32: WM8978I2SCfg(2, 3); I2SConfig(s_iSampleRate, DATA_LEN_32, CH_LEN_32); break;
      default: printf("WavPlayerPoll：格式错误，未知的通道长度\r\n"); s_enumPlayerState = WAV_PLAYER_STATE_FINISH; return;
    }
    printf("WavPlayerPoll：配置WM8978和I2S成功\r\n");

    //查找Data Chunk开头
    temp = 0;
    while(0x61746164 != *(u32*)((u8*)s_pAudioBuf1 + temp))
    {
      temp = temp + 4;
    }

    //设置发送起始位置（要确保4字节对齐）
    s_iReadPos = temp + 8;
    while(0 != (s_iReadPos % 4))
    {
      s_iReadPos = s_iReadPos - s_iBitsPerSample / 8;
    }
    printf("WavPlayerPoll：查找Data Chunk成功，数据起始位置：%d\r\n", s_iReadPos);

    //清空字节计数
    s_iReadNum = 0;

    //设置文件读取位置
    f_lseek(s_pWavFile, s_iReadPos);

    //标记尚未开启传输
    s_iTransmitFlag = 0;

    //切换到下一状态
    s_enumPlayerState = WAV_PLAYER_STATE_PLAY;
  }

  //正在播放
  if(WAV_PLAYER_STATE_PLAY == s_enumPlayerState)
  {
    //第一次读取音频数据时默认使用缓冲区1
    if(0 == s_iTransmitFlag)
    {
      s_iNotUseBuf = s_pAudioBuf1;
      needReadFlag = 1;
    }

    //获取未在使用的缓冲区
    else
    {
      if(s_iNotUseBuf != (u8*)I2SGetTransmitNotUsedMemoryAddr())
      {
        s_iNotUseBuf = (u8*)I2SGetTransmitNotUsedMemoryAddr();
        needReadFlag = 1;
      }
      else
      {
        needReadFlag = 0;
      }
    }

    //读取新一批数据
    if(1 == needReadFlag)
    {
      f_read(s_pWavFile, s_iNotUseBuf, WAV_BUF_SIZE, &s_iReadNum);

      //记录读取位置
      s_iReadPos = s_iReadPos + s_iReadNum;

      //更新播放时间
      s_iPlayTime = s_iReadPos / s_iBitRate;
			
			//更新显示歌曲进度
      UpdataProgress(s_iPlayTime, s_iSongAllTime);

      //最后一批数据缓冲区末尾清零操作
      if(s_iReadNum != WAV_BUF_SIZE)
      {
        for(i = s_iReadNum; i < WAV_BUF_SIZE; i++)
        {
          s_iNotUseBuf[i] = 0;
        }
      }
    }

    //开启DMA双缓冲区传输
    if(0 == s_iTransmitFlag)
    {
      s_iTransmitFlag = 1;

      //双通道发送数据
      if(16 == s_iBitsPerSample)
      {
        I2SDualDMATransmit16((u16*)s_pAudioBuf1, (u16*)s_pAudioBuf2, s_iReadNum / 2);
      }
      else if(32 == s_iBitsPerSample)
      {
        I2SDualDMATransmit32((u32*)s_pAudioBuf1, (u32*)s_pAudioBuf2, s_iReadNum / 4);
      }
    }

    //发送完成
    if(s_pWavFile->fptr >= s_pWavFile->fsize)
    {
      //终止DMA传输
      I2SEndDMATransmit();
      s_iTransmitFlag = 0;

      //切换到下一状态
      s_enumPlayerState = WAV_PLAYER_STATE_FINISH;
      printf("WavPlayerPoll：文件发送完毕\r\n");
    }
  }

  //完成
  if(WAV_PLAYER_STATE_FINISH == s_enumPlayerState)
  {
    //终止DMA传输
    if(0 != s_iTransmitFlag)
    {
      I2SEndDMATransmit();
      s_iTransmitFlag = 0;
    }

    //串口输出提示
    printf("WavPlayerPoll：播放Wav结束\r\n");

    //关闭文件
    f_close(s_pWavFile);

    //释放内存
    MyFree(SRAMIN, s_pWavFile);
    MyFree(SRAMIN, s_pAudioBuf1);
    MyFree(SRAMIN, s_pAudioBuf2);

    //切换到下一状态
    s_enumPlayerState = WAV_PLAYER_STATE_IDLE;
  }
}

/*********************************************************************************************************
* 函数名称：SetWavFileName
* 函数功能：设置音频文件名（包含名字和路径）
* 输入参数：name:音频文件名（包含名字和路径）
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void SetWavFileName(char* name)
{
  u32 i;

  //保存名字
  i = 0;
  while((0 != name[i]) && (i <= (WAV_NAME_LEN_MAX - 2)))
  {
    s_arrWavFileName[i] = name[i];
    i++;
  }
  s_arrWavFileName[i] = 0;
}

/*********************************************************************************************************
* 函数名称：StartWavPlay
* 函数功能：开始播放
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void StartWavPlay(void)
{
  //空闲状态下启动播放
  if(WAV_PLAYER_STATE_IDLE == s_enumPlayerState)
  {
    s_enumPlayerState = WAV_PLAYER_STATE_START;
    WavPlayerPoll();
  }
	
	//暂停状态切换到播放状态
  if(WAV_PLAYER_STATE_PAUSE == s_enumPlayerState)
  {
    s_enumPlayerState = WAV_PLAYER_STATE_PLAY;
  }
}

/*********************************************************************************************************
* 函数名称：PauseWavPlay
* 函数功能：暂停播放
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void PauseWavPlay(void)
{
  //播放状态切换到暂停状态
  if(WAV_PLAYER_STATE_PLAY == s_enumPlayerState)
  {
    s_enumPlayerState = WAV_PLAYER_STATE_PAUSE;
  }
}

/*********************************************************************************************************
* 函数名称：IsWavPlayerIdle
* 函数功能：判断Wav播放器是否为空闲
* 输入参数：void
* 输出参数：void
* 返 回 值：0-忙碌，1-空闲
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
u8 IsWavPlayerIdle(void)
{
  return (WAV_PLAYER_STATE_IDLE == s_enumPlayerState);
}

/*********************************************************************************************************
* 函数名称：GetWavTime
* 函数功能：获取歌曲播放时间
* 输入参数：void
* 输出参数：currentTime:歌曲当前播放时间
            allTime:歌曲总时间
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：空闲状态下返回0
*********************************************************************************************************/
void GetWavTime(u32* currentTime, u32* allTime)
{
  if(WAV_PLAYER_STATE_IDLE != s_enumPlayerState)
  {
    *currentTime = s_iPlayTime;
    *allTime = s_iSongAllTime;
  }
  else
  {
    *currentTime = 0;
    *allTime = 0;
  }
}

/*********************************************************************************************************
* 函数名称：GetWavPlayerState
* 函数功能：获取Wav播放器状态
* 输入参数：void
* 输出参数：void
* 返 回 值：Wav播放器状态
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
EnumWavPlayerState GetWavPlayerState(void)
{
  return s_enumPlayerState;
}
