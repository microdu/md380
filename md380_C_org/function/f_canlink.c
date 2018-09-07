/*************** (C) COPYRIGHT 2012   Co., Ltd****************
* File Name          : f_canlink.c
* Author             : 
* Version            : V0
* Date               : 08/25/2012
* Description        : CAN_LINK驱动库

********************************************************************************/
//#include "DSP28x_Project.h"     							// DSP2803x Headerfile Include File	
//#include "main.h"											// 包含头文件

#include "f_funcCode.h"
#include "f_dspcan.h"
#include "f_canlink.h"
#include "f_comm.h"
#include "f_plc.h"
#include "f_p2p.h"



#define DEBUG_F_CANLINK              1



#if DEBUG_F_CANLINK

Uint16 CanRxTxCon = 0;
// 定义CAN_LINK接收缓存
struct CANLINK_REC_BUF	CanlinkRecBuf;						// 接收缓存 4缓存


// 内部使用函数声明
#if (0 == CAN_REMOTE_EN)
void CanLinkRemoteDeal(struct CANLINK_DATA_BUF *dataPi);    // CAN使用硬件远程帧
#endif
void CanLlinkDataDeal(struct CANLINK_DATA_BUF *dataPi);		// CAN_LINK接收数据处理
void CanLinkConDeal(struct CANLINK_DATA_BUF *dataPi);		// 命令帧处理

void CanHostTx(Uint16 addr, Uint16 data);

Uint32	CanLinkChara[2] = {									// 变频器产品标识
							0x00000000,
							0x00002774,                     // 汇川产品，变频器
							};
							
static Uint16 sBaud = 0, sSourID = 0;						// 波特率与源地址							

/*******************************************************************************
* 函数名称          : void InitCanlinkTran(Uint16 num)
* 入口参数			: 	本机ID
						自动应答数据
* 出口				：
* 创建	            : 	
* 版本		        : 
* 时间              : 
* 说明				: 初始化CAN_LINK发送
********************************************************************************/
void InitCanlinkTran(Uint16	addr, Uint32 *dataPi)
{
	union CANLINK_MsgID msgId;
	Uint16	i;
    
	addr &= CANLINK_ID_MASK;
	// 			源站     命令		问答	   自动应答		屏蔽使能   扩展位
	msgId.all = 0xff | (0xfful<<16) | (1ul<<24) | (1ul<<29) | (0ul<<30) | (1ul<<31);

//    msgId.bit.srcSta = 0xff;								// 源地址
    msgId.bit.destSta = addr;								// 本机地址
//    msgId.bit.code = 0xff;									// 命令代码
//    msgId.bit.aq = 1;										// 问答标志
    msgId.bit.framf = CAN_REMOTE_FRAME;						// 帧类型标志，远程帧
//    msgId.bit.aam = 1;										// 自动应答标志
//    msgId.bit.ame = 0;										// 接收屏蔽使能，发送邮箱无效
//    msgId.bit.ide = 1;										// 扩展帧标志

#if CAN_REMOTE_EN	
	InitTranMbox(AAM_MBOX_N, msgId.all, dataPi);			// 初始化自动应答邮箱
#endif	
    msgId.bit.aam = 0;										// 自动应答标志，普通发送邮箱不自动应答

	for (i=0; i<TRAN_MBOX_NUM; i++)
	{
		InitTranMbox(TRAN_BOX_N-i, msgId.all, dataPi);		// 初始化发送邮箱
	}
}

