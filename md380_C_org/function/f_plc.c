/*************** (C) COPYRIGHT 2012 Co., Ltd****************
* File Name          : f_plc.c
* Author             : 	
* Version            : V0.0.1
* Date               : 09/25/2012
* Description        : PLC卡功能文件



********************************************************************************/
#include "f_funcCode.h"
#include "f_dspcan.h"
#include "f_canlink.h"
#include "f_comm.h"
#include "f_plc.h"


#define DEBUG_F_PLC              1


#if DEBUG_F_PLC

Uint16 InvTrFastIndex = 1,									// 发送表索引
	   InvTrSlowIndex = 1;

Uint16 InvTrFastTab[INV_TRAN_FAST_NUM+1] = {0, 0x703d};		// 变频器发送快表
Uint16 InvTrSlowTab[INV_TRAN_SLOW_NUM+1] = {0, 0};			// 发送慢表
Uint16 InvRecFastTab[INV_REC_FAST_NUM+1] = {0, 0};			// 接收快表
Uint16 InvRecSlowTab[INV_REC_SLOW_NUM+1] = {0, 0};  		// 接收慢表

// 内部函数定义
Uint16 FuncCodeRwAtrrib(Uint16 addr,Uint16 rwMode);


// 其它文件引用的宏
// 某些功能码，通讯不能进行W
#define COMM_NO_W_FC_0  GetCodeIndex(funcCode.code.tuneCmd) // 调谐
#define COMM_NO_W_FC_1  GetCodeIndex(funcCode.code.menuMode)// 菜单模式
#define COMM_NO_W_FC_2  GetCodeIndex(funcCode.code.motorFcM2.tuneCmd)      // 调谐
#define COMM_NO_W_FC_3  GetCodeIndex(funcCode.code.motorFcM3.tuneCmd)      // 调谐
#define COMM_NO_W_FC_4  GetCodeIndex(funcCode.code.motorFcM4.tuneCmd)      // 调谐
#define COMM_NO_W_FC_5  GetCodeIndex(funcCode.code.funcParaView)           // 功能菜单模式属性
// 某些功能码，通讯不能进行R
#define COMM_NO_R_FC_0  GetCodeIndex(funcCode.code.userPassword)            // 用户密码
// 某些功能码，通讯不能进行RW
#define COMM_NO_RW_FC_0 GetCodeIndex(funcCode.code.userPasswordReadOnly)    // 只读用户密码
#define COMM_NO_RW_FC_2  GetCodeIndex(funcCode.code.plcEnable)               // PLC功能
#define COMM_READ_CURRENT_FC GetCodeIndex(funcCode.group.u0[4])              // 通讯读取电流

#define COMM_READ_FC    0       							// 通讯读功能码
#define COMM_WRITE_FC   1       							// 通讯写功能码



/*******************************************************************************
* 函数名称          : Uint16 InvTranTabCfg(struct CANLINK_DATA_BUF *dataPi)
* 入口参数			: 数据缓存指针
* 出口				："0" 		操作成功
*					: "其它"	参加数出错
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 09/25/2010
* 说明				: 变频器发送表配置
********************************************************************************/
Uint16 InvTranTabCfg(struct CANLINK_DATA_BUF *dataPi)
{
	Uint16 err;
	err = FuncCodeRwAtrrib(dataPi->mdl.data.datal, COMM_READ_FC);
	if (err)
		return (COMM_ERR_PARA); 							// 配置数据错 无效参数

	err = COMM_ERR_ADDR;		                            // 不成功无效地址
	if (dataPi->mdl.data.datah >= INV_SLOW_TAB_ADDR)		// 配置发送慢表
	{
		if ( (dataPi->mdl.data.datah - INV_SLOW_TAB_ADDR) <= INV_TRAN_SLOW_NUM )
		{
			InvTrSlowTab[dataPi->mdl.data.datah - INV_SLOW_TAB_ADDR] = dataPi->mdl.data.datal;
			err = COMM_ERR_NONE;							// 操作成功
		}
	}
	else if (dataPi->mdl.data.datah != 1)					// 发送快表，“1”地址固定不允许写操作
	{
		if (dataPi->mdl.data.datah <= INV_TRAN_FAST_NUM)
		{
			InvTrFastTab[dataPi->mdl.data.datah] = dataPi->mdl.data.datal;
			err = COMM_ERR_NONE;							// 操作成功
		}
	}
	
	return (err);
}

