/*************** (C) COPYRIGHT 2012   Co., Ltd****************
* File Name          : f_dspcan.h
* Author             : 	
* Version            : V0.0.1
* Date               : 08/09/2012
* Description        : DSP CAN总线底层驱动库，包含文件

********************************************************************************/
#ifndef		__f_dspcan__
#define		__f_dspcan__

// 自动应答邮箱数据定义
//#define		

// CAN波特率宏定义
#define		CAN_BAUD_SUM							7		// CAN波特率总数
#define		CAN_BAUD_1M								6
#define		CAN_BAUD_500K							5
#define		CAN_BAUD_250K							4
#define		CAN_BAUD_125K							3
#define		CAN_BAUD_100K							2
#define		CAN_BAUD_50K							1
#define		CAN_BAUD_20K							0

#define		CAN_INIT_TIME							1		// 初始化进行中
#define		CAN_INIT_SUCC							2		// 初始化成功
#define		CAN_INIT_TIMEOUT						3		// 初始化超时，失败
#define		CAN_INIT_BAUD_ERR						4		// 波特率选择出错

// 收发状态标志
#define		CAN_MBOX_TRAN_SUCC						0		// CAN邮箱操作成功，不表示发送成功
#define		CAN_MBOX_NUM_ERROR						1		// CAN邮箱号出错
#define		CAN_MBOX_BUSY							2		// CAN邮箱忙
#define		CAN_MBOX_EMPTY							3		// CAN接收邮箱空，
#define		CAN_MBOX_REC_SUCC						4		// CAN邮箱接收数据成功
#define		CAN_MBOX_REC_OVER						5		// CAN邮箱接收有数据溢出

typedef struct 
{
	Uint16	BRPREG;
	Uint16	TSEG2REG;
	Uint16 	TSEG1REG;
}CAN_BAUD;



#define		ECANREGS			ECanaRegs					// 选择使用CAN接口		
#define		ECANMBOXES			ECanaMboxes					// 邮箱宏定义
#define		ECANLAMS			ECanaLAMRegs				// 验收滤波寄存器定义


//typedef struct
//{
	

//}

// 外部函数声明
extern Uint16 InitdspECan(Uint16 baud);						// DSP CAN模块初始化
extern void InitTranMbox(Uint16 mbox, Uint32 msgid, Uint32 *dataPi);
															// 初始化发送邮箱
extern void InitRecMbox(Uint16 mbox, Uint32 msgid, Uint32 lam);
															// 初始化邮箱为接收邮箱
Uint16 eCanDataTran(Uint16 mbox, Uint16 len, Uint32 msgid, Uint32 *dataPi);
															// 指定发送邮箱发送数据
extern Uint16 eCanDataRec(Uint16 mbox, Uint32 *dataPi);		// 指定邮箱数据读取
															














#endif


