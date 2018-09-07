#ifndef __F_IO_H__
#define __F_IO_H__


#include "f_funcCode.h"



//=====================================================================
// DI端子定义
//
// DI端子数量
#define DI_TERMINAL_NUMBER          (DI_NUMBER_PHSIC +     \
                                     DI_NUMBER_V +         \
                                     DI_NUMBER_AI_AS_DI)  
                                     
#if !DEBUG_F_POSITION_CTRL
#define DI_FUNC_NUMBER              60      // DI的功能选择数量
#elif 1
#define DI_FUNC_NUMBER              100     // DI的功能选择数量
#endif

#define DI_FUNC_NO_FUNC             0   // 无功能
#define DI_FUNC_ACC_DEC_TIME_SRC1   16  // 加减速时间选择端子
#define DI_FUNC_ACC_DEC_TIME_SRC2   17  // 加减速时间选择端子
#define DI_FUNC_COUNTER_TICKER_IN   25  // 计数器输入(DI5)
#define DI_FUNC_LENGTH_TICKER_IN    27  // 长度计数器输入(DI5)
#define DI_FUNC_FVC                 31  // 强制切换为FVC
#define DI_FUNC_APTP_ZERO           76  // aptp零点输入(DI5)

struct DI_FUNC1_BITS
{
    Uint16 noFunc:1;                // 0: 无功能
    Uint16 fwd:1;                   // 1: 正转运行FWD
    Uint16 rev:1;                   // 2: 反转运行REV
    Uint16 tripleLineCtrl:1;        // 3: 三线式运行控制
    Uint16 fwdJog:1;                // 4: 正向点动
    Uint16 revJog:1;                // 5: 反向点动
    Uint16 up:1;                    // 6: 端子UP
    Uint16 down:1;                  // 7: 端子DOWN

    Uint16 closePwm:1;              // 8:    自由停车，即封锁PWM输出
    Uint16 errorReset:1;            // 9:    故障复位
    Uint16 runPause:1;              // 10:   运行暂停
    Uint16 externalErrOpenIn:1;     // 11:   外部故障输入(常开)
    Uint16 multiSet:4;              // 15:12 多段指令端子4，3，2，1

    Uint16 accDecTimeSrc:2;         // 17:16 加减速时间选择端子2，1
    Uint16 frqSrcSwitch:1;          // 18    频率源切换
    Uint16 clearUpDownFrq:1;        // 19    UP/DOWN设定清零
    Uint16 localOrRemote:1;         // 20    运行命令切换端子，DI/comm<->panel
    Uint16 forbidAccDecSpd:1;       // 21    加减速禁止
    Uint16 pidPause:1;              // 22    PID暂停
    Uint16 resetPLC:1;              // 23    PLC状态复位
    
    Uint16 swingPause:1;            // 24    摆频暂停
    Uint16 counterTickerIn:1;       // 25    计数器输入(DI5)
    Uint16 resetCounter:1;          // 26    计数器复位
    Uint16 lengthTickerIn:1;        // 27    长度计数器输入(DI5)
    Uint16 resetLengthCounter:1;    // 28    长度计数器复位
    Uint16 forbidTorqueCtrl:1;      // 29    转矩控制禁止
    Uint16 pulseIn:1;               // 30    脉冲输入
    Uint16 motorCtrlMode2Fvc:1;     // 31    强制切换为FVC(保留)
};
union DI_FUNC1
{
    Uint32                  all;
    struct DI_FUNC1_BITS    bit;
};

