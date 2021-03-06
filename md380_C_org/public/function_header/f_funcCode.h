/*****************************************************************
 *
 * 功能码定义的头文件
 * 
 * Time-stamp: <2012-08-20 08:57:13  Shisheng.Zhi, 0354>
 * 
 *
 *
 *
 *****************************************************************/

#ifndef __F_FUNCCODE_H__
#define __F_FUNCCODE_H__



       // 2803x还是2808平台
#include "DSP28x_Project.h"

#include "f_debug.h"
#include "f_interface.h"
#include "f_p2p.h"




// 请不要随意修改这两个值，否则EEPROM中的值会全部(包括P0组、P1组、各种记录)恢复出厂值。
#define EEPROM_CHECK1       (0x0009)                        // EEPROM校验字1
#define EEPROM_CHECK2       (0xFFFF - EEPROM_CHECK1)        // EEPROM校验字2


#define USER_MENU_GROUP     FUNCCODE_GROUP_FE

// 获得功能码code在全部功能码中的index，即功能码在数组funcCode.all[]中的下标。
// 根据 FUNCCODE_ALL 的定义，一个功能码的表示有4种办法，对应的获得index的办法：
// 1. funcCode.all[i]     ---- i
// 2. funcCode.f5[7]      ---- GetCodeIndex(funcCode.group.f5[7])
// 3. group, grade        ---- GetGradeIndex(group, grade)
// 4. funcCode.code.maxFrq---- GetCodeIndex(funcCode.code.maxFrq)
#define GetCodeIndex(code)    ((Uint16)((&(code)) - (&(funcCode.all[0]))))
#define GetGradeIndex(group, grade)  (funcCodeGradeSum[group] + (grade))


//=====================================================================
// 功能码属性位定义
//
// 单位
#define ATTRIBUTE_UNIT_HZ_BIT           0
#define ATTRIBUTE_UNIT_A_BIT            1
#define ATTRIBUTE_UNIT_V_BIT            2

// 读写属性
#define ATTRIBUTE_READ_AND_WRITE        0   // (任何时候)可写
#define ATTRIBUTE_READ_ONLY_WHEN_RUN    1   // 运行时只读
#define ATTRIBUTE_READ_ONLY_ANYTIME     2   // 只读

// 多功能码组合属性
#define ATTRIBUTE_MULTI_LIMIT_SINGLE    0   // 单独的功能码
#define ATTRIBUTE_MULTI_LIMIT_DEC       1   // 多个功能码，十进制
#define ATTRIBUTE_MULTI_LIMIT_HEX       2   // 多个功能码，十六进制

struct  FUNC_ATTRIBUTE_BITS
{                           // bits   description
    Uint16 point:3;         // 2:0    radix point,小数点。
                            //        0-无小数点，1-1位小数，...，4-4位小数
                            //        (0.0000-100,00.000-011,000.00-010,0000.0-001,00000-000)
    Uint16 unit:3;          // 5:3    unit,单位
                            //        1-hz, 2-A, 3-RPM, 4-V, 6-%; 001-Hz, 010-A, 100-V
    Uint16 displayBits:3;   // 8:6    5个数码管要显示的位数。0-显示0位，1-显示1位，...，5-显示5位
    Uint16 upperLimit:1;    // 9      1-参数由上限相关功能码限制，0-直接由上限限制
    Uint16 lowerLimit:1;    // 10     1-参数由下限相关功能码限制，0-直接由下限限制
    Uint16 writable:2;      // 12:11  参数读写特性，00-可以读写, 01-运行中只读，10-参数只读
    Uint16 signal:1;        // 13     符号，unsignal-0; signal-1
    Uint16 multiLimit:2;    // 15:14  该功能码为多个功能码的组合. 
                            //        00-单独功能码(非组合); 
                            //        01-十进制,  多个功能码的组合; 
                            //        10-十六进制,多个功能码的组合; 
};

union FUNC_ATTRIBUTE
{
   Uint16                      all;
   struct FUNC_ATTRIBUTE_BITS  bit;
};
//=====================================================================



//=====================================================================
// 功能码属性表：上限、下限、属性
// 功能码的出厂值，包括EEPROM_CHECK、掉电记忆，但不包括显示组
typedef struct FUNCCODE_ATTRIBUTE_STRUCT
{
    Uint16                  lower;          // 下限
    Uint16                  upper;          // 上限
    Uint16                  init;           // 出厂值
    union FUNC_ATTRIBUTE    attribute;      // 属性

    Uint16                  eepromIndex;    // 对应EEPROM存储的index
} FUNCCODE_ATTRIBUTE;

extern const FUNCCODE_ATTRIBUTE funcCodeAttribute[];
//=====================================================================



//=====================================================================
// 功能码的code的一些数据结构定义
struct PLC_STRUCT
{
    Uint16 runTime;         // PLC第_段运行时间
    Uint16 accDecTimeSet;   // PLC第_段加减速时间选择
};
//=================================


//=================================
struct AI_SET_CURVE  // AI设定曲线
{
    Uint16 minIn;       // 曲线最小输入(电压值)
    Uint16 minInSet;    // 曲线最小输入对应设定(百分比)
    Uint16 maxIn;       // 曲线最大输入(电压值)
    Uint16 maxInSet;    // 曲线最大输入对应设定(百分比)
};
//=================================


//=================================
struct AI_JUMP
{
    Uint16 point;   // 设定跳跃点
    Uint16 arrange; // 设定跳跃幅度
};
//=================================


//=================================
struct ANALOG_CALIBRATE_CURVE  // 模拟量校正曲线，AIAO
{
    Uint16 before1;     // 校正前电压1
    Uint16 after1;      // 校正后电压1
    
    Uint16 before2;     // 校正前电压2
    Uint16 after2;      // 校正后电压2
};
//=================================


//=================================
typedef struct AO_PARA_STRUCT
{
    Uint16 offset;          // AO零偏系数
    Uint16 gain;            // AO增益
} AO_PARA;
//=================================


//=================================
struct FC_GROUP_HIDE_STRUCT
{
    Uint16 password;    // 功能码组隐藏特性密码
    
    Uint16 f;           // F组隐藏特性（F0-FF）
    Uint16 a;           // A组隐藏特性（A0-AF）
    Uint16 b;           // B组隐藏特性（B0-BF）
    Uint16 c;           // C组隐藏特性（C0-CF）
};
union FC_GROUP_HIDE
{
    Uint16 all[5];
    struct FC_GROUP_HIDE_STRUCT elem;
};
//=================================

//=================================

//=================================
struct ERROR_SCENE_STRUCT
{
    Uint16 errorFrq;                    // 第三次(最近一次)故障时频率
    Uint16 errorCurrent;                // 第三次(最近一次)故障时电流
    Uint16 errorGeneratrixVoltage;      // 第三次(最近一次)故障时母线电压
    Uint16 errorDiStatus;               // 第三次(最近一次)故障时输入端子状态
    Uint16 errorDoStatus;               // 第三次(最近一次)故障时输出端子状态
    
    Uint16 errorInverterStatus;         // 第三次(最近一次)故障时变频器状态
    Uint16 errorTimeFromPowerUp;        // 第三次(最近一次)故障时时间（从本次上电开始计时）
    Uint16 errorTimeFromRun;            // 第三次(最近一次)故障时时间（从运行时开始计时）
};

union ERROR_SCENE
{
    Uint16 all[sizeof(struct ERROR_SCENE_STRUCT)];

    struct ERROR_SCENE_STRUCT elem;
};
//=================================


//=================================
#define APTP_NUM    32  // aptp点数
typedef struct
{
    Uint16 low;     // aptp低位，0-9999
    Uint16 high;    // aptp高位，0-65535
} LENGTH_SET;
// 范围: 0-655359999
//=================================


//=================================
#define MOTOR_TYPE_ACI_GENERAL  0   // 普通异步电机
#define MOTOR_TYPE_ACI_INV      1   // 变频异步电机
#define MOTOR_TYPE_PMSM         2   // 永磁同步电机
struct MOTOR_PARA_STRUCT
{
    // 电机基本参数
    Uint16 motorType;               // F1-00  电机类型选择
    Uint16 ratingPower;             // F1-01  电机额定功率
    Uint16 ratingVoltage;           // F1-02  电机额定电压
    Uint16 ratingCurrent;           // F1-03  电机额定电流
    Uint16 ratingFrq;               // F1-04  电机额定频率
    Uint16 ratingSpeed;             // F1-05  电机额定转速

    // 异步机调谐参数
    Uint16 statorResistance;        // F1-06  异步机定子电阻
    Uint16 rotorResistance;         // F1-07  异步机转子电阻
    Uint16 leakInductance;          // F1-08  异步机漏感抗
    Uint16 mutualInductance;        // F1-09  异步机互感抗
    Uint16 zeroLoadCurrent;         // F1-10  异步机空载电流
    Uint16 rsvdF11[5];

    // 同步机调谐参数
    Uint16 pmsmRs;                  // F1-16  同步机定子电阻
    Uint16 pmsmLd;                  // F1-17  同步机d轴电感
    Uint16 pmsmLq;                  // F1-18  同步机q轴电感
    Uint16 pmsmRsLdUnit;            // F1-19  同步机电感电阻单位
    Uint16 pmsmCoeff;               // F1-20  同步机反电动势系数
    Uint16 pmsmCheckTime;           // F1-21  同步机涑鋈毕嗉觳馐奔�
    Uint16 rsvdF12[5];
    
};

struct PG_PARA_STRUCT
{
    // PG卡参数
    Uint16 encoderPulse;            // F1-27    编码器脉冲线数
    Uint16 pgType;                  // F1-28    编码器类型
    Uint16 fvcPgSrc;                // F1-29    速度反馈PG卡选择, 0-QEP1,1-QEP2(扩展)    
    Uint16 enCoderDir;              // F1-30    编码器相序/主方向
    Uint16 enCoderAngle;            // F1-31    编码器安装角
    Uint16 uvwSignDir;              // F1-32    UVW信号方向
    Uint16 uvwSignAngle;            // F1-33    UVW信号零点位置角
    Uint16 enCoderPole;             // F1-34    旋变极对数
    Uint16 rsvdF11;                 // F1-35    UVW极对数 
    Uint16 fvcPgLoseTime;           // F1-36    速度反馈PG断线检测时间    
};

union MOTOR_PARA
{
    Uint16 all[sizeof(struct MOTOR_PARA_STRUCT)];
    
    struct MOTOR_PARA_STRUCT elem;
};

union PG_PARA
{
    Uint16 all[sizeof(struct PG_PARA_STRUCT)];
    
    struct PG_PARA_STRUCT elem;
};

//=================================

//=================================
enum MOTOR_SN
{
    MOTOR_SN_1,     // 第1电机
    MOTOR_SN_2,     // 第2电机
    MOTOR_SN_3,     // 第3电机
    MOTOR_SN_4      // 第4电机
};
//=================================


//=================================


//------------------------------------------------
struct VC_PARA
{
    Uint16 vcSpdLoopKp1;            // F2-00  速度环比例增益1
    Uint16 vcSpdLoopTi1;            // F2-01  速度环积分时间1
    Uint16 vcSpdLoopChgFrq1;        // F2-02  切换频率1
    Uint16 vcSpdLoopKp2;            // F2-03  速度环比例增益2
    Uint16 vcSpdLoopTi2;            // F2-04  速度环积分时间2
    
    Uint16 vcSpdLoopChgFrq2;        // F2-05  切换频率2
    Uint16 vcSlipCompCoef;          // F2-06  转差补偿系数
    Uint16 vcSpdLoopFilterTime;     // F2-07  速度环滤波时间常数
    Uint16 vcOverMagGain;           // F2-08  矢量控制过励磁增益
    Uint16 spdCtrlDriveTorqueLimitSrc;  // F2-09  速度控制(驱动)转矩上限源
    
    Uint16 spdCtrlDriveTorqueLimit;     // F2-10  速度控制(驱动)转矩上限数字设定
    Uint16 spdCtrlBrakeTorqueLimitSrc;  // F2-11  速度控制(制动)转矩上限源
    Uint16 spdCtrlBrakeTorqueLimit;     // F2-12  速度控制(制动)转矩上限数字设定
    Uint16 mAcrKp;                  // F2-13  M轴电流环Kp
    Uint16 mAcrKi;                  // F2-14  M轴电流环Ki
    
    Uint16 tAcrKp;                  // F2-15  T轴电流环Kp
    Uint16 tAcrKi;                  // F2-16  T轴电流环Ki
    Uint16 spdLoopI;                // F2-17  速度环积分属性

    Uint16 weakFlusMode;            // F2-18 同步机弱磁模式
    Uint16 weakFlusCoef;            // F2-19 同步机弱磁系数
    Uint16 weakFlusCurMax;          // F2-20 最大弱磁电流
    Uint16 weakFlusAutoCoef;        // F2-21 弱磁自动调谐系数
    Uint16 weakFlusIntegrMul;       // F2-22 弱磁积分倍数
};
//------------------------------------------------



