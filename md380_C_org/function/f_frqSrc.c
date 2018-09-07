//======================================================================
//
// ÆµÂÊÔ´´¦Àí¡£ 
//
// Time-stamp: <2012-10-24 15:06:59  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_frqSrc.h"
#include "f_main.h"
#include "f_runSrc.h"
#include "f_io.h"
#include "f_menu.h"
#include "f_ui.h"
#include "f_p2p.h"
#include "f_comm.h"

int32 upDownFrq;
Uint16 upDownFrqInit;
int32 upDownFrqTmp;

int32 uPDownFrqMax;
int32 uPDownFrqMin;
int32 uPDownFrqLower1;
int32 uPDownFrqLower2;

#if F_DEBUG_RAM    // ½öµ÷ÊÔ¹¦ÄÜ£¬ÔÚCCSµÄbuild optionÖÐ¶¨ÒåµÄºê

#define DEBUG_F_MULTI_SET_PLC       0   // ¼òÒ×PLC£¬¶à¶ÎËÙ
#define DEBUG_F_JUMP_FRQ            0   // ÌøÔ¾ÆµÂÊ
#define DEBUG_F_FRQ_SRC_COMPOSE     0   // ÆµÂÊÔ´µþ¼Ó

#elif 1

#define DEBUG_F_MULTI_SET_PLC       1
#define DEBUG_F_JUMP_FRQ            1
#define DEBUG_F_FRQ_SRC_COMPOSE     1

#endif


union FRQ_FLAG frqFlag;


#define PLC_END_LOOP_TIME_MAX   250 // Íê³ÉÒ»¸öPLCÑ­»·£¬Êä³öÒ»¸ö_msµÄÂö³åÐÅºÅ
Uint16 plcStep;         // PLCµÄ½×¶Î£¬[0, PLC_STEP_MAX]
Uint32 plcTime;         // µ±Ç°PLC½×¶ÎµÄÔËÐÐÊ±¼ä£¬Ä¿Ç°µ¥Î»Îª10ms
Uint16 bPlcEndOneLoop;  // plcÍê³ÉÒ»¸öÑ­»·µÄ±êÖ¾£¬¸Ã±êÖ¾±£³Ö(PLC_END_LOOP_TIME_MAX)ms
Uint16 plcStepRemOld;   // PLC¼ÇÒäµÄstep
Uint32 plcTimeRemOld;   // PLC¼ÇÒäµÄtime

int32 frq;              // Ë²Ê±Öµ, ¹¦ÄÜ´«µÝ¸øÐÔÄÜµÄÔËÐÐÆµÂÊ
int32 frqTmp;           // ¼ÆËãfrqµÄÁÙÊ±±äÁ¿
int32 frqFrac;          // Ð¡Êý²¿·Ö, Q15
int32 frqTmpFrac;       // ¼ÆËãfrqµÄÁÙÊ±±äÁ¿µÄÐ¡Êý²¿·Ö, Q15
int32 frqDroop;         // ÏÂ´¹Ö®ºóµÄËÙ¶È£¬Ë²Ê±Öµ, ¹¦ÄÜ´«µÝ¸øÐÔÄÜµÄÔËÐÐÆµÂÊ
int32 frqCurAimFrac;

int32 frqAim;           // Éè¶¨(Ä¿±ê)ÆµÂÊ
int32 frqAimTmp;        // ¼ÆËãfrqAimµÄÁÙÊ±±äÁ¿, ÏÂÒ»´Î¼ÆËã²»»áÊ¹ÓÃ¡£ÌøÔ¾ÆµÂÊÖ®ºó£¬µã¶¯ÆµÂÊÖ®Ç°¡£
int32 frqAimTmp0;       // ¼ÆËãfrqAimµÄÁÙÊ±±äÁ¿, ÏÂÒ»´Î¼ÆËã»áÊ¹ÓÃ¡£ÆµÂÊÔ´Éè¶¨Ö®ºó£¬ÌøÔ¾ÆµÂÊÖ®Ç°¡£

int32 frqCurAim;        // µ±Ç°Ä¿±êÆµÂÊ£¬×¢ÒâÔÚÃ¿´Îµ÷ÓÃAccDecFrqCalc()Ö®Ç°Òª¸üÐÂfrqCurAim

Uint16 timeBench;       // ¼Ó¼õËÙÊ±¼ä»ù×¼

int32 frqDigitalTmp;    // Êý×ÖÉè¶¨ÆµÂÊÁÙÊ±Öµ


struct FRQ_XY frqXy;       

LOCALF int32 frqDigitalPlc;
LOCALF Uint16 presetFrqOld;    // Ô¤ÖÃÆµÂÊµÄoldÖµ

Uint16 upperFrq;        // ÉÏÏÞÆµÂÊ
Uint16 lowerFrq;        // ÏÂÏÞÆµÂÊ
Uint16 maxFrq;          // ×î´óÆµÂÊ
Uint16 benchFrq;        // »ù×¼ÆµÂÊ
int32 frqAiPu;          // AI,pulseµÄ100%¶ÔÓ¦ÆµÂÊ

Uint16 frqPuQ15;        // ÆµÂÊ×ª»»Ê±£¬Q15µÄ1PUµÄÆµÂÊ£¬¼´maxFrq+20Hz

Uint16 frqCalcSrc;
Uint16 frqCalcSrcOld;
Uint16 plcStepOld;          // Íê³ÉÒ»¸öPLCÑ­»·Ê¹ÓÃ
Uint16 bStopPlc;            // PLCµÄËùÓÐ¶ÎÊ±¼äÈ«Îª0£¬»òÕßµ¥´ÎÔËÐÐÍ£»ú

Uint16 bFrqDigital;         // Êý×ÖÉè¶¨ÆµÂÊ±êÖ¾£¬²»°üÀ¨UP/DOWNÍê³ÉÖ®ºóÒ»¶ÎÊ±¼äµÄÏÔÊ¾´¦ÀíÊ±¼ä
Uint16 frqKeyUpDownDelta;   // Ê¹ÓÃÃæ°åUP/DOWNÔö¼õÆµÂÊµÄdelata

LOCALF Uint32 reverseTicker;// Õý·´×ªÊ±¼ä¼ÆÊ±

Uint16 bAntiReverseRun;     // ´¦ÓÚ½ûÖ¹·´×ªÖÐ¡£µ±¹¦ÄÜÂëÉèÖÃÎª½ûÖ¹·´×ª£¬ÇÒ¸ºÆµÂÊ»òÕß·´×ªÔËÐÐÃüÁî£¬²ÅÎª1.

LOCALF JUMP_FRQ jumpFrq1;   // ÌøÔ¾ÆµÂÊ
LOCALF JUMP_FRQ jumpFrq2;   // ÌøÔ¾ÆµÂÊ2

// ÔËÐÐÄ£Ê½
Uint16 runMode; // ÔËÐÐÄ£Ê½
int32 frqAimOld4Dir;

// ÃüÁîÀ¦°óÆµÂÊÔ´
Uint16 runCmdBind;

LOCALD int32 FrqAimUpDownDeal(void);
LOCALD int32 FrqSrcOperate(void);
LOCALD int32 FrqXyCalc(Uint16 src);
LOCALD int32 FrqPlcSetDeal(void);
LOCALD void JumpFrqDeal(void);
LOCALD void JumpFrqEndCalc(JUMP_FRQ *p);
LOCALD void UpdateLimitFrq(void);
LOCALD void UpdateRunMode(void);


int32 GetFrqX(void);
int32 GetFrqY(void);
int32 GetFrqComp(void);
int32 GetFrq(void);

