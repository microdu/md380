/***************************************************************
文件功能：和VF控制相关的数据结构定义，变量申明
文件版本：
最新更新：
************************************************************/
#ifndef MOTOR_VF_INCLUDE_H
#define MOTOR_VF_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorDefine.h"
#include "SubPrgInclude.h"
#include "MotorCurrentTransform.h"

/************************************************************/
/***********************结构体定义***************************/
typedef struct VF_INFO_STRUCT_DEF {
	Uint 	VFLineType;				//VF曲线选择
	Uint 	VFTorqueUp;				//VF转矩提升增益
	Uint 	VFTorqueUpLim;			//VF转矩提升截至频率
	Uint 	VFOverExc;				//VF过励磁增益
	Uint 	VFWsComp;				//VF转差补偿增益
	Uint 	VFOvShock;				//VF抑制振荡增益
	Uint 	VFLineFreq1;			//多点VF频率1
	Uint 	VFLineVolt1;			//多点VF电压1
	Uint 	VFLineFreq2;			//多点VF频率2
	Uint 	VFLineVolt2;			//多点VF电压2
	Uint 	VFLineFreq3;			//多点VF频率3
	Uint 	VFLineVolt3;			//多点VF电压3
	Uint 	SVCTorqueUp;			//svc转矩提升增益
	Uint 	SVCTorqueUpLim;			//svc转矩提升截至频率

    Uint    vfResComp;      // 低频电阻压降是否重新分解电流
    Uint    ovGain;     // 过压失速增益
    Uint    ovPoint;    // 过压失速抑制点
    Uint    ovPointCoff;
    Uint    ocGain;     // 过流失速增益
    Uint    ocPoint;    // 过流失速点
    int     FreqApply;  // vf 失速控制产生的频率

    int     vfMode;     // Vf失速控制模式，0:280模式，1:新模式
    int     tpLst;
}VF_INFO_STRUCT;	//VF参数设置数据结构

typedef struct  VF_AUTO_TORQUEBOOST_VAR_DEF{
    Uint        VfAutoTorqueEnable;     //  1- Execute auto torque boost
    int         DestinationVolt;        //  自动转矩提升闭环的目标电压
    int         VfCurrentIs;
    int         VfReverseAngle;
    int         VfRIsSinFai;
    int         VfRVCosFai;
    int         VfAngleSin;
    int         VfReverseVolt;
    int         VfTorqueEnableTime;
    PID_STRUCT  AutoTorquePID;
}VF_AUTO_TORQUEBOOST_VAR;	//自动转矩提升用变量结构

typedef struct OVER_SHOCK_STRUCT_DEF{
	int				IMFilter;
    int             IO;             //抑制振荡的励磁电流给定值应缓慢增加，否则大功率会在启动时引起一个电流尖峰
	PID_STRUCT		pid;
    Uint            TimesNub;       //启动后4s钟内，抑制振荡目标电流使用空载电流设定值
    Uint            oscMode;        //抑制振荡模式
    int             ShockDecrease;  //对抑制振荡增益的削减，用于DPWM调制时逐步取消抑制振荡

    int             OscVolt;
    //int             OscPhase;
}OVER_SHOCK_STRUCT;

typedef struct VF_WS_COMP_STRUCT_DEF {
	Uint 	   Coff;
	long       CompFreq;
    int        DelayTime;
    int        FilterCoff; //滤波系数
    MT_STRUCT  WsCompMT;
    MT_STRUCT  WsCompMTApply;
}VF_WS_COMP_STRUCT;	

typedef struct VAR_AVR_STRUCT_DEF{
	int		UDCFilter;
	int		CoffApply;
	int		Cnt;
}VAR_AVR_STRUCT;//过励磁模块(可调AVR功能)使用的数据结构