//------------------------------------------------
// 第2电机的功能码，包括电机参数、控制参数
struct MOTOR_FC
{
    union MOTOR_PARA motorPara;     // Ax-00  Ax-26 第2/3/4电机参数. 同第1电机参数
    union PG_PARA    pgPara;        // Ax-27  Ax-36 第1电机PG卡参数
    Uint16 tuneCmd;                 // Ax-37  调谐命令
    struct VC_PARA vcPara;          // Ax-38  Ax-60
    
    Uint16 motorCtrlMode;           // Ax-61  第2/3/4电机控制方式
    Uint16 accDecTimeMotor;         // Ax-62  第2/3/4电机加减速时间选择
    Uint16 torqueBoost;             // Ax-63  转矩提升
    Uint16 rsvdA21;                 // Ax-64  振荡抑制增益模式/
    Uint16 antiVibrateGain;         // Ax-65  振荡抑制增益
};
//------------------------------------------------/





#define AI_NUMBER               3           // AI端子个数

#define AO_NUMBER               2           // AO端子个数

#define HDI_NUMBER              1           // HDI端子个数

#define HDO_NUMBER              1           // HDO端子个数

#define DI_NUMBER_PHSIC         10          // 物理DI端子
#define DI_NUMBER_V             5           // 虚拟DI端子
#define DI_NUMBER_AI_AS_DI      AI_NUMBER   // AI作为DI

#define DO_NUMBER_PHSIC         5           // 物理DO端子
#define DO_NUMBER_V             5           // 虚拟DO端子

#define PLC_STEP_MAX            16          // PLC、多段指令段数




//=====================================================================
// EEPROM的使用长度，包括中间预留部分
// EEPROM地址分配，2010-08-13
// 0            -   保留，rsvd4All
// 1,2          -   EEPROM-CHK
// 3            -   AIAO CHK
// 4-944        -   MD380使用
// 945-1149     -   预留MD380使用
// 1150-1329    -   IS380/MD380M使用
// 其中,16-63   -   掉电记忆使用
#define EEPROM_INDEX_USE_LENGTH     994     // 最后一个eeprom地址+1
#define EEPROM_INDEX_USE_INDEX        4     // 功能参数使用EEPROM起始
// 预留给其它使用的功能码index
#define FUNCCODE_RSVD4ALL_INDEX     GetCodeIndex(funcCode.code.rsvd4All)


#define REM_P_OFF_MOTOR     5   // 性能使用的掉电记忆。包含在CORE_TO_FUNC_DISP_DATA_NUM里面


// 功能码的组数
#define FUNCCODE_GROUP_NUM  83  // 包括预留的组, 见funcCodeGradeCurrentMenuMode. 
// EEPROM中顺序: EEPROM_CHK, 掉电记忆, FF, FP, F0-FE, A0-AF, B0-BF, C0-CF
// 显示顺序:     F0-FE, FF, FP, A0-AF, B0-BF, C0-CF, EEPROM_CHK(不显示), 掉电记忆(不显示), U0-UF
// U0-UF不占用EEPROM空间

#define FUNCCODE_GROUP_F0   0       // F0组
#define FUNCCODE_GROUP_F1   1       // F1组
#define FUNCCODE_GROUP_FE   14      // FF组
#define FUNCCODE_GROUP_FF   15      // FF组
#define FUNCCODE_GROUP_FP   16      // FP组
#define FUNCCODE_GROUP_A0   17      // A0组
#define FUNCCODE_GROUP_A5   22      // A5组
#define FUNCCODE_GROUP_AA   27      // AA组
#define FUNCCODE_GROUP_AB   28      // AB组
#define FUNCCODE_GROUP_AE   31      // AE组
#define FUNCCODE_GROUP_AF   32      // AF组
#define FUNCCODE_GROUP_B0   33      // B0组
#define FUNCCODE_GROUP_BF   48      // BF组
#define FUNCCODE_GROUP_C0   49      // C0组
//#define FUNCCODE_GROUP_CC   61      // CC组 用户定制功能码组
#define FUNCCODE_GROUP_CF   64      // CF组
#define FUNCCODE_GROUP_U0   67      // U0组，显示
#define FUNCCODE_GROUP_U3   70      // U3组，显示
#define FUNCCODE_GROUP_UF   (FUNCCODE_GROUP_NUM - 1)    // UF，性能调试显示

// 每组功能码的个数
// 除FF组之外，每组预留2个功能码。为了在增加功能码时，尽量不用恢复出厂参数。
#define F0NUM           (28+ 1  )   // F0  基本功能组
#define F1NUM           (38+ 0  )   // F1  电机参数
#define F2NUM           (23+ 0  )   // F2  矢量控制参数
#define F3NUM           (16+ 0  )   // F3  V/F控制参数

#define F4NUM           (40+ 0  )   // F4  输入端子
#define F5NUM           (23+ 0  )   // F5  输出端子
#define F6NUM           (16+ 0  )   // F6  启停控制
#define F7NUM           (15+ 0  )   // F7  键盘与显示

#define F8NUM           (54+ 0  )   // F8  辅助功能
#define F9NUM           (71+ 0  )   // F9  故障与保护
#define FANUM           (29+ 0  )   // FA  PID功能 
#define FBNUM           (10+ 0  )   // FB  摆频、定长和计数

#define FCNUM           (52+ 0  )   // FC  多端速、PLC
#define FDNUM           ( 8+ 0  )   // FD  通讯参数
#define FENUM           (32+ 0  )   // FE  280有320没有的功能码
#define FFNUM           (13+ 0  )   // FF  厂家参数

#define FPNUM           ( 6+ 0  )   // FP  用户密码, 参数初始化

#define A0NUM           ( 9+ 0  )   // A0
#define A1NUM           (22+0   )   // A1
#define A2NUM           (F1NUM+2+F2NUM+3) // A2
#define A3NUM           (A2NUM)             // A3
#define A4NUM           (A2NUM)             // A4
#define A5NUM           (10+ 0  )   // A5
#define A6NUM           (30+0   )   // A6
#define A7NUM           (9+3    )   // A7
#define A8NUM           (8+0    )   // A8
#define A9NUM           (30     )   // A9
#define AANUM           (1+ 0  )    // AA
#define ABNUM           (1+ 0   )   // AB
#define ACNUM           (20     )   // AC
#define ADNUM           (1+0    )   // AD
#define AENUM           (ACNUM  )   // AE
#define AFNUM           ( 1+0   )   // AF

#define B0NUM           ( 0+1   )   // B0
#define B1NUM           ( 0+1   )   // B1
#define B2NUM           ( 0+1   )   // B2
#define B3NUM           ( 0+1   )   // B3
                                    
#define B4NUM           ( 0+1   )   // B4
#define B5NUM           ( 0+1   )   // B5
#define B6NUM           ( 0+1   )   // B6
#define B7NUM           ( 0+1   )   // B7
                                    
#define B8NUM           ( 0+1   )   // B8
#define B9NUM           ( 0+1   )   // B9
#define BANUM           ( 0+1   )   // BA
#define BBNUM           ( 0+1   )   // BB
                                    
#define BCNUM           ( 0+1   )   // BC
#define BDNUM           ( 0+1   )   // BD
#define BENUM           ( 0+1   )   // BE
#define BFNUM           ( 0+1   )   // BF

#define C0NUM           ( 0+1   )   // C0
#define C1NUM           ( 0+1   )   // C1
#define C2NUM           ( 0+1)      // C2
#define C3NUM           ( 0+1)      // C3

#define C4NUM           ( 0+1)      // C4
#define C5NUM           ( 0+1)      // C5
#define C6NUM           ( 0+1)      // C6
#define C7NUM           ( 0+1)      // C7

#define C8NUM           ( 0+1)      // C8
#define C9NUM           ( 0+1)      // C9
#define CANUM           ( 0+1)      // CA
#define CBNUM           ( 0+1)      // CB
                                
#define CCNUM           ( 1+0)      // CC   
#define CDNUM           ( 0+1)      // CD
#define CENUM           ( 0+1)      // CE
#if DEBUG_F_MOTOR_FUNCCODE
#define CFNUM           FUNC_TO_CORE_DEBUG_DATA_NUM     // CF  调试，func2motor
#elif 1
#define CFNUM           ( 0+1)      // CF
#endif

#define CHK_NUM   (4 +0  )  //     eepromCheckWord(2)，rsvd4All(1)放在这里的最前面, AIAOChk(1)也在这里
#define REM_NUM   (48)      // 掉电记忆，包括性能使用的掉电记忆。(包括性能的掉电记忆)

#define U0NUM     (100+0)   // U0  显示使用，不占用EEPROM，尽量少的占用程序空间(无出厂值、上下限，但有属性)
#define U1NUM     ( 0+1)    // U1
#define U2NUM     ( 0+1)    // U2
#define U3NUM     (10+2)    // U3

#define U4NUM     ( 0+1)    // U4
#define U5NUM     ( 0+1)    // U5
#define U6NUM     ( 0+1)    // U6
#define U7NUM     ( 0+1)    // U7

#define U8NUM     ( 0+1)    // U8
#define U9NUM     ( 0+1)    // U9
#define UANUM     ( 0+1)    // UA
#define UBNUM     ( 0+1)    // UB

#define UCNUM     ( 0+1)    // UC
#define UDNUM     ( 0+1)    // UD
#define UENUM     ( 0+1)    // UE
#if DEBUG_F_MOTOR_FUNCCODE
#define UFNUM     CORE_TO_FUNC_DISP_DATA_NUM  // UF  调试，motor2func
#elif 1
#define UFNUM     ( 0+1)    // UF 性能调试使用
#endif

#define FNUM_PARA      (F0NUM + F1NUM + F2NUM + F3NUM +     \
                        F4NUM + F5NUM + F6NUM + F7NUM +     \
                        F8NUM + F9NUM + FANUM + FBNUM +     \
                        FCNUM + FDNUM + FENUM + FFNUM +     \
                                                            \
                        FPNUM +                             \
                                                            \
                        A0NUM + A1NUM + A2NUM + A3NUM +     \
                        A4NUM + A5NUM + A6NUM + A7NUM +     \
                        A8NUM + A9NUM + AANUM + ABNUM +     \
                        ACNUM + ADNUM + AENUM + AFNUM +     \
                                                            \
                        B0NUM + B1NUM + B2NUM + B3NUM +     \
                        B4NUM + B5NUM + B6NUM + B7NUM +     \
                        B8NUM + B9NUM + BANUM + BBNUM +     \
                        BCNUM + BDNUM + BENUM + BFNUM +     \
                                                            \
                        C0NUM + C1NUM + C2NUM + C3NUM +     \
                        C4NUM + C5NUM + C6NUM + C7NUM +     \
                        C8NUM + C9NUM + CANUM + CBNUM +     \
                        CCNUM + CDNUM + CENUM + CFNUM       \
                        )                                   // 所有功能码，不包括显示
#define FNUM_EEPROM    (FNUM_PARA + CHK_NUM + REM_NUM)      // 需要存储在EEPROM中的所有参数
#define FNUM_ALL       (FNUM_EEPROM +                       \
                        U0NUM + U1NUM + U2NUM + U3NUM +     \
                        U4NUM + U5NUM + U6NUM + U7NUM +     \
                        U8NUM + U9NUM + UANUM + UBNUM +     \
                        UCNUM + UDNUM + UENUM + UFNUM       \
                       )                                    // 所有功能码、参数，包括显示