/*******************************************************************************
* 函数名称          : Uint16 CanlinkDataTran(Uint32 *dataPi, Uint16 len, Uint16 timeOut)
* 入口参数			: 	消息ID
						发送数据
* 出口				：	CANLINK_RT_SUCC		数据发送成功
*						CANLINK_RT_BUSY		邮箱忙
*						CANLINK_RT_TIMEOUT	发送超时
* 创建	            : 	
* 版本		        : 
* 时间              : 
* 说明				: CAN_LINK发送
********************************************************************************/
Uint16 CanlinkDataTran(Uint32 *dataPi, Uint16 len, Uint16 timeOut)
{
	Uint16 stat, i;
	Uint32 msgid;
	static Uint16 count = 0;
	
	msgid = *dataPi++;
	for (i=0; i<TRAN_MBOX_NUM; i++)
	{
		stat = eCanDataTran(TRAN_BOX_N-i, len, msgid, dataPi);
		if (CAN_MBOX_TRAN_SUCC == stat)
		{
			count = 0;
			return (CANLINK_RT_SUCC);						// 发送成功		
		}
	}
	if (++count >= timeOut)
	{
		count = 0;
		return (CANLINK_RT_TIMEOUT);						// 发送超时
	}
	else
	{
		return (CANLINK_RT_BUSY);							// 发送邮箱忙
	}
}

/*******************************************************************************
* 函数名称          : void InitCanlinkRec(Uint16 addr)
* 入口参数			: 本机ID
* 出口				：
* 创建	            : 	
* 版本		        : 
* 时间              : 
* 说明				: 初始化CAN_LINK发送
********************************************************************************/
void InitCanlinkRec(Uint16 addr)
{
	union CANLINK_MsgID msgId;
	Uint32 lam;
	Uint16	i;
    
	addr &= CANLINK_ID_MASK;
	// 			  屏蔽使能   扩展位		问帧		
	msgId.all =  (1ul<<30) | (1ul<<31);// | (1ul<<24);

//    msgId.bit.srcSta = 0xff;								// 源地址
    msgId.bit.destSta = addr;								// 接收目标地址， 本站地址
//    msgId.bit.code = 0xff;									// 命令代码
//    msgId.bit.aq = 1;										// 问答标志
//    msgId.bit.framf = CAN_REMOTE_FRAME;						// 帧类型标志，远程帧
//    msgId.bit.aam = 1;										// 自动应答标志
//    msgId.bit.ame = 0;										// 接收屏蔽使能，发送邮箱无效
//    msgId.bit.ide = 1;										// 扩展帧标志
	lam = (~(0xfful<<8)) & ( ~(7ul<<29));// & (~(1ul<<24));		// 只能接收扩展帧 "0"滤波匹配
// 			目标地址	  只接收扩展帧		问帧			// 接收数据不分问、答帧，满足PLC卡数据帧接收
	for (i=0; i<REC_MBOX_NUM-1; i++)
	{
		InitRecMbox((REC_BOX_N-i) | 0x40, msgId.all, lam);	// 初始化接收邮箱
	}
	InitRecMbox((REC_BOX_N-i), msgId.all, lam);				// 最后接收邮箱允许覆盖	
	
//	CanlinkRecBuf.num = 0;                                  // 接收数据计数值为零
}

/*******************************************************************************
* 函数名称          : Uint16 CanlinkDataRec(Uint32 msgid, Uint32 *dataPi, Uint16 timeOut)
* 入口参数			: 	消息ID
						发送数据
* 出口				：CANLINK_R_EMPTY	数据邮箱空
*					  CANLINK_R_OVER    接收成功，数据缓存有溢出
*					  CANLINK_RT_SUCC	接收成功
* 创建	            : 	
* 版本		        : 
* 时间              : 
* 说明				: 初始化CAN_LINK发送
********************************************************************************/
Uint16 CanlinkDataRec(struct CANLINK_REC_BUF *dataPi)
{
	Uint16 i, stat;

	dataPi->bufFull = 0;
	for (i=0; i<REC_MBOX_NUM; i++)
	{
		stat = eCanDataRec(REC_BOX_N-i, (Uint32 *)(&(dataPi->buf[i])) );
		if (CAN_MBOX_EMPTY != stat)//(CAN_MBOX_REC_SUCC == stat) || (CAN_MBOX_REC_OVER == stat) )	// 接收到数据
			dataPi->bufFull |= 1<<i;						// 接收缓存有效
	}
	if ( 0 == dataPi->bufFull)								// 未收数据 
		return (CANLINK_R_EMPTY);							// 接收邮箱空，返回
	if (CAN_MBOX_REC_OVER == stat)
		return (CANLINK_R_OVER);
	else
		return (CANLINK_RT_SUCC);
}


