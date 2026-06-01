/*********************************************************************************************************
* 模块名称：Recorder.c
* 摘    要：录音模块
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
#include "Recorder.h"
#include "WM8978.h"
#include "I2S.h"
#include "SysTick.h"
#include "ff.h"
#include "stdio.h"
#include "WavPlayer.h"
#include "GUITop.h"
#include "Timer.h"
#include "Malloc.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define SEND_BUF_SIZE          (32)       //发送缓冲区大小（按字节计数）
#define RECORDER_BUF_SIZE      (1024 * 1) //录音数据缓冲区大小（按字节计数）
#define RECORDER_NAME_LEN_MAX  (256)      //名字缓冲区最大长度（按字节计数）

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/
/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
//录音机状态机
static EnumRecorderState s_enumRecorderState = RECORDER_IDLE;

//发送双缓冲区（录音时需要发送空数据来驱动）
static u16 s_arrSendBuf1[SEND_BUF_SIZE / 2] = {0};
static u16 s_arrSendBuf2[SEND_BUF_SIZE / 2] = {0};

//接收双缓冲区
static u16 s_arrReceiveBuf1[RECORDER_BUF_SIZE / 2];
static u16 s_arrReceiveBuf2[RECORDER_BUF_SIZE / 2];

//名字缓冲区（含路径）
static char s_arrFileName[RECORDER_NAME_LEN_MAX];

//录音时间
static u64 s_iSongTime = 0;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void EnterRecordMode(void);                          //激活录音模式
static void ExitRecordMode(void);                           //退出录音模式
static void InitWaveHeaderStruct(StructWavHeader* wavhead); //初始化Wav头
static void GetNewRecName(char *name);                      //获得新的文件名
static void CheckRecDir(void);                              //校验录音机目录是否存在，若不存在则新建该目录
static void ShowRecordTime(u32 time);                       //显示录音时长

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：EnterRecordMode
* 函数功能：激活录音模式
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void EnterRecordMode (void)
{
  //配置WM8978
  WM8978ADDACfg(0, 1);     //使能ADC输入，禁用DAC输出
  WM8978InputCfg(1, 0, 0); //开启MIC录音，禁止Line In和AUX输入
  WM8978OutputCfg(0, 0);   //关闭放音输出
  WM8978MICGain(31);       //MIC增益设置
  WM8978LineinGain(3);     //Line In增益设置
  WM8978I2SCfg(2, 0);      //使用飞利浦标准，16位采样率

  //配置I2S
  if(0 != I2SConfig(8000, DATA_LEN_16, CH_LEN_16))
  {
    printf("EnterRecordMode ：不支持的采样率\r\n");
    while(1){}
  }

  //触发I2S接收传输
  I2SDualDMATransmit16(s_arrSendBuf1, s_arrSendBuf2, SEND_BUF_SIZE / 2);

  //I2S接收数据
  I2SDualDMAReceive16(s_arrReceiveBuf1, s_arrReceiveBuf2, RECORDER_BUF_SIZE / 2);
}

/*********************************************************************************************************
* 函数名称：ExitRecordMode
* 函数功能：退出录音模式
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ExitRecordMode(void)
{
  //终止I2S接收数据
  I2SEndDMAReceive();

  //终止I2S接收触发传输
  I2SEndDMATransmit();
}

/*********************************************************************************************************
* 函数名称：InitWaveHeaderStruct
* 函数功能：初始化Wav头
* 输入参数：wavhead：Wav头结构体
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void InitWaveHeaderStruct(StructWavHeader* wavhead)
{
  wavhead->riff.chunkID      = 0x46464952;                //"RIFF"
  wavhead->riff.chunkSize    = 0;                         //还未确定,最后需要计算
  wavhead->riff.format       = 0x45564157;                //"WAVE"
  wavhead->fmt.chunkID       = 0x20746D66;                //"fmt "
  wavhead->fmt.chunkSize     = 16;                        //大小为16个字节
  wavhead->fmt.audioFormat   = 0x01;                      //0x01,表示PCM，0x02,表示IMA ADPCM
  wavhead->fmt.numOfChannels = 2;                         //双声道
  wavhead->fmt.sampleRate    = 8000;                      //8Khz采样率 采样速率
  wavhead->fmt.byteRate      = wavhead->fmt.sampleRate*2; //16位,即2个字节
  wavhead->fmt.blockAlign    = 2;                         //块大小,2个字节为一个块
  wavhead->fmt.bitsPerSample = 16;                        //16位PCM
  wavhead->data.chunkID      = 0x61746164;                //"data"
  wavhead->data.chunkSize    = 0;                         //数据大小,还需要计算
}

/*********************************************************************************************************
* 函数名称：GetNewRecName
* 函数功能：获得新的文件名
* 输入参数：void
* 输出参数：name:文件名（包含名字和路径）
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：打开文件时FIL必须是静态变量或动态内存分配的内存，录音文件默认储存在SD卡中
*********************************************************************************************************/
static void GetNewRecName(char *name)
{
  static FIL s_fileRec; //歌曲文件
  FRESULT    result;    //文件操作返回变量
  u32        index;     //录音文件计数

  index = 0;
  while(index < 0xFFFFFFFF)
  {
    //生成新的名字
    sprintf((char*)name, "0:recorder/REC%d.wav", index);

    //检查当前文件是否已经存在（若是能成功打开则说明文件已存在）
    result = f_open(&s_fileRec, (const TCHAR*)name, FA_OPEN_EXISTING | FA_READ);
    if(FR_NO_FILE == result)
    {
      break;
    }
    else
    {
      f_close(&s_fileRec);
    }

    index++;
  }
}