#if 0
typedef struct VF_CURRENT_CONTROL_DEF{
    Uint      disVfCsr;

    Uint      ocPoint;
    Uint      currentLimit;
    Uint      active;      // 电流环起作用标志
    Uint      vfCsrKP;
    Uint      vfCsrKI;
    Uint      vfCsrKD;
    
    int16       detaVolt;
    PID_STRUCT	pid;	
}VF_CURRENT_CONTROL;
#endif

typedef struct THREE_ORDER_FILTER_DEF{ 
    long    OutK1;
    long    OutK2;
    long    OutK3;
    
    long    InK1;
    long    InK2;
    long    InK3;
    
    long    OutData1;
    long    OutData2;
    long    OutData3;
    
    long    InData1;
    long    Indata2;
    long    Indata3;    
}THREE_ORDER_FILTER; //三阶滤波函数使用的系数

typedef struct VF_VAR_CALC_DEF{
    int     vfTq;       // Q12
    int     vfPt;       // Q12
}VF_VAR_CALC;

typedef struct VF_OVER_UDC_DAMP_DEF
{
    //int16 mStepUdc;   // 运行频率变化增量(电压)
    int16 uDcBrakePt;
    int16 vfOvUdcPt;
    
    PID_STRUCT pidUdc;
    PID_STRUCT pidPower;

    int16       powerFdb;
    int16       powerSet;
    Uint    maxPower;
} VF_OVER_UDC_DAMP;

typedef struct VF_FREQ_DEAL_DEF
{
    int    stepCur;
    int    stepUdc;
    int    stepSet;
    int    stepEnd;
    Uint    freqDir;
    Uint    freqSet;
    Uint    freqAim;
    Uint    freqApply;
    Uint    preSpdFlag; 
    Uint    spedChg;
}VF_FREQ_DEAL;

typedef struct OVER_CURRENT_DAMP_DEF2{
	int		LowFreq;			//低频转折点 (SI)

	int		stepApply;			//实际使用的步长
	int		stepLAmp;			//电流小于0.88限流点的步长
	int		stepLAmpLim;			//电流小于0.88限流点的步长上限
	int		stepHAmp;			//电流大于0.88限流点的步长
	int		CurBak;				//上一拍电流值

	int		maxStepLF;			//低于低频转折点的最大步长
	int		maxStepHF;		//高于低频转折点的最大步长
	int		ocPointQ12;
	int		subStep;			//电流大于0.88限流点时步长减小速度
	int     addStep;
	int		Flag;				//标志
								//BIT0	=1 表示速度已经追上设定速度
								//BIT15 =1 表示电流已经超过限流点的0.88倍
} OVER_CURRENT_DAMP2;//过流抑制模块使用的数据结构

typedef struct OVER_CURRENT_DAMP_DEF{
	int		StepApply;			//实际使用的步长
	int		StepLow;			//电流小于0.88限流点的步长
	int		StepLowLim;			//电流小于0.88限流点的步长上限
	int		StepHigh;			//电流大于0.88限流点的步长
	int		CurBak;				//上一拍电流值
	int		LowFreq;			//低频转折点
	int		MaxStepLow;			//低于低频转折点的最大步长
	int		MaxStepHigh;		//高于低频转折点的最大步长
	int		CurLim;
	int		SubStep;			//电流大于0.88限流点时步长减小速度
	int		Flag;				//标志
								//BIT0	=1 表示速度已经追上设定速度
								//BIT15 =1 表示电流已经超过限流点的0.88倍
}OVER_CURRENT_DAMP;//过流抑制模块使用的数据结构

typedef struct OVER_UDC_CTL_STRUCT_DEF{
	int		CoffApply;
	int		CoffAdd;
	int		Limit;              // 过压抑制点
	int		StepApply;
    int     LastStepApply;
	int		StepBak;
	int		ExeCnt;
	int		UdcBak;
	int		Flag;
	int		FreqMax;			//减速第一拍的频率
	int     AccTimes;           //过压抑制导致的频率增加次数
	int		OvUdcLimitTime;

    //int     PreStepApply;
}OVER_UDC_CTL_STRUCT;//过流抑制模块使用的数据结构