/*******************************************************************************
* 函数名称          : Uint16 InitCanlink(Uint16 addr, Uint16 baud, Uint32 *dataPi)
* 入口参数			: 	本机ID
*						波特率选择 
*						自动应答数据缓存
* 出口				：CAN_INIT_TIME	 初始化进行中
*					  CAN_INIT_SUCC  初始化成功
*					  CAN_INIT_TIMEOUT 初始化超时
*					  CAN_INIT_BAUD_ERR 波特率出错
* 创建	            : 	
* 版本		        : 
* 时间              : 
* 说明				: 初始化CAN_LINK驱动模块
********************************************************************************/
Uint16 InitCanlink(Uint16 addr, Uint16 baud, Uint32 *dataPi)
{
	Uint16 stat;
	addr &= CANLINK_ID_MASK;								// 过滤参数
//	baud &= 0x7;
	
	stat = InitdspECan(baud);								// eCAN模块初始化
	if (stat != CAN_INIT_SUCC)
		return (stat);
		
	InitCanlinkTran(addr, dataPi);							// 初始化CAN_LINK发送，
	InitCanlinkRec(addr);									// 接收	
	return (CAN_INIT_SUCC);
}


/*******************************************************************************
* 函数名称          : void CanlinkFun(void)
* 入口参数			: 
* 出口				：
* 创建	            : 	
* 版本		        : 
* 时间              : 
* 说明				: CAN_LINK功能模块，使用2ms间隔调用操作
*					  需要功能码   站ID  1~63
*					  波特率设置	     1~2，设置0关闭CAN功能
********************************************************************************/
#if (1 == CAN_LINK_TIME_DEBUG)
Uint32	CanTime1, CanTime2, CanTime;
#endif
void CanlinkFun(void)
{
	Uint16 stat;
	static Uint16 timeOutErr = 0, initFlag = 0;                             // 初始化完成标志
	

#if (1 == CAN_LINK_TIME_DEBUG)
	CanTime1 = ECANREGS.CANTSC;
#endif

#if DEBUG_F_P2P_CTRL
    // 点对点通讯有效
    if (p2pData.p2pEnable)
    {
        // 1:主机    2:从机
        CanRxTxCon = p2pData.p2pEnable + funcCode.code.p2pTypeSel;
    }
    else
    {
        CanRxTxCon = 0;
    }

#endif
/*                                                          // CANlink协议永远执行，不关闭
    if ((funcCode.code.commProtocolSec != PROFIBUS)         // PROFIBUS卡带CAN接口
        && (funcCode.code.commProtocolSec != CANLINK)       // CAN-LINK卡带CAN接口
        && (funcCode.code.plcEnable != 1))                  // PLC卡带CAN接口
    {
        initFlag = 0;                                       // 不执行CAN模块
        return;
    }
*/                                                           // 修改参数重新初始化
    if (funcCode.code.plcEnable)                            // 使用PLC卡
    {
        if ( (sBaud != CAN_BAUD_1M) || (sSourID != INV_PLC_ID) )
		{
			sBaud = CAN_BAUD_1M;
			sSourID = INV_PLC_ID;		
			initFlag = 0;			
		}
    }
    // 点对点通讯
    else if (funcCode.code.p2pEnable)
    {
        Uint16 p2pID;
        Uint16 p2pBaud;
        
        p2pBaud = CAN_LINK_BAUD_SEL;   // 点对点通讯波特率
        
        // 从机 地址1
        if (funcCode.code.p2pTypeSel)        {
            p2pID = COMM_P2P_SLAVE_ADDRESS;   // 从机地址
        }
        // 主机 地址2
        else
        {
            p2pID = COMM_P2P_MASTER_ADDRESS;
        }

        if ( (sBaud != p2pBaud) || (sSourID != p2pID) )
        {
            sBaud = p2pBaud;
            sSourID = p2pID;
            initFlag = 0;
        }
    }
    else if ( (sBaud != CAN_LINK_BAUD_SEL) || (sSourID != CAN_LINK_S_ADDR) )
    {														// 使用CAN_LINK协议	
        sBaud = CAN_LINK_BAUD_SEL;
        sSourID = CAN_LINK_S_ADDR;
        initFlag = 0;
    }

	if (initFlag == 0)										// 初始化CAN_LINK
	{
        funcCode.code.u3[10] = 0;
        funcCode.code.u3[11] = 0;
        if (CAN_INIT_SUCC != InitCanlink(sSourID, sBaud, CanLinkChara) )
			return ;
		else
			initFlag = 0xcc;                                // 初始化成功
			
		return;		
	}

#if DEBUG_F_P2P_CTRL
    // 为点对点通讯的主机
    if (CanRxTxCon == P2P_COMM_HOST)                        // 只发送固定数据
    {
        // 点对点通讯发送周期
        p2pData.p2pSendTcnt++;
        if (p2pData.p2pSendTcnt >= p2pData.p2pSendPeriod)
        {
            // para(从机接收数据地址,主机发送数据)
            CanHostTx(COMM_P2P_COMM_ADDRESS_DATA, p2pData.P2PSendData);   // 发送数据
            p2pData.p2pSendTcnt = 0;
        }        
        return;
    }
#endif

	stat = CanlinkDataRec(&CanlinkRecBuf);					// 接收CAN数据

	if ((CANLINK_RT_SUCC == stat) || (CANLINK_R_OVER == stat) )	
	{														// 如接收数据成功
		for (stat=0; stat<REC_MBOX_NUM; stat++)				// 处理接收数据
		{
			if ( (CanlinkRecBuf.bufFull & (1u<<stat) )== (1<<stat))
			{
//				CanlinkRecBuf.buf[stat].msgid.all -= 1;
															// 调用CAN_LINK数据处理功能
				CanLlinkDataDeal((struct CANLINK_DATA_BUF *)(&CanlinkRecBuf.buf[stat]) );
			}	
		}
        timeOutErr = 0;
        funcCode.code.u3[10] = 0;
	}
    else                                                    // 未接收到数据
    {
        if (++timeOutErr > 4)                               // 0~4计数，10ms
        {
            if (funcCode.code.u3[10] < 65535)
                funcCode.code.u3[10]++;                     // 超时计数

            // 可编程功能有效时报错
            if ((funcCode.code.plcEnable)
                && ((curTime.powerOnTimeSec > 10) // 上电时间超过10秒才判断
                || (curTime.powerOnTimeM > 0))  
                )
            {
                errorOther = ERROR_COMM;
                errorInfo = COMM_ERROR_PLC;
            }
                
            timeOutErr = 0;
        }
    }
#if (1 == CAN_LINK_TIME_DEBUG)	
	CanTime2 = ECANREGS.CANTSC;
	CanTime = CanTime2 - CanTime1;
#endif    
}

