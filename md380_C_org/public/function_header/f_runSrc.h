#ifndef __F_COMMAND_H__
#define __F_COMMAND_H__

#include "f_funcCode.h"



// 加减速
#define CONST_SPEED     0       // 恒速
#define ACC_SPEED       1       // 加速
#define DEC_SPEED       2       // 减速


//=====================================================================
// runCmd，运行命令字
//
#define FORWARD_DIR         0   // 正方向
#define REVERSE_DIR         1   // 反方向

#define RUN_CMD_NO_JOG      0   // 无点动命令
#define RUN_CMD_FWD_JOG     1   // 正向点动
#define RUN_CMD_REV_JOG     2   // 反向点动
#define RUN_CMD_FWD_REV_JOG 3   // 既有正向点动命令，又有反向点动命令
struct RUN_CMD_BITS
{                               // bits  description
    Uint16 common0:1;           // 0     +, 普通运行命令(非点动、调谐)，中间值，且保留下次使用
    Uint16 common:1;            // 1     -, 逻辑处理之后的普通运行命令(非点动)
    Uint16 jog:2;               // 3:2   -, 0-no jog, 1-jog
    Uint16 rsvd1:2;             // 5:4   

    Uint16 dir:1;               // 6     +, 0-fwd, 1-rev. 表示运行方向，不包括点动方向

    Uint16 pause:1;             // 7     -, 运行暂停
    
    Uint16 freeStop:1;          // 8     -,
    Uint16 hurryStop:1;         // 9     ?, 目前暂未使用

    Uint16 otherStop:1;         // 10    +, 其他情况的停机/非默认停机，强制保护
    Uint16 startProtect:1;      // 11    +, 启动保护

    Uint16 errorReset:1;        // 12    -, 故障复位

    Uint16 rsvd:3;              // 14:13
};

union RUN_CMD
{
    Uint16              all;
    struct RUN_CMD_BITS bit;
};

extern union RUN_CMD runCmd;

//=====================================================================
// 变频器运行状态信息
//=====================================================================
struct INVT_STATUS_BITS
{
    Uint16 run:2;           // 0;停机  1;正转   2;反转
    Uint16 accDecStatus:2;  // 0;恒速  1;加速   2;减速
    Uint16 uv:1;            // 0;正常  1;欠压
};

union INVT_STATUS
{
    Uint16               all;
    struct INVT_STATUS_BITS bit;
};
extern union INVT_STATUS invtStatus;

//=====================================================================
// runFlag, 变频器运行过程中的状态字
//
struct RUN_FLAG_BITS
{                               // bits  description
    Uint16 run:1;               // 0    (总的)运行标志
    
    Uint16 common:1;            // 1    普通运行(非点动、非调谐)
    Uint16 jog:1;               // 2    点动运行
    Uint16 tune:1;              // 3    调谐运行

    Uint16 jogWhenRun:1;        // 4    运行中点动
    
    Uint16 accDecStatus:2;      // 6:5  0 恒速； 1 加速； 2 减速


// 之下的bit位在shutdown时不要清除
    Uint16 plc:1;               // 7     PLC运行
    Uint16 pid:1;               // 8     PID运行
    Uint16 torque:1;            // 9     转矩控制
    
    Uint16 dir:1;               // 10    设定频率方向(功能码F0-12运行方向之前), 0-fwd, 1-rev
    Uint16 curDir:1;            // 11    当前运行频率方向, 0-fwd, 1-rev
    Uint16 dirReversing:1;      // 12    正在反向标志, 0-当前没有反向, 1-正在反向
    Uint16 dirFinal:1;          // 13    设定频率方向(功能码F0-12运行方向之后), 0-fwd, 1-rev

    Uint16 servo:1;             // 14
    Uint16 rsvd:1;              // 15
};


union RUN_FLAG
{
    Uint16               all;
    struct RUN_FLAG_BITS bit;
};
extern union RUN_FLAG runFlag;

struct RUN_STATUS_FIRST_STEP_BITS
{
    Uint16 accStep:1;          // 加速运行第一拍
    Uint16 invarianceStep:1;   // 恒速运行第一排
    Uint16 decStep:1;          // 减速运行第一拍
};

union RUN_STATUS_FIRST_STEP
{
    Uint16 all;
    struct RUN_STATUS_FIRST_STEP_BITS bit;
};

extern union RUN_STATUS_FIRST_STEP runStatus1Step;

//=====================================================================


//=====================================================================
// dspMainCmd, 转递给性能的主命令字
//
struct DSP_MAIN_COMMAND_BITS
{                               // bits  description
    Uint16 run:1;               // 0,    0:stop, 1:run
    Uint16 startBrake:1;        // 1,    start brake
    Uint16 stopBrake:1;         // 2,    stop brake
    Uint16 motorCtrlMode:2;     // 4:3   00-SVC, 01-VC, 10-VF
    Uint16 startFlux:1;         // 5:    预励磁
    Uint16 torqueCtrl:1;        // 6     转矩控制
    Uint16 accDecStatus:2;      // 8:7   0 恒速； 1 加速； 2 减速. //! 目前转差补偿使用了该标志
    Uint16 spdLoopI1:1;         // 9     速度环积分分离
};

