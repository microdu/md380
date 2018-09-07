/*****************************************************************
 *
 * ¹¦ÄÜÂë¶¨ÒåµÄÍ·ÎÄ¼ş
 * 
 * Time-stamp: <2012-08-20 08:57:13  Shisheng.Zhi, 0354>
 * 
 *
 *
 *
 *****************************************************************/

#ifndef __F_FUNCCODE_H__
#define __F_FUNCCODE_H__



       // 2803x»¹ÊÇ2808Æ½Ì¨
#include "DSP28x_Project.h"

#include "f_debug.h"
#include "f_interface.h"
#include "f_p2p.h"




// Çë²»ÒªËæÒâĞŞ¸ÄÕâÁ½¸öÖµ£¬·ñÔòEEPROMÖĞµÄÖµ»áÈ«²¿(°üÀ¨P0×é¡¢P1×é¡¢¸÷ÖÖ¼ÇÂ¼)»Ö¸´³ö³§Öµ¡£
#define EEPROM_CHECK1       (0x0009)                        // EEPROMĞ£Ñé×Ö1
#define EEPROM_CHECK2       (0xFFFF - EEPROM_CHECK1)        // EEPROMĞ£Ñé×Ö2


#define USER_MENU_GROUP     FUNCCODE_GROUP_FE

// »ñµÃ¹¦ÄÜÂëcodeÔÚÈ«²¿¹¦ÄÜÂëÖĞµÄindex£¬¼´¹¦ÄÜÂëÔÚÊı×éfuncCode.all[]ÖĞµÄÏÂ±ê¡£
// ¸ù¾İ FUNCCODE_ALL µÄ¶¨Òå£¬Ò»¸ö¹¦ÄÜÂëµÄ±íÊ¾ÓĞ4ÖÖ°ì·¨£¬¶ÔÓ¦µÄ»ñµÃindexµÄ°ì·¨£º
// 1. funcCode.all[i]     ---- i
// 2. funcCode.f5[7]      ---- GetCodeIndex(funcCode.group.f5[7])
// 3. group, grade        ---- GetGradeIndex(group, grade)
// 4. funcCode.code.maxFrq---- GetCodeIndex(funcCode.code.maxFrq)
#define GetCodeIndex(code)    ((Uint16)((&(code)) - (&(funcCode.all[0]))))
#define GetGradeIndex(group, grade)  (funcCodeGradeSum[group] + (grade))


//=====================================================================
// ¹¦ÄÜÂëÊôĞÔÎ»¶¨Òå
//
// µ¥Î»
#define ATTRIBUTE_UNIT_HZ_BIT           0
#define ATTRIBUTE_UNIT_A_BIT            1
#define ATTRIBUTE_UNIT_V_BIT            2

// ¶ÁĞ´ÊôĞÔ
#define ATTRIBUTE_READ_AND_WRITE        0   // (ÈÎºÎÊ±ºò)¿ÉĞ´
#define ATTRIBUTE_READ_ONLY_WHEN_RUN    1   // ÔËĞĞÊ±Ö»¶Á
#define ATTRIBUTE_READ_ONLY_ANYTIME     2   // Ö»¶Á

// ¶à¹¦ÄÜÂë×éºÏÊôĞÔ
#define ATTRIBUTE_MULTI_LIMIT_SINGLE    0   // µ¥¶ÀµÄ¹¦ÄÜÂë
#define ATTRIBUTE_MULTI_LIMIT_DEC       1   // ¶à¸ö¹¦ÄÜÂë£¬Ê®½øÖÆ
#define ATTRIBUTE_MULTI_LIMIT_HEX       2   // ¶à¸ö¹¦ÄÜÂë£¬Ê®Áù½øÖÆ

struct  FUNC_ATTRIBUTE_BITS
{                           // bits   description
    Uint16 point:3;         // 2:0    radix point,Ğ¡Êıµã¡£
                            //        0-ÎŞĞ¡Êıµã£¬1-1Î»Ğ¡Êı£¬...£¬4-4Î»Ğ¡Êı
                            //        (0.0000-100,00.000-011,000.00-010,0000.0-001,00000-000)
    Uint16 unit:3;          // 5:3    unit,µ¥Î»
                            //        1-hz, 2-A, 3-RPM, 4-V, 6-%; 001-Hz, 010-A, 100-V
    Uint16 displayBits:3;   // 8:6    5¸öÊıÂë¹ÜÒªÏÔÊ¾µÄÎ»Êı¡£0-ÏÔÊ¾0Î»£¬1-ÏÔÊ¾1Î»£¬...£¬5-ÏÔÊ¾5Î»
    Uint16 upperLimit:1;    // 9      1-²ÎÊıÓÉÉÏÏŞÏà¹Ø¹¦ÄÜÂëÏŞÖÆ£¬0-Ö±½ÓÓÉÉÏÏŞÏŞÖÆ
    Uint16 lowerLimit:1;    // 10     1-²ÎÊıÓÉÏÂÏŞÏà¹Ø¹¦ÄÜÂëÏŞÖÆ£¬0-Ö±½ÓÓÉÏÂÏŞÏŞÖÆ
    Uint16 writable:2;      // 12:11  ²ÎÊı¶ÁĞ´ÌØĞÔ£¬00-¿ÉÒÔ¶ÁĞ´, 01-ÔËĞĞÖĞÖ»¶Á£¬10-²ÎÊıÖ»¶Á
    Uint16 signal:1;        // 13     ·ûºÅ£¬unsignal-0; signal-1
    Uint16 multiLimit:2;    // 15:14  ¸Ã¹¦ÄÜÂëÎª¶à¸ö¹¦ÄÜÂëµÄ×éºÏ. 
                            //        00-µ¥¶À¹¦ÄÜÂë(·Ç×éºÏ); 
                            //        01-Ê®½øÖÆ,  ¶à¸ö¹¦ÄÜÂëµÄ×éºÏ; 
                            //        10-Ê®Áù½øÖÆ,¶à¸ö¹¦ÄÜÂëµÄ×éºÏ; 
};

union FUNC_ATTRIBUTE
{
   Uint16                      all;
   struct FUNC_ATTRIBUTE_BITS  bit;
};
//=====================================================================



//=====================================================================
// ¹¦ÄÜÂëÊôĞÔ±í£ºÉÏÏŞ¡¢ÏÂÏŞ¡¢ÊôĞÔ
// ¹¦ÄÜÂëµÄ³ö³§Öµ£¬°üÀ¨EEPROM_CHECK¡¢µôµç¼ÇÒä£¬µ«²»°üÀ¨ÏÔÊ¾×é
typedef struct FUNCCODE_ATTRIBUTE_STRUCT
{
    Uint16                  lower;          // ÏÂÏŞ
    Uint16                  upper;          // ÉÏÏŞ
    Uint16                  init;           // ³ö³§Öµ
    union FUNC_ATTRIBUTE    attribute;      // ÊôĞÔ

    Uint16                  eepromIndex;    // ¶ÔÓ¦EEPROM´æ´¢µÄindex
} FUNCCODE_ATTRIBUTE;

extern const FUNCCODE_ATTRIBUTE funcCodeAttribute[];
//=====================================================================



//=====================================================================
// ¹¦ÄÜÂëµÄcodeµÄÒ»Ğ©Êı¾İ½á¹¹¶¨Òå
struct PLC_STRUCT
{
    Uint16 runTime;         // PLCµÚ_¶ÎÔËĞĞÊ±¼ä
    Uint16 accDecTimeSet;   // PLCµÚ_¶Î¼Ó¼õËÙÊ±¼äÑ¡Ôñ
};
//=================================


//=================================
struct AI_SET_CURVE  // AIÉè¶¨ÇúÏß
{
    Uint16 minIn;       // ÇúÏß×îĞ¡ÊäÈë(µçÑ¹Öµ)
    Uint16 minInSet;    // ÇúÏß×îĞ¡ÊäÈë¶ÔÓ¦Éè¶¨(°Ù·Ö±È)
    Uint16 maxIn;       // ÇúÏß×î´óÊäÈë(µçÑ¹Öµ)
    Uint16 maxInSet;    // ÇúÏß×î´óÊäÈë¶ÔÓ¦Éè¶¨(°Ù·Ö±È)
};
//=================================


//=================================
struct AI_JUMP
{
    Uint16 point;   // Éè¶¨ÌøÔ¾µã
    Uint16 arrange; // Éè¶¨ÌøÔ¾·ù¶È
};
//=================================


//=================================
struct ANALOG_CALIBRATE_CURVE  // Ä£ÄâÁ¿Ğ£ÕıÇúÏß£¬AIAO
{
    Uint16 before1;     // Ğ£ÕıÇ°µçÑ¹1
    Uint16 after1;      // Ğ£ÕıºóµçÑ¹1
    
    Uint16 before2;     // Ğ£ÕıÇ°µçÑ¹2
    Uint16 after2;      // Ğ£ÕıºóµçÑ¹2
};
//=================================


//=================================
typedef struct AO_PARA_STRUCT
{
    Uint16 offset;          // AOÁãÆ«ÏµÊı
    Uint16 gain;            // AOÔöÒæ
} AO_PARA;
//=================================


//=================================
struct FC_GROUP_HIDE_STRUCT
{
    Uint16 password;    // ¹¦ÄÜÂë×éÒş²ØÌØĞÔÃÜÂë
    
    Uint16 f;           // F×éÒş²ØÌØĞÔ£¨F0-FF£©
    Uint16 a;           // A×éÒş²ØÌØĞÔ£¨A0-AF£©
    Uint16 b;           // B×éÒş²ØÌØĞÔ£¨B0-BF£©
    Uint16 c;           // C×éÒş²ØÌØĞÔ£¨C0-CF£©
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
    Uint16 errorFrq;                    // µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±ÆµÂÊ
    Uint16 errorCurrent;                // µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±µçÁ÷
    Uint16 errorGeneratrixVoltage;      // µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±Ä¸ÏßµçÑ¹
    Uint16 errorDiStatus;               // µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±ÊäÈë¶Ë×Ó×´Ì¬
    Uint16 errorDoStatus;               // µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±Êä³ö¶Ë×Ó×´Ì¬
    
    Uint16 errorInverterStatus;         // µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±±äÆµÆ÷×´Ì¬
    Uint16 errorTimeFromPowerUp;        // µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±Ê±¼ä£¨´Ó±¾´ÎÉÏµç¿ªÊ¼¼ÆÊ±£©
    Uint16 errorTimeFromRun;            // µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±Ê±¼ä£¨´ÓÔËĞĞÊ±¿ªÊ¼¼ÆÊ±£©
};

union ERROR_SCENE
{
    Uint16 all[sizeof(struct ERROR_SCENE_STRUCT)];

    struct ERROR_SCENE_STRUCT elem;
};
//=================================


//=================================
#define APTP_NUM    32  // aptpµãÊı
typedef struct
{
    Uint16 low;     // aptpµÍÎ»£¬0-9999
    Uint16 high;    // aptp¸ßÎ»£¬0-65535
} LENGTH_SET;
// ·¶Î§: 0-655359999
//=================================


//=================================
#define MOTOR_TYPE_ACI_GENERAL  0   // ÆÕÍ¨Òì²½µç»ú
#define MOTOR_TYPE_ACI_INV      1   // ±äÆµÒì²½µç»ú
#define MOTOR_TYPE_PMSM         2   // ÓÀ´ÅÍ¬²½µç»ú
struct MOTOR_PARA_STRUCT
{
    // µç»ú»ù±¾²ÎÊı
    Uint16 motorType;               // F1-00  µç»úÀàĞÍÑ¡Ôñ
    Uint16 ratingPower;             // F1-01  µç»ú¶î¶¨¹¦ÂÊ
    Uint16 ratingVoltage;           // F1-02  µç»ú¶î¶¨µçÑ¹
    Uint16 ratingCurrent;           // F1-03  µç»ú¶î¶¨µçÁ÷
    Uint16 ratingFrq;               // F1-04  µç»ú¶î¶¨ÆµÂÊ
    Uint16 ratingSpeed;             // F1-05  µç»ú¶î¶¨×ªËÙ

    // Òì²½»úµ÷Ğ³²ÎÊı
    Uint16 statorResistance;        // F1-06  Òì²½»ú¶¨×Óµç×è
    Uint16 rotorResistance;         // F1-07  Òì²½»ú×ª×Óµç×è
    Uint16 leakInductance;          // F1-08  Òì²½»úÂ©¸Ğ¿¹
    Uint16 mutualInductance;        // F1-09  Òì²½»ú»¥¸Ğ¿¹
    Uint16 zeroLoadCurrent;         // F1-10  Òì²½»ú¿ÕÔØµçÁ÷
    Uint16 rsvdF11[5];

    // Í¬²½»úµ÷Ğ³²ÎÊı
    Uint16 pmsmRs;                  // F1-16  Í¬²½»ú¶¨×Óµç×è
    Uint16 pmsmLd;                  // F1-17  Í¬²½»údÖáµç¸Ğ
    Uint16 pmsmLq;                  // F1-18  Í¬²½»úqÖáµç¸Ğ
    Uint16 pmsmRsLdUnit;            // F1-19  Í¬²½»úµç¸Ğµç×èµ¥Î»
    Uint16 pmsmCoeff;               // F1-20  Í¬²½»ú·´µç¶¯ÊÆÏµÊı
    Uint16 pmsmCheckTime;           // F1-21  Í¬²½»úä³öÈ±Ïà¼ì²âÊ±¼ä
    Uint16 rsvdF12[5];
    
};

struct PG_PARA_STRUCT
{
    // PG¿¨²ÎÊı
    Uint16 encoderPulse;            // F1-27    ±àÂëÆ÷Âö³åÏßÊı
    Uint16 pgType;                  // F1-28    ±àÂëÆ÷ÀàĞÍ
    Uint16 fvcPgSrc;                // F1-29    ËÙ¶È·´À¡PG¿¨Ñ¡Ôñ, 0-QEP1,1-QEP2(À©Õ¹)    
    Uint16 enCoderDir;              // F1-30    ±àÂëÆ÷ÏàĞò/Ö÷·½Ïò
    Uint16 enCoderAngle;            // F1-31    ±àÂëÆ÷°²×°½Ç
    Uint16 uvwSignDir;              // F1-32    UVWĞÅºÅ·½Ïò
    Uint16 uvwSignAngle;            // F1-33    UVWĞÅºÅÁãµãÎ»ÖÃ½Ç
    Uint16 enCoderPole;             // F1-34    Ğı±ä¼«¶ÔÊı
    Uint16 rsvdF11;                 // F1-35    UVW¼«¶ÔÊı 
    Uint16 fvcPgLoseTime;           // F1-36    ËÙ¶È·´À¡PG¶ÏÏß¼ì²âÊ±¼ä    
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
    MOTOR_SN_1,     // µÚ1µç»ú
    MOTOR_SN_2,     // µÚ2µç»ú
    MOTOR_SN_3,     // µÚ3µç»ú
    MOTOR_SN_4      // µÚ4µç»ú
};
//=================================


//=================================


//------------------------------------------------
struct VC_PARA
{
    Uint16 vcSpdLoopKp1;            // F2-00  ËÙ¶È»·±ÈÀıÔöÒæ1
    Uint16 vcSpdLoopTi1;            // F2-01  ËÙ¶È»·»ı·ÖÊ±¼ä1
    Uint16 vcSpdLoopChgFrq1;        // F2-02  ÇĞ»»ÆµÂÊ1
    Uint16 vcSpdLoopKp2;            // F2-03  ËÙ¶È»·±ÈÀıÔöÒæ2
    Uint16 vcSpdLoopTi2;            // F2-04  ËÙ¶È»·»ı·ÖÊ±¼ä2
    
    Uint16 vcSpdLoopChgFrq2;        // F2-05  ÇĞ»»ÆµÂÊ2
    Uint16 vcSlipCompCoef;          // F2-06  ×ª²î²¹³¥ÏµÊı
    Uint16 vcSpdLoopFilterTime;     // F2-07  ËÙ¶È»·ÂË²¨Ê±¼ä³£Êı
    Uint16 vcOverMagGain;           // F2-08  Ê¸Á¿¿ØÖÆ¹ıÀø´ÅÔöÒæ
    Uint16 spdCtrlDriveTorqueLimitSrc;  // F2-09  ËÙ¶È¿ØÖÆ(Çı¶¯)×ª¾ØÉÏÏŞÔ´
    
