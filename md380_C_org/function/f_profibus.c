//======================================================================
//
// Time-stamp: <2012-02-23  Zhd, 0656>
//
// PROFIBUS 协议
//======================================================================


#include "f_comm.h"



#define DEBUG_F_DP1     1



#if DEBUG_F_DP1


// DP卡-变频器交互数据帧头
#define PROFIBUS_START_HIGH    0xAA      // DP卡与变频器正常交互数据帧头高位值 
#define PROFIBUS_START_LOW     0x55      // DP卡与变频器正常交互数据帧头低位值
#define PROFIBUS_CHECK         0x5A      // DP卡与变频器连通检测数据帧头值

// 帧头
#define DPHighStart       rcvFrame[0]    // DP卡接收数据帧头高位
#define DPlowStart        rcvFrame[1]    // DP卡接收数据帧头低位 
// PKE IND
#define DPFuncCmd         rcvFrame[2]>>4 // DP卡功能码操作命令字
#define DPhighFuncAddr    rcvFrame[3]    // DP卡操作功能码参数地址高位
#define DPlowFuncAddr     rcvFrame[4]    // DP卡操作功能码参数地址低位
// PKW
#define PZDHighCmd        rcvFrame[6]    // DP卡更改PZD11~PZD12对应功能码参数值命令 0-无操作  1-写PZDx值到对应功能码(bit0~bit1对应PZD11~PZD12) 
#define PZDlowCmd         rcvFrame[7]    // DP卡更改PZD3 ~PZD10对应功能码参数值命令 0-无操作  1-写PZDx值到对应功能码(bit0~bit7对应PZD3~PZD10) 
#define DPhighFuncData    rcvFrame[8]    // DP卡更改功能码参数值数据高位(DPFuncCmd值为2或14：更改功能码) 
#define DPlowFuncData     rcvFrame[9]    // DP卡更改功能码参数值数据低位

// DP卡功能码操作命令
#define DP_FUNC_READ          0x1        // 读功能码
#define DP_FUNC_WRITE_RAM     0x2        // 写功能码值
#define DP_FUNC_WRITE_EEPROM  0xE        // 写功能码值且保存至EEPROM

// PROFIBUS数据格式
#define PPO1                     0
#define PPO2                     1
#define PPO3                     2
#define PPO5                     3
#define DP_RCV_CRC_ERR_NUMBER   10             // CRC校验出错次数，到达该次数后置为接收

struct DP_CONTROL_CMD_BITS
{
    Uint16 stop:1;         // bit0-减速停机
    Uint16 freeStop:1;     // bit1-自由停机
    Uint16 fwdRun:1;       // bit2-正转运行
    Uint16 revRun:1;       // bit3-反转运行
    Uint16 fwdJog:1;       // bit4-正转点动
    Uint16 revJog:1;       // bit5-反转点动
    Uint16 reset:1;        // bit6-故障复位
#if 0    
    Uint16 DpCmdEnable:1;  // bit7-DP命令有效 
#endif 
    Uint16 rsvd1:9;
};
union DP_CONTROL_CMD
{
    Uint16 all;
    struct DP_CONTROL_CMD_BITS bit;
};


struct DP_STATUS_BITS
{
    Uint16 run:1;           // bit0-运行/停机
    Uint16 fwdRev:1;        // bit1-正转/反转
    Uint16 error:1;         // bit2-变频器故障
    Uint16 frqArrive:1;     // bit3-频率到达
    Uint16 rsvd1:12;        // bit4-bit15
    
};


union DP_STATUS
{
    Uint16 all;
    struct DP_STATUS_BITS bit;
};


union DP_CONTROL_CMD dpControlCmd;             // DP控制命令字
union DP_CONTROL_CMD dpControlCmdBak;          // DP控制命令字备份
union DP_STATUS dpStatus;                      // DP变频器状态返回字               
//Uint16 dpStatus;
Uint16 dpFrqAim;                                // DP目标频率或转矩给定
const Uint16 DPDataNum[4] = {12, 20, 4, 32};   // PPO1、PPO2、PPO3、PPO5协议对应的数据个数
Uint16 rcvRigthFlag;                           // 帧头判断 为1表示遇到0xAA
Uint16 DPRcvTest;                              // DP连通测试标志(为1表示当前为连通测试中)