Uint16 frqPlcType;
//=====================================================================
// 
// ¸ù¾ÝÆµÂÊÔ´(280FµÄ¹¦ÄÜÂëÊÇF0-01)£¬¼ÆËãÉè¶¨ÆµÂÊfrqAimTmp0
//
// ÓÉÓÚÔËÐÐ·½Ïò¡¢µã¶¯ÃüÁîÊÇÔÚÃüÁîÔ´´¦ÀíµÄ£¬
// ËùÒÔÔËÐÐ·½Ïò¡¢µã¶¯ÒÔ¼°Ö®ºóµÄ·À·´×ªÑ¡Ôñ¡¢ÔËÐÐ·½ÏòÏà·´Ò²·ÅÔÚÃüÁîÔ´ÖÐ´¦Àí¡£
//
//=====================================================================
void FrqSrcDeal(void)
{
    static int32 frqAimTmpBak;

// ÉÏÏÂÏÞÆµÂÊ¼ÆËã
    UpdateLimitFrq();

// ¸üÐÂÔËÐÐÄ£Ê½
    UpdateRunMode();

    runFlag.bit.plc = 0;
    runFlag.bit.pid = 0;
    runFlag.bit.torque = 0;

    frqPlcType = 0;
    
    if (RUN_MODE_TORQUE_CTRL == runMode)
    {
        runFlag.bit.torque = 1;
    }

    frqCalcSrc = funcCode.code.frqCalcSrc;
    
    if (frqCalcSrcOld != frqCalcSrc)    // frqCalcSrc¸Ä±äÁË
    {
        frqCurAimFrac = 0;  //-= Ð¡Êýµã¸Ä±ä
    }

    if (presetFrqOld != funcCode.code.presetFrq)   // ÈôÔ¤ÖÃÆµÂÊ¸Ä±äÁË(F0-03,»Ö¸´³ö³§²ÎÊý,Í¨Ñ¶ÐÞ¸ÄµÈ)£¬¸üÐÂÉè¶¨ÆµÂÊ
    {
        presetFrqOld = funcCode.code.presetFrq;
    }

// ÆµÂÊÔ´¼ÆËã
    if ((!runFlag.bit.jog)          // µã¶¯
// ¹ÊÕÏÊ±¸üÐÂÆµÂÊÔ´
//        && (!errorCode)             // ¹ÊÕÏÂë
        && (RUN_MODE_TORQUE_CTRL != runMode)    // Ã»ÓÐ×ª¾Ø¿ØÖÆ
        )
    {
        frqAimTmp0 = FrqSrcOperate();
    }
    else
    {
        accDecFrqPrcFlag = ACC_DEC_FRQ_NONE;      // ¼üÅÌUP/DOWNÐÞ¸ÄÆµÂÊ±êÖ¾£¬ÇåÁã
    }

    // PLC×´Ì¬¸´Î»
    if (diFunc.f1.bit.resetPLC)
    {
        plcStep = 0;
        plcTime = 0;
    }

    frqCalcSrcOld = frqCalcSrc;     // ¸üÐÂfrqCalcSrcOld
    plcStepOld = plcStep;   // ¸üÐÂplcStepOld
// ÅÐ¶ÏÊÇ·ñ³¬¹ýÉÏÏÞÆµÂÊ
    if (frqAimTmp0 > upperFrq)
    {
        frqAimTmp0 = upperFrq;
    }
    else if (frqAimTmp0 < -(int32)upperFrq)
    {
        frqAimTmp0 = -(int32)upperFrq;
    }

#if 0
// ×ª¾Ø¿ØÖÆÊ±ÅÐ¶ÏÊÇ·ñ³¬¹ý×ª¾Ø¿ØÖÆ×î´óÆµÂÊ
    if (RUN_MODE_TORQUE_CTRL == runMode)    // ·Ç×ª¾Ø¿ØÖÆ
    {
        // ÕýÏò×î´óÆµÂÊ
        if (frqAimTmp0 >= 0)
        {
            if (frqAimTmp0 > funcCode.code.torqueCtrlFwdMaxFrq)
            {
                frqAimTmp0 = funcCode.code.torqueCtrlFwdMaxFrq;
            }
        }
        // ·´Ïò×î´óÆµÂÊ
        else
        {
            if (frqAimTmp0 < -(int32)funcCode.code.torqueCtrlRevMaxFrq)
            {
                (frqAimTmp0 = -(int32)funcCode.code.torqueCtrlRevMaxFrq);
            }
        }
    }
    
#endif

     // ÆµÂÊÉè¶¨ÆðÐ§¶Ë×Ó
    if (diSelectFunc.f2.bit.frqOk && (!diFunc.f2.bit.frqOk))
    {    
        frqAimTmp0 = frqAimTmpBak;
    }
    else
    {
        frqAimTmpBak = frqAimTmp0;
    }
        
// ¸³Öµ¸øfrqAimTmp
    frqAimTmp = frqAimTmp0;

// Éè¶¨ÆµÂÊÏÔÊ¾Öµ
    frqAimDisp = ABS_INT32(frqAimTmp);   // ÌøÔ¾ÆµÂÊÖ®Ç°£¬µã¶¯Ö®ºóµÄÖµ

// ÌøÔ¾ÆµÂÊ´¦Àí£¬µã¶¯ÆµÂÊ²»ÊÜÌøÔ¾ÆµÂÊÓ°Ïì¡£
// ÌøÔ¾ÆµÂÊ¼ÆËãÖ®Ç°£¬¾ÍÓëÆô¶¯ÆµÂÊ±È½Ï¡£
// ·ñÔò£¬ÈôÓÉÓÚÌøÔ¾ÆµÂÊµÄlowµÍÓÚÆô¶¯ÆµÂÊ£¬²»»áÆô¶¯¡£
// ÌøÔ¾ÆµÂÊ¼ÆËãÖ®ºó£¬ÔÙºÍÏÂÏÞÆµÂÊ±È½Ï£¬ÊÇ·ñÆô¶¯¡£
// ·ñÔò£¬ÈôÉè¶¨ÆµÂÊ¸ßÓÚÏÂÏÞÆµÂÊ£¬»áÆô¶¯£¬ÓÖÓÉÓÚÌøÔ¾ÆµÂÊµÄlowµÍÓÚÏÂÏÞÆµÂÊ£¬»áÁ¢¼´½øÈëÍ£»ú¹ý³Ì¡£
    JumpFrqDeal();
}


//=====================================================================
// 
// 1. ¸ù¾ÝÔËÐÐ·½Ïò,ÔËÐÐ·½Ê½(µã¶¯»¹ÊÇÆÕÍ¨ÔËÐÐ)£¬ÌøÔ¾ÆµÂÊ
// ¼ÆËãÉè¶¨ÆµÂÊ(Ä¿±êÆµÂÊfrqAim)
// 2. ¸üÐÔÉè¶¨ÆµÂÊºÍÔËÐÐÆµÂÊµÄ·½Ïò£¬ÅÐ¶Ïµ±Ç°ÊÇ·ñÕýÔÚ·´Ïò(ÊÇ·ñÔÚÕý·´×ªËÀÇøÄÚ)¡£
//
// ÓÉÓÚÔËÐÐ·½Ïò¡¢µã¶¯ÃüÁîÊÇÔÚÃüÁîÔ´´¦ÀíµÄ£¬
// ËùÒÔ¸Ãº¯ÊýÓ¦¸ÃÔÚÃüÁîÔ´ÖÐµ÷ÓÃ¡£
// 
//=====================================================================
void UpdateFrqAim(void)
{
    int32 frq4Dir;           // Ë²Ê±ÆµÂÊ, ÅÐ¶Ï·½ÏòÊ¹ÓÃ

// ¸³Öµ¸øfrqAimTmp
    //frqAimTmp = frqAimTmp0;  // FrqSrcDealÎª2ms¶øUpdateFrqAimÎª0.5ºÁÃë,¿ÉÄÜµ¼ÖÂÒÔÏÂ²Ù×÷È¡·´¶à´ÎÖ´ÐÐ
    //JumpFrqDeal();
    
// ÔËÐÐ·½Ïò
    if (FORWARD_DIR != runCmd.bit.dir)
    {
        frqAimTmp = -frqAimTmp;
    }

// µã¶¯¡£µã¶¯·½ÏòÓërunCmd.bit.dirÎÞ¹Ø¡£
    if (runFlag.bit.jog)            // ÕæÕýÓÐÐ§µÄµã¶¯ÃüÁî
    {                               // µã¶¯ÃüÁîÈ¡ÏûÖ®ºóÒ²¿ÉÄÜ½øÈë¸Ã¶Î´úÂë
        if (RUN_CMD_FWD_JOG == runCmd.bit.jog)  // µã¶¯·½Ïò£¬¶ÀÁ¢ÓÚÆÕÍ¨ÔËÐÐ·½Ïò
        {
            frqAimTmp = funcCode.code.jogFrq;
        }
        else if (RUN_CMD_REV_JOG == runCmd.bit.jog) //
        {
            frqAimTmp = -(int16)funcCode.code.jogFrq;
        }

        frqAimDisp = funcCode.code.jogFrq; // µã¶¯Ê±£¬Éè¶¨ÆµÂÊÏÔÊ¾Îªµã¶¯ÆµÂÊ
    }

#if DEBUG_F_PLC_CTRL
    frqAimPLCDisp = (int16)(frqAimTmp*10000/maxFrq);
#endif
    
// µã¶¯ÆµÂÊ²»ÄÜ³¬¹ýÉÏÏÞÆµÂÊ
// ÅÐ¶ÏÊÇ·ñ³¬¹ýÉÏÏÞÆµÂÊ

    frqFlag.bit.frqSetLimit = 0;
    if (frqAimTmp >= upperFrq)
    {
        frqAimTmp = upperFrq;
        frqFlag.bit.frqSetLimit = 1;
    }
    else if (frqAimTmp <= -(int32)upperFrq)
    {
        frqAimTmp = -(int32)upperFrq;
        frqFlag.bit.frqSetLimit = 1;
    }

// ¸³Öµ¸øfrqAim
    frqAim = frqAimTmp;

// ÅÐ¶ÏÉè¶¨ÆµÂÊµÄ·½Ïò
    if (frqAim > 0)
    {
        runFlag.bit.dir = FORWARD_DIR;
    }
    else if (frqAim < 0)
    {
        runFlag.bit.dir = REVERSE_DIR;
    }
    else
    {
        runFlag.bit.dir = runCmd.bit.dir;
    }

// ·´×ª½ûÖ¹
    bAntiReverseRun = 0;
    if (funcCode.code.antiReverseRun)
    {
        if (frqAimTmp < 0)  // ·´×ª½ûÖ¹£¬Éè¶¨ÆµÂÊÎª¸º£¬Éè¶¨ÆµÂÊÏÔÊ¾Îª0¡£
        {
            frqAimDisp = 0;
            frqFlag.bit.frqSetLimit = 1;
        }
        
        if ((frqAimTmp < 0) || (frqAim < 0))
        {
            frqAim = 0;      // ·´×ª½ûÖ¹£¬ÊäÈë¸ºÆµÂÊÊ±£¬ÒÔ0ÆµÔËÐÐ¡£
            frqFlag.bit.frqSetLimit = 1;
            bAntiReverseRun = 1;
        }
    }
    
// ÔËÐÐ·½ÏòÏà·´
    frq4Dir = frq;
    //frq4Dir = frqRun;  // Õý·´×ªËÀÇøÊ¹ÓÃ·´À¡µÄÆµÂÊ
    runFlag.bit.dirFinal = runFlag.bit.dir;
    if (FUNCCODE_runDir_REVERSE == funcCode.code.runDir)
    {
        frqAim = -frqAim;
        frq4Dir = -frq4Dir;

        runFlag.bit.dirFinal = ~runFlag.bit.dirFinal;
    }

// ÅÐ¶ÏÔËÐÐÆµÂÊµÄ·½Ïò
    if (frq4Dir > 0)
    {
        runFlag.bit.curDir = FORWARD_DIR;
    }
    else if (frq4Dir < 0)
    {
        runFlag.bit.curDir = REVERSE_DIR;
    }
    else    // Õý·´×ªËÀÇø
    {
        if (runFlag.bit.run
            && ((int64)frqAimOld4Dir * frqAim < 0)    // ¸ºÊýÓë0Ö®¼ä£¬²»ÄÜ½øÈë
            )
        {
            if (++reverseTicker >= (Uint32)funcCode.code.zeroSpeedDeadTime
                * (Uint16)(TIME_UNIT_ZERO_SPEED_DEAD / RUN_CTRL_PERIOD))
            {
                runFlag.bit.curDir = runFlag.bit.dir;
                
                //if (frqAim)
                    frqAimOld4Dir = frqAim;
            }
        }
        else
        {
            runFlag.bit.curDir = runFlag.bit.dir;

            //if (frqAim)      // Éè¶¨ÆµÂÊÎª0Ê±£¬²»¸üÐÂfrqAimOld4Dir
            {
                frqAimOld4Dir = frqAim;
            }
        }
    }

// ÅÐ¶ÏÊÇ·ñÕýÔÚ·´Ïò
    if ((runFlag.bit.curDir != runFlag.bit.dir) // ÕýÔÚ·´Ïò
//        && (funcCode.code.zeroSpeedDeadTime)    // Ã»ÓÐ ËÀÇøÊ±¼ä£¬frqCurAimÖ±½Ó¸³ÖµÎªÉè¶¨ÆµÂÊ£¬²»ÔÚÖÐ¼ä¸³ÖµÎª0.¿¼ÂÇsÇúÏß
        )
    {
        runFlag.bit.dirReversing = 1;
    }
    else        // Ã»ÓÐ·´Ïò
    {
        runFlag.bit.dirReversing = 0;
        reverseTicker = 0;      // tickerÇåÁã
    }
}