//=====================================================================
//
// 功能码组，F0组，F1组, ...
//
// eepromCheckWord放在最前面
// 厂家参数、用户密码、用户定制菜单放在前面，防止增删功能码时被误修改。
// fc与eeprom有对应关系表，所以以上要求不需要了
//
//=====================================================================
struct FUNCCODE_GROUP 
{
//======================================
    Uint16 f0[F0NUM];               // F0 基本功能组
    Uint16 f1[F1NUM];               // F1 电机参数
    Uint16 f2[F2NUM];               // F2 矢量控制参数
    Uint16 f3[F3NUM];               // F3 V/F控制参数
    
//======================================
    Uint16 f4[F4NUM];               // F4 输入端子
    Uint16 f5[F5NUM];               // F5 输出端子
    Uint16 f6[F6NUM];               // F6 启停控制
    Uint16 f7[F7NUM];               // F7 键盘与显示
    
//======================================
    Uint16 f8[F8NUM];               // F8 辅助功能
    Uint16 f9[F9NUM];               // F9 故障与保护
    Uint16 fa[FANUM];               // FA PID功能
    Uint16 fb[FBNUM];               // FB 摆频、定长和计数

//======================================
    Uint16 fc[FCNUM];               // FC 多端速、PLC
    Uint16 fd[FDNUM];               // FD 通讯参数
    Uint16 fe[FENUM];               // FE 280有320没有的功能码
    Uint16 ff[FFNUM];               // FF 厂家参数
    
//======================================
    Uint16 fp[FPNUM];               // FP 功能码管理
    
//======================================
    Uint16 a0[A0NUM];               // A0
    Uint16 a1[A1NUM];               // A1
    Uint16 a2[A2NUM];               // A2
    Uint16 a3[A3NUM];               // A3

//======================================
    Uint16 a4[A4NUM];               // A4
    Uint16 a5[A5NUM];               // A5
    Uint16 a6[A6NUM];               // A6
    Uint16 a7[A7NUM];               // A7

//======================================
    Uint16 a8[A8NUM];               // A8
    Uint16 a9[A9NUM];               // A9
    Uint16 aa[AANUM];               // AA
    Uint16 ab[ABNUM];               // AB

//======================================
    Uint16 ac[ACNUM];               // AC
    Uint16 ad[ADNUM];               // AD
    Uint16 ae[AENUM];               // AE AIAO出厂校正
    Uint16 af[AFNUM];               // AF 功能码组隐藏
    
//======================================
    Uint16 b0[B0NUM];               // B0
    Uint16 b1[B1NUM];               // B1
    Uint16 b2[B2NUM];               // B2
    Uint16 b3[B3NUM];               // B3

//======================================
    Uint16 b4[B4NUM];               // B4
    Uint16 b5[B5NUM];               // B5
    Uint16 b6[B6NUM];               // B6
    Uint16 b7[B7NUM];               // B7

//======================================
    Uint16 b8[B8NUM];               // B8
    Uint16 b9[B9NUM];               // B9
    Uint16 ba[BANUM];               // BA
    Uint16 bb[BBNUM];               // BB

//======================================
    Uint16 bc[BCNUM];               // BC
    Uint16 bd[BDNUM];               // BD
    Uint16 be[BENUM];               // BE
    Uint16 bf[BFNUM];               // BF

//======================================
    Uint16 c0[C0NUM];               // C0
    Uint16 c1[C1NUM];               // C1
    Uint16 c2[C2NUM];               // C2
    Uint16 c3[C3NUM];               // C3

//======================================
    Uint16 c4[C4NUM];               // C4
    Uint16 c5[C5NUM];               // C5
    Uint16 c6[C6NUM];               // C6
    Uint16 c7[C7NUM];               // C7

//======================================
    Uint16 c8[C8NUM];               // C8
    Uint16 c9[C9NUM];               // C9
    Uint16 ca[CANUM];               // CA
    Uint16 cb[CBNUM];               // CB

//======================================
    Uint16 cc[CCNUM];               // CC   
    Uint16 cd[CDNUM];               // CD
    Uint16 ce[CENUM];               // CE
    Uint16 cf[CFNUM];               // CF

// 之前的功能码有上下限，属性；之后没有，节省空间
//======================================

//======================================
    Uint16 fChk[CHK_NUM];           // eepromCheckWord

//======================================
    Uint16 remember[REM_NUM];       // 掉电记忆

// 之前的数据要放在EEPROM中
//======================================

//======================================
// 之后的数据不需要放在EEPROM中，仅RAM
    Uint16 u0[U0NUM];               // U0 显示
    Uint16 u1[U1NUM];               // U1
    Uint16 u2[U2NUM];               // U2
    Uint16 u3[U3NUM];               // U3

//======================================
    Uint16 u4[U4NUM];               // U4
    Uint16 u5[U5NUM];               // U5
    Uint16 u6[U6NUM];               // U6
    Uint16 u7[U7NUM];               // U7

//======================================
    Uint16 u8[U8NUM];               // U8
    Uint16 u9[U9NUM];               // U9
    Uint16 ua[UANUM];               // UA
    Uint16 ub[UBNUM];               // UB

//======================================
    Uint16 uc[UCNUM];               // UC
    Uint16 ud[UDNUM];               // UD
    Uint16 ue[UENUM];               // UE
    Uint16 uf[UFNUM];               // UF, 显示，性能调试使用
//======================================
};


//=====================================================================
//
// 功能码，F0-00, F0-01, ..., F1-00, F1-01, ...
//
//=====================================================================
struct FUNCCODE_CODE 
{
//======================================
// F0 基本功能组
    Uint16 inverterGpTypeDisp;      // F0-00  GP类型显示
    Uint16 motorCtrlMode;           // F0-01  (电机)控制方式
    Uint16 runSrc;                  // F0-02  命令源选择
    Uint16 frqXSrc;                 // F0-03  主频率源X选择
    Uint16 frqYSrc;                 // F0-04  辅助频率源Y选择
    Uint16 frqYRangeBase;           // F0-05  辅助频率源Y范围选择
    Uint16 frqYRange;               // F0-06  辅助频率源Y范围
    Uint16 frqCalcSrc;              // F0-07  频率源选择
    Uint16 presetFrq;               // F0-08  预置频率
    Uint16 runDir;                  // F0-09  运行方向
    Uint16 maxFrq;                  // F0-10  最大频率
    Uint16 upperFrqSrc;             // F0-11  上限频率源
    Uint16 upperFrq;                // F0-12  上限频率数值设定
    Uint16 upperFrqOffset;          // F0-13  上限频率偏置
    Uint16 lowerFrq;                // F0-14  下限频率数值设定
    Uint16 carrierFrq;              // F0-15  载波频率
    Uint16 varFcByTem;              // F0-16  载波频率随温度调整
    Uint16 accTime1;                // F0-17  加速时间1
    Uint16 decTime1;                // F0-18  减速时间1
    Uint16 accDecTimeUnit;          // F0-19  加减速时间的单位
    Uint16 frqYOffsetSrc;           // F0-20  辅助频率源偏置选择
    Uint16 frqYOffsetFc;            // F0-21  辅助频率源偏置的数字设定
    Uint16 frqPoint;                // F0-22  频率指令小数点
    Uint16 frqRemMode;              // F0-23  数字设定频率记忆选择
    enum MOTOR_SN motorSn;          // F0-24  电机选择
    Uint16 accDecBenchmark;         // F0-25  加减速时间基准频率
    Uint16 updnBenchmark;           // F0-26  运行时频率指令UP/DOWN基准
    Uint16 frqRunCmdBind;           // F0-27  命令源捆绑频率源
    Uint16 commProtocolSec;         // F0-28  通讯协议选择

//======================================
// F1 电机参数
    union MOTOR_PARA motorParaM1;   // F1-00  F1-26 第1电机参数
    union PG_PARA    pgParaM1;      // f1-27  F1-36 第1电机PG卡参数
    Uint16 tuneCmd;                 // F1-37  调谐选择

//======================================
// F2 矢量控制参数
    struct VC_PARA vcParaM1;        // 第1电机矢量控制参数


//======================================
// F3 V/F控制参数
    Uint16 vfCurve;                 // F3-00  VF曲线设定
    Uint16 torqueBoost;             // F3-01  转矩提升
    Uint16 boostCloseFrq;           // F3-02  转矩提升截止频率
    Uint16 vfFrq1;                  // F3-03  多点VF频率点1
    Uint16 vfVol1;                  // F3-04  多点VF电压点1
    Uint16 vfFrq2;                  // F3-05  多点VF频率点2
    Uint16 vfVol2;                  // F3-06  多点VF电压点2
    Uint16 vfFrq3;                  // F3-07  多点VF频率点3
    Uint16 vfVol3;                  // F3-08  多点VF电压点3
    Uint16 slipCompCoef;            // F3-09  转差补偿系数
    Uint16 vfOverMagGain;           // F3-10  VF过励磁增益
    Uint16 antiVibrateGain;         // F3-11  振荡抑制增益

    Uint16 rsvdF31;//antiVibrateGainMode;     // F3-12  振荡抑制增益模式
    Uint16 vfVoltageSrc;            // F3-13  VF分离的电压源
    Uint16 vfVoltageDigtalSet;      // F3-14  VF分离的电压源数字设定
    Uint16 vfVoltageAccTime;        // F3-15  VF分离的电压上升时间

//======================================
// F4 输入端子
    Uint16 diFunc[DI_NUMBER_PHSIC]; // F4-00  --F4-09   DI1端子功能选择
    
    Uint16 diFilterTime;            // F4-10  DI滤波时间
    Uint16 diControlMode;           // F4-11  端子命令方式
    Uint16 diUpDownSlope;           // F4-12  端子UP/DOWN速率，改为0.001Hz

    Uint16 curveSet2P1[4];          // F4-13,...,F4-16  曲线1，2点，最大值，最小值
    Uint16 ai1FilterTime;           // F4-17  AI1滤波时间, 10ms

    Uint16 curveSet2P2[4];          // F4-18,...,F4-21  曲线2，2点
    Uint16 ai2FilterTime;           // F4-22  AI2滤波时间, 10ms

    Uint16 curveSet2P3[4];          // F4-23,...,F4-26  曲线3，2点
    Uint16 ai3FilterTime;           // F4-27  AI3滤波时间, 10ms

    Uint16 curveSet2P4[4];          // F4-28,...,F4-31  HDI曲线，2点
    Uint16 pulseInFilterTime;       // F4-32  PULSE滤波时间, 10ms

    Uint16 aiCurveSrc;              // F4-33  AI设定曲线选择

    Uint16 aiLimitSrc;              // F4-34  AI下限选择

    Uint16 diDelayTime[3];          // F4-35  DI1延迟时间
    Uint16 diLogic[2];              // F4-38  DI有效状态选择1
                                    // F4-39  DI有效状态选择2

//======================================
// F5 输出端子
    Uint16 fmOutSelect;             // F5-00  多功能端子输出选择
    Uint16 doFunc[DO_NUMBER_PHSIC]; // F5-01  FMR输出选择
                                    // F5-02  控制板RELAY输出选择
                                    // F5-03  扩展卡RELAY输出选择
                                    // F5-04  DO1输出选择
                                    // F5-05  扩展卡DO2输出选择

    Uint16 aoFunc[AO_NUMBER+HDO_NUMBER];    // F5-06  FMP输出选择
                                            // F5-07  AO1输出选择
                                            // F5-08  扩展卡AO2输出选择
    Uint16 fmpOutMaxFrq;                    // F5-09  FMP输出最大频率

    AO_PARA aoPara[AO_NUMBER];              // F5-10  AO1零偏系数
                                            // F5-11  AO1增益
                                            // F5-12  AO2零偏系数
                                            // F5-13  AO2增益
    Uint16 aoLpfTime[AO_NUMBER+HDO_NUMBER]; // F5-14  HDO,AO1,AO2输出滤波时间
    
    Uint16 doDelayTime[DO_NUMBER_PHSIC];// F5-17  RELAY1输出延迟时间
                                        // F5-18  RELAY2输出延迟时间
                                        // F5-19  DO1输出延迟时间
                                    
                                        // F5-20  DO2输出延迟时间
                                        // F5-21  DO3输出延迟时间
    Uint16 doLogic;                     // F5-22  DO有效状态选择

//======================================
// F6 启停控制
    Uint16 startMode;               // F6-00  启动方式
    Uint16 speedTrackMode;          // F6-01  转速跟踪方式
    Uint16 speedTrackVelocity;      // F6-02  转速跟踪快慢
    Uint16 startFrq;                // F6-03  启动频率
    Uint16 startFrqTime;            // F6-04  启动频率保持时间
    Uint16 startBrakeCurrent;       // F6-05  启动直流制动电流
    Uint16 startBrakeTime;          // F6-06  启动直流制动时间
    Uint16 accDecSpdCurve;          // F6-07  加减速方式
    Uint16 sCurveStartPhaseTime;    // F6-08  S曲线开始段时间比例
    Uint16 sCurveEndPhaseTime;      // F6-09  S曲线结束段时间比例
    Uint16 stopMode;                // F6-10  停机方式
    Uint16 stopBrakeFrq;            // F6-11  停机直流制动起始频率
    Uint16 stopBrakeWaitTime;       // F6-12  停机直流制动等待时间
    Uint16 stopBrakeCurrent;        // F6-13  停机直流制动电流
    Uint16 stopBrakeTime;           // F6-14  停机直流制动时间
    Uint16 brakeDutyRatio;          // F6-15  制动使用率

//======================================
// F7 键盘与显示
    Uint16 rsvdF71;                 // F7-00  保留
    Uint16 mfkKeyFunc;              // F7-01  MF.K键功能选择
    Uint16 stopKeyFunc;             // F7-02  STOP键功能
    Uint16 ledDispParaRun1;         // F7-03  LED运行显示参数1
    Uint16 ledDispParaRun2;         // F7-04  LED运行显示参数2
    
    Uint16 ledDispParaStop;         // F7-05  LED停机显示参数
    Uint16 speedDispCoeff;          // F7-06  负载速度显示系数
    Uint16 radiatorTemp;            // F7-07  逆变器模块散热器温度
    Uint16 temp2;                   // F7-08  整流桥散热器温度
    Uint16 runTimeAddup;            // F7-09  累计运行时间, 单位: h
    
