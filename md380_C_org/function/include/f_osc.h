/*************** (C) COPYRIGHT 2010  Inovance Technology Co., Ltd****************
* File Name          : sci_osc.c
* Author             : Yanyi	
* Version            : V0.0.1
* Date               : 05/18/2010
* Description        : MD380示波器SCI后台软件模块包含文件
***************************************************************************************************
* 修改				：
* 版本				：V1.00
* 时间				：
* 说明				：修改增加数据校验，修改发送双缓存操作模式防止交替出错
***************************************************************************************************
* 修改				：
* 版本				：V1.10
* 时间				：
* 说明				：增加数据SCI出错处理，防止波特率不正确造成SCI进入错误状态
*					  修改正波特率计算问题造成19200波特率设置错误
**************************************************************************************************/

#ifndef	 _sci_osc_h_
#define	 _sci_osc_h_

#include "f_funcCode.h"
// 内部宏定义
// 功能配置区
#define			OSC_SCI_SEL						1			// "1"选择SCIA	“2”选择SCIB
#define			OSC_CON_CHECK					1			// 控制帧校验使能
#define			OSC_DATA_CHECK					1			// 数据帧校验使能，“1”使用简单奇偶校验，“0”关闭
#define			FC_CODE_CONTROL_EN				1			// 后台控制功能禁能，“1”使能，“0”关闭
#define			OSC_TX_INT_EN					1			// OSC发送中断使能 "1"使用中断
#define			SCI_INT_LOAD_RAM				0			// SCI中断加载到RAM内运行

#if DSP_2803X
#define			PERIPHERAL_CLK					15E6		// 外设时钟
#else
#define			PERIPHERAL_CLK					25E6		// 外设时钟
#endif
// 发送中断加速使能，当发送数据长度是4的整数倍可使能，其它情况使能将会出错
#define			SCI_TX_INT_SPEEDUP				1			// 使能加速 

#if DSP_2803X
    #define			RS485_ENABLE			    0			// 28035不允许使能485功能
#else
    #define			RS485_ENABLE			    1			// 使能485功能
#endif


// RS485的接收发送切换

#if RS485_ENABLE == 1
// RS485的接收发送切换
    #if DSP_2803X
    #define RS485_RTS_O (GpioDataRegs.GPBDAT.bit.GPIO39)
    #else
    #define RS485_RTS_O (GpioDataRegs.GPADAT.bit.GPIO27)
    #endif
    #define RS485_R_O     0
	#define RS485_T_O     1

#endif

// 以下部分一般不需修改
/*********************************************************************************************************************/
#if 	(1 == OSC_SCI_SEL)
	#define			SCI_OSC_REGS					SciaRegs// 选择SCIA接口
#else
	#define			SCI_OSC_REGS					ScibRegs// 选择SCIB
#endif

#define			OSC_CON_FRAME_HEAD				0xAC		// 命令帧帧头
#define			OSC_DATA_FRAME_HEAD_A			0xA5		// 数据帧帧头
#define			OSC_DATA_FRAME_HEAD_B			0xCD		// 
		

// 内部常数与命令
#define			OSC_BUF_DATA_LEN				64			// 示波器发送缓冲数据长度
#define			FC_FRAME_LEN					8			// 控制帧长度
// 
#define			OSC_SCI_EN						0xCA		// 示波器模块使能，其它值关闭

// 串口状态
#define			SCI_RT_BUSY						0x03		// SCI忙
#define			SCI_RS485_TX_BUSY				0x04		// SCI使用485接口发送忙	


// 示波器串口控制命令
#define         FC_COMM_TEST                    0x30        // 通讯测试命令
#define			FC_CHANNL_SEL					0x31		// 通道选择命令
#define			FC_PARA_CFG						0x32		// 参数配置
//#define			FC_AQU_SPEED					0x32		// 采样速度
//#define			FC_BAUD_CFG						0x33		// 波特率修改
//#define			FC_RUN_CONTINUE					0x34		// 连续运行
#define			FC_START_OSC					0x33		// 启动
#define			FC_STOP_OSC						0x34		// 停止
#define			FC_OSC_OFF						0x35		// 关闭示波器模式	

// 功能访问
#define			FC_READ_FC_DATA					0x41		// 读内部功能码操作
#define			FC_WRITE_FC_DATA				0x42		// 写内部功能码操作

// 后台控制结构
#define			CONTROL_FRAME_HEAD				OscConFrameBuf[0]
#define			CONTROL_FRAME_FC				OscConFrameBuf[1]
#define			CONTROL_FRAME_P1				OscConFrameBuf[2]
#define			CONTROL_FRAME_P2				OscConFrameBuf[3]
#define			CONTROL_FRAME_P3				OscConFrameBuf[4]
#define			CONTROL_FRAME_P4				OscConFrameBuf[5]
#define			CONTROL_FRAME_CRCL				OscConFrameBuf[6]
#define			CONTROL_FRAME_CRCH				OscConFrameBuf[7]


#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif


typedef unsigned char  uint8;                   /* defined for unsigned 8-bits integer variable 	无符号8位整型变量  */
typedef signed   char  int8;                    /* defined for signed 8-bits integer variable		有符号8位整型变量  */
typedef unsigned int   uint16;                  /* defined for unsigned 16-bits integer variable 	无符号16位整型变量 */
//typedef signed   short int16;                   /* defined for signed 16-bits integer variable 		有符号16位整型变量 */
typedef unsigned  long uint32;                  /* defined for unsigned 32-bits integer variable 	无符号32位整型变量 */
//typedef signed   int   int32;                   /* defined for signed 32-bits integer variable 		有符号32位整型变量 */
typedef float          fp32;                    /* single precision floating point variable (32bits) 单精度浮点数（32位长度） */
typedef double         fp64;                    /* double precision floating point variable (64bits) 双精度浮点数（64位长度） */


// 获取示波器数据宏定义



// 示波器后台模块控制块声明
typedef struct
{
	uint8	status;            								// 采样状态(0-停止采样  1-开始采样) 
    uint8	baudRate;										// 波特率选择 0：115'200	1：57'600	2: 19'200 
    uint8 	interval;          								// 采样间隔(*0.5ms) 	1~8								
    uint8	runContinue;           							// 连续运行(1-停机工作  0-停机不工作)
    uint8	ch1Addr;           								// 通道1地址(采样数据1地址), 写入“0xFF”禁用该通道
    uint8	ch2Addr;           								// 通道2地址(采样数据2地址)
    uint8	ch3Addr;           								// 通道3地址(采样数据3地址)
    uint8	ch4Addr;           								// 通道4地址(采样数据4地址)   
	uint8   chSum;											// 通道总数
} DSP_OSC_CON_DATA;

// 示波器发送数据结构
typedef	struct
{
	uint8	frameHead1;										// 数据帧头
	uint8 	frameHead2;
	uint8 	frameNum;										// 帧号
	uint8	oscDataBuf[OSC_BUF_DATA_LEN];					// 数据缓存区
	uint8	check;											// 校验
	uint8	rwPI;											// 数据读写索引
	uint8	full;											// 满标志					
} OSC_DATA_TYPE;

// 串口中断处理控制结构
typedef struct
{
	uint8 *buf;												// 缓冲区指针
	uint8 len;												// 收、发长度
	uint8 busy;												// 串口忙信号，"1" 发送中		"0"空闲状态 
	uint8 rxflag;											// 接收标志，收到数据rxflag置“1”
//	uint8 err;												// 串口出错
} SCI_RT_CON_DATA;



// 外部调用函数与全局变量声明
extern void OscSciFunction(void);


#endif														// end sic_osc.h




