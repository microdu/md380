//======================================================================
//
// 转矩控制相关处理
//
// Time-stamp: <2012-06-14 16:04:40  Shisheng.Zhi, 0354>
//
//======================================================================


#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_io.h"
#include "f_main.h"
#include "f_p2p.h"

#if F_DEBUG_RAM                     // 仅调试功能，在CCS的build option中定义的宏
#define DEBUG_F_TORQUE      0       // 转矩控制
#elif 1
#define DEBUG_F_TORQUE      1       // 转矩控制
#endif

int16 upperTorque;          // 速度控制时为转矩上限，转矩控制时为设定转矩
int16 torqueAim;            // 转矩目标设定

Uint16 torqueSetEthDp;      // PLC通讯给定的设定转矩，转矩控制时使用
Uint16 torqueUpperEthDp;    // PLC通讯给定的转矩上限，速度控制时使用

#define TORQUE_LIMIT_TIME   800     // 转矩限定中，判断时间，_ms
extern Uint16 itDisp;

#if DEBUG_F_TORQUE

LINE_CHANGE_STRUCT torqurLine = LINE_CHANGE_STRTUCT_DEFALUTS;

// 更新转矩控制目标值
void UpdateTorqueAim(void);

// 转矩线性计算
void AccDecTorqueCalc(void);

//=====================================================================
// 
// 转矩上限
// 
//=====================================================================
void TorqueCalc(void)
{
    UpdateTorqueAim();       // 更新转矩目标设定

    AccDecTorqueCalc();      // 转矩加减速计算
}

// 更新转矩目标设定
void UpdateTorqueAim(void)
{
    int16 tmp,tmp1;
    Uint16 torqueSrc;   // 转矩源
    int16 torquePu;     // 转矩的PU

    // 非转矩控制
    if (RUN_MODE_TORQUE_CTRL != runMode)    
    {
        // 速度控制转矩源、转矩上限
        torqueSrc = motorFc.vcPara.spdCtrlDriveTorqueLimitSrc;
        torquePu = motorFc.vcPara.spdCtrlDriveTorqueLimit;
    }
    // 转矩控制
    else    
    {
        // 驱动控制转矩源、转矩上限
        torqueSrc = funcCode.code.driveUpperTorqueSrc;  // 驱动转矩上限源
        torquePu = funcCode.code.driveUpperTorque;      // 驱动转矩上限数字设定
    }

    // 驱动转矩上限源
    switch (torqueSrc)
    {     
        case FUNCCODE_upperTorqueSrc_FC:  // 功能码设定
            tmp = torquePu;
            break;

        case FUNCCODE_upperTorqueSrc_AI1:  // AI1
        case FUNCCODE_upperTorqueSrc_AI2:  // AI2
        case FUNCCODE_upperTorqueSrc_AI3:  // AI3
            tmp = torqueSrc - FUNCCODE_upperTorqueSrc_AI1;
            tmp = ((int32)aiDeal[tmp].set * torquePu) >> 15;
            break;
    
        case FUNCCODE_upperTorqueSrc_PULSE:  // PULSE
            tmp = ((int32)pulseInSet * torquePu) >> 15;
            break;

        case FUNCCODE_upperTorqueSrc_COMM:   // 通讯
            // funcCode.code.frqComm不能超过32767，目前 [-10000, +10000]
#if DEBUG_F_PLC_CTRL
            if (funcCode.code.plcEnable)
            {
                tmp = (int16)funcCode.code.plcTorqueSet;
            }
            else
#endif      
#if DEBUG_F_P2P_CTRL
            if ((CanRxTxCon == P2P_COMM_SLAVE) // 点对点通讯有效且接收数据作为转矩给定
                && (funcCode.code.p2pRevDataSel == P2P_REV_TORQUE_SET)
                )
            { 
                tmp = ((int32)(int16)p2pData.processValue * 2000) / 10000;
            }
            else
#endif
            {
                tmp = ((int32)(int16)funcCode.code.frqComm * torquePu) / 10000;
            }
            break;

        case FUNCCODE_upperTorqueSrc_MIN_AI1_AI2:  // MIN(AI1,AI2)
            tmp = ((int32)aiDeal[0].set * torquePu) >> 15;
            tmp1 = ((int32)aiDeal[1].set * torquePu) >> 15;
            
            if (tmp > tmp1)
            {
                tmp = tmp1;
            }
            break;
            
        case FUNCCODE_upperTorqueSrc_MAX_AI1_AI2:  // MAX(AI1,AI2)
            tmp = ((int32)aiDeal[0].set * torquePu) >> 15;
            tmp1 = ((int32)aiDeal[1].set * torquePu) >> 15;
            
            if (tmp < tmp1)
            {
                tmp = tmp1;
            }
            break;
            
        default:
            break;
    }

    torqueAim = tmp;

#if 0
#define RATING_TORQUE      2000                              // 额定转矩, 200.0%
#define TORQUE_LINE_FILTER funcCode.code.torqueFilter*5      // 转矩给定线性滤波
    // 转矩给定线性滤波
    torqurLine.aimValue = ABS_INT16(torqueAim);
    torqurLine.tickerAll = TORQUE_LINE_FILTER;
    torqurLine.maxValue = RATING_TORQUE; 
    torqurLine.curValue = upperTorque;
    torqurLine.calc(&torqurLine);
    upperTorque = torqurLine.curValue;


    AccDecTorqueCalc();
#endif
}