//=====================================================================
//
// Êý×ÖÉè¶¨ÆµÂÊ.
// ÆµÂÊÔ´ÎªÊý×ÖÉè¶¨£¬¼´UP/DOWNµ÷½Ú£¬°üÀ¨¼üÅÌºÍDI
//
// ±¸×¢£º
//      PLCºÍ¶à¶ÎÖ¸ÁîÒ²»áµ÷ÓÃ±¾º¯Êý
//
//=====================================================================   
LOCALF int32 FrqAimUpDownDeal(void)
{
    int32 frqDeltaDi;                      // Êý×ÖÉè¶¨UP/DOWNÆµÂÊµÄÔöÁ¿
    Uint16 up = diFunc.f1.bit.up;
    Uint16 down = diFunc.f1.bit.down;
    int32 delta = 0;
    static Uint16 bFrqDigitalOld;
    static int16 frqDeltaDiRemainder;      // Êý×ÖÉè¶¨UP/DOWNÆµÂÊµÄÔöÁ¿ÓàÖµ
    //frqFlag.bit.upDown = 1;
    
// µ±DIµÄUP¶Ë×ÓÎÞÐ§¡¢DIµÄDOWN¶Ë×ÓÎÞÐ§¡¢
// 0¼¶²Ëµ¥ÏÂ¼üÅÌÃ»ÓÐµ¥¶À°´ÏÂUP¡¢0¼¶²Ëµ¥ÏÂ¼üÅÌÃ»ÓÐµ¥¶À°´ÏÂDOWN£¬²ÅÈÏÎªÃ»ÓÐ½øÐÐÊý×ÖÉè¶¨ÆµÂÊ
    if ((up || down || accDecFrqPrcFlag)
        && (MENU0_DISP_STATUS_RUN_STOP == menu0DispStatus)  // ÔÚÔËÐÐ/Í£»úÏÔÊ¾Ê±²Åup/down¡£¼´£¬ÔÚ¹ÊÕÏ/µ÷Ð³ÏÔÊ¾Ê±²»¿Éup/down
        && (!runFlag.bit.dirReversing)  // Î´´¦ÓÚÕý·´×ªÇÐ»»ÖÐ
        && ((diSelectFunc.f2.bit.frqOk&&diFunc.f2.bit.frqOk) || (!diSelectFunc.f2.bit.frqOk))    // ÆµÂÊÉè¶¨ÆðÐ§¶Ë×Ó¹¦ÄÜ±»Ñ¡ÔñÇÒ¶Ë×ÓÓÐÐ§ »ò ÆµÂÊÉè¶¨¶Ë×ÓÎ´Ñ¡Ôñ
        && ((ERROR_LEVEL_RUN != errorAttribute.bit.level) 
           || ((ERROR_LEVEL_RUN == errorAttribute.bit.level) && (funcCode.code.errorRunFrqSrc == ERR_RUN_FRQ_AIM)))
        )
    {
        bFrqDigital = 1;
    }
    else
    {
        bFrqDigital = 0;
    }

    // ÔËÐÐ×´Ì¬ÏÂ£¬UP/DOWN½øÐÐÊý×ÖÉè¶¨ÆµÂÊ£¬´Óµ±Ç°(µÄË²Ê±)ÆµÂÊ¿ªÊ¼ÐÞ¸Ä¡£
    if (bFrqDigitalOld != bFrqDigital)      // Êý×ÖÉè¶¨ÆµÂÊ±êÖ¾¸Ä±äÁË
    {
        if (bFrqDigital
            && runFlag.bit.common           // Ö®Ç°Ã»ÓÐÊý×ÖÉè¶¨ÆµÂÊ£¬ÇÒµ±Ç°µç»úÔËÐÐ
            && (SWING_NONE == swingStatus)  // Ã»ÓÐ½øÈë°ÚÆµ
            )
        {
            //frqDigitalTmp = ABS_INT32(frq); // Éè¶¨ÆµÂÊ´Óµ±Ç°ÆµÂÊµÄ¾ø¶ÔÖµ¿ªÊ¼ÐÞ¸Ä
            if(funcCode.code.updnBenchmark //|| 
               // ((frqFlag.bit.comp) && 
               // (funcCode.code.frqYSrc >= FUNCCODE_frqXySrc_AI1))
              )
            {
                // ÒÔÉè¶¨ÆµÂÊÎª»ù×¼
            }
            else
            {
                // ´¦ÓÚÏÂÏÞÆµÂÊÏÞÖÆÖÐ
    			if ((ABS_INT32(frqAim) < lowerFrq) && (ABS_INT32(frq) <= lowerFrq))
    			{
                    if (runFlag.bit.curDir^runDirPanelOld)
                    {
                        upDownFrq = uPDownFrqLower2;
                    }
                    else
                    {
                        upDownFrq = uPDownFrqLower1;
                    }
    			}
    			else
                {
        			// ·ÇÊý×Ö+Êý×Ö
                    if ((frqFlag.bit.comp)
						&& ((((frqXy.x + frqXy.z) < 0)
                        	&& (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1))
						 || (((frqXy.y + frqXy.z + funcCode.code.presetFrq) < 0)
                        	&& (funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1))
							)
                        )
        			{
    					upDownFrq += ABS_INT32(frqAim) - ABS_INT32(frq);
    				}
    				// X(Êý×Ö)
    		        // Y(Êý×Ö)
    		        // X(Êý×Ö) + Y(Êý×Ö)£¬Ò²ÊÇ·Çµþ¼Ó
    		        // X(Êý×Ö) + Y(·ÇÊý×Ö)
                    else
    				{
    					upDownFrq += ABS_INT32(frq) - ABS_INT32(frqAim);
    				}
                }
			}
        }

        if (!bFrqDigital)
        {
            bFrqDigitalDone4WaitDelay = 1;  // UP/DOWN½áÊø±êÖ¾
        }

        frqDeltaDiRemainder = 0;  // Êý×ÖÉè¶¨ÆµÂÊ±êÖ¾¸Ä±äÊ±£¬ÓàÖµÇåÁã
    }

    if (bFrqDigital)    // ÕýÔÚUP/DOWNÉè¶¨ÆµÂÊ
    {
        upDownFrqInit = 0;
        frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_ON;
        
        if (ACC_DEC_FRQ_WAIT == accDecFrqPrcFlag)  // ¼üÅÌUP/DOWNÐÞ¸ÄÆµÂÊ
        {
            if (KEY_UP == keyFunc)      // ¼üÅÌÔö¼ÓÉè¶¨ÆµÂÊ£¬¼´0¼¶²Ëµ¥ÏÂ°´¼üUP
            {
                delta = (int32)frqKeyUpDownDelta;
            }
            else //if (KEY_DOWN == keyFunc) // ¼üÅÌ¼õÐ¡Éè¶¨ÆµÂÊ£¬¼´0¼¶²Ëµ¥ÏÂ°´¼üDOWN
            {
                delta = -(int32)frqKeyUpDownDelta;
            }

            accDecFrqPrcFlag = ACC_DEC_FRQ_DONE;      // °´¼üUP/DOWN ÒÑ¾­´¦Àí
        }
        else if ((up || down)       // DI¶Ë×ÓµÄUP/DOWNÓÐÐ§
                && (!(up && down) && (!frqFlag.bit.upDown))  // Í¬Ê±ÓÐÐ§£¬²»±ä»¯
                )
        {
            int32 diUpDownSlope = funcCode.code.diUpDownSlope;
            
            if (down)
            {
                diUpDownSlope = -diUpDownSlope;
            }
            
            frqDeltaDi = (diUpDownSlope * FRQ_SRC_PERIOD + frqDeltaDiRemainder) / (10000);
            frqDeltaDiRemainder = (diUpDownSlope * FRQ_SRC_PERIOD + frqDeltaDiRemainder) % (10000);
            
            delta = frqDeltaDi;
        }
    }

    frqFlag.bit.upDown = 1;
    upDownFrq += delta;

    if (frqFlag.bit.upDownoperationStatus == UP_DN_OPERATION_OFF)
    {
        upDownFrq = (int16)upDownFrqInit;  
    }
    
    if (upDownFrq > uPDownFrqMax)
    {
        upDownFrq = uPDownFrqMax;
    }
    else if (upDownFrq < uPDownFrqMin)
    {
        upDownFrq = uPDownFrqMin;
    }
    
#if 1
    if ((diFunc.f1.bit.clearUpDownFrq)           // DI¶Ë×ÓÓÐÐ§£ºUP/DOWNÉè¶¨ÇåÁã£¨¶Ë×Ó¡¢¼üÅÌ£©
//        || (frqSrcOld != frqSrc)                    // ÇÐ»»µ½Êý×ÖÆµÂÊÉè¶¨£¬»òÕßÇÐ»»µ½PLC/¶à¶ÎËÙ
        )
    {
        upDownFrq = 0;
        frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_OFF;
    }
#endif

    bFrqDigitalOld = bFrqDigital;

    // Ö÷¸¨ÆµÊ¼ÆËã£¬ÇÒYÒ²ÎªÊý×ÖÉè¶¨£¬Ô¤ÖÃÆµÂÊ(F0-08)²»Æð×÷ÓÃ
    if ((frqFlag.bit.comp) && (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1))
    {
        // X(·ÇÊý×Ö) + Y(Êý×Ö)
        return upDownFrq;
    }
    else
    {
        // X(Êý×Ö)
        // Y(Êý×Ö)
        // X(Êý×Ö) + Y(Êý×Ö)£¬Ò²ÊÇ·Çµþ¼Ó
        // X(Êý×Ö) + Y(·ÇÊý×Ö)
        return (upDownFrq + (int32)funcCode.code.presetFrq);
    }
}