union DSP_MAIN_COMMAND
{
    Uint16                       all;
    struct DSP_MAIN_COMMAND_BITS bit;
};

extern union DSP_MAIN_COMMAND dspMainCmd;
//=====================================================================


//=====================================================================
// dspMainCmd1, 转递给性能的主命令字1，运行中不可更改
//
struct DSP_MAIN_COMMAND1_BITS
{                               // bits  description
    Uint16 pgLocation:2;        // 1:0,  速度反馈PG选择. 0：本地PG，1：扩展PG，2：PULSE脉冲输入（DI5）
    Uint16 fvcPgLogic:1;        // 2,    速度反馈PG卡，AB相序. 1-B超前A.
    Uint16 deadCompMode:3;      // 5:3,  死区补偿模式选择. 0－死区补偿模式0；1－死区补偿模式1
    Uint16 modulationMode:1;    // 6     调制方式，0-异步调制，1-同步调制
    Uint16 rsvd:1;              // 7     保留
    Uint16 rsvd1:1;             // 8     保留
    Uint16 frqPoint:2;          // 10:9  频率指令小数点。0: 1Hz；1：0.1Hz；2：0.01Hz
    Uint16 speedTrack:2;        // 12:11 0－转速跟踪无效，1－转速跟踪模式1，2－转速跟踪模式2
    Uint16 shortGnd:1;          // 13    上电对地短路检测标志
};

union DSP_MAIN_COMMAND1
{
    Uint16                       all;
    struct DSP_MAIN_COMMAND1_BITS bit;
};

extern union DSP_MAIN_COMMAND1 dspMainCmd1;
//=====================================================================


//=====================================================================
// dspSubCmd, 转递给性能的辅命令字
//
struct DSP_SUB_COMMAND_BITS
{                                       // bits  description
    Uint16 errorDealing:1;              // 0,    1:ERROR TALK, 功能正在进行故障处理
    Uint16 outPhaseLossProtect:1;       // 1,    输出缺相保护
    Uint16 inPhaseLossProtect:1;        // 2,    输入缺相保护
    Uint16 overloadMode:1;              // 3,    电机过载保护使能
    Uint16 loseLoadProtectMode:1;       // 4,    输出掉载保护使能标志
    Uint16 poffTransitoryNoStop:1;      // 5,    瞬停不停使能
    Uint16 cbc:1;                       // 6,    逐波限流功能使能标志
    Uint16 varFcByTem:1;                // 7    载波频率随温度调整
    Uint16 fanRunWhenWaitStopBrake:1;   // 8    停机直流制动等待时间内风扇运行标志
    Uint16 contactorMode:1;             // 9    接触器吸合保护
};

union DSP_SUB_COMMAND
{
    Uint16                      all;
    struct DSP_SUB_COMMAND_BITS bit;
};

extern union DSP_SUB_COMMAND dspSubCmd;
//=====================================================================



//=====================================================================
// 性能传递给功能的状态字
//
struct DSP_STATUS_BITS
{                                   // bits  description
    Uint16 runEnable:1;		        // 0     1-初始化完成，可以运行标志
    Uint16 uv:1;                    // 1     母线电压欠压故障标志  0-欠压  1-不欠压
    Uint16 run:1;                   // 2     运行/停机状态标志
    Uint16 rsvd:1;                  // 3
    Uint16 speedTrackEnd:1;         // 4     转速跟踪结束标志
    Uint16 inverterPreOl:1;         // 5     变频器过载预报警标志
    Uint16 motorPreOl:1;            // 6     电机过载预报警标志
    Uint16 fan:1;                   // 7     风扇运行标志，主轴伺服使用；其他保留
    Uint16 outAirSwitchOff:1;       // 8     变频器输出空开断开标志，即掉载标志
};

union DSP_STATUS 
{
   Uint16                   all;
   struct DSP_STATUS_BITS   bit;
};

extern union DSP_STATUS dspStatus;
//-----------------------------------------------------


//=====================================================================
// 性能传递给功能的辅助状态字
//
struct DSP_SUB_STATUS_BITS
{                                   // bits  description
    Uint16 accDecStatus:2;          // 1:0   0 恒速； 1 加速； 2 减速
    Uint16 rsvd:14;                 // 15:2  保留
};

union DSP_SUB_STATUS 
{
   Uint16                       all;
   struct DSP_SUB_STATUS_BITS   bit;
};

extern union DSP_SUB_STATUS dspSubStatus;
//-----------------------------------------------------