    Uint16 spdCtrlDriveTorqueLimit;     // F2-10  ËÙ¶È¿ØÖÆ(Çı¶¯)×ª¾ØÉÏÏŞÊı×ÖÉè¶¨
    Uint16 spdCtrlBrakeTorqueLimitSrc;  // F2-11  ËÙ¶È¿ØÖÆ(ÖÆ¶¯)×ª¾ØÉÏÏŞÔ´
    Uint16 spdCtrlBrakeTorqueLimit;     // F2-12  ËÙ¶È¿ØÖÆ(ÖÆ¶¯)×ª¾ØÉÏÏŞÊı×ÖÉè¶¨
    Uint16 mAcrKp;                  // F2-13  MÖáµçÁ÷»·Kp
    Uint16 mAcrKi;                  // F2-14  MÖáµçÁ÷»·Ki
    
    Uint16 tAcrKp;                  // F2-15  TÖáµçÁ÷»·Kp
    Uint16 tAcrKi;                  // F2-16  TÖáµçÁ÷»·Ki
    Uint16 spdLoopI;                // F2-17  ËÙ¶È»·»ı·ÖÊôĞÔ

    Uint16 weakFlusMode;            // F2-18 Í¬²½»úÈõ´ÅÄ£Ê½
    Uint16 weakFlusCoef;            // F2-19 Í¬²½»úÈõ´ÅÏµÊı
    Uint16 weakFlusCurMax;          // F2-20 ×î´óÈõ´ÅµçÁ÷
    Uint16 weakFlusAutoCoef;        // F2-21 Èõ´Å×Ô¶¯µ÷Ğ³ÏµÊı
    Uint16 weakFlusIntegrMul;       // F2-22 Èõ´Å»ı·Ö±¶Êı
};
//------------------------------------------------



//------------------------------------------------
// µÚ2µç»úµÄ¹¦ÄÜÂë£¬°üÀ¨µç»ú²ÎÊı¡¢¿ØÖÆ²ÎÊı
struct MOTOR_FC
{
    union MOTOR_PARA motorPara;     // Ax-00  Ax-26 µÚ2/3/4µç»ú²ÎÊı. Í¬µÚ1µç»ú²ÎÊı
    union PG_PARA    pgPara;        // Ax-27  Ax-36 µÚ1µç»úPG¿¨²ÎÊı
    Uint16 tuneCmd;                 // Ax-37  µ÷Ğ³ÃüÁî
    struct VC_PARA vcPara;          // Ax-38  Ax-60
    
    Uint16 motorCtrlMode;           // Ax-61  µÚ2/3/4µç»ú¿ØÖÆ·½Ê½
    Uint16 accDecTimeMotor;         // Ax-62  µÚ2/3/4µç»ú¼Ó¼õËÙÊ±¼äÑ¡Ôñ
    Uint16 torqueBoost;             // Ax-63  ×ª¾ØÌáÉı
    Uint16 rsvdA21;                 // Ax-64  Õñµ´ÒÖÖÆÔöÒæÄ£Ê½/
    Uint16 antiVibrateGain;         // Ax-65  Õñµ´ÒÖÖÆÔöÒæ
};
//------------------------------------------------/





#define AI_NUMBER               3           // AI¶Ë×Ó¸öÊı

#define AO_NUMBER               2           // AO¶Ë×Ó¸öÊı

#define HDI_NUMBER              1           // HDI¶Ë×Ó¸öÊı

#define HDO_NUMBER              1           // HDO¶Ë×Ó¸öÊı

#define DI_NUMBER_PHSIC         10          // ÎïÀíDI¶Ë×Ó
#define DI_NUMBER_V             5           // ĞéÄâDI¶Ë×Ó
#define DI_NUMBER_AI_AS_DI      AI_NUMBER   // AI×÷ÎªDI

#define DO_NUMBER_PHSIC         5           // ÎïÀíDO¶Ë×Ó
#define DO_NUMBER_V             5           // ĞéÄâDO¶Ë×Ó

#define PLC_STEP_MAX            16          // PLC¡¢¶à¶ÎÖ¸Áî¶ÎÊı




//=====================================================================
// EEPROMµÄÊ¹ÓÃ³¤¶È£¬°üÀ¨ÖĞ¼äÔ¤Áô²¿·Ö
// EEPROMµØÖ··ÖÅä£¬2010-08-13
// 0            -   ±£Áô£¬rsvd4All
// 1,2          -   EEPROM-CHK
// 3            -   AIAO CHK
// 4-944        -   MD380Ê¹ÓÃ
// 945-1149     -   Ô¤ÁôMD380Ê¹ÓÃ
// 1150-1329    -   IS380/MD380MÊ¹ÓÃ
// ÆäÖĞ,16-63   -   µôµç¼ÇÒäÊ¹ÓÃ
#define EEPROM_INDEX_USE_LENGTH     994     // ×îºóÒ»¸öeepromµØÖ·+1
#define EEPROM_INDEX_USE_INDEX        4     // ¹¦ÄÜ²ÎÊıÊ¹ÓÃEEPROMÆğÊ¼
// Ô¤Áô¸øÆäËüÊ¹ÓÃµÄ¹¦ÄÜÂëindex
#define FUNCCODE_RSVD4ALL_INDEX     GetCodeIndex(funcCode.code.rsvd4All)


#define REM_P_OFF_MOTOR     5   // ĞÔÄÜÊ¹ÓÃµÄµôµç¼ÇÒä¡£°üº¬ÔÚCORE_TO_FUNC_DISP_DATA_NUMÀïÃæ


// ¹¦ÄÜÂëµÄ×éÊı
#define FUNCCODE_GROUP_NUM  83  // °üÀ¨Ô¤ÁôµÄ×é, ¼ûfuncCodeGradeCurrentMenuMode. 
// EEPROMÖĞË³Ğò: EEPROM_CHK, µôµç¼ÇÒä, FF, FP, F0-FE, A0-AF, B0-BF, C0-CF
// ÏÔÊ¾Ë³Ğò:     F0-FE, FF, FP, A0-AF, B0-BF, C0-CF, EEPROM_CHK(²»ÏÔÊ¾), µôµç¼ÇÒä(²»ÏÔÊ¾), U0-UF
// U0-UF²»Õ¼ÓÃEEPROM¿Õ¼ä

#define FUNCCODE_GROUP_F0   0       // F0×é
#define FUNCCODE_GROUP_F1   1       // F1×é
#define FUNCCODE_GROUP_FE   14      // FF×é
#define FUNCCODE_GROUP_FF   15      // FF×é
#define FUNCCODE_GROUP_FP   16      // FP×é
#define FUNCCODE_GROUP_A0   17      // A0×é
#define FUNCCODE_GROUP_A5   22      // A5×é
#define FUNCCODE_GROUP_AA   27      // AA×é
#define FUNCCODE_GROUP_AB   28      // AB×é
#define FUNCCODE_GROUP_AE   31      // AE×é
#define FUNCCODE_GROUP_AF   32      // AF×é
#define FUNCCODE_GROUP_B0   33      // B0×é
#define FUNCCODE_GROUP_BF   48      // BF×é
#define FUNCCODE_GROUP_C0   49      // C0×é
//#define FUNCCODE_GROUP_CC   61      // CC×é ÓÃ»§¶¨ÖÆ¹¦ÄÜÂë×é
#define FUNCCODE_GROUP_CF   64      // CF×é
#define FUNCCODE_GROUP_U0   67      // U0×é£¬ÏÔÊ¾
#define FUNCCODE_GROUP_U3   70      // U3×é£¬ÏÔÊ¾
#define FUNCCODE_GROUP_UF   (FUNCCODE_GROUP_NUM - 1)    // UF£¬ĞÔÄÜµ÷ÊÔÏÔÊ¾

// Ã¿×é¹¦ÄÜÂëµÄ¸öÊı
// ³ıFF×éÖ®Íâ£¬Ã¿×éÔ¤Áô2¸ö¹¦ÄÜÂë¡£ÎªÁËÔÚÔö¼Ó¹¦ÄÜÂëÊ±£¬¾¡Á¿²»ÓÃ»Ö¸´³ö³§²ÎÊı¡£
#define F0NUM           (28+ 1  )   // F0  »ù±¾¹¦ÄÜ×é
#define F1NUM           (38+ 0  )   // F1  µç»ú²ÎÊı
#define F2NUM           (23+ 0  )   // F2  Ê¸Á¿¿ØÖÆ²ÎÊı
#define F3NUM           (16+ 0  )   // F3  V/F¿ØÖÆ²ÎÊı

#define F4NUM           (40+ 0  )   // F4  ÊäÈë¶Ë×Ó
#define F5NUM           (23+ 0  )   // F5  Êä³ö¶Ë×Ó
#define F6NUM           (16+ 0  )   // F6  ÆôÍ£¿ØÖÆ
#define F7NUM           (15+ 0  )   // F7  ¼üÅÌÓëÏÔÊ¾

#define F8NUM           (54+ 0  )   // F8  ¸¨Öú¹¦ÄÜ
#define F9NUM           (71+ 0  )   // F9  ¹ÊÕÏÓë±£»¤
#define FANUM           (29+ 0  )   // FA  PID¹¦ÄÜ 
#define FBNUM           (10+ 0  )   // FB  °ÚÆµ¡¢¶¨³¤ºÍ¼ÆÊı

#define FCNUM           (52+ 0  )   // FC  ¶à¶ËËÙ¡¢PLC
#define FDNUM           ( 8+ 0  )   // FD  Í¨Ñ¶²ÎÊı
#define FENUM           (32+ 0  )   // FE  280ÓĞ320Ã»ÓĞµÄ¹¦ÄÜÂë
#define FFNUM           (13+ 0  )   // FF  ³§¼Ò²ÎÊı

#define FPNUM           ( 6+ 0  )   // FP  ÓÃ»§ÃÜÂë, ²ÎÊı³õÊ¼»¯

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
#define CFNUM           FUNC_TO_CORE_DEBUG_DATA_NUM     // CF  µ÷ÊÔ£¬func2motor
#elif 1
#define CFNUM           ( 0+1)      // CF
#endif

#define CHK_NUM   (4 +0  )  //     eepromCheckWord(2)£¬rsvd4All(1)·ÅÔÚÕâÀïµÄ×îÇ°Ãæ, AIAOChk(1)Ò²ÔÚÕâÀï
#define REM_NUM   (48)      // µôµç¼ÇÒä£¬°üÀ¨ĞÔÄÜÊ¹ÓÃµÄµôµç¼ÇÒä¡£(°üÀ¨ĞÔÄÜµÄµôµç¼ÇÒä)

#define U0NUM     (100+0)   // U0  ÏÔÊ¾Ê¹ÓÃ£¬²»Õ¼ÓÃEEPROM£¬¾¡Á¿ÉÙµÄÕ¼ÓÃ³ÌĞò¿Õ¼ä(ÎŞ³ö³§Öµ¡¢ÉÏÏÂÏŞ£¬µ«ÓĞÊôĞÔ)
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
#define UFNUM     CORE_TO_FUNC_DISP_DATA_NUM  // UF  µ÷ÊÔ£¬motor2func
#elif 1
#define UFNUM     ( 0+1)    // UF ĞÔÄÜµ÷ÊÔÊ¹ÓÃ
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
                        )                                   // ËùÓĞ¹¦ÄÜÂë£¬²»°üÀ¨ÏÔÊ¾
#define FNUM_EEPROM    (FNUM_PARA + CHK_NUM + REM_NUM)      // ĞèÒª´æ´¢ÔÚEEPROMÖĞµÄËùÓĞ²ÎÊı
#define FNUM_ALL       (FNUM_EEPROM +                       \
                        U0NUM + U1NUM + U2NUM + U3NUM +     \
                        U4NUM + U5NUM + U6NUM + U7NUM +     \
                        U8NUM + U9NUM + UANUM + UBNUM +     \
                        UCNUM + UDNUM + UENUM + UFNUM       \
                       )                                    // ËùÓĞ¹¦ÄÜÂë¡¢²ÎÊı£¬°üÀ¨ÏÔÊ¾


//=====================================================================
//
// ¹¦ÄÜÂë×é£¬F0×é£¬F1×é, ...
//
// eepromCheckWord·ÅÔÚ×îÇ°Ãæ
// ³§¼Ò²ÎÊı¡¢ÓÃ»§ÃÜÂë¡¢ÓÃ»§¶¨ÖÆ²Ëµ¥·ÅÔÚÇ°Ãæ£¬·ÀÖ¹ÔöÉ¾¹¦ÄÜÂëÊ±±»ÎóĞŞ¸Ä¡£
// fcÓëeepromÓĞ¶ÔÓ¦¹ØÏµ±í£¬ËùÒÔÒÔÉÏÒªÇó²»ĞèÒªÁË
//
//=====================================================================
struct FUNCCODE_GROUP 
{
//======================================
    Uint16 f0[F0NUM];               // F0 »ù±¾¹¦ÄÜ×é
    Uint16 f1[F1NUM];               // F1 µç»ú²ÎÊı
    Uint16 f2[F2NUM];               // F2 Ê¸Á¿¿ØÖÆ²ÎÊı
    Uint16 f3[F3NUM];               // F3 V/F¿ØÖÆ²ÎÊı
    
//======================================
    Uint16 f4[F4NUM];               // F4 ÊäÈë¶Ë×Ó
    Uint16 f5[F5NUM];               // F5 Êä³ö¶Ë×Ó
    Uint16 f6[F6NUM];               // F6 ÆôÍ£¿ØÖÆ
    Uint16 f7[F7NUM];               // F7 ¼üÅÌÓëÏÔÊ¾
    
//======================================
    Uint16 f8[F8NUM];               // F8 ¸¨Öú¹¦ÄÜ
    Uint16 f9[F9NUM];               // F9 ¹ÊÕÏÓë±£»¤
    Uint16 fa[FANUM];               // FA PID¹¦ÄÜ
    Uint16 fb[FBNUM];               // FB °ÚÆµ¡¢¶¨³¤ºÍ¼ÆÊı

//======================================
    Uint16 fc[FCNUM];               // FC ¶à¶ËËÙ¡¢PLC
    Uint16 fd[FDNUM];               // FD Í¨Ñ¶²ÎÊı
    Uint16 fe[FENUM];               // FE 280ÓĞ320Ã»ÓĞµÄ¹¦ÄÜÂë
    Uint16 ff[FFNUM];               // FF ³§¼Ò²ÎÊı
    
//======================================
    Uint16 fp[FPNUM];               // FP ¹¦ÄÜÂë¹ÜÀí
    
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
    Uint16 ae[AENUM];               // AE AIAO³ö³§Ğ£Õı
    Uint16 af[AFNUM];               // AF ¹¦ÄÜÂë×éÒş²Ø
    
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

// Ö®Ç°µÄ¹¦ÄÜÂëÓĞÉÏÏÂÏŞ£¬ÊôĞÔ£»Ö®ºóÃ»ÓĞ£¬½ÚÊ¡¿Õ¼ä
//======================================

//======================================
    Uint16 fChk[CHK_NUM];           // eepromCheckWord

//======================================
    Uint16 remember[REM_NUM];       // µôµç¼ÇÒä

// Ö®Ç°µÄÊı¾İÒª·ÅÔÚEEPROMÖĞ
//======================================

//======================================
// Ö®ºóµÄÊı¾İ²»ĞèÒª·ÅÔÚEEPROMÖĞ£¬½öRAM
    Uint16 u0[U0NUM];               // U0 ÏÔÊ¾
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
    Uint16 uf[UFNUM];               // UF, ÏÔÊ¾£¬ĞÔÄÜµ÷ÊÔÊ¹ÓÃ
//======================================
};