/*******************************************************************************
* 函数名称          : Uint16 InvRecTabCfg(struct CANLINK_DATA_BUF *dataPi)
* 入口参数			: 数据缓存指针
* 出口				："0" 操作成功
*					: "其它"	参加数出错
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 09/25/2010
* 说明				: 变频器接收表配置
********************************************************************************/
Uint16 InvRecTabCfg(struct CANLINK_DATA_BUF *dataPi)
{
	Uint16 err;
	err = FuncCodeRwAtrrib(dataPi->mdl.data.datal, COMM_WRITE_FC);
	if (err)
		return (err);										// 配置数据错

	err = COMM_ERR_PARA;									// 预置参数出错		
	if (dataPi->mdl.data.datah >= INV_SLOW_TAB_ADDR)		// 配置接收慢表
	{
		if ( (dataPi->mdl.data.datah - INV_SLOW_TAB_ADDR) <= INV_TRAN_SLOW_NUM )
		{
			InvRecSlowTab[dataPi->mdl.data.datah - INV_SLOW_TAB_ADDR] = dataPi->mdl.data.datal;
			err = COMM_ERR_NONE;							// 操作成功
		}
	}
	else                                                    // 接收快表
	{
		if (dataPi->mdl.data.datah <= INV_TRAN_FAST_NUM)
		{
			InvRecFastTab[dataPi->mdl.data.datah] = dataPi->mdl.data.datal;
			err = COMM_ERR_NONE;							// 操作成功
		}
	}
	return (err);
}


/*******************************************************************************
* 函数名称          : Uint16 FuncCodeRwAtrrib(Uint16 addr,Uint16 rwMode)
* 入口参数			: addr 		访问地址
*					: reMode	读写模式
* 出口				："0"		可操作
*					：“!=0”		不可操作
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 09/25/2010
* 说明				: 功能码读写属性判断
********************************************************************************/
Uint16 FuncCodeRwAtrrib(Uint16 addr,Uint16 rwMode)
{
	Uint16 index, group, grade, highH;
    Uint16 funcCodeGradeComm[FUNCCODE_GROUP_NUM];
//    Uint16 errType = COMM_ERR_NONE;
	if (addr == 0)
		return COMM_ERR_NONE;								// 可以写"0"取消设置
		
    highH = (addr & 0xF000);
// 验证功能码地址块	
	if (
//			(highH == 0x0000)||      							// Fx-RAM
        (highH == 0xF000) ||  								// Fx
        (highH == 0xA000) ||   								// Ax
        (highH == 0xB000) ||  								// Bx
        (highH == 0xC000) ||  								// Cx
		((highH == 0x7000) && (rwMode == COMM_READ_FC) ) ||	// Ux	只读
		( ((addr & 0xFF00) == 0x7300) && (rwMode == COMM_WRITE_FC) )
															// U3组可以写
//        || ((addr & 0xFF00) == 0x1F00)          			// FP，1Fxx 用户密码参数不允许加入表格
		) 
    {
															// 地址有效
    }
    else
    {
        return COMM_ERR_ADDR;								// 返回地址无效s
    }
// 获取group, grade
    group = (addr >> 8) & 0x0F;								// 取得组
    grade = addr & 0xFF;
	
    if (0xA000 == highH)                 					// Ax
    {
        group += FUNCCODE_GROUP_A0;
    }
    else if (0xB000 == highH)            					// Bx
    {
        group += FUNCCODE_GROUP_B0;
    }
    else if (0xC000 == highH)            					// Cx
    {
        group += FUNCCODE_GROUP_C0;
    }
/*    else if ((addr & 0xFF00) == 0x1F00)  					// FP 不允许表操作
    {
        group = FUNCCODE_GROUP_FP;
    }
*/
    else if (0x7000 == highH)            					// Ux
    {
        group += FUNCCODE_GROUP_U0;
    }
	
	if (group == FC_GROUP_FACTORY)							// FF组 系统(厂家功能码)锁定
		return COMM_ERR_ADDR;//COMM_ERR_SYSTEM_LOCKED;
		
// 更新通讯情况下，对每一group，用户可以操作的grade个数
    UpdataFuncCodeGrade(funcCodeGradeComm);
        
    if (grade >= funcCodeGradeComm[group]) 					// 超过界限
    {
        return COMM_ERR_ADDR;
    }
// 验证特定功能码，读、写属性
    index = GetGradeIndex(group, grade);    				// 计算功能码序号
    if (
			(COMM_NO_RW_FC_0 == index) ||       			// 某些功能码，通讯不能进行RW
			(COMM_NO_RW_FC_2  == index)||
			(	(COMM_WRITE_FC == rwMode) &&       			// 某些功能码，通讯不能进行W
				(	(COMM_NO_W_FC_0 == index) ||
					(COMM_NO_W_FC_1 == index) ||
					(COMM_NO_W_FC_2 == index) ||
					(COMM_NO_W_FC_3 == index) ||
					(COMM_NO_W_FC_4 == index) ||
					(COMM_NO_W_FC_5 == index)
				)
			) ||
			((COMM_READ_FC == rwMode) && (COMM_NO_R_FC_0 == index))
															// 某些功能码，通讯不能进行R
		)
    {
        return COMM_ERR_ADDR;      // 无效地址
    }

	return COMM_ERR_NONE;
}

