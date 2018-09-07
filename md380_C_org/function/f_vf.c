//======================================================================
//
// VF分离
//
// Time-stamp: <2012-10-22 21:06:51  Shisheng.Zhi, 0354>
//
//======================================================================


#include "f_main.h"
#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_io.h"



#if F_DEBUG_RAM        // 仅调试功能，在CCS的build option中定义的宏

#define DEBUG_F_VF          0       // VF分离

#elif 1

#if !DEBUG_F_POSITION_CTRL
#define DEBUG_F_VF          1       // VF分离
#elif 1
#define DEBUG_F_VF          0       // VF分离
#endif

#endif



Uint16 vfSeprateVolAim;        // VF分离的电压的目标值
Uint16 vfSeparateVol;           // VF分离输出电压


#if DEBUG_F_VF
LINE_CHANGE_STRUCT vfSeprateVolAccLine = LINE_CHANGE_STRTUCT_DEFALUTS;
void vfSeparateDeal(void);
extern int32 FrqPlcSetDeal(void);
extern Uint16 outVoltageDisp;

//-----------------------------------------------------------
//
// VF分离
//
//-----------------------------------------------------------
void vfSeparateDeal(void)
{
    int16 tmp;
    
    // 不是VF分离，不处理
    if (((FUNCCODE_vfCurve_ALL_SEPARATE != funcCode.code.vfCurve)     // 不为VF分离
       && (FUNCCODE_vfCurve_HALF_SEPARATE != funcCode.code.vfCurve))
	   || (motorFc.motorCtrlMode != FUNCCODE_motorCtrlMode_VF)       // 不为VF运行
	   )
    {
        vfSeparateVol = 0;
        vfSeprateVolAim = 0;
        return;
    }

    switch (funcCode.code.vfVoltageSrc)     // VF分离的电压源
    {
        case FUNCCODE_vfVoltageSrc_FC:      // 功能码设定
            vfSeprateVolAim = funcCode.code.vfVoltageDigtalSet;
            break;

        case FUNCCODE_vfVoltageSrc_AI1:     // AI1
        case FUNCCODE_vfVoltageSrc_AI2:     // AI2
        case FUNCCODE_vfVoltageSrc_AI3:     // AI3
            tmp = aiDeal[funcCode.code.vfVoltageSrc - FUNCCODE_vfVoltageSrc_AI1].set;
            vfSeprateVolAim = (int32)tmp * funcCode.code.motorParaM1.elem.ratingVoltage >> 15;
            break;

        case FUNCCODE_vfVoltageSrc_PULSE:  // PULSE脉冲设定
            vfSeprateVolAim = (int32)pulseInSet * funcCode.code.motorParaM1.elem.ratingVoltage >> 15;
            break;

        case FUNCCODE_vfVoltageSrc_MULTI_SET: // 多段速
            vfSeprateVolAim = (int32)UpdateMultiSetFrq(diFunc.f1.bit.multiSet) 
                * funcCode.code.motorParaM1.elem.ratingVoltage / maxFrq;
            break;

        case FUNCCODE_vfVoltageSrc_PLC:  // PLC
            vfSeprateVolAim = (int32)FrqPlcSetDeal() 
                * funcCode.code.motorParaM1.elem.ratingVoltage / maxFrq;
            break;

        case FUNCCODE_vfVoltageSrc_PID:  // PID
            vfSeprateVolAim = (int32)FrqPidSetDeal() 
                * funcCode.code.motorParaM1.elem.ratingVoltage / maxFrq;
            break;

        case FUNCCODE_vfVoltageSrc_COMM: // 通讯
            vfSeprateVolAim = (int32)(int16)funcCode.code.frqComm 
                * funcCode.code.motorParaM1.elem.ratingVoltage / 10000;
            break;

        default:
            break;
    }

    vfSeprateVolAim = ABS_INT16((int16)vfSeprateVolAim);

    // 半分离模式
    if (FUNCCODE_vfCurve_HALF_SEPARATE == funcCode.code.vfCurve)
    {
        // V/F=2 * X * （电机额定电压）/（电机额定频率）
        // V = 2 * F * X * (电机额定电压) / (电机额定频率)
        // V = 2 * F * vfSeprateVolAim / 电机额定频率
        vfSeprateVolAim = ((Uint32)2 * ABS_INT32(frq) * vfSeprateVolAim) / motorFc.motorPara.elem.ratingFrq;
    }

     // 停机状态输出电压为0
    if (!runFlag.bit.run)
    {
        vfSeparateVol = 0;
        return;
    }

    // 限制最大输出电压为电机额定电压
    if (vfSeprateVolAim > motorFc.motorPara.elem.ratingVoltage)
    {
        vfSeprateVolAim = motorFc.motorPara.elem.ratingVoltage;
    }
     
// VF分离的电压上升时间
    vfSeprateVolAccLine.aimValue = vfSeprateVolAim;
    vfSeprateVolAccLine.tickerAll = (Uint32)funcCode.code.vfVoltageAccTime * (TIME_UNIT_VF_VOL_ACC_TIME / VF_CALC_PERIOD);
    vfSeprateVolAccLine.maxValue = motorFc.motorPara.elem.ratingVoltage;
    vfSeprateVolAccLine.curValue = vfSeparateVol;    // 在当前输出电压的基础上变化
    vfSeprateVolAccLine.calc(&vfSeprateVolAccLine);
    vfSeparateVol = vfSeprateVolAccLine.curValue;
}


#elif 1

void vfSeparateDeal(void){}




#endif