//=====================================================================
//
// ¹¦ÄÜÂë£¬F0-00, F0-01, ..., F1-00, F1-01, ...
//
//=====================================================================
struct FUNCCODE_CODE 
{
//======================================
// F0 »ù±¾¹¦ÄÜ×é
    Uint16 inverterGpTypeDisp;      // F0-00  GPÀàĞÍÏÔÊ¾
    Uint16 motorCtrlMode;           // F0-01  (µç»ú)¿ØÖÆ·½Ê½
    Uint16 runSrc;                  // F0-02  ÃüÁîÔ´Ñ¡Ôñ
    Uint16 frqXSrc;                 // F0-03  Ö÷ÆµÂÊÔ´XÑ¡Ôñ
    Uint16 frqYSrc;                 // F0-04  ¸¨ÖúÆµÂÊÔ´YÑ¡Ôñ
    Uint16 frqYRangeBase;           // F0-05  ¸¨ÖúÆµÂÊÔ´Y·¶Î§Ñ¡Ôñ
    Uint16 frqYRange;               // F0-06  ¸¨ÖúÆµÂÊÔ´Y·¶Î§
    Uint16 frqCalcSrc;              // F0-07  ÆµÂÊÔ´Ñ¡Ôñ
    Uint16 presetFrq;               // F0-08  Ô¤ÖÃÆµÂÊ
    Uint16 runDir;                  // F0-09  ÔËĞĞ·½Ïò
    Uint16 maxFrq;                  // F0-10  ×î´óÆµÂÊ
    Uint16 upperFrqSrc;             // F0-11  ÉÏÏŞÆµÂÊÔ´
    Uint16 upperFrq;                // F0-12  ÉÏÏŞÆµÂÊÊıÖµÉè¶¨
    Uint16 upperFrqOffset;          // F0-13  ÉÏÏŞÆµÂÊÆ«ÖÃ
    Uint16 lowerFrq;                // F0-14  ÏÂÏŞÆµÂÊÊıÖµÉè¶¨
    Uint16 carrierFrq;              // F0-15  ÔØ²¨ÆµÂÊ
    Uint16 varFcByTem;              // F0-16  ÔØ²¨ÆµÂÊËæÎÂ¶Èµ÷Õû
    Uint16 accTime1;                // F0-17  ¼ÓËÙÊ±¼ä1
    Uint16 decTime1;                // F0-18  ¼õËÙÊ±¼ä1
    Uint16 accDecTimeUnit;          // F0-19  ¼Ó¼õËÙÊ±¼äµÄµ¥Î»
    Uint16 frqYOffsetSrc;           // F0-20  ¸¨ÖúÆµÂÊÔ´Æ«ÖÃÑ¡Ôñ
    Uint16 frqYOffsetFc;            // F0-21  ¸¨ÖúÆµÂÊÔ´Æ«ÖÃµÄÊı×ÖÉè¶¨
    Uint16 frqPoint;                // F0-22  ÆµÂÊÖ¸ÁîĞ¡Êıµã
    Uint16 frqRemMode;              // F0-23  Êı×ÖÉè¶¨ÆµÂÊ¼ÇÒäÑ¡Ôñ
    enum MOTOR_SN motorSn;          // F0-24  µç»úÑ¡Ôñ
    Uint16 accDecBenchmark;         // F0-25  ¼Ó¼õËÙÊ±¼ä»ù×¼ÆµÂÊ
    Uint16 updnBenchmark;           // F0-26  ÔËĞĞÊ±ÆµÂÊÖ¸ÁîUP/DOWN»ù×¼
    Uint16 frqRunCmdBind;           // F0-27  ÃüÁîÔ´À¦°óÆµÂÊÔ´
    Uint16 commProtocolSec;         // F0-28  Í¨Ñ¶Ğ­ÒéÑ¡Ôñ

//======================================
// F1 µç»ú²ÎÊı
    union MOTOR_PARA motorParaM1;   // F1-00  F1-26 µÚ1µç»ú²ÎÊı
    union PG_PARA    pgParaM1;      // f1-27  F1-36 µÚ1µç»úPG¿¨²ÎÊı
    Uint16 tuneCmd;                 // F1-37  µ÷Ğ³Ñ¡Ôñ

//======================================
// F2 Ê¸Á¿¿ØÖÆ²ÎÊı
    struct VC_PARA vcParaM1;        // µÚ1µç»úÊ¸Á¿¿ØÖÆ²ÎÊı


//======================================
// F3 V/F¿ØÖÆ²ÎÊı
    Uint16 vfCurve;                 // F3-00  VFÇúÏßÉè¶¨
    Uint16 torqueBoost;             // F3-01  ×ª¾ØÌáÉı
    Uint16 boostCloseFrq;           // F3-02  ×ª¾ØÌáÉı½ØÖ¹ÆµÂÊ
    Uint16 vfFrq1;                  // F3-03  ¶àµãVFÆµÂÊµã1
    Uint16 vfVol1;                  // F3-04  ¶àµãVFµçÑ¹µã1
    Uint16 vfFrq2;                  // F3-05  ¶àµãVFÆµÂÊµã2
    Uint16 vfVol2;                  // F3-06  ¶àµãVFµçÑ¹µã2
    Uint16 vfFrq3;                  // F3-07  ¶àµãVFÆµÂÊµã3
    Uint16 vfVol3;                  // F3-08  ¶àµãVFµçÑ¹µã3
    Uint16 slipCompCoef;            // F3-09  ×ª²î²¹³¥ÏµÊı
    Uint16 vfOverMagGain;           // F3-10  VF¹ıÀø´ÅÔöÒæ
    Uint16 antiVibrateGain;         // F3-11  Õñµ´ÒÖÖÆÔöÒæ

    Uint16 rsvdF31;//antiVibrateGainMode;     // F3-12  Õñµ´ÒÖÖÆÔöÒæÄ£Ê½
    Uint16 vfVoltageSrc;            // F3-13  VF·ÖÀëµÄµçÑ¹Ô´
    Uint16 vfVoltageDigtalSet;      // F3-14  VF·ÖÀëµÄµçÑ¹Ô´Êı×ÖÉè¶¨
    Uint16 vfVoltageAccTime;        // F3-15  VF·ÖÀëµÄµçÑ¹ÉÏÉıÊ±¼ä

//======================================
// F4 ÊäÈë¶Ë×Ó
    Uint16 diFunc[DI_NUMBER_PHSIC]; // F4-00  --F4-09   DI1¶Ë×Ó¹¦ÄÜÑ¡Ôñ
    
    Uint16 diFilterTime;            // F4-10  DIÂË²¨Ê±¼ä
    Uint16 diControlMode;           // F4-11  ¶Ë×ÓÃüÁî·½Ê½
    Uint16 diUpDownSlope;           // F4-12  ¶Ë×ÓUP/DOWNËÙÂÊ£¬¸ÄÎª0.001Hz

    Uint16 curveSet2P1[4];          // F4-13,...,F4-16  ÇúÏß1£¬2µã£¬×î´óÖµ£¬×îĞ¡Öµ
    Uint16 ai1FilterTime;           // F4-17  AI1ÂË²¨Ê±¼ä, 10ms

    Uint16 curveSet2P2[4];          // F4-18,...,F4-21  ÇúÏß2£¬2µã
    Uint16 ai2FilterTime;           // F4-22  AI2ÂË²¨Ê±¼ä, 10ms

    Uint16 curveSet2P3[4];          // F4-23,...,F4-26  ÇúÏß3£¬2µã
    Uint16 ai3FilterTime;           // F4-27  AI3ÂË²¨Ê±¼ä, 10ms

    Uint16 curveSet2P4[4];          // F4-28,...,F4-31  HDIÇúÏß£¬2µã
    Uint16 pulseInFilterTime;       // F4-32  PULSEÂË²¨Ê±¼ä, 10ms

    Uint16 aiCurveSrc;              // F4-33  AIÉè¶¨ÇúÏßÑ¡Ôñ

    Uint16 aiLimitSrc;              // F4-34  AIÏÂÏŞÑ¡Ôñ

    Uint16 diDelayTime[3];          // F4-35  DI1ÑÓ³ÙÊ±¼ä
    Uint16 diLogic[2];              // F4-38  DIÓĞĞ§×´Ì¬Ñ¡Ôñ1
                                    // F4-39  DIÓĞĞ§×´Ì¬Ñ¡Ôñ2

//======================================
// F5 Êä³ö¶Ë×Ó
    Uint16 fmOutSelect;             // F5-00  ¶à¹¦ÄÜ¶Ë×ÓÊä³öÑ¡Ôñ
    Uint16 doFunc[DO_NUMBER_PHSIC]; // F5-01  FMRÊä³öÑ¡Ôñ
                                    // F5-02  ¿ØÖÆ°åRELAYÊä³öÑ¡Ôñ
                                    // F5-03  À©Õ¹¿¨RELAYÊä³öÑ¡Ôñ
                                    // F5-04  DO1Êä³öÑ¡Ôñ
                                    // F5-05  À©Õ¹¿¨DO2Êä³öÑ¡Ôñ

    Uint16 aoFunc[AO_NUMBER+HDO_NUMBER];    // F5-06  FMPÊä³öÑ¡Ôñ
                                            // F5-07  AO1Êä³öÑ¡Ôñ
                                            // F5-08  À©Õ¹¿¨AO2Êä³öÑ¡Ôñ
    Uint16 fmpOutMaxFrq;                    // F5-09  FMPÊä³ö×î´óÆµÂÊ

    AO_PARA aoPara[AO_NUMBER];              // F5-10  AO1ÁãÆ«ÏµÊı
                                            // F5-11  AO1ÔöÒæ
                                            // F5-12  AO2ÁãÆ«ÏµÊı
                                            // F5-13  AO2ÔöÒæ
    Uint16 aoLpfTime[AO_NUMBER+HDO_NUMBER]; // F5-14  HDO,AO1,AO2Êä³öÂË²¨Ê±¼ä
    
    Uint16 doDelayTime[DO_NUMBER_PHSIC];// F5-17  RELAY1Êä³öÑÓ³ÙÊ±¼ä
                                        // F5-18  RELAY2Êä³öÑÓ³ÙÊ±¼ä
                                        // F5-19  DO1Êä³öÑÓ³ÙÊ±¼ä
                                    
                                        // F5-20  DO2Êä³öÑÓ³ÙÊ±¼ä
                                        // F5-21  DO3Êä³öÑÓ³ÙÊ±¼ä
    Uint16 doLogic;                     // F5-22  DOÓĞĞ§×´Ì¬Ñ¡Ôñ

//======================================
// F6 ÆôÍ£¿ØÖÆ
    Uint16 startMode;               // F6-00  Æô¶¯·½Ê½
    Uint16 speedTrackMode;          // F6-01  ×ªËÙ¸ú×Ù·½Ê½
    Uint16 speedTrackVelocity;      // F6-02  ×ªËÙ¸ú×Ù¿ìÂı
    Uint16 startFrq;                // F6-03  Æô¶¯ÆµÂÊ
    Uint16 startFrqTime;            // F6-04  Æô¶¯ÆµÂÊ±£³ÖÊ±¼ä
    Uint16 startBrakeCurrent;       // F6-05  Æô¶¯Ö±Á÷ÖÆ¶¯µçÁ÷
    Uint16 startBrakeTime;          // F6-06  Æô¶¯Ö±Á÷ÖÆ¶¯Ê±¼ä
    Uint16 accDecSpdCurve;          // F6-07  ¼Ó¼õËÙ·½Ê½
    Uint16 sCurveStartPhaseTime;    // F6-08  SÇúÏß¿ªÊ¼¶ÎÊ±¼ä±ÈÀı
    Uint16 sCurveEndPhaseTime;      // F6-09  SÇúÏß½áÊø¶ÎÊ±¼ä±ÈÀı
    Uint16 stopMode;                // F6-10  Í£»ú·½Ê½
    Uint16 stopBrakeFrq;            // F6-11  Í£»úÖ±Á÷ÖÆ¶¯ÆğÊ¼ÆµÂÊ
    Uint16 stopBrakeWaitTime;       // F6-12  Í£»úÖ±Á÷ÖÆ¶¯µÈ´ıÊ±¼ä
    Uint16 stopBrakeCurrent;        // F6-13  Í£»úÖ±Á÷ÖÆ¶¯µçÁ÷
    Uint16 stopBrakeTime;           // F6-14  Í£»úÖ±Á÷ÖÆ¶¯Ê±¼ä
    Uint16 brakeDutyRatio;          // F6-15  ÖÆ¶¯Ê¹ÓÃÂÊ

//======================================
// F7 ¼üÅÌÓëÏÔÊ¾
    Uint16 rsvdF71;                 // F7-00  ±£Áô
    Uint16 mfkKeyFunc;              // F7-01  MF.K¼ü¹¦ÄÜÑ¡Ôñ
    Uint16 stopKeyFunc;             // F7-02  STOP¼ü¹¦ÄÜ
    Uint16 ledDispParaRun1;         // F7-03  LEDÔËĞĞÏÔÊ¾²ÎÊı1
    Uint16 ledDispParaRun2;         // F7-04  LEDÔËĞĞÏÔÊ¾²ÎÊı2
    
    Uint16 ledDispParaStop;         // F7-05  LEDÍ£»úÏÔÊ¾²ÎÊı
    Uint16 speedDispCoeff;          // F7-06  ¸ºÔØËÙ¶ÈÏÔÊ¾ÏµÊı
    Uint16 radiatorTemp;            // F7-07  Äæ±äÆ÷Ä£¿éÉ¢ÈÈÆ÷ÎÂ¶È
    Uint16 temp2;                   // F7-08  ÕûÁ÷ÇÅÉ¢ÈÈÆ÷ÎÂ¶È
    Uint16 runTimeAddup;            // F7-09  ÀÛ¼ÆÔËĞĞÊ±¼ä, µ¥Î»: h
    
    Uint16 productVersion;          // F7-10  ²úÆ·ºÅ
    Uint16 softVersion;             // F7-11  Èí¼ş°æ±¾ºÅ
    Uint16 speedDispPointPos;       // F7-12  ¸ºÔØËÙ¶ÈÏÔÊ¾Ğ¡ÊıµãÎ»ÖÃ
    Uint16 powerUpTimeAddup;        // F7-13  ÀÛ¼ÆÉÏµçÊ±¼ä
    Uint16 powerAddup;              // F7-14  ÀÛ¼ÆºÄµçÁ¿
    

//======================================
// F8 ¸¨Öú¹¦ÄÜ
    Uint16 jogFrq;                  // F8-00  µã¶¯ÔËĞĞÆµÂÊ
    Uint16 jogAccTime;              // F8-01  µã¶¯¼ÓËÙÊ±¼ä
    Uint16 jogDecTime;              // F8-02  µã¶¯¼õËÙÊ±¼ä
    Uint16 accTime2;                // F8-03  ¼ÓËÙÊ±¼ä2
    Uint16 decTime2;                // F8-04  ¼õËÙÊ±¼ä2
    Uint16 accTime3;                // F8-05  ¼ÓËÙÊ±¼ä3
    Uint16 decTime3;                // F8-06  ¼õËÙÊ±¼ä3
    Uint16 accTime4;                // F8-07  ¼ÓËÙÊ±¼ä4
    Uint16 decTime4;                // F8-08  ¼õËÙÊ±¼ä4
    Uint16 jumpFrq1;                // F8-09  ÌøÔ¾ÆµÂÊ1
    Uint16 jumpFrq2;                // F8-10  ÌøÔ¾ÆµÂÊ2
    Uint16 jumpFrqRange;            // F8-11  ÌøÔ¾ÆµÂÊ·ù¶È
    Uint16 zeroSpeedDeadTime;       // F8-12  Õı·´×ªËÀÇøÊ±¼ä
    Uint16 antiReverseRun;          // F8-13  ·´×ª¿ØÖÆ, 0-ÔÊĞí·´×ª£¬1-½ûÖ¹·´×ª
    Uint16 lowerDeal;               // F8-14  ÆµÂÊµÍÓÚÏÂÏŞÆµÂÊÔËĞĞ¶¯×÷
    Uint16 droopCtrl;               // F8-15  ÏÂ´¹¿ØÖÆ
    Uint16 powerUpTimeArriveSet;    // F8-16  Éè¶¨ÉÏµçµ½´ïÊ±¼ä
    Uint16 runTimeArriveSet;        // F8-17  Éè¶¨ÔËĞĞµ½´ïÊ±¼ä
    Uint16 startProtect;            // F8-18  Æô¶¯±£»¤Ñ¡Ôñ
    Uint16 frqFdtValue;             // F8-19  ÆµÂÊ¼ì²âÖµ(FDTµçÆ½)
    Uint16 frqFdtLag;               // F8-20  ÆµÂÊ¼ì²âÖÍºóÖµ
    Uint16 frqArriveRange;          // F8-21  ÆµÂÊµ½´ï¼ì³ö·ù¶È
    Uint16 jumpFrqMode;             // F8-22  ¼Ó¼õËÙ¹ı³ÌÖĞÌøÔ¾ÆµÂÊÊÇ·ñÓĞĞ§
    Uint16 runTimeOverAct;          // F8-23  Éè¶¨ÔËĞĞÊ±¼äµ½´ï¶¯×÷Ñ¡Ôñ
  
    // ADD
    Uint16 powerUpTimeOverAct;      // F8-24  Éè¶¨ÉÏµçÊ±¼äµ½´ï¶¯×÷Ñ¡Ôñ
    Uint16 accTimefrqChgValue;      // F8-25  ¼ÓËÙÊ±¼ä1/2ÇĞ»»ÆµÂÊµã 
    Uint16 decTimefrqChgValue;      // F8-26  ¼õËÙÊ±¼ä1/2ÇĞ»»ÆµÂÊµã 
    Uint16 jogWhenRun;              // F8-27  ¶Ë×Óµã¶¯ÓÅÏÈ
    Uint16 frqFdt1Value;            // F8-28  ÆµÂÊ¼ì²âÖµ(FDT1µçÆ½)
    Uint16 frqFdt1Lag;              // F8-29  ÆµÂÊ¼ì²â1ÖÍºóÖµ
    Uint16 frqArriveValue1;         // F8-30  ÆµÂÊµ½´ï¼ì²âÖµ1 
    Uint16 frqArriveRange1;         // F8-31  ÆµÂÊµ½´ï¼ì³ö1·ù¶È
    Uint16 frqArriveValue2;         // F8-32  ÆµÂÊµ½´ï¼ì²âÖµ2 
    Uint16 frqArriveRange2;         // F8-33  ÆµÂÊµ½´ï¼ì³ö2·ù¶È

