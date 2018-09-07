//=====================================================================
//
// ĞÔÄÜÓë¹¦ÄÜµÄ½Ó¿Ú
//
//=====================================================================


#include "f_funcCode.h"
#include "f_runSrc.h"
#include "f_main.h"
#include "f_frqSrc.h"
#include "f_io.h"
#include "f_menu.h"
#include "f_posCtrl.h"

#if F_DEBUG_RAM
#define DEBUG_F_INTERFACE                   0   // ĞÔÄÜ, ¹¦ÄÜ½»»¥
#define DEBUG_F_POWER_ADD                   0   // ÀÛ¼ÆºÄµçÁ¿
#elif 1
#define DEBUG_F_INTERFACE                   1   // ĞÔÄÜ, ¹¦ÄÜ½»»¥
#define DEBUG_F_POWER_ADD                   1   // ÀÛ¼ÆºÄµçÁ¿
#endif

#define DROOP_VOLTAGE_LEVEL 6500  // ÏÂ´¹¿ØÖÆÄ¸ÏßµçÑ¹»ù×¼
//========================================
// ¹¦ÄÜ´«µİ¸øĞÔÄÜµÄÊı¾İ
Uint16 frq2Core;
Uint16 frqCurAim2Core;
Uint16 rsvdData;            // ±£ÁôÊı¾İ
Uint16 driveCoeffFrqFdb;
Uint16 vcSpdLoopKp1;
Uint16 vcSpdLoopKp2;
Uint16 jerkFf;

extern Uint16 vfSeparateVol;

Uint16 speedMotor;      // 
Uint16 motorRun;        // ±àÂëÆ÷·´À¡ËÙ¶È

int32 frqRun;           // ĞÔÄÜÊµ¼Ê·¢³öµÄÆµÂÊ(¶ÔVF¶øÑÔ£¬ÊÇÍ¬²½×ªËÙ), 0.01Hz
int32 frqVFRun;           // ĞÔÄÜÊµ¼Ê·¢³öµÄÆµÂÊ(¶ÔVF¶øÑÔ£¬ÊÇÍ¬²½×ªËÙ), 0.01Hz
int32 frqVFRunRemainder;  // ¼ÆËãÓàÖµ
Uint16 outVoltage;
Uint16 outCurrent;      // Q12
Uint16 generatrixVoltage;   // µ¥Î»: 0.1V
int16 outPower;         // µ¥Î»Îª0.1KW
Uint16 errorCodeFromMotor;

Uint16 errorsCodeFromMotor[2];
Uint16 motorErrInfor[5];

//#define OUT_CURRENT_FILTER_TIME 30  // Êä³öµçÁ÷ÂË²¨Ê±¼äÏµÊı
//LOCALF LowPassFilter currentInLpf = LPF_DEFALUTS;

Uint16 outCurrentDispOld;
//Uint16 PGErrorFlag;     // ²ÎÊı±æÊ¶ºóPG¿¨¹ÊÕÏ±êÖ¾  2-±àÂëÆ÷ÏßÊıÉè¶¨´íÎó  1-Î´½Ó±àÂëÆ÷

// ĞÔÄÜ´«µİ¸ø¹¦ÄÜ¼´Ê±
int16 gPhiRt;         // ¹¦ÂÊÒòÊı½Ç¶È
int16 gPhiRtDisp;     // ÏÔÊ¾Öµ
Uint16 pmsmRotorPos;  // Í¬²½»ú×ª×ÓÎ»ÖÃ(ĞÔÄÜÊµÊ±¸üĞÂ)
Uint16 frqFdb;        // µç»ú·´À¡×ªËÙ
Uint16 frqFdbDisp;    // µç»ú·´À¡ËÙ¶ÈÏÔÊ¾Öµ
int32  frqFdbTmp;
Uint16 frqFdbFlag;
Uint16 torqueCurrent;
Uint16 enCoderPosition;
Uint16 ABZPos;
Uint16 ZCount;        // ZĞÅºÅ¼ÆÊı
Uint16 antiVibrateCoeff;

#if DEBUG_F_INTERFACE
void copyDataFunc2Motor2ms(void);
// ĞÔÄÜ´«µİ¸ø¹¦ÄÜµÄÊı¾İ
Uint16 currentOc;
Uint16 currentPu;
Uint16 spdLoopOut;                  // ĞÔÄÜ²¿·ÖµÄËÙ¶È»·Êä³ö


LOCALF Uint32 softOCTicker;         // Èí¼ş¹ıÁ÷¼ÆÊ±
extern Uint16 softOcDoFlag;

// ¹¦ÄÜ==>ĞÔÄÜµÄ²ÎÊı£¬0.5ms
void copyDataFunc2Motor05ms(void)
{
    gSendToMotor05MsDataBuff[0] = dspMainCmd.all;                            // 0   Ö÷ÃüÁî×Ö
    gSendToMotor05MsDataBuff[1] = dspMainCmd1.all;                           // 1   Ö÷ÃüÁî×Ö1
    gSendToMotor05MsDataBuff[2] = tuneCmd;                                   // 2   µ÷Ğ³Ñ¡Ôñ
    gSendToMotor05MsDataBuff[3] = motorFc.motorPara.elem.motorType;          // 3   µç»úÀàĞÍ
    gSendToMotor05MsDataBuff[4] = frq2Core;                                  // 4   ÊµÊ±ËÙ¶È¸ø¶¨
    gSendToMotor05MsDataBuff[5] = vfSeparateVol;                             // 5   VF·ÖÀëÊ±µÄÊä³öµçÑ¹
    gSendToMotor05MsDataBuff[6] = motorFc.motorPara.elem.ratingVoltage;      // 6   µç»ú¶î¶¨µçÑ¹
    gSendToMotor05MsDataBuff[7] = motorFc.motorPara.elem.ratingCurrent;      // 7   µç»ú¶î¶¨µçÁ÷
    gSendToMotor05MsDataBuff[8] = motorFc.motorPara.elem.ratingFrq;          // 8   µç»ú¶î¶¨ÆµÂÊ
    gSendToMotor05MsDataBuff[9] = funcCode.code.vfCurve;                     // 9   VFÇúÏßÑ¡Ôñ
    gSendToMotor05MsDataBuff[10] = funcCode.code.ovGain;                      // 10  F9-03 ¹ıÑ¹Ê§ËÙÔöÒæ
    gSendToMotor05MsDataBuff[11] = funcCode.code.ovPoint;                     // 11  F9-04 ¹ıÑ¹Ê§ËÙ±£»¤µçÑ¹    
    gSendToMotor05MsDataBuff[12] = funcCode.code.ocGain;                      // 12  F9-05 ¹ıÁ÷Ê§ËÙÔöÒæ
    gSendToMotor05MsDataBuff[13] = funcCode.code.ocPoint;                     // 13  F9-06 ¹ıÁ÷Ê§ËÙ±£»¤µçÁ÷
}

