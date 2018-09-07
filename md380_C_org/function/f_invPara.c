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
// Time-stamp: <2012-08-14 12:01:32  Shisheng.Zhi, 0354>
//
//======================================================================


#include "f_menu.h"
#include "f_eeprom.h"
#include "f_comm.h"
#include "f_invPara.h"

#if F_DEBUG_RAM
#define DEBUG_F_TUNE                0       // 调谐
#elif 1
#define DEBUG_F_TUNE                1       // 调谐
#endif

struct INV_PARA invPara;

// 机型过压点
const Uint16 ovVoltageInitValue[INV_TYPE_VOLTAGE_NUM] = {8100,4000,4000,8900,13000,20000};

// 注意! 机型下限要与invTypeParaTablexxxx的起始机型一致
const struct INV_TYPE_LIMIT invTypeLimitTable[INV_TYPE_VOLTAGE_NUM] = 
{
    {380,  INV_TYPE_LOWER_LIMIT_T380, INV_TYPE_UPPER_LIMIT_T380},   // 0, T380
    {220,  INV_TYPE_LOWER_LIMIT_T220, INV_TYPE_UPPER_LIMIT_T220},   // 1, T220
    {220,  INV_TYPE_LOWER_LIMIT_S220, INV_TYPE_UPPER_LIMIT_S220},   // 2, S220
    {480,  INV_TYPE_LOWER_LIMIT_T480, INV_TYPE_UPPER_LIMIT_T480},   // 3, T480
    {690,  INV_TYPE_LOWER_LIMIT_T690, INV_TYPE_UPPER_LIMIT_T690},   // 4, T690
    {1140,  INV_TYPE_LOWER_LIMIT_T1140, INV_TYPE_UPPER_LIMIT_T1140},// 5, T1140
};

// 第1电机参数的index
// 机型相关，使用 0-6， 电机额定功率，电机额定电流，定子电阻，转子电阻，漏感，互感，空载电流
// 功率相关，使用 1-6， 额定电流，定子电阻，转子电阻，漏感，互感，空载电流
// 调谐，    使用 2-10，定子电阻，转子电阻，漏感，互感，空载电流，同步机Rs，Ld, Lq, UV两相增益偏差
// 要与 INV_TYPE_RELATED_PARA 的顺序一致
const Uint16 motor1ParaIndex[] = 
{
    // 基本电机参数
    GetCodeIndex(funcCode.code.motorParaM1.elem.ratingPower),        // 0  F1-01  电机额定功率 
    GetCodeIndex(funcCode.code.motorParaM1.elem.ratingCurrent),      // 1  F1-03  电机额定电流
    // 异步机参数
    GetCodeIndex(funcCode.code.motorParaM1.elem.statorResistance),   // 2  F1-06  定子电阻
    GetCodeIndex(funcCode.code.motorParaM1.elem.rotorResistance),    // 3  F1-07  转子电阻
    GetCodeIndex(funcCode.code.motorParaM1.elem.leakInductance),     // 4  F1-08  漏感抗
    GetCodeIndex(funcCode.code.motorParaM1.elem.mutualInductance),   // 5  F1-09  互感抗   
    GetCodeIndex(funcCode.code.motorParaM1.elem.zeroLoadCurrent),    // 6  F1-10  空载电流
    // 同步机参数
    GetCodeIndex(funcCode.code.motorParaM1.elem.pmsmRs),             // 7  F1-16  同步机定子电阻
    GetCodeIndex(funcCode.code.motorParaM1.elem.pmsmLd),             // 8  F1-17  同步机d轴电感
    GetCodeIndex(funcCode.code.motorParaM1.elem.pmsmLq),             // 9  F1-18  同步机q轴电感
    GetCodeIndex(funcCode.code.motorParaM1.elem.pmsmRsLdUnit),       // 10 F1-19  同步电机电阻、电感单位
    GetCodeIndex(funcCode.code.motorParaM1.elem.pmsmCoeff),          // 11 F1-20  同步机反电动势系数
    // PI参数
    GetCodeIndex(funcCode.code.vcParaM1.mAcrKp),                     // 12 F2-13  M轴电流环Kp
    GetCodeIndex(funcCode.code.vcParaM1.mAcrKi),                     // 13 F2-14  M轴电流环Ki
    GetCodeIndex(funcCode.code.vcParaM1.tAcrKp),                     // 14 F2-15  T轴电流环Kp
    GetCodeIndex(funcCode.code.vcParaM1.tAcrKi),                     // 15 F2-16  T轴电流环Ki    
    GetCodeIndex(funcCode.code.motorParaM1.elem.pmsmCheckTime),      // 16 F1-21  同步机输出缺相检测时间
    // PG卡参数
    GetCodeIndex(funcCode.code.pgParaM1.elem.enCoderDir),            // 17 F1-30  旋变方向
    GetCodeIndex(funcCode.code.pgParaM1.elem.enCoderAngle),          // 18 F1-31  编码器安装角
    GetCodeIndex(funcCode.code.uvGainWarp),                          // 19 FF-05  UV两相增益偏差
    GetCodeIndex(funcCode.code.pgParaM1.elem.uvwSignDir),            // 20 F1-32  UVW信号方向
    GetCodeIndex(funcCode.code.pgParaM1.elem.uvwSignAngle),          // 21 F1-33  UVW信号零点位置角
};