    Uint16 oCurrentChkValue;        // F8-34  ÁãµçÁ÷¼ì²âÖµ
    Uint16 oCurrentChkTime;         // F8-35  ÁãµçÁ÷¼ì²âÑÓ³ÙÊ±¼ä
    Uint16 softOCValue;             // F8-36  Èí¼ş¹ıÁ÷µã
    Uint16 softOCDelay;             // F8-37  Èí¼ş¹ıÁ÷¼ì²âÑÓ³ÙÊ±¼ä

    Uint16 currentArriveValue1;     // F8-38  µçÁ÷µ½´ï¼ì²âÖµ1
    Uint16 currentArriveRange1;     // F8-39  µçÁ÷µ½´ï¼ì²â1·ù¶È
    Uint16 currentArriveValue2;     // F8-40  µçÁ÷µ½´ï¼ì²âÖµ1
    Uint16 currentArriveRange2;     // F8-41  µçÁ÷µ½´ï¼ì²â1·ù¶È

    Uint16 setTimeMode;             // F8-42  ¶¨Ê±¹¦ÄÜÑ¡Ôñ
    Uint16 setTimeSource;           // F8-43  ¶¨Ê±Ê±¼äÉè¶¨Ñ¡Ôñ
    Uint16 setTimeValue;            // F8-44  Éè¶¨ÔËĞĞÊ±¼ä
    
    Uint16 ai1VoltageLimit;         // F8-45  AI1ÊäÈëµçÑ¹ÏÂÏŞ
    Uint16 ai1VoltageUpper;         // F8-46  AI1ÊäÈëµçÑ¹ÉÏÏŞ

    Uint16 temperatureArrive;       // F8-47  Ä£¿éÎÂ¶Èµ½´ï
    Uint16 fanControl;              // F8-48  ·çÉÈ¿ØÖÆ
    Uint16 wakeUpFrq;               // F8-49  »½ĞÑÆµÂÊ
    Uint16 wakeUpTime;              // F8-50  »½ĞÑÑÓ³ÙÊ±¼ä
    Uint16 dormantFrq;              // F8-51  ĞİÃßÆµÂÊ
    Uint16 dormantTime;             // F8-52  ĞİÃßÑÓ³ÙÊ±¼ä
    Uint16 setTimeArrive;           // F8-53  µ±Ç°ÔËĞĞµ½´ïÊ±¼ä
    
//======================================
// F9 ¹ÊÕÏÓë±£»¤
    Uint16 overloadMode;                // F9-00  µç»ú¹ıÔØ±£»¤Ñ¡Ôñ
    Uint16 overloadGain;                // F9-01  µç»ú¹ıÔØ±£»¤ÔöÒæ
    Uint16 foreOverloadCoef;            // F9-02  µç»ú¹ıÔØÔ¤¾¯ÏµÊı
    Uint16 ovGain;                      // F9-03  ¹ıÑ¹Ê§ËÙÔöÒæ
    Uint16 ovPoint;                     // F9-04  ¹ıÑ¹Ê§ËÙ±£»¤µçÑ¹
    
    Uint16 ocGain;                      // F9-05  ¹ıÁ÷Ê§ËÙÔöÒæ
    Uint16 ocPoint;                     // F9-06  ¹ıÁ÷Ê§ËÙ±£»¤µçÁ÷
    Uint16 shortCheckMode;              // F9-07  ÉÏµç¶ÔµØ¶ÌÂ·±£»¤¹¦ÄÜ
    Uint16 rsvdF91;                     // F9-08  ±£Áô
    Uint16 errAutoRstNumMax;            // F9-09  ¹ÊÕÏ×Ô¶¯¸´Î»´ÎÊı
    
    Uint16 errAutoRstRelayAct;          // F9-10  ¹ÊÕÏ×Ô¶¯¸´Î»ÆÚ¼ä¹ÊÕÏ¼ÌµçÆ÷¶¯×÷Ñ¡Ôñ
    Uint16 errAutoRstSpaceTime;         // F9-11  ¹ÊÕÏ×Ô¶¯¸´Î»¼ä¸ôÊ±¼ä, 0.1s 
    Uint16 inPhaseLossProtect;          // F9-12  ÊäÈëÈ±Ïà±£»¤Ñ¡Ôñ
    Uint16 outPhaseLossProtect;         // F9-13  Êä³öÈ±Ïà±£»¤Ñ¡Ôñ
    Uint16 errorLatest1;                // F9-14  µÚÒ»´Î¹ÊÕÏÀàĞÍ
    
    Uint16 errorLatest2;                // F9-15  µÚ¶ş´Î¹ÊÕÏÀàĞÍ
    Uint16 errorLatest3;                // F9-16  µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÀàĞÍ

    union ERROR_SCENE errorScene3;      // F9-17  µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±ÆµÂÊ                              
                                        // F9-18  µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±µçÁ÷                             
                                        // F9-19  µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±Ä¸ÏßµçÑ¹                                                                                  
                                        // F9-20  µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±ÊäÈë¶Ë×Ó×´Ì¬                     
                                        // F9-21  µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±Êä³ö¶Ë×Ó×´Ì¬   
                                        
                                        // F9-22  µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±±äÆµÆ÷×´Ì¬                       
                                        // F9-23  µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±Ê±¼ä£¨´Ó±¾´ÎÉÏµç¿ªÊ¼¼ÆÊ±£©       
                                        // F9-24  µÚÈı´Î(×î½üÒ»´Î)¹ÊÕÏÊ±Ê±¼ä£¨´ÓÔËĞĞÊ±¿ªÊ¼¼ÆÊ±£©         

    Uint16 rsvdF92[2];                  // F9-25  F9-26
    
    union ERROR_SCENE errorScene2;      // F9-27  µÚ¶ş´Î¹ÊÕÏÏÖ³¡
    Uint16 rsvdF921[2];                 // F9-35  F9-36

    union ERROR_SCENE errorScene1;      // F9-37  µÚÒ»´Î¹ÊÕÏÏÖ³¡
    Uint16 rsvdF922[2];                 // F9-45  F9-46
    
    Uint16 errorAction[5];              // F9-47  -F9-51  ¹ÊÕÏÊ±±£»¤¶¯×÷Ñ¡Ôñ1-5
    Uint16 errorShow[2];                // F9-52  -F9-53  ¹ÊÕÏÖ¸Ê¾Ñ¡Ôñ1,2
    Uint16 errorRunFrqSrc;              // F9-54  ¹ÊÕÏÊ±¼ÌĞøÔËĞĞÆµÂÊÑ¡Ôñ
    Uint16 errorSecondFrq;              // F9-55  Òì³£±¸ÓÃÆµÂÊÉè¶¨
    
    Uint16 motorOtMode;                 // F9-56  µç»úÎÂ¶È´«¸ĞÆ÷ÀàĞÍ
    Uint16 motorOtProtect;              // F9-57  µç»ú¹ıÈÈ±£»¤ãĞÖµ
    Uint16 motorOtCoef;                 // F9-58  µç»ú¹ıÈÈÔ¤±¨¾¯ãĞÖµ
     
    Uint16 pOffTransitoryNoStop;        // F9-59  Ë²Í£²»Í£¹¦ÄÜÑ¡Ôñ
    Uint16 pOffTransitoryFrqDecSlope;   // F9-60  Ë²Í£¶¯×÷ÔİÍ£ÅĞ¶ÏµçÑ¹
    Uint16 pOffVolBackTime;             // F9-61  Ë²Í£²»Í£µçÑ¹»ØÉıÅĞ¶ÏÊ±¼ä
    
    Uint16 pOffThresholdVol;            // F9-62  Ë²Í£²»Í£¶¯×÷ÅĞ¶ÏµçÑ¹
    Uint16 loseLoadProtectMode;         // F9-63  µôÔØ±£»¤Ñ¡Ôñ
    Uint16 loseLoadLevel;               // F9-64  µôÔØ¼ì³öË®Æ½
    Uint16 loseLoadTime;                // F9-65  µôÔØ¼ì³öÊ±¼ä
    Uint16 rsvdF923;                    // F9-66  ¹ÊÕÏÆµÂÊµÄĞ¡Êıµã

    Uint16 osChkValue;                  // F9-67 ¹ıËÙ¶È¼ì²âÖµ
    Uint16 osChkTime;                   // F9-68 ¹ıËÙ¶È¼ì²âÊ±¼ä
    Uint16 devChkValue;                 // F9-69 ËÙ¶ÈÆ«²î¹ı´ó¼ì²âÖµ
    Uint16 devChkTime;                  // F9-70 ËÙ¶ÈÆ«²î¹ı´ó¼ì²âÊ±¼ä

#if 0 
    Uint16 losePowerStopEnable;         // F9-71 µôµçÍ£»ú¹¦ÄÜÓĞĞ§
    Uint16 losePowerStopSel;            // F9-72 µôµçÍ£»ú´¦Àí·½Ê½
    Uint16 losePowerLowerFrq;           // F9-73 µôµç³õÊ¼µøÂäÆµÂÊ
    Uint16 losePowerLowerTime;          // F9-74 µôµç³õÊ¼µøÂäÊ±¼ä
    Uint16 losePowerDectime;            // F9-75 µôµç¼õËÙÊ±¼ä
    Uint16 losePowerP;                  // F9-76 µôµçµøÂäµçÑ¹±Õ»·±ÈÀı
    Uint16 losePowerI;                  // F9-77 µôµçµøÂäµçÑ¹±Õ»·»ı·Ö
    
#endif
    

//======================================
// FA PID¹¦ÄÜ
    Uint16 pidSetSrc;               // FA-00  PID¸ø¶¨Ô´
    Uint16 pidSet;                  // FA-01  PIDÊıÖµ¸ø¶¨, 0.1%
    Uint16 pidFdbSrc;               // FA-02  PID·´À¡Ô´
    Uint16 pidDir;                  // FA-03  PID×÷ÓÃ·½Ïò
    Uint16 pidDisp;                 // FA-04  PID¸ø¶¨·´À¡Á¿³Ì
    
    Uint16 pidKp;                   // FA-05  ±ÈÀıÔöÒæP
    Uint16 pidTi;                   // FA-06  »ı·ÖÊ±¼äI
    Uint16 pidTd;                   // FA-07  Î¢·ÖÊ±¼äD
    Uint16 reverseCutOffFrq;        // FA-08  PID·´×ª½ØÖ¹ÆµÂÊ
    Uint16 pidErrMin;               // FA-09  PIDÆ«²î¼«ÏŞ
    Uint16 pidDLimit;               // FA-10  PIDÎ¢·ÖÏŞ·ù
    Uint16 pidSetChangeTime;        // FA-11  PID¸ø¶¨±ä»¯Ê±¼ä
    Uint16 pidFdbLpfTime;           // FA-12  PID·´À¡ÂË²¨Ê±¼ä
    Uint16 pidOutLpfTime;           // FA-13  PIDÊä³öÂË²¨Ê±¼ä
    Uint16 pidSampleTime;           // FA-14  PID²ÉÑùÖÜÆÚ(ÔİÎ´×ö)
    Uint16 pidKp2;                  // FA-15  PID±ÈÀıÔöÒæP2
    Uint16 pidTi2;                  // FA-16  PID»ı·ÖÊ±¼äI2
    Uint16 pidTd2;                  // FA-17  PIDÎ¢·ÖÊ±¼äD2
    Uint16 pidParaChgCondition;     // FA-18  PID²ÎÊıÇĞ»»Ìõ¼ş
    Uint16 pidParaChgDelta1;        // FA-19  PID²ÎÊıÇĞ»»Æ«²î1
    Uint16 pidParaChgDelta2;        // FA-20  PID²ÎÊıÇĞ»»Æ«²î2
    Uint16 pidInit;                 // FA-21  PID³õÖµ
    Uint16 pidInitTime;             // FA-22  PID³õÖµ±£³ÖÊ±¼ä
    Uint16 pidOutDeltaMax;          // FA-23  PIDÁ½´ÎÊä³öÖ®¼äÆ«²îµÄ×î´óÖµ
    Uint16 pidOutDeltaMin;          // FA-24  PIDÁ½´ÎÊä³öÖ®¼äÆ«²îµÄ×îĞ¡Öµ
    Uint16 pidIAttribute;           // FA-25  PID»ı·ÖÊôĞÔ
    Uint16 pidFdbLoseDetect;        // FA-26  PID·´À¡¶ªÊ§¼ì²âÖµ
    Uint16 pidFdbLoseDetectTime;    // FA-27  PID·´À¡¶ªÊ§¼ì²âÊ±¼ä
    Uint16 pidCalcMode;             // FA-28  PIDÔËËãÄ£Ê½(Í£»úÊÇ·ñÔËËã). ¹©Ë®Ä£Ê½ÏÂ£¬Í£»úÊ±PIDÒ²¼ÆËã.

//======================================
// FB °ÚÆµ¡¢¶¨³¤ºÍ¼ÆÊı
    Uint16 swingBaseMode;           // FB-00  °ÚÆµÉè¶¨·½Ê½
    Uint16 swingAmplitude;          // FB-01  °ÚÆµ·ù¶È
    Uint16 swingJumpRange;          // FB-02  Í»ÌøÆµÂÊ·ù¶È
    Uint16 swingPeriod;             // FB-03  °ÚÆµÖÜÆÚ
    Uint16 swingRiseTimeCoeff;      // FB-04  °ÚÆµµÄÈı½Ç²¨ÉÏÉıÊ±¼ä
    Uint16 lengthSet;               // FB-05  Éè¶¨³¤¶È
    Uint16 lengthCurrent;           // FB-06  Êµ¼Ê³¤¶È
    Uint16 lengthPulsePerMeter;     // FB-07  Ã¿Ã×Âö³åÊı£¬µ¥Î»: 0.1
    Uint16 counterSet;              // FB-08  Éè¶¨¼ÆÊıÖµ
    Uint16 counterPoint;            // FB-09  Ö¸¶¨¼ÆÊıÖµ

//======================================
// FC ¶à¶ËËÙ¡¢PLC
    Uint16 plcFrq[PLC_STEP_MAX];                  // FC-00  --FC-15   ¶à¶ÎÖ¸Áî0-¶à¶ÎÖ¸Áî15
    Uint16 plcRunMode;                  // FC-16  PLCÔËĞĞ·½Ê½
    Uint16 plcPowerOffRemMode;          // FC-17  PLCµôµç¼ÇÒäÑ¡Ôñ
    struct PLC_STRUCT plcAttribute[PLC_STEP_MAX]; // FC-18  --FC-49   PLCÔËĞĞÊ±¼ä£¬¼Ó¼õËÙÊ±¼äÑ¡Ôñ
    Uint16 plcTimeUnit;                 // FC-50  PLCÔËĞĞÊ±¼äµ¥Î»
    Uint16 plcFrq0Src;                  // FC-51  ¶à¶ÎÖ¸Áî0¸ø¶¨·½Ê½

// FD Í¨Ñ¶²ÎÊı
    Uint16 commBaudRate;            // FD-00  ²¨ÌØÂÊ
    Uint16 commParity;              // FD-01  Êı¾İ¸ñÊ½
    Uint16 commSlaveAddress;        // FD-02  ±¾»úµØÖ·
    Uint16 commDelay;               // FD-03  Ó¦´ğÑÓ³Ù
    Uint16 commOverTime;            // FD-04  Í¨Ñ¶³¬Ê±Ê±¼ä
    Uint16 commProtocol;            // FD-05  Í¨Ñ¶Êı¾İ´«ËÍ¸ñÊ½Ñ¡Ôñ   
    Uint16 commReadCurrentPoint;    // FD-06  Í¨Ñ¶¶ÁÈ¡µçÁ÷·Ö±æÂÊ
    Uint16 commMaster;              // FD-07  Í¨Ñ¶Ö÷´Ó·½Ê½

// FE  280ÓĞ320Ã»ÓĞµÄ¹¦ÄÜÂë
    Uint16 userCustom[FENUM];       // FE ÓÃ»§¶¨ÖÆ¹¦ÄÜÂë×é

// FF ³§¼Ò²ÎÊı
    Uint16 factoryPassword;         // FF-00  ³§¼ÒÃÜÂë
    Uint16 inverterType;            // FF-01  ±äÆµÆ÷»úĞÍ
    Uint16 inverterGpType;          // FF-02  G/PĞÍºÅ
    Uint16 inverterPower;           // FF-03  ±äÆµÆ÷¹¦ÂÊ
    Uint16 tempCurve;               // FF-04  ÎÂ¶ÈÇúÏß
    