/*******************************************************************************
* 函数名称          : void CanLlinkDataDeal(struct CANLINK_DATA_BUF *dataPi)
* 入口参数			: 
* 出口				：
* 创建	            : 	
* 版本		        : 
* 时间              : 
* 说明				: CAN_LINK协议接收数据处理
********************************************************************************/
void CanLlinkDataDeal(struct CANLINK_DATA_BUF *dataPi)
{
	switch (dataPi->msgid.bit.framf)						// 帧类型处理
	{
#if (0 == CAN_REMOTE_EN)
        case CAN_REMOTE_FRAME:								// CAN_LINK远程帧软件处理
			CanLinkRemoteDeal(dataPi);
			break;
#endif
        case CAN_CONTROL_FRAME:								// 命令帧处理
			CanLinkConDeal(dataPi);
			break;
		case CAN_CONFIG_FRAME:								// 配置帧处理

			break;
		case CAN_DATA_FRAME:								// 数据帧处理
			PlcDataFramDeal(dataPi);
		
			break;
			
		default:
            if (funcCode.code.u3[11] < 65535)
                funcCode.code.u3[11]++;                     // 数据出错计数
			break;				
			
	}		
}


/*******************************************************************************
* 函数名称          : void CanLinkRemoteDeal(struct CANLINK_DATA_BUF *dataPi)
* 入口参数			: 数据缓存指针
* 出口				：
* 创建	            : 	
* 版本		        : 
* 时间              : 
* 说明				: CAN_LINK远程帧软件处理
********************************************************************************/
#if (0 == CAN_REMOTE_EN)
void CanLinkRemoteDeal(struct CANLINK_DATA_BUF *dataPi)
{
	if (dataPi->msgid.bit.aq == CANlink_ACK)                // 收到应答帧不处理
        return;
//	dataPi->msgid.bit.framf = CAN_REMOTE_FRAME;			    //
    dataPi->msgid.bit.destSta = sSourID;					// 本机地址
//	dataPi->msgid.bit.srcSta = 0xff	;						// 
    dataPi->msgid.bit.aq = CANlink_ACK;						// 远程帧使用该位响应
//	dataPi->msgid.bit.code = 0xff;							//
	dataPi->mdl.all= CanLinkChara[0];
	dataPi->mdh.all= CanLinkChara[1];
    
    funcCode.code.u3[10] = 0;
    funcCode.code.u3[11] = 0;
    
	CanlinkDataTran((Uint32*)(dataPi), 8, 1000);			// 发送远程帧响应

    

}
#endif