// ¹¦ÄÜ==>ĞÔÄÜµÄ²ÎÊı£¬2ms
// µ÷ÊÔ²ÎÊı£¬²»·ÅÔÚÕâÀï£¬Ö±½Ó´«µİ¸øgSendToMotor2MsDataBuff[]µÄ¶ÔÓ¦Î»ÖÃ
void copyDataFunc2Motor2ms(void)
{
#if  DEBUG_F_INTERFACE
    gSendToMotor2MsDataBuff[0] = dspSubCmd.all;                             // 0    ¸¨ÖúÃüÁî×Ö
    gSendToMotor2MsDataBuff[1] = frqCurAim2Core;                            // 1    Ä¿±êÆµÂÊ
    gSendToMotor2MsDataBuff[2] = funcCode.code.maxFrq;                      // 2    ×î´óÆµÂÊ
    gSendToMotor2MsDataBuff[3] = funcCode.code.carrierFrq;                  // 3    ÔØ²¨ÆµÂÊ
    gSendToMotor2MsDataBuff[4] = motorFc.motorPara.elem.ratingPower;        // 4    µç»ú¶î¶¨¹¦ÂÊ

#if 1    
    memcpy(&gSendToMotor2MsDataBuff[5], &motorFc.motorPara.elem.ratingSpeed, 6);
#else 
    gSendToMotor2MsDataBuff[5] = motorFc.motorPara.elem.ratingSpeed;        // 5    µç»ú¶î¶¨×ªËÙ
    gSendToMotor2MsDataBuff[6] = motorFc.motorPara.elem.statorResistance;   // 6    ¶¨×Óµç×è
    gSendToMotor2MsDataBuff[7] = motorFc.motorPara.elem.rotorResistance;    // 7    ×ª×Óµç×è
    gSendToMotor2MsDataBuff[8] = motorFc.motorPara.elem.leakInductance;     // 8    Â©¸Ğ
    gSendToMotor2MsDataBuff[9] = motorFc.motorPara.elem.mutualInductance;   // 9   »¥¸Ğ
    gSendToMotor2MsDataBuff[10] = motorFc.motorPara.elem.zeroLoadCurrent ;  // 10   ¿ÕÔØµçÁ÷
#endif

#if 1    
    memcpy(&gSendToMotor2MsDataBuff[11], &motorFc.motorPara.elem.pmsmRs, 3);
#else 
    gSendToMotor2MsDataBuff[11] = motorFc.motorPara.elem.pmsmRs;             // 11   Í¬²½»ú¶¨×Óµç×è
    gSendToMotor2MsDataBuff[12] = motorFc.motorPara.elem.pmsmLd;             // 12   Í¬²½»úDÖáµç¸Ğ
    gSendToMotor2MsDataBuff[13] = motorFc.motorPara.elem.pmsmLq;             // 13   Í¬²½»úQÖáµç¸Ğ
#endif

    gSendToMotor2MsDataBuff[14] = motorFc.vcPara.vcSpdLoopKp1;               // 14   ËÙ¶È»·Kp1
    gSendToMotor2MsDataBuff[15] = motorFc.vcPara.vcSpdLoopTi1;               // 15   ËÙ¶È»·Ti1
    gSendToMotor2MsDataBuff[16] = motorFc.vcPara.vcSpdLoopKp2;               // 16   ËÙ¶È»·Kp2
    gSendToMotor2MsDataBuff[17] = motorFc.vcPara.vcSpdLoopTi2;               // 17   ËÙ¶È»·Ti2
    gSendToMotor2MsDataBuff[18] = motorFc.vcPara.vcSpdLoopChgFrq1;           // 18   ËÙ¶È»·ÇĞ»»ÆµÂÊ1
    gSendToMotor2MsDataBuff[19] = motorFc.vcPara.vcSpdLoopChgFrq2;           // 19   ËÙ¶È»·ÇĞ»»ÆµÂÊ2
    gSendToMotor2MsDataBuff[20] = motorFc.vcPara.vcSlipCompCoef;             // 20   VC×ª²î²¹³¥ÔöÒæ    
    gSendToMotor2MsDataBuff[21] = motorFc.vcPara.vcSpdLoopFilterTime;        // 21   ËÙ¶È»·µ÷½ÚÆ÷Êä³öÂË²¨Ê±¼ä
    gSendToMotor2MsDataBuff[22] = motorFc.pgPara.elem.encoderPulse;          // 22   ±àÂëÆ÷Âö³åÊı
    gSendToMotor2MsDataBuff[23] = motorFc.vcPara.vcOverMagGain;              // 23   Ê¸Á¿¿ØÖÆ¹ıÀø´ÅÔöÒæ
    gSendToMotor2MsDataBuff[24] = funcCode.code.loseLoadLevel;               // 24   µôÔØ¼ì²âË®Æ½
    gSendToMotor2MsDataBuff[25] = funcCode.code.loseLoadTime;                // 25   µôÔØ¼ì²âÊ±¼ä
    gSendToMotor2MsDataBuff[26] = upperTorque;                               // 26   ×ª¾ØÏŞ¶¨
    gSendToMotor2MsDataBuff[27] = motorFc.torqueBoost;                       // 27   ×ª¾ØÌáÉı

#if 1    
    memcpy(&gSendToMotor2MsDataBuff[28], &funcCode.code.boostCloseFrq, 9);
#else 
    gSendToMotor2MsDataBuff[28] = funcCode.code.boostCloseFrq;               // 28   VFÌáÉı½ØÖ¹ÆµÂÊ
    gSendToMotor2MsDataBuff[29] = funcCode.code.vfFrq1;                      // 29   VFÆµÂÊµã1
    gSendToMotor2MsDataBuff[30] = funcCode.code.vfVol1;                      // 30   VFµçÑ¹µã1
    gSendToMotor2MsDataBuff[31] = funcCode.code.vfFrq2;                      // 31   VFÆµÂÊµã2
    gSendToMotor2MsDataBuff[32] = funcCode.code.vfVol2;                      // 32   VFµçÑ¹µã2
    gSendToMotor2MsDataBuff[33] = funcCode.code.vfFrq3;                      // 33   VFÆµÂÊµã3
    gSendToMotor2MsDataBuff[34] = funcCode.code.vfVol3;                      // 34   VFµçÑ¹µã3
    gSendToMotor2MsDataBuff[35] = funcCode.code.slipCompCoef;                // 35   VF×ª²î²¹³¥
    gSendToMotor2MsDataBuff[36] = funcCode.code.vfOverMagGain;               // 36   VF¹ıÀø´ÅÔöÒæ
#endif
    
    gSendToMotor2MsDataBuff[37] = motorFc.antiVibrateGain;                   // 37   VFÒÖÖÆÕñµ´ÔöÒæ
    gSendToMotor2MsDataBuff[38] = funcCode.code.startBrakeCurrent;           // 38   Æô¶¯Ö±Á÷ÖÆ¶¯µçÁ÷
    gSendToMotor2MsDataBuff[39] = funcCode.code.stopBrakeCurrent;            // 39   Í£»úÖ±Á÷ÖÆ¶¯µçÁ÷
    gSendToMotor2MsDataBuff[40] = funcCode.code.brakeDutyRatio;              // 40   ÖÆ¶¯Ê¹ÓÃÂÊ
    gSendToMotor2MsDataBuff[41] = funcCode.code.overloadGain;                // 41   µç»ú¹ıÔØ±£»¤ÔöÒæ
    gSendToMotor2MsDataBuff[42] = funcCode.code.foreOverloadCoef;            // 42   µç»ú¹ıÔØÔ¤±¨¾¯ÏµÊı
    gSendToMotor2MsDataBuff[43] = funcCode.code.softPwm;                     // 43   Ëæ»úPWMÑ¡Ôñ
    gSendToMotor2MsDataBuff[44] = funcCode.code.curSampleDelayComp;          // 44   µçÁ÷¼ì²âÑÓÊ±²¹³¥
    gSendToMotor2MsDataBuff[45] = funcCode.code.uvPoint;                     // 45   Ç·Ñ¹µã 
    gSendToMotor2MsDataBuff[46] = motorFc.pgPara.elem.pgType;                // 46   ±àÂëÆ÷ÀàĞÍ
    gSendToMotor2MsDataBuff[47] = driveCoeffFrqFdb;                          // 47   ²âËÙ´«¶¯±È
    gSendToMotor2MsDataBuff[48] = funcCode.code.inverterType;                // 48   ±äÆµÆ÷»úĞÍ
    gSendToMotor2MsDataBuff[49] = funcCode.code.inverterGpType;              // 49   GP»úĞÍ
    gSendToMotor2MsDataBuff[50] = funcCode.code.tempCurve;                   // 50   ÎÂ¶ÈÇúÏßÑ¡Ôñ
    gSendToMotor2MsDataBuff[51] = funcCode.code.volJudgeCoeff;               // 51   µçÑ¹Ğ£ÕıÏµÊı
    gSendToMotor2MsDataBuff[52] = funcCode.code.curJudgeCoeff;               // 52   µçÁ÷Ğ£ÕıÏµÊı
    gSendToMotor2MsDataBuff[53] = funcCode.code.uvGainWarp;                  // 53   UVÁ½ÏàÔöÒæÆ«²î
    gSendToMotor2MsDataBuff[54] = funcCode.code.svcMode;                     // 54   SVCÓÅ»¯Ñ¡Ôñ 0-²»ÓÅ»¯  1-ÓÅ»¯Ä£Ê½1  2-ÓÅ»¯Ä£Ê½2
    gSendToMotor2MsDataBuff[55] = funcCode.code.deadTimeSet;                 // 55   ËÀÇøÊ±¼äµ÷Õû-1140V×¨ÓÃ
    gSendToMotor2MsDataBuff[56] = funcCode.code.speedTrackMode;              // 56   ×ªËÙ¸ú×Ù
    gSendToMotor2MsDataBuff[57] = funcCode.code.speedTrackVelocity;          // 57   ×ªËÙ¸ú×Ù
    gSendToMotor2MsDataBuff[58] = funcCode.code.pmsmRotorPos;                // 58   Í¬²½»ú×ª×ÓÎ»ÖÃ
    gSendToMotor2MsDataBuff[59] = motorFc.pgPara.elem.enCoderPole;           // 59   Ğı±ä¼«¶ÔÊı
    gSendToMotor2MsDataBuff[60] = motorFc.motorPara.elem.pmsmCoeff;          // 60   Í¬²½»ú·´µç¶¯ÊÆÏµÊı  

#if 1    
    memcpy(&gSendToMotor2MsDataBuff[61], &motorFc.vcPara.mAcrKp, 4);
#else     
    gSendToMotor2MsDataBuff[61] = motorFc.vcPara.mAcrKp;                     // 61  MÖáµçÁ÷»·Kp
    gSendToMotor2MsDataBuff[62] = motorFc.vcPara.mAcrKi;                     // 62  MÖáµçÁ÷»·Ki
    gSendToMotor2MsDataBuff[63] = motorFc.vcPara.tAcrKp;                     // 63  TÖáµçÁ÷»·Kp
    gSendToMotor2MsDataBuff[64] = motorFc.vcPara.tAcrKi;                     // 64  TÖáµçÁ÷»·Ki
#endif

    gSendToMotor2MsDataBuff[65] = motorFc.pgPara.elem.enCoderDir;            // 65  Ğı±ä·½Ïò
    gSendToMotor2MsDataBuff[66] = motorFc.pgPara.elem.enCoderAngle;          // 66  ±àÂëÆ÷°²×°½Ç
    gSendToMotor2MsDataBuff[67] = funcCode.code.pwmMode;                     // 67 DPWMÇĞ»»ÉÏÏŞÆµÂÊ
    gSendToMotor2MsDataBuff[68] = motorFc.pgPara.elem.uvwSignDir;            // 68 UVWĞÅºÅ·½Ïò
    gSendToMotor2MsDataBuff[69] = motorFc.pgPara.elem.uvwSignAngle;          // 69 UVWĞÅºÅÁãµãÎ»ÖÃ½Ç

#if 1    
    memcpy(&gSendToMotor2MsDataBuff[70], &motorFc.vcPara.weakFlusMode, 5);
#else  
    gSendToMotor2MsDataBuff[71] = motorFc.vcPara.weakFlusMode;               // 70 F2-18 Í¬²½»úÈõ´ÅÄ£Ê½
    gSendToMotor2MsDataBuff[72] = motorFc.vcPara.weakFlusCoef;               // 71 F2-19 Í¬²½»úÈõ´ÅÏµÊı
    gSendToMotor2MsDataBuff[73] = motorFc.vcPara.weakFlusCurMax;             // 72 F2-20 ×î´óÈõ´ÅµçÁ÷
    gSendToMotor2MsDataBuff[74] = motorFc.vcPara.weakFlusAutoCoef;           // 73 F2-21 Èõ´Å×Ô¶¯µ÷Ğ³ÏµÊı
    gSendToMotor2MsDataBuff[75] = motorFc.vcPara.weakFlusIntegrMul;          // 74 F2-22 Èõ´Å»ı·Ö±¶Êı
#endif  

#endif
}

