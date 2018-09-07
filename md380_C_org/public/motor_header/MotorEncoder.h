/***************************************************************************
文件说明：
文件功能：
文件版本：
最新更新：
更新日志：
***************************************************************************/
#ifndef MOTOR_ENCODER_INCLUDE_H
#define MOTOR_ENCODER_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif


//***************************************************************************
#include "MotorInclude.h"
#include "SystemDefine.h"
#include "MotorDefine.h"


//***************************************************************************
#define C_MAX_C_ZERO            200             //正余弦编码器Z信号到达时候C信号最大电平
#define C_MIN_D_ZERO            19000           //正余弦编码器Z信号到达时候D信号最小电平
#define C_UVW_FOR_ZERO          5
#define C_UVW_BACK_ZERO         6

#define DIR_FORWARD   1
#define DIR_BACKWARD  2
#define DIR_ERROR     0

//#define	RCK	(GpioDataRegs.GPBDAT.bit.GPIO34) // 与74HC594/165的配合
#define GetQepCnt()			(*EQepRegs).QPOSCNT
#define SetQepCnt(X)		    (*EQepRegs).QPOSCNT = X;

#define ACTIVE_HARDWARE_LOGICAL_U   1           
#define ACTIVE_HARDWARE_LOGICAL_V   1
#define ACTIVE_HARDWARE_LOGICAL_W   1           // UVW PG 卡硬件逻辑有一相反调了，软件在这里纠正

/***********************结构体定义***************************/
typedef enum PG_TYPE_ENUM_STRUCT_DEF{  
    PG_TYPE_ABZ,                //普通ABZ编码器
    PG_TYPE_UVW,                //带UVW信号的ABZ差分编码器
    PG_TYPE_RESOLVER,           //旋转变压器
    PG_TYPE_SC,                 //带正余弦信号的ABZ差分编码器 
    PG_TYPE_SPECIAL_UVW,        //省线方式UVW编码器
    PG_TYPE_NULL=100,           //没有接编码器
    PG_TYPE_RT                  // ??
}PG_TYPE_ENUM_STRUCT;       //编码器类型

typedef enum QEP_INDEX_ENUM_STRUCT_DEF{
    QEP_SELECT_1,               //选定外设QEP1用于测速
    QEP_SELECT_2,               //选定外设QEP2用于测速
    QEP_SELECT_PULSEIN,         //使用PULSE输入测速
    QEP_SELECT_NONE=100         //未选定用来测速的QEP模块
}QEP_INDEX_ENUM_STRUCT;

typedef struct PG_DATA_STRUCT_DEF {
	Uint 	PGType;
    QEP_INDEX_ENUM_STRUCT   QEPIndex;   //当前测速使用的QEP模块，要求在280xDSP的两个QEP模块之间切换

    Uint    PGMode;                     //区分增量式编码器和非增量式。0为增量式
    Uint    PGTypeGetFromFun;           //功能传递的PG卡类型
	Uint 	PulseNum;				    //编码器线数
    Uint    SpeedDir;                   //ABZ,UVW编码器代表AB信号，旋变代表旋变信号
    Uint    SpeedDirLast;               //
    Uint    PGDir;                      //辨识得到的编码器速度正反向
    Uint    PGErrorFlag;                //辨识得到的编码器错误信息 0-正常;1-未检测到编码器;2-编码器线数设定错误 

    Uint    imPgEstTick;                // im 编码器辨识计数器
    int     imDirAdder;
    long    imFreqErr;                  // 积分编码器测速误差
    long    imFrqEncoder;               // 
}PG_DATA_STRUCT;	            //编码器相关参数

typedef struct ROTOR_SPEED_SMOOTH_DEF {
    int     LastSpeed;
    int     SpeedMaxErr;
}ROTOR_SPEED_SMOOTH;

typedef struct FVC_SPEED_STRUCT_DEF {
//	int 	SpeedApply;			//实际使用转子速度
    //FVC
	int		Flag;
    Uint    TransRatio;         //电机测速传动比
    Uint    MTZeroCnt;
	int 	SpeedEncoder;		// 通过编码器检测到的转子频率, 传动比折算后
	int     SpeedTemp;          // 折算前编码器测速值
	//M法测速变量
	long 	MLastPos;			//前一次位置
	int 	MDetaPos;			//位置偏差
	int     MDetaPosBak;
	int 	MSpeed;				//M法测速值
	//T法测速变量
	Uint 	MTPulseNum;			//脉冲数N
	Ulong 	MTTime;				//N脉冲的时间
	Uint	MTCnt;
	int 	MTSpeed;			//MT法测速值
	Uint	MTLimitTime;		//
    Uint    IntCnt;
    Uint    IntNum;
    
    ROTOR_SPEED_SMOOTH  MSpeedSmooth;
    ROTOR_SPEED_SMOOTH  MTSpeedSmooth;
}FVC_SPEED_STRUCT;	//速度反馈部分数据结构