//=====================================================================
//
// ÆµÂÊÔ´ÎªPLC
//
//=====================================================================
LOCALF int32 FrqPlcSetDeal(void)
{
#if DEBUG_F_MULTI_SET_PLC
    static Uint16 plcEndLoopDelayTicker;
    static Uint16 plcTimeTicker;
    static Uint16 plcFlag;
    static Uint16 plcStepEnable;
    static Uint16 bPlcEndOneLoopEnable = 1;
    Uint32 plcTimeMax = 0;
    Uint16 loop = 0;
    Uint16 plcStep1;            // PLCµÄ½×¶Î£¬[0, PLC_STEP_MAX-1]
    int32 frqPlc;

    runFlag.bit.plc = 1;
    bStopPlc = 0;               // Í£Ö¹PLC±êÖ¾

#define PLC_TIME_TICKER     5   // 5¸öPLC´¦ÀíÖÜÆÚ£¬plcTimeÔö¼Ó1£¬·ÀÖ¹plcTimeÒç³ö
    // ×ªËÙ¸ú×Ù¡¢Æô¶¯Ö±Á÷ÖÆ¶¯Ê±¼ä²»¼ÆÈëPLCÊ±¼ä£¬Æô¶¯ÆµÂÊ±£³ÖÊ±¼ä¼ÆÈëPLCÊ±¼ä¡£    
    if (((RUN_STATUS_START == runStatus) && (START_RUN_STATUS_HOLD_START_FRQ == startRunStatus))
        || (RUN_STATUS_NORMAL == runStatus)
        )
    {
        if ((!frqPlcType)  // ±¾´Î2msÑ­»·Î´½øÐÐPLCÔËÐÐ´¦Àí
            && (++plcTimeTicker >= PLC_TIME_TICKER))   // ·ÀÖ¹plcTimeMaxÒç³ö
        {
            plcTimeTicker = 0;
            plcTime++;
        }
        
        plcFlag = 1;    // ÉÏµçÖ®ºóÊ¹ÓÃ¹ýPLC
    }
    else if (!runFlag.bit.run)  // ÕýÔÚ(µ¥´ÎÔËÐÐ)Í£»ú£¬²»¸üÐÂplcStep¡£
    {
        
        // ²»µôµç¼ÇÒä¡£Í£»ú¼ÇÒäÔÝ²»¿¼ÂÇ
        if (((FUNCCODE_plcPowerOffRemMode_REM != (funcCode.code.plcPowerOffRemMode%10))
             || (plcFlag))  // ÉÏµçÖ®ºóÊ¹ÓÃ¹ýPLC
             && (FUNCCODE_plcStopRemMode_REM != (funcCode.code.plcPowerOffRemMode/10)) // ·ÇÍ£»ú¼ÇÒä
            && (!runCmd.bit.pause)
            )
        {
		    plcStep = 0;    // ´Óstep0ÖØÐÂ¿ªÊ¼ËÑË÷
		    plcTime = 0;
        }       
        else
        {
            if (plcStep >= PLC_STEP_MAX)
            {
                plcStepRemOld = 0;
                plcTimeRemOld = 0;
            }
            plcStep = plcStepRemOld;    // ´ÓplcStepRemOldÖØÐÂ¿ªÊ¼ËÑË÷
            plcTime = plcTimeRemOld;    // »Ö¸´plcTime
        }       
    }

    if (!plcStep)
    {
        bPlcEndOneLoopEnable = 1;
    }
    
    if (plcStep < PLC_STEP_MAX)
    {
        plcTimeMax = (Uint32)funcCode.code.plcAttribute[plcStep].runTime
            * ((TIME_UNIT_PLC / PLC_TIME_TICKER) / FRQ_SRC_PERIOD);
        if (FUNCCODE_plcTimeUnit_H == funcCode.code.plcTimeUnit) // PLCÔËÐÐÊ±¼äµ¥Î»
            plcTimeMax *= TIME_UNIT_SEC_PER_HOUR;
    }
#undef PLC_TIME_TICKER

    if (plcTime >= plcTimeMax)
    {
        Uint16 bEndSearch = 0;
        
        plcTime = 0;
        plcTimeTicker = 0;

        for (;;)    // ËÑË÷ÏÂÒ»¸öÔËÐÐÊ±¼ä²»Îª0µÄplcStep
        {
            if (++plcStep >= PLC_STEP_MAX)
            {
                plcStep = PLC_STEP_MAX;     // ·ÀÖ¹plcStepÒ»Ö±Ôö¼Ó
                // plcRunMode¸Ä±äÊ±£¬Ò²ÄÜ´¦Àí
                if (FUNCCODE_plcRunMode_REPEAT == funcCode.code.plcRunMode) // Ò»Ö±Ñ­»·
                {
                    plcStep = 0;
                }

                if (FUNCCODE_plcRunMode_ONCE_RUN == funcCode.code.plcRunMode) 
                {
                    plcStep = plcStepEnable;
					bEndSearch = 1;
                }

                if ((plcStepOld < PLC_STEP_MAX) && (runFlag.bit.common) && (!runFlag.bit.jog))  // Íê³ÉÒ»¸öPLCÑ­»·
                {
                    plcEndLoopDelayTicker = 0;
                    if (bPlcEndOneLoopEnable)
                    {
                        bPlcEndOneLoop = 1;
                        bPlcEndOneLoopEnable = 0;
                    }
                }
            }

            if (plcStep < PLC_STEP_MAX) // plcStepÔÚÓÐÐ§·¶Î§ÄÚ
            {
                if (funcCode.code.plcAttribute[plcStep].runTime)    // plcÔËÐÐÊ±¼ä²»Îª0£¬ÔòÍ£Ö¹ËÑË÷ÍË³öÑ­»·£¬Éè¶¨ÆµÂÊÎªµ±Ç°plcStepÆµÂÊ
                {
                    bEndSearch = 1;
                }
            }
            else
            {
                bEndSearch = 1;  // PLCÔËÐÐ²»ÊÇÒ»Ö±Ñ­»·£¬ÇÒplcStep >= PLC_STEP_MAX, ÔòÍË³öÑ­»·
            }
            
            if (++loop > PLC_STEP_MAX)      // È«²¿plc½×¶ÎµÄÔËÐÐÊ±¼ä¶¼Îª0£¬²»ÔÙÑ­»·£¬²»Æô¶¯PLC
            {
                bStopPlc = 1;
                bEndSearch = 1;
            }

            if (bEndSearch)
            {
                break;
            }
        }
    }

    if (plcStep >= PLC_STEP_MAX)    // µ¥´ÎÔËÐÐ½áÊøÍ£»ú£¬»òÕßµ¥´ÎÔËÐÐ½áÊø±£³ÖÖÕÖµ
    {        
        if (FUNCCODE_plcRunMode_ONCE_STOP == funcCode.code.plcRunMode) // µ¥´ÎÔËÐÐ½áÊøÍ£»ú
        {
            bStopPlc = 1;

            if (!runFlag.bit.common)    // PLCµ¥´ÎÔËÐÐ½áÊøÍ£»ú£¬¼Ó¼õËÙÊ±¼äÊ¹ÓÃ×îºóÒ»¶Î¼Ó¼õËÙÊ±¼ä²»Îª0µÄÄÇÒ»¶ÎPLCµÄ¼Ó¼õËÙÊ±¼ä
            {
                plcStep = 0;
            }
        }
        // µ¥´ÎÔËÐÐ½áÊø±£³ÖÖÕÖµ¡£±£³ÖÎª×îºóÒ»¶ÎµÄfrqAimTmp0
    }

    plcStep1 = plcStep;
    plcStepEnable = plcStep;
    if (plcStep1 >= PLC_STEP_MAX)
    {
        plcStep1 = PLC_STEP_MAX - 1;
    }

// PLCµÚ_¶Î¼Ó¼õËÙÊ±¼äÑ¡Ôñ
    accDecTimeSrcPlc = funcCode.code.plcAttribute[plcStep1].accDecTimeSet;

// ´¦ÀífrqAimTmp0
    frqPlc = UpdateMultiSetFrq(plcStep1);

// Íê³ÉÒ»¸öPLCÑ­»·£¬Êä³öÒ»¸ö_msµÄÂö³åÐÅºÅ
// ÆµÂÊÔ´Ñ¡Ôñ£¬ÔËÐÐÊ±²»¿ÉÐÞ¸Ä¡£ËùÒÔ¸Ã_msµÄ´¦Àí·ÅÔÚÕâÀïÃ»ÓÐÎÊÌâ¡£
// Ò²Òª¿¼ÂÇPLCµ¥´ÎÔËÐÐÍ£»ú/±£³ÖÖÕÖµ
    if (bPlcEndOneLoop)
    {
        if (++plcEndLoopDelayTicker >= PLC_END_LOOP_TIME_MAX / FRQ_SRC_PERIOD)
        {
            //plcEndLoopDelayTicker = 0;
            bPlcEndOneLoop = 0;
        }
    }
    
    frqPlcType = 1;

    return frqPlc;
#else
    return 0;
#endif
}