// ¹¦ÄÜ==>ĞÔÄÜµÄµ÷ÊÔ²ÎÊı£¬2ms
// µ÷ÊÔ²ÎÊı£¬²»·ÅÔÚÕâÀï£¬Ö±½Ó´«µİ¸øgSendToMotor2MsDataBuff[]µÄ¶ÔÓ¦Î»ÖÃ
void copyDataFunc2CF2ms(void)
{
#if 1
    memcpy(&gSendToMotor2MsDataBuff[FUNC_TO_MOTOR_2MS_DATA_NUM], &funcCode.group.cf[0], 40);
#elif 1
    gSendToMotor2MsDataBuff[98] = funcCode.group.cf[0];
    ...
    gSendToMotor2MsDataBuff[137] = funcCode.group.cf[39];
    
#endif    
}                                                  
                                                   
// µ÷ÊÔ²ÎÊı£¬²»·ÅÔÚÕâÀï£¬Ö±½Ó¶ÁgSendToFunctionDataBuff[]µÄ¶ÔÓ¦Î»ÖÃ
extern Uint16 errorInfoFromMotor;     
extern Uint16 motorCtrlTuneStatus;

void copyDataCore2Func2ms(void)
{
    dspStatus.all = gSendToFunctionDataBuff[0];                             // 0    ×´Ì¬×Ö
    motorCtrlTuneStatus = gSendToFunctionDataBuff[1];                       // 1    ²ÎÊı±æÊ¶×´Ì¬×Ö
    errorsCodeFromMotor[0] = gSendToFunctionDataBuff[2];                    // 2    ĞÔÄÜ¹ÊÕÏ´úÂë1
    errorsCodeFromMotor[1] = gSendToFunctionDataBuff[3];                    // 3    ĞÔÄÜ¹ÊÕÏ´úÂë2
    motorErrInfor[0] = gSendToFunctionDataBuff[4];                      // 4    ¹ÊÕÏÌáÊ¾ĞÅÏ¢1
    motorErrInfor[1] = gSendToFunctionDataBuff[5];                      // 5    ¹ÊÕÏÌáÊ¾ĞÅÏ¢2
    motorErrInfor[2] = gSendToFunctionDataBuff[6];                      // 6    ¹ÊÕÏÌáÊ¾ĞÅÏ¢3
    motorErrInfor[3] = gSendToFunctionDataBuff[7];                      // 7    ¹ÊÕÏÌáÊ¾ĞÅÏ¢4
    motorErrInfor[4] = gSendToFunctionDataBuff[8];                      // 8    ¹ÊÕÏÌáÊ¾ĞÅÏ¢5
    currentOc = gSendToFunctionDataBuff[9];                                 // 9    ¹ıÁ÷Ê±µÄµçÁ÷
    speedMotor = gSendToFunctionDataBuff[10];                                // 10   Êä³öÆµÂÊ
    outVoltage = gSendToFunctionDataBuff[11];                                // 11   Êä³öµçÑ¹
    funcCode.code.radiatorTemp = gSendToFunctionDataBuff[12];                // 12   É¢ÈÈÆ÷ÎÂ¶È
    currentPu = gSendToFunctionDataBuff[13];                                 // 13   µçÁ÷±êçÛÖµ£¬µ±µç»ú¶î¶¨µçÁ÷Óë±äÆµÆ÷µÄ¶î¶¨µçÁ÷Ïà²î½Ï´óÊ±£¬¿ÉÄÜÓëµç»ú¶î¶¨µçÁ÷²»Í¬.
    funcCode.code.motorSoftVersion = gSendToFunctionDataBuff[14];            // 14   DSPÈí¼ş°æ±¾ºÅ£¬·ÅÈëFF-07
    aiDeal[0].sample = gSendToFunctionDataBuff[15];                          // 15   AI1µÄ²ÉÑùÖµ£¬ÒÑ¾­ÂË²¨
    aiDeal[1].sample = gSendToFunctionDataBuff[16];                          // 16   AI2µÄ²ÉÑùÖµ£¬ÒÑ¾­ÂË²¨
    aiDeal[2].sample = gSendToFunctionDataBuff[17];                          // 17   AI3µÄ²ÉÑùÖµ£¬ÒÑ¾­ÂË²¨
    generatrixVoltage = gSendToFunctionDataBuff[18];                         // 18   Ö±Á÷Ä¸ÏßµçÑ¹
    torqueCurrent = gSendToFunctionDataBuff[19];                             // 19   ×ª¾ØµçÁ÷£¬»ùÖµÊÇ´«µİ¹ıÀ´µÄ¡°Êµ¼ÊÊ¹ÓÃµÄµçÁ÷»ùÖµ¡±
    outCurrent = gSendToFunctionDataBuff[20];                                // 20   Êä³öµçÁ÷
    pmsmRotorPos = gSendToFunctionDataBuff[21];                              // 21   Í¬²½»ú×ª×ÓÎ»ÖÃ
    //PGErrorFlag = gSendToFunctionDataBuff[22];                               // 22   ²ÎÊı±æÊ¶PG¿¨¹ÊÕÏ±êÖ¾
    motorRun = gSendToFunctionDataBuff[23];                                  // 23   µç»ú·´À¡ÆµÂÊ  
    outPower = gSendToFunctionDataBuff[24];                                  // 24   Êä³ö¹¦ÂÊ
    itDisp   = gSendToFunctionDataBuff[25];                                  // 25   Êä³ö×ª¾Ø
    enCoderPosition = gSendToFunctionDataBuff[26];                           // 26   Ğı±ä»úĞµÎ»ÖÃ
    gPhiRt = gSendToFunctionDataBuff[27];                                    // 27   ¹¦ÂÊÒòÊı½Ç
    ZCount = gSendToFunctionDataBuff[28];                                    // 28   ZĞÅºÅ¼ÆÊıÆ÷
    antiVibrateCoeff = gSendToFunctionDataBuff[29];                          // 29   VFÕñµ´ÏµÊı
}