    Uint16 productVersion;          // F7-10  产品号
    Uint16 softVersion;             // F7-11  软件版本号
    Uint16 speedDispPointPos;       // F7-12  负载速度显示小数点位置
    Uint16 powerUpTimeAddup;        // F7-13  累计上电时间
    Uint16 powerAddup;              // F7-14  累计耗电量
    

//======================================
// F8 辅助功能
    Uint16 jogFrq;                  // F8-00  点动运行频率
    Uint16 jogAccTime;              // F8-01  点动加速时间
    Uint16 jogDecTime;              // F8-02  点动减速时间
    Uint16 accTime2;                // F8-03  加速时间2
    Uint16 decTime2;                // F8-04  减速时间2
    Uint16 accTime3;                // F8-05  加速时间3
    Uint16 decTime3;                // F8-06  减速时间3
    Uint16 accTime4;                // F8-07  加速时间4
    Uint16 decTime4;                // F8-08  减速时间4
    Uint16 jumpFrq1;                // F8-09  跳跃频率1
    Uint16 jumpFrq2;                // F8-10  跳跃频率2
    Uint16 jumpFrqRange;            // F8-11  跳跃频率幅度
    Uint16 zeroSpeedDeadTime;       // F8-12  正反转死区时间
    Uint16 antiReverseRun;          // F8-13  反转控制, 0-允许反转，1-禁止反转
    Uint16 lowerDeal;               // F8-14  频率低于下限频率运行动作
    Uint16 droopCtrl;               // F8-15  下垂控制
    Uint16 powerUpTimeArriveSet;    // F8-16  设定上电到达时间
    Uint16 runTimeArriveSet;        // F8-17  设定运行到达时间
    Uint16 startProtect;            // F8-18  启动保护选择
    Uint16 frqFdtValue;             // F8-19  频率检测值(FDT电平)
    Uint16 frqFdtLag;               // F8-20  频率检测滞后值
    Uint16 frqArriveRange;          // F8-21  频率到达检出幅度
    Uint16 jumpFrqMode;             // F8-22  加减速过程中跳跃频率是否有效
    Uint16 runTimeOverAct;          // F8-23  设定运行时间到达动作选择
  
    // ADD
    Uint16 powerUpTimeOverAct;      // F8-24  设定上电时间到达动作选择
    Uint16 accTimefrqChgValue;      // F8-25  加速时间1/2切换频率点 
    Uint16 decTimefrqChgValue;      // F8-26  减速时间1/2切换频率点 
    Uint16 jogWhenRun;              // F8-27  端子点动优先
    Uint16 frqFdt1Value;            // F8-28  频率检测值(FDT1电平)
    Uint16 frqFdt1Lag;              // F8-29  频率检测1滞后值
    Uint16 frqArriveValue1;         // F8-30  频率到达检测值1 
    Uint16 frqArriveRange1;         // F8-31  频率到达检出1幅度
    Uint16 frqArriveValue2;         // F8-32  频率到达检测值2 
    Uint16 frqArriveRange2;         // F8-33  频率到达检出2幅度

    Uint16 oCurrentChkValue;        // F8-34  零电流检测值
    Uint16 oCurrentChkTime;         // F8-35  零电流检测延迟时间
    Uint16 softOCValue;             // F8-36  软件过流点
    Uint16 softOCDelay;             // F8-37  软件过流检测延迟时间

    Uint16 currentArriveValue1;     // F8-38  电流到达检测值1
    Uint16 currentArriveRange1;     // F8-39  电流到达检测1幅度
    Uint16 currentArriveValue2;     // F8-40  电流到达检测值1
    Uint16 currentArriveRange2;     // F8-41  电流到达检测1幅度

    Uint16 setTimeMode;             // F8-42  定时功能选择
    Uint16 setTimeSource;           // F8-43  定时时间设定选择
    Uint16 setTimeValue;            // F8-44  设定运行时间
    
    Uint16 ai1VoltageLimit;         // F8-45  AI1输入电压下限
    Uint16 ai1VoltageUpper;         // F8-46  AI1输入电压上限

    Uint16 temperatureArrive;       // F8-47  模块温度到达
    Uint16 fanControl;              // F8-48  风扇控制
    Uint16 wakeUpFrq;               // F8-49  唤醒频率
    Uint16 wakeUpTime;              // F8-50  唤醒延迟时间
    Uint16 dormantFrq;              // F8-51  休眠频率
    Uint16 dormantTime;             // F8-52  休眠延迟时间
    Uint16 setTimeArrive;           // F8-53  当前运行到达时间
    
//======================================
// F9 故障与保护
    Uint16 overloadMode;                // F9-00  电机过载保护选择
    Uint16 overloadGain;                // F9-01  电机过载保护增益
    Uint16 foreOverloadCoef;            // F9-02  电机过载预警系数
    Uint16 ovGain;                      // F9-03  过压失速增益
    Uint16 ovPoint;                     // F9-04  过压失速保护电压
    
    Uint16 ocGain;                      // F9-05  过流失速增益
    Uint16 ocPoint;                     // F9-06  过流失速保护电流
    Uint16 shortCheckMode;              // F9-07  上电对地短路保护功能
    Uint16 rsvdF91;                     // F9-08  保留
    Uint16 errAutoRstNumMax;            // F9-09  故障自动复位次数
    
    Uint16 errAutoRstRelayAct;          // F9-10  故障自动复位期间故障继电器动作选择
    Uint16 errAutoRstSpaceTime;         // F9-11  故障自动复位间隔时间, 0.1s 
    Uint16 inPhaseLossProtect;          // F9-12  输入缺相保护选择
    Uint16 outPhaseLossProtect;         // F9-13  输出缺相保护选择
    Uint16 errorLatest1;                // F9-14  第一次故障类型
    
    Uint16 errorLatest2;                // F9-15  第二次故障类型
    Uint16 errorLatest3;                // F9-16  第三次(最近一次)故障类型

    union ERROR_SCENE errorScene3;      // F9-17  第三次(最近一次)故障时频率                              
                                        // F9-18  第三次(最近一次)故障时电流                             
                                        // F9-19  第三次(最近一次)故障时母线电压                                                                                  
                                        // F9-20  第三次(最近一次)故障时输入端子状态                     
                                        // F9-21  第三次(最近一次)故障时输出端子状态   
                                        
                                        // F9-22  第三次(最近一次)故障时变频器状态                       
                                        // F9-23  第三次(最近一次)故障时时间（从本次上电开始计时）       
                                        // F9-24  第三次(最近一次)故障时时间（从运行时开始计时）         

    Uint16 rsvdF92[2];                  // F9-25  F9-26
    
    union ERROR_SCENE errorScene2;      // F9-27  第二次故障现场
    Uint16 rsvdF921[2];                 // F9-35  F9-36

    union ERROR_SCENE errorScene1;      // F9-37  第一次故障现场
    Uint16 rsvdF922[2];                 // F9-45  F9-46
    
    Uint16 errorAction[5];              // F9-47  -F9-51  故障时保护动作选择1-5
    Uint16 errorShow[2];                // F9-52  -F9-53  故障指示选择1,2
    Uint16 errorRunFrqSrc;              // F9-54  故障时继续运行频率选择
    Uint16 errorSecondFrq;              // F9-55  异常备用频率设定
    
    Uint16 motorOtMode;                 // F9-56  电机温度传感器类型
    Uint16 motorOtProtect;              // F9-57  电机过热保护阈值
    Uint16 motorOtCoef;                 // F9-58  电机过热预报警阈值
     
    Uint16 pOffTransitoryNoStop;        // F9-59  瞬停不停功能选择
    Uint16 pOffTransitoryFrqDecSlope;   // F9-60  瞬停动作暂停判断电压
    Uint16 pOffVolBackTime;             // F9-61  瞬停不停电压回升判断时间
    
    Uint16 pOffThresholdVol;            // F9-62  瞬停不停动作判断电压
    Uint16 loseLoadProtectMode;         // F9-63  掉载保护选择
    Uint16 loseLoadLevel;               // F9-64  掉载检出水平
    Uint16 loseLoadTime;                // F9-65  掉载检出时间
    Uint16 rsvdF923;                    // F9-66  故障频率的小数点

    Uint16 osChkValue;                  // F9-67 过速度检测值
    Uint16 osChkTime;                   // F9-68 过速度检测时间
    Uint16 devChkValue;                 // F9-69 速度偏差过大检测值
    Uint16 devChkTime;                  // F9-70 速度偏差过大检测时间

#if 0 
    Uint16 losePowerStopEnable;         // F9-71 掉电停机功能有效
    Uint16 losePowerStopSel;            // F9-72 掉电停机处理方式
    Uint16 losePowerLowerFrq;           // F9-73 掉电初始跌落频率
    Uint16 losePowerLowerTime;          // F9-74 掉电初始跌落时间
    Uint16 losePowerDectime;            // F9-75 掉电减速时间
    Uint16 losePowerP;                  // F9-76 掉电跌落电压闭环比例
    Uint16 losePowerI;                  // F9-77 掉电跌落电压闭环积分
    
#endif
    

//======================================
// FA PID功能
    Uint16 pidSetSrc;               // FA-00  PID给定源
    Uint16 pidSet;                  // FA-01  PID数值给定, 0.1%
    Uint16 pidFdbSrc;               // FA-02  PID反馈源
    Uint16 pidDir;                  // FA-03  PID作用方向
    Uint16 pidDisp;                 // FA-04  PID给定反馈量程
    
    Uint16 pidKp;                   // FA-05  比例增益P
    Uint16 pidTi;                   // FA-06  积分时间I
    Uint16 pidTd;                   // FA-07  微分时间D
    Uint16 reverseCutOffFrq;        // FA-08  PID反转截止频率
    Uint16 pidErrMin;               // FA-09  PID偏差极限
    Uint16 pidDLimit;               // FA-10  PID微分限幅
    Uint16 pidSetChangeTime;        // FA-11  PID给定变化时间
    Uint16 pidFdbLpfTime;           // FA-12  PID反馈滤波时间
    Uint16 pidOutLpfTime;           // FA-13  PID输出滤波时间
    Uint16 pidSampleTime;           // FA-14  PID采样周期(暂未做)
    Uint16 pidKp2;                  // FA-15  PID比例增益P2
    Uint16 pidTi2;                  // FA-16  PID积分时间I2
    Uint16 pidTd2;                  // FA-17  PID微分时间D2
    Uint16 pidParaChgCondition;     // FA-18  PID参数切换条件
    Uint16 pidParaChgDelta1;        // FA-19  PID参数切换偏差1
    Uint16 pidParaChgDelta2;        // FA-20  PID参数切换偏差2
    Uint16 pidInit;                 // FA-21  PID初值
    Uint16 pidInitTime;             // FA-22  PID初值保持时间
    Uint16 pidOutDeltaMax;          // FA-23  PID两次输出之间偏差的最大值
    Uint16 pidOutDeltaMin;          // FA-24  PID两次输出之间偏差的最小值
    Uint16 pidIAttribute;           // FA-25  PID积分属性
    Uint16 pidFdbLoseDetect;        // FA-26  PID反馈丢失检测值
    Uint16 pidFdbLoseDetectTime;    // FA-27  PID反馈丢失检测时间
    Uint16 pidCalcMode;             // FA-28  PID运算模式(停机是否运算). 供水模式下，停机时PID也计算.

//======================================
// FB 摆频、定长和计数
    Uint16 swingBaseMode;           // FB-00  摆频设定方式
    Uint16 swingAmplitude;          // FB-01  摆频幅度
    Uint16 swingJumpRange;          // FB-02  突跳频率幅度
    Uint16 swingPeriod;             // FB-03  摆频周期
    Uint16 swingRiseTimeCoeff;      // FB-04  摆频的三角波上升时间
    Uint16 lengthSet;               // FB-05  设定长度
    Uint16 lengthCurrent;           // FB-06  实际长度
    Uint16 lengthPulsePerMeter;     // FB-07  每米脉冲数，单位: 0.1
    Uint16 counterSet;              // FB-08  设定计数值
    Uint16 counterPoint;            // FB-09  指定计数值

//======================================
// FC 多端速、PLC
    Uint16 plcFrq[PLC_STEP_MAX];                  // FC-00  --FC-15   多段指令0-多段指令15
    Uint16 plcRunMode;                  // FC-16  PLC运行方式
    Uint16 plcPowerOffRemMode;          // FC-17  PLC掉电记忆选择
    struct PLC_STRUCT plcAttribute[PLC_STEP_MAX]; // FC-18  --FC-49   PLC运行时间，加减速时间选择
    Uint16 plcTimeUnit;                 // FC-50  PLC运行时间单位
    Uint16 plcFrq0Src;                  // FC-51  多段指令0给定方式

// FD 通讯参数
    Uint16 commBaudRate;            // FD-00  波特率
    Uint16 commParity;              // FD-01  数据格式
    Uint16 commSlaveAddress;        // FD-02  本机地址
    Uint16 commDelay;               // FD-03  应答延迟
    Uint16 commOverTime;            // FD-04  通讯超时时间
    Uint16 commProtocol;            // FD-05  通讯数据传送格式选择   
    Uint16 commReadCurrentPoint;    // FD-06  通讯读取电流分辨率
    Uint16 commMaster;              // FD-07  通讯主从方式

// FE  280有320没有的功能码
    Uint16 userCustom[FENUM];       // FE 用户定制功能码组

// FF 厂家参数
    Uint16 factoryPassword;         // FF-00  厂家密码
    Uint16 inverterType;            // FF-01  变频器机型
    Uint16 inverterGpType;          // FF-02  G/P型号
    Uint16 inverterPower;           // FF-03  变频器功率
    Uint16 tempCurve;               // FF-04  温度曲线
    