struct DI_FUNC2_BITS
{
    Uint16 brake:1;                 // 32+0: 直流制动
    Uint16 externalErrCloseIn:1;    // 32+1: 外部故障常闭输入
    Uint16 frqOk:1;                 // 32+2: 频率设定起效端子
    Uint16 pidDirRev:1;             // 32+3: PID作用方向取反端子
    Uint16 stopPanel:1;             // 32+4: 外部停车端子，仅对面板控制有效
    Uint16 diOrComm:1;              // 32+5: 命令源切换端子2，DI<-->comm
    Uint16 pidPauseI:1;             // 32+6: PID积分暂停端子
    Uint16 frqXSrc2Preset:1;        // 32+7: 主频率源X与预置频率切换    
    Uint16 frqYSrc2Preset:1;        // 32+8: 辅频率源Y与预置频率切换
    Uint16 motorSnDi:2;             // 41,42: 电机选择端子。选择了该端子，电机选择功能码无效
    Uint16 pidChg:1;                // 43:  PID参数切换端子。0-PID1, 1-PID2
    Uint16 userError1:1;            // 44:  用户自定义故障1
    Uint16 userError2:1;            // 45:  用户自定义故障2
    Uint16 SpdTorqSwitch:1;         // 46:  速度控制/转矩控制切换
    Uint16 emergencyStop:1;         // 47:  紧急停车
    Uint16 stop4dec:1;              // 48: 外部端子停机(按减速时间4,任何时候有效)
    Uint16 decBrake:1;              // 49: 减速直流制动
    Uint16 clearSetRunTime:1;       // 50: 本次运行时间清零
    Uint16 rsvd2:13;
};
union DI_FUNC2
{
    Uint32                  all;
    struct DI_FUNC2_BITS    bit;
};

struct DI_FUNC
{
    union DI_FUNC1  f1;
    union DI_FUNC2  f2;
};

struct DI_STATUS_BITS
{
    Uint16 di1:1;       // 0
    Uint16 di2:1;       // 1
    Uint16 di3:1;       // 2
    Uint16 di4:1;       // 3 
    
    Uint16 di5:1;       // 4
    Uint16 di6:1;       // 5
    Uint16 di7:1;       // 6
    Uint16 di8:1;       // 7
    
    Uint16 di9:1;       // 8
    Uint16 di10:1;      // 9
    Uint16 vdi1:1;      // 10
    Uint16 vdi2:1;      // 11
     
    Uint16 vdi3:1;      // 12
    Uint16 vdi4:1;      // 13
    Uint16 vdi5:1;      // 14
    Uint16 ai1:1;       // 15
    
    Uint16 ai2:1;
    Uint16 ai3:1;
    Uint16 rsvd:14;
};

union DI_STATUS
{
    Uint32 all;
    struct DI_STATUS_BITS bit;
};

struct DI_STATUS_ALL
{
    union DI_STATUS a;          // 当前的DI端子状态，延时前
    union DI_STATUS b;          // 当前的DI端子状态，延时后，正反逻辑前
    union DI_STATUS c;          // 当前的DI端子状态，延时后，正反逻辑后
};


//======================================================================
// DO 输出选择
#define DO_TERMINAL_NUMBER           (DO_NUMBER_PHSIC + DO_NUMBER_V)  // DO端子数量

#define DO_FUNC_COMM_CTRL            20
#define DO_FUNC_NUMBER               42  // DO的功能选择数量

#if DSP_2803X
#define FUNCCODE_FM_OUT_SELECT_MAX   1
#else
#define FUNCCODE_FM_OUT_SELECT_MAX   2
#endif

struct DO_FUNC1_BITS
{
    Uint16 noFunc:1;                // 0: 无功能
    Uint16 run:1;                   // 1: 变频器运行中
    Uint16 error:1;                 // 2: 故障输出
    Uint16 frqFdtArrive:1;          // 3: 频率水平检测FDT到达
    Uint16 frqArrive:1;             // 4: 频率到达
    Uint16 zeroSpeedRun:1;          // 5: 零速运行中
    Uint16 motorPreOl:1;            // 6: 电机过载预报警
    Uint16 inverterPreOl:1;         // 7: 变频器过载预报警
    
    Uint16 counterSetArrive:1;      // 8:    设定计数脉冲值到达
    Uint16 counterPointArrive:1;    // 9:    指定计数脉冲值到达
    Uint16 lengthArrive:1;          // 10:   长度到达
    Uint16 plcEndLoop:1;            // 11    PLC循环完成
    Uint16 runTimeArrive:1;         // 12    运行时间到达
    Uint16 frqLimit:1;              // 13    频率限定中
    Uint16 torqueLimit:1;           // 14    转矩限定中
    Uint16 runReadyOk:1;            // 15    运行准备就绪
    
