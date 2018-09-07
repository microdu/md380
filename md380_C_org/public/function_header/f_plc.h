/*************** (C) COPYRIGHT  Co., Ltd****************
* File Name          : f_plc.h
* Author             : 	
* Version            : 
* Date               : 
* Description        : PLC卡相关声明包含文件



********************************************************************************/

#ifndef __f_plc__
#define	__f_plc__

#define		PLC_CARD_ID      			1					// PLC使用ID
#define		INV_PLC_ID      			2					// 变频器PLC使用ID

#define		INV_TRAN_FAST_NUM			6					// 变频器发送快表
#define		INV_TRAN_SLOW_NUM			30					// 变频器发送慢表
#define		INV_REC_FAST_NUM			6					// 变频器接收快表
#define		INV_REC_SLOW_NUM			30					// 变频器接收慢表


#define		INV_FAST_TAB_ADDR			0					// 变频器快表基地址
#define		INV_SLOW_TAB_ADDR			0x100				// 变频器慢表

#define		PLC_DISP_U3NUM				10					// PLC监视功能码长度
#define		PLC_FUNC_C0NUM				15					// PLC使用参加数功能码长度


// 自定义功能码属性定义
typedef struct C0_ATTRIBUTE_STRUCT
{
    Uint16                  lower;          // 下限
    Uint16                  upper;          // 上限
    Uint16                  init;           // 出厂值
    union FUNC_ATTRIBUTE    attribute;      // 属性
} C0_FUNCCODE_ATTRIBUTE;


// 外部使用函数定义
extern Uint16 InvTranTabCfg(struct CANLINK_DATA_BUF *dataPi);
extern Uint16 InvRecTabCfg(struct CANLINK_DATA_BUF *dataPi);
extern void PlcDataFramDeal(struct CANLINK_DATA_BUF *dataPi);


#endif