//=====================================================================
//
// ¸üÐÂ¶à¶ÎÖ¸Áî¡¢PLCµÄÆµÂÊ
//
// ÊäÈë£º
//      step -- ¶à¶ÎÖ¸Áîstep
// ÒªÇó: 0 <= step <= PLC_STEP_MAX-1
//
//=====================================================================
LOCALF int32 UpdateMultiSetFrq(Uint16 step)
{
#if DEBUG_F_MULTI_SET_PLC
    static Uint16 stepOld;
    int32 frqPlc;
    int16 i;
    
    if (!step)               // plcStep == 0
    {
        switch (funcCode.code.plcFrq0Src) // F8-00 ¶à¶ÎËÙ0¸ø¶¨·½Ê½
        {
            case FUNCCODE_plcFrq0Src_FC:
                frqPlc = (int32)(int16)funcCode.code.plcFrq[0] * maxFrq / 1000;
                break;

            case FUNCCODE_plcFrq0Src_AI1:
            case FUNCCODE_plcFrq0Src_AI2:
            case FUNCCODE_plcFrq0Src_AI3:
                i = funcCode.code.plcFrq0Src - FUNCCODE_plcFrq0Src_AI1;
                frqPlc = ((int32)aiDeal[i].set * maxFrq + (1 << 14)) >> 15;
                break;
                
            case FUNCCODE_plcFrq0Src_PULSE:
                frqPlc = ((int32)pulseInSet * maxFrq + (1 << 14)) >> 15;
                break;
                
            case FUNCCODE_plcFrq0Src_PID:
                frqPlc = FrqPidSetDeal();
                break;
                
            case FUNCCODE_plcFrq0Src_PRESET_FRQ:
                if (stepOld)    // stepOld != 0
                {
                    frqDigitalTmp = frqDigitalPlc;  // »Ö¸´frqDigitalTmp
                }
                
                frqPlc = FrqAimUpDownDeal();
                break;
                
            default:
                break;
        }
    }
    else    // plcStep != 0
    {
        if (!stepOld)    // stepOld == 0
        {
            frqDigitalPlc = frqDigitalTmp;  // ±£´æfrqDigitalTmp
        }

        frqPlc = (int32)(int16)funcCode.code.plcFrq[step] * maxFrq / 1000;
    }

    stepOld = step;

    return frqPlc;
#endif
}



//=====================================================================
//
// ÉÏÏÂÏÞÆµÂÊ£¬×î´óÆµÂÊ¸üÐÂ
// ÉÏÏÞÆµÂÊÔ´¡ÔñÎªAIÊ±£¬100.0%ÔÓ¦¹¦ÄÜÂëÉè¶¨
//
//=====================================================================
LOCALF void UpdateLimitFrq(void)
{
    int32 tmp;
    Uint16 i;

// ×î´óÆµÂÊ
    maxFrq = funcCode.code.maxFrq;
    frqPuQ15 = maxFrq + 20 * decNumber[funcCode.code.frqPoint];

// ÉÏÏÞÆµÂÊ
    tmp = funcCode.code.upperFrq;
    switch (funcCode.code.upperFrqSrc)
    {
        case FUNCCODE_upperFrqSrc_FC:  // ÊýÖµÉè¶¨
            break;

        case FUNCCODE_upperFrqSrc_AI1: // AI1
        case FUNCCODE_upperFrqSrc_AI2: // AI2
        case FUNCCODE_upperFrqSrc_AI3: // AI3
            i = funcCode.code.upperFrqSrc - FUNCCODE_upperFrqSrc_AI1;
            tmp = ((int32)aiDeal[i].set * maxFrq + (1<<14)) >> 15;
            tmp += funcCode.code.upperFrqOffset;    // ÉÏÏÞÆµÂÊÆ«ÖÃ
            break;
            
        case FUNCCODE_upperFrqSrc_PULSE: // PULSEÂö³åÉè¶¨(DI5)
            tmp = ((int32)pulseInSet * maxFrq + (1<<14)) >> 15;
            tmp += funcCode.code.upperFrqOffset;    // ÉÏÏÞÆµÂÊÆ«ÖÃ
            break;
            
        case FUNCCODE_upperFrqSrc_COMM:
            // funcCode.code.frqComm²»ÄÜ³¬¹ý32767£¬Ä¿Ç° [-10000, +10000]
            tmp = ((int32)(int16)funcCode.code.frqComm * maxFrq + 5000) / 10000;   // ËÄÉáÎåÈë
            break;

        default:
            break;
    }

    

    if (tmp < 0L)        // AI£¬PulseIn¿ÉÄÜÎª¸ºÊý
        tmp = 0;
    if (tmp > (int32)maxFrq)
        tmp = maxFrq;
    
    upperFrq = tmp;

// ÏÂÏÞÆµÂÊ
    lowerFrq = funcCode.code.lowerFrq;

// ÉÏÏÞÆµÂÊ²»ÄÜµÍÓÚÏÂÏÞÆµÂÊ
    if (upperFrq < lowerFrq)
    {
        upperFrq = lowerFrq;
    }
}


//=====================================================================
//
// º¯Êý  : ÌøÔ¾ÆµÂÊ´¦Àí
// ²ÎÊý  : 
//
//=====================================================================
LOCALF void JumpFrqDeal(void)
{
#if DEBUG_F_JUMP_FRQ
    int32 tmp = ABS_INT32(frqAimTmp);
    int32 tmp1 = ABS_INT32(frqTmp);

    jumpFrq1.frq = funcCode.code.jumpFrq1;
    jumpFrq1.range = funcCode.code.jumpFrqRange;
    JumpFrqEndCalc(&jumpFrq1);

    jumpFrq2.frq = funcCode.code.jumpFrq2;
    jumpFrq2.range = funcCode.code.jumpFrqRange;
    JumpFrqEndCalc(&jumpFrq2);

    if ((jumpFrq2.low <= jumpFrq1.low) && (jumpFrq1.low <= jumpFrq2.high))
    {
        jumpFrq1.low = jumpFrq2.low;
    }
    if ((jumpFrq1.low <= jumpFrq2.low) && (jumpFrq2.low <= jumpFrq1.high))
    {   
        jumpFrq2.low = jumpFrq1.low;
    }
    if ((jumpFrq2.low <= jumpFrq1.high) && (jumpFrq1.high <= jumpFrq2.high))
    {   
        jumpFrq1.high = jumpFrq2.high;
    }
    if ((jumpFrq1.low <= jumpFrq2.high) && (jumpFrq2.high <= jumpFrq1.high))
    {   
        jumpFrq2.high = jumpFrq1.high;
    }

    if ((jumpFrq1.low < tmp) && (tmp < jumpFrq1.high))  // ÔÚÌøÔ¾ÆµÂÊÇø¼ä
    {
        // Éè¶¨ÆµÂÊÌøÔ¾
        if ((tmp < ABS_INT32(frq)) && (tmp < ABS_INT32(frqAim))) // Òª¿¼ÂÇ£ºµÍÓÚÏÂÏÞÆµÂÊ£¬ÒÔÏÂÏÞÆµÂÊÔËÐÐ¡£
        {
            tmp = jumpFrq1.high;
        }
        else
        {
            tmp = jumpFrq1.low;
        }
        
    }
    if ((jumpFrq2.low < tmp) && (tmp < jumpFrq2.high))  // ÔÚÌøÔ¾ÆµÂÊÇø¼ä
    {
        // Éè¶¨ÆµÂÊÌøÔ¾
        if ((tmp < ABS_INT32(frq)) && (tmp < ABS_INT32(frqAim)))
        {
            tmp = jumpFrq2.high;
        }
        else
        {
            tmp = jumpFrq2.low;
        }
    }

    // Éè¶¨ÆµÂÊÌøÔ¾
    if (tmp > (int32)upperFrq)
    {
        tmp = (int32)upperFrq;
    }
    frqAimTmp = (frqAimTmp >= 0) ? (tmp) : (-tmp);
    
    
    if (funcCode.code.jumpFrqMode)
    {
        if ((jumpFrq1.low < tmp1) && (tmp1 < jumpFrq1.high))  // ÔÚÌøÔ¾ÆµÂÊÇø¼ä
        {
            if((ABS_INT16(frqAimTmp) >= ABS_INT16(frqTmp))
                    && (runFlag.bit.run)
                    && (runCmd.bit.common)
                )
            {
                tmp1 = jumpFrq1.high;
            }
            else
            {
                tmp1 = jumpFrq1.low;
            }        
        }
        if ((jumpFrq2.low < tmp1) && (tmp1 < jumpFrq2.high))  // ÔÚÌøÔ¾ÆµÂÊÇø¼ä
        {
            if((ABS_INT16(frqAimTmp) >= ABS_INT16(frqTmp))
                && (runFlag.bit.run)
                && (runCmd.bit.common)
            )
            {
                tmp1 = jumpFrq2.high;
            }
            else
            {
                tmp1 = jumpFrq2.low;
            }
        }

        if (tmp1 > (int32)upperFrq)
        {
            tmp1 = (int32)upperFrq;
        }

        frqTmp = (frqTmp >= 0) ? (tmp1) : (-tmp1);
    }
#endif
}

