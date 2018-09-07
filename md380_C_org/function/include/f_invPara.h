//======================================================================
//
// 变频器参数、以及相关参数
// 
// 包括：
// 变频器机型、变频器功率、变频器额定电压
// 载波频率、
// 电机额定功率、电机额定电流、定子电阻、转子电阻、漏感抗、互感抗、空载电流
// 
// 振荡抑制增益
//
// Time-stamp: <2008-08-14 12:01:32  Shisheng.Zhi, 0354>
//
//======================================================================


#ifndef __F_INV_PARA_H__
#define __F_INV_PARA_H__



#if F_DEBUG_RAM
#define DEBUG_F_MOTOR_POWER_RELATE      0   // 电机功率处理
#define DEBUG_F_INV_TYPE_RELATE         0   // 变频器机型处理
#elif 1
#define DEBUG_F_MOTOR_POWER_RELATE      1
#define DEBUG_F_INV_TYPE_RELATE         1
#endif


#define INV_TYPE_VOLTAGE_NUM    6       // _种电压等级

#define INV_TYPE_VOLTAGE_T380   0   // 三相380V
#define INV_TYPE_VOLTAGE_T220   1   // 三相220V
#define INV_TYPE_VOLTAGE_S220   2   // 单相220V
#define INV_TYPE_VOLTAGE_T480   3   // 三相480V
#define INV_TYPE_VOLTAGE_T690   4   // 三相690V
#define INV_TYPE_VOLTAGE_T1140  5   // 三相1140V

// 变频器机型，1-7， 2相
// 变频器机型，>= 8，3相
// 性能传递的母线电压含有一位小数点
#define INV_TYPE_THREE_PHASE_START          8       // 3相的起始机型
#define INV_TYPE_BIG_ACC_DEC_START_T380     21      // 三相380V加减速时间出厂值为大值的起始机型
#define INV_TYPE_BIG_ACC_DEC_START_T220     18      // 三相220V加减速时间出厂值为大值的起始机型
#define INV_TYPE_POINT_LIMIT_T380           21      // 三相380V电流、电机参数的小数点
#define INV_TYPE_POINT_LIMIT_T220           18      // 三相220V电流、电机参数的小数点
#define MOTOR_PARA_POINT_POWER_LIMIT        750     // 电机参数、电流的小数点的功率分界线, 75.0kW。>=此值，
#define RATING_SPEED_RPM                    1460    // 额定转速

#define GetInvParaPointer(invType)            \
(pInvTypeParaTable[invPara.volLevel] + ((invType) - invTypeLimitTable[invPara.volLevel].lower)) \



#define INV_TYPE_LOWER_LIMIT_T380  8    // T380的机型下限
#define INV_TYPE_UPPER_LIMIT_T380  34   // T380的机型上限
#define INV_TYPE_LENGTH_T380                                    \
(INV_TYPE_UPPER_LIMIT_T380 - INV_TYPE_LOWER_LIMIT_T380 + 1)     \

#define INV_TYPE_LOWER_LIMIT_T220  7    // T220的机型下限
#define INV_TYPE_UPPER_LIMIT_T220  22   // T220的机型上限
#define INV_TYPE_LENGTH_T220                                    \
(INV_TYPE_UPPER_LIMIT_T220 - INV_TYPE_LOWER_LIMIT_T220 + 1)     \

#define INV_TYPE_LOWER_LIMIT_S220  1    //  S220的机型下限
#define INV_TYPE_UPPER_LIMIT_S220  7    //  S220的机型上限
#define INV_TYPE_LENGTH_S220                                    \
(INV_TYPE_UPPER_LIMIT_S220 - INV_TYPE_LOWER_LIMIT_S220 + 1)     \

#define INV_TYPE_LOWER_LIMIT_T480  8    // T480的机型下限
#define INV_TYPE_UPPER_LIMIT_T480  34    // T480的机型上限
#define INV_TYPE_LENGTH_T480                                    \
(INV_TYPE_UPPER_LIMIT_T480 - INV_TYPE_LOWER_LIMIT_T480 + 1)     \

#define INV_TYPE_LOWER_LIMIT_T690  21    // T690的机型下限
#define INV_TYPE_UPPER_LIMIT_T690  36   // T690的机型上限
#define INV_TYPE_LENGTH_T690                                    \
(INV_TYPE_UPPER_LIMIT_T690 - INV_TYPE_LOWER_LIMIT_T690 + 1)     \

#define INV_TYPE_LOWER_LIMIT_T1140  19    // T690的机型下限
#define INV_TYPE_UPPER_LIMIT_T1140  37   // T690的机型上限
#define INV_TYPE_LENGTH_T1140                                   \
(INV_TYPE_UPPER_LIMIT_T1140 - INV_TYPE_LOWER_LIMIT_T1140 + 1)   \


// 机型相关参数
// 电机参数的顺序，与功能码要一致
struct INV_TYPE_RELATED_PARA_STRUCT
{
    Uint16 carrierFrq;              // 载波频率

    Uint16 ratingPower;             // 电机额定功率
    Uint16 ratingCurrent;           // 电机额定电流
    Uint16 statorResistance;        // 定子电阻
    Uint16 rotorResistance;         // 转子电阻
    Uint16 leakInductance;          // 漏感抗
    Uint16 mutualInductance;        // 互感抗
    Uint16 zeroLoadCurrent;         // 空载电流

    Uint16 antiVibrateGain;         // 振荡抑制增益
};
typedef union
{
    Uint16 all[sizeof(struct INV_TYPE_RELATED_PARA_STRUCT)];

    struct INV_TYPE_RELATED_PARA_STRUCT elem;
} INV_TYPE_RELATED_PARA;



struct INV_TYPE_RELATED_PARA_ALL
{
    INV_TYPE_RELATED_PARA t380[INV_TYPE_LENGTH_T380];   // T380
    INV_TYPE_RELATED_PARA t220[INV_TYPE_LENGTH_T220];   // T220
    INV_TYPE_RELATED_PARA s220[INV_TYPE_LENGTH_S220];   // S220
    INV_TYPE_RELATED_PARA t480[INV_TYPE_LENGTH_T480];   // T480
    INV_TYPE_RELATED_PARA t690[INV_TYPE_LENGTH_T690];   // T690
    INV_TYPE_RELATED_PARA t1140[INV_TYPE_LENGTH_T1140]; // T1140
};


struct INV_TYPE_LIMIT
{
    Uint16 vol;     // 本电压等级的电压

    Uint16 lower;   // 本电压等级的机型下限
    Uint16 upper;   // 本电压等级的机型上限
};
extern const struct INV_TYPE_LIMIT invTypeLimitTable[];

extern const INV_TYPE_RELATED_PARA * pInvTypeParaTable[];


LOCALD void InverterTypeRelatedNoMotorParaDeal(Uint16 invType);
Uint16 ValidateInvType(void);
void MotorPowerRelatedParaDeal(Uint16 power, Uint16 motorSn4Power);
void InverterTypeRelatedParaDeal(void);     // 修改变频器机型时的处理
void InverterTypeRelatedNoMotorParaSaveDeal(void);




#endif  // __F_INV_PARA_H__