// 计算当前转矩设定
void AccDecTorqueCalc(void)
{
    int32 accDecTime;

    // 非转矩控制
    if (RUN_MODE_TORQUE_CTRL != runMode)    // 非转矩控制
    {
        upperTorque = torqueAim;
        return;
    }

    if (!runFlag.bit.run)       // 停机
    {
        upperTorque = 0;
        return;
    }

    if (torqueAim == upperTorque)  // 已经达到目标转矩
    {
        return;
    }

    if (((upperTorque >= 0) && (torqueAim > upperTorque))
        || ((upperTorque <= 0) && (torqueAim < upperTorque)))   // 转矩加速
    {
        accDecTime = (int32)funcCode.code.torqueCtrlAccTime;
    }
    else
    {
        accDecTime = (int32)funcCode.code.torqueCtrlDecTime;
    }
    
    accDecTime = accDecTime * (Uint16)(TIME_UNIT_TORQUE_CTRL_ACC_DEC / TORQUE_CTRL_PERIOD); // 转换成ticker

#define RATING_TORQUE   2000    // 额定转矩, 100.0%
    // 直线加减速
    torqurLine.aimValue = torqueAim;
    torqurLine.tickerAll = accDecTime;
    torqurLine.maxValue = RATING_TORQUE; 
    torqurLine.curValue = upperTorque;
    torqurLine.calc(&torqurLine);
    upperTorque = torqurLine.curValue;
}



/************************************************************
 * 功能描述: 判断是否处于转矩限定中
 * 返回: 
 *       ret :  0-未处于转矩限定中   1-处于转矩限定中
 *
************************************************************/
Uint16 TorqueLimitCalc(void)
{
    static Uint16 torqueLimitCurTicker;
    static Uint16 torqueLimitSpdTicker;
    Uint16 ret = 0;

// 转矩超过85%，持续时间超过_
    if (itDisp > (int32)(ABS_INT32(upperTorque) * 85) / 100)
    {
        if (++torqueLimitCurTicker >= TORQUE_LIMIT_TIME / DO_CALC_PERIOD)
        {
            torqueLimitCurTicker = TORQUE_LIMIT_TIME / DO_CALC_PERIOD + 1;
        }
    }
    else
    {
        torqueLimitCurTicker = 0;
    }

// 速度偏差超过2Hz，持续时间超过_
    if ( (ABS_INT32(frq - frqRun)) > 200 )
    {
        if (++torqueLimitSpdTicker >= TORQUE_LIMIT_TIME / DO_CALC_PERIOD)
        {
            torqueLimitSpdTicker = TORQUE_LIMIT_TIME / DO_CALC_PERIOD + 1;
        }
    }
    else
    {
        torqueLimitSpdTicker = 0;
    }

// 判断是否在 转矩限定中
    if ((torqueLimitCurTicker >= TORQUE_LIMIT_TIME / DO_CALC_PERIOD)
// 2009.05.12，与李工讨论后，去掉该条件
// 这个条件应该保留
        && (torqueLimitSpdTicker >= TORQUE_LIMIT_TIME / DO_CALC_PERIOD)
        )
    {
        ret = 1;
    }

    return (ret);
}


#elif 1

void TorqueCalc(void)
{
    upperTorque = motorFc.spdCtrlTorqueLimit;
}

void UpdateTorqueAim(void)
{
    torqueAim = motorFc.spdCtrlTorqueLimit;
}

Uint16 TorqueLimitCalc(void)
{
    return 0;
}
#endif







