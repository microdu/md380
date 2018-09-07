/***************************************************************
文件功能：电机控制模块程序的头文件.主要是用到的全局变量和函数的声明.
          变量声明次序与MotorVar.c文件中的定义顺序相同
文件版本：
最新更新：
************************************************************/
#ifndef MOTOR_INCLUDE_H
#define MOTOR_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

//#include "SystemDefine.h"
//#include "MotorDefine.h"
#include "MotorCurrentTransform.h"
#include "SubPrgInclude.h"
#include "MotorVFInclude.h"
#include "MotorVCInclude.h"
#include "MotorSvcInclude.h"
#include "MotorInvProtectInclude.h"
#include "MotorPwmInclude.h"
#include "MotorStructDefine.h"
#include "MotorInfoCollectInclude.h"
#include "MotorParaIDinclude.h"
#include "MotorPublicCalInclude.h"
#include "MotorSpeedCheceInclude.h"
#include "MotorDataExchange.h"
#include "MotorPmsmMain.h"
#include "MotorEncoder.h"
#include "MotorPmsmParEst.h"

/************************************************************
    变量引用 BEGIN
************************************************************/
/*****************以下为基本变量声明*************************/
extern INV_STRUCT 				gInvInfo;		//变频器信息
extern CONTROL_MOTOR_TYPE_ENUM  gCtrMotorType;  //电机类型和控制模式的组合
extern MOTOR_STRUCT 			gMotorInfo;		//电机信息
extern MOTOR_EXTERN_STRUCT		gMotorExtInfo;	//电机扩展信息（实际值表示）
extern MOTOR_EXTERN_STRUCT		gMotorExtPer;	//电机扩展信息（标么值表示）
extern RUN_STATUS_STRUCT 		gMainStatus;	//主运行状态
extern BASE_COMMAND_STRUCT		gMainCmd;		//主命令
extern MAIN_COMMAND_EXTEND_UNION gExtendCmd;   //主命令字扩展
extern SUB_COMMAND_UNION         gSubCommand;	//辅命令字结构
/************************************************************/

/**********以下为和电机控制相关设定参数声明******************/
extern BASE_PAR_STRUCT			gBasePar;	//基本运行参数
extern COM_PAR_INFO_STRUCT		gComPar;	//公共参数
extern MOTOR_POWER_TORQUE       gPowerTrq;

/**********************独立模块变量声明**********************/
extern CPU_TIME_STRUCT			 gCpuTime;
extern MOTOR_DEBUG_DATA_RECEIVE_STRUCT     gTestDataReceive;//预留的用于驱动部分调试的数据
extern MOTOR_DEBUG_DATA_DISPLAY_STRUCT     gTestDataDisplay;//预留的用于显示驱动部分调试数据

/************************************************************/
/**********************以下为常量声明************************/
extern Uint const gDeadBandTable[];
extern Uint const gDeadCompTable[];
extern Uint const gInvCurrentTable220S[];
//extern Uint const gInvCurrentTable220T[];
extern Uint const gInvTypeTable380To220T[16];
extern Uint const gInvCurrentTable380T[];
extern Uint const gInvCurrentTable690T[];
extern Uint const gInvCurrentTable1140T[];
extern Uint const gInvCurrentTable220T[];
extern Uint  const gInvVoltageInfo220S[]; 
extern Uint  const gInvVoltageInfo220T[];
extern Uint  const gInvVoltageInfo380T[];
extern Uint  const gInvVoltageInfo480T[];
extern Uint  const gInvVoltageInfo690T[]; 
extern Uint  const gInvVoltageInfo1140T[];
/**********************调试用变量*************************/
extern int * pVD1;
extern int * pVD2;
extern int * pVD3;
extern int * pVD4;
extern int * pVD5;
extern int * pVD6;
extern int   startSave;

/**********************调试函数说明*************************/
extern void SaveDebugData16(Uint);
extern void SaveDebugData32(unsigned long);
extern void ResetDebugBuffer(void);

/**********************以下是中断程序说明*********************/
extern interrupt void ADC_Over_isr(void);
extern interrupt void EPWM1_TZ_isr(void);
extern interrupt void EPWM1_zero_isr(void);
extern interrupt void PG_Zero_isr(void);

/*********************系统初始化函数声明*********************/
extern void InitSysCtrl(void);   
extern void InitInterrupt(void);   
extern void InitPeripherals(void);   
extern void InitForMotorApp(void);
extern void InitForFunctionApp(void);   
extern void SetInterruptEnable(void);
extern void EnableDog(void);
extern void DisableDog(void);
extern void KickDog(void);
void InitSetAdc(void);

/************************************************************
函数引用 END
************************************************************/

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================