//===================================================================
// 函数名称: 后台写参数
// 参数    ：addr  地址
//			 dat   写数据
//           eeprom 写EEPROM 使能
// return  ：执行状态(为0执行成功)
// 创建    : Yanyi	
//===================================================================
Uint16 CanControlWriter(Uint16 addr, Uint16 data, Uint16 eeprom)
{
    Uint16 oscReturn;
	// 数据写操作
    if (eeprom)
        commRcvData.commCmdSaveEeprom = SCI_WRITE_WITH_EEPROM;
    sciFlag.all = 0;                                        // 清零通讯状态标志
    oscReturn = CommWrite(addr, data);
    commRcvData.commCmdSaveEeprom = SCI_WRITE_NO_EEPROM;    // 返回仅保存RAM

    return oscReturn;
}


//===================================================================
// 函数名称: Uint16 CanControlRead(Uint16 addr, Uint16* result)
// 入口参数：addr   地址   
//			 dat	读取数据指针
// return	 ：执行状态(为0执行成功)
// 创建	   : 	
// 说明	   ：CAN_LINK读参数
//===================================================================
Uint16 CanControlRead(Uint16 addr, Uint16* result)
{
    Uint16 oscReturn;
    // 数据读操作
    sciFlag.all = 0;                                        // 清零通讯状态标志
    oscReturn = CommRead(addr, 1);
    *result = commReadData[0];
    return oscReturn;
}