    Uint16 uvGainWarp;              // FF-05  UVÁ½ÏàµçÁ÷²ÉÑùÔöÒæÆ«²î
    Uint16 funcSoftVersion;         // FF-06  ±£Áô
    Uint16 motorSoftVersion;        // FF-07  ĞÔÄÜÈí¼ş°æ±¾ºÅ
    Uint16 volJudgeCoeff;           // FF-08  µçÑ¹Ğ£ÕıÏµÊı
    Uint16 curJudgeCoeff;           // FF-09  µçÁ÷Ğ£ÕıÏµÊı
    
    Uint16 motorDebugFc;            // FF-10  ĞÔÄÜµ÷ÊÔ¹¦ÄÜÂëÏÔÊ¾¸öÊı
    Uint16 aiaoCalibrateDisp;       // FF-11  AIAOĞ£Õı¹¦ÄÜÂëÏÔÊ¾
    Uint16 memoryAddr;              // FF-12  ÄÚ´æµØÖ·²é¿´

// FP ÓÃ»§ÃÜÂë, ²ÎÊı³õÊ¼»¯
    Uint16 userPassword;            // FP-00  ÓÃ»§ÃÜÂë
    Uint16 paraInitMode;            // FP-01  ²ÎÊı³õÊ¼»¯
    Uint16 funcParaView;            // FP-02  ¹¦ÄÜ²ÎÊıÄ£Ê½ÊôĞÔ
    Uint16 menuMode;                // FP-03  ¸öĞÔ»¯²ÎÊıÄ£Ê½Ñ¡Ôñ
    
    Uint16 userPasswordReadOnly;    // FP-04  Ö»¶ÁÓÃ»§ÃÜÂë
    Uint16 rsvdFp;                  // FP-05  ±£Áô

// A0 ×ª¾Ø¿ØÖÆºÍÏŞ¶¨²ÎÊı
    Uint16 torqueCtrl;              // A0-00  ×ª¾Ø¿ØÖÆ
    Uint16 driveUpperTorqueSrc;     // A0-01  Çı¶¯×ª¾ØÉÏÏŞÔ´
    Uint16 brakeUpperTorqueSrc;     // A0-02  ÖÆ¶¯×ª¾ØÉÏÏŞÔ´
    Uint16 driveUpperTorque;        // A0-03  Çı¶¯×ª¾ØÉÏÏŞ
    Uint16 torqueFilter;            // A0-04  ×ª¾ØÂË²¨
    Uint16 torqueCtrlFwdMaxFrq;     // A0-05  ×ª¾Ø¿ØÖÆÕıÏò×î´óÆµÂÊ
    Uint16 torqueCtrlRevMaxFrq;     // A0-06  ×ª¾Ø¿ØÖÆ·´Ïò×î´óÆµÂÊ
    Uint16 torqueCtrlAccTime;       // A0-07  ×ª¾Ø¼ÓËÙÊ±¼ä
    Uint16 torqueCtrlDecTime;       // A0-08  ×ª¾Ø¼õËÙÊ±¼ä
    
// A1 ĞéÄâDI¡¢ĞéÄâDO
    Uint16 vdiFunc[5];              // A1-00  --A1-04 VDI1¶Ë×Ó¹¦ÄÜÑ¡Ôñ
    Uint16 vdiSrc;                  // A1-05  VDI¶Ë×ÓÓĞĞ§×´Ì¬À´Ô´
    Uint16 vdiFcSet;                // A1-06  VDI¶Ë×Ó¹¦ÄÜÂëÉè¶¨ÓĞĞ§×´Ì¬
    Uint16 aiAsDiFunc[3];           // A1-07  --A1-09 AI1¶Ë×Ó¹¦ÄÜÑ¡Ôñ£¨µ±×÷DI£©
    Uint16 diLogicAiAsDi;           // A1-10  AI×÷ÎªDIÓĞĞ§×´Ì¬Ñ¡Ôñ
    Uint16 vdoFunc[5];              // A1-11  --A1-15 ĞéÄâVDO1¡«VDO5Êä³öÑ¡Ôñ
    Uint16 vdoDelayTime[5];         // A1-16  --A1-20 VDO1¡«VDO5ÑÓ³ÙÊ±¼ä
    Uint16 vdoLogic;                // A1-21  VDOÊä³ö¶Ë×ÓÓĞĞ§×´Ì¬Ñ¡Ôñ
    
// A2 µÚ2µç»ú²ÎÊı
    struct MOTOR_FC motorFcM2;      // µÚ2µç»ú²ÎÊı
    
// A3 µÚ2µç»ú²ÎÊı
    struct MOTOR_FC motorFcM3;      // µÚ3µç»ú²ÎÊı
    
// A4 µÚ2µç»ú²ÎÊı
    struct MOTOR_FC motorFcM4;      // µÚ4µç»ú²ÎÊı
    
// A5 ¿ØÖÆÓÅ»¯²ÎÊı
    Uint16 pwmMode;                 // A5-00    DPWMÇĞ»»ÉÏÏŞÆµÂÊ
    Uint16 modulationMode;          // A5-01    µ÷ÖÆ·½Ê½£¬0-Òì²½µ÷ÖÆ£¬1-Í¬²½µ÷ÖÆ
    Uint16 deadCompMode;            // A5-02    ËÀÇø²¹³¥Ä£Ê½Ñ¡Ôñ
    Uint16 softPwm;                 // A5-03    Ëæ»úPWM
    Uint16 cbcEnable;               // A5-04    Öğ²¨ÏŞÁ÷Ê¹ÄÜ
    Uint16 curSampleDelayComp;      // A5-05    µçÁ÷¼ì²âÑÓÊ±²¹³¥
    Uint16 uvPoint;                 // A5-06    Ç·Ñ¹µãÉèÖÃ
    Uint16 svcMode;                 // A5-07    SVCÓÅ»¯Ñ¡Ôñ 0-²»ÓÅ»¯  1-ÓÅ»¯Ä£Ê½1  2-ÓÅ»¯Ä£Ê½2
    Uint16 deadTimeSet;             // A5-08    ËÀÇøÊ±¼äµ÷Õû-1140V×¨ÓÃ
    Uint16 ovPointSet;              // A5-09    ¹ıÑ¹µãÉèÖÃ

      
//======================================
// A6 Ä£ÄâÁ¿ÇúÏß
    Uint16 curveSet4P1[8];          // A6-00    --A6-07  ÇúÏß4£¬4µã£¬×î´óÖµ£¬×îĞ¡Öµ£¬2¸öÖĞ¼äµã
    Uint16 curveSet4P2[8];          // A6-08    --A6-15  ÇúÏß5£¬4µã
    Uint16 rsvdA41[8];

    struct AI_JUMP aiJumpSet[AI_NUMBER]; // A6-24 --A6-29, AI1, AI2, AI3ÌøÔ¾

// A7 ±£Áô               
    Uint16 plcEnable;               // A7-00 PLC¿¨¹¦ÄÜÑ¡Ôñ
    Uint16 outPortControl;          // A7-01 Êä³ö¶Ë×Ó¿ØÖÆ
    Uint16 plcAI3Cfg;               // A7-02 PLC AI3¹¦ÄÜÅäÖÃ
    Uint16 fmpValue;                // A7-03 FMPÊä³ö 
    Uint16 ao1Value;                // A7-04 AO1Êä³ö
    Uint16 inPortOut;               // A7-05 ¿ª¹ØÁ¿Êä³ö
    Uint16 plcFrqSet;               // A7-06 PLC¿¨ÆµÂÊ¸ø¶¨
    Uint16 plcTorqueSet;            // A7-07 PLC¿¨×ª¾Ø¸ø¶¨
    Uint16 plcCmd;                  // A7-08 PLC¿¨ÃüÁî¸ø¶¨
    Uint16 plcErrorCode;            // A7-09 PLC¿¨¹ÊÕÏ¸ø¶¨
    Uint16 rsvdA7[2];

    
// A8 ±£Áô  
    Uint16 p2pEnable;               // A8-00 µã¶ÔµãÍ¨Ñ¶¹¦ÄÜÑ¡Ôñ
    Uint16 p2pTypeSel;              // A8-01 Ö÷´ÓÑ¡Ôñ
    Uint16 p2pSendDataSel;          // A8-02 Ö÷»ú·¢ËÍÊı¾İ   0:Êä³ö×ª¾Ø  1:ÔËĞĞÆµÂÊ  2:Éè¶¨ÆµÂÊ  3:·´À¡ËÙ¶È
    Uint16 p2pRevDataSel;           // A8-03 ´Ó»ú½ÓÊÕÊı¾İ   0:×ª¾Ø¸ø¶¨  1:ÆµÂÊ¸ø¶¨  
    Uint16 p2pRevOffset;            // A8-04 ½ÓÊÜÊı¾İÁãÆ«
    Uint16 p2pRevGain;              // A8-05 ½ÓÊÕÊı¾İÔöÒæ
    Uint16 p2pOverTime;             // A8-06 µã¶ÔµãÍ¨Ñ¶ÖĞ¶Ï¼ì²âÊ±¼ä
    Uint16 p2pSendPeriod;           // A8-07 µã¶ÔµãÍ¨Ñ¶Ö÷»úÊı¾İ·¢ËÍÖÜÆÚ
    
// A9 ±£Áô  
    Uint16 A9[A9NUM];               // A9

// AA Ê¸Á¿ÓÅ»¯²ÎÊı

    Uint16 AA[AANUM];               // AA
    #if 0
    Uint16 motorCtrlM1;             // AA-00 Àø´Åµ÷Õû·½Ê½
    Uint16 motorCtrlM2;             // AA-01 ×î´óµçÑ¹¼ÆËã·½Ê½
    Uint16 motorCtrlM3;             // AA-02 Àø´ÅµçÁ÷µ÷½ÚÆ÷KP
    Uint16 motorCtrlM4;             // AA-03 Àø´ÅµçÁ÷µ÷½ÚÆ÷KI
    Uint16 motorCtrlM5;             // AA-04 Àø´ÅµçÁ÷ÕıÏòĞŞÕıÁ¿
    Uint16 motorCtrlM6;             // AA-05 Àø´ÅµçÁ÷·´ÏòĞŞÕıÁ¿
    Uint16 motorCtrlM7;             // AA-06 ×ª²îµ÷½ÚÉÏÏŞ
    Uint16 motorCtrlM8;             // AA-07 ×ª²îµ÷½ÚÔöÒæ
    Uint16 motorCtrlM9;             // AA-08 »¥¸ĞÔöÒæ
    Uint16 motorCtrlM10;            // AA-09 Êä³öÆµÂÊĞŞÕıÄ£Ê½
    Uint16 motorCtrlM11;            // AA-10 µçÑ¹ĞŞÕıãĞÖµµ÷Õû
    Uint16 motorCtrlM12;            // AA-11 µçÑ¹ĞŞÕıÔöÒæ
    Uint16 motorCtrlM13;            // AA-12 ËÙ¶È»·µ÷Õû
    Uint16 motorCtrlM14;            // AA-13 Ğı±ä¼ì²âÂË²¨
    Uint16 motorCtrlM15;            // AA-14 Ğı±ä½Ç¶È²¹³¥
    Uint16 motorCtrlM16;            // AA-15 SVC×ª¾Ø¿ØÖÆÓÅ»¯
    #endif
// AB VFÓÅ»¯²ÎÊı    
    Uint16 AB[ABNUM];                // AB
    #if 0
    Uint16 vfCtrlM2;                // AB-01 DPWMÇĞ»»ÏÂÏŞÆµÂÊ   86
    Uint16 vfCtrlM3;                // AB-02 ËÀÇø²¹³¥ÓÅ»¯¿ªÆô   87
    Uint16 vfCtrlM4;                // AB-03 ËÀÇøÇ¯Î»²¹³¥ÏµÊı    1
    Uint16 vfCtrlM5;                // AB-04 Ç¯Î»ÓÅ»¯ÏÂÏŞÆµÂÊ   101
    Uint16 vfCtrlM6;                // AB-05 Ç¯Î»ÓÅ»¯ÉÏÏŞÆµÂÊ   102
    Uint16 vfCtrlM7;                // AB-06 Õñµ´ÒÖÖÆÄ£Ê½       89
    Uint16 vfCtrlM8;                // AB-07 Õñµ´ÒÖÖÆ·ùÖµµ÷Õû   90
    #endif
// AC AIAOĞ£Õı
    struct ANALOG_CALIBRATE_CURVE aiCalibrateCurve[AI_NUMBER];  // AC-00    ----AC-11, AI1/2/3Ğ£ÕıÇúÏß
    struct ANALOG_CALIBRATE_CURVE aoCalibrateCurve[AO_NUMBER];  // AC-12    ----AC-19, AO1/AO2Ğ£ÕıÇúÏß
    
// AD ±£Áô    
    Uint16 AD[ADNUM]; 

// AE AIAO³ö³§Ğ£ÕıÖµ
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
    Uint16 rsvd4All;                // ±£Áô£¬·ÅÔÚ×îÇ°Ãæ
    Uint16 eepromCheckWord1;        //        eepromCheckWord1
    Uint16 eepromCheckWord2;        //        eepromCheckWord2
    Uint16 aiaoChkWord;             // AIAO³ö³§Ğ£Õı

//======================================
// REMEMBER µôµç±£´æ£¬¹²48¸ö
    Uint16 extendType;                  // FR-00  extendType
    Uint16 plcStepRem;                  // FR-01  PLCµ±Ç°step
    Uint16 plcTimeHighRem;              // FR-02  PLCµ±Ç°stepÔËĞĞµÄÊ±¼ä£¬¸ßÎ»
    Uint16 plcTimeLowRem;               // FR-03  PLCµ±Ç°stepÔËĞĞµÄÊ±¼ä£¬µÍÎ»
    Uint16 dispParaRunBit;              // FR-04  ÔËĞĞÊ±LEDÏÔÊ¾²ÎÊıµÄbitÎ»Öµ
    Uint16 dispParaStopBit;             // FR-05  Í£»úÊ±LEDÏÔÊ¾²ÎÊıµÄbitÎ»ÖÃ
    Uint16 runTimeAddupSec;             // FR-06  ÀÛ¼ÆÔËĞĞÊ±¼äµÄs(Ãë)
    Uint16 counterTicker;               // FR-07  ¼ÆÊıÆ÷ÊäÈëµÄticker
    Uint16 lengthTickerRemainder;       // FR-08  ³¤¶È¼ÆÊıÆ÷µÄtickerDeltaµÄRemainder
    Uint16 frqComm;                     // FR-09  Í¨Ñ¶ĞŞ¸ÄÆµÂÊÖµ, 100.00%-maxFrq
    Uint16 upDownFrqRem;                // FR-10  UP/DOWNµÄÆµÂÊ
    Uint16 pmsmRotorPos;                // FR-11  Í¬²½»ú×ª×ÓÎ»ÖÃ
    Uint16 powerAddupInt;               // FR-12  ÀÛ¼ÆºÄµçÁ¿¸¨Öú¼ÆËã
    Uint16 powerUpTimeAddupSec;         // FR-13  ÀÛ¼ÆÉÏµçÊ±¼äµÄs(Ãë)
    Uint16 errorFrqUnit;                // FR-14  ¹ÊÕÏÊ±ÆµÂÊ¼ÇÂ¼
    Uint16 saveUserParaFlag1;           // FR-15  ÒÑ±£´æÓÃ»§²ÎÊı±êÖ¾1
    Uint16 saveUserParaFlag2;           // FR-16  ÒÑ±£´æÓÃ»§²ÎÊı±êÖ¾2
    Uint16 speedFdbDir;                 // FR-17  µç»ú·´À¡ËÙ¶È·½Ïò
    Uint16 rsvdRem[2];                  // FR-18~FR-19
    Uint16 rsvdRem1[23];                // Ô¤Áô
    Uint16 remPOffMotorCtrl[REM_P_OFF_MOTOR];  // FR-43~FR-47  ĞÔÄÜÊ¹ÓÃµÄµôµç¼ÇÒä
//======================================
    Uint16 u0[U0NUM];               // U0 ÏÔÊ¾
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
    Uint16 uf[UFNUM];               // UF, ÏÔÊ¾£¬ĞÔÄÜµ÷ÊÔÊ¹ÓÃ
//======================================
};


//=====================================================================
//
// ¹¦ÄÜÂëµÄ¶¨Òå¡£
// ÁªºÏÌå£¬³ÉÔ±·Ö±ğÎªÊı×é£¬½á¹¹Ìå£¬½á¹¹Ìå
// ÓÚÊÇ£¬Ò»¸ö¹¦ÄÜÂëµÄ·ÃÎÊ£¬ÓĞÈıÖÖ·½Ê½:
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