    Uint16 ai1GreaterThanAi2:1;     // 16    AI1 > AI2
    Uint16 upperFrqArrive:1;        // 17    上限频率到达
    Uint16 lowerFrqArrive:1;        // 18    下限频率到达
    Uint16 uv:1;                    // 19    欠压状态输出
    Uint16 commCtrl:1;              // 20    通讯控制
    Uint16 pcOk:1;                  // 21    定位完成
    Uint16 pcNear:1;                // 22    定位接近
    Uint16 zeroSpeedRun1:1;         // 23    零速运行中(停机有效)
    Uint16 powerUpTimeArrive:1;     // 24    上电时间到达
    Uint16 frqFdtArrive1:1;         // 25:   频率水平检测FDT1到达
    Uint16 frqArrive1:1;            // 26:   频率到达1
    Uint16 frqArrive2:1;            // 27:   频率到达2
    Uint16 currentArrive1:1;        // 28:   电流到达1
    Uint16 currentArrive2:1;        // 29:   电流到达2
    Uint16 setTimeArrive:1;         // 30:   定时到达
    Uint16 ai1limit:1;              // 31:   AI1输入超出上下限
};

union DO_FUNC1
{
    Uint32                  all;
    struct DO_FUNC1_BITS    bit;
};

struct DO_FUNC2_BITS
{
    Uint16 loseLoad:1;                // 32+0: 掉载中
    Uint16 speedDir:1;                // 32+1: 转速方向
    Uint16 oCurrent:1;                // 32+2: 电流检测到达输出
    Uint16 tempArrive:1;              // 32+3: 模块温度到达    
    Uint16 softOc:1;                  // 32+4: 软件过流输出
    Uint16 lowerFrqArrive:1;          // 32+5: 下限频率到达(与运行有关)
    Uint16 errorOnStop:1;             // 32+6: 故障输出(故障停机才输出)
    Uint16 motorForeOT:1;             // 32+7: 电机过温预报警
    Uint16 setRunTimeArrive:1;        // 32+8: 当前运行时间到达
    Uint16 errorOnNoUV;               // 32+9: 故障输出(欠压不输出)
    Uint16 rsvd0:6;                   // 
	Uint16 rsvd1:16;                  // 32+31
};

union DO_FUNC2
{
    Uint32                  all;
    struct DO_FUNC2_BITS    bit;
};

struct DO_FUNC
{
    union DO_FUNC1  f1;
    union DO_FUNC2  f2;
};

// DO数字输出端子状态字
struct DO_STATUS_BITS
{
// 与功能码顺序一致
    Uint16 do3:1;       // bit0-DO3
    Uint16 relay1:1;    // bit1-relay1
    Uint16 relay2:1;    // bit2-relay2
    Uint16 do1:1;       // bit3-DO1
    
    Uint16 do2:1;       // bit4-DO2
    Uint16 vdo1:1;      // bit5-VDO1
    Uint16 vdo2:1;      // bit6-VDO2
    Uint16 vdo3:1;      // bit7-VDO3
    
    Uint16 vdo4:1;      // bit8-VDO4
    Uint16 vdo5:1;      // bit9-VDO5
    
    Uint16 rsvd2:6;     // 保留
};

union DO_STATUS
{
    Uint16 all;
    struct DO_STATUS_BITS bit;
};

struct DO_STATUS_ALL
{
    union DO_STATUS a;          // 当前的DO端子状态，延时前
    union DO_STATUS b;          // 当前的DO端子状态，延时后，正反逻辑前
    union DO_STATUS c;          // 当前的DO端子状态，延时后，正反逻辑后
};

// DO数字输出端子状态字(硬件)
struct DO_HW_STATUS_BITS
{
// 与硬件一致
    Uint16 relay1:1;    // relay1
    Uint16 do2:1;       // DO2
    Uint16 do1:1;       // DO1
    Uint16 relay2:1;    // relay2