typedef struct HVF_OSC_DAMP_STRUCT_DEF
{
#if 0
    int FreqSet;                // 功能设定同步频率
    int FreqSynApply;           // 实际同步频率
    int FreqSpliEst;            // 估计转差
    int VoltOsc;
    int CompLeakageLs;          // 是否补偿漏感电压
#endif

    int OscDampGain;            // 振荡抑制增益
    int VoltSmSet;              // m轴电压
    int VoltEmf;                // vvvf反电势，vers freq
    
    int VoltAmp;                // 输出电压幅值
    int VoltPhase;              // 电压相位

#if 0
//debug
    int detVoltT;
    int detVoltM;
    // gama 模型
    int Rs;
    int Lt;             //Q format
    int Lg;             // 漏感
    int LgPerSet;       // 漏感系数设定
    int CurM;
    int CurMDealy;      // 延迟滤波
    int taoCurMDealy;
    int CurTDealy;
#endif

    int Rs;
    int CurMagSet;          // 励磁电流设定
    int VoltAmpAdjActive;    // 电压幅值是否调整， 小功率调整有利于转矩提升
    
}HVF_OSC_DAMP_STRUCT ;

typedef struct HVF_OSC_JUDGE_INDEX_DEF
{
    int AnglePowerFactor;      // 功率因素角
    
    int wCntUse;
    int wCntRltm;
    int maxAngle;
    int minAngle;
    int oscIndex;
    
}HVF_OSC_JUDGE_INDEX;

typedef struct HVF_DB_COMP_OPT_DEF
{
    int     HVfDbCompOptActive;     // :=1 开启优化
    int     PhasPredictGain;        // 相位预测增益
    
    int     CurPhaseFeed;
    int     CurPhaseFeed_pre;  // 上一拍
    int     CurPhasePredict;   // 预测相位
    int     StepPhaseSet;       // 每个中断的相位step(Q15)
    int     CurPhaseStepFed;    // 检测得到的电流相位变化
    int     CurPhaseStepPredict;// 预测相位变化量
    int     PhaseFwdFedCoeff;   // 前馈系数

    int     DbOptActHFreq;      // 优化生效上限频率点
    int     DbOptActLFreq;      // 优化生效下限频率点

    int     DbCompCpwmWidth;    // 
}HVF_DB_COMP_OPT;

/************************************************************/
/*******************供外部引用变量声明***********************/
extern VF_INFO_STRUCT			gVFPar;		//VF参数
extern VF_AUTO_TORQUEBOOST_VAR  gVFAutoVar;
extern OVER_SHOCK_STRUCT		gVfOsc;	//抑制振荡用结构变量
extern VF_WS_COMP_STRUCT		gWsComp;
extern VAR_AVR_STRUCT			gVarAvr;

//extern VF_CURRENT_CONTROL       gVfCsr;
extern MT_STRUCT				gHVfCur;

extern THREE_ORDER_FILTER      gOscAmp;
extern OVER_UDC_CTL_STRUCT		gOvUdc;
extern Uint const gExponentVf[][129];

extern HVF_OSC_DAMP_STRUCT      gHVfOscDamp;
extern HVF_OSC_JUDGE_INDEX      gHVfOscIndex;
extern HVF_DB_COMP_OPT          gHVfDeadBandCompOpt;

/*******************供外部引用函数声明***********************/

int  CalOutVotInVFStatus(int);
void OverShockControl(void);
void VFOverMagneticControl(void);
void VfVarInitiate(void);
void VFWsTorqueBoostComm( void );
void VFWSCompControl(void);		
void VfFreqDeal2(void);
void VFSpeedControl(void);
void VFAutoTorqueBoost(void);

void VfFreqDeal(void);
void VfOverCurDeal();
void VfOverUdcDeal();

#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================

