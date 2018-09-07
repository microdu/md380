//=====================================================================
//
// 性能与功能的接口
//
//=====================================================================

#ifndef __F_INTERFACE_H__
#define __F_INTERFACE_H__

//-----------------------------------------------
// 性能与功能的交互
#define FUNC_TO_MOTOR_05MS_DATA_NUM     14      // 功能==>性能的参数个数，0.5ms
extern int16 gSendToMotor05MsDataBuff[];        // 功能==>性能的参数，0.5ms

#define FUNC_TO_MOTOR_2MS_DATA_NUM      75     // 功能==>性能的参数个数，2ms
#define FUNC_TO_CORE_DEBUG_DATA_NUM     40      // 调试使用，func2motor
extern int16 gSendToMotor2MsDataBuff[];         // 功能==>性能的参数，2ms

// 最后FUNC_TO_CORE_DEBUG_DATA_NUM个参数，CF组功能码，每拍都会传递给性能，性能调试使用

#define MOTOR_TO_Func_2MS_DATA_NUM      30      // 性能==>功能的参数个数，2ms
#define CORE_TO_FUNC_DISP_DATA_NUM	    30      // 调试使用，motor2func
extern int16 gSendToFunctionDataBuff[];         // 性能==>功能的参数，2ms
// 最后CORE_TO_FUNC_DISP_DATA_NUM个参数，UF组，性能显示使用。前面(REM_P_OFF_MOTOR)个掉电记忆

#define MOTOR_TO_Func_05MS_DATA_NUM       0
extern int16 gRealTimeToFunctionDataBuff[];     // 性能==>功能的参数，实时

#define TUNE_DATA_NUM                  20       // 调谐的参数个数
extern int16 gParaIdToFunctionDataBuff[];       // 调谐
//-----------------------------------------------




// 功能传递给性能的数据
extern Uint16 frq2Core;
extern Uint16 frqCurAim2Core;

// 性能传递给功能的数据
extern Uint16 errorCodeFromMotor;
extern Uint16 currentOc;
extern Uint16 generatrixVoltage;
extern Uint16 outVoltage;
extern Uint16 outCurrent;
extern int16 outPower;
extern Uint16 currentPu;
extern Uint16 rsvdData;

extern int32 frqRun;
extern int32 frqVFRun;
extern int32 frqVFRunRemainder;
extern Uint16 frqFdb;
extern Uint16 frqFdbDisp;
extern int32  frqFdbTmp;
extern Uint16 frqFdbFlag;
extern Uint16 motorRun;     // 编码器反馈速度
//extern Uint16 PGErrorFlag;  // 参数辨识PG卡故障标志

extern Uint16 outCurrentDispOld;

extern Uint16 enCoderPosition;
extern Uint16 ABZPos;
// 性能传递给功能即时
extern int16 gPhiRtDisp;     // 2    功率因数角度

extern Uint16 pmsmRotorPos;  // 同步机转子位置(性能实时更新)

#endif // __F_INTERFACE_H__