    Uint16 uvGainWarp;              // FF-05  UV两相电流采样增益偏差
    Uint16 funcSoftVersion;         // FF-06  保留
    Uint16 motorSoftVersion;        // FF-07  性能软件版本号
    Uint16 volJudgeCoeff;           // FF-08  电压校正系数
    Uint16 curJudgeCoeff;           // FF-09  电流校正系数
    
    Uint16 motorDebugFc;            // FF-10  性能调试功能码显示个数
    Uint16 aiaoCalibrateDisp;       // FF-11  AIAO校正功能码显示
    Uint16 memoryAddr;              // FF-12  内存地址查看

// FP 用户密码, 参数初始化
    Uint16 userPassword;            // FP-00  用户密码
    Uint16 paraInitMode;            // FP-01  参数初始化
    Uint16 funcParaView;            // FP-02  功能参数模式属性
    Uint16 menuMode;                // FP-03  个性化参数模式选择
    
    Uint16 userPasswordReadOnly;    // FP-04  只读用户密码
    Uint16 rsvdFp;                  // FP-05  保留

// A0 转矩控制和限定参数
    Uint16 torqueCtrl;              // A0-00  转矩控制
    Uint16 driveUpperTorqueSrc;     // A0-01  驱动转矩上限源
    Uint16 brakeUpperTorqueSrc;     // A0-02  制动转矩上限源
    Uint16 driveUpperTorque;        // A0-03  驱动转矩上限
    Uint16 torqueFilter;            // A0-04  转矩滤波
    Uint16 torqueCtrlFwdMaxFrq;     // A0-05  转矩控制正向最大频率
    Uint16 torqueCtrlRevMaxFrq;     // A0-06  转矩控制反向最大频率
    Uint16 torqueCtrlAccTime;       // A0-07  转矩加速时间
    Uint16 torqueCtrlDecTime;       // A0-08  转矩减速时间
    
// A1 虚拟DI、虚拟DO
    Uint16 vdiFunc[5];              // A1-00  --A1-04 VDI1端子功能选择
    Uint16 vdiSrc;                  // A1-05  VDI端子有效状态来源
    Uint16 vdiFcSet;                // A1-06  VDI端子功能码设定有效状态
    Uint16 aiAsDiFunc[3];           // A1-07  --A1-09 AI1端子功能选择（当作DI）
    Uint16 diLogicAiAsDi;           // A1-10  AI作为DI有效状态选择
    Uint16 vdoFunc[5];              // A1-11  --A1-15 虚拟VDO1～VDO5输出选择
    Uint16 vdoDelayTime[5];         // A1-16  --A1-20 VDO1～VDO5延迟时间
    Uint16 vdoLogic;                // A1-21  VDO输出端子有效状态选择
    
// A2 第2电机参数
    struct MOTOR_FC motorFcM2;      // 第2电机参数
    
// A3 第2电机参数
    struct MOTOR_FC motorFcM3;      // 第3电机参数
    
// A4 第2电机参数
    struct MOTOR_FC motorFcM4;      // 第4电机参数
    
// A5 控制优化参数
    Uint16 pwmMode;                 // A5-00    DPWM切换上限频率
    Uint16 modulationMode;          // A5-01    调制方式，0-异步调制，1-同步调制
    Uint16 deadCompMode;            // A5-02    死区补偿模式选择
    Uint16 softPwm;                 // A5-03    随机PWM
    Uint16 cbcEnable;               // A5-04    逐波限流使能
    Uint16 curSampleDelayComp;      // A5-05    电流检测延时补偿
    Uint16 uvPoint;                 // A5-06    欠压点设置
    Uint16 svcMode;                 // A5-07    SVC优化选择 0-不优化  1-优化模式1  2-优化模式2
    Uint16 deadTimeSet;             // A5-08    死区时间调整-1140V专用
    Uint16 ovPointSet;              // A5-09    过压点设置

      
//======================================
// A6 模拟量曲线
    Uint16 curveSet4P1[8];          // A6-00    --A6-07  曲线4，4点，最大值，最小值，2个中间点
    Uint16 curveSet4P2[8];          // A6-08    --A6-15  曲线5，4点
    Uint16 rsvdA41[8];

    struct AI_JUMP aiJumpSet[AI_NUMBER]; // A6-24 --A6-29, AI1, AI2, AI3跳跃

// A7 保留               
    Uint16 plcEnable;               // A7-00 PLC卡功能选择
    Uint16 outPortControl;          // A7-01 输出端子控制
    Uint16 plcAI3Cfg;               // A7-02 PLC AI3功能配置
    Uint16 fmpValue;                // A7-03 FMP输出 
    Uint16 ao1Value;                // A7-04 AO1输出
    Uint16 inPortOut;               // A7-05 开关量输出
    Uint16 plcFrqSet;               // A7-06 PLC卡频率给定
    Uint16 plcTorqueSet;            // A7-07 PLC卡转矩给定
    Uint16 plcCmd;                  // A7-08 PLC卡命令给定
    Uint16 plcErrorCode;            // A7-09 PLC卡故障给定
    Uint16 rsvdA7[2];

    
// A8 保留  
    Uint16 p2pEnable;               // A8-00 点对点通讯功能选择
    Uint16 p2pTypeSel;              // A8-01 主从选择
    Uint16 p2pSendDataSel;          // A8-02 主机发送数据   0:输出转矩  1:运行频率  2:设定频率  3:反馈速度
    Uint16 p2pRevDataSel;           // A8-03 从机接收数据   0:转矩给定  1:频率给定  
    Uint16 p2pRevOffset;            // A8-04 接受数据零偏
    Uint16 p2pRevGain;              // A8-05 接收数据增益
    Uint16 p2pOverTime;             // A8-06 点对点通讯中断检测时间
    Uint16 p2pSendPeriod;           // A8-07 点对点通讯主机数据发送周期
    
// A9 保留  
    Uint16 A9[A9NUM];               // A9

// AA 矢量优化参数

    Uint16 AA[AANUM];               // AA
    #if 0
    Uint16 motorCtrlM1;             // AA-00 励磁调整方式
    Uint16 motorCtrlM2;             // AA-01 最大电压计算方式
    Uint16 motorCtrlM3;             // AA-02 励磁电流调节器KP
    Uint16 motorCtrlM4;             // AA-03 励磁电流调节器KI
    Uint16 motorCtrlM5;             // AA-04 励磁电流正向修正量
    Uint16 motorCtrlM6;             // AA-05 励磁电流反向修正量
    Uint16 motorCtrlM7;             // AA-06 转差调节上限
    Uint16 motorCtrlM8;             // AA-07 转差调节增益
    Uint16 motorCtrlM9;             // AA-08 互感增益
    Uint16 motorCtrlM10;            // AA-09 输出频率修正模式
    Uint16 motorCtrlM11;            // AA-10 电压修正阈值调整
    Uint16 motorCtrlM12;            // AA-11 电压修正增益
    Uint16 motorCtrlM13;            // AA-12 速度环调整
    Uint16 motorCtrlM14;            // AA-13 旋变检测滤波
    Uint16 motorCtrlM15;            // AA-14 旋变角度补偿
    Uint16 motorCtrlM16;            // AA-15 SVC转矩控制优化
    #endif
// AB VF优化参数    
    Uint16 AB[ABNUM];                // AB
    #if 0
    Uint16 vfCtrlM2;                // AB-01 DPWM切换下限频率   86
    Uint16 vfCtrlM3;                // AB-02 死区补偿优化开启   87
    Uint16 vfCtrlM4;                // AB-03 死区钳位补偿系数    1
    Uint16 vfCtrlM5;                // AB-04 钳位优化下限频率   101
    Uint16 vfCtrlM6;                // AB-05 钳位优化上限频率   102
    Uint16 vfCtrlM7;                // AB-06 振荡抑制模式       89
    Uint16 vfCtrlM8;                // AB-07 振荡抑制幅值调整   90
    #endif
// AC AIAO校正
    struct ANALOG_CALIBRATE_CURVE aiCalibrateCurve[AI_NUMBER];  // AC-00    ----AC-11, AI1/2/3校正曲线
    struct ANALOG_CALIBRATE_CURVE aoCalibrateCurve[AO_NUMBER];  // AC-12    ----AC-19, AO1/AO2校正曲线
    
// AD 保留    
    Uint16 AD[ADNUM]; 

// AE AIAO出厂校正值
    struct ANALOG_CALIBRATE_CURVE aiFactoryCalibrateCurve[AI_NUMBER];   // AE-00 
    struct ANALOG_CALIBRATE_CURVE aoFactoryCalibrateCurve[AO_NUMBER];   // AE-12

    Uint16 AF[AFNUM];               // AF
                                    
//======================================
    Uint16 b0[B0NUM];               // B0
    Uint16 b1[B1NUM];               // B1
    Uint16 b2[B2NUM];               // B2
    Uint16 b3[B3NUM];               // B3

//======================================
    Uint16 b4[B4NUM];               // B4
    Uint16 b5[B5NUM];               // B5
    Uint16 b6[B6NUM];               // B6
    Uint16 b7[B7NUM];               // B7
    
//======================================
    Uint16 b8[B8NUM];               // B8
    Uint16 b9[B9NUM];               // B9
    Uint16 ba[BANUM];               // BA
    Uint16 bb[BBNUM];               // BB

//======================================
    Uint16 bc[BCNUM];               // BC
    Uint16 bd[BDNUM];               // BD
    Uint16 be[BENUM];               // BE
    Uint16 bf[BFNUM];               // BF
//======================================


//======================================
    Uint16 c0[C0NUM];               // C0
    Uint16 c1[C1NUM];               // C1
    Uint16 c2[C2NUM];               // C2
    Uint16 c3[C3NUM];               // C3

//======================================
    Uint16 c4[C4NUM];               // C4
    Uint16 c5[C5NUM];               // C5
    Uint16 c6[C6NUM];               // C6
    Uint16 c7[C7NUM];               // C7

//======================================
    Uint16 c8[C8NUM];               // C8
    Uint16 c9[C9NUM];               // C9
    Uint16 ca[CANUM];               // CA
    Uint16 cb[CBNUM];               // CB

//======================================
    Uint16 cc[CCNUM];               // CC   
    Uint16 cd[CDNUM];               // CD
    Uint16 ce[CENUM];               // CE
    Uint16 cf[CFNUM];               // CF
//======================================

//======================================
// eepromCheckWord
    Uint16 rsvd4All;                // 保留，放在最前面
    Uint16 eepromCheckWord1;        //        eepromCheckWord1
    Uint16 eepromCheckWord2;        //        eepromCheckWord2
    Uint16 aiaoChkWord;             // AIAO出厂校正

//======================================
// REMEMBER 掉电保存，共48个
    Uint16 extendType;                  // FR-00  extendType
    Uint16 plcStepRem;                  // FR-01  PLC当前step
    Uint16 plcTimeHighRem;              // FR-02  PLC当前step运行的时间，高位
    Uint16 plcTimeLowRem;               // FR-03  PLC当前step运行的时间，低位
    Uint16 dispParaRunBit;              // FR-04  运行时LED显示参数的bit位值
    Uint16 dispParaStopBit;             // FR-05  停机时LED显示参数的bit位置
    Uint16 runTimeAddupSec;             // FR-06  累计运行时间的s(秒)
    Uint16 counterTicker;               // FR-07  计数器输入的ticker
    Uint16 lengthTickerRemainder;       // FR-08  长度计数器的tickerDelta的Remainder
    Uint16 frqComm;                     // FR-09  通讯修改频率值, 100.00%-maxFrq
    Uint16 upDownFrqRem;                // FR-10  UP/DOWN的频率
    Uint16 pmsmRotorPos;                // FR-11  同步机转子位置
    Uint16 powerAddupInt;               // FR-12  累计耗电量辅助计算
    Uint16 powerUpTimeAddupSec;         // FR-13  累计上电时间的s(秒)
    Uint16 errorFrqUnit;                // FR-14  故障时频率记录
    Uint16 saveUserParaFlag1;           // FR-15  已保存用户参数标志1
    Uint16 saveUserParaFlag2;           // FR-16  已保存用户参数标志2
    Uint16 speedFdbDir;                 // FR-17  电机反馈速度方向
    Uint16 rsvdRem[2];                  // FR-18~FR-19
    Uint16 rsvdRem1[23];                // 预留
    Uint16 remPOffMotorCtrl[REM_P_OFF_MOTOR];  // FR-43~FR-47  性能使用的掉电记忆
//======================================
    Uint16 u0[U0NUM];               // U0 显示
    Uint16 u1[U1NUM];               // U1
    Uint16 u2[U2NUM];               // U2
    Uint16 u3[U3NUM];               // U3

//======================================
    Uint16 u4[U4NUM];               // U4
    Uint16 u5[U5NUM];               // U5
    Uint16 u6[U6NUM];               // U6
    Uint16 u7[U7NUM];               // U7

//======================================
    Uint16 u8[U8NUM];               // U8
    Uint16 u9[U9NUM];               // U9
    Uint16 ua[UANUM];               // UA
    Uint16 ub[UBNUM];               // UB

//======================================
    Uint16 uc[UCNUM];               // UC
    Uint16 ud[UDNUM];               // UD
    Uint16 ue[UENUM];               // UE
    Uint16 uf[UFNUM];               // UF, 显示，性能调试使用
//======================================
};