// ĞÔÄÜµ½¹¦ÄÜ¼àÊÓ²ÎÊı
void copyDataCore2Uf2ms(void)
{
#if 1
    memcpy(&funcCode.group.uf[0], &gSendToFunctionDataBuff[MOTOR_TO_Func_2MS_DATA_NUM], 30);
#elif 1
    funcCode.group.uf[0]  = gSendToFunctionDataBuff[26];
...
    funcCode.group.uf[29] = gSendToFunctionDataBuff[55];
    
#endif    
}

void copyDataCore2Func05ms(void)
{
}


#elif 1

Uint16 currentPu;

#endif



void UpdateDataCore2Func(void);
void UpdateDataFunc2Core0p5ms(void);
void UpdateDataFunc2Core2ms(void);


#define CHECK_SPEED_DIR_NO     0   // Î´¼ì²â±àÂëÆ÷·´À¡ËÙ¶È·½Ïò
#define CHECK_SPEED_DIR_START  1   // ¿ªÊ¼±àÂëÆ÷·´À¡ËÙ¶È·½Ïò¼ìâ
#define CHECK_SPEED_DIR_END    2   // Íê³É±àÂëÆ÷·´À¡ËÙ¶È·½Ïò¼ì²â
#define CHECK_SPEED_DIR_QUITE  3   // ÍË³ö±àÂëÆ÷·´À¡ËÙ¶È·½Ïò¼ì²â
//=====================================================================
//
// ¼ì²â±àÂëÆ÷·´À¡ËÙ¶ÈÓëÊµ¼Ê×ªËÙ¶ÔÓ¦
// Í¬Ïò:±àÂëÆ÷·´À¡·½Ïò¼´Îªµç»ú·½Ïò
// ·´Ïò:±àÂëÆ÷·´À¡·½ÏòÈ¡·´ºóÎªµç»ú·½Ïò
//
//=====================================================================
void checkSpeedFdbDir(int32 frqFdb)
{   
    
    static Uint16 checkFlag = CHECK_SPEED_DIR_NO;  // ·´À¡ÓëËÙ¶È·½Ïò¼ì²â×´Ì¬
    static Uint16 speedDir;
    static Uint16 speedDirBak = 0;
    static Uint16 speedFdbDirTcnt = 0;
    
    if (checkFlag == CHECK_SPEED_DIR_QUITE)
    {
        return;
    }
    
    // Í£»úÇÒ·´À¡ËÙ¶ÈÎª0Ê±Æô¶¯·½Ïò¼ì²â±êÖ¾
    if ((!runFlag.bit.run)
        && (!frqFdb)
        && (checkFlag == CHECK_SPEED_DIR_NO)
        )
    {
        checkFlag = CHECK_SPEED_DIR_START;         // ¿ªÊ¼¼ì²â
    }

    if (runFlag.bit.run                                // ´¦ÓÚÔËĞĞÖĞ
        &&(checkFlag == CHECK_SPEED_DIR_START)         // Ö®Ç°ÎŞÅĞ¶Ï·½Ïò
        )
    {
        speedDirBak = speedDir;  // ±¸·İÇ°Ò»¸ö·½Ïò
        
        // Êµ¼ÊÔËĞĞÆµÂÊÓë·´À¡ÆµÂÊ·½ÏòÒ»ÖÂ
        if ((int64)frqTmp*((int32)frqFdb) > 0) 
        {
            speedDir = 0;  // ±àÂëÆ÷·´À¡ÓëÔËĞĞÆµÂÊÍ¬Ïò
        }
        // Êµ¼ÊÔËĞĞÆµÂÊÓë·´À¡ÆµÂÊ·½ÏòÏà·´
        else if ((int64)frqTmp*((int32)frqFdb) < 0) 
        {
            speedDir = 1;  // ±àÂëÆ÷·´À¡ÓëÔËĞĞÆµÂÊ·´Ïò
        }

        if (speedDir == speedDirBak)
        {
            speedFdbDirTcnt++;
        }
        else
        {
            speedFdbDirTcnt = 0;
        }
        
        // Íê³É±àÂëÆ÷·´À¡ËÙ¶È·½Ïò¼ì²â
        if (speedFdbDirTcnt > 2500)      // 5sÄÚ¼ì²âµÄÎªÒ»¸ö·½Ïò±êÖ¾Ê±ÅĞ¶Ï¸ÃÖµÎªÕæÊµ·½Ïò
        {
            checkFlag = CHECK_SPEED_DIR_END;
        }
    }

    if ((!runFlag.bit.run)
        &&(checkFlag == CHECK_SPEED_DIR_END)
        )
    {
        funcCode.code.speedFdbDir = speedDir;
        checkFlag = CHECK_SPEED_DIR_QUITE;    // ÍË³ö¼ì²â
    }
}