//=====================================================================
//
// ÌøÔ¾ÆµÂÊµÄÁ½¶Ëµã¼ÆËã
//
// ÊäÈë£ºp->frq   -- ÌøÔ¾ÆµÂÊ
//       p->range -- ÌøÔ¾ÆµÂÊ·ù¶È
// Êä³ö£ºp->low   -- ÌøÔ¾ÆµÂÊ·¶Î§µÄlow
//       p->high  -- ÌøÔ¾ÆµÂÊ·¶Î§µÄhigh
//
//=====================================================================
#define JUMP_FRQ_RANGE_ALL  0   // ÌøÔ¾ÆµÂÊµÄhigh, lowÖ®¼äµÄ²îÖµÎªjumpFrqRangeµÄ2±¶¡£
#define JUMP_FRQ_RANGE_HALF 1   // ÌøÔ¾ÆµÂÊµÄhigh, lowÖ®¼äµÄ²îÖµÎªjumpFrqRange¡£
#define JUMP_FRQ_RANGE  JUMP_FRQ_RANGE_ALL  // ÌøÔ¾ÆµÂÊµÄhigh, lowÖ®¼äµÄ²îÖµÎªjumpFrqRangeµÄ2±¶¡£
//#define JUMP_FRQ_RANGE  JUMP_FRQ_RANGE_HALF // ÌøÔ¾ÆµÂÊµÄhigh, lowÖ®¼äµÄ²îÖµÎªjumpFrqRangeµÄ1±¶¡£
LOCALF void JumpFrqEndCalc(JUMP_FRQ *p)
{
#if DEBUG_F_JUMP_FRQ
    int32 low;

#if 0       // ÌøÔ¾ÆµÂÊÉèÎª0Ê±£¬²»Æð×÷ÓÃ¡£ÓëÖ®Ç°±£³ÖÒ»ÖÂ
    if (0 == p->frq)    // ÌøÔ¾ÆµÂÊÉèÎª0Ê±£¬²»Æð×÷ÓÃ¡£
    {
        p->low = 0;
        p->high = 0;
        return;
    }
#endif

    low = (int32)p->frq - ((int32)p->range >> JUMP_FRQ_RANGE);
    if (low < 0)
        low = 0;
    p->low = low;

    p->high = (int32)p->frq + ((int32)p->range >> JUMP_FRQ_RANGE);
#endif
}
#undef JUMP_FRQ_RANGE_ALL
#undef JUMP_FRQ_RANGE_HALF
#undef JUMP_FRQ_RANGE



//=====================================================================
//
// ¸üÐÂÔËÐÐÄ£Ê½
// ×ª¾Ø¿ØÖÆ£¬ËÙ¶È¿ØÖÆ£¬Î»ÖÃ¿ØÖÆ
//
//=====================================================================
void UpdateRunMode(void)
{
    static Uint16 runModeOld;

#if (DEBUG_F_POSITION_CTRL)
    if ((FUNCCODE_posCtrl_POSITION_CTRL == funcCode.code.posCtrl)   // Î»ÖÃ¿ØÖÆ
        || ((FUNCCODE_posCtrl_SWITCH_TO_PC == funcCode.code.posCtrl)    // ËÙ¶È/×ª¾Ø¿ØÖÆ<->Î»ÖÃ¿ØÖÆ
            && (diFunc.f3.bit.posCtrl))
        || ((FUNCCODE_posCtrl_SWITCH_FROM_PC == funcCode.code.posCtrl)  // Î»ÖÃ¿ØÖÆ<->ËÙ¶È/×ª¾Ø¿ØÖÆ
            && (!diFunc.f3.bit.posCtrl))
        )
    {
        if (FUNCCODE_motorCtrlMode_FVC == motorFc.motorCtrlMode)
        {
            runMode = RUN_MODE_POSITION_CTRL;   // Î»ÖÃ¿ØÖÆ
        }
        else if (runFlag.bit.run)
        {
            errorOther = 98;    // µ±Ç°²»ÊÇFVC£¬Ôò±¨´í98
        }
    }
    else
#endif
    {
        // ËÙ¶È¿ØÖÆ/×ª¾Ø¿ØÖÆÇÐ»»
        runMode = funcCode.code.torqueCtrl ^ diFunc.f2.bit.SpdTorqSwitch;
        if (FUNCCODE_motorCtrlMode_VF == motorFc.motorCtrlMode)     // VFÔËÐÐ
        {
            runMode = RUN_MODE_SPEED_CTRL;                  // ËÙ¶È¿ØÖÆ
        }

        // ×ª¾Ø¿ØÖÆ½ûÖ¹
        runMode = runMode & (~diFunc.f1.bit.forbidTorqueCtrl);

        // ×ª¾Ø¿ØÖÆ
        if (RUN_MODE_TORQUE_CTRL == runMode)
        {
			// ¸üÐÂÉè¶¨×ª¾Ø
			TorqueCalc();

            if (torqueAim >= 0)
            {
                frqAimTmp0 = funcCode.code.torqueCtrlFwdMaxFrq;   // ×ª¾Ø¿ØÖÆÕýÏò×î´óÆµÂÊ
            }
            else
            {
                frqAimTmp0 = -(int32)funcCode.code.torqueCtrlRevMaxFrq;   // ×ª¾Ø¿ØÖÆ·´Ïò×î´óÆµÂÊ
            }
        }
        // ×ª¾Ø¿ØÖÆ --> ·Ç×ª¾Ø¿ØÖÆ
        else if (RUN_MODE_TORQUE_CTRL == runModeOld)    
        {
            frqTmp = frqRun;
        }
    }
 
    runModeOld = runMode;
}



int32 FrqSrcOperate(void)
{
    int32 frq;
    Uint16 digit[5];
    GetNumberDigit1(digit, funcCode.code.frqRunCmdBind);
    runCmdBind = digit[runSrc];

    // ÃüÁîÔ´°ó¶¨µÄÆµÂÊÔ´
    if (runCmdBind > 0)
    {
        frqAiPu = maxFrq;
        frqFlag.bit.comp = 0;
        frq = FrqXyCalc(runCmdBind);
        frqXDisp = 0; // ÏÔÊ¾
        frqYDisp = 0;
    }
    // ÎÞ°ó¶¨
    else
    {
        frq = GetFrq();

        frqXDisp = ABS_INT16(frqXy.x); // ÏÔÊ¾
        frqYDisp = ABS_INT16(frqXy.y);
    }
    return frq;
}