Uint16 saveEepromIndex = 0;
Uint16 saveEepromFlag = 0;
//====================================================================
//
// 数据接收后信息解析
//
//====================================================================
void ProfibusRcvDataDeal(void)
{
    commRcvData.commAddr = 0;
    commRcvData.commData = 0;
    commRcvData.commCmd = 0;
    saveEepromFlag = 0;
    // PPO3协议无读写功能操作
	// 连通测试无读写操作
    if ((commProtocol != PPO3) && (!DPRcvTest))
    {
		//  操作功能参数地址
		commRcvData.commAddr = (DPhighFuncAddr << 8) + DPlowFuncAddr;  
		//  更改功能参数值
        commRcvData.commData = (DPhighFuncData << 8) + DPlowFuncData;  
        // 读功能码
        if (DPFuncCmd == DP_FUNC_READ)
        {
            commRcvData.commCmd = SCI_CMD_READ;
			commRcvData.commData = 1;
        }
		// 写功能码
        else if(DPFuncCmd == DP_FUNC_WRITE_RAM)
        {
            commRcvData.commCmd = SCI_CMD_WRITE;                  // 写命令操作
            commRcvData.commCmdSaveEeprom = SCI_WRITE_NO_EEPROM;  // 仅保存RAM
        }
		// 写功能码且保存EEPROM
        else if(DPFuncCmd == DP_FUNC_WRITE_EEPROM)
        {
            // 写EEPROM思路(防止循环写坏EEPROM)
            // 1、记录需要写EEPROM标志
            // 2、写RAM
            // 3、写RAM成功，获得当前功能参数索引
            // 4、如果之前有存储索引，则先保存前一个索引EEPROM操作
            // 5、保存当前功能参数索引
            // 6、掉电时，如果存储的参数索引有值，则存EEPROM
            saveEepromFlag = 1;   // 需要存储EEPROM操作
            commRcvData.commCmd = SCI_CMD_WRITE;                  // 写命令操作
            commRcvData.commCmdSaveEeprom = SCI_WRITE_NO_EEPROM;  // 
        }

    }
#if 0    
    else
    {
		// 为PPO3或连通测试时 无操作功能参数命令、地址、数据
        
    }
#endif    

    commRcvData.slaveAddr = 1;    // 从机地址(DP卡到)  // --? 考虑
	// 接收到的CRC校验值
	commRcvData.crcRcv = (rcvFrame[commRcvData.rcvNumMax - 1] << 8) + rcvFrame[commRcvData.rcvNumMax - 2];  
	 // CRC校验长度
	commRcvData.crcSize = commRcvData.rcvNumMax - 2;                   
}


//====================================================================
//
// PROFIBUS帧头判断
// 参数: tmp-接收帧数据
// 返回: 0-帧头判断过程中
//       1-不需要帧头判断，直接存储数据
//
//===================================================================
Uint16 ProfibusStartDeal(Uint16 tmp)
{
    // 帧头判断标志有效
    if (!commRcvData.rcvDataJuageFlag)
    {
		// 接收到新的数据时清连通测试判断标志
        DPRcvTest = 0;
		// 接收数据长度
        commRcvData.rcvNumMax = DPDataNum[commProtocol] + 4;   // 接收数据个数
		// 为连通测试帧头
        if (tmp == PROFIBUS_CHECK)
        {
            rcvFrame[0] = tmp;
            commRcvData.rcvFlag = 0;         
            rcvRigthFlag = 2;         // 置2开始接收DP卡拨码地址       
        }
		// 接收DP卡拨码地址
        else if(rcvRigthFlag == 2)
        {
            rcvFrame[1] = tmp;
            commRcvData.rcvFlag = 0;         
            rcvRigthFlag = 3;         // 置3开始判断第三帧数据是否为0x55
        }
        else if(rcvRigthFlag == 3)
        {
			rcvRigthFlag = 0;
            // 连通测试的第一位为 0x5A 第三位为0x55
            if(tmp == PROFIBUS_START_LOW)
            {
				DPRcvTest = 1;
                rcvFrame[2] = tmp;
                commRcvData.rcvDataJuageFlag = 1;  // PROFIBUS清帧头判断标志
                commRcvData.rcvFlag = 2;           // 为2表示本机数据处理
                commRcvData.rcvNum = 3;            // 接收数据计数   
                commRcvData.rcvNumMax = 5;         // 连通测试数据帧仅有3位数据位2位校验位
            }
        }
        // 判断 0xAA 0x55 帧头信息
        else if(tmp == PROFIBUS_START_HIGH)
        {
            rcvRigthFlag = 1;      // 接收帧头第一数据正确
            rcvFrame[0] = tmp;   
            commRcvData.rcvFlag = 0;
        }
        else if(rcvRigthFlag == 1) // 接收帧头第一数据正确
        {   
            rcvRigthFlag = 0;      // 清接收帧头第一数据正确
            if(tmp == PROFIBUS_START_LOW)
            {
                rcvFrame[1] = tmp;
                commRcvData.rcvDataJuageFlag = 1;  // PROFIBUS清帧头判断标志
                commRcvData.rcvFlag = 2;           // 开始数据接收
                commRcvData.rcvNum = 2;            // 接收数据计数   
            }              
        }
        else
        {
            rcvRigthFlag = 0;
            commRcvData.rcvCrcErrCounter  = 0;
            commRcvData.rcvFlag = 0;
        }
        return 0;
    }
    return 1;
}