//=====================================================================
//
// 功能码的定义。
// 联合体，成员分别为数组，结构体，结构体
// 于是，一个功能码的访问，有三种方式:
// funcCode.all[index]     index = GetCodeIndex(funcCode.code.presetFrq);
// funcCode.group.f0[8]    index = GetCodeIndex(funcCode.group.f0[8]);
// funcCode.code.presetFrq
// 
//=====================================================================
typedef union FUNCCODE_ALL_UNION
{
    Uint16 all[FNUM_ALL];

    struct FUNCCODE_GROUP group;

    struct FUNCCODE_CODE code;
} FUNCCODE_ALL;


// 主辅频率源选择
#define FUNCCODE_frqXySrc_FC                0   // 功能码设定，掉电不记忆
#define FUNCCODE_frqXySrc_FC_P_OFF_REM      1   // 功能码设定，掉电记忆
#define FUNCCODE_frqXySrc_AI1               2   // AI1
#define FUNCCODE_frqXySrc_AI2               3   // AI2
#define FUNCCODE_frqXySrc_AI3               4   // AI3
#define FUNCCODE_frqXySrc_PULSE             5   // PULSE脉冲设定(DI5)
#define FUNCCODE_frqXySrc_MULTI_SET         6   // 多段指令
#define FUNCCODE_frqXySrc_PLC               7   // PLC
#define FUNCCODE_frqXySrc_PID               8   // PID
#define FUNCCODE_frqXySrc_COMM              9   // 通讯设定

// 辅助频率源Y范围选择
#define FUNCCODE_frqYRangeBase_MAX_FRQ      0   // 相对于最大频率
#define FUNCCODE_frqYRangeBase_FRQ_X        1   // 相对于主频率源X

// 频率源(切换关系)选择
#define FUNCCODE_frqCalcSrc_X               0   // 主频率源X
#define FUNCCODE_frqCalcSrc_COMPOSE         1   // 主辅运算结果
#define FUNCCODE_frqCalcSrc_X_OR_Y          2   // 主 <--> 辅
#define FUNCCODE_frqCalcSrc_X_OR_COMPOSE    3   // 主 <--> 主辅运算结果
#define FUNCCODE_frqCalcSrc_Y_OR_COMPOSE    4   // 辅 <--> 主辅运算结果

// 主辅频率运算关系
#define FUNCCODE_frqCalcSrc_ADD             0   // 主 + 辅
#define FUNCCODE_frqCalcSrc_SUBTRATION      1   // 主 - 辅
#define FUNCCODE_frqCalcSrc_MAX             2   // MAX(主, 辅)
#define FUNCCODE_frqCalcSrc_MIN             3   // MIN(主, 辅)
#define FUNCCODE_frqCalcSrc_4               4   // 
#define FUNCCODE_frqCalcSrc_5               5   // 

// 上限频率源
#define FUNCCODE_upperFrqSrc_FC         0   // 功能码设定
#define FUNCCODE_upperFrqSrc_AI1        1   // AI1
#define FUNCCODE_upperFrqSrc_AI2        2   // AI2
#define FUNCCODE_upperFrqSrc_AI3        3   // AI3
#define FUNCCODE_upperFrqSrc_PULSE      4   // PULSE脉冲设定(DI5)
#define FUNCCODE_upperFrqSrc_COMM       5   // 通讯给定

// 频率指令小数点
#define FUNCCODE_frqPoint_1             0   // 0: 0个小数点，1Hz
#define FUNCCODE_frqPoint_0_1           1   // 1: 1个小数点，0.1Hz
#define FUNCCODE_frqPoint_0_01          2   // 2: 2个小数点，0.01Hz

// 载波频率调整选择
//#define FUNCCODE_autoCarrierFrq_0

// 加减速时间的单位
#define FUNCCODE_accDecTimeUnit_0POINT  0   // 0个小数点，1s
#define FUNCCODE_accDecTimeUnit_1POINT  1   // 1个小数点，0.1s
#define FUNCCODE_accDecTimeUnit_2POINT  2   // 2个小数点，0.01s

// 数值设定频率记忆设定
#define FUNCCODE_frqRemMode_POWEROFF_NO     0   // 掉电不记忆
#define FUNCCODE_frqRemMode_POWEROFF_YES    1   // 掉电记忆
#define FUNCCODE_frqRemMode_STOP_NO         0   // 停机不记忆
#define FUNCCODE_frqRemMode_STOP_YES        1   // 停机记忆

// 加减速方式
#define FUNCCODE_accDecSpdCurve_LINE        0   // 直线加减速
#define FUNCCODE_accDecSpdCurve_S_CURVE_A   1   // S曲线1，普通二次方
#define FUNCCODE_accDecSpdCurve_S_CURVE_B   2   // S曲线2，参考三菱S曲线B
#define ACC_DEC_LINE    FUNCCODE_accDecSpdCurve_LINE
#define ACC_DEC_SA      FUNCCODE_accDecSpdCurve_S_CURVE_A
#define ACC_DEC_SB      FUNCCODE_accDecSpdCurve_S_CURVE_B

// 转矩上限源
#define FUNCCODE_upperTorqueSrc_FC      0   // 功能码设定
#define FUNCCODE_upperTorqueSrc_AI1     1   // AI1
#define FUNCCODE_upperTorqueSrc_AI2     2   // AI2
#define FUNCCODE_upperTorqueSrc_AI3     3   // AI3
#define FUNCCODE_upperTorqueSrc_PULSE   4   // PULSE
#define FUNCCODE_upperTorqueSrc_COMM    5   // 通讯
#define FUNCCODE_upperTorqueSrc_MIN_AI1_AI2 6  // min(ai1,ai2)
#define FUNCCODE_upperTorqueSrc_MAX_AI1_AI2 7  // max(ai1,ai2)

// FVC的PG卡选择, 0-QEP1(本地PG),1-QEP2(扩展PG)
#define FUNCCODE_fvcPgSrc_QEP1          0   // QEP1
#define FUNCCODE_fvcPgSrc_QEP2          1   // QEP2, 扩展PG卡

#define TIME_UNIT_ACC_DEC_SPEED         100 // 加减速时间单位, ms


// VF曲线设定
#define FUNCCODE_vfCurve_Line               0   // 直线VF
#define FUNCCODE_vfCurve_DOT                1   // 多点VF
#define FUNCCODE_vfCurve_SQUARE             2   // 平方VF
#define FUNCCODE_vfCurve_ALL_SEPARATE       10  // VF完全分离模式
#define FUNCCODE_vfCurve_HALF_SEPARATE      11  // VF半分离模式

// vfVoltageSrc, VF分离的电压源
#define FUNCCODE_vfVoltageSrc_FC            0   // 功能码设定
#define FUNCCODE_vfVoltageSrc_AI1           1   // AI1
#define FUNCCODE_vfVoltageSrc_AI2           2   // AI2
#define FUNCCODE_vfVoltageSrc_AI3           3   // AI3
#define FUNCCODE_vfVoltageSrc_PULSE         4   // PULSE脉冲设定(DI5)
#define FUNCCODE_vfVoltageSrc_MULTI_SET     5   // 多段指令
#define FUNCCODE_vfVoltageSrc_PLC           6   // PLC
#define FUNCCODE_vfVoltageSrc_PID           7   // PID
#define FUNCCODE_vfVoltageSrc_COMM          8   // 通讯设定

// 位置控制选择
#define FUNCCODE_posCtrl_NONE               0   // 非位置控制
#define FUNCCODE_posCtrl_POSITION_CTRL      1   // 位置控制
#define FUNCCODE_posCtrl_SWITCH_TO_PC       2   // 速度/转矩控制<->位置控制
#define FUNCCODE_posCtrl_SWITCH_FROM_PC     3   // 位置控制<->速度/转矩控制

// 位置控制模式
#define FUNCCODE_pcMode_PCMD            0   // Pcmd
#define FUNCCODE_pcMode_APTP            1   // APTP
#define FUNCCODE_pcMode_SWITCH_TO_APTP  2   // Pcmd<->AP2P

// 位置指令脉冲方式
#define FUNCCODE_pcPulseType_PULSE_AND_DIR  0   // 脉冲+方向
#define FUNCCODE_pcPulseType_QUADRATURE     1   // 2路正交脉冲
#define FUNCCODE_pcPulseType_CW_AND_CCW     2   // CW+CCW

// 定位控制模式
#define FUNCCODE_aptpMode_RELATIVE      0   // 相对式
#define FUNCCODE_aptpMode_ABSOLUTE      1   // 绝对式
#define FUNCCODE_aptpMode_INDEX         2   // 分度盘

// 位置控制零点选择
#define FUNCCODE_pcZeroSelect_ENCODER   0   // 编码器index信号
#define FUNCCODE_pcZeroSelect_DI        1   // DI端子

// PG卡安装位置
#define FUNCCODE_pgLocation_MOTOR       0   // 电机轴
#define FUNCCODE_pgLocation_AXIS        1   // 机床主轴

//=====================================================================
// (电机)控制方式
#define FUNCCODE_motorCtrlMode_SVC  0   // SVC
#define FUNCCODE_motorCtrlMode_FVC  1   // FVC
#define FUNCCODE_motorCtrlMode_VF   2   // VF

// 命令源选择
#define FUNCCODE_runSrc_PANEL       0   // 操作面板控制通道
#define FUNCCODE_runSrc_DI          1   // 端子命令通道
#define FUNCCODE_runSrc_COMM        2   // 串行口通讯控制通道
#define FUNCCODE_runSrc_AUTO_RUN    3   // 上电运行

// 运行方向
#define FUNCCODE_runDir_NO_REVERSE      0   // 方向一致
#define FUNCCODE_runDir_REVERSE         1   // 方向相反

// 调谐选择
#define FUNCCODE_tuneCmd_NONE           0   // 无操作
#define FUNCCODE_tuneCmd_ACI_STATIC     1   // 异步机静止调谐
#define FUNCCODE_tuneCmd_ACI_WHOLE      2   // 异步机完整调谐
#define FUNCCODE_tuneCmd_PMSM_11        11  // 同步机
#define FUNCCODE_tuneCmd_PMSM_12        12  // 同步机
#define FUNCCODE_tuneCmd_PMSM_13        13  // 同步机

// 端子命令方式
#define FUNCCODE_diControlMode_2LINE1   0   // 两线式1
#define FUNCCODE_diControlMode_2LINE2   1   // 两线式2
#define FUNCCODE_diControlMode_3LINE1   2   // 三线式1
#define FUNCCODE_diControlMode_3LINE2   3   // 三线式2

// 多功能端子输出选择
#define FUNCCODE_fmOutSelect_PULSE      0   // FMP脉冲输出
#define FUNCCODE_fmOutSelect_DO         1   // DO
#define FUNCCODE_fmOutSelect_AO         2   // AO

// 启动方式
#define FUNCCODE_startMode_DIRECT_START 0   // 直接启动
#define FUNCCODE_startMode_SPEED_TRACK  1   // 转速跟踪再启动
#define FUNCCODE_startMode_FORE_MAG     2   // 异步机励磁启动

// 停机方式
#define FUNCCODE_stopMode_DEC_STOP      0   // 减速停机
#define FUNCCODE_stopMode_FREESTOP      1   // 自由停机
#define FUNCCODE_stopMode_HURRY_STOP    2   // 急停停机

// 频率低于下限频率运行动作
#define FUNCCODE_lowerDeal_RUN_LOWER    0   // 以下限频率运行
#define FUNCCODE_lowerDeal_DELAY_STOP   1   // 延时停机
#define FUNCCODE_lowerDeal_RUN_ZERO     2   // 零速运行

// 设定运行时间到达动作选择
#define FUNCCODE_runTimeOverAct_RUN     0   // 继续运行
#define FUNCCODE_runTimeOverAct_STOP    1   // 停机

// 设定上电时间到达动作选择
#define FUNCCODE_powerUpTimeOverAct_RUN     0   // 继续运行
#define FUNCCODE_powerUpTimeOverAct_STOP    1   // 停机

// PID给定源
#define FUNCCODE_pidSetSrc_FC               0   // 功能码设定
#define FUNCCODE_pidSetSrc_AI1              1   // AI1
#define FUNCCODE_pidSetSrc_AI2              2   // AI2
#define FUNCCODE_pidSetSrc_AI3              3   // AI3
#define FUNCCODE_pidSetSrc_PULSE            4   // PULSE
#define FUNCCODE_pidSetSrc_COMM             5   // 通讯
#define FUNCCODE_pidSetSrc_MULTI_SET        6   // 多段指令