// Ö÷¸¨ÆµÂÊÔ´Ñ¡Ôñ
#define FUNCCODE_frqXySrc_FC                0   // ¹¦ÄÜÂëÉè¶¨£¬µôµç²»¼ÇÒä
#define FUNCCODE_frqXySrc_FC_P_OFF_REM      1   // ¹¦ÄÜÂëÉè¶¨£¬µôµç¼ÇÒä
#define FUNCCODE_frqXySrc_AI1               2   // AI1
#define FUNCCODE_frqXySrc_AI2               3   // AI2
#define FUNCCODE_frqXySrc_AI3               4   // AI3
#define FUNCCODE_frqXySrc_PULSE             5   // PULSEÂö³åÉè¶¨(DI5)
#define FUNCCODE_frqXySrc_MULTI_SET         6   // ¶à¶ÎÖ¸Áî
#define FUNCCODE_frqXySrc_PLC               7   // PLC
#define FUNCCODE_frqXySrc_PID               8   // PID
#define FUNCCODE_frqXySrc_COMM              9   // Í¨Ñ¶Éè¶¨

// ¸¨ÖúÆµÂÊÔ´Y·¶Î§Ñ¡Ôñ
#define FUNCCODE_frqYRangeBase_MAX_FRQ      0   // Ïà¶ÔÓÚ×î´óÆµÂÊ
#define FUNCCODE_frqYRangeBase_FRQ_X        1   // Ïà¶ÔÓÚÖ÷ÆµÂÊÔ´X

// ÆµÂÊÔ´(ÇĞ»»¹ØÏµ)Ñ¡Ôñ
#define FUNCCODE_frqCalcSrc_X               0   // Ö÷ÆµÂÊÔ´X
#define FUNCCODE_frqCalcSrc_COMPOSE         1   // Ö÷¸¨ÔËËã½á¹û
#define FUNCCODE_frqCalcSrc_X_OR_Y          2   // Ö÷ <--> ¸¨
#define FUNCCODE_frqCalcSrc_X_OR_COMPOSE    3   // Ö÷ <--> Ö÷¸¨ÔËËã½á¹û
#define FUNCCODE_frqCalcSrc_Y_OR_COMPOSE    4   // ¸¨ <--> Ö÷¸¨ÔËËã½á¹û

// Ö÷¸¨ÆµÂÊÔËËã¹ØÏµ
#define FUNCCODE_frqCalcSrc_ADD             0   // Ö÷ + ¸¨
#define FUNCCODE_frqCalcSrc_SUBTRATION      1   // Ö÷ - ¸¨
#define FUNCCODE_frqCalcSrc_MAX             2   // MAX(Ö÷, ¸¨)
#define FUNCCODE_frqCalcSrc_MIN             3   // MIN(Ö÷, ¸¨)
#define FUNCCODE_frqCalcSrc_4               4   // 
#define FUNCCODE_frqCalcSrc_5               5   // 

// ÉÏÏŞÆµÂÊÔ´
#define FUNCCODE_upperFrqSrc_FC         0   // ¹¦ÄÜÂëÉè¶¨
#define FUNCCODE_upperFrqSrc_AI1        1   // AI1
#define FUNCCODE_upperFrqSrc_AI2        2   // AI2
#define FUNCCODE_upperFrqSrc_AI3        3   // AI3
#define FUNCCODE_upperFrqSrc_PULSE      4   // PULSEÂö³åÉè¶¨(DI5)
#define FUNCCODE_upperFrqSrc_COMM       5   // Í¨Ñ¶¸ø¶¨

// ÆµÂÊÖ¸ÁîĞ¡Êıµã
#define FUNCCODE_frqPoint_1             0   // 0: 0¸öĞ¡Êıµã£¬1Hz
#define FUNCCODE_frqPoint_0_1           1   // 1: 1¸öĞ¡Êıµã£¬0.1Hz
#define FUNCCODE_frqPoint_0_01          2   // 2: 2¸öĞ¡Êıµã£¬0.01Hz

// ÔØ²¨ÆµÂÊµ÷ÕûÑ¡Ôñ
//#define FUNCCODE_autoCarrierFrq_0

// ¼Ó¼õËÙÊ±¼äµÄµ¥Î»
#define FUNCCODE_accDecTimeUnit_0POINT  0   // 0¸öĞ¡Êıµã£¬1s
#define FUNCCODE_accDecTimeUnit_1POINT  1   // 1¸öĞ¡Êıµã£¬0.1s
#define FUNCCODE_accDecTimeUnit_2POINT  2   // 2¸öĞ¡Êıµã£¬0.01s

// ÊıÖµÉè¶¨ÆµÂÊ¼ÇÒäÉè¶¨
#define FUNCCODE_frqRemMode_POWEROFF_NO     0   // µôµç²»¼ÇÒä
#define FUNCCODE_frqRemMode_POWEROFF_YES    1   // µôµç¼ÇÒä
#define FUNCCODE_frqRemMode_STOP_NO         0   // Í£»ú²»¼ÇÒä
#define FUNCCODE_frqRemMode_STOP_YES        1   // Í£»ú¼ÇÒä

// ¼Ó¼õËÙ·½Ê½
#define FUNCCODE_accDecSpdCurve_LINE        0   // Ö±Ïß¼Ó¼õËÙ
#define FUNCCODE_accDecSpdCurve_S_CURVE_A   1   // SÇúÏß1£¬ÆÕÍ¨¶ş´Î·½
#define FUNCCODE_accDecSpdCurve_S_CURVE_B   2   // SÇúÏß2£¬²Î¿¼ÈıÁâSÇúÏßB
#define ACC_DEC_LINE    FUNCCODE_accDecSpdCurve_LINE
#define ACC_DEC_SA      FUNCCODE_accDecSpdCurve_S_CURVE_A
#define ACC_DEC_SB      FUNCCODE_accDecSpdCurve_S_CURVE_B

// ×ª¾ØÉÏÏŞÔ´
#define FUNCCODE_upperTorqueSrc_FC      0   // ¹¦ÄÜÂëÉè¶¨
#define FUNCCODE_upperTorqueSrc_AI1     1   // AI1
#define FUNCCODE_upperTorqueSrc_AI2     2   // AI2
#define FUNCCODE_upperTorqueSrc_AI3     3   // AI3
#define FUNCCODE_upperTorqueSrc_PULSE   4   // PULSE
#define FUNCCODE_upperTorqueSrc_COMM    5   // Í¨Ñ¶
#define FUNCCODE_upperTorqueSrc_MIN_AI1_AI2 6  // min(ai1,ai2)
#define FUNCCODE_upperTorqueSrc_MAX_AI1_AI2 7  // max(ai1,ai2)

// FVCµÄPG¿¨Ñ¡Ôñ, 0-QEP1(±¾µØPG),1-QEP2(À©Õ¹PG)
#define FUNCCODE_fvcPgSrc_QEP1          0   // QEP1
#define FUNCCODE_fvcPgSrc_QEP2          1   // QEP2, À©Õ¹PG¿¨

#define TIME_UNIT_ACC_DEC_SPEED         100 // ¼Ó¼õËÙÊ±¼äµ¥Î», ms


// VFÇúÏßÉè¶¨
#define FUNCCODE_vfCurve_Line               0   // Ö±ÏßVF
#define FUNCCODE_vfCurve_DOT                1   // ¶àµãVF
#define FUNCCODE_vfCurve_SQUARE             2   // Æ½·½VF
#define FUNCCODE_vfCurve_ALL_SEPARATE       10  // VFÍêÈ«·ÖÀëÄ£Ê½
#define FUNCCODE_vfCurve_HALF_SEPARATE      11  // VF°ë·ÖÀëÄ£Ê½

// vfVoltageSrc, VF·ÖÀëµÄµçÑ¹Ô´
#define FUNCCODE_vfVoltageSrc_FC            0   // ¹¦ÄÜÂëÉè¶¨
#define FUNCCODE_vfVoltageSrc_AI1           1   // AI1
#define FUNCCODE_vfVoltageSrc_AI2           2   // AI2
#define FUNCCODE_vfVoltageSrc_AI3           3   // AI3
#define FUNCCODE_vfVoltageSrc_PULSE         4   // PULSEÂö³åÉè¶¨(DI5)
#define FUNCCODE_vfVoltageSrc_MULTI_SET     5   // ¶à¶ÎÖ¸Áî
#define FUNCCODE_vfVoltageSrc_PLC           6   // PLC
#define FUNCCODE_vfVoltageSrc_PID           7   // PID
#define FUNCCODE_vfVoltageSrc_COMM          8   // Í¨Ñ¶Éè¶¨

// Î»ÖÃ¿ØÖÆÑ¡Ôñ
#define FUNCCODE_posCtrl_NONE               0   // ·ÇÎ»ÖÃ¿ØÖÆ
#define FUNCCODE_posCtrl_POSITION_CTRL      1   // Î»ÖÃ¿ØÖÆ
#define FUNCCODE_posCtrl_SWITCH_TO_PC       2   // ËÙ¶È/×ª¾Ø¿ØÖÆ<->Î»ÖÃ¿ØÖÆ
#define FUNCCODE_posCtrl_SWITCH_FROM_PC     3   // Î»ÖÃ¿ØÖÆ<->ËÙ¶È/×ª¾Ø¿ØÖÆ

// Î»ÖÃ¿ØÖÆÄ£Ê½
#define FUNCCODE_pcMode_PCMD            0   // Pcmd
#define FUNCCODE_pcMode_APTP            1   // APTP
#define FUNCCODE_pcMode_SWITCH_TO_APTP  2   // Pcmd<->AP2P

// Î»ÖÃÖ¸ÁîÂö³å·½Ê½
#define FUNCCODE_pcPulseType_PULSE_AND_DIR  0   // Âö³å+·½Ïò
#define FUNCCODE_pcPulseType_QUADRATURE     1   // 2Â·Õı½»Âö³å
#define FUNCCODE_pcPulseType_CW_AND_CCW     2   // CW+CCW

// ¶¨Î»¿ØÖÆÄ£Ê½
#define FUNCCODE_aptpMode_RELATIVE      0   // Ïà¶ÔÊ½
#define FUNCCODE_aptpMode_ABSOLUTE      1   // ¾ø¶ÔÊ½
#define FUNCCODE_aptpMode_INDEX         2   // ·Ö¶ÈÅÌ

// Î»ÖÃ¿ØÖÆÁãµãÑ¡Ôñ
#define FUNCCODE_pcZeroSelect_ENCODER   0   // ±àÂëÆ÷indexĞÅºÅ
#define FUNCCODE_pcZeroSelect_DI        1   // DI¶Ë×Ó

// PG¿¨°²×°Î»ÖÃ
#define FUNCCODE_pgLocation_MOTOR       0   // µç»úÖá
#define FUNCCODE_pgLocation_AXIS        1   // »ú´²Ö÷Öá

//=====================================================================
// (µç»ú)¿ØÖÆ·½Ê½
#define FUNCCODE_motorCtrlMode_SVC  0   // SVC
#define FUNCCODE_motorCtrlMode_FVC  1   // FVC
#define FUNCCODE_motorCtrlMode_VF   2   // VF

// ÃüÁîÔ´Ñ¡Ôñ
#define FUNCCODE_runSrc_PANEL       0   // ²Ù×÷Ãæ°å¿ØÖÆÍ¨µÀ
#define FUNCCODE_runSrc_DI          1   // ¶Ë×ÓÃüÁîÍ¨µÀ
#define FUNCCODE_runSrc_COMM        2   // ´®ĞĞ¿ÚÍ¨Ñ¶¿ØÖÆÍ¨µÀ
#define FUNCCODE_runSrc_AUTO_RUN    3   // ÉÏµçÔËĞĞ

// ÔËĞĞ·½Ïò
#define FUNCCODE_runDir_NO_REVERSE      0   // ·½ÏòÒ»ÖÂ
#define FUNCCODE_runDir_REVERSE         1   // ·½ÏòÏà·´

// µ÷Ğ³Ñ¡Ôñ
#define FUNCCODE_tuneCmd_NONE           0   // ÎŞ²Ù×÷
#define FUNCCODE_tuneCmd_ACI_STATIC     1   // Òì²½»ú¾²Ö¹µ÷Ğ³
#define FUNCCODE_tuneCmd_ACI_WHOLE      2   // Òì²½»úÍêÕûµ÷Ğ³
#define FUNCCODE_tuneCmd_PMSM_11        11  // Í¬²½»ú
#define FUNCCODE_tuneCmd_PMSM_12        12  // Í¬²½»ú
#define FUNCCODE_tuneCmd_PMSM_13        13  // Í¬²½»ú

// ¶Ë×ÓÃüÁî·½Ê½
#define FUNCCODE_diControlMode_2LINE1   0   // Á½ÏßÊ½1
#define FUNCCODE_diControlMode_2LINE2   1   // Á½ÏßÊ½2
#define FUNCCODE_diControlMode_3LINE1   2   // ÈıÏßÊ½1
#define FUNCCODE_diControlMode_3LINE2   3   // ÈıÏßÊ½2

// ¶à¹¦ÄÜ¶Ë×ÓÊä³öÑ¡Ôñ
#define FUNCCODE_fmOutSelect_PULSE      0   // FMPÂö³åÊä³ö
#define FUNCCODE_fmOutSelect_DO         1   // DO
#define FUNCCODE_fmOutSelect_AO         2   // AO

// Æô¶¯·½Ê½
#define FUNCCODE_startMode_DIRECT_START 0   // Ö±½ÓÆô¶¯
#define FUNCCODE_startMode_SPEED_TRACK  1   // ×ªËÙ¸ú×ÙÔÙÆô¶¯
#define FUNCCODE_startMode_FORE_MAG     2   // Òì²½»úÀø´ÅÆô¶¯

// Í£»ú·½Ê½
#define FUNCCODE_stopMode_DEC_STOP      0   // ¼õËÙÍ£»ú
#define FUNCCODE_stopMode_FREESTOP      1   // ×ÔÓÉÍ£»ú
#define FUNCCODE_stopMode_HURRY_STOP    2   // ¼±Í£Í£»ú

// ÆµÂÊµÍÓÚÏÂÏŞÆµÂÊÔËĞĞ¶¯×÷
#define FUNCCODE_lowerDeal_RUN_LOWER    0   // ÒÔÏÂÏŞÆµÂÊÔËĞĞ
#define FUNCCODE_lowerDeal_DELAY_STOP   1   // ÑÓÊ±Í£»ú
#define FUNCCODE_lowerDeal_RUN_ZERO     2   // ÁãËÙÔËĞĞ

// Éè¶¨ÔËĞĞÊ±¼äµ½´ï¶¯×÷Ñ¡Ôñ
#define FUNCCODE_runTimeOverAct_RUN     0   // ¼ÌĞøÔËĞĞ
#define FUNCCODE_runTimeOverAct_STOP    1   // Í£»ú

// Éè¶¨ÉÏµçÊ±¼äµ½´ï¶¯×÷Ñ¡Ôñ
#define FUNCCODE_powerUpTimeOverAct_RUN     0   // ¼ÌĞøÔËĞĞ
#define FUNCCODE_powerUpTimeOverAct_STOP    1   // Í£»ú

// PID¸ø¶¨Ô´
#define FUNCCODE_pidSetSrc_FC               0   // ¹¦ÄÜÂëÉè¶¨
#define FUNCCODE_pidSetSrc_AI1              1   // AI1
#define FUNCCODE_pidSetSrc_AI2              2   // AI2
#define FUNCCODE_pidSetSrc_AI3              3   // AI3
#define FUNCCODE_pidSetSrc_PULSE            4   // PULSE
#define FUNCCODE_pidSetSrc_COMM             5   // Í¨Ñ¶
#define FUNCCODE_pidSetSrc_MULTI_SET        6   // ¶à¶ÎÖ¸Áî

// PID·´À¡Ô´
#define FUNCCODE_pidFdbSrc_AI1              0   // AI1
#define FUNCCODE_pidFdbSrc_AI2              1   // AI2
#define FUNCCODE_pidFdbSrc_AI3              2   // AI3
#define FUNCCODE_pidFdbSrc_AI1_SUB_AI2      3   // AI1-AI2
#define FUNCCODE_pidFdbSrc_PULSE            4   // PULSE
#define FUNCCODE_pidFdbSrc_COMM             5   // Í¨Ñ¶
#define FUNCCODE_pidFdbSrc_AI1_ADD_AI2      6   // AI1+AI2
#define FUNCCODE_pidFdbSrc_MAX_AI           7   // MAX(|AI1|, |AI2|)
#define FUNCCODE_pidFdbSrc_MIN_AI           8   // MIN(|AI1|, |AI2|)

