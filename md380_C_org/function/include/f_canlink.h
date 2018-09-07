/*************** (C) COPYRIGHT 2010  Inovance Technology Co., Ltd****************
* File Name          : f_canlink.h
* Author             : Yanyi	
* Version            : V0.0.1
* Date               : 08/25/2010
* Description        : CAN_LINK驱动库
					  邮箱31可用作自动应答邮箱
					  邮箱30~16可用作接收邮箱
					  15~0可用作发送邮箱


********************************************************************************/

#ifndef	__f_canlink__
#define	__f_canlink__

#define		CAN_REMOTE_EN   		0						// CAN远程帧使能

// 帧类型定义
#define		CAN_REMOTE_FRAME		0xd						// 远程帧		1101
#define		CAN_CONTROL_FRAME		0x8						// 命令控制帧	1000
#define		CAN_CONFIG_FRAME		0xA						// 配置帧		1010
#define		CAN_DATA_FRAME			0xB						// 数据帧		1011

#define     CANlink_ASK             1                       // CAN问帧帧
#define     CANlink_ACK             0                       // 应答帧

// 收发状态标志
#define		CANLINK_RT_SUCC			0						// 数据发送成功
#define		CANLINK_RT_BUSY			1						// 邮箱忙
#define		CANLINK_RT_TIMEOUT		2						// 超时
#define		CANLINK_R_EMPTY			3						// 未接收到数据
#define		CANLINK_R_OVER			4						// 接收数据有溢出

// ID号屏蔽标识
#define		CANLINK_ID_MASK			0x3f					// ID屏蔽标识

// 邮箱数定义
#define		TRAN_MBOX_NUM			4						// 发送邮箱数
#define		REC_MBOX_NUM			4						// 接收邮箱数


// 功能邮箱号
#define		AAM_MBOX_N				31						// 自动应答邮箱号，自动应答邮箱接收优先级最高
#define		REC_BOX_N				30						// 接收邮箱号
#define		TRAN_BOX_N				15						// 发送邮箱号

// 协议命令代码
#define		CAN_LINK_DEL			1						// 删除CAN_LINK配置信息
#define		CAN_LINK_INC			2						// 增加设备CAN_LINK配置信息
#define		CAN_LINK_R_CFG			3						// 读配置
#define		CAN_LINK_R_REG			4						// 读寄存器
#define		CAN_LINK_W_REG			5						// 写寄存器
#define		CAN_LINK_R_INFO			6						// 读站点设备信息
#define		CAN_LINK_R_WAR			7						// 读告警信息
#define		CAN_LINK_W_EEP			0x0A					// 写EEPROM

#define		CAN_TRAN_TAB_CFG		0x0B					// 发送表配置
#define		CAN_REC_TAB_CFG			0x0C					// 接收表配置
#define		CAN_FUN_U3_CFG			0x0D					// U3自定义功能码配置
#define		CAN_FUN_C0_CFG			0x0E					// C0自定义功能码配置
#define		CAN_READ_PLC_INFO		0x0F					// 读PLC卡设备信息


#define		CAN_LINK_S_WAR			0x10					// 发送告警信息
#define		CAN_LINK_Q_CFG			0x20					// 配置请求命令


// 功能码定义
#define		CAN_LINK_S_ADDR			(funcCode.code.commSlaveAddress & CANLINK_ID_MASK) // 本站地址
#define		CAN_LINK_BAUD_SEL		(funcCode.code.commBaudRate/1000)    // 波特率设置

#define     P2P_COMM_HOST           1         // 点对点通讯主机
#define     P2P_COMM_SLAVE          2         // 点对点通讯从机



// CANLINK消息ID位定义
struct	CANLINK_MsgID_BITS	
{
	Uint16	srcSta:8;									// 源站点ID
	Uint16	destSta:8;									// 目标站ID		8
	Uint16	code:8;										// 命令代码		16
	Uint16	aq:1;										// 问答标志		24
	Uint16	framf:4;									// 帧类型标识	25
	Uint16	aam:1;										// 自动应答位	29
	Uint16	ame:1;										// 屏蔽使能位	30
	Uint16	ide:1;										// 				31
};




// CAN_LINK 数据结构
union CANLINK_MsgID
{
	Uint32	all;
	struct	CANLINK_MsgID_BITS bit;
};

struct CANLINK_DATAHL
{
	Uint16 datah;
	Uint16 datal;
};

// 数据定义
union CANLINK_DATA
{
	Uint32 all;
	struct CANLINK_DATAHL data;	
};

// CAN_LINK接收数据类型
struct CANLINK_DATA_BUF
{
	union CANLINK_MsgID msgid;
	union CANLINK_DATA mdl;
	union CANLINK_DATA mdh;	
	Uint32 len;                                             // 缓存数据长度
};

// CAN_LINK接收缓存数据结构定义
struct CANLINK_REC_BUF
{
	Uint16 bufFull;											// 缓存有效标识位，bit0 "1" buf[0]数据有效
	struct CANLINK_DATA_BUF buf[REC_MBOX_NUM];
};


extern Uint16 CanRxTxCon;
extern Uint32 canLinkTicker;

extern void CanlinkFun(void);
extern Uint16 CanControlWriter(Uint16 addr, Uint16 data, Uint16 eeprom);
extern Uint16 CanControlRead(Uint16 addr, Uint16* result);
extern Uint16 CanlinkDataTran(Uint32 *dataPi, Uint16 len, Uint16 timeOut);


#define CAN_LINK_TIME_DEBUG                 0




#endif