// 故障时继续运行频率选择
#define ERR_RUN_FRQ_RUN       0  // 以当前运行频率运行
#define ERR_RUN_FRQ_AIM       1  // 以设定频率运行
#define ERR_RUN_FRQ_UPPER     2  // 以上限频率运行
#define ERR_RUN_FRQ_LOWER     3  // 以下限频率运行
#define ERR_RUN_FRQ_SECOND    4  // 以异常时备用频率运行

//-----------------------------------------------------
// runStatus，当前运行状态/步骤
//
enum RUN_STATUS
{
    RUN_STATUS_WAIT,        // 等待启动
    RUN_STATUS_ZERO,        // 零频运行
    RUN_STATUS_START,       // 启动
    RUN_STATUS_NORMAL,      // (正常)运行
    RUN_STATUS_STOP,        // 停机
    RUN_STATUS_JOG,         // 点动运行

    RUN_STATUS_POS_CTRL,    // 位置控制
    
    RUN_STATUS_TUNE,        // 调谐运行
    RUN_STATUS_DI_BRAKE_DEC,  // DI端子直流制动频率减速
    RUN_STATUS_DI_BRAKE,      // DI端子的直流制动(非启动直流制动和停机直流制动)
    RUN_STATUS_LOSE_LOAD,     // 掉载运行
    RUN_STATUS_SHUT_DOWN      // shut down, 关断
};
extern enum RUN_STATUS runStatus;
//-----------------------------------------------------

//-----------------------------------------------------
enum START_RUN_STATUS
{
    START_RUN_STATUS_SPEED_TRACK,       // 转速跟踪
    START_RUN_STATUS_BRAKE,             // 启动直流制动
    START_RUN_STATUS_PRE_FLUX,          // 预励磁启动
    START_RUN_STATUS_HOLD_START_FRQ     // 启动频率保持
};
#define START_RUN_STATUS_INIT           START_RUN_STATUS_SPEED_TRACK
extern enum START_RUN_STATUS startRunStatus;
//-----------------------------------------------------

//-----------------------------------------------------
enum STOP_RUN_STATUS
{
    STOP_RUN_STATUS_DEC_STOP,           // 减速停车
    STOP_RUN_STATUS_WAIT_BRAKE,         // 停机直流制动等待
    STOP_RUN_STATUS_BRAKE               // 停机直流制动
};
#define STOP_RUN_STATUS_INIT            STOP_RUN_STATUS_DEC_STOP
extern enum STOP_RUN_STATUS stopRunStatus;
//-----------------------------------------------------

//-----------------------------------------------------
// 直线变化的计算：加减速，PID给定的计算
// 已知从 0 到 最大值maxValue 的变化时间为 tickerAll，
// 每次新的计算，remainder应该清零，但是影响很小。
//
typedef struct
{
    void (*calc)(void *);       // 函数指针

    int32 maxValue;             // 最大值
    int32 aimValue;             // 目标值
    int32 curValue;             // 当前值

    Uint32 tickerAll;           // 从0到最大值的ticker
    int32 remainder;            // 计算delta的余值
} LINE_CHANGE_STRUCT;

#define LINE_CHANGE_STRTUCT_DEFALUTS       \
{                                          \
    (void (*)(void *))LineChangeCalc       \
}

//-----------------------------------------------------

extern Uint32 accFrqTime;
extern Uint32 decFrqTime;
extern Uint16 startProtectSrc;
extern Uint16 otherStopSrc;

extern Uint16 runSrc;

extern Uint16 bDiAccDecTime;
extern Uint16 accDecTimeSrcPlc;

extern Uint16 runDirPanelOld;
extern Uint16 shuntFlag;
extern Uint16 setRunLostTime;
extern Uint16 setRunTimeAim;  // 设定定时运行时间;
extern Uint16 swingFrqLimit;  // 摆频限定中

enum SWING_STATUS
{
    SWING_NONE,             // 摆频等待，即没有摆频
    SWING_UP,               // 摆频的上升阶段
    SWING_DOWN              // 摆频的下降阶段
};
extern enum SWING_STATUS swingStatus;

extern Uint16 tuneCmd;
extern int32 accel;
extern Uint16 accelDisp;


extern Uint16 pcFdbDisp;


extern int32 frqCurAimOld;
extern int32 accTimeOld;
extern int32 decTimeOld;

extern LINE_CHANGE_STRUCT frqLine;


// function
void UpdateRunCmd(void);
void RunSrcDeal(void);
void AccDecTimeCalc(void);
void LineChangeCalc(LINE_CHANGE_STRUCT *p);
void LineFrqChangeCalc(LINE_CHANGE_STRUCT *p);

void AccDecFrqCalc(int32 accTime, int32 decTime, Uint16 mode);

void setTimeRun(void);
void runTimeCal(void);

extern int16 upperTorque;
extern int16 torqueAim;
void TorqueCalc(void);
Uint16 TorqueLimitCalc(void);

#endif  // __F_COMMAND_H__








