/***************************************************************
文件功能：开环矢量控制
文件版本：
最新更新：
************************************************************/
#ifndef MOTOR_SVC_INCLUDE_H
#define MOTOR_SVC_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "MotorInclude.h"
/************************************************************/
/***********************结构体定义***************************/
typedef struct SVC_FLUX_CAL_STRUCT_DEF {
	long 	FilterTime;
	Uint 	SampleTime;
}SVC_FLUX_CAL_STRUCT;	//计算磁通用的变量集合

typedef struct FLUX_STRUCT_DEF {
	Uint	Amp;
	Uint	Theta;
}FLUX_STRUCT;	//磁通观测结果用的变量集合
typedef struct SVC_SPEED_STRUCT_DEF {
	int		SvcSynSpeed;
	int		SvcWs;
	int		SvcRotorSpeed;

	//SVCD0 专用变量
	int		SvcLastFluxPos;
	Uint	SvcSignal;           //用于SVC0启动的稳定性补偿
    int     DetaTimer;         //仅用于SVC0
	Ulong	Timer;             //仅用于SVC0同步速计算
    int     DetaPhase;         //仅用于SVC0转子磁场同步速计算
    
}SVC_SPEED_STRUCT;	//SVC转速估计数据结构

/************************************************************/
/*******************供外部引用变量声明***********************/
                                                    
extern FLUX_STRUCT              gFluxR;
extern FLUX_STRUCT              gFluxS;
extern MT_STRUCT_Q24           gIMTQ24_obs;  //除转速估计外，需供电流控制函数使用
extern MT_STRUCT_Q24           gIMTQ12_obs;  //除转速估计外，需供电流控制函数使用

/************************************************************/
/*******************供外部引用函数声明***********************/
void ResetSVC(void);
void SVCCalRotorSpeed(void);
void SvcCalOutVolt(void);
void SVCCalFlux_380(void);        
void SvcCalOutVolt_380(void);
void SVCCalRotorSpeed_380(void);



#ifdef __cplusplus
}
#endif /* extern "C" */
#endif  // end of definition