//---------------------------------------------------------------------------------------------------
// 变频器机型相关的参数
#if (DEBUG_F_MOTOR_POWER_RELATE || DEBUG_F_INV_TYPE_RELATE)
// 机型相关参数，三相380V
const INV_TYPE_RELATED_PARA invTypeParaTableT380[INV_TYPE_LENGTH_T380] =
{
// 载波频率 功率  电流  定子电阻 转子电阻 漏感抗 互感抗 空载电流 
// carrier  power  rateCur Rs      rr      lo     lm     IM     振荡抑制增益    // 功率    // 机型 
    { 60,   7,     210,    9134,   4281,   3799,  4077,  155,   0,          },  // 0.75    // 8  
    { 60,   15,    380,    4421,   2813,   2065,  3457,  187,   0,          },  // 1.5     // 9  
    { 60,   22,    510,    2706,   1543,   1011,  2450,  267,   0,          },  // 2.2     // 10 
    { 60,   37,    900,    1204,   908,    528,   1586,  424,   0,          },  // 3.7     // 11 
    { 60,   55,    1300,   804,    708,    475,   1206,  518,   0,          },  // 5.5     // 12 
    { 60,   75,    1700,   610,    584,    430,   1069,  630,   0,          },  // 7.5     // 13 
    { 60,   110,   2500,   410,    220,    260,   776,   890,   0,          },  // 11      // 14 
    { 40,   150,   3200,   273,    200,    220,   578,   1160,  0,          },  // 15      // 15 
    { 40,   185,   3700,   200,    130,    150,   537,   1280,  0,          },  // 18.5    // 16 
    { 40,   220,   4500,   152,    138,    115,   342,   1965,  0,          },  // 22      // 17 
    { 40,   300,   6000,   110,    98,     100,   322,   2070,  0,          },  // 30      // 18 
    { 40,   370,   7500,   69,     74,     60,    308,   2190,  20,         },  // 37      // 19 
    { 40,   450,   9000,   61,     66,     52,    223,   3020,  20,         },  // 45      // 20 
    { 30,   550,   11000,  50,     60,     41,    183,   3690,  30,         },  // 55      // 21 
// carrier  power  rateCur Rs      rr      lo     lm     IM     振荡抑制增益    // 功率    // 机型 
    { 20,   750,   1520,   300,    460,    290,   1640,  420 ,  30,         },  // 75      // 22 
    { 20,   900,   1760,   220,    380,    200,   1400,  490 ,  40,         },  // 90      // 23 
    { 20,   1100,  2100,   120,    320,    150,   1200,  580 ,  40,         },  // 110     // 24 
    { 20,   1320,  2530,   100,    270,    130,   1060,  680 ,  40,         },  // 132     // 25 
    { 20,   1600,  3040,   70,     240,    110,   800,   815 ,  50,         },  // 160     // 26 
    { 20,   2000,  3800,   40,     210,    90,    600,   950 ,  50,         },  // 200     // 27
    { 20,   2200,  4260,   28,     190,    75,    450,   1050,  50,         },  // 220     // 28 
    { 20,   2500,  4650,   20,     170,    55,    350,   1200,  50,         },  // 250     // 29 
    { 20,   2800,  5200,   14,     150,    45,    280,   1300,  50,         },  // 280     // 30
    { 20,   3150,  5850,   10,     120,    30,    200,   1500,  50,         },  // 315     // 31
    { 20,   3550,  6500,   8,      100,    22,    140,   1640,  50,         },  // 355     // 32
    { 20,   4000,  7250,   6,      85,     17,    100,   1750,  50,         },  // 400     // 33
    { 20,   4500,  8200,   4,      63,     13,    70,    1900,  50,         },  // 450     // 34
    
    { 20,   5000,  7250,   4,      63,     13,    70,    1900,  50,         },  // 500     // 35
    { 20,   5500,  8200,   4,      63,     13,    70,    1900,  50,         },  // 550     // 36
    { 20,   6300,  9360,   4,      63,     13,    70,    1900,  50,         },  // 630     // 37
        
};

