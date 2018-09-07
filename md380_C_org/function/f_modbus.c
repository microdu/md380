//======================================================================
//
// Time-stamp: <2012-06-23  LeiMin, 0656>
//
// MODBUS 协议
//======================================================================

#include "f_comm.h"

#define RTU_MASTER_FRAME_NUM_MAX  8     // 主机命令帧的字符个数

// MODBUS协议
#define RTUslaveAddress rcvFrame[0]      // RTU帧的从机地址
#define RTUcmd          rcvFrame[1]      // RTU帧的命令字
#define RTUhighAddr     rcvFrame[2]      // RTU帧的地址高字节
#define RTUlowAddr      rcvFrame[3]      // RTU帧的地址低字节
#define RTUhighData     rcvFrame[4]      // RTU帧的数据高字节
#define RTUlowData      rcvFrame[5]      // RTU帧的数据低字节
#define RTUlowCrc       rcvFrame[6]      // RTU帧的CRC校验低字节
#define RTUhighCrc      rcvFrame[7]      // RTU帧的CRC校验高字节

//====================================================================
//
// 数据接收后信息解析
//
//====================================================================
void ModbusRcvDataDeal(void)
{    
    commRcvData.slaveAddr = RTUslaveAddress;                // 从机地址
    commRcvData.commCmd = RTUcmd;                           // 通讯命令
    commRcvData.commAddr = (RTUhighAddr << 8) + RTUlowAddr; // 操作地址
    commRcvData.commData = (RTUhighData << 8) + RTUlowData; // 操作数据
    commRcvData.crcRcv = (RTUhighCrc << 8) + RTUlowCrc;     // CRC校验值    
	commRcvData.crcSize = 6;                                // CRC校验长度
	commRcvData.commCmdSaveEeprom = SCI_WRITE_WITH_EEPROM;  // 存储EEPROM命令
}


//====================================================================
//
// MODBUS帧头判断
// 参数: tmp-接收帧数据
// 返回: 0-帧头判断过程中
//       1-不需要帧头判断，直接存储接收数据
//
//===================================================================
Uint16 ModbusStartDeal(Uint16 tmp)
{
    if ((commTicker > commRcvData.frameSpaceTime))
    {
        RTUslaveAddress = tmp;          // 超过3.5个字符时间，新的一帧的开始
        // 广播模式    
        if (RTUslaveAddress == 0)       
        {
            commRcvData.rcvNum = 1;
            commRcvData.rcvFlag = 1;    // 接收到帧的第一个字节，且是广播模式
        }
        else if ( (funcCode.code.commProtocolSec == CAN_OPEN) && (RTUslaveAddress == 1) )
        {
            commRcvData.rcvNum = 1;
            commRcvData.rcvFlag = 2;                        // CANOPEN使用 本机地址 1
        }
		// 本机地址
        else if (RTUslaveAddress == funcCode.code.commSlaveAddress) 
        {
            commRcvData.rcvNum = 1;
            commRcvData.rcvFlag = 2;                        // CANOPEN使用 本机地址 1
        }
		// 其它地址
        else
        {
            commRcvData.rcvFlag = 0;    // 地址不对应，数据不接收 
        }
        
        return 0;
    }

    return 1;
}


//====================================================================
//
// 更新通讯参数
// 1、接收数据个数
// 2、新帧开始判断时间
// 3、应答延迟时间
//
//====================================================================
void UpdateModbusCommFormat(Uint16 baudRate)
{
    commRcvData.rcvNumMax= RTU_MASTER_FRAME_NUM_MAX;      // 接收数据个数
    commRcvData.frameSpaceTime = 385 * 2 / baudRate+ 1;   // 3.5 char time=3.5*(1+8+2)/baud
    commRcvData.delay = funcCode.code.commDelay * 2;      // 应答延迟时间, _*0.5ms
}


//====================================================================
//
// 准备发送数据
// 参数: err-通讯命令处理出错信息,为0表示操作成功
// 返回: 1、发送数据长度
//       2、发送数据信息
//
//====================================================================
void ModbusSendDataDeal(Uint16 err)
{
	int16 i;
    sendFrame[0] = rcvFrame[0];   // 回复接收帧头前2位
    sendFrame[1] = rcvFrame[1];   // 回复接收帧头前2位
    commSendData.sendNumMax = 8;  // 发送数据长度
    if (err)
    {
        sendFrame[2] = 0x80;
	    sendFrame[3] = 0x01;
	    sendFrame[4] = 0x00;
        sendFrame[5] = err;
    }
    else if (sciFlag.bit.pwdPass)           // 密码通过：返回0x8888
    {
        sendFrame[2] = RTUhighAddr;
        sendFrame[3] = RTUlowAddr;
        sendFrame[4] = 0x88;
        sendFrame[5] = 0x88;
	}
    else if (sciFlag.bit.write)             // 写数据操作处理。若有错误，则报错，不会真正写
    {
        sendFrame[2] = RTUhighAddr;
        sendFrame[3] = RTUlowAddr;
        sendFrame[4] = RTUhighData;
        sendFrame[5] = RTUlowData;
	}
    else if (sciFlag.bit.read)              // 通讯参数读取处理，真正需要读取
    {
        Uint16 sendNum;
        Uint16 readDataStartIndex;
        sendNum = commRcvData.commData << 1;
        
        // 标准的MODEBUS通讯协议
        if (commProtocol)
        {
            sendFrame[2] = sendNum;                 // 接收到的是功能码个数*2
            commSendData.sendNumMax = sendNum + 5;  // 最大发送字符个数
            readDataStartIndex = 3;
        }
        // 非标准MODBUS通讯协议
        else if (commProtocol == 0)
        {
            sendFrame[2] = sendNum >> 8;     // 功能码个数高位
            sendFrame[3] = sendNum & 0x00ff; // 功能码个数低位
            commSendData.sendNumMax = sendNum + 6;    // 最大发送字符个数
            readDataStartIndex = 4;
        }

        // 读取的数据值
        for (i = commRcvData.commData - 1; i >= 0; i--)
        {
            sendFrame[(i << 1) + readDataStartIndex] = commReadData[i] >> 8;
            sendFrame[(i << 1) + readDataStartIndex + 1] = commReadData[i] & 0x00ff;
        }
    }
}


//====================================================================
//
// 通讯出错判断
// 返回: 0、通讯正常
//       1、通讯出错
//
//====================================================================
Uint16 ModbusCommErrCheck(void)
{
    if ((funcCode.code.commOverTime) && (commTicker >= (Uint32)funcCode.code.commOverTime * 2 * TIME_UNIT_sciCommOverTime))
    {
        // MODBUS通讯超时检测计时
        commTicker = (Uint32)funcCode.code.commOverTime * 2 * TIME_UNIT_sciCommOverTime;  
        return 1;
    }
    else
    {
        return 0;
    }
}