/*******************************************************************************
* 函数名称          : void CanLlinkDataDeal(struct CANLINK_DATA_BUF *dataPi)
* 入口参数			: 数据缓存指针
* 出口				：
* 创建	            : 	
* 版本		        : 1
* 时间              : 
* 说明				: CAN_LINK协议命令帧处理
********************************************************************************/
void CanLinkConDeal(struct CANLINK_DATA_BUF *dataPi)
{
	Uint16 err, len = 4;
//	struct CANLINK_DATA_BUF	txBuf;							// 发送缓存	1缓存	

	if (dataPi->msgid.bit.aq == CANlink_ACK)                // 收到应答帧不处理
        return;
    
	switch (dataPi->msgid.bit.code)							// 命令编码
	{
		case CAN_LINK_R_CFG:								// 读配置

			break;
		case CAN_LINK_R_REG:								// 读寄存器
			err = CanControlRead(dataPi->mdl.data.datah, (Uint16*) (&dataPi->mdl.data.datal) );
//			len = 4;
			break;
		case CAN_LINK_W_REG:								// 写寄存器
			err = CanControlWriter(dataPi->mdl.data.datah, dataPi->mdl.data.datal, 0);
//			len = 4;
			break;
		case CAN_LINK_R_INFO:								// 读站点设备信息

			break;
		case CAN_LINK_R_WAR:								// 读告警信息

			break;
		case CAN_LINK_W_EEP:								// 写EEPROM
			err = CanControlWriter(dataPi->mdl.data.datah, dataPi->mdl.data.datal, 1);
//			len = 4;
			break;
// PLC卡CAN相关命令

		case CAN_TRAN_TAB_CFG:								// 发送表配置
			err = InvTranTabCfg(dataPi);
//			len = 4;
			break;
		case CAN_REC_TAB_CFG:								// 接收表配置
			err = InvRecTabCfg(dataPi);
//			len = 4;
			break;
		
		default:
			err = COMM_ERR_CMD;								// 读写命令无效
			break;				
			
	}

#if DEBUG_F_P2P_CTRL
    // 为点对点通讯的从机
    if (CanRxTxCon == P2P_COMM_SLAVE)                        // == 2只接收不返回
	{
        // 点对点通讯异常时间清0
        p2pData.p2pCommErrTcnt = 0;
        return;
	}
#endif
    
	dataPi->msgid.bit.destSta = dataPi->msgid.bit.srcSta;	// 写目标地址
	dataPi->msgid.bit.srcSta = sSourID;						// 写源地址
	dataPi->msgid.bit.aq = CANlink_ACK;						// 答标志


    if (err)
    {
        dataPi->msgid.bit.code = 0xff;                      // 操作出错，命令码返回0xFF
        dataPi->mdl.data.datah = 0x8001;
	    dataPi->mdl.data.datal = err;
    }
    else if ( (sciFlag.bit.pwdPass)                         // 密码通过：返回 0x8888
//    && (0x1f00 == dataPi->mdl.data.datah) 
    )
    {
        dataPi->mdl.data.datal = 0x8888;
	}
/*    else if (sciFlag.bit.write)                             // 写数据操作处理。若有错误，则报错，不会真正写
    {
	}
    else if (sciFlag.bit.read)                              // 通讯参数读取处理，真正需要读取
    {

    }

	if (err == COMM_ERR_CMD)								// 命令无效
	{
		dataPi->msgid.bit.code = 0xff;						// 返回出错命令
		len = 0;
	}else if (err == COMM_ERR_ADDR)							// 地址无效
	{
		dataPi->mdl.data.datah += 1;
	}else if (err == COMM_ERR_PARA)							// 数据无效
	{
		dataPi->mdl.data.datal += 1;	
	}
*/
    if (err)
    {
        if (funcCode.code.u3[11] < 65535)
            funcCode.code.u3[11]++;                         // 数据出错计数
    }
	
	CanlinkDataTran((Uint32*)(dataPi), len, 1000);			// 发送响应帧
}



/*******************************************************************************
* 函数名称          : void CanHostTx(void)
* 入口参数			: addr CAN_LINK目标寄存器地址
*                     data 写目标寄存器数据
* 出口				：无
* 创建	            : 	
* 版本		        : 
* 时间              : 
* 说明				: 使用CAN_LINK协议发送指定数据
********************************************************************************/
void CanHostTx(Uint16 addr, Uint16 data)
{
    struct CANLINK_DATA_BUF databuf;
    Uint16 sendDataStatus;
    
    databuf.msgid.bit.destSta = COMM_P2P_SLAVE_ADDRESS;	    // 从机地址固定为2
	databuf.msgid.bit.srcSta = sSourID;						// 写源地址
	databuf.msgid.bit.code = CAN_LINK_W_REG;                // 写寄存器命令
	databuf.msgid.bit.framf = CAN_CONTROL_FRAME;            // 命令帧
	databuf.msgid.bit.aq = 1;								// 问帧
    databuf.mdl.data.datah = addr;                          // 指定地址
    databuf.mdl.data.datal = data;                          // 数据
	
	sendDataStatus = CanlinkDataTran((Uint32*)(&databuf), 4, 1000);			// 发送响应帧

    // 主机发送异常
    if (sendDataStatus)
    {
        // 主机未发送成功
        p2pData.p2pCommErrTcnt++;
    }
}
#elif 1

void CanlinkFun(void){}

#endif











