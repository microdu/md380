#ifndef __F_FRQSRC_H__
#define __F_FRQSRC_H__


#include "f_funcCode.h"

#define ACC_DEC_FRQ_NONE    0
#define ACC_DEC_FRQ_WAIT    1
#define ACC_DEC_FRQ_DONE    2



typedef struct
{
// input
    Uint16 frq;     // 跳跃频率
    Uint16 range;   // 跳跃频率幅度

// output
    Uint32 low;     // 跳跃频率范围的low
    Uint32 high;    // 跳跃频率范围的high
} JUMP_FRQ;


// 功能部分使用的PID
typedef struct
{
    void  (*calc)(void *);  // Pointer to calculation function

    int32  ref;             // PID给定，Q15
    int32  fdb;             // PID反馈，Q15

    int32  Kp;              // 比例增益
    int32  Ki;              // 积分增益
    int32  Kd;              // 微分增益

    int32  Kp2;             // 第2参数的P
    int32  Ki2;             // 第2参数
    int32  Kd2;             // 第2参数

    int16  Qp;              // Kp的Q格式
    int16  Qi;              // Ki的Q格式
    int16  Qd;              // Kd的Q格式

    int32  deltaMax;        // Q15, 两次输出之间偏差的最大值
    int32  deltaMin;        // Q15, 两次输出之间偏差的最大值
    int32  outMax;          // Q15，PID输出最大值
    int32  outMin;          // Q15，PID输出最小值
    int32  pidDLimit;       // Q15, PID微分限幅
    int32  errorDead;       // 小于偏差极限，则PID不调节, Q15
    int32  errorSmall;      // 变积分使用
    int32  errorBig;        // 变积分使用

    int32  error;           // PID偏差，Q15, 注意可能超过0xffff
    int32  error1;          // 
    int32  error2;

    int32  delta;           // 两次输出之间的差值

    int32  deltaPRem;       // P计算的Remainder
    int64  deltaIRem;       // I计算的Remainder
    int32  deltaDRem;       // D计算的Remainder
    int32  deltaRemainder;  // 两次输出之间限幅之后的余值，保留下次计算使用

    int32  out;             // Output: PID output, Q15
    Uint16 sampleTime;      // PID采样周期
    Uint16 sampleTcnt;      // PID采样周期计时
} PID_FUNC;


#define PID_FUNC_DEFAULTS         \
{                                 \
    (void (*)(void *))PidFuncCalc \
}



extern PID_FUNC pidFunc;

extern int32 frq;
extern int32 frqTmp;
extern int32 frqFrac;
extern int32 frqTmpFrac;
extern int32 frqCurAimFrac;
extern int32 frqDroop; 
extern int32 frqAim;
extern int32 frqAiPu;
extern int32 frqAimTmp;
extern int32 frqAimTmp0;
extern int32 frqCurAim;
extern Uint16 frqKeyUpDownDelta;
extern Uint16 upperFrq;
extern Uint16 lowerFrq;
extern Uint16 maxFrq;
extern Uint16 benchFrq;
extern Uint16 frqPuQ15;
extern int32 uPDownFrqMax;
extern int32 uPDownFrqMin;
extern Uint16 upDownFrqInit;
extern Uint16 plcStep;
extern Uint32 plcTime;
extern Uint16 bPlcEndOneLoop;
extern Uint16 plcStepRemOld;
extern Uint32 plcTimeRemOld;

extern Uint16 timeBench;

extern int32 frqDigitalTmp;
extern Uint16 bFrqDigital;

extern Uint16 frqCalcSrcOld;

extern Uint16 bStopPlc;
extern Uint16 bAntiReverseRun;

extern int32 frqAimOld4Dir;

#define RUN_MODE_SPEED_CTRL     0   // 速度控制
#define RUN_MODE_TORQUE_CTRL    1   // 转矩控制
#define RUN_MODE_POSITION_CTRL  2   // 位置控制
extern Uint16 runMode; // 运行模式

#define UP_DN_OPERATION_ON          1
#define UP_DN_OPERATION_OFF         0
struct FRQ_FLAG_BITS
{
    Uint16 comp:1;          // 0:   运算
    Uint16 upDown:1;        // 数字设定，UP/DOWN可修改
    Uint16 x:1;             // 1:   X
    Uint16 y:1;             // 2:   Y
    
    Uint16 fcPosLimit:1;    // 3:   有数字设定，且正向限幅
    Uint16 fcNegLimit:1;    // 4:   有数字设定，且正向限幅
    
    Uint16 upDownoperationStatus:1;        // 5:   本次有UP/DN操作标志(防止上限频率变大后,UP/DN因为被限幅，无法随着变化)

    Uint16 frqSetLimit:1;                    // 设定频率被限制
};
union FRQ_FLAG
{
    Uint16 all;
    struct FRQ_FLAG_BITS bit;
};

extern union FRQ_FLAG frqFlag;

struct FRQ_XY
{
    int32 x;    // 主频率X
    int32 y;    // 辅频率Y
    int32 z;    // 辅频率Y偏置
};

extern struct FRQ_XY frqXy; 

void FrqSrcDeal(void);
void UpdateFrqAim(void);
void PidFuncCalc(PID_FUNC *);

extern Uint16 dpFrqAim;
extern Uint16 frqCalcSrc;
extern Uint16 plcStepOld;
int32 FrqPidSetDeal(void);
int32 UpdateMultiSetFrq(Uint16 step);
void ResetUpDownFrq(void);

extern int32 upDownFrq;
extern int32 upDownFrqTmp;

#endif // __F_FRQSRC_H__