#define UP_DOWN_LIMIT_X 0
#define UP_DOWN_LIMIT_Y 1
#define UP_DOWN_LIMIT_NO_COMP 0
#define UP_DOWN_LIMIT_COMP 1
int32 frqMax;
int32 frqMin;
void getUpDownLimit(Uint16 comp, Uint16 type)
{
    int32 noUpDownFrq;
    int32 min;
    int32 upDownFrqPu;
    int32 minFrq;
    minFrq = frqMin;
    // Îª×éºÏ(Êý×Ö+·ÇÊý×Ö /  ·ÇÊý×Ö+Êý×Ö)
    if (comp == UP_DOWN_LIMIT_COMP)
    {
        //·ÇÊý×Ö+Êý×Ö
        if (type == UP_DOWN_LIMIT_Y)  // YÎªÊý×Ö
        {
            // X(·ÇÊý×Ö) + Y(Êý×Ö)
            noUpDownFrq = frqXy.x + frqXy.z;
        }
        // Êý×Ö+·ÇÊý×Ö
        else if (type == UP_DOWN_LIMIT_X)  // XÎªÊý×Ö
        {
            noUpDownFrq = frqXy.y + frqXy.z + (int32)funcCode.code.presetFrq;
            //if (frqXy.y >= 0)
            {
                min = 0 + frqXy.y + frqXy.z;
                if (min > minFrq)     // È¡´óÖµ
                {
                    minFrq = min;
                }
            }
            #if 0
            else
            {
                max = 0 + frqXy.y - frqXy.z;
                if (max < frqMax)     // È¡´óÖµ
                {
                    frqMax = max;
                }
            }
            #endif    
            
        }
    }
    else
    {
        noUpDownFrq = frqXy.z + (int32)funcCode.code.presetFrq;
        minFrq = frqXy.z;
    }

    // ×îÐ¡ÖµÏÞ·ù
    if (minFrq > frqMax)
    {
        minFrq = frqMax;
    }
    
    uPDownFrqMax = frqMax - noUpDownFrq;
    uPDownFrqMin = minFrq - noUpDownFrq;
    uPDownFrqLower1 = lowerFrq - noUpDownFrq;
    uPDownFrqLower2 = (int16)(-lowerFrq) - noUpDownFrq;

     // µþ¼ÓÊ±¼ÆËã¸¨ÖúÆµÂÊY(Êý×Ö)µÄ·¶Î§
    if ((comp == UP_DOWN_LIMIT_COMP) && (type == UP_DOWN_LIMIT_Y)) 
    {
        // µþ¼ÓÊ±¸¨ÖúÆµÂÊÔ´YµÄ·¶Î§
        if (funcCode.code.frqYRangeBase)
        {
            upDownFrqPu  = ((Uint32)ABS_INT32(frqXy.x)*funcCode.code.frqYRange) / 100;
        }
        else
        {
            upDownFrqPu = ((Uint32)maxFrq*funcCode.code.frqYRange) / 100;
        }

        if (uPDownFrqMax > upDownFrqPu)
        {
            uPDownFrqMax = upDownFrqPu;
        }

        if (uPDownFrqMin < (-upDownFrqPu))
        {
            uPDownFrqMin = -upDownFrqPu;
        }
    }
}

void ResetUpDownFrq(void)
{
    frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_OFF;
    upDownFrq = 0;
    uPDownFrqMax = 0;
    uPDownFrqMin = 0;
    uPDownFrqLower1= 0;
    uPDownFrqLower2= 0;

}

// »ñÈ¡¾­¹ýÖ÷¸¨ÔËËãÖ®ºóµÄÆµÂÊ
int32 GetFrq(void)
{
    int32 frq;
//    static int upperFrqBak;
//    static int lowerFrqBak;

    frqMax = upperFrq;         // ×î´ó£¬Ä¬ÈÏÎª ÉÏÏÞÆµÂÊ
    frqMin = -(int32)upperFrq; // ×îÐ¡£¬Ä¬ÈÏÎª -ÉÏÏÞÆµÂÊ

    
#if 0
    if ((upperFrqBak != upperFrqBak)
        || (lowerFrqBak != lowerFrq)
        )
    {
        ResetUpDownFrq();
    }
    
	upperFrqBak = upperFrq;
	lowerFrqBak = lowerFrq;
#endif

    frqFlag.bit.comp = 0;           // ÏÈÇåÁã
    frqFlag.bit.upDown = 0;
    frqFlag.bit.x = 0;
    frqFlag.bit.y = 0;
    frqXy.x = 0;
    frqXy.y = 0;
    frqXy.z = 0;                    // ÈôÃ»ÓÐµþ¼Ó£¬¸ÃÖµÈÔÈ»Îª0
    
    //switch ((funcCode.code.frqCalcSrc >> 0) & 0x000F)
    switch (funcCode.code.frqCalcSrc%10)
    {
        case FUNCCODE_frqCalcSrc_X: // Ö÷ÆµÂÊÔ´X
            frqFlag.bit.x = 1;  // Ö÷ÆµÂÊÔ´XÓÐÐ§
            
            break;

        case FUNCCODE_frqCalcSrc_COMPOSE: // Ö÷¸¨ÔËËã½á¹û
            frqFlag.bit.x = 1;  // Ö÷ÆµÂÊÔ´XÓÐÐ§
            frqFlag.bit.y = 1;  // ¸¨ÖúÆµÂÊÔ´YÓÐÐ§
            
            break;

        case FUNCCODE_frqCalcSrc_X_OR_Y: // Ö÷ <--> ¸¨
            if (!diFunc.f1.bit.frqSrcSwitch)
            {
                frqFlag.bit.x = 1;  // Ö÷ÆµÂÊÔ´XÓÐÐ§
            }
            else
            {
                frqFlag.bit.y = 1;  // ¸¨ÖúÆµÂÊÔ´YÓÐÐ§
            }
            
            break;

        case FUNCCODE_frqCalcSrc_X_OR_COMPOSE: // Ö÷ <--> Ö÷¸¨ÔËËã½á¹û
            if (!diFunc.f1.bit.frqSrcSwitch)
            {
                frqFlag.bit.x = 1;  // Ö÷ÆµÂÊÔ´XÓÐÐ§
            }
            else
            {
                frqFlag.bit.x = 1;  // Ö÷ÆµÂÊÔ´XÓÐÐ§
                frqFlag.bit.y = 1;  // ¸¨ÖúÆµÂÊÔ´YÓÐÐ§
            }
            
            break;

        case FUNCCODE_frqCalcSrc_Y_OR_COMPOSE: // ¸¨ <--> Ö÷¸¨ÔËËã½á¹û
            if (!diFunc.f1.bit.frqSrcSwitch)
            {
                frqFlag.bit.y = 1;  // ¸¨ÖúÆµÂÊÔ´YÓÐÐ§
            }
            else
            {
                frqFlag.bit.x = 1;  // Ö÷ÆµÂÊÔ´XÓÐÐ§
                frqFlag.bit.y = 1;  // ¸¨ÖúÆµÂÊÔ´YÓÐÐ§
            }
            break;

        default:
            break;
    }   

    if (frqFlag.bit.x && frqFlag.bit.y)
    {
        frqXy.z = funcCode.code.frqYOffsetFc; // ¸¨ÖúÆµÂÊÆ«ÖÃ
        
        //if (((funcCode.code.frqCalcSrc >> 4) & 0x000F) == 0)
        if ((funcCode.code.frqCalcSrc/10) == 0)
        {
            if ((funcCode.code.frqXSrc >= FUNCCODE_frqXySrc_AI1) ||
                (funcCode.code.frqYSrc >= FUNCCODE_frqXySrc_AI1))
            {
                frqFlag.bit.comp = 1;                 // ÖÃcomp±êÖ¾
            }
        }
    }

    // X+YÎª×éºÏÆµÂÊ
    if (frqFlag.bit.comp)
    {
        // Êý×Ö+·ÇÊý×Ö
        if (funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1)
        {
            GetFrqY();  // ¼ÆËã¸¨ÖúÆµÂÊY
            getUpDownLimit(UP_DOWN_LIMIT_COMP, UP_DOWN_LIMIT_X);
            GetFrqX();  // ¼ÆËãÖ÷ÆµÂÊX
        }
        // ·ÇÊý×Ö+Êý×Ö
        else if (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1)
        {
            GetFrqX();  // ¼ÆËãÖ÷ÆµÂÊX
            getUpDownLimit(UP_DOWN_LIMIT_COMP, UP_DOWN_LIMIT_Y); 
            GetFrqY();  // ¼ÆËã¸¨ÖúÆµÂÊY
        }
        // ·ÇÊý×Ö+·ÇÊý×Ö
        else
        {
            GetFrqX();  // ¼ÆËãÖ÷ÆµÂÊX
            GetFrqY();  // ¼ÆËã¸¨ÖúÆµÂÊY
        }
    }
    // Ö÷
    else 
    {
        // Ö÷
        if (frqFlag.bit.x)
        {
            if (funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1)
            {
                getUpDownLimit(UP_DOWN_LIMIT_NO_COMP, UP_DOWN_LIMIT_X); 
            }
            GetFrqX();  // ¼ÆËãÖ÷ÆµÂÊX
        }
        
        // ¸¨
        if (frqFlag.bit.y)
        {
            // ¸¨ÖúÆµÂÊYµÄ·¶Î§ÎªX
            if (funcCode.code.frqYRangeBase)
            {
                GetFrqX();
            }
            
            if (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1)
            {
                getUpDownLimit(UP_DOWN_LIMIT_NO_COMP, UP_DOWN_LIMIT_Y); 
            }
            
            GetFrqY();  // ¼ÆËã¸¨ÖúÆµÂÊY
        }   
    }
    
    if (frqFlag.bit.x && frqFlag.bit.y)
    {
        frq = GetFrqComp();
    }
    else if (frqFlag.bit.x)
    {
        frq = frqXy.x;
    }
    else if (frqFlag.bit.y)
    {
        frq = frqXy.y;
    }

    if (frq > frqMax)
    {
        frq = frqMax;
    }
    else if (frq < frqMin)
    {
        frq = frqMin;
    }

    // ·ÀÖ¹Ö÷¸¨ÆµÂÊÇÐ»»Ê±Ò»¸öÎªÊý×ÖÒ»¸ö²»ÎªÊý×Ö
    // ÇÐ»»Îª·ÇÊý×ÖÊ±frqFlag.bit.upDownÎÞ·¨ÖÃ1,´ËÊ±upDownFrq»á±»Çå³ý
    if (frqFlag.bit.upDown)
    {
        if(!((funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1) || 
            (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1)
            ))
        {
            frqFlag.bit.fcPosLimit = 0;
            frqFlag.bit.fcNegLimit = 0;
            frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_OFF;
            upDownFrqTmp = 0;
            upDownFrq = 0;
            
        }
    }
    return frq;
}