typedef struct IPM_UVW_PG_STRUCT_DEF {
	Uint	UVWData;
	Uint	UVWAngle;       // UVW信号绝对位置角度, 由于编码器和电机极对数相等，因此表示电角度
	
	Uint    U_Value;
	Uint    V_Value;
	Uint    W_Value;
	Uint	ZeroCnt;

    Uint    LogicU;         // U信号逻辑
    Uint    LogicV;         // V信号逻辑
    Uint    LogicW;         // W信号逻辑

    Uint	UvwDir;                 // UVW信号正反向

    Uint    debugdeta1;
    Uint    debugdeta2;
    
    Uint    lastAgl;
    Uint    NewAgl;
    Uint    ErrAgl;
    Uint    TuneFlag;
    
    llong   TotalErr;
    Ulong   UvwCnt;

    int     UvwZIntErr;      // Z中断uvw误差
    int     UvwZIntErr_deg;
    
    Uint    UvwZeroPos;     // uvw的零点位置角度
    Uint    UvwZeroPos_deg;

    Uint    UvwEstStep;
    int     UvwCnt2;        // 机械周期计数器
}IPM_UVW_PG_STRUCT;//永磁同步电机上UVW编码器的数据结构

typedef struct IPM_PG_DIR_STRUCT_DEF {
	int	    ABAngleBak;
    int     ABDirCnt;
    int     ABDir;
	int	    UVWAngleBak;
    int     UVWDirCnt;
    int     UVWDir;
	int	    CDAngleBak;
    int     CDDirCnt;
    int     CDDir;
    int     CDErr;
    int     RtPhaseBak;
    int     RtDirCnt;
}IPM_PG_DIR_STRUCT; //永磁同步电机上用于识别编码器接线方向的数据结构

typedef struct ROTOR_TRANS_STRUCT_DEF{
    Uint    RTPos;              // 通过旋变获取的电角度(Totor Transformer)
    Uint    RtorBuffer;         // 读取spi的buffer
    int     RealTimeSpeed;
    Ulong 	TimeBak;				//上一次测速的基准时间
    Ulong	DetaTime;
	Uint    PosBak;
	int		DetaPos;
	int		FreqFeed;
	Uint	Flag;
    Uint    Poles;  
    Uint    PolesRatio;         // 旋变极对数比例
    int     PosComp;
	int     ConFlag;
    Uint    SimuZBack;
    Uint    SimuZBack2;

    Uint    IntNum;
    Uint    IntCnt;

    Uint    RtRealPos;          // 旋变的时候角度，加了补偿角度之后

    Uint    AbsRotPos;          // 旋变绝对位置，0-4096
}ROTOR_TRANS_STRUCT; 	// 旋变数据结构


//***************************************************************************
extern struct EQEP_REGS        *EQepRegs;
extern IPM_UVW_PG_STRUCT        gUVWPG;
extern PG_DATA_STRUCT			gPGData;
extern IPM_PG_DIR_STRUCT        gPGDir;
extern BURR_FILTER_STRUCT		gSpeedFilter;
extern CUR_LINE_STRUCT_DEF		gSpeedLine;
extern ROTOR_TRANS_STRUCT		gRotorTrans;
extern FVC_SPEED_STRUCT		gFVCSpeed;

//*******************供外部引用函数声明***********************
extern void GetUvwPhase(void);
extern Uint JudgeABDir(void);
extern Uint JudgeUVWDir(void);
extern Uint JudgeRTDir(void);
extern void GetMTTimeNum(void);
extern void GetMDetaPos(void);
extern void RotorTransCalVel(void);
extern void GetRotorTransPos(void);
extern void RotorTransSamplePos(void);
extern void VCGetFeedBackSpeed(void);
extern void ReInitForPG(void);
extern void InitSetQEP(void);


#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

/*===========================================================================*/
// End of file.
/*===========================================================================*/

