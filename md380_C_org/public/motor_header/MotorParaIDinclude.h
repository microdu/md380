/***************************************************************
文件功能：电机参数辨识相关的变量和函数声明。包括同步机和异步机
文件版本：
最新更新：

************************************************************/
#ifndef MOTOR_PARA_ID_INCLUDE_H
#define MOTOR_PARA_ID_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorInclude.h"
#include "MotorDefine.h"

#define TUNE_STEP_NOW (gGetParVarable.ParEstContent[gGetParVarable.ParEstMstep])

// // 结构体定义 
typedef enum TUNE_FLOW_ENUM_DEF{
    TUNE_NULL = 0,		         //没有调谐
    TUNE_IM_STATIC,              //异步机电机参数静态调谐
    TUNE_IM_ROTOR,		         //异步机电机参数旋转调谐
    TUNE_PM_COMP_LOAD = 11,   //同步机空载零点位置识别
    TUNE_PM_COMP_NO_LOAD,  	     //同步机带载零点位置识别
    TUNE_PM_PARA_temp,           // 13 debug
    TUNE_INERTIA = 20            //电机和负载惯量调谐
}TUNE_FLOW_ENUM;             //用户设定的辨识方式，是不同辨识单元的组合

typedef struct UV_AMP_COFF_STRUCT_DEF {
	Ulong	TotalU;
	Ulong	TotalV;
	Ulong	TotalVoltL;
	Ulong	TotalIL;
	Ulong	TotalVolt;
	Ulong	TotalI;
	Uint	Number;
	Uint	Comper;
	Uint	ComperL;
	Uint 	UDivVGet;
	Uint 	UDivV;
	Uint 	UDivVSave;
    Uint    IdRsCnt;            //定子电阻辨识次数
    Uint    IdRsDelay;          //定子电阻重复辨识等待时间
    Uint    IdRsBak;            //上次定子电阻辨识值
    Uint    Rs_PRD;             //定子调谐载频
}UV_BIAS_COFF_STRUCT;        //检测两相电流检测增益偏差的数据结构

typedef enum PAR_EST_MAIN_STEP_ENUM{
    IDENTIFY_RS,            // im and pm 定子电阻辨识
    IDENTIFY_RR_LO,         // im 异步机转子电阻和漏感辨识
    IDENTIFY_LM_IO,         // im 异步机互感和空载电流辨识

    PM_EST_POLSE_POS,       // pm 磁极位置辨识
    PM_EST_NO_LOAD,         // pm 空载编码器零点位置辨识
    PM_EST_WITH_LOAD,       // pm 带载编码器零点位置辨识
    PM_EST_BEMF,            // pm 反电动势辨识
    
    IDENTIFY_END            //结束辨识，恢复到正常运行状态
}PAR_EST_MAIN_STEP;     //将用户设定的辨识过程，分解为多个独立模块，顺序执行

typedef enum TUNE_TO_FUNCTION_WORLD_ENUM_DEF{
    TUNE_INITIAL,                           //初始状态
    TUNE_ACC = 50,  			            //开始加速
    TUNE_DEC = 51,				            // 开始减速
    TUNE_SUCCESS = 100,                     //参数计算完成，可以保存
    TUNE_FINISH = 1000                      //参数辨识因故障或用户输入而停止
}TUNE_TO_FUNCTION_WORLD_ENUM;        //参数辨识过程中反馈给功能模块的状态字

typedef struct MOTOR_PARAMETER_IDENTIFY_STRUCT{
    PAR_EST_MAIN_STEP    ParEstContent[IDENTIFY_PROGRESS_LENGTH];       //实际参数辨识的主循环过程
    Uint16                              ParEstMstep;                    //参数辨识当前顺序号
    Uint16                              IdSubStep;                          //控制各个参数辨识的内部循环
    TUNE_TO_FUNCTION_WORLD_ENUM         StatusWord;
    TUNE_FLOW_ENUM                      TuneType;                           // 功能传递的调谐类型选择

    int  QtEstDelay;       // quit par-est delay counter
}MOTOR_PARA_EST;

typedef struct IDENTIFY_RRLO_VARIABLE_STRUCT{
int     WaitCnt;
int     IsSampleValue[20];
int     CurrentMax;
int     CurrentRatio;
int     RrL07PulseOverSig;
int     RrL0CycleCnt;
int     SampleTimes;
Uint    PwmCompareValue;
int     UdcVoltage;
long    RrAndRsMax;
long    RrAndRsMin;
long    LoMax;
long    LoMin;
long    RrAndRsAccValue;
long    LoAccValue;
} IDENTIFY_RRLO_VARIABLE;

typedef struct IDENTIFY_LMIO_VARIABLE_STRUCT{
int     WaitCnt;
int     DestinationFreq;
int     DataSavedNum;
Uint    VFOvShock;
long    lIsValue;
long    LmAccValue;
long    IoAccValue;
long    lImAccValue;
long    lIsAccValue;
}IDENTIFY_LMIO_VARIABLE;

// // 供外部引用变量声明 
extern UV_BIAS_COFF_STRUCT		 gUVCoff;
extern MOTOR_PARA_EST            gGetParVarable;
extern MOTOR_EXTERN_STRUCT		 gMotorExtReg;
extern IDENTIFY_RRLO_VARIABLE    gRrLoIdentify;
extern IDENTIFY_LMIO_VARIABLE    gLmIoIdentify;

// // 供外部引用函数声明 
extern void RunCaseGetPar(void);
extern void PrepareParForTune(void);
extern void ChgParForEst(void);
extern void LmIoInPeriodInt(void);

#ifdef __cplusplus
}
#endif /* extern "C" */
#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================