// PID反馈源
#define FUNCCODE_pidFdbSrc_AI1              0   // AI1
#define FUNCCODE_pidFdbSrc_AI2              1   // AI2
#define FUNCCODE_pidFdbSrc_AI3              2   // AI3
#define FUNCCODE_pidFdbSrc_AI1_SUB_AI2      3   // AI1-AI2
#define FUNCCODE_pidFdbSrc_PULSE            4   // PULSE
#define FUNCCODE_pidFdbSrc_COMM             5   // 通讯
#define FUNCCODE_pidFdbSrc_AI1_ADD_AI2      6   // AI1+AI2
#define FUNCCODE_pidFdbSrc_MAX_AI           7   // MAX(|AI1|, |AI2|)
#define FUNCCODE_pidFdbSrc_MIN_AI           8   // MIN(|AI1|, |AI2|)

// PID参数切换条件
#define FUNCCODE_pidParaChgCondition_NO         0   // 不切换
#define FUNCCODE_pidParaChgCondition_DI         1   // DI端子
#define FUNCCODE_pidParaChgCondition_PID_ERROR  2   // 根据偏差自动切换

// PID运算模式
#define FUNCCODE_pidCalcMode_NO             0   // 停机时不运算
#define FUNCCODE_pidCalcMode_YES            1   // 停机时运算

// 摆频设定方式
#define FUNCCODE_swingBaseMode_AGAIN_FRQSETAIM  0   // 相对于中心频率(设定频率)
#define FUNCCODE_swingBaseMode_AGAIN_MAXFRQ     1   // 相对于最大频率

// MF.K键功能选择
#define FUNCCODE_mfkKeyFunc_NONE        0   // MF.K键功能无效
#define FUNCCODE_mfkKeyFunc_SWITCH      1   // 与操作面板通道切换
#define FUNCCODE_mfkKeyFunc_REVERSE     2   // 正反转切换
#define FUNCCODE_mfkKeyFunc_FWD_JOG     3   // 正转点动命令
#define FUNCCODE_mfkKeyFunc_REV_JOG     4   // 反转点动命令

// STOP/RES键功能
#define FUNCCODE_stopKeyFunc_KEYBOARD   0   // 停机功能仅在键盘控制方式时有效
#define FUNCCODE_stopKeyFunc_ALL        1   // 均有效

// 多段指令0给定方式
#define FUNCCODE_plcFrq0Src_FC          0   // 功能码FC-00给定
#define FUNCCODE_plcFrq0Src_AI1         1   // AI1
#define FUNCCODE_plcFrq0Src_AI2         2   // AI2
#define FUNCCODE_plcFrq0Src_AI3         3   // AI3
#define FUNCCODE_plcFrq0Src_PULSE       4   // PULSE
#define FUNCCODE_plcFrq0Src_PID         5   // PID给定
#define FUNCCODE_plcFrq0Src_PRESET_FRQ  6   // 预置频率

// PLC运行方式
#define FUNCCODE_plcRunMode_ONCE_STOP   0   // 单次运行结束停机
#define FUNCCODE_plcRunMode_ONCE_RUN    1   // 单次运行结束保持终值
#define FUNCCODE_plcRunMode_REPEAT      2   // 一直循环

// PLC掉电记忆选择
#define FUNCCODE_plcPowerOffRemMode_NO_REM  0   // 掉电不记忆
#define FUNCCODE_plcPowerOffRemMode_REM     1   // 掉电记忆
// PLC停机记忆选择
#define FUNCCODE_plcStopRemMode_NO_REM  0   // 掉电不记忆
#define FUNCCODE_plcStopRemMode_REM     1   // 掉电记忆

// PLC运行时间单位
#define FUNCCODE_plcTimeUnit_S      0   // S(秒)
#define FUNCCODE_plcTimeUnit_H      1   // H(小时)

// 电机温度传感器类型
#define FUNCCODE_tempSenorType_NONE         0       // PTC100
#define FUNCCODE_tempSenorType_PTC100       1       // PTC100
#define FUNCCODE_tempSenorType_PTC1000      2       // PTC1000
#define FUNCCODE_tempSenorType_NTC          3       // NTC

// 数据格式
#define FUNCCODE_sciParity_NONE     0   // 无校验(8-N-2)
#define FUNCCODE_sciParity_EVEN     1   // 偶校验(8-E-1)
#define FUNCCODE_sciParity_ODD      2   // 奇校验(8-O-1)
#define FUNCCODE_sciParity_NONE1    3   // 无校验(8-N-1)



// 功能码的时间单位
// 注意，程序中为了减小不必要的计算和空间占用，部分使用了
// X * (TIME_UNIT_WAIT_STOP_BRAKE / RUN_CTRL_PERIOD) 的方式
// 而不是，(X * TIME_UNIT_WAIT_STOP_BRAKE) / RUN_CTRL_PERIOD
// 之后修改这些时间单位，可能有必要修改。
#define TIME_UNIT_SEC_PER_HOUR          3600    // 1hour = 3600sec
#define TIME_UNIT_MIN_PER_HOUR          60      // 1hour = 60min
#define TIME_UNIT_SEC_PER_MIN           60      // 1min  = 60sec
#define TIME_UNIT_MS_PER_SEC            1000    // 1s = 1000ms

#define TIME_UNIT_VF_VOL_ACC_TIME       100     // VF分离的电压上升时间

#define TIME_UNIT_AI_PULSE_IN_FILTER    10      // AI,pulseIn滤波时间, ms
#define TIME_UNIT_DI_DELAY              100     // DI输出延迟时间, ms
#define TIME_UNIT_DO_DELAY              100     // DO输出延迟时间, ms
#define TIME_UNIT_START_FRQ_WAIT        100      // 启动频率保持时间，ms
#define TIME_UNIT_START_BRAKE           100     // 启动直流制动时间，ms
#define TIME_UNIT_WAIT_STOP_BRAKE       100     // 停机直流制动等待时间，ms
#define TIME_UNIT_STOP_BRAKE            100     // 停机直流制动时间，ms
#define TIME_UNIT_ZERO_SPEED_DEAD       100     // 正反转死区时间
#define TIME_UNIT_LOWER_STOP_DELAY      100     // 频率低于下限频率时停机的延迟时间
#define TIME_UNIT_PID_SET_CHANGE        10      // PID给定变化时间
#define TIME_UNIT_PID_FILTER            10      // PID反馈，输出滤波时间
#define TIME_UNIT_PID_INIT              10      // PID初值保持时间
#define TIME_UNIT_PID_FDB_LOSE          100     // PID反馈丢失检测时间
#define TIME_UNIT_SWING_PERIOD          100      // 摆频周期
#define TIME_UNIT_sciCommOverTime       100     // 通讯超时时间
#define TIME_UNIT_ERR_AUTO_RST_DELAY    100     // 故障自动复位间隔时间，ms
#define TIME_UNIT_ERR_AUTO_RST_CLR      (TIME_UNIT_SEC_PER_HOUR*100UL) // 故障自动复位次数清除时间, 0.1h
#define TIME_UNIT_P_OFF_VOL_BACK        10      // 瞬停不停电压回升判断时间
#define TIME_UNIT_PLC                   100     // PLC运行时间单位

#define TIME_UNIT_ACC_DEC_SPEED_SERVO   10      // 伺服加减速时间单位
#define TIME_UNIT_WAKE_UP               100     // 唤醒时间的单位
#define TIME_UNIT_DORMANT               100     // 休眠时间的单位
#define TIME_UNIT_CURRENT_CHK           10      // 电流检测时间单位
#define TIME_UNIT_TORQUE_CTRL_ACC_DEC   10      // 转矩控制时间单位
//=====================================================================

#if 0//F_DEBUG_RAM
#define ACC_DEC_T_INIT1  ((Uint32)2*TIME_UNIT_MS_PER_SEC/TIME_UNIT_ACC_DEC_SPEED)    // 加减速时间出厂值，2s，机型 <= 20
#define ACC_DEC_T_INIT2  ((Uint32)5*TIME_UNIT_MS_PER_SEC/TIME_UNIT_ACC_DEC_SPEED)    // 5s，机型 > 20
#else
#define ACC_DEC_T_INIT1  ((Uint32)10*TIME_UNIT_MS_PER_SEC/TIME_UNIT_ACC_DEC_SPEED)   // 加减速时间出厂值，20s，机型 <= 20
#define ACC_DEC_T_INIT2  ((Uint32)30*TIME_UNIT_MS_PER_SEC/TIME_UNIT_ACC_DEC_SPEED)   // 50s，机型 > 20
#endif

#define RATING_FRQ_INIT_0   50      // 电机额定频率，0个小数点
#define RATING_FRQ_INIT_1   500     // 电机额定频率，1个小数点
#define RATING_FRQ_INIT_2   5000    // 电机额定频率，2个小数点
#define BAUD_NUM_MAX        12   // 波特率选择范围的最大值
//#define BAUD_NUM_MAX 10
#define PARA_INIT_MODE_MAX  501       // 参数初始化上限值
#define INV_TYPE_MAX   30

#define MENU_MODE_MAX       3   // 菜单模式的最大值

//=====================================================================
// 有些功能码的上下限是其它某个功能码，这里是在funcCode中的index

// 程序中使用的一些功能码的index
//= 如果增加/删除了功能码，这里要修改!
#define FACTORY_PWD_INDEX      (GetCodeIndex(funcCode.code.factoryPassword))   // FF-00 厂家密码
#define INV_TYPE_INDEX         (GetCodeIndex(funcCode.code.inverterType))      // FF-01 变频器机型
#define RATING_POWER_INVERTER_INDEX  (GetCodeIndex(funcCode.code.inverterPower))     // FF-03 变频器功率
#define FUNCCODE_FACTORY_START_INDEX     (GetCodeIndex(funcCode.group.ff[0]))            // FF组的开始
#define FUNCCODE_FACTORY_END_INDEX       (GetCodeIndex(funcCode.group.ff[FFNUM - 1]))    // FF组的结束

#define FC_MOTOR1_START_INDEX   (GetCodeIndex(funcCode.code.motorParaM1.all[0]))      // 第1电机参数的起始
#define FC_MOTOR1_END_INDEX     (GetCodeIndex(funcCode.code.pgParaM1.all[sizeof(struct PG_PARA_STRUCT) - 1]))  // 第1电机参数的结束

#define FC_MOTOR2_START_INDEX   (GetCodeIndex(funcCode.code.motorFcM2.motorPara.all[0]))      // 第2电机参数的起始
#define FC_MOTOR2_END_INDEX     (GetCodeIndex(funcCode.code.motorFcM2.pgPara.all[sizeof(struct PG_PARA_STRUCT) - 1]))  // 第2电机参数的结束

#define FC_MOTOR3_START_INDEX   (GetCodeIndex(funcCode.code.motorFcM3.motorPara.all[0]))      // 第3电机参数的起始
#define FC_MOTOR3_END_INDEX     (GetCodeIndex(funcCode.code.motorFcM3.pgPara.all[sizeof(struct PG_PARA_STRUCT) - 1]))  // 第3电机参数的结束

#define FC_MOTOR4_START_INDEX   (GetCodeIndex(funcCode.code.motorFcM4.motorPara.all[0]))      // 第4电机参数的起始
#define FC_MOTOR4_END_INDEX     (GetCodeIndex(funcCode.code.motorFcM4.pgPara.all[sizeof(struct PG_PARA_STRUCT) - 1]))  // 第4电机参数的结束



#define PRESET_FRQ_INDEX        (GetCodeIndex(funcCode.code.presetFrq))      // F0-08   预置频率
#define MAX_FRQ_INDEX           (GetCodeIndex(funcCode.code.maxFrq))         // F0-10   最大频率
#define UPPER_FRQ_INDEX         (GetCodeIndex(funcCode.code.upperFrq))       // F0-12   上限频率
#define LOWER_FRQ_INDEX         (GetCodeIndex(funcCode.code.lowerFrq))       // F0-14   下限频率
#define ACC_TIME1_INDEX         (GetCodeIndex(funcCode.code.accTime1))       // F0-17   加速时间1
#define DEC_TIME1_INDEX         (GetCodeIndex(funcCode.code.decTime1))       // F0-18   减速时间1
#define CARRIER_FRQ_INDEX       (GetCodeIndex(funcCode.code.carrierFrq))     // F0-15   载波频率

#define RATING_POWER_INDEX      (GetCodeIndex(funcCode.code.motorParaM1.elem.ratingPower))    // 电机额定功率
#define RATING_VOL_INDEX        (GetCodeIndex(funcCode.code.motorParaM1.elem.ratingVoltage))  // 电机额定电压
#define RATING_CUR_INDEX        (GetCodeIndex(funcCode.code.motorParaM1.elem.ratingCurrent))  // 电机额定电流
#define RATING_CUR_INDEX2       (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.ratingCurrent))    // 第2电机额定电流
#define RATING_CUR_INDEX3       (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.ratingCurrent))   // 第3电机额定电流
#define RATING_CUR_INDEX4       (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.ratingCurrent))   // 第4电机额定电流


#define RATING_FRQ_INDEX        (GetCodeIndex(funcCode.code.motorParaM1.elem.ratingFrq))      // 电机额定频率