    Uint16 error:1;     // 故障指示灯
    Uint16 run:1;       // 运行指示灯
    Uint16 rsvd:1;      // 保留为空
    Uint16 fan:1;       // 风扇运行
};
union DO_HW_STATUS
{
    Uint16 all;
    struct DO_HW_STATUS_BITS bit;
};
extern union DO_HW_STATUS doHwStatus;
//======================================================================

struct DIDO_DELAY_STRUCT  // 考虑之后，可能1->0, 0->1都有可能
{
    Uint32 high;    // 0->1的延时
    Uint32 low;     // 1->0的延时
};
union DI_DELAY_TICKER_UNION
{
    struct DIDO_DELAY_STRUCT all[3];

    struct      // 要与功能码的顺序一致
    {
        struct DIDO_DELAY_STRUCT di1;
        struct DIDO_DELAY_STRUCT di2;
        struct DIDO_DELAY_STRUCT di3;
    } single;
};
union DO_DELAY_TICKER_UNION
{
    struct DIDO_DELAY_STRUCT all[DO_TERMINAL_NUMBER];

    struct      // 要与功能码的顺序一致
    {
        struct DIDO_DELAY_STRUCT do3;
        struct DIDO_DELAY_STRUCT relay1;
        struct DIDO_DELAY_STRUCT relay2;
        struct DIDO_DELAY_STRUCT do1;
        struct DIDO_DELAY_STRUCT do2;
    } single;
};

struct DI_HW_STATUS_BITS
{
    Uint16 di1:1;       // 0
    Uint16 di2:1;       // 1
    Uint16 di3:1;       // 2
    Uint16 di4:1;       // 3 
    
    Uint16 di5:1;       // 4
    Uint16 di6:1;       // 5
    Uint16 di7:1;       // 6
    Uint16 di8:1;       // 7
    
    Uint16 di9:1;       // 8
    Uint16 di10:1;      // 9
};

union DI_HW_STATUS
{
    Uint32 all;
    struct DI_HW_STATUS_BITS bit;
};

extern union DI_HW_STATUS diHwStatus;

//======================================================================
// FMP, AO
#define AOFMP_FMP   0
#define AOFMP_AO1   1
#define AOFMP_AO2   2
//======================================================================


//======================================================================
// AO,FMP输出选择
#define AO_FMP_FUNC_NUMBER          13  // AO设定的个数
#define AO_FMP_FUNC_FRQ_SET         0   // 运行频率
#define AO_FMP_FUNC_FRQ_AIM         1   // 设定频率
#define AO_FMP_FUNC_OUT_CURRENT     2   // 输出电流
#define AO_FMP_FUNC_OUT_TORQUE      3   // 输出转矩(绝对值)
#define AO_FMP_FUNC_OUT_POWER       4   // 输出功率
#define AO_FMP_FUNC_OUT_VOLTAGE     5   // 输出电压
#define AO_FMP_FUNC_PULSE_IN        6   // PULSE脉冲输入
#define AO_FMP_FUNC_AI1             7   // AI1
#define AO_FMP_FUNC_AI2             8   // AI2
#define AO_FMP_FUNC_AI3             9   // AI3(扩展卡)
#define AO_FMP_FUNC_LENGTH          10   // 长度
#define AO_FMP_FUNC_COUNTER         11   // 计数值
#define AO_FMP_FUNC_COMM            12   // 通讯控制输出
#define AO_FMP_FUNC_SPEED           13   // 输出转速
#define AO_FMP_FUNC_OUT_CURRENT_1   14   // 输出电流  对应0~1000A
#define AO_FMP_FUNC_OUT_VOLTAGE_1   15   // 输出电压  对应0~1000V
#define AO_FMP_FUNC_OUT_TORQUE_DIR  16   // 输出转矩(带正负)
//======================================================================