// 机型相关参数，三相220V
const INV_TYPE_RELATED_PARA invTypeParaTableT220[INV_TYPE_LENGTH_T220] = 
{
// carrier  power  rateCur Rs      rr      lo     lm     IM     振荡抑制增益    // 功率    // 机型  // 对应380V机型 功率
    { 60,   4,     210,    9134,   4281,   3799,  4077,  155,   0,          },  // 0.4kw   // 107      8  0.75kw
    { 60,   7,     380,    4421,   2813,   2065,  3457,  187,   0,          },  // 0.75kw  // 108      9  1.5kw
    { 60,   11,    510,    2706,   1543,   1011,  2450,  267,   0,          },  // 1.1kw   // 109      10 2.2kw
    { 60,   22,    900,    1204,   908,    528,   1586,  424,   0,          },  // 2.2kw   // 110      11 3.7kw
    { 60,   37,    1300,   804,    708,    475,   1206,  518,   0,          },  // 3.7kw   // 111      12 5.5kw
    { 60,   55,    2500,   410,    220,    260,   776,   890,   0,          },  // 5.5kw   // 112      14 11kw
    { 40,   75,    3200,   273,    200,    220,   578,   1160,  0,          },  // 7.5kw   // 113      15 15kw
    { 40,   110,   4500,   152,    138,    115,   342,   1965,  0,          },  // 11kw    // 114      17 22kw
    { 40,   150,   6000,   110,    98,     100,   322,   2070,  0,          },  // 15kw    // 115      18 30kw
    { 40,   185,   7500,   69,     74,     60,    308,   2190,  20,         },  // 18.5kw  // 116      19 37kw
    { 40,   220,   9000,   61,     66,     52,    223,   3020,  20,         },  // 22kw    // 117      20 45kw
    { 30,   300,   11000,  50,     60,     41,    183,   3690,  30,         },  // 30kw    // 118      21 55kw
    // carrier  power  rateCur Rs      rr      lo     lm     IM     振荡抑制增益    // 功率    // 机型 
    { 20,   370,   1520,   300,    460,    290,   1640,  420 ,  30,         },  // 37kw    // 119      22 75kw
    { 20,   450,   1760,   220,    380,    200,   1400,  490 ,  40,         },  // 45kw    // 120      23 90kw
    { 20,   550,   2100,   120,    320,    150,   1200,  580 ,  40,         },  // 55kw    // 121      24 110kw
    { 20,   750,   3040,   70,     240,    110,   800,   815 ,  50,         },  // 75kw    // 122      26 160kw

};

// 机型相关参数，两相220V
const INV_TYPE_RELATED_PARA invTypeParaTableS220[INV_TYPE_LENGTH_S220] =
{
// carrier  power  rateCur Rs      rr      lo     lm     IM     振荡抑制增益    // 功率    // 机型 
    { 60,   2,     170,    15330,  10440,  4700,  3720,  90,    0,          },  // 0.2kw   // 201  
    { 60,   5,     230,    4007,   2462,   1358,  1821,  202,   0,          },  // 0.55kw  // 202  
    { 60,   7,     400,    2963,   1697,   1008,  1472,  255,   0,          },  // 0.75kw  // 203  
    { 60,   15,    700,    1534,   1043,   628,   1308,  292,   0,          },  // 1.5kw   // 204  
    { 60,   22,    960,    1025,   680,    340,   885,   450,   0,          },  // 2.2kw   // 205  
    { 60,   37,    1700,   735,    450,    268,   598,   637,   0,          },  // 3.7kw   // 206  
    { 60,   55,    2500,   435,    350,    180,   350,   1100,  0,          },  // 5.5kw   // 207  
};

// 机型相关参数，三相480V
const INV_TYPE_RELATED_PARA invTypeParaTableT480[INV_TYPE_LENGTH_T480] = 
{
// carrier  power  rateCur Rs      rr      lo     lm     IM     振荡抑制增益    // 功率    // 机型 
//    { 60,   2,     170,    15330,  10440,  4700,  3720,  90,    0,          },  // 0.2kw   // 301  

// 载波频率 功率  电流  定子电阻 转子电阻 漏感抗 互感抗 空载电流 
// carrier  power  rateCur Rs      rr      lo     lm     IM     振荡抑制增益    // 功率    // 机型 
    { 60,   7,     210,    9134,   4281,   3799,  4077,  155,   0,          },  // 0.75    // 308  
    { 60,   15,    380,    4421,   2813,   2065,  3457,  187,   0,          },  // 1.5     // 309  
    { 60,   22,    510,    2706,   1543,   1011,  2450,  267,   0,          },  // 2.2     // 310 
    { 60,   37,    900,    1204,   908,    528,   1586,  424,   0,          },  // 3.7     // 311 
    { 60,   55,    1300,   804,    708,    475,   1206,  518,   0,          },  // 5.5     // 312 
    { 60,   75,    1700,   610,    584,    430,   1069,  630,   0,          },  // 7.5     // 313 
    { 60,   110,   2500,   410,    220,    260,   776,   890,   0,          },  // 11      // 314 
    { 40,   150,   3200,   273,    200,    220,   578,   1160,  0,          },  // 15      // 315 
    { 40,   185,   3700,   200,    130,    150,   537,   1280,  0,          },  // 18.5    // 316 
    { 40,   220,   4500,   152,    138,    115,   342,   1965,  0,          },  // 22      // 317 
    { 40,   300,   6000,   110,    98,     100,   322,   2070,  0,          },  // 30      // 318 
    { 40,   370,   7500,   69,     74,     60,    308,   2190,  20,         },  // 37      // 319 
    { 40,   450,   9000,   61,     66,     52,    223,   3020,  20,         },  // 45      // 320 
    { 30,   550,   11000,  50,     60,     41,    183,   3690,  30,         },  // 55      // 321 
// carrier  power  rateCur Rs      rr      lo     lm     IM     振荡抑制增益    // 功率    // 机型 
    { 20,   750,   1520,   300,    460,    290,   1640,  420 ,  30,         },  // 75      // 322 
    { 20,   900,   1760,   220,    380,    200,   1400,  490 ,  40,         },  // 90      // 323 
    { 20,   1100,  2100,   120,    320,    150,   1200,  580 ,  40,         },  // 110     // 324 
    { 20,   1320,  2530,   100,    270,    130,   1060,  680 ,  40,         },  // 132     // 325 
    { 20,   1600,  3040,   70,     240,    110,   800,   815 ,  50,         },  // 160     // 326 
    { 20,   2000,  3800,   40,     210,    90,    600,   950 ,  50,         },  // 200     // 327
    { 20,   2200,  4260,   28,     190,    75,    450,   1050,  50,         },  // 220     // 328 
    { 20,   2500,  4650,   20,     170,    55,    350,   1200,  50,         },  // 250     // 329 
    { 20,   2800,  5200,   14,     150,    45,    280,   1300,  50,         },  // 280     // 330
    { 20,   3150,  5850,   10,     120,    30,    200,   1500,  50,         },  // 315     // 331
    { 20,   3550,  6500,   8,      100,    22,    140,   1640,  50,         },  // 355     // 332
    { 20,   4000,  7250,   6,      85,     17,    100,   1750,  50,         },  // 400     // 333
    { 20,   4500,  8200,   4,      63,     13,    70,    1900,  50,         },  // 450     // 334
#if 0    
    { 20,   5000,  7250,   4,      63,     13,    70,    1900,  50,         },  // 500     // 335
    { 20,   5500,  8200,   4,      63,     13,    70,    1900,  50,         },  // 550     // 336
#endif        
};

