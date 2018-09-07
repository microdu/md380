//======================================================================
//
// Time-stamp: 
//
// P2P comm
// 点对点通讯数据处理
//
//======================================================================

#ifndef __F_P2P_H__
#define __F_P2P_H__

#define  COMM_P2P_COMM_ADDRESS_DATA     0x1001  // 点对点通讯数据地址
#define  COMM_P2P_COMM_ADDRESS_COMMAND          // 点对点通讯数据地址
#define  COMM_P2P_MASTER_ADDRESS      1         // 点对点通讯主机固定地址
#define  COMM_P2P_SLAVE_ADDRESS       2         // 点对点通讯从机固定地址

#define P2P_OUT_TORQUE      0   // 输出转矩
#define P2P_FRQ_SET         1   // 运行频率
#define P2P_FRQ_AIM         2   // 设定频率
#define P2P_FRQ_FDB         3   // 反馈频率

#define P2P_REV_TORQUE_SET  0   // 转矩给定
#define P2P_REV_FRQ_SET     1   // 频率给定

#if DEBUG_F_P2P_CTRL
typedef struct
{
    Uint16 p2pEnable;     // P2P当前是否有效
    Uint16 P2PSendData;   // 通讯发送数据
    Uint16 P2PRevData;    // 通讯接收数据
    Uint16 processValue;  // 接收数据进行线性处理后
    Uint16 p2pSendPeriod; // 通讯发送周期
    Uint16 p2pSendTcnt;   // 点对点发送计时
    Uint16 p2pCommErrTcnt;// 点对点通讯异常计时
} P2P_DATA_STRUCT;

extern P2P_DATA_STRUCT p2pData;

void P2PDataDeal(void);
#elif
#endif
#endif