//====================================================================
//
// 获得DP数葜斜淦灯髅钚畔?
//
//===================================================================
void getDPControlCmd(void)
{
#if 0    
    if (!dpControlCmd.bit.DpCmdEnable)
    {
        commRunCmd = SCI_RUN_CMD_NONE;
    }
    else 
    
    commRunCmd = SCI_RUN_CMD_NONE;

    if (dpControlCmd.bit.stop)
    {
        commRunCmd = SCI_RUN_CMD_STOP;
    }
    else if(dpControlCmd.bit.freeStop)
    {
        commRunCmd = SCI_RUN_CMD_FREE_STOP;
    }
    else if(dpControlCmd.bit.fwdRun)
    {
        commRunCmd = SCI_RUN_CMD_FWD_RUN;
    }
    else if(dpControlCmd.bit.revRun)
    {
        commRunCmd = SCI_RUN_CMD_REV_RUN;
    }
    else if(dpControlCmd.bit.fwdJog)
    {
        commRunCmd = SCI_RUN_CMD_FWD_JOG;
    }
    else if(dpControlCmd.bit.revJog)
    {
        commRunCmd = SCI_RUN_CMD_REV_JOG;
    }
    else if(dpControlCmd.bit.reset)
    {
        if (!dpControlCmdBak.bit.reset)
        {
            commRunCmd = SCI_RUN_CMD_RESET_ERROR;
        }
    }
    

    dpControlCmdBak = dpControlCmd;

    #endif 
}


//====================================================================
//
// 反馈DP变频器状态信息
//
//===================================================================
void setDPStatusInfo(void)
{
    dpStatus.all = 0;
    
    // 停机/运行 状态
    dpStatus.bit.run = runFlag.bit.run;     
    // 正转/反转 状态
    dpStatus.bit.fwdRev = runFlag.bit.dir;
    
	// 目标频率到达 
    if ((runFlag.bit.run) 
        && (frq == frqAim))
    {

        dpStatus.bit.frqArrive = 1;
    }

    // 故障
    if (errorCode)
    {
        dpStatus.bit.error = 1;       
    }
   
   
}


//====================================================================
//
// 更新通讯参数
// 1、接收数据个数
// 2、新帧开始判断时间
// 3、应答延迟时间
//
//====================================================================
void UpdateProfibusCommFormat(Uint16 baudRate)
{
	// 为连通测试时发送数据长度为5
    if (DPRcvTest)
    {
        commRcvData.rcvNumMax = 5;
    }
    else
    {
        commRcvData.rcvNumMax = DPDataNum[commProtocol] + 4;   // 接收数据个数
    }
	// DP卡没有用时间延迟判断帧头
    commRcvData.frameSpaceTime = 0;
	// DP卡通讯无应答延迟时间
    commRcvData.delay = 0;
}