// 机型相关参数，三相690V
const INV_TYPE_RELATED_PARA invTypeParaTableT690[INV_TYPE_LENGTH_T690] = 
{
// carrier  power  rateCur Rs      rr      lo     lm     IM     振荡抑制增益    // 功率    // 机型 
#if 0
    { 60,   7,     80,     9134,   4281,   3799,  4077,  155,   0,          },  // 0.75    // 408  
    { 60,   15,    160,    4000,   2653,   2130,  3622,  187,   0,          },  // 1.5     // 409  
    { 60,   22,    240,    2706,   1543,   1011,  2450,  267,   0,          },  // 2.2     // 410 
    { 60,   37,    400,    1050,   1000,   750,   1870,  380,   0,          },  // 3.7     // 411 
    { 60,   55,    600,    680,    640,    600,   1250,  518,   0,          },  // 5.5     // 412 
    { 60,   75,    800,    610,    584,    430,   1069,  630,   0,          },  // 7.5     // 413 
    { 40,   110,   1200,   410,    220,    260,   776,   890,   0,          },  // 11      // 414 
    { 40,   150,   1600,   273,    200,    220,   578,   1160,  0,          },  // 15      // 415 
    { 40,   185,   2000,   200,    130,    150,   537,   1280,  0,          },  // 18.5    // 416 
    { 40,   220,   2400,   152,    138,    115,   342,   1965,  0,          },  // 22      // 417 
    { 40,   300,   3300,   110,    98,     100,   322,   2070,  0,          },  // 30      // 418 
    { 40,   370,   4100,   69,     74,     60,    308,   2190,  20,         },  // 37      // 419 
    { 40,   450,   5000,   61,     66,     52,    223,   3020,  20,         },  // 45      // 420 
#endif        
    { 20,   550,   6200,   50,     60,     41,    183,   3690,  30,         },  // 55      // 421 
// carrier  power  rateCur Rs      rr      lo     lm     IM     振荡抑制增益    // 功率    // 机型 
    { 20,   750,   850,    300,    460,    290,   1640,  420,   30,         },  // 75      // 422 
    { 20,   900,   1020,   220,    380,    200,   1400,  490,   40,         },  // 90      // 423 
    { 20,   1100,  1250,   553,    862,    620,   4106,  302,   40,         },  // 110     // 424 
    { 20,   1320,  1500,   100,    270,    130,   1060,  680,   40,         },  // 132     // 425 
    { 20,   1600,  1750,   70,     240,    110,   800,   815,   50,         },  // 160     // 426 
    { 20,   2000,  2150,   40,     210,    90,    600,   950,   50,         },  // 200     // 427
    { 20,   2200,  2450,   28,     190,    75,    450,   1050,  50,         },  // 220     // 428 
    { 20,   2500,  2600,   20,     170,    55,    350,   1200,  50,         },  // 250     // 429 
    { 20,   2800,  2990,   14,     150,    45,    280,   1300,  50,         },  // 280     // 430
    { 20,   3150,  3300,   10,     120,    30,    200,   1500,  50,         },  // 315     // 431
    { 20,   3550,  3740,   8,      100,    22,    140,   1640,  50,         },  // 355     // 432
    { 20,   4000,  4100,   6,      85,     17,    100,   1750,  50,         },  // 400     // 433
    { 20,   4500,  4650,   4,      63,     13,    70,    1900,  50,         },  // 450     // 434
    { 20,   5000,  5500,   4,      63,     13,    70,    1900,  50,         },  // 500     // 435
    { 20,   5500,  5900,   4,      63,     13,    70,    1900,  50,         },  // 550     // 436
};