//=====================================================================
//
// ¸üĞÂĞÔÄÜ´«µİ¸ø¹¦ÄÜµÄ½»»¥Êı¾İ
//
//=====================================================================
Uint16 powerAddDec;

#define POWER_ADD_COUNT_DEC    5000
#define POWER_ADD_COUNT_INT    3600 // 100*3600*500/5000
void UpdateDataCore2Func(void)
{
    int32 tmp;
    int32 frqFdbDispTmp;
    static int32 outCurrentDispOldTmp;
    static LowPassFilter frqFdbLpf = LPF_DEFALUTS;

#if DEBUG_F_INTERFACE
    copyDataCore2Func2ms();     // ĞÔÄÜ´«µİ¸ø¹¦ÄÜ2ms
    copyDataCore2Func05ms();    // ĞÔÄÜ´«µİ¸ø¹¦ÄÜ0.5ms
   
#endif


#if F_DEBUG_RAM    //+= Ã»ÓĞĞÔÄÜ³ÌĞòÊ±ºòµÄµ÷ÊÔ
    if (ERROR_DEALING)        // ¹¦ÄÜÒÑ¾­´¦Àí£¬ĞÔÄÜ½«¹ÊÕÏÂëÇåÁã
    {
        errorCodeFromMotor = ERROR_NONE;     // errorCodeFromMotor = 0
    }

    if (dspMainCmd.bit.run && (!errorCodeFromMotor)) // ¹¦ÄÜÓĞÔËĞĞÃüÁî£¬ÇÒĞÔÄÜÃ»ÓĞ´íÎó
    {
        dspStatus.bit.run = 1;
    }
    else
    {
        dspStatus.bit.run = 1;
    }

    speedMotor = frq2Core;

    {
        static int16 k;
        
        if (++k >= (Uint16)(200/RUN_CTRL_PERIOD))  // _msÖ®ºó£¬²Å½«ÕâĞ©±êÖ¾ÖÃÎª1
        {
            dspStatus.bit.runEnable = 1;
        }
    }

    currentPu = funcCode.code.motorParaM1.elem.ratingCurrent;  // currentPu

    speedMotor = frq2Core;  // µ÷ÊÔÊ±£¬·´À¡ËÙ¶È×ÜÊÇ ¸ø¶¨ËÙ¶È

    generatrixVoltage = 12345;      // Ä¸ÏßµçÑ¹
    outCurrent = 2048;              // Êä³öµçÁ÷, Q12
    outVoltage = 1<<12;             // Êä³öµçÑ¹, Q12
//    radiatorTemp = -9;              // É¢ÈÈÆ÷ÎÂ¶È
#endif


#if DEBUG_F_MOTOR_FUNCCODE
#if !F_DEBUG_RAM
// ÏÔÊ¾£¬µ÷ÊÔÊ¹ÓÃ£¬motor2func
    copyDataCore2Uf2ms();  // ĞÔÄÜ´«µİ¸ø¹¦ÄÜ£¬ÏÔÊ¾
#endif
#endif

    // ±äÆµÆ÷Êµ¼ÊÔËĞĞÆµÂÊ
    frqRun = ((int32)(int16)speedMotor * (int32)frqPuQ15 + (1 << 14)) >> 15;

    if (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_VF)
    {
        // ¼ÆËã¹ıÑ¹¹ıÁ÷ÒÖÖÆÊ±Ê¹ÓÃµÄÆµÂÊ
    	frqVFRun = ((int32)(int16)speedMotor * (int32)frqPuQ15) >> 15;


    	if (frqVFRun < frqRun)
    	{
    		frqVFRunRemainder = ((int32)(int16)speedMotor * (int32)frqPuQ15) - (frqRun<<15);
    	}
    	else
    	{
    	    // ¼ÆËãÓàÖµ
    	    frqVFRunRemainder = ((int32)(int16)speedMotor * (int32)frqPuQ15) - (frqVFRun<<15);
    	}
    }
    else

    {
        frqVFRunRemainder = 0;
    }

    // µç»úÊµ¼Ê×ªËÙ,±àÂëÆ÷²âËÙ
    frqFdbTmp = ((int32)(int16)motorRun * (int32)frqPuQ15 + (1 << 14)) >> 15;

#define    OUT_FDB_FRQ_DISP_FILTER_TIME   30 
    frqFdbLpf.t = OUT_FDB_FRQ_DISP_FILTER_TIME;
    frqFdbLpf.in = (frqFdbTmp);
    frqFdbLpf.calc(&frqFdbLpf);
    // µç»ú·´À¡ËÙ¶È
    frqFdbDispTmp = frqFdbLpf.out;

    // µÚÒ»´ÎÔËĞĞÊ±±àÂëÆ÷·´À¡ËÙ¶È·½Ïò¼ì²â
    checkSpeedFdbDir(frqFdbDispTmp);    

    if (funcCode.code.speedFdbDir != funcCode.code.runDir)
    {
        frqFdbDispTmp = -frqFdbDispTmp;   // ·´À¡ËÙ¶ÈÈ¡·´×÷ÎªÕı³£ËÙ¶È
    }
    
    if(ABS_INT32(frqFdbDispTmp) > (Uint32)32000)
    {
        frqFdb = (frqFdbDispTmp > 0) ? (int16)32000 : (int16)-32000;
    }
    else
    {
        frqFdb = frqFdbDispTmp;
    }    

    // ¹¦ÂÊÒòÊı½Ç¶È
    gPhiRtDisp = (int32)gPhiRt*1800 >> 15;

#if DSP_2803X
    ABZPos = EQep1Regs.QPOSCNT & 0x0000FFFF;
#else
    if (motorFc.pgPara.elem.fvcPgSrc == FUNCCODE_fvcPgSrc_QEP2)
    {
        ABZPos = EQep2Regs.QPOSCNT & 0x0000FFFF;   
    }
    else
    {
        ABZPos = EQep1Regs.QPOSCNT & 0x0000FFFF;   
    }
#endif


    
#if DEBUG_F_INTERFACE
    if (speedMotor == frq2Core)  // ¹æ±ÜÔËËãÒıÆğµÄÎó²î
    {
        frqRun = frqDroop;
    }

    if(ABS_INT32(frqRun) > (Uint32)32000)
    {
        frqRunDisp = (frqRun > 0) ? (int16)32000 : (int16)-32000;
    }
    else
    {
        frqRunDisp = frqRun;
    }   
    
#if DEBUG_F_POSITION_CTRL
    pcOriginDisp = pcOrigin / 4;    // ÏÔÊ¾4±¶ÆµÖ®Ç°
#endif

// ÏÔÊ¾·´À¡ËÙ¶È£¬0.1rpm
//    funcCode.group.f2[18] = ((int32)(int16)frqFdb * ((int32)maxFrq + 2000) * 3 + (1 << 14)) >> 15;
// ÏÔÊ¾·´À¡ËÙ¶È£¬0.01Hz
//    funcCode.group.f2[18] = ((int32)(int16)frqFdb * ((int32)maxFrq + 2000) + (1 << 14)) >> 15;
//    funcCode.group.f2[18] = frqRun;

    outCurrentDispOld = outCurrentDisp;    // ±£´æÉÏÒ»ÅÄµÄÊä³öµçÁ÷£¬¹ÊÕÏ¼ÇÂ¼Ê¹ÓÃ
//  currentInLpf.t = OUT_CURRENT_FILTER_TIME;   // ÂË²¨Ê±¼äÏµÊı
    tmp = ((int32)outCurrent * currentPu) >> 8;

#if 1
    if (ABS_INT32(outCurrentDispOldTmp - tmp) > 15) // ¼õĞ¡ÏÔÊ¾ÒıÆğµÄ²¨¶¯. 15/16
    {
        outCurrentDisp = (tmp + (1 << 3)) >> 4;
        outCurrentDispOldTmp = tmp;
    }
#else    
    // Êä³öµçÁ÷ÏÔÊ¾ÂË²¨
    currentInLpf.in = (tmp + (1 << 3)) >> 4;
    currentInLpf.calc(&currentInLpf);
    outCurrentDisp = currentInLpf.out;
#endif
    currentOcDisp = ((int32)currentOc * currentPu + (1 << 11)) >> 12;

    outVoltageDisp = ((int32)outVoltage * invPara.ratingVoltage + (1 << 11)) >> 12;

    // Èí¼ş¹ıÁ÷
    softOcDoFlag = 0;
    if(runFlag.bit.run &&(funcCode.code.softOCValue) && (outCurrentDisp >= ((Uint32)funcCode.code.softOCValue*motorFc.motorPara.elem.ratingCurrent/1000)))
    {
        // Èí¼ş¹ıÁ÷¼ì²âÊ±¼ä(¹ıÁ÷µãÎªÁã²»¼ì²â)
        if(softOCTicker < ((Uint32)funcCode.code.softOCDelay*TIME_UNIT_CURRENT_CHK / CORE_FUNC_PERIOD))
        {
            softOCTicker++;
        }
        else
        {
            softOcDoFlag = 1;
            // errorOther = ERRROR_SOFT_OC;  // Èí¼ş¹ıÁ÷
        }
    }
    else
    {
        softOCTicker = 0;
    }
    
#if DEBUG_F_POWER_ADD
    // ÀÛ¼ÆºÄµçÁ¿¼ÆËã
    if (outPower > 0)   // Îª¸º·¢µç²»»ØÀ¡
    {
        powerAddDec += outPower;                 // Ğ¡Êı²¿·Ö

        if (powerAddDec > POWER_ADD_COUNT_DEC)   // ÕûÊı²¿·Ö+1
        {
            // Ğ¡Êı²¿·ÖÇå0
            powerAddDec = 0;
            // ÕûÊı²¿·ÖÔö1
            if (++funcCode.code.powerAddupInt >= POWER_ADD_COUNT_INT)
            {
                // ´ïµ½1¶Èµç
                funcCode.code.powerAddupInt = 0;
                // ÀÛ¼ÆºÄµçÁ¿Ôö1
                funcCode.code.powerAddup++;
            }
        }
    }
#endif

    vcSpdLoopKp1 = funcCode.code.vcParaM1.vcSpdLoopKp1;
    vcSpdLoopKp2 = funcCode.code.vcParaM1.vcSpdLoopKp2;
#if DEBUG_F_POSITION_CTRL
    bPcErrorOk = 0;
    bPcErrorNear = 0;
#endif

    // bit0: 0-ÔËĞĞ  1-Í£»ú
    // bit1¡¢2:  0-ºãËÙ 1-¼ÓËÙ  2-¼õËÙ 
    // bit3: 0-Õı³£ 1-µôµç
    invtStatus.bit.run = runFlag.bit.run;
    invtStatus.bit.accDecStatus = runFlag.bit.accDecStatus;
    invtStatus.bit.uv = bUv;
#endif

}