//====================================================================
//
// 准备发送数据
// 参数: err-通讯命令处理出错信息,为0表示操作成功
// 返回: 1、发送数据长度
//       2、发送数据信息
//
//====================================================================
void ProfibusSendDataDeal(Uint16 err)
{
    Uint16 PZD2ReturnData;
    Uint16 readDataStartIndex;
	Uint16 i, pzdNum, indexRead,indexWrite, group, grade;
    Uint16 pzdDataSaveCmd, pzdSaveData;
    commSendData.sendNumMax = commRcvData.rcvNumMax;

    // 连通测试
    if (DPRcvTest)
    {
        sendFrame[0] = PROFIBUS_CHECK;      // 连通测试帧头   

        // DP卡本机地址,硬件拨码为0时以该值为准
        sendFrame[1] = funcCode.code.commSlaveAddress;
        
        // PROFIBUS-DP数据格式
        sendFrame[2] = commProtocol + 1;    // PPO数据格式
        if (sendFrame[2] > 3)
        {
            sendFrame[2] = 5;   // PPO5
        }
        return;
    }

	// 发送数据PKW
    sendFrame[0] = PROFIBUS_START_HIGH;    // 发送帧头高位 0xAA
    sendFrame[1] = PROFIBUS_START_LOW;     // 发送帧头低位 0x55  
    sendFrame[2] = 0x10;                   // 高4位为1表示功能码操作成功
    sendFrame[3] = DPhighFuncAddr;         // 操作功能码地址高位
	sendFrame[4] = DPlowFuncAddr;          // 操作功能码地址低位
    sendFrame[5] = 0x00;                   // 保留
    sendFrame[6] = 0x00;                   // 保留(可作为返回PZD参数读取/修改状态高位)
    sendFrame[7] = 0x00;                   // 保留(可作为返回PZD参数读取/修改状态低位)
    sendFrame[8] = DPhighFuncData;         // 操作功能码值高位(读取功能码时更新为读取的值)
    sendFrame[9] = DPlowFuncData;          // 操作功能码值低位(读取功能码时更新为读取的值)

	// 处理功能码操作命令存在故障
	if (!(DPFuncCmd))
	{
        sendFrame[2] = 0x0;                // 高4位为1表示功能码操作成功
        sendFrame[3] = 0x0;                // 操作功能码地址高位
	    sendFrame[4] = 0x0;                // 操作功能码地址低位
	}
    else if (err)
    {
        sendFrame[2] = 0x70; // 高4位返回为7表示操作失败    
        sendFrame[8] = 0x00; // 故障代码信息高位
        sendFrame[9] = err;  // 故障代码信息低位
    }
	// 读功能码操作成功
    else if (sciFlag.bit.read)              // 通讯参数读取处理，真正需要读取
    {
        // 返回读取值
        sendFrame[8] = commReadData[0] >> 8;
        sendFrame[9] = commReadData[0] & 0x00ff;
    }
    else if (sciFlag.bit.write)
    {
        // 掉电存储EEPROM
        if (saveEepromFlag)
        {
            Uint16 highH,group,grade,index;
            // 获得功能参数首地址
            highH = (commRcvData.commAddr & 0xF000);

            // 获取group, grade
            group = (DPhighFuncAddr >> 8) & 0x0F;
            grade = DPlowFuncAddr & 0xFF;

            if (0xA000 == highH)       // Ax
            {
                group += FUNCCODE_GROUP_A0;
            }
            else if(0xB000 == highH)
            {
                group += FUNCCODE_GROUP_B0;
            }

            index = GetGradeIndex(group, grade);
            // 如果之前存在需要保存的数据
            if ((saveEepromIndex) 
                && (index != saveEepromIndex)
                )
            {
                // 存储EEPROM
                SaveOneFuncCode(saveEepromIndex);
            }
            saveEepromIndex = index;

            
        }
    }
    
    // 非PPO3数据格式 PZD参数起始地址为10
    // PPO3 数据格式  PZD参数起始地址为2
    if (commProtocol != PPO3)
    {
        readDataStartIndex = 10;
		pzdNum = (commSendData.sendNumMax - 16) >> 1;    // 操作的PZD快捷参数长度
    }
	else
	{
		readDataStartIndex = 2;
		pzdNum = 0;    // PPO3操作的PZD快捷参数长度为0               
	}

	// PZD1、PZD2接收
    if (!sciFlag.bit.crcChkErr)   // CRC校验成功
    {
        Uint16 frq;
        // 控制字
        commRunCmd = rcvFrame[readDataStartIndex + 1];  
        // 目标频率
        frq = (rcvFrame[readDataStartIndex + 2] << 8) + rcvFrame[readDataStartIndex + 3];
        // 最大频率限制
        if (frq <= maxFrq)
        {
            dpFrqAim = frq;
        }
    }
  
	// PZD1、PZD2发送
    setDPStatusInfo();     // 状态字
	if (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_FVC)
	{
        PZD2ReturnData = frqFdb;       // 闭环矢量时反馈实际转速
	}
    else
    {
        PZD2ReturnData = frqRunDisp;   // 开环矢量时反馈同步转速
    }
	
	// PZD1返回数据(变频器状态信息)
    sendFrame[readDataStartIndex++]     = (dpStatus.all >> 8);
    sendFrame[readDataStartIndex++] = (dpStatus.all & 0x00ff);
	// PZD2返回数据(运行转矩或频率)
	sendFrame[readDataStartIndex++] = (PZD2ReturnData >> 8);
    sendFrame[readDataStartIndex++] = (PZD2ReturnData & 0x00ff);


	// PZD快捷数据保存命令标志
	pzdDataSaveCmd = (PZDHighCmd << 8) + PZDlowCmd;

	// 周期性数据输入输出(PZD3~PZD12)
	for (i = 0; i < pzdNum; i++)
	{
        // PZD输出地址(主站->变频器)
		group = funcCode.group.fe[i] / 100;
        grade = funcCode.group.fe[i] % 100;
		indexWrite = GetGradeIndex(group, grade);
        // PZD输入地址(变频器->主站)
	    group = funcCode.group.fe[i + 10] / 100;
        grade = funcCode.group.fe[i + 10] % 100;
		indexRead = GetGradeIndex(group, grade);
        
        // PZD输出
        if (!sciFlag.bit.crcChkErr)
        {
            pzdSaveData = (rcvFrame[readDataStartIndex] << 8) + rcvFrame[readDataStartIndex + 1];
            if (ModifyFunccodeUpDown(indexWrite, &pzdSaveData, 0) == COMM_ERR_NONE)
            {
                ModifyFunccodeEnter(indexWrite, pzdSaveData);
            }
        }
        
		// PZD输入
		sendFrame[readDataStartIndex++] = (funcCode.all[indexRead] >> 8);
    	sendFrame[readDataStartIndex++] = (funcCode.all[indexRead] & 0x00ff);  
	}          

}