// PID²ÎÊıÇĞ»»Ìõ¼ş
#define FUNCCODE_pidParaChgCondition_NO         0   // ²»ÇĞ»»
#define FUNCCODE_pidParaChgCondition_DI         1   // DI¶Ë×Ó
#define FUNCCODE_pidParaChgCondition_PID_ERROR  2   // ¸ù¾İÆ«²î×Ô¶¯ÇĞ»»

// PIDÔËËãÄ£Ê½
#define FUNCCODE_pidCalcMode_NO             0   // Í£»úÊ±²»ÔËËã
#define FUNCCODE_pidCalcMode_YES            1   // Í£»úÊ±ÔËËã

// °ÚÆµÉè¶¨·½Ê½
#define FUNCCODE_swingBaseMode_AGAIN_FRQSETAIM  0   // Ïà¶ÔÓÚÖĞĞÄÆµÂÊ(Éè¶¨ÆµÂÊ)
#define FUNCCODE_swingBaseMode_AGAIN_MAXFRQ     1   // Ïà¶ÔÓÚ×î´óÆµÂÊ

// MF.K¼ü¹¦ÄÜÑ¡Ôñ
#define FUNCCODE_mfkKeyFunc_NONE        0   // MF.K¼ü¹¦ÄÜÎŞĞ§
#define FUNCCODE_mfkKeyFunc_SWITCH      1   // Óë²Ù×÷Ãæ°åÍ¨µÀÇĞ»»
#define FUNCCODE_mfkKeyFunc_REVERSE     2   // Õı·´×ªÇĞ»»
#define FUNCCODE_mfkKeyFunc_FWD_JOG     3   // Õı×ªµã¶¯ÃüÁî
#define FUNCCODE_mfkKeyFunc_REV_JOG     4   // ·´×ªµã¶¯ÃüÁî

// STOP/RES¼ü¹¦ÄÜ
#define FUNCCODE_stopKeyFunc_KEYBOARD   0   // Í£»ú¹¦ÄÜ½öÔÚ¼üÅÌ¿ØÖÆ·½Ê½Ê±ÓĞĞ§
#define FUNCCODE_stopKeyFunc_ALL        1   // ¾ùÓĞĞ§

// ¶à¶ÎÖ¸Áî0¸ø¶¨·½Ê½
#define FUNCCODE_plcFrq0Src_FC          0   // ¹¦ÄÜÂëFC-00¸ø¶¨
#define FUNCCODE_plcFrq0Src_AI1         1   // AI1
#define FUNCCODE_plcFrq0Src_AI2         2   // AI2
#define FUNCCODE_plcFrq0Src_AI3         3   // AI3
#define FUNCCODE_plcFrq0Src_PULSE       4   // PULSE
#define FUNCCODE_plcFrq0Src_PID         5   // PID¸ø¶¨
#define FUNCCODE_plcFrq0Src_PRESET_FRQ  6   // Ô¤ÖÃÆµÂÊ

// PLCÔËĞĞ·½Ê½
#define FUNCCODE_plcRunMode_ONCE_STOP   0   // µ¥´ÎÔËĞĞ½áÊøÍ£»ú
#define FUNCCODE_plcRunMode_ONCE_RUN    1   // µ¥´ÎÔËĞĞ½áÊø±£³ÖÖÕÖµ
#define FUNCCODE_plcRunMode_REPEAT      2   // Ò»Ö±Ñ­»·

// PLCµôµç¼ÇÒäÑ¡Ôñ
#define FUNCCODE_plcPowerOffRemMode_NO_REM  0   // µôµç²»¼ÇÒä
#define FUNCCODE_plcPowerOffRemMode_REM     1   // µôµç¼ÇÒä
// PLCÍ£»ú¼ÇÒäÑ¡Ôñ
#define FUNCCODE_plcStopRemMode_NO_REM  0   // µôµç²»¼ÇÒä
#define FUNCCODE_plcStopRemMode_REM     1   // µôµç¼ÇÒä

// PLCÔËĞĞÊ±¼äµ¥Î»
#define FUNCCODE_plcTimeUnit_S      0   // S(Ãë)
#define FUNCCODE_plcTimeUnit_H      1   // H(Ğ¡Ê±)

// µç»úÎÂ¶È´«¸ĞÆ÷ÀàĞÍ
#define FUNCCODE_tempSenorType_NONE         0       // PTC100
#define FUNCCODE_tempSenorType_PTC100       1       // PTC100
#define FUNCCODE_tempSenorType_PTC1000      2       // PTC1000
#define FUNCCODE_tempSenorType_NTC          3       // NTC

// Êı¾İ¸ñÊ½
#define FUNCCODE_sciParity_NONE     0   // ÎŞĞ£Ñé(8-N-2)
#define FUNCCODE_sciParity_EVEN     1   // Å¼Ğ£Ñé(8-E-1)
#define FUNCCODE_sciParity_ODD      2   // ÆæĞ£Ñé(8-O-1)
#define FUNCCODE_sciParity_NONE1    3   // ÎŞĞ£Ñé(8-N-1)



// ¹¦ÄÜÂëµÄÊ±¼äµ¥Î»
// ×¢Òâ£¬³ÌĞòÖĞÎªÁË¼õĞ¡²»±ØÒªµÄ¼ÆËãºÍ¿Õ¼äÕ¼ÓÃ£¬²¿·ÖÊ¹ÓÃÁË
// X * (TIME_UNIT_WAIT_STOP_BRAKE / RUN_CTRL_PERIOD) µÄ·½Ê½
// ¶ø²»ÊÇ£¬(X * TIME_UNIT_WAIT_STOP_BRAKE) / RUN_CTRL_PERIOD
// Ö®ºóĞŞ¸ÄÕâĞ©Ê±¼äµ¥Î»£¬¿ÉÄÜÓĞ±ØÒªĞŞ¸Ä¡£
#define TIME_UNIT_SEC_PER_HOUR          3600    // 1hour = 3600sec
#define TIME_UNIT_MIN_PER_HOUR          60      // 1hour = 60min
#define TIME_UNIT_SEC_PER_MIN           60      // 1min  = 60sec
#define TIME_UNIT_MS_PER_SEC            1000    // 1s = 1000ms

#define TIME_UNIT_VF_VOL_ACC_TIME       100     // VF·ÖÀëµÄµçÑ¹ÉÏÉıÊ±¼ä

#define TIME_UNIT_AI_PULSE_IN_FILTER    10      // AI,pulseInÂË²¨Ê±¼ä, ms
#define TIME_UNIT_DI_DELAY              100     // DIÊä³öÑÓ³ÙÊ±¼ä, ms
#define TIME_UNIT_DO_DELAY              100     // DOÊä³öÑÓ³ÙÊ±¼ä, ms
#define TIME_UNIT_START_FRQ_WAIT        100      // Æô¶¯ÆµÂÊ±£³ÖÊ±¼ä£¬ms
#define TIME_UNIT_START_BRAKE           100     // Æô¶¯Ö±Á÷ÖÆ¶¯Ê±¼ä£¬ms
#define TIME_UNIT_WAIT_STOP_BRAKE       100     // Í£»úÖ±Á÷ÖÆ¶¯µÈ´ıÊ±¼ä£¬ms
#define TIME_UNIT_STOP_BRAKE            100     // Í£»úÖ±Á÷ÖÆ¶¯Ê±¼ä£¬ms
#define TIME_UNIT_ZERO_SPEED_DEAD       100     // Õı·´×ªËÀÇøÊ±¼ä
#define TIME_UNIT_LOWER_STOP_DELAY      100     // ÆµÂÊµÍÓÚÏÂÏŞÆµÂÊÊ±Í£»úµÄÑÓ³ÙÊ±¼ä
#define TIME_UNIT_PID_SET_CHANGE        10      // PID¸ø¶¨±ä»¯Ê±¼ä
#define TIME_UNIT_PID_FILTER            10      // PID·´À¡£¬Êä³öÂË²¨Ê±¼ä
#define TIME_UNIT_PID_INIT              10      // PID³õÖµ±£³ÖÊ±¼ä
#define TIME_UNIT_PID_FDB_LOSE          100     // PID·´À¡¶ªÊ§¼ì²âÊ±¼ä
#define TIME_UNIT_SWING_PERIOD          100      // °ÚÆµÖÜÆÚ
#define TIME_UNIT_sciCommOverTime       100     // Í¨Ñ¶³¬Ê±Ê±¼ä
#define TIME_UNIT_ERR_AUTO_RST_DELAY    100     // ¹ÊÕÏ×Ô¶¯¸´Î»¼ä¸ôÊ±¼ä£¬ms
#define TIME_UNIT_ERR_AUTO_RST_CLR      (TIME_UNIT_SEC_PER_HOUR*100UL) // ¹ÊÕÏ×Ô¶¯¸´Î»´ÎÊıÇå³ıÊ±¼ä, 0.1h
#define TIME_UNIT_P_OFF_VOL_BACK        10      // Ë²Í£²»Í£µçÑ¹»ØÉıÅĞ¶ÏÊ±¼ä
#define TIME_UNIT_PLC                   100     // PLCÔËĞĞÊ±¼äµ¥Î»

#define TIME_UNIT_ACC_DEC_SPEED_SERVO   10      // ËÅ·ş¼Ó¼õËÙÊ±¼äµ¥Î»
#define TIME_UNIT_WAKE_UP               100     // »½ĞÑÊ±¼äµÄµ¥Î»
#define TIME_UNIT_DORMANT               100     // ĞİÃßÊ±¼äµÄµ¥Î»
#define TIME_UNIT_CURRENT_CHK           10      // µçÁ÷¼ì²âÊ±¼äµ¥Î»
#define TIME_UNIT_TORQUE_CTRL_ACC_DEC   10      // ×ª¾Ø¿ØÖÆÊ±¼äµ¥Î»
//=====================================================================

#if 0//F_DEBUG_RAM
#define ACC_DEC_T_INIT1  ((Uint32)2*TIME_UNIT_MS_PER_SEC/TIME_UNIT_ACC_DEC_SPEED)    // ¼Ó¼õËÙÊ±¼ä³ö³§Öµ£¬2s£¬»úĞÍ <= 20
#define ACC_DEC_T_INIT2  ((Uint32)5*TIME_UNIT_MS_PER_SEC/TIME_UNIT_ACC_DEC_SPEED)    // 5s£¬»úĞÍ > 20
#else
#define ACC_DEC_T_INIT1  ((Uint32)10*TIME_UNIT_MS_PER_SEC/TIME_UNIT_ACC_DEC_SPEED)   // ¼Ó¼õËÙÊ±¼ä³ö³§Öµ£¬20s£¬»úĞÍ <= 20
#define ACC_DEC_T_INIT2  ((Uint32)30*TIME_UNIT_MS_PER_SEC/TIME_UNIT_ACC_DEC_SPEED)   // 50s£¬»úĞÍ > 20
#endif

#define RATING_FRQ_INIT_0   50      // µç»ú¶î¶¨ÆµÂÊ£¬0¸öĞ¡Êıµã
#define RATING_FRQ_INIT_1   500     // µç»ú¶î¶¨ÆµÂÊ£¬1¸öĞ¡Êıµã
#define RATING_FRQ_INIT_2   5000    // µç»ú¶î¶¨ÆµÂÊ£¬2¸öĞ¡Êıµã
#define BAUD_NUM_MAX        12   // ²¨ÌØÂÊÑ¡Ôñ·¶Î§µÄ×î´óÖµ
//#define BAUD_NUM_MAX 10
#define PARA_INIT_MODE_MAX  501       // ²ÎÊı³õÊ¼»¯ÉÏÏŞÖµ
#define INV_TYPE_MAX   30

#define MENU_MODE_MAX       3   // ²Ëµ¥Ä£Ê½µÄ×î´óÖµ

//=====================================================================
// ÓĞĞ©¹¦ÄÜÂëµÄÉÏÏÂÏŞÊÇÆäËüÄ³¸ö¹¦ÄÜÂë£¬ÕâÀïÊÇÔÚfuncCodeÖĞµÄindex

// ³ÌĞòÖĞÊ¹ÓÃµÄÒ»Ğ©¹¦ÄÜÂëµÄindex
//= Èç¹ûÔö¼Ó/É¾³ıÁË¹¦ÄÜÂë£¬ÕâÀïÒªĞŞ¸Ä!
#define FACTORY_PWD_INDEX      (GetCodeIndex(funcCode.code.factoryPassword))   // FF-00 ³§¼ÒÃÜÂë
#define INV_TYPE_INDEX         (GetCodeIndex(funcCode.code.inverterType))      // FF-01 ±äÆµÆ÷»úĞÍ
#define RATING_POWER_INVERTER_INDEX  (GetCodeIndex(funcCode.code.inverterPower))     // FF-03 ±äÆµÆ÷¹¦ÂÊ
#define FUNCCODE_FACTORY_START_INDEX     (GetCodeIndex(funcCode.group.ff[0]))            // FF×éµÄ¿ªÊ¼
#define FUNCCODE_FACTORY_END_INDEX       (GetCodeIndex(funcCode.group.ff[FFNUM - 1]))    // FF×éµÄ½áÊø

#define FC_MOTOR1_START_INDEX   (GetCodeIndex(funcCode.code.motorParaM1.all[0]))      // µÚ1µç»ú²ÎÊıµÄÆğÊ¼
#define FC_MOTOR1_END_INDEX     (GetCodeIndex(funcCode.code.pgParaM1.all[sizeof(struct PG_PARA_STRUCT) - 1]))  // µÚ1µç»ú²ÎÊıµÄ½áÊø

#define FC_MOTOR2_START_INDEX   (GetCodeIndex(funcCode.code.motorFcM2.motorPara.all[0]))      // µÚ2µç»ú²ÎÊıµÄÆğÊ¼
#define FC_MOTOR2_END_INDEX     (GetCodeIndex(funcCode.code.motorFcM2.pgPara.all[sizeof(struct PG_PARA_STRUCT) - 1]))  // µÚ2µç»ú²ÎÊıµÄ½áÊø

#define FC_MOTOR3_START_INDEX   (GetCodeIndex(funcCode.code.motorFcM3.motorPara.all[0]))      // µÚ3µç»ú²ÎÊıµÄÆğÊ¼
#define FC_MOTOR3_END_INDEX     (GetCodeIndex(funcCode.code.motorFcM3.pgPara.all[sizeof(struct PG_PARA_STRUCT) - 1]))  // µÚ3µç»ú²ÎÊıµÄ½áÊø

#define FC_MOTOR4_START_INDEX   (GetCodeIndex(funcCode.code.motorFcM4.motorPara.all[0]))      // µÚ4µç»ú²ÎÊıµÄÆğÊ¼
#define FC_MOTOR4_END_INDEX     (GetCodeIndex(funcCode.code.motorFcM4.pgPara.all[sizeof(struct PG_PARA_STRUCT) - 1]))  // µÚ4µç»ú²ÎÊıµÄ½áÊø



#define PRESET_FRQ_INDEX        (GetCodeIndex(funcCode.code.presetFrq))      // F0-08   Ô¤ÖÃÆµÂÊ
#define MAX_FRQ_INDEX           (GetCodeIndex(funcCode.code.maxFrq))         // F0-10   ×î´óÆµÂÊ
#define UPPER_FRQ_INDEX         (GetCodeIndex(funcCode.code.upperFrq))       // F0-12   ÉÏÏŞÆµÂÊ
#define LOWER_FRQ_INDEX         (GetCodeIndex(funcCode.code.lowerFrq))       // F0-14   ÏÂÏŞÆµÂÊ
#define ACC_TIME1_INDEX         (GetCodeIndex(funcCode.code.accTime1))       // F0-17   ¼ÓËÙÊ±¼ä1
#define DEC_TIME1_INDEX         (GetCodeIndex(funcCode.code.decTime1))       // F0-18   ¼õËÙÊ±¼ä1
#define CARRIER_FRQ_INDEX       (GetCodeIndex(funcCode.code.carrierFrq))     // F0-15   ÔØ²¨ÆµÂÊ

#define RATING_POWER_INDEX      (GetCodeIndex(funcCode.code.motorParaM1.elem.ratingPower))    // µç»ú¶î¶¨¹¦ÂÊ
#define RATING_VOL_INDEX        (GetCodeIndex(funcCode.code.motorParaM1.elem.ratingVoltage))  // µç»ú¶î¶¨µçÑ¹
#define RATING_CUR_INDEX        (GetCodeIndex(funcCode.code.motorParaM1.elem.ratingCurrent))  // µç»ú¶î¶¨µçÁ÷
#define RATING_CUR_INDEX2       (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.ratingCurrent))    // µÚ2µç»ú¶î¶¨µçÁ÷
#define RATING_CUR_INDEX3       (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.ratingCurrent))   // µÚ3µç»ú¶î¶¨µçÁ÷
#define RATING_CUR_INDEX4       (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.ratingCurrent))   // µÚ4µç»ú¶î¶¨µçÁ÷