// Ö÷¸¨ÔËËã
int32 GetFrqComp(void)
{
#if DEBUG_F_FRQ_SRC_COMPOSE
    int32 frq;
    int32 x;
    int32 y;   
    int32 frqYCompMax;   // ¸¨ÖúÆµÂÊYµþ¼ÓÊ±ÉÏÏÞ

    // µþ¼ÓÊ±¸¨ÖúÆµÂÊÔ´YµÄ·¶Î§
    if (funcCode.code.frqYRangeBase)
    {
        frqYCompMax  = ((Uint32)ABS_INT32(frqXy.x)*funcCode.code.frqYRange) / 100;
    }
    else
    {
        frqYCompMax = ((Uint32)maxFrq*funcCode.code.frqYRange) / 100;
    }

    // YÆµÂÊÏÞ·ù
    if (frqXy.y > frqYCompMax)
    {
        frqXy.y = frqYCompMax;
    }

    
    //switch ((funcCode.code.frqCalcSrc >> 4) & 0x000F)
    switch (funcCode.code.frqCalcSrc/10)
    {
        case FUNCCODE_frqCalcSrc_ADD:   // Ö÷ + ¸¨
            x = frqXy.x;

            // X+YÊ±£¬µ±¸¨ÖúÆµÂÊÔ´ÎªÊý×Ö¸ø¶¨Ê±£¬Ô¤ÖÃÆµÂÊ£¨F0-08£©²»Æð×÷ÓÃ£¬UP/DOWNÆð×÷ÓÃ
            if ((funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1) &&
                (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1))
            {
                frqXy.y = 0;
            }
            y = frqXy.y;
            
            frq = x + y;
            //frqXy.z = zFc;

#if 0
            if (frq > upperFrq)
            {
                frq = upperFrq;
                frqXy.y = upperFrq - x - z;
            }
            else if (frq < -(int32)upperFrq)
            {
                frq = -(int32)upperFrq;
                frqXy.y = -(int32)upperFrq - x - z;
            }
#endif
            break;

        case FUNCCODE_frqCalcSrc_SUBTRATION: // Ö÷ - ¸¨
            x = frqXy.x;
            y = frqXy.y; 
            
            frq = x - y;
            //frqXy.z = zFc;
            break;

        case FUNCCODE_frqCalcSrc_MAX: // MAX(Ö÷, ¸¨)
            x = frqXy.x;
            y = frqXy.y; 

            //frq = GetMax(x, y);
            
            if (ABS_INT32(x) >= ABS_INT32(y))
            {
                frq = x;
            }
            else
            {
                frq = y;
            }
            
            //frqXy.z = zFc;
            break;

        case FUNCCODE_frqCalcSrc_MIN: // MIN(Ö÷, ¸¨)
            x = frqXy.x;
            y = frqXy.y; 
            
            if (ABS_INT32(x) < ABS_INT32(y))
            {
                frq = x;
            }
            else
            {
                frq = y;
            }

            //frqXy.z = zFc;
            break;

        default:
            break;
    }
    
    frq += frqXy.z;   // ¸¨ÖúÆµÂÊÆ«ÖÃ
    
#if 0
    if (frq > upperFrq)
    {
        frq = upperFrq;
    }
    else if (frq < -(int32)upperFrq)
    {
        frq = -(int32)upperFrq;
    }
#endif

    return frq;
#endif
}



//===========================================================
// »ñÈ¡Ö÷ÆµÂÊX
//===========================================================
int32 GetFrqX(void)
{
    int32 frq;
    
    frqAiPu = maxFrq;
    //frqFlag.bit.x = 1;

    // Ö÷ÆµÂÊÔ´XÓëÔ¤ÖÃÆµÂÊÇÐ»»
    if (diFunc.f2.bit.frqXSrc2Preset) 
    {
        frq = funcCode.code.presetFrq;
    }
    else
    {
        frq = FrqXyCalc(funcCode.code.frqXSrc);
    }

    frqXy.x = frq;     // ¸³Öµ
    
    return frq;
}


// »ñÈ¡¸¨ÆµÂÊY
int32 GetFrqY(void)
{
#if DEBUG_F_FRQ_SRC_COMPOSE
    int32 frq;
    
    // Îªµþ¼Ó
    if (frqFlag.bit.comp)    
    {
        // µþ¼ÓÊ±Y·¶Î§ÎªX
        if (FUNCCODE_frqYRangeBase_FRQ_X == funcCode.code.frqYRangeBase)
        {
            // X(Êý×Ö) + Y(·ÇÊý×Ö)Ê±ÒÔÔ¤ÖÃÆµÂÊ×÷ÎªYµÄÁ¿³Ì
            if ((funcCode.code.frqYSrc >= FUNCCODE_frqXySrc_AI1) 
                && (funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1)
                )
            {
                frqAiPu = funcCode.code.presetFrq;
            }
            else
            {
                frqAiPu = ABS_INT32(frqXy.x);
            }   
        }
        // µþ¼ÓÊ±Y·¶Î§Îª×î´óÆµÂÊ
        else
        {
            frqAiPu = maxFrq;
        }

        frqAiPu = ((Uint32)frqAiPu * funcCode.code.frqYRange) / 100;
    }
    // Ïà¶ÔÓÚ×î´óÆµÂÊ
    else    
    {
        frqAiPu = maxFrq;
    }

    // ¸¨ÆµÂÊÔ´YÓëÔ¤ÖÃÆµÂÊÇÐ»»
    if (diFunc.f2.bit.frqYSrc2Preset) 
    {
        frq = funcCode.code.presetFrq;
    }
    else
    {
        frq = FrqXyCalc(funcCode.code.frqYSrc);
    }        

    // µþ¼ÓÊ±¸¨ÖúÆµÂÊYÏÞ·ù
    if (frqFlag.bit.comp)
    {
        if (frq > frqAiPu)
        {
            frq = frqAiPu;
        }
        else if (frq < -frqAiPu)
        {
            frq = - frqAiPu;
        }
    }
    
    frqXy.y = frq;     // ¸³Öµ
    
    return frq;
#endif
}


//=====================================================================
//
//
//=====================================================================
LOCALF int32 FrqXyCalc(Uint16 src)
{
    int32 tmp = 0;
    
    switch (src)         // ·Çµã¶¯£¬²Å½øÈëÆµÂÊÔ´´¦Àí
    {
        case FUNCCODE_frqXySrc_FC:              // ¹¦ÄÜÂëÉè¶¨£¬µôµç²»¼ÇÒä
        case FUNCCODE_frqXySrc_FC_P_OFF_REM:    // ¹¦ÄÜÂëÉè¶¨£¬µôµç¼ÇÒä
            tmp = FrqAimUpDownDeal();
            break;

        case FUNCCODE_frqXySrc_AI1:             // AI1
        case FUNCCODE_frqXySrc_AI2:             // AI2
        case FUNCCODE_frqXySrc_AI3:             // AI3
            tmp = src - FUNCCODE_frqXySrc_AI1;
            tmp = ((int32)aiDeal[tmp].set * frqAiPu + (1 << 14)) >> 15;
            break;

        case FUNCCODE_frqXySrc_PULSE:           // PULSEÂö³åÉè¶¨(DI5)
            tmp = ((int32)pulseInSet * frqAiPu + (1 << 14)) >> 15;
            break;

        case FUNCCODE_frqXySrc_MULTI_SET:       // ¶à¶ÎÖ¸Áî
            tmp = UpdateMultiSetFrq(diFunc.f1.bit.multiSet);
            break;

        case FUNCCODE_frqXySrc_PLC:             // PLC
            tmp = FrqPlcSetDeal();
            break;

        case FUNCCODE_frqXySrc_PID:             // PID
            tmp = FrqPidSetDeal();
            break;

        case FUNCCODE_frqXySrc_COMM:            // Í¨Ñ¶Éè¶¨
            // funcCode.code.frqComm²»ÄÜ³¬¹ý32767£¬Ä¿Ç° [-10000, +10000]

#if DEBUG_F_PLC_CTRL
            if (funcCode.code.plcEnable)  // PLC¹¦ÄÜÓÐÐ§
            {
                tmp = (int32)(int16)funcCode.code.plcFrqSet * maxFrq / 10000;
            }
            else
#endif
#if DEBUG_F_P2P_CTRL
            if ((CanRxTxCon == P2P_COMM_SLAVE)   // µã¶ÔµãÍ¨Ñ¶´Ó»úÓÐÐ§ÇÒ½ÓÊÕÊý¾Ý×÷ÎªÆµÂÊ¸ø¶¨
                && (funcCode.code.p2pRevDataSel == P2P_REV_FRQ_SET)
                )
            { 
                tmp = ((int32)(int16)p2pData.processValue * maxFrq) / 10000;
            }
            else
#endif
            if (funcCode.code.commProtocolSec == PROFIBUS)   // PROFIBUS-DPÆµÂÊ¸ø¶¨
            {
                tmp = (int32)(Uint16)dpFrqAim;                    // Ö±½Ó¸ø¶¨ÆµÂÊ
            }
            else
            {
                tmp = (int32)(int16)funcCode.code.frqComm * maxFrq / 10000;
            }
            break;

        default:
            break;
    }

    return tmp;
}





