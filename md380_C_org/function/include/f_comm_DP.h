#ifndef __F_COMM_DP_H__
#define __F_COMM_DP_H__

#include "f_funcCode.h"

// 帧格式类型
#define DP_TO_DSP 4
#define DSP_TO_DP 5
#define READ_DP_PARAMETER 2
#define DSP_TO_DP_PARAMETER 3
#define ERR_FRAME 7

// 错误类型
#define DP_OK 0
#define DP_ERR 5
#define DSP_REWORK_ADD 1
#define DSP_REWORK_MODE 2

// DP卡数据模式
#define PPO1 1
#define PPO2 2
#define PPO3 3
#define PPO5 5

// DP卡数据模式 对应的数据流的字节个数
#define PPO1_NUMBER 12
#define PPO2_NUMBER 20
#define PPO3_NUMBER 4
#define PPO5_NUMBER 32

// DP卡串口波特率设置
#define SCI_BAUD_RATE1 0           // 115.2kpbs
#define SCI_BAUD_RATE2 1           // 208.3kpbs
#define SCI_BAUD_RATE3 2           // 256kpbs
#define SCI_BAUD_RATE4 3           // 512kpbs

#define OLD_DATA_MODE  0           // 旧的数据传输模式,罗梅林提供的方案

#define BUG_SCI_BACK_DATA 0        // 测试DSP串口反馈接受的数据


#define SCI_SEND_CONNECT_NUMBER 5  // 测试串口发送的数据个数
#define SCI_SEND_READ_NUMBER 5

#define SEND_DATA_NUMBER 32        // profibus 各种模式下最大的数据个数 PPO5 最多32个字节
#define RCV_DATA_NUMBER 32

#define SEND_DATA_SCI_NUMBER 40    // 串口通讯数据交互的缓冲区数据个数 实际最多使用35个
#define RCV_DATA_SCI_NUMBER 40

#define SCI_RCV_ONE_FARME_TIME 60000

#define SCI_RCV_ONE_FARME_TIME1 600000

#define RCV_CRC_ERR_NUMBER 10

#define NO_FIFO 1

 // DP参数
typedef  struct
{
    Uint16 dpAddress;      // DP卡地址
    Uint16 dpDataFormat;   // DP卡数据模式
} DP_PARAMETER;  

extern DP_PARAMETER dpParameter;

enum COMM_DP_STATUS
{
    SCI_CONNECT = 0,                 // SCI连接中
    SCI_CONNECT_OK = 1               // SCI连接成功
};

enum DP_READ_FLAG_State{DP_READ_NO=0, DP_READ_YES=!DP_READ_NO};
enum SCI_RCV_FLAG_State{SCI_RCV_NO=0, SCI_RCV_YES=!SCI_RCV_NO};
enum SCI_RCV_TICK_FLAG_State{SCI_RCV_TICK_NO=0, SCI_RCV_TICK_YES=!SCI_RCV_TICK_NO};

struct FRAME_START_BITS
{  
    Uint16 frameType:3;               // 帧类型    低3位
    Uint16 errType:3;                 // 错误类型
    Uint16 commSciFlag:1;             // 通讯标志   高位   往后往高位走    
    Uint16 commProfibusFlag:1;        // 通讯标志   高位   往后往高位走  
};

union FRAME_START {
    Uint16                   all;
    struct FRAME_START_BITS   bit;
};


struct SCI_DATA_DP
{
    volatile struct SCI_REGS *pSciRegs;
    
    Uint16 sendData_SCI[SEND_DATA_SCI_NUMBER];   // 发送给STM32数据
    Uint16 rcvData_SCI[RCV_DATA_SCI_NUMBER];     // 接收STM32的数据
    Uint16 commDpRcvNumber;                      // 接受数据计数
    Uint16 commDpSendNum;                        // 发送的数据计数
    Uint16 commDpSendNumMax;                     // 每次发送的数据个数
    Uint16 frameFlagDp;                          // 不同数据帧
    Uint16 rcvTickDp;                            // 接受数据的时间间隔
    Uint16 rcvDataJuageFlag;
    Uint16 rcvCrcErrCounter;                     // 接收数据CRC校验出错
    Uint16 rcvRigthFlag;                         // 接收桢头第一数据正确标志
    
    union FRAME_START frameStart;                // 接收到的桢头
    union FRAME_START frameSendStart;            // 发送的桢头
    
    enum SCI_RCV_TICK_FLAG_State rcvTickFlag;    // 接收计时的标志
    enum DP_READ_FLAG_State dpReadFlag;          // dp卡读地址 模式标志
    enum SCI_RCV_FLAG_State sciRcvFlag;          // sci接收完成标志

    enum COMM_DP_STATUS commDpStatus;            // SCI工作状态
    
};
extern struct SCI_DATA_DP sciM380DpData;


// 与数据处理接口的数组及标志
extern Uint16 sendData_DP[SEND_DATA_NUMBER];     // 发送的DP卡数据
extern Uint16 rcvData_DP[RCV_DATA_NUMBER];       // 接收DSP的数据
enum DP_SCI_COMM_RCV_FLAG{DP_SCI_COMM_RCV_NO=0, DP_SCI_COMM_RCV_YES=!DP_SCI_COMM_RCV_NO};     // SCI接收数据完成
enum DP_SCI_COMM_SEND_FLAG{DP_SCI_COMM_SEND_NO=0, DP_SCI_COMM_SEND_YES=!DP_SCI_COMM_SEND_NO}; // DSP准备发送的数据准备完成


void InitSetSciDp(struct SCI_DATA_DP *p);
void InitSciaGpioDp(void);
void SciDpDeal(struct SCI_DATA_DP *p);
extern void InitSciDpBaudRate(struct SCI_DATA_DP *p);
#endif




