//=====================================================================
//
// ¸üĞÂ¹¦ÄÜ´«µİ¸øĞÔÄÜµÄ½»»¥Êı¾İ£¬0.5ms
//
//=====================================================================    
void UpdateDataFunc2Core0p5ms(void)
{
    int32 droopValue; // ÏÂ´¹·¶Î§
	static Uint16 frq2CoreTmp;
    
// ÏÂ´¹¿ØÖÆ
    if (funcCode.code.droopCtrl 
        && frq 
        && (generatrixVoltage < DROOP_VOLTAGE_LEVEL) 
        && (runStatus != RUN_STATUS_TUNE))             // ²»Îªµ÷Ğ³ÔËĞĞ
    {
        droopValue = ((int64)torqueCurrentAct * funcCode.code.droopCtrl) / motorFc.motorPara.elem.ratingCurrent;
        frqDroop = frq - (int64)frq * droopValue / 1000;
    }
    else
    {
        frqDroop = frq;
    }
    
    frq2CoreTmp = (frqVFRunRemainder + (frqDroop << 15)) / ((int32)frqPuQ15);
#if 1
    if (((funcCode.code.ovGain) || (funcCode.code.ocGain))
            && (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_VF)   // VFÔËĞĞ
            && (motorFc.motorPara.elem.motorType != MOTOR_TYPE_PMSM)  // ·ÇÍ¬²½»ú
            && (!funcCode.code.droopCtrl)                             // ÏÂ´¹¿ØÖÆÎŞĞ§
            && (runStatus != RUN_STATUS_TUNE)                         // ²»´¦ÓÚµ÷Ğ³×´Ì¬
            && (runFlag.bit.accDecStatus == DEC_SPEED)
			&& (ABS_INT16((int16)frq2CoreTmp) >= ABS_INT16((int16)speedMotor))
        ) // ¹ıÑ¹Ê§ËÙ£¬¹ıÁ÷Ê§ËÙÔöÒæ¶¼Îª0
    {
        //frq2Core = (frqVFRunRemainder + (frqDroop << 15)) / ((int32)frqPuQ15); 
    }
    else
    {
        frq2Core = frq2CoreTmp;
    }
#endif

#if DEBUG_F_INTERFACE  
#if !F_DEBUG_RAM
    copyDataFunc2Motor05ms();  // ¹¦ÄÜ´«µİ¸øĞÔÄÜ£¬0.5ms  
#endif
#endif
}


