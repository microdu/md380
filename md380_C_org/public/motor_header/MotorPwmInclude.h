/***************************************************************
文件功能：PWM发波和载波计算相关
文件版本：
最新更新：
************************************************************/
#ifndef MOTOR_PWM_INCLUDE_H
#define MOTOR_PWM_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorInclude.h"
#include "MotorDefine.h"

/************************************************************/
/************************宏定义******************************/

/************************************************************/
/***********************结构体定义***************************/
typedef struct FC_CAL_STRUCT_DEF{
	Uint	Cnt;
	Uint	Time;
	Uint	FcBak;
    Uint    FcLimitOvLoad;
}FC_CAL_STRUCT;//计算载波频率程序使用的数据结构
typedef struct ANGLE_STRUCT_DEF {
	long 	StepPhase;			//步长角度（计算出来）
	long 	StepPhaseApply;		//步长角度（实际使用）
	long 	IMPhase; 			//M轴角度
	int 	OutPhase; 			//PWM角度
	int  	CompPhase;			//相位延迟补偿角度
	int  	RotorPhase;			//转子角度
}ANGLE_STRUCT;
typedef struct OUT_VOLT_STRUCT_DEF {
	int  	Volt;				//Q12中间计算过程的输出电压和相位
	int     MaxOutVolt;         //以电机电压的标么值
	int     vfSplit;            //si, 1V, VF分离时的输出电压
	int   	VoltPhase;

	int  	VoltApply;			//Q12计算调制系数使用的输出电压和相位
	int     VoltDisplay;        //显示输出电压，基值为变频器额定电压
	int   	VoltPhaseApply;
	Uint	VoltSVCCalSignal;
	Uint	VoltSVCCalSignalB;

    Uint    antiVolt;
    int16   detVfVoltUp;
}OUT_VOLT_STRUCT;
typedef enum ZERO_LENGTH_PHASE_SELECT_ENUM_DEF{
    ZERO_VECTOR_U,        //DPWM调制时，U相发全脉宽
    ZERO_VECTOR_V,        //DPWM调制时，V相发全脉宽  
    ZERO_VECTOR_W,        //DPWM调制时，W相发全脉宽
    ZERO_VECTOR_NONE=100  //没有发全脉宽的相
}ZERO_LENGTH_PHASE_SELECT_ENUM;//PWM输出，指明哪一相发的是全脉宽
typedef struct PWM_OUT_STRUCT_DEF {
	long	U;
	long	V;
	long	W;				//该结构前面的参数不要改变
    ZERO_LENGTH_PHASE_SELECT_ENUM  gZeroLengthPhase; 

	Uint	gPWMPrd;		//计算得到载波周期
	Uint	gPWMPrdApply;	//中断中实际使用载波周期

	Uint	AsynModle;		//异步调制模式/同步调制模式选择
	Uint	PWMModle;		//连续调制模式/离散调制模式选择
	Uint    SoftPWMTune;
	int    	SoftPWMCoff;

    int     PwmModeSwitchHF;
    int     PwmModeSwitchLF;
}PWM_OUT_STRUCT;	//作为PWM输出的结构

typedef struct DEAD_BAND_STRUCT_DEF{
   int		DeadBand;			//死区时间
   Uint     DeadTimeSet;       //1140V死区时间调整系数
   int		Comp;				//补偿时间
   Uint     CompCoff;           //死区补偿量校正系数
   int     	CompU;
   int     	CompV;
   int     	CompW;
   int		MTPhase;
   long     InvCurFilter;
}DEAD_BAND_STRUCT; 	//死区/死区补偿相关变量
typedef struct SYN_PWM_STRUCT_DEF {
	Ulong   FcApply;
	Uint	ModuleApply;	//实际使用的调制方式
	Uint 	Index;			//同步调制的载波比
	Uint	AbsFreq;
	Uint	Flag;
}SYN_PWM_STRUCT;	//


/************************************************************/
/*******************供外部引用变量声明***********************/
extern FC_CAL_STRUCT			gFcCal;
extern ANGLE_STRUCT			    gPhase;		//角度结构
extern OUT_VOLT_STRUCT			gOutVolt;
extern Uint					    gRatio;			//调制系数
extern PWM_OUT_STRUCT			gPWM;
extern SYN_PWM_STRUCT			gSynPWM;
extern DEAD_BAND_STRUCT		    gDeadBand;

/************************************************************/
/*******************供外部引用函数声明***********************/
void DeadBandComp(void);
void SendPWM(void);
void CalCarrierWaveFreq(void);
void SoftPWMProcess(void);
void CalOutputPhase(void);
void CalDeadBandComp(void);
void InitSetPWM(void);
void OutPutPWMVC(void);
void OutPutPWMVF(void);

void ImSvcDeadBandComp(void);
void HVfDeadBandComp(void);
void ImFvcDeadBandComp(void);
void PmFvcDeadBandComp(void);


#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================