// 机型相关参数，三相1140V
const INV_TYPE_RELATED_PARA invTypeParaTableT1140[INV_TYPE_LENGTH_T1140] = 
{
// carrier  power  rateCur Rs      rr      lo     lm     IM     振荡抑制增益   // 功率    // 机型  
    { 10,  370,  2500,    69,     74,     60,    308,   2190,  20,         },  // 37      // 519 
    { 10,   450,  3000,    61,     66,     52,    223,   3020,  20,         },  // 45      // 520 
    { 10,   550,  3700,    50,     60,     41,    183,   3690,  30,         },  // 55      // 521 
// carrier  power  rateCur Rs      rr      lo     lm     IM     振荡抑制增益    // 功率    // 机型 
    { 10,   750,   500,    300,    460,    290,   1640,  420,   30,         },  // 75      // 522 
    { 10,   900,   590,    220,    380,    200,   1400,  490,   40,         },  // 90      // 523 
    { 10,   1100,  700,    553,    862,    620,   4106,  302,   40,         },  // 110     // 524 
    { 10,   1320,  910,    100,    270,    130,   1060,  680,   40,         },  // 132     // 525 
    { 10,   1600,  1060,   70,     240,    110,   800,   815,   50,         },  // 160     // 526 
    { 10,   2000,  1300,   40,     210,    90,    600,   950,   50,         },  // 200     // 527
    { 10,   2200,  1480,   28,     190,    75,    450,   1050,  50,         },  // 220     // 528 
    { 10,   2500,  1570,   20,     170,    55,    350,   1200,  50,         },  // 250     // 529 
    { 10,   2800,  1810,   14,     150,    45,    280,   1300,  50,         },  // 280     // 530
    { 10,   3150,  2000,   10,     120,    30,    200,   1500,  50,         },  // 315     // 531
    { 10,   3550,  2260,   8,      100,    22,    140,   1640,  50,         },  // 355     // 532
    { 10,   4000,  2480,   6,      85,     17,    100,   1750,  50,         },  // 400     // 533
    { 10,   4500,  2810,   4,      63,     13,    70,    1900,  50,         },  // 450     // 534
    { 10,   5000,  3330,   4,      63,     13,    70,    1900,  50,         },  // 500     // 535
    { 10,   5600,  3480,   4,      63,     13,    70,    1900,  50,         },  // 560     // 536
    { 10,   6300,  4000,   4,      63,     13,    70,    1900,  50,         },  // 630     // 537
};

// 机型相关参数
const INV_TYPE_RELATED_PARA * pInvTypeParaTable[INV_TYPE_VOLTAGE_NUM] = 
{
    invTypeParaTableT380,       // 三相380V
    invTypeParaTableT220,       // 三相220V
    invTypeParaTableS220,       // 两相220V
    invTypeParaTableT480,       // 三相480V
    invTypeParaTableT690,       // 三相690V
    invTypeParaTableT1140,      // 三相1140V
};

// 不同电压等级电流小数点临界点
const Uint16 INV_TYPE_POINT_LIMIT_TABLE[INV_TYPE_VOLTAGE_NUM] = 
{
    INV_TYPE_POINT_LIMIT_T380,       // 三相380V
    INV_TYPE_POINT_LIMIT_T220,       // 三相220V
    INV_TYPE_POINT_LIMIT_T380,       // 两相220V
    INV_TYPE_POINT_LIMIT_T380,       // 三相480V
    INV_TYPE_POINT_LIMIT_T380,       // 三相690V
    INV_TYPE_POINT_LIMIT_T380,       // 三相1140V
};

// 不同电压等级加减速时间出厂值为大值的起始机型
const Uint16 INV_TYPE_BIG_ACC_DEC_START_TABLE[INV_TYPE_VOLTAGE_NUM] = 
{
    INV_TYPE_BIG_ACC_DEC_START_T380,       // 三相380V
    INV_TYPE_BIG_ACC_DEC_START_T220,       // 三相220V
    INV_TYPE_BIG_ACC_DEC_START_T380,       // 两相220V
    INV_TYPE_BIG_ACC_DEC_START_T380,       // 三相480V
    INV_TYPE_BIG_ACC_DEC_START_T380,       // 三相690V
    INV_TYPE_BIG_ACC_DEC_START_T380,       // 三相1140V
};


#endif
//---------------------------------------------------------------------------------------------------




//---------------------------------------------------------------------------------------------------
// 电机2参数的index，相对于电机1参数index的增量
#define MOTOR2_INDEX_INC_TO_MOTOR1                          \
(GetCodeIndex(funcCode.code.motorFcM2.motorPara.all[0]) -    \
GetCodeIndex(funcCode.code.motorParaM1.all[0]))              \

// 电机3参数的index，相对于电机1参数index的增量
#define MOTOR3_INDEX_INC_TO_MOTOR1                          \
(GetCodeIndex(funcCode.code.motorFcM3.motorPara.all[0]) -    \
GetCodeIndex(funcCode.code.motorParaM1.all[0]))              \

// 电机4参数的index，相对于电机1参数index的增量
#define MOTOR4_INDEX_INC_TO_MOTOR1                          \
(GetCodeIndex(funcCode.code.motorFcM4.motorPara.all[0]) -    \
GetCodeIndex(funcCode.code.motorParaM1.all[0]))              \

// 第1/2/3/4电机的电机参数，相对与第1电机参数的CodeIndex增量
const Uint16 motorSnIndexIncToMotor1[4] = 
{
    0,                              // 第1电机
    MOTOR2_INDEX_INC_TO_MOTOR1,     // 第2电机
    MOTOR3_INDEX_INC_TO_MOTOR1,     // 第3电机
    MOTOR4_INDEX_INC_TO_MOTOR1,     // 第4电机
};
//---------------------------------------------------------------------------------------------------





