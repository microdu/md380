/***************************************************************
文件功能：和矢量控制相关的数据结构定义，变量申明
文件版本：
最新更新：

************************************************************/
#ifndef MOTOR_VC_INCLUDE_H
#define MOTOR_VC_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorDefine.h"
#include "SubPrgInclude.h"
#include "MotorInclude.h"
/************************************************************/
/************************数据结构定义************************/
typedef struct VC_INFO_STRUCT_DEF {
	Uint 	ASRKpLow;				//低频速度环KP
	Uint 	ASRTILow;				//低频速度环TI
	Uint 	ASRKpHigh;				//高频速度环KP
	Uint 	ASRTIHigh;				//高频速度环TI
	Uint 	ASRSwitchLow;			//低频切换频率
	Uint 	ASRSwitchHigh;			//高频切换频率

	//Uint 	ACRKpLow;				//电流环KP
	//Uint 	ACRKiLow;				//电流环KI

    Uint    AcrImKp;        // 同步机和异步机电流环参数
    Uint    AcrImKi;        // M 轴电流环积分
    Uint    AcrItKp;        // T轴电流环增益
    Uint    AcrItKi;        // T轴电流环积分
    Uint    SvcMode;
    
    Uint    VcOverExc;              //矢量过励磁增益
	Uint 	VCTorqueLim;			//VC转矩限定
	Uint 	VCWsCoff;				//VC转差补偿
	Uint 	VCSpeedFilter;			//VC速度环滤波时间
	Uint    FreqWsCalMode;          //转差修正量计算模式
	Uint    FreqWsVolSub;           //按T轴电压计算转差修正量时，电压修正量
	Uint    FreqWsVolModify;        //按T轴电压计算转差修正量时，修正量设定值	
}VC_INFO_STRUCT;	//和矢量控制相关的参数设置数据结构
typedef struct UDC_LIMIT_IT_STRUCT_DEF {
    int    UDCLimit;           //由于母线电压上升，限制的转矩上限
    int    UDCBak;
    int    UDCBakCnt;
    int    UDCDeta;
    int    FirstOvUdcFlag;     //第一次母线电压上升到限制值
    PID_STRUCT  UdcPid;
}UDC_LIMIT_IT_STRUCT;	//速度环调节数据结构
typedef struct ASR_STRUCT_DEF {
	PID_STRUCT	Asr;
    Uint        AsrQpi;             // 按位，个位:Qp；十位:Qi；
    PID_STRUCT  TorqueLimitPid;     // 非转矩控制，最大电流限制pid
	int  	KPHigh;
	int  	KPLow;
	int  	KIHigh;
	int  	KILow;
	int  	SwitchHigh;
	int  	SwitchLow;
	int  	TorqueLimit;
}ASR_STRUCT;	//速度环调节数据结构
typedef struct VC_CSR_PARA_DEF{
    long   ImModify;            //开环矢量电压计算运行时间  
    Uint   ExecSetMode;         // 0 励磁电流反比速度设定；1 根据最大电压调整
    Uint   ImAcrKp;             //励磁电流给定值调节器KP
    Uint   ImAcrKi;             //励磁电流给定值调节器KI
    Uint   ImModefyAdd;         //励磁电流给定值正向修正量
    Uint   ImModefySub;         //励磁电流给定值反向修正量
    Uint   LmGain;              //弱磁区互感变化增益
}VC_CSR_PARA;

typedef struct MODIFYWS_STRUCT_DEF{
	int    Faiq;
    Uint   WsMax;   //调节器上限设定值
    Uint   Kp;      
    int    Amp;
	int    Wsout; 
    int    Tmp;
	int    Delta;
	int    tc;
 	int    tmp1;
 	int 	tmp2;               // SVC 优化开启功能码
 	Uint    Theta;  
 	int    Ea;
 	int    Eb; 
 	int    Utheta;  
	int    Xztotal;
}MODIFYWS_STRUCT;


/************************************************************/
/************************变量引用申明************************/
extern VC_INFO_STRUCT			gVCPar;			//VC参数
extern MT_STRUCT_Q24            gIMTSet;		//MT轴系下的设定电流
extern MT_STRUCT_Q24            gIMTSetApply;	//MT轴系下的电流指令值
extern MT_STRUCT				gUMTSet;		//MT轴系下的设定电压
extern AMPTHETA_STRUCT			gUAmpTheta;		//极坐标下设定电压

extern PID32_STRUCT     gImAcrQ24;
extern PID32_STRUCT     gItAcrQ24;
extern PID32_STRUCT        gIMAcr;
extern PID32_STRUCT        gITAcr;

extern ASR_STRUCT				gAsr;			//速度环
extern VC_CSR_PARA              gVcCsrPara;
extern MODIFYWS_STRUCT          gModifyws;
extern MT_STRUCT               gPWMVAlphBeta;
extern UDC_LIMIT_IT_STRUCT      gUdcLimitIt;
/************************************************************/
/*******************供外部引用函数声明***********************/
extern void ResetParForVC(void);
extern void CalIMSet(void);
extern void VCSpeedControl(void);
extern void VcCalABVolt(void);
extern void PrepImCsrPara(void);
extern void PrepPmsmCsrPrar(void);
extern void PrepareCsrPara(void);
extern void VCCsrControl(void);


#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================