/*******************************************************************************
* 函数名称          : void PlcDataFramDeal(struct CANLINK_DATA_BUF *dataPi)
* 入口参数			: dataPi 帧数据指针
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 09/25/2010
* 说明				: 处理PLC卡数据帧
********************************************************************************/
void PlcDataFramDeal(struct CANLINK_DATA_BUF *dataPi)
{
	Uint16 addr, *InvRecTab, *InvTrTab;					    // 收发表指针
	Uint16 result, invTrIndex, indexReset = 1;              // "1"时复位索引
	Uint16 len = 0;
	
	addr = dataPi->mdl.data.datal;							// 数据地址索引
	
	if ( (addr > 0) && (addr <= INV_REC_FAST_NUM) )			// 写接收快表
	{
		InvRecTab = InvRecFastTab;                          // 接收快表地址
		
		InvTrTab = InvTrFastTab;							// 发送表格处理
		invTrIndex = InvTrFastIndex;
	}
	else if ( (addr > INV_SLOW_TAB_ADDR) && (addr <= (INV_SLOW_TAB_ADDR + INV_REC_SLOW_NUM) ) )
	{														// 慢表处理
        addr -= INV_SLOW_TAB_ADDR;                          // 除掉基地址
        InvRecTab = InvRecSlowTab;                          // 接收慢表
		
		InvTrTab = InvTrSlowTab;
		invTrIndex = InvTrSlowIndex;
	}
	else
	{
        if (funcCode.code.u3[11] < 65535)
            funcCode.code.u3[11]++;                         // 数据出错计数

        return;												// 地址出错直接返回
	}
// 接收表格处理	
	if (InvRecTab[addr])									// 写表操作
	{														// 所有表格操作都是写RAM
		CanControlWriter(InvRecTab[addr], dataPi->mdl.data.datah, 0);
		if (InvRecTab[addr+1])
		{
			CanControlWriter(InvRecTab[addr+1], dataPi->mdh.data.datal, 0);		
			if (InvRecTab[addr+2])	
				CanControlWriter(InvRecTab[addr+2], dataPi->mdh.data.datah, 0);			
		}
	}
	
// 发送表格返回
	if (InvTrTab[invTrIndex])
	{
        if (InvTrTab == InvTrFastTab)
            dataPi->mdl.data.datal = invTrIndex;				// 发送数据表地址索引
        else
            dataPi->mdl.data.datal = invTrIndex + INV_SLOW_TAB_ADDR;
        
        CanControlRead(InvTrTab[invTrIndex], &result);
		dataPi->mdl.data.datah = result;
		len += 4;
		if (InvTrTab[invTrIndex+1])
		{
			CanControlRead(InvTrTab[invTrIndex+1], &result);
			dataPi->mdh.data.datal = result;
			len += 2;
			if (InvTrTab[invTrIndex+2])
			{
				CanControlRead(InvTrTab[invTrIndex+2], &result);
				dataPi->mdh.data.datah = result;
				len += 2;
				if (InvTrTab[invTrIndex+3])					// 下一个单元等于0，下次操作从头开始
					indexReset = 0;
			}
		}
	}
	dataPi->msgid.bit.destSta = PLC_CARD_ID;				// 写目标地址
	dataPi->msgid.bit.srcSta = INV_PLC_ID;					// 写源地址
	CanlinkDataTran((Uint32*)(dataPi), len, 1000);			// 发送响应帧
	
// 发送快、慢表位置处理
	if (InvTrTab == InvTrFastTab)							// 快表
	{
		if ( (InvTrFastIndex > (INV_TRAN_FAST_NUM - 3) ) 
			|| (0 != indexReset) )							// 不等于0 复位索引寄存器
			InvTrFastIndex = 1;
		else
			InvTrFastIndex += 3;		
	}
	else													// 慢表
	{
		if ( (InvTrSlowIndex > (INV_TRAN_SLOW_NUM - 3) ) 
			|| (0 != indexReset) )
			InvTrSlowIndex = 1;
		else
			InvTrSlowIndex += 3;	
	}
}


#else

Uint16 InvTranTabCfg(struct CANLINK_DATA_BUF *dataPi)
{
    dataPi = dataPi;
    return 0;
}
Uint16 InvRecTabCfg(struct CANLINK_DATA_BUF *dataPi)
{
    dataPi = dataPi;
    return 0;
}
void PlcDataFramDeal(struct CANLINK_DATA_BUF *dataPi)
{
    dataPi = dataPi;
}

#endif