#define ZERO_LOAD_CURRENT_INDEX (GetCodeIndex(funcCode.code.motorParaM1.elem.zeroLoadCurrent))// 空载电流
#define STATOR_RESISTANCE_INDEX (GetCodeIndex(funcCode.code.motorParaM1.elem.statorResistance))// 定子电阻

#define ANTI_VIBRATE_GAIN_INDEX (GetCodeIndex(funcCode.code.antiVibrateGain))// F3-11   振荡抑制增益
#define ANTI_VIBRATE_GAIN_MOTOR2_INDEX (GetCodeIndex(funcCode.code.motorFcM2.antiVibrateGain))// A4-52   振荡抑制增益
#define ANTI_VIBRATE_GAIN_MOTOR3_INDEX (GetCodeIndex(funcCode.code.motorFcM3.antiVibrateGain))// A5-52   振荡抑制增益
#define ANTI_VIBRATE_GAIN_MOTOR4_INDEX (GetCodeIndex(funcCode.code.motorFcM4.antiVibrateGain))// A6-52   振荡抑制增益

#define TUNE_CMD_INDEX_1  (GetCodeIndex(funcCode.code.tuneCmd))  // 调谐
#define TUNE_CMD_INDEX_2  (GetCodeIndex(funcCode.code.motorFcM2.tuneCmd))  // 调谐
#define TUNE_CMD_INDEX_3  (GetCodeIndex(funcCode.code.motorFcM3.tuneCmd))  // 调谐
#define TUNE_CMD_INDEX_4  (GetCodeIndex(funcCode.code.motorFcM4.tuneCmd))  // 调谐

#define VC_CHG_FRQ1_INDEX (GetCodeIndex(funcCode.code.vcParaM1.vcSpdLoopChgFrq1))  // 矢量控制速度环 切换频率1
#define VC_CHG_FRQ2_INDEX (GetCodeIndex(funcCode.code.vcParaM1.vcSpdLoopChgFrq2))  // 矢量控制速度环 切换频率2

#define VC_CHG_FRQ1_INDEX2 (GetCodeIndex(funcCode.code.motorFcM2.vcPara.vcSpdLoopChgFrq1))  // 矢量控制速度环 切换频率1
#define VC_CHG_FRQ2_INDEX2 (GetCodeIndex(funcCode.code.motorFcM2.vcPara.vcSpdLoopChgFrq2))  // 矢量控制速度环 切换频率2

#define VC_CHG_FRQ1_INDEX3 (GetCodeIndex(funcCode.code.motorFcM3.vcPara.vcSpdLoopChgFrq1))  // 矢量控制速度环 切换频率1
#define VC_CHG_FRQ2_INDEX3 (GetCodeIndex(funcCode.code.motorFcM3.vcPara.vcSpdLoopChgFrq2))  // 矢量控制速度环 切换频率2

#define VC_CHG_FRQ1_INDEX4 (GetCodeIndex(funcCode.code.motorFcM4.vcPara.vcSpdLoopChgFrq1))  // 矢量控制速度环 切换频率1
#define VC_CHG_FRQ2_INDEX4 (GetCodeIndex(funcCode.code.motorFcM4.vcPara.vcSpdLoopChgFrq2))  // 矢量控制速度环 切换频率2


#define TORQUE_BOOST_INDEX      (GetCodeIndex(funcCode.code.torqueBoost))    // F1-05   转矩提升

#define TORQUE_BOOST_MOTOR2_INDEX      (GetCodeIndex(funcCode.code.motorFcM2.torqueBoost))    // D0-52   第2电机转矩提升
#define TORQUE_BOOST_MOTOR3_INDEX      (GetCodeIndex(funcCode.code.motorFcM3.torqueBoost))    // D0-52   第3电机转矩提升
#define TORQUE_BOOST_MOTOR4_INDEX      (GetCodeIndex(funcCode.code.motorFcM4.torqueBoost))    // D0-52   第4电机转矩提升

#define SVC_MODE_INDX           (GetCodeIndex(funcCode.code.svcMode))        // A5-07 SVC模式选择
#define OV_POINT_SET_INDEX      (GetCodeIndex(funcCode.code.ovPointSet))     // A5-09 过压点设置

#define VF_FRQ1_INDEX           (GetCodeIndex(funcCode.code.vfFrq1))         // F3-03   多点VF频率点1
#define VF_FRQ2_INDEX           (GetCodeIndex(funcCode.code.vfFrq2))         // F3-05   多点VF频率点2
#define VF_FRQ3_INDEX           (GetCodeIndex(funcCode.code.vfFrq3))         // F3-07   多点VF频率点3


#define CURVE1_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet2P1[0]))       // F2-08   AI1最小输入
#define CURVE1_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet2P1[2]))       // F2-10   AI1最大输入
#define CURVE2_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet2P2[0]))       // F2-14   AI2最小输入
#define CURVE2_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet2P2[2]))       // F2-16   AI2最大输入
#define CURVE3_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet2P3[0]))       // F2-14   AI3最小输入
#define CURVE3_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet2P3[2]))       // F2-16   AI3最大输入
#define PULSE_IN_MIN_INDEX      (GetCodeIndex(funcCode.code.curveSet2P4[0]))     // F2-20   PULSE最小输入
#define PULSE_IN_MAX_INDEX      (GetCodeIndex(funcCode.code.curveSet2P4[2]))     // F2-22   PULSE最大输入

#define CURVE4_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet4P1[0]))       // A8-00   AI4最小输入
#define CURVE4_INFLEX1_INDEX    (GetCodeIndex(funcCode.code.curveSet4P1[2]))       // A8-02   AI4拐点1输入
#define CURVE4_INFLEX2_INDEX    (GetCodeIndex(funcCode.code.curveSet4P1[4]))       // A8-04   AI4拐点2输入
#define CURVE4_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet4P1[6]))       // A8-06   AI4最大输入
#define CURVE5_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet4P2[0]))       // A8-08   AI5最小输入
#define CURVE5_INFLEX1_INDEX    (GetCodeIndex(funcCode.code.curveSet4P2[2]))       // A8-10   AI5拐点1输入
#define CURVE5_INFLEX2_INDEX    (GetCodeIndex(funcCode.code.curveSet4P2[4]))       // A8-12   AI5拐点2输入
#define CURVE5_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet4P2[6]))       // A8-14   AI5最大输入


#define ACC_TIME2_INDEX         (GetCodeIndex(funcCode.code.accTime2))       // F8-03 加速时间2
#define DEC_TIME2_INDEX         (GetCodeIndex(funcCode.code.decTime2))       // F8-04 减速时间2

#define ACC_TIME3_INDEX         (GetCodeIndex(funcCode.code.accTime3))       // F8-05 加速时间3
#define DEC_TIME3_INDEX         (GetCodeIndex(funcCode.code.decTime3))       // F8-06 减速时间3

#define ACC_TIME4_INDEX         (GetCodeIndex(funcCode.code.accTime4))       // F8-07 加速时间4
#define DEC_TIME4_INDEX         (GetCodeIndex(funcCode.code.decTime4))       // F8-08 减速时间4

#define RUN_TIME_ADDUP_INDEX    (GetCodeIndex(funcCode.code.runTimeAddup))     // F7-09  累计运行时间
#define POWER_TIME_ADDUP_INDEX  (GetCodeIndex(funcCode.code.powerUpTimeAddup)) // F7-13  累计上电时间
#define POWER_ADDUP_INDEX       (GetCodeIndex(funcCode.code.powerAddup))       // F7-14  累计耗电量


#define AI1_LIMIT               (GetCodeIndex(funcCode.code.ai1VoltageLimit)) //  F8-45  AI保护下限
#define AI1_UPPER               (GetCodeIndex(funcCode.code.ai1VoltageUpper)) //  F8-46  AI保护上限

#define PID_PARA_CHG_DELTA1_MAX (GetCodeIndex(funcCode.code.pidParaChgDelta2))  // FA-20  PID参数切换偏差2
#define PID_PARA_CHG_DELTA2_MIN (GetCodeIndex(funcCode.code.pidParaChgDelta1))  // FA-19  PID参数切换偏差1

#define DORMANT_UPPER           (GetCodeIndex(funcCode.code.wakeUpFrq))       // 休眠频率上限
#define WAKE_UP_LIMIT           (GetCodeIndex(funcCode.code.dormantFrq))      // 唤醒频率下限
#define RADIATOR_TEMP_INDEX     (GetCodeIndex(funcCode.code.radiatorTemp))   // FB-19   逆变器模块散热器温度
#define ERROR_LATEST1_INDEX     (GetCodeIndex(funcCode.code.errorLatest1))   // FB-20   第一次故障类型
#define ERROR_LATEST2_INDEX     (GetCodeIndex(funcCode.code.errorLatest2))   // FB-21   第二次故障类型
#define ERROR_LATEST3_INDEX     (GetCodeIndex(funcCode.code.errorLatest3))   // FB-22   (最近一次)第三次故障类型
#define ERROR_FRQ_INDEX         (GetCodeIndex(funcCode.code.errorScene3.elem.errorFrq))       // FB-23   故障时频率
#define ERROR_CURRENT_INDEX     (GetCodeIndex(funcCode.code.errorScene3.elem.errorCurrent))   // FB-24   故障时电流
#define ERROR_UDC_INDEX         (GetCodeIndex(funcCode.code.errorScene3.elem.errorGeneratrixVoltage)) // FB-25 故障时母线电压
#define ERROR_DI_STATUS_INDEX   (GetCodeIndex(funcCode.code.errorScene3.elem.errorDiStatus))  // FB-26   故障时输入端子状态
#define ERROR_DO_STATUS_INDEX   (GetCodeIndex(funcCode.code.errorScene3.elem.errorDoStatus))  // FB-27   故障时输出端子状态
#define LAST_ERROR_RECORD_INDEX (GetCodeIndex(funcCode.code.errorScene1.all[sizeof(struct ERROR_SCENE_STRUCT) - 1]))  // 最后一个故障记录

#define MIN_CBC_TIME_INDEX       (GetCodeIndex(funcCode.code.cbcMinTime))         // A0-14   逐波限流时间下限
#define MAX_CBC_TIME_INDEX       (GetCodeIndex(funcCode.code.cbcMaxTime))         // A0-15   逐波限流时间上限



#define PC_LOOP_CHG_FRQ1_I      (GetCodeIndex(funcCode.code.pcLoopChgFrq1))     //          切换频率1
#define PC_LOOP_CHG_FRQ2_I      (GetCodeIndex(funcCode.code.pcLoopChgFrq2))     //          切换频率2

#define EEPROM_CHECK_INDEX      (GetCodeIndex(funcCode.code.eepromCheckWord1))  // eepromCheckWord1

#define RUN_TIME_ADDUP_SEC_INDEX    (GetCodeIndex(funcCode.code.runTimeAddupSec))   // FR-07 F209  累计运行时间的s


#define EEPROM_CHECK_INDEX1     (GetCodeIndex(funcCode.code.eepromCheckWord1))  // eepromCheckWord1
#define EEPROM_CHECK_INDEX2     (GetCodeIndex(funcCode.code.eepromCheckWord2))  // eepromCheckWord2

#define SAVE_USER_PARA_PARA1    (GetCodeIndex(funcCode.code.saveUserParaFlag1))
#define SAVE_USER_PARA_PARA2    (GetCodeIndex(funcCode.code.saveUserParaFlag2))

#define AI_AO_CHK_FLAG          (GetCodeIndex(funcCode.code.aiaoChkWord))       // AIAO校正标志
#define AI_AO_CALIB_START       (GetCodeIndex(funcCode.code.aiFactoryCalibrateCurve[0].before1))  // aiao厂家校正开始
#define AI_AO_CALIB_STOP        (GetCodeIndex(funcCode.code.aoFactoryCalibrateCurve[0].after2))   // aiao厂家校正结束

#define AI1_CALB_START          (GetCodeIndex(funcCode.code.aiFactoryCalibrateCurve[0].before1))
#define AI2_CALB_STOP           (GetCodeIndex(funcCode.code.aiFactoryCalibrateCurve[1].after2))
#define AO1_CALB_START          (GetCodeIndex(funcCode.code.aoFactoryCalibrateCurve[0].before1))
#define AO1_CALB_STOP           (GetCodeIndex(funcCode.code.aoFactoryCalibrateCurve[0].after2)) 

//-------------------------------
#define FC_GROUP_FACTORY    FUNCCODE_GROUP_FF   // 厂家参数组
#define FC_GROUP_FC_MANAGE  FUNCCODE_GROUP_FP   // 功能码管理
#define FC_START_GROUP      FUNCCODE_GROUP_F0   // 功能码组显示的第1组
//--------------------------------


extern FUNCCODE_ALL funcCode;           // 功能码的RAM值
//extern FUNCCODE_ALL funcCodeEeprom;     // 功能码的EEPROM值

extern const Uint16 funcCodeGradeSum[];

extern Uint16 saveEepromIndex;    // DP卡掉电存储
extern const Uint16 ovVoltageInitValue[];
extern const Uint16 funcCodeGradeAll[];
extern Uint16 funcCodeGradeCurMenuMode[];

#endif  // __F_FUNCCODE_H__