/*********************************************************************************************************
* 函数名称：CheckRecDir
* 函数功能：校验录音机目录是否存在，若不存在则新建该目录
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void CheckRecDir(void)
{
  DIR     recDir; //目标路径
  FRESULT result; //文件操作返回变量

  result = f_opendir(&recDir,"0:recorder");
  if(FR_NO_PATH == result)
  {
    f_mkdir("0:recorder");
  }
  else
  {
    f_closedir(&recDir);
  }
}

/*********************************************************************************************************
* 函数名称：ShowRecordTime
* 函数功能：显示录音时长
* 输入参数：time：音频播放总长
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：为避免频繁更新显示造成卡顿，每隔500ms更新一次时间显示
*********************************************************************************************************/
static void ShowRecordTime(u32 time)
{
  static u64 s_ilastTime   = 0;    //上次更新显示系统时间
  static u64 s_iCurrenTime = 0;    //当前系统时间
  static u8*  s_iTimeBuf   = NULL; //显示时间字符串转换缓冲区
  u32 secound, min, hour;

  //获取当前时间
  s_iCurrenTime = GetSysTime();
  if((s_iCurrenTime - s_ilastTime) >= 500)
  {
    s_iTimeBuf   = MyMalloc(SDRAMEX, RECORDER_NAME_LEN_MAX);
    if(NULL == s_iTimeBuf)
    {
      printf("ShowRecordTime：申请内存失败\r\n");
      return;
    }

    //计算秒
    secound = time % 60;

    //计算分钟
    time = time / 60;
    min = time % 60;

    //计算小时
    time = time / 60;
    hour = time;

    //字符串转换
    sprintf((char*)s_iTimeBuf, "%02d:%02d:%02d", hour, min, secound);

    //刷新显示
    UpdataText((char*)s_iTimeBuf);

    //释放内存
    MyFree(SDRAMEX, s_iTimeBuf);

    //保存时间
    s_ilastTime = s_iCurrenTime;
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitRecorder
* 函数功能：初始化录音机录音机
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitRecorder(void)
{
  s_enumRecorderState = RECORDER_IDLE;
}

/*********************************************************************************************************
* 函数名称：RecorderPoll
* 函数功能：录音机轮询任务
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：打开文件时FIL必须是静态变量或动态内存分配的内存
*********************************************************************************************************/
void RecorderPoll(void)
{
  static StructWavHeader s_structWavHeader; //Wav文件头
  static FIL  s_filfRec;       //歌曲文件
  static u8*  s_pAudioBuf;     //音频数据临时缓冲区
  static u32  s_iAudioByteCnt; //音频数据总量计数
  FRESULT     result;          //文件操作返回变量
  u32         writeNum;        //成功写入数据量

  //空闲状态或暂停状态
  if((RECORDER_IDLE == s_enumRecorderState) || (RECORDER_PAUSE == s_enumRecorderState))
  {
    return;
  }

  //开始录音
  if(RECORDER_START == s_enumRecorderState)
  {
    printf("RecorderPoll：开始录音\r\n");

    //校验路径
    CheckRecDir();

    //获得新的录音文件名
    GetNewRecName(s_arrFileName);
		
		//保存录音名到录音播放模块，用于录音播放，不用可删除
    SetWavFileName(s_arrFileName);
		
    //创建录音文件，如果文件已存在，则它将被截断并覆盖
    f_open(&s_filfRec, (const TCHAR*)s_arrFileName, FA_CREATE_ALWAYS | FA_WRITE);
		
    printf("RecorderPoll：成功创建录音文件：%s\r\n", s_arrFileName);

    //清除计数
    s_iAudioByteCnt = 0;
		
		//清空进度条显示
    ClearProgress();

    //初始化Wav文件头
    InitWaveHeaderStruct(&s_structWavHeader);

    //进入录音模式
    EnterRecordMode ();

    //记录当前I2S未使用的缓冲区
    s_pAudioBuf = (u8*)I2SGetReceiveNotUsedMemoryAddr();

    //切换到下一个状态
    s_enumRecorderState = RECORDER_REV;

    //录音时长清零
    s_iSongTime = 0;
  }

  //接收音频数据
  if(RECORDER_REV == s_enumRecorderState)
  {
    //I2S双DMA切换了缓冲区，说明有一帧数据已准备好
    if(s_pAudioBuf != (u8*)I2SGetReceiveNotUsedMemoryAddr())
    {
      //保存缓冲区地址
      s_pAudioBuf = (u8*)I2SGetReceiveNotUsedMemoryAddr();

      //将音频数据写到文件中
      f_write(&s_filfRec, s_pAudioBuf, RECORDER_BUF_SIZE, &writeNum);

      //更新音频数据量计数
      s_iAudioByteCnt = s_iAudioByteCnt + RECORDER_BUF_SIZE;
    }

    //记录时间
    s_iSongTime = s_iAudioByteCnt / 32000;
		
		//更新时间显示
    ShowRecordTime(s_iSongTime);
  }

  //完成录音
  if(RECORDER_FINISH == s_enumRecorderState)
  {
    //记录时间
    s_iSongTime = s_iAudioByteCnt / 32000;
		
		//更新时间显示
    ShowRecordTime(s_iSongTime);

    //输出提示
    printf("RecorderPoll：完成录音\r\n");

    //保存整个文件大小到Wav文件头
    s_structWavHeader.riff.chunkSize = s_iAudioByteCnt + 36;

    //保存音频数据量到Wav文件头
    s_structWavHeader.data.chunkSize = s_iAudioByteCnt;

    //将Wav文件头写入文件中
    f_lseek(&s_filfRec, 0); //偏移到文件起始位置
    result = f_write(&s_filfRec, (const void*)(&s_structWavHeader), sizeof(StructWavHeader), &writeNum);
    if(FR_OK != result)
    {
      printf("RecorderPoll：写入文件头失败\r\n");
    }

    //关闭文件
    f_close(&s_filfRec);

    //退出录音模式
    ExitRecordMode();

    //进入空闲状态
    s_enumRecorderState = RECORDER_IDLE;

    //打印提示
    printf("RecorderPoll：成功保存录音文件：%s\r\n", s_arrFileName);
  }
}

/*********************************************************************************************************
* 函数名称：StartEndRecorder
* 函数功能：开始/结束录音
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：仅限空闲状态下使用
*********************************************************************************************************/
void StartEndRecorder(void)
{
  //空闲状态下启动录音
  if(RECORDER_IDLE == s_enumRecorderState)
  {
    s_enumRecorderState = RECORDER_START;
    RecorderPoll();
  }

	//暂停状态切换到录音状态
  else if(RECORDER_PAUSE == s_enumRecorderState)
  {
    s_enumRecorderState = RECORDER_REV;
  }
	
  //录音状态下停止录音
  else if(RECORDER_REV == s_enumRecorderState)
  {
    s_enumRecorderState = RECORDER_FINISH;
    RecorderPoll();
  }
}

/*********************************************************************************************************
* 函数名称：PauseContinueRecoder
* 函数功能：暂停录音
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：仅限录音状态下使用
*********************************************************************************************************/
void PauseRecorder(void)
{
  //录音状态切换到暂停状态
  if(RECORDER_REV == s_enumRecorderState)
  {
    s_enumRecorderState = RECORDER_PAUSE;
  }

}

/*********************************************************************************************************
* 函数名称：IsRecorderIdle
* 函数功能：判断录音机是否为空闲状态
* 输入参数：void
* 输出参数：void
* 返 回 值：0:录音机不为空闲状态
						1:录音机为空闲状态
* 创建日期：2021年07月01日
* 注    意：仅限录音或暂停状态下使用
*********************************************************************************************************/
u8 IsRecorderIdle(void)
{
  return (RECORDER_IDLE == s_enumRecorderState);
}

/*********************************************************************************************************
* 函数名称：GetRecordName
* 函数功能：获取录音名字
* 输入参数：void
* 输出参数：void
* 返 回 值：字符串首地址
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
char* GetRecordName(void)
{
  return s_arrFileName;
}

/*********************************************************************************************************
* 函数名称：GetRecordTime
* 函数功能：获取录音时长
* 输入参数：void
* 输出参数：void
* 返 回 值：录音时长（s）
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
u64 GetRecordTime(void)
{
  return s_iSongTime;
}