#define RATING_FRQ_INDEX        (GetCodeIndex(funcCode.code.motorParaM1.elem.ratingFrq))      // µç»ú¶î¶¨ÆµÂÊ

#define ZERO_LOAD_CURRENT_INDEX (GetCodeIndex(funcCode.code.motorParaM1.elem.zeroLoadCurrent))// ¿ÕÔØµçÁ÷
#define STATOR_RESISTANCE_INDEX (GetCodeIndex(funcCode.code.motorParaM1.elem.statorResistance))// ¶¨×Óµç×è

#define ANTI_VIBRATE_GAIN_INDEX (GetCodeIndex(funcCode.code.antiVibrateGain))// F3-11   Õñµ´ÒÖÖÆÔöÒæ
#define ANTI_VIBRATE_GAIN_MOTOR2_INDEX (GetCodeIndex(funcCode.code.motorFcM2.antiVibrateGain))// A4-52   Õñµ´ÒÖÖÆÔöÒæ
#define ANTI_VIBRATE_GAIN_MOTOR3_INDEX (GetCodeIndex(funcCode.code.motorFcM3.antiVibrateGain))// A5-52   Õñµ´ÒÖÖÆÔöÒæ
#define ANTI_VIBRATE_GAIN_MOTOR4_INDEX (GetCodeIndex(funcCode.code.motorFcM4.antiVibrateGain))// A6-52   Õñµ´ÒÖÖÆÔöÒæ

#define TUNE_CMD_INDEX_1  (GetCodeIndex(funcCode.code.tuneCmd))  // µ÷Ğ³
#define TUNE_CMD_INDEX_2  (GetCodeIndex(funcCode.code.motorFcM2.tuneCmd))  // µ÷Ğ³
#define TUNE_CMD_INDEX_3  (GetCodeIndex(funcCode.code.motorFcM3.tuneCmd))  // µ÷Ğ³
#define TUNE_CMD_INDEX_4  (GetCodeIndex(funcCode.code.motorFcM4.tuneCmd))  // µ÷Ğ³

#define VC_CHG_FRQ1_INDEX (GetCodeIndex(funcCode.code.vcParaM1.vcSpdLoopChgFrq1))  // Ê¸Á¿¿ØÖÆËÙ¶È»· ÇĞ»»ÆµÂÊ1
#define VC_CHG_FRQ2_INDEX (GetCodeIndex(funcCode.code.vcParaM1.vcSpdLoopChgFrq2))  // Ê¸Á¿¿ØÖÆËÙ¶È»· ÇĞ»»ÆµÂÊ2

#define VC_CHG_FRQ1_INDEX2 (GetCodeIndex(funcCode.code.motorFcM2.vcPara.vcSpdLoopChgFrq1))  // Ê¸Á¿¿ØÖÆËÙ¶È»· ÇĞ»»ÆµÂÊ1
#define VC_CHG_FRQ2_INDEX2 (GetCodeIndex(funcCode.code.motorFcM2.vcPara.vcSpdLoopChgFrq2))  // Ê¸Á¿¿ØÖÆËÙ¶È»· ÇĞ»»ÆµÂÊ2

#define VC_CHG_FRQ1_INDEX3 (GetCodeIndex(funcCode.code.motorFcM3.vcPara.vcSpdLoopChgFrq1))  // Ê¸Á¿¿ØÖÆËÙ¶È»· ÇĞ»»ÆµÂÊ1
#define VC_CHG_FRQ2_INDEX3 (GetCodeIndex(funcCode.code.motorFcM3.vcPara.vcSpdLoopChgFrq2))  // Ê¸Á¿¿ØÖÆËÙ¶È»· ÇĞ»»ÆµÂÊ2

#define VC_CHG_FRQ1_INDEX4 (GetCodeIndex(funcCode.code.motorFcM4.vcPara.vcSpdLoopChgFrq1))  // Ê¸Á¿¿ØÖÆËÙ¶È»· ÇĞ»»ÆµÂÊ1
#define VC_CHG_FRQ2_INDEX4 (GetCodeIndex(funcCode.code.motorFcM4.vcPara.vcSpdLoopChgFrq2))  // Ê¸Á¿¿ØÖÆËÙ¶È»· ÇĞ»»ÆµÂÊ2


#define TORQUE_BOOST_INDEX      (GetCodeIndex(funcCode.code.torqueBoost))    // F1-05   ×ª¾ØÌáÉı

#define TORQUE_BOOST_MOTOR2_INDEX      (GetCodeIndex(funcCode.code.motorFcM2.torqueBoost))    // D0-52   µÚ2µç»ú×ª¾ØÌáÉı
#define TORQUE_BOOST_MOTOR3_INDEX      (GetCodeIndex(funcCode.code.motorFcM3.torqueBoost))    // D0-52   µÚ3µç»ú×ª¾ØÌáÉı
#define TORQUE_BOOST_MOTOR4_INDEX      (GetCodeIndex(funcCode.code.motorFcM4.torqueBoost))    // D0-52   µÚ4µç»ú×ª¾ØÌáÉı

#define SVC_MODE_INDX           (GetCodeIndex(funcCode.code.svcMode))        // A5-07 SVCÄ£Ê½Ñ¡Ôñ
#define OV_POINT_SET_INDEX      (GetCodeIndex(funcCode.code.ovPointSet))     // A5-09 ¹ıÑ¹µãÉèÖÃ

#define VF_FRQ1_INDEX           (GetCodeIndex(funcCode.code.vfFrq1))         // F3-03   ¶àµãVFÆµÂÊµã1
#define VF_FRQ2_INDEX           (GetCodeIndex(funcCode.code.vfFrq2))         // F3-05   ¶àµãVFÆµÂÊµã2
#define VF_FRQ3_INDEX           (GetCodeIndex(funcCode.code.vfFrq3))         // F3-07   ¶àµãVFÆµÂÊµã3


#define CURVE1_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet2P1[0]))       // F2-08   AI1×îĞ¡ÊäÈë
#define CURVE1_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet2P1[2]))       // F2-10   AI1×î´óÊäÈë
#define CURVE2_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet2P2[0]))       // F2-14   AI2×îĞ¡ÊäÈë
#define CURVE2_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet2P2[2]))       // F2-16   AI2×î´óÊäÈë
#define CURVE3_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet2P3[0]))       // F2-14   AI3×îĞ¡ÊäÈë
#define CURVE3_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet2P3[2]))       // F2-16   AI3×î´óÊäÈë
#define PULSE_IN_MIN_INDEX      (GetCodeIndex(funcCode.code.curveSet2P4[0]))     // F2-20   PULSE×îĞ¡ÊäÈë
#define PULSE_IN_MAX_INDEX      (GetCodeIndex(funcCode.code.curveSet2P4[2]))     // F2-22   PULSE×î´óÊäÈë

#define CURVE4_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet4P1[0]))       // A8-00   AI4×îĞ¡ÊäÈë
#define CURVE4_INFLEX1_INDEX    (GetCodeIndex(funcCode.code.curveSet4P1[2]))       // A8-02   AI4¹Õµã1ÊäÈë
#define CURVE4_INFLEX2_INDEX    (GetCodeIndex(funcCode.code.curveSet4P1[4]))       // A8-04   AI4¹Õµã2ÊäÈë
#define CURVE4_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet4P1[6]))       // A8-06   AI4×î´óÊäÈë
#define CURVE5_MIN_INDEX        (GetCodeIndex(funcCode.code.curveSet4P2[0]))       // A8-08   AI5×îĞ¡ÊäÈë
#define CURVE5_INFLEX1_INDEX    (GetCodeIndex(funcCode.code.curveSet4P2[2]))       // A8-10   AI5¹Õµã1ÊäÈë
#define CURVE5_INFLEX2_INDEX    (GetCodeIndex(funcCode.code.curveSet4P2[4]))       // A8-12   AI5¹Õµã2ÊäÈë
#define CURVE5_MAX_INDEX        (GetCodeIndex(funcCode.code.curveSet4P2[6]))       // A8-14   AI5×î´óÊäÈë


#define ACC_TIME2_INDEX         (GetCodeIndex(funcCode.code.accTime2))       // F8-03 ¼ÓËÙÊ±¼ä2
#define DEC_TIME2_INDEX         (GetCodeIndex(funcCode.code.decTime2))       // F8-04 ¼õËÙÊ±¼ä2

#define ACC_TIME3_INDEX         (GetCodeIndex(funcCode.code.accTime3))       // F8-05 ¼ÓËÙÊ±¼ä3
#define DEC_TIME3_INDEX         (GetCodeIndex(funcCode.code.decTime3))       // F8-06 ¼õËÙÊ±¼ä3

#define ACC_TIME4_INDEX         (GetCodeIndex(funcCode.code.accTime4))       // F8-07 ¼ÓËÙÊ±¼ä4
#define DEC_TIME4_INDEX         (GetCodeIndex(funcCode.code.decTime4))       // F8-08 ¼õËÙÊ±¼ä4

#define RUN_TIME_ADDUP_INDEX    (GetCodeIndex(funcCode.code.runTimeAddup))     // F7-09  ÀÛ¼ÆÔËĞĞÊ±¼ä
#define POWER_TIME_ADDUP_INDEX  (GetCodeIndex(funcCode.code.powerUpTimeAddup)) // F7-13  ÀÛ¼ÆÉÏµçÊ±¼ä
#define POWER_ADDUP_INDEX       (GetCodeIndex(funcCode.code.powerAddup))       // F7-14  ÀÛ¼ÆºÄµçÁ¿


#define AI1_LIMIT               (GetCodeIndex(funcCode.code.ai1VoltageLimit)) //  F8-45  AI±£»¤ÏÂÏŞ
#define AI1_UPPER               (GetCodeIndex(funcCode.code.ai1VoltageUpper)) //  F8-46  AI±£»¤ÉÏÏŞ

#define PID_PARA_CHG_DELTA1_MAX (GetCodeIndex(funcCode.code.pidParaChgDelta2))  // FA-20  PID²ÎÊıÇĞ»»Æ«²î2
#define PID_PARA_CHG_DELTA2_MIN (GetCodeIndex(funcCode.code.pidParaChgDelta1))  // FA-19  PID²ÎÊıÇĞ»»Æ«²î1

#define DORMANT_UPPER           (GetCodeIndex(funcCode.code.wakeUpFrq))       // ĞİÃßÆµÂÊÉÏÏŞ
#define WAKE_UP_LIMIT           (GetCodeIndex(funcCode.code.dormantFrq))      // »½ĞÑÆµÂÊÏÂÏŞ
#define RADIATOR_TEMP_INDEX     (GetCodeIndex(funcCode.code.radiatorTemp))   // FB-19   Äæ±äÆ÷Ä£¿éÉ¢ÈÈÆ÷ÎÂ¶È
#define ERROR_LATEST1_INDEX     (GetCodeIndex(funcCode.code.errorLatest1))   // FB-20   µÚÒ»´Î¹ÊÕÏÀàĞÍ
#define ERROR_LATEST2_INDEX     (GetCodeIndex(funcCode.code.errorLatest2))   // FB-21   µÚ¶ş´Î¹ÊÕÏÀàĞÍ
#define ERROR_LATEST3_INDEX     (GetCodeIndex(funcCode.code.errorLatest3))   // FB-22   (×î½üÒ»´Î)µÚÈı´Î¹ÊÕÏÀàĞÍ
#define ERROR_FRQ_INDEX         (GetCodeIndex(funcCode.code.errorScene3.elem.errorFrq))       // FB-23   ¹ÊÕÏÊ±ÆµÂÊ
#define ERROR_CURRENT_INDEX     (GetCodeIndex(funcCode.code.errorScene3.elem.errorCurrent))   // FB-24   ¹ÊÕÏÊ±µçÁ÷
#define ERROR_UDC_INDEX         (GetCodeIndex(funcCode.code.errorScene3.elem.errorGeneratrixVoltage)) // FB-25 ¹ÊÕÏÊ±Ä¸ÏßµçÑ¹
#define ERROR_DI_STATUS_INDEX   (GetCodeIndex(funcCode.code.errorScene3.elem.errorDiStatus))  // FB-26   ¹ÊÕÏÊ±ÊäÈë¶Ë×Ó×´Ì¬
#define ERROR_DO_STATUS_INDEX   (GetCodeIndex(funcCode.code.errorScene3.elem.errorDoStatus))  // FB-27   ¹ÊÕÏÊ±Êä³ö¶Ë×Ó×´Ì¬
#define LAST_ERROR_RECORD_INDEX (GetCodeIndex(funcCode.code.errorScene1.all[sizeof(struct ERROR_SCENE_STRUCT) - 1]))  // ×îºóÒ»¸ö¹ÊÕÏ¼ÇÂ¼

#define MIN_CBC_TIME_INDEX       (GetCodeIndex(funcCode.code.cbcMinTime))         // A0-14   Öğ²¨ÏŞÁ÷Ê±¼äÏÂÏŞ
#define MAX_CBC_TIME_INDEX       (GetCodeIndex(funcCode.code.cbcMaxTime))         // A0-15   Öğ²¨ÏŞÁ÷Ê±¼äÉÏÏŞ



#define PC_LOOP_CHG_FRQ1_I      (GetCodeIndex(funcCode.code.pcLoopChgFrq1))     //          ÇĞ»»ÆµÂÊ1
#define PC_LOOP_CHG_FRQ2_I      (GetCodeIndex(funcCode.code.pcLoopChgFrq2))     //          ÇĞ»»ÆµÂÊ2

#define EEPROM_CHECK_INDEX      (GetCodeIndex(funcCode.code.eepromCheckWord1))  // eepromCheckWord1

#define RUN_TIME_ADDUP_SEC_INDEX    (GetCodeIndex(funcCode.code.runTimeAddupSec))   // FR-07 F209  ÀÛ¼ÆÔËĞĞÊ±¼äµÄs


#define EEPROM_CHECK_INDEX1     (GetCodeIndex(funcCode.code.eepromCheckWord1))  // eepromCheckWord1
#define EEPROM_CHECK_INDEX2     (GetCodeIndex(funcCode.code.eepromCheckWord2))  // eepromCheckWord2

#define SAVE_USER_PARA_PARA1    (GetCodeIndex(funcCode.code.saveUserParaFlag1))
#define SAVE_USER_PARA_PARA2    (GetCodeIndex(funcCode.code.saveUserParaFlag2))

#define AI_AO_CHK_FLAG          (GetCodeIndex(funcCode.code.aiaoChkWord))       // AIAOĞ£Õı±êÖ¾
#define AI_AO_CALIB_START       (GetCodeIndex(funcCode.code.aiFactoryCalibrateCurve[0].before1))  // aiao³§¼ÒĞ£Õı¿ªÊ¼
#define AI_AO_CALIB_STOP        (GetCodeIndex(funcCode.code.aoFactoryCalibrateCurve[0].after2))   // aiao³§¼ÒĞ£Õı½áÊø

#define AI1_CALB_START          (GetCodeIndex(funcCode.code.aiFactoryCalibrateCurve[0].before1))
#define AI2_CALB_STOP           (GetCodeIndex(funcCode.code.aiFactoryCalibrateCurve[1].after2))
#define AO1_CALB_START          (GetCodeIndex(funcCode.code.aoFactoryCalibrateCurve[0].before1))
#define AO1_CALB_STOP           (GetCodeIndex(funcCode.code.aoFactoryCalibrateCurve[0].after2)) 

//-------------------------------
#define FC_GROUP_FACTORY    FUNCCODE_GROUP_FF   // ³§¼Ò²ÎÊı×é
#define FC_GROUP_FC_MANAGE  FUNCCODE_GROUP_FP   // ¹¦ÄÜÂë¹ÜÀí
#define FC_START_GROUP      FUNCCODE_GROUP_F0   // ¹¦ÄÜÂë×éÏÔÊ¾µÄµÚ1×é
//--------------------------------


extern FUNCCODE_ALL funcCode;           // ¹¦ÄÜÂëµÄRAMÖµ
//extern FUNCCODE_ALL funcCodeEeprom;     // ¹¦ÄÜÂëµÄEEPROMÖµ

extern const Uint16 funcCodeGradeSum[];

extern Uint16 saveEepromIndex;    // DP¿¨µôµç´æ´¢
extern const Uint16 ovVoltageInitValue[];
extern const Uint16 funcCodeGradeAll[];
extern Uint16 funcCodeGradeCurMenuMode[];

#endif  // __F_FUNCCODE_H__