//=====================================================================
//
// ¸üĞÂ¹¦ÄÜ´«µİ¸øĞÔÄÜµÄ½»»¥Êı¾İ£¬2ms
//
//=====================================================================
extern Uint32 aptpSetOrigin;
void UpdateDataFunc2Core2ms(void)
{
#if DEBUG_F_INTERFACE      
    frqCurAim2Core = ((frqCurAim << 15) + frqCurAimFrac) / ((int32)frqPuQ15); // µ±Ç°µÄÄ¿±êÆµÂÊ

    // ÃüÁî×Ö
    // ¸øĞÔÄÜ´«µİÃüÁî 0.5ms
    dspMainCmd.bit.motorCtrlMode = motorFc.motorCtrlMode;
    dspMainCmd.bit.accDecStatus = runFlag.bit.accDecStatus;     // ¸ù¾İrunFlagµÄ¼Ó¼õËÙ±êÖ¾£¬¸ø¶¨dspMainCmdµÄ¼Ó¼õËÙ±êÖ¾
    dspMainCmd.bit.torqueCtrl = funcCode.code.torqueCtrl;       // ×ª¾Ø¿ØÖÆ
    dspMainCmd.bit.spdLoopI1 = motorFc.vcPara.spdLoopI;         // »ı·Ö·ÖÀë
    // ¸¨ÖúÃüÁî×Ö(2ms)    
    dspSubCmd.bit.outPhaseLossProtect = funcCode.code.outPhaseLossProtect;  // Êä³öÈ±Ïà±£»¤
    dspSubCmd.bit.inPhaseLossProtect = funcCode.code.inPhaseLossProtect%10; // ÊäÈëÈ±Ïà±£»¤
    dspSubCmd.bit.contactorMode = funcCode.code.inPhaseLossProtect/10;      // ½Ó´¥Æ÷ÎüºÏ±£»¤
    dspSubCmd.bit.overloadMode = funcCode.code.overloadMode;                // µç»ú¹ıÔØ±£»¤
    dspSubCmd.bit.loseLoadProtectMode = funcCode.code.loseLoadProtectMode;  // Êä³öµôÔØ±£»¤Ê¹ÄÜ±êÖ¾
    dspSubCmd.bit.poffTransitoryNoStop = funcCode.code.pOffTransitoryNoStop;// Ë²Í£²»Í£
    //dspSubCmd.bit.overModulation = funcCode.code.overModulation;            // ¹ıµ÷ÖÆÊ¹ÄÜ
    //dspSubCmd.bit.fieldWeak = funcCode.code.fieldWeak;                      // Èõ´Å¿ØÖÆ
    dspSubCmd.bit.cbc = funcCode.code.cbcEnable;                            // Öğ²¨ÏŞÁ÷Ê¹ÄÜ
    //dspSubCmd.bit.narrowPulseMode = funcCode.code.narrowPulseMode;          // Õ­Âö³å¿ØÖÆÑ¡Ôñ
    //dspSubCmd.bit.currentSampleMode = funcCode.code.currentSampleMode;      // µçÁ÷¼ì²âÂË²¨£¨ÌŞ³ıÃ«´Ì£©Ñ¡Ôñ
    dspSubCmd.bit.varFcByTem = funcCode.code.varFcByTem;                    // ÔØ²¨ÆµÂÊËæÎÂ¶Èµ÷Õû£¬MD280Ò»Ö±ÓĞĞ§
    //dspSubCmd.bit.pmsmInitPosNoSame = funcCode.code.pmsmInitPosNoSame;
    //dspSubCmd.bit.pmsmZeroPosBig = funcCode.code.pmsmZeroPosBig;
    // Ö÷ÃüÁî×Ö1(0.5ms)
    dspMainCmd1.bit.pgLocation = motorFc.pgPara.elem.fvcPgSrc;            // FVCµÄPG¿¨Ñ¡Ôñ, 0-QEP1,1-QEP2(À©Õ¹)
    // ÀîÈóµ÷ÕûA0×é¹¦ÄÜÂë
    //dspMainCmd1.bit.pwmMode = funcCode.code.pwmMode;                // PWMÄ£Ê½Ñ¡Ôñ. 
    dspMainCmd1.bit.modulationMode = funcCode.code.modulationMode;  // µ÷ÖÆ·½Ê½
    dspMainCmd1.bit.deadCompMode = funcCode.code.deadCompMode;      // ËÀÇø²¹³¥Ä£Ê½Ñ¡Ôñ
    dspMainCmd1.bit.frqPoint = funcCode.code.frqPoint;              // ÆµÂÊÖ¸Áîµ¥Î»


    //jerkFf = (((int64)jerk * 32 * (int64)funcCode.code.servoKp2) << 15) / ((int32)maxFrq + 2000) / 100;

    driveCoeffFrqFdb = 1000;
#if !F_DEBUG_RAM

    // ¹ıÑ¹ÒÖÖÆµã¼ÆËã
    //GetOverUdcPoint();
    copyDataFunc2Motor2ms();  // ¹¦ÄÜ´«µİ¸øĞÔÄÜ£¬2ms  
#else    
#endif

#if DEBUG_F_MOTOR_FUNCCODE
#if !F_DEBUG_RAM
// µ÷ÊÔÊ¹ÓÃ¹¦ÄÜÂë£¬func2motor 
    copyDataFunc2CF2ms();  // ¹¦ÄÜ´«µİ¸øĞÔÄÜ
#endif
#endif
#endif
}