//=====================================================================
//
// 修改变频器机型时的处理
//
// 改变变频器机型时，需要修改的功能码(MD280目前有14个)：
// 1、电机参数：额定功率、额定电压、额定电流、额定转速、额定频率、空载电流、定子电阻、转子电阻，漏感，互感
// 2、变频器功率FF-03
// 3、转矩提升
// 4、振荡抑制增益
// 5、载波频率
// 6、加减速时间1，加减速时间2
//
//=====================================================================
void InverterTypeRelatedParaDeal(void)
{
#if DEBUG_F_INV_TYPE_RELATE
    Uint16 ratingFrq;
    Uint16 invType;
    int16 i;

    UpdateInvType();            // 更新实际机型、电压等级
    invType = invPara.type;     // 更新

// 电机额定功率，电机额定电流，定子电阻，转子电阻，漏感，互感，空载电流
// 这些参数，每个电机都是单独功能码，INV_TYPE_RELATED_PARA 中也有。
// 
    for (i = 6; i >= 0; i--)
    {
        Uint16 index1;          // 第1电机参数index
        Uint16 index2;          // 第2电机参数index
        Uint16 index3;
        Uint16 index4;
        Uint16 tmp;

        index1 = motor1ParaIndex[i];
        index2 = motor1ParaIndex[i] + MOTOR2_INDEX_INC_TO_MOTOR1;
        index3 = motor1ParaIndex[i] + MOTOR3_INDEX_INC_TO_MOTOR1;
        index4 = motor1ParaIndex[i] + MOTOR4_INDEX_INC_TO_MOTOR1;

        tmp = GetInvParaPointer(invType)->all[i+1];
        
        funcCode.all[index1] = tmp;    // 第1电机参数
        funcCode.all[index2] = tmp;    // 第2电机参数
        funcCode.all[index3] = tmp;    // 第3电机参数
        funcCode.all[index4] = tmp;    // 第4电机参数
        
        SaveOneFuncCode(index1);
        SaveOneFuncCode(index2);
        SaveOneFuncCode(index3);
        SaveOneFuncCode(index4);
    }
    
// 电机额定电压，电机额定频率，额定转速
    ratingFrq = RATING_FRQ_INIT_0 * decNumber[funcCode.code.frqPoint];


    // 第1电机
    funcCode.code.motorParaM1.elem.ratingVoltage = invPara.ratingVoltage;// 电机额定电压
    funcCode.code.motorParaM1.elem.ratingFrq = ratingFrq;                // 电机额定频率
    funcCode.code.motorParaM1.elem.ratingSpeed = RATING_SPEED_RPM;       // 额定转速
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorParaM1.elem.ratingVoltage));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorParaM1.elem.ratingFrq));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorParaM1.elem.ratingSpeed));

    // 第2电机
    funcCode.code.motorFcM2.motorPara.elem.ratingVoltage = invPara.ratingVoltage;
    funcCode.code.motorFcM2.motorPara.elem.ratingFrq = ratingFrq;   
    funcCode.code.motorFcM2.motorPara.elem.ratingSpeed = RATING_SPEED_RPM;
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.ratingVoltage));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.ratingFrq));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.ratingSpeed));

    // 第3电机
    funcCode.code.motorFcM3.motorPara.elem.ratingVoltage = invPara.ratingVoltage;
    funcCode.code.motorFcM3.motorPara.elem.ratingFrq = ratingFrq;   
    funcCode.code.motorFcM3.motorPara.elem.ratingSpeed = RATING_SPEED_RPM;
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.ratingVoltage));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.ratingFrq));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.ratingSpeed));

    // 第4电机
    funcCode.code.motorFcM4.motorPara.elem.ratingVoltage = invPara.ratingVoltage;
    funcCode.code.motorFcM4.motorPara.elem.ratingFrq = ratingFrq;   
    funcCode.code.motorFcM4.motorPara.elem.ratingSpeed = RATING_SPEED_RPM;
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.ratingVoltage));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.ratingFrq));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.ratingSpeed));

// 变频器功率
    funcCode.code.inverterPower = funcCode.code.motorParaM1.elem.ratingPower;
    SaveOneFuncCode(GetCodeIndex(funcCode.code.inverterPower));

// 与机型相关的非电机参数功能码
    InverterTypeRelatedNoMotorParaDeal(invType);

// 出厂值与机型相关的非电机参数功能码，保存至EEPROM
    InverterTypeRelatedNoMotorParaSaveDeal();
#endif
}


//=====================================================================
// 
// 需要参数：
// 1. inverterType
// 2. ratingPower，电机额定功率
// 
// 与机型相关的功能码，非电机参数
// 
//=====================================================================