//====================================================================
//
// 通讯出错判断(判断通讯出错后置SCI为数据接收状态)
// 返回: 0、通讯正常
//       1、通讯出错
//
//====================================================================
Uint16 ProfibusCommErrCheck(void)
{
#if 1

    if ((funcCode.code.commOverTime) && (commTicker >= (Uint32)funcCode.code.commOverTime * 2 * TIME_UNIT_sciCommOverTime))
    {
        // MODBUS通讯超时检测计时
        commTicker = (Uint32)funcCode.code.commOverTime * 2 * TIME_UNIT_sciCommOverTime;  
        return 1;
    }

    if (commRcvData.rcvCrcErrCounter > DP_RCV_CRC_ERR_NUMBER)
    {
       // PROFIBUS CRC校验出错计数
       commRcvData.rcvDataJuageFlag = 0;           // 重新开始判断帧头     
       commRcvData.rcvCrcErrCounter = 0;           // 重新开始判断是否CRC校验出错
       return 1;
    }

    return 0;
#else
	return 0;
#endif
}



#elif 1


Uint16 ProfibusCommErrCheck(void){}
void ProfibusRcvDataDeal(void){}
void ProfibusSendDataDeal(Uint16 err){}
Uint16 ProfibusStartDeal(Uint16 tmp){}
void UpdateProfibusCommFormat(Uint16 baudRate){}
    


#endif