// 两点(x1,y1), (x2,y2), 求第三点(x, y)的y
// 请确保: x1 < x2
typedef struct
{
    void (*calc)(void *);     // Pointer to calculation functon
    
    int16 mode;               // 1，表示x和y不限幅；0，限幅

    int32 x1;
    int32 y1;                 // (x1,y1)
    int32 x2;
    int32 y2;                 // (x2,y2)

    int32 x;                  // 需要求解点(x,y)的x, 输入
    int32 y;                  // 需要求解点(x,y)的y, 输出
} LINE_STRUCT;


void LineCalc(LINE_STRUCT *p);

#define LINE_STRTUCT_DEFALUTS       \
{                                   \
    (void (*)(void *))LineCalc      \
}

typedef struct
{
    void (*calc)(void *);         // Pointer to calculation functon

    int32 t;                      // 滤波时间

    int32 in;                     // 输入
    int32 out;                    // 输出

    int32 outOld;                 // 上一次的输出
    int32 remainder;              // 运算过程中的余数
} LowPassFilter;

void LpfCalc(LowPassFilter *p);

#define LPF_DEFALUTS            \
{                               \
    (void (*)(void *))LpfCalc   \
}


//=====================================================================
// ECAP
// ECCTL1 ( ECAP Control Reg 1)
//==========================
// CAPxPOL bits
#define   EC_RISING       0x0
#define   EC_FALLING      0x1
// CTRRSTx bits
#define   EC_ABS_MODE     0x0
#define   EC_DELTA_MODE   0x1
// PRESCALE bits
#define   EC_BYPASS       0x0
#define   EC_DIV1         0x0
#define   EC_DIV2         0x1
#define   EC_DIV4         0x2
#define   EC_DIV6         0x3
#define   EC_DIV8         0x4
#define   EC_DIV10        0x5
// ECCTL2 ( ECAP Control Reg 2)
//==========================
// CONT/ONESHOT bit
#define   EC_CONTINUOUS   0x0
#define   EC_ONESHOT      0x1
// STOPVALUE bit
#define   EC_EVENT1       0x0
#define   EC_EVENT2       0x1
#define   EC_EVENT3       0x2
#define   EC_EVENT4       0x3
// RE-ARM bit
#define   EC_ARM          0x1
// TSCTRSTOP bit
#define   EC_FREEZE       0x0
#define   EC_RUN          0x1
// SYNCO_SEL bit
#define   EC_SYNCIN       0x0
#define   EC_CTR_PRD      0x1
#define   EC_SYNCO_DIS    0x2
// CAP/APWM mode bit
#define   EC_CAP_MODE     0x0
#define   EC_APWM_MODE    0x1
// APWMPOL bit
#define   EC_ACTV_HI      0x0
#define   EC_ACTV_LO      0x1
// Generic
#define   EC_DISABLE      0x0
#define   EC_ENABLE       0x1
#define   EC_FORCE        0x1
//=====================================================================

extern struct DI_FUNC diFunc;
extern struct DI_FUNC diSelectFunc;
extern struct DI_STATUS_ALL diStatus;


extern struct DO_FUNC doFunc;
extern struct DO_STATUS_ALL doStatus;


extern Uint32 pulseInFrq;
extern int16 pulseInSet;
extern Uint16 lineSpeed;

//extern Uint16 fanControl;

struct AI_DEAL
{
    Uint16 sample;          // 采样值，Q16
    
    int16 voltageOrigin;    // 校正前电压
    int16 voltage;          // 校正后电压
    
    int16 set;              // AI设定
};
extern struct AI_DEAL aiDeal[];


void InitSetEcap4(void);
void InitSetEcap2(void);
void InitSetEcap1();

void DiCalc(void);
void DoCalc(void);
void PulseInCalc(void);
void PulseInSample(void);
void FMPDeal(void);
void AiCalc(void);
void AoCalcChannel(Uint16);

void InitDIGpio(void);
void InitDOGpio(void);
void InitECap3Gpio(void);
void InitSetEcap3(void);
void InitECap4Gpio(void);

void InitSetEPWM4(void);
void InitSetEPWM5(void);
void InitSetEPWM6(void);

void InitEPwm4Gpio(void);
void InitEPwm5Gpio(void);
void InitEPwm6Gpio(void);



#endif // __F_IO_H__