LOCALF void InverterTypeRelatedNoMotorParaDeal(Uint16 invType)
{
#if DEBUG_F_INV_TYPE_RELATE
    Uint16 accDecInit;
    if (invType < invPara.bitAccDecStart)  // 机型 < 21
    {
        accDecInit = ACC_DEC_T_INIT1;
    }
    else
    {
        accDecInit = ACC_DEC_T_INIT2;
    }

    funcCode.all[ACC_TIME1_INDEX] = accDecInit; // 加速时间1
    funcCode.all[DEC_TIME1_INDEX] = accDecInit; // 减速时间1
    funcCode.all[ACC_TIME2_INDEX] = accDecInit; // 加速时间2
    funcCode.all[DEC_TIME2_INDEX] = accDecInit; // 减速时间2
    funcCode.all[ACC_TIME3_INDEX] = accDecInit; // 加速时间3
    funcCode.all[DEC_TIME3_INDEX] = accDecInit; // 减速时间3
    funcCode.all[ACC_TIME4_INDEX] = accDecInit; // 加速时间4
    funcCode.all[DEC_TIME4_INDEX] = accDecInit; // 减速时间4
    
// 载波频率
    funcCode.all[CARRIER_FRQ_INDEX] = GetInvParaPointer(invType)->elem.carrierFrq;

// 振荡抑制增益
    funcCode.all[ANTI_VIBRATE_GAIN_INDEX] = GetInvParaPointer(invType)->elem.antiVibrateGain;
    funcCode.code.motorFcM2.antiVibrateGain = funcCode.code.antiVibrateGain;
    funcCode.code.motorFcM3.antiVibrateGain = funcCode.code.antiVibrateGain;
    funcCode.code.motorFcM4.antiVibrateGain = funcCode.code.antiVibrateGain;

// 转矩提升。要在功率处理之后
    funcCode.all[TORQUE_BOOST_INDEX] = TorqueBoostDeal(funcCode.code.motorParaM1.elem.ratingPower);
    funcCode.code.motorFcM2.torqueBoost = funcCode.code.torqueBoost;
    funcCode.code.motorFcM3.torqueBoost = funcCode.code.torqueBoost;
    funcCode.code.motorFcM4.torqueBoost = funcCode.code.torqueBoost;

// 过压点
    funcCode.code.ovPointSet = ovVoltageInitValue[invPara.volLevel];
#endif
}



// 出厂值与机型相关的非电机参数功能码，保存至EEPROM
void InverterTypeRelatedNoMotorParaSaveDeal(void)
{
#if DEBUG_F_INV_TYPE_RELATE
    SaveOneFuncCode(ACC_TIME1_INDEX);
    SaveOneFuncCode(DEC_TIME1_INDEX);
    SaveOneFuncCode(ACC_TIME2_INDEX);
    SaveOneFuncCode(DEC_TIME2_INDEX);
    SaveOneFuncCode(ACC_TIME3_INDEX);
    SaveOneFuncCode(DEC_TIME3_INDEX);
    SaveOneFuncCode(ACC_TIME4_INDEX);
    SaveOneFuncCode(DEC_TIME4_INDEX);

    SaveOneFuncCode(CARRIER_FRQ_INDEX);

    SaveOneFuncCode(ANTI_VIBRATE_GAIN_INDEX);
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM2.antiVibrateGain));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM3.antiVibrateGain));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM4.antiVibrateGain));

    SaveOneFuncCode(TORQUE_BOOST_INDEX);
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM2.torqueBoost));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM3.torqueBoost));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM4.torqueBoost));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.ovPointSet));
#endif
}



//=====================================================================
//
// 修改电机额定功率的处理
//
// 修改电机额定功率时，需要相应修改电机参数，包括：
// 1、电机额定电流
// 2、空载电流
// 3、定子电阻
// 4、转矩提升
// 5、转子电阻
// 6、漏感
// 7、互感
//
//=====================================================================
// 根据当前电压等级和机型，获得对应结构体的起始地址
void MotorPowerRelatedParaDeal(Uint16 power, Uint16 motorSn4Power)
{
#if DEBUG_F_MOTOR_POWER_RELATE
    Uint16 i;
    Uint32 flag;
    Uint16 start;           // 搜索的起始机型
    Uint16 end;             // 搜索的结束机型
    Uint16 invTypeOfPower;  // 当前功率所处的机型位置
    int16 j;
    Uint16 dataPointFormat[6] = {1,1,1,1,1,1};
    
    start = invTypeLimitTable[invPara.volLevel].lower;
    end = invTypeLimitTable[invPara.volLevel].upper;

// 已知，机型从小到大，功率也从小到大
    for (i = start; i <= end; i++)
    {
        if (GetInvParaPointer(i)->elem.ratingPower >= power)
            break;
    }

// 比较与谁近
    invTypeOfPower = i;
    if (end < i)            // i = end + 1，超过最大功率
    {
        invTypeOfPower--;
    }
    else if (start < i)     // start < i
    {
        if (power - GetInvParaPointer(i-1)->elem.ratingPower
            < GetInvParaPointer(i)->elem.ratingPower - power) // 注意大小
        {
            invTypeOfPower--;
        }
    }

// 转矩提升
    if (MOTOR_SN_1 == motorSn4Power)        // 第1电机
    {
        funcCode.code.torqueBoost = TorqueBoostDeal(power);
        SaveOneFuncCode(GetCodeIndex(funcCode.code.torqueBoost));
    }
    else if (MOTOR_SN_2 == motorSn4Power)   // 第2电机
    {
        funcCode.code.motorFcM2.torqueBoost = TorqueBoostDeal(power);
        SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM2.torqueBoost));
    }
    else if (MOTOR_SN_3 == motorSn4Power)   // 第3电机
    {
        funcCode.code.motorFcM3.torqueBoost = TorqueBoostDeal(power);
        SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM3.torqueBoost));
    }
    else if (MOTOR_SN_4 == motorSn4Power)   // 第4电机
    {
        funcCode.code.motorFcM4.torqueBoost = TorqueBoostDeal(power);
        SaveOneFuncCode(GetCodeIndex(funcCode.code.motorFcM4.torqueBoost));
    }

    if ((invPara.type > invPara.pointLimit) 
        && (invTypeOfPower <= invPara.pointLimit))
    {
        flag = 10;
        dataPointFormat[0] = 100; 
        dataPointFormat[5] = 100; 
        
    }
    else if ((invPara.type <= invPara.pointLimit) 
        && (invTypeOfPower > invPara.pointLimit))
    {
        flag = 10;
		dataPointFormat[1] = 100; 
        dataPointFormat[2] = 100; 
        dataPointFormat[3] = 100; 
        dataPointFormat[4] = 100; 
    }
    else
    {
        flag = 1;
    }
    
