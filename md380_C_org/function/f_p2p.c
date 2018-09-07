//======================================================================
//
// Time-stamp: <2012-2-21 Lei.Min, 0656>
//
// P2P comm
// 点对点通讯数据处理
//
//======================================================================


#include "f_ui.h"
#include "f_menu.h"
#include "f_frqSrc.h"
#include "f_comm.h"
#include "f_runSrc.h"
#include "f_io.h"
#include "f_error.h"
#include "f_p2p.h"

#if F_DEBUG_RAM
#else
#endif

#if DEBUG_F_P2P_CTRL

P2P_DATA_STRUCT p2pData;


void P2PDataDeal(void)
{
    // 点对点通讯有效
    if (funcCode.code.p2pEnable) 
    {
        // 点对点通讯有效
        p2pData.p2pEnable = 1;   
    }
    else
    {
        p2pData.p2pEnable = 0;     // 点对点通讯无效
        p2pData.p2pSendTcnt = 0;   // 清点对点发送计时
        p2pData.p2pCommErrTcnt = 0;
    }

    // 点对点通讯有效
    if (p2pData.p2pEnable)
    {
        // 主机传送数据
        if (!funcCode.code.p2pTypeSel)
        {
            // 主机传送数据选择
            switch(funcCode.code.p2pSendDataSel)
            {
                // 发送输出转矩给从机
                case P2P_OUT_TORQUE:
                    p2pData.P2PSendData = (int32)itDisp * 10000 / 2000;
                    break;

                // 发送设定频率给从机
                case P2P_FRQ_SET:
                    p2pData.P2PSendData = frqRun * 10000 / maxFrq;
                    break;

                // 发送目标频率给从机
                case P2P_FRQ_AIM:
                    p2pData.P2PSendData = frqAimTmp * 10000 / maxFrq;
                    break;

                // 发送编码器反馈频率给从机
                case P2P_FRQ_FDB:
                    p2pData.P2PSendData = (int32)frqFdb * 10000 / maxFrq;
                    break;

                default:
                    break;
            }

            // 点对点发送周期
            p2pData.p2pSendPeriod = (funcCode.code.p2pSendPeriod + 1) >> 1;
            
        }
        // 从机接收数据
        else
        {  
            // 数据接收中断计时(接收到数据时清0)
            p2pData.p2pCommErrTcnt++;
            
            // 增益零偏处理
            p2pData.processValue = ((int32)(int16)p2pData.P2PRevData*funcCode.code.p2pRevGain / 100) + ((int16)funcCode.code.p2pRevOffset);

            // 限幅
            if (((int16)p2pData.processValue) >= ((int16)10000))
            {
                p2pData.processValue = ((int16)10000);
            }
            else if (((int16)p2pData.processValue) <= ((int16)-10000))
            {
                p2pData.processValue = (Uint16)((int16)-10000);
            }
        }

        // 点对点通讯中断检测
        if ((p2pData.p2pCommErrTcnt >= (funcCode.code.p2pOverTime*50))
            && (funcCode.code.p2pOverTime))
        {
            errorOther = ERROR_COMM;     // 通讯故障
            errorInfo = COMM_ERROR_MODBUS;
            p2pData.p2pCommErrTcnt = 0;
        }
    }
}
#else
void P2PDataDeal(void);
#endif