// 额定电流，定子电阻，转子电阻，漏感，互感，空载电流
    for (j = 6; j >= 1; j--)
    {
        Uint16 index = motor1ParaIndex[j] + motorSnIndexIncToMotor1[motorSn4Power];
        
        funcCode.all[index] = (flag*(GetInvParaPointer(invTypeOfPower)->all[j+1]))/dataPointFormat[j-1];    // 第x电机参数
        SaveOneFuncCode(index);
    }
#endif
}



//=====================================================================
//
// 转矩提升处理。与功率有对应关系
// 0.4kW --- 0.75kW 6%
// 1.5kW --- 3.7kW  4%
// 5.5kW --- 7.5kW  3%
//  11kW ---  37kW  2%
//  45kW ---        1%
// 
//=====================================================================
Uint16 TorqueBoostDeal(Uint16 power)
{
#if (DEBUG_F_INV_TYPE_RELATE || DEBUG_F_MOTOR_POWER_RELATE)
    Uint16 torqueBoost;
    
    if (power <= 10)        // P <= 1.0kW
    {
        torqueBoost = 60;   // 6.0%
    }
    else if (power <= 40)   // p <= 4.0kW
    {
        torqueBoost = 40;   // 4.0%
    }
    else if (power <= 75)   // p <= 7.5kW
    {
        torqueBoost = 30;   // 3.0%
    }
    else if (power <= 370)  // p <= 37kW
    {
        torqueBoost = 20;   // 2.0%
    }
    else
    {
        torqueBoost = 10;   // 1.0%
    }

    return torqueBoost;
#endif
}



// 确认机型是否有效
Uint16 ValidateInvType(void)
{
#if DEBUG_F_INV_TYPE_RELATE
    Uint16 i;
    Uint16 upper;
    Uint16 lower;
    Uint16 ret = COMM_ERR_NONE;
    Uint16 invType = funcCode.code.inverterType;

    i = invType / 100;

    upper = invTypeLimitTable[i].upper + (i * 100);
    lower = invTypeLimitTable[i].lower + (i * 100);

    if (!((lower <= invType) && (invType <= upper)))  // 功率不在规定范围
    {
        ret = COMM_ERR_PARA;
    }
    
    return ret;
#endif
}



// input:  funcCode.code.inverterType
// output: invPara
// 根据用户输入机型，获得实际机型(变频器功率)，变频器额定电压
void UpdateInvType(void)
{
    invPara.type = funcCode.code.inverterType % 100;                    // 个位和十位，表示功率
    invPara.volLevel = funcCode.code.inverterType / 100;                // 百位表示电压等级
    invPara.ratingVoltage = invTypeLimitTable[invPara.volLevel].vol;    // 
    invPara.pointLimit = INV_TYPE_POINT_LIMIT_TABLE[invPara.volLevel];  
    invPara.bitAccDecStart = INV_TYPE_BIG_ACC_DEC_START_TABLE[invPara.volLevel];
}



// 保存调谐数据
extern enum MOTOR_SN motorSn;
void SaveTuneData(void)
{
#if DEBUG_F_TUNE
    int16 i;
    
    // 定子电阻，转子电阻，漏感，互感，空载电流
    // 同步机Rs, Ld, Lq
    // UV两相增益偏差
    // UVW信号方向 UVW信号零点位置角
    for (i = sizeof(motor1ParaIndex)-1; i >= 2; i--)
    {
        Uint16 index;               // 需要保存的参数index
        Uint16 index4motorTune;     // 对应电机传递的index
        Uint16 add = motorSnIndexIncToMotor1[motorSn];

        if (i == (sizeof(motor1ParaIndex)-3))    // UV两相增益偏差
        {
            add = 0;    // 所有电机使用同一个功能码
        }
        
        index = *(motor1ParaIndex + i) + add;   // 调谐保存参数从异步机定子电阻开始
        index4motorTune = i - 2;

        funcCode.all[index] = gParaIdToFunctionDataBuff[index4motorTune];
        SaveOneFuncCode(index);
    }
#endif
}







