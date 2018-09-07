//======================================================================
//
// ²Ëµ¥´¦Àí
//
// Time-stamp: <2012-08-14 12:01:32  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_menu.h"
#include "f_main.h"
#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_io.h"
#include "f_ui.h"
#include "f_eeprom.h"
#include "f_comm.h"
#include "f_posCtrl.h"
#include "f_invPara.h"
#include "f_fcDeal.h"
#include "f_p2p.h"

#if F_DEBUG_RAM

#define DEBUG_F_USER_MENU_MODE              0   // ÓÃ»§²Ëµ¥Ä£Ê½
#define DEBUG_F_CHECK_MENU_MODE             0   // Ð£Ñé²Ëµ¥Ä£Ê½
#define DEBUG_F_NO_SAME                     0   // NoSameDeal
#define DEBUG_F_DISP_DIDO_STATUS_SPECIAL    0   // DIDOÖ±¹ÛÏÔÊ¾
#define DEBUG_F_GROUP_HIDE                  0   // groupHide
#define DEBUG_F_MOTOR_FUNCCODE1             0
#define DEBUG_F_MFK                         0   // MFK´¦Àí
#define DEBUG_F_PASSWORD                    0   // ÃÜÂë´¦Àí
#define DEBUG_F_ERROR_TUNE_USE_SHIFT        0   // ¹ÊÕÏ/µ÷Ð³Ê±¿ÉÒÔÊ¹ÓÃshift
#define DEBUG_FRQ_POINT                     0
#define DEBUG_ACC_DEC_TIME_POINT            0

#elif 1

#define DEBUG_F_NO_SAME                     1
#define DEBUG_F_USER_MENU_MODE              1
#define DEBUG_F_CHECK_MENU_MODE             1
#define DEBUG_F_DISP_DIDO_STATUS_SPECIAL    1
#define DEBUG_F_GROUP_HIDE                  1
#define DEBUG_F_MOTOR_FUNCCODE1             1
#define DEBUG_F_MFK                         1
#define DEBUG_F_PASSWORD                    1
#define DEBUG_F_ERROR_TUNE_USE_SHIFT        1   // ¹ÊÕÏ/µ÷Ð³Ê±¿ÉÒÔÊ¹ÓÃshift
#define DEBUG_FRQ_POINT                     1
#define DEBUG_ACC_DEC_TIME_POINT            1

#endif




// 2¼¶²Ëµ¥¿ÉÒÔÊ¹ÓÃshift°´¼ü
// 1,2¼¶²Ëµ¥ÉÁË¸, ÊäÈëÓÃ»§ÃÜÂëÊ±ÉÁË¸
//#define NEWMENU_MENU3_USE_LEFT_SFIFT    0  // 3¼¶²Ëµ¥¿ÉÒÔÊ¹ÓÃMFK°´¼ü×÷Îª×óÒÆ
//#define NEWMENU_REMEMBER_GRADE          0  // ¼ÇÒägroupµÄgrade


// Ê®½øÖÆµÄ¸öÎ»¡¢Ê®Î»¡¢°ÙÎ»¡¢Ç§Î»¡¢ÍòÎ»
const int16 decNumber[5] = {1,      10,     100,    1000, 10000};
// Ê®Áù½øÖÆµÄ¸öÎ»¡¢Ê®Î»¡¢°ÙÎ»¡¢Ç§Î»
const int16 hexNumber[4] = {0x0001, 0x0010, 0x0100, 0x1000};
const int16 deltaK[3] = {0, 1, -1};

int16 userMenuModeFcIndex;



#if 0
struct RESTORE_COMPANY_PARA_EXCEPT_INDEX
{
    Uint16 start;
    Uint16 end;
};

#define RESTORE_COMPANY_PARA_EXCEPT_NUMBER  6
static const struct RESTORE_COMPANY_PARA_EXCEPT_INDEX exceptRestoreSeries[RESTORE_COMPANY_PARA_EXCEPT_NUMBER] =
{
{GetCodeIndex(funcCode.group.ff[0]), GetCodeIndex(funcCode.group.ff[FFNUM-1])}, // FF ³§¼Ò²ÎÊý
{GetCodeIndex(funcCode.group.fp[0]), GetCodeIndex(funcCode.group.fp[FPNUM-1])}, // FP ¹¦ÄÜÂë¹ÜÀí
{GetCodeIndex(funcCode.group.f1[0]), GetCodeIndex(funcCode.group.f1[F1NUM-2])}, // F1 µç»ú²ÎÊý
{GetCodeIndex(funcCode.group.a3[0]), GetCodeIndex(funcCode.group.a3[F1NUM-2])}, // A3 µÚ2µç»ú²ÎÊý
{GetCodeIndex(funcCode.group.ae[0]), GetCodeIndex(funcCode.group.ae[AENUM-1])}, // AE AIAO³ö³§Ð£Õý
{GetCodeIndex(funcCode.code.errorLatest1), LAST_ERROR_RECORD_INDEX},    // µÚÒ»´Î¹ÊÕÏÀàÐÍ£¬..., ×îºóÒ»¸ö¹ÊÕÏ¼ÇÂ¼
};
#endif


Uint16 GetDispDigits(Uint16 index);

struct FC_MOTOR_DEBUG   // ÐÔÄÜµ÷ÊÔÊ¹ÓÃ¹¦ÄÜÂë×éµÄ¸öÊý
{
    Uint16 fc;  // ¹¦ÄÜÂë
    Uint16 u;   // ¼àÊÓ
};
struct FC_MOTOR_DEBUG motorDebugFc;


enum MENU0_DISP_STATUS menu0DispStatus;     // 0¼¶²Ëµ¥µÄÏÔÊ¾×´Ì¬
void UpdateMenu0DispStatus(void);

struct MENU_FUNC_CODE
{
    Uint16 group;
    Uint16 grade;
};
struct MENU_FUNC_CODE menuFc[MENU_MODE_MAX+1];      // ÇÐ»»²Ëµ¥Ä£Ê½Ê±¼ÇÒägroupºÍgrade
// ²Ëµ¥Ä£Ê½
enum MENU_MODE menuMode;        // ²Ëµ¥Ä£Ê½
enum MENU_MODE menuModeTmp;     // ²Ëµ¥Ä£Ê½£¬ÁÙÊ±Öµ
enum MENU_MODE menuModeOld;

enum MENU_MODE_OPERATE menuModeStatus;
enum FAC_PASS_RANDOM_VIEW_OPERATE facPassViewStatus;

#if DEBUG_F_NO_SAME
// ÆµÂÊÔ´£¬¹¦ÄÜÂëµÄÖµÒª»¥³â
#define FRQ_SRC_NO_SAME_NUMER   2
const Uint16 frqSrcFuncIndex[] =
{
    GetCodeIndex(funcCode.code.frqXSrc),
    GetCodeIndex(funcCode.code.frqYSrc),
};

// DI£¬
const Uint16 diFuncIndex[] =
{
    GetCodeIndex(funcCode.code.diFunc[0]),
    GetCodeIndex(funcCode.code.diFunc[1]),
    GetCodeIndex(funcCode.code.diFunc[2]),
    GetCodeIndex(funcCode.code.diFunc[3]),
    GetCodeIndex(funcCode.code.diFunc[4]),

    GetCodeIndex(funcCode.code.diFunc[5]),
    GetCodeIndex(funcCode.code.diFunc[6]),
    GetCodeIndex(funcCode.code.diFunc[7]),
    GetCodeIndex(funcCode.code.diFunc[8]),
    GetCodeIndex(funcCode.code.diFunc[9]),

    GetCodeIndex(funcCode.code.vdiFunc[0]),
    GetCodeIndex(funcCode.code.vdiFunc[1]),
    GetCodeIndex(funcCode.code.vdiFunc[2]),
    GetCodeIndex(funcCode.code.vdiFunc[3]),
    GetCodeIndex(funcCode.code.vdiFunc[4]),

    GetCodeIndex(funcCode.code.aiAsDiFunc[0]),
    GetCodeIndex(funcCode.code.aiAsDiFunc[1]),
    GetCodeIndex(funcCode.code.aiAsDiFunc[2]),
};
#endif



#define DECIMAL_DISPLAY_UPDATE_TIME     6       // 0¼¶²Ëµ¥ÏÔÊ¾(ÔËÐÐÊ±ÏÔÊ¾£¬Í£»úÊ±ÏÔÊ¾)Ð¡Êýµãºó2Î»ÏÔÊ¾»º³å¸üÐÂÊ±¼ä£¬_*12ms
#define UP_DOWN_DEAL_DONE_TIME          800     // UP/DOWN´¦ÀíÖ®ºóµÄ´¦ÀíÊ±¼ä£¬ÓÃÓÚÍ£Ö¹ÉÁË¸»òÕß¿ìËÙÉÁË¸£¬_ms




#define ONE_PLACE           0   // ¸öÎ»
#define TEN_PLACE           1   // Ê®Î»
#define HUNDRED_PLACE       2   // °ÙÎ»
#define THOUSAND_PLACE      3   // Ç§Î»
#define TEN_THOUSAND_PLACE  4   // ÍòÎ»


//===================================================================
enum MENU_LEVEL menuLevel;      // µ±Ç°²Ëµ¥¼¶±ð£¬¼´0,1,2,3¼¶²Ëµ¥
Uint16 menu3Number;             // 3¼¶²Ëµ¥ÏÔÊ¾µÄÖµ
Uint16 menuPwdNumber;           // pwd²Ëµ¥µÄÖµ

struct CURRENT_FUNC_CODE
{
    Uint16 index;               // µ±Ç°¹¦ÄÜÂëÔÚfuncCode.all[]Êý×éµÄÏÂ±ê

    Uint16 group;               // µ±Ç°¹¦ÄÜÂëµÄgroup
    Uint16 grade;               // µ±Ç°¹¦ÄÜÂëµÄgrade
};
struct CURRENT_FUNC_CODE curFc; // µ±Ç°¹¦ÄÜÂë
Uint16 curFcDispDigits;         // µ±Ç°¹¦ÄÜÂëµÄÏÔÊ¾Î»Êý


LOCALF Uint16 ticker4LowerDisp;  // ÔËÐÐÊ±ÏÔÊ¾£¬×îºó2Î»²»Òª¸üÐÂ¹ý¿ì

Uint16 superFactoryPass;
Uint16 groupHidePwdStatus;          // ¹¦ÄÜÂë×éÒþ²Ø
Uint16 groupHideChkOkFlag;          // ¹¦ÄÜÂë×éÒþ²ØÃÜÂëÐ£Ñéok

#define FC_READ_ONLY_FLAG                                               \
    ((!funcCode.code.userPasswordReadOnly) ||                           \
    (curFc.index == GetCodeIndex(funcCode.code.userPassword)) ||        \
    (curFc.index == GetCodeIndex(funcCode.code.userPasswordReadOnly))   \
    )

// ÊÇ·ñ¿ÉÐ´¡£1-µ±Ç°¿ÉÐ´£¬0-µ±Ç°²»¿ÉÐ´
#define IsWritable(attribute)                                       \
 ((FC_READ_ONLY_FLAG) &&                                            \
 ((ATTRIBUTE_READ_AND_WRITE == (attribute).bit.writable) ||         \
 ((ATTRIBUTE_READ_ONLY_WHEN_RUN == (attribute).bit.writable) &&     \
 (!runFlag.bit.run)))                                               \
)                                                                   \

enum FACTORY_PWD_STATUS
{
    FACTORY_PWD_LOCK,          // ³§¼ÒÃÜÂë, lock×´Ì¬
    FACTORY_PWD_UNLOCK         // ³§¼ÒÃÜÂë, unlock×´Ì¬
};
LOCALF enum FACTORY_PWD_STATUS factoryPwdStatus;    // ³§¼ÒÃÜÂë£¬Ä¬ÈÏÎª0(lock)
// ³õÊ¼ÖµÎªlock×´Ì¬¡£
// ½öµ±£º½øÈëFF-00£¬ÊäÈëÕýÈ·ÃÜÂëºóenter£¬factoryPwdStatus ==> unlock×´Ì¬
// µ±£ºÍË»Øµ½2¼¶²Ëµ¥(FF-xx -> Fx)
//     ½øÈë²»ÊÇFF×éµÄ3¼¶²Ëµ¥(FF-xx -> Fx-xx£¬enter)
//     factoryPwdStatus ==> lock×´Ì¬

Uint16 accDecFrqPrcFlag;   // ¼üÅÌUP/DOWNÐÞ¸ÄÆµÂÊ±êÖ¾
Uint16 bFrqDigitalDone4WaitDelay; // UP/DOWNÍê³ÉÖ®ºóÒ»¶ÎÊ±¼äµÄÏÔÊ¾´¦ÀíÊ±¼ä
LOCALF Uint16 accDecFrqTicker;
Uint16 frqDisp;
Uint16 frqAimDisp;
Uint16 frqPLCDisp;       // PLC¿É±à³Ì¿¨¶ÁÈ¡ÔËÐÐ
Uint16 frqAimPLCDisp;    // PLC¿¨±à³Ì¿¨¶ÁÈ¡Éè¶¨ÆµÂÊ
Uint16 pidFuncRefDisp;
Uint16 pidFuncFdbDisp;
Uint16 outVoltageDisp;
Uint16 outCurrentDisp;      // Êä³öµçÁ÷£¬Êµ¼ÊÖµ£¬
Uint16 itDisp;              // Êä³ö×ª¾Ø
Uint16 loadSpeedDisp;
Uint16 currentOcDisp;
Uint16 pulseInFrqDisp;
Uint16 frqRunDisp;
Uint16 pcOriginDisp;
Uint16 frqXDisp;
Uint16 frqYDisp;
Uint32 torqueCurrentAct;   // ×ª¾ØµçÁ÷

// °üÊ¾µÄÎÄ¼þ
// U×é£¬Í£»ú×´Ì¬ÏÔÊ¾µÈ
#include "f_funcCode_disp.c"


// °´ÏÂRUN¼ü
LOCALD void MenuOnRun(void);

// °´ÏÂSTOP¼ü
LOCALD void MenuOnStop(void);

// °´ÏÂMF.K¼ü
LOCALD void MenuOnMfk(void);


// °´ÏÂPRG¼ü
LOCALD void Menu0OnPrg(void);
void Menu1OnPrg(void);
LOCALD void Menu2OnPrg(void);
LOCALD void Menu3OnPrg(void);

// °´ÏÂUP¼ü
LOCALD void Menu0OnUp(void);    // 0¼¶²Ëµ¥ÏÂ°´¼üUPµÄº¯Êý
LOCALD void Menu1OnUp(void);    // 1¼¶²Ëµ¥ÏÂ°´¼üUPµÄº¯Êý
LOCALD void Menu2OnUp(void);    // 2¼¶²Ëµ¥ÏÂ°´¼üUPµÄº¯Êý
LOCALD void Menu3OnUp(void);    // 3¼¶²Ëµ¥ÏÂ°´¼üUPµÄº¯Êý

// °´ÏÂDOWN¼ü
LOCALD void Menu0OnDown(void);
LOCALD void Menu1OnDown(void);
LOCALD void Menu2OnDown(void);
LOCALD void Menu3OnDown(void);

// °´ÏÂENTER¼ü
LOCALD void Menu0OnEnter(void);
LOCALD void Menu1OnEnter(void);
LOCALD void Menu2OnEnter(void);
LOCALD void Menu3OnEnter(void);

// °´ÏÂSHIFT¼ü
LOCALD void Menu0OnShift(void);
LOCALD void Menu1OnShift(void);
LOCALD void Menu2OnShift(void);
LOCALD void Menu3OnShift(void);

// °´ÏÂQUICK¼ü
LOCALD void MenuOnQuick(void);


LOCALD void Menu0OnUpDown(void);
LOCALD void Menu1OnUpDown(Uint16 flag);
LOCALD void Menu2OnUpDown(Uint16 flag);
LOCALD void Menu3OnUpDown(Uint16 flag);


// ÏÔÊ¾»º³å¸üÐÂ
LOCALD void UpdateMenu0DisplayBuffer(void);
LOCALD void UpdateMenu1DisplayBuffer(void);
LOCALD void UpdateMenu2DisplayBuffer(void);
LOCALD void UpdateMenu3DisplayBuffer(void);
LOCALD void UpdateDisplayBufferAttribute(const Uint16 data, const union FUNC_ATTRIBUTE attribute);
void UpdateDisplayBufferVisualIoStatus(Uint32 value);
void UpdateDisplayBufferVisualDiFunc(Uint16 valueH, Uint32 valueL);
LOCALD void UpdateErrorDisplayBuffer(void);
LOCALD void UpdateTuneDisplayBuffer(void);


LOCALD void MenuPwdOnPrg(void);
LOCALD void MenuPwdHintOnUp(Uint16 flag);
LOCALD void MenuPwdHint2Input(void);
LOCALD void MenuPwdHintOnDown(Uint16 flag);
LOCALD void MenuPwdHintOnShift(void);
LOCALD void MenuPwdHintOnQuick(void);
LOCALD void UpdateMenuPwdHintDisplayBuffer(void);

LOCALD void MenuPwdInputOnPrg(void);
LOCALD void MenuPwdInputOnUp(void);
LOCALD void MenuPwdInputOnEnter(void);
LOCALD void MenuPwdInputOnDown(void);
LOCALD void MenuPwdInputOnShift(void);
LOCALD void MenuPwdInputOnQuick(void);
LOCALD void UpdateMenuPwdInputDisplayBuffer(void);

void MenuPwdInputOnUpDown(Uint16 flag);

void MenuModeSwitch(void);


struct MENU_ATTRIBUTE menuAttri[MENU_LEVEL_NUM];
#if F_DEBUG_RAM
#pragma DATA_SECTION(menu, "const_zone");
#endif
const sysMenu menu[MENU_LEVEL_NUM] =
{
// 0¼¶²Ëµ¥
    {Menu0OnPrg,        Menu0OnUpDown,      Menu0OnEnter,
     MenuOnMfk,         Menu0OnUpDown,      Menu0OnShift,
     MenuOnRun,         MenuOnStop,         MenuOnQuick,
     UpdateMenu0DisplayBuffer},
// 1¼¶²Ëµ¥
    {Menu1OnPrg,        Menu1OnUp,          Menu1OnEnter,
     MenuOnMfk,         Menu1OnDown,        Menu1OnShift,
     MenuOnRun,         MenuOnStop,         MenuOnQuick,
     UpdateMenu1DisplayBuffer},
// 2¼¶²Ëµ¥
    {Menu2OnPrg,        Menu2OnUp,          Menu2OnEnter,
     MenuOnMfk,         Menu2OnDown,        Menu2OnShift,
     MenuOnRun,         MenuOnStop,         MenuOnQuick,
     UpdateMenu2DisplayBuffer},
// 3¼¶²Ëµ¥
    {Menu3OnPrg,        Menu3OnUp,          Menu3OnEnter,
     MenuOnMfk,         Menu3OnDown,        Menu3OnShift,
     MenuOnRun,         MenuOnStop,         MenuOnQuick,
     UpdateMenu3DisplayBuffer},
// PwdHint²Ëµ¥
    {MenuPwdOnPrg,      MenuPwdHint2Input,  MenuPwdHint2Input,
     MenuOnMfk,         MenuPwdHint2Input,  MenuPwdHintOnShift,
     MenuOnRun,         MenuOnStop,         MenuPwdHintOnQuick,
     UpdateMenuPwdHintDisplayBuffer},
// PwdInput²Ëµ¥
    {MenuPwdOnPrg,      MenuPwdInputOnUp,   MenuPwdInputOnEnter,
     MenuOnMfk,         MenuPwdInputOnDown, MenuPwdInputOnShift,
     MenuOnRun,         MenuOnStop,         MenuPwdInputOnQuick,
     UpdateMenuPwdInputDisplayBuffer},
};
//(void (*)(void))Menu0OnPrg

   
// ·­×ª´¦Àíº¯Êý
LOCALD Uint16 OverTurnDeal(Uint16 data, Uint16 upper, Uint16 lower, Uint16 flag);
// 0¼¶²Ëµ¥Ñ­»·ÒÆÎ»º¯Êý
LOCALD void cycleShiftDeal(Uint16 flag);



struct GROUP_DISPLAY
{
    Uint16 dispF;   // ¹¦ÄÜÂë×éF0, ÏÔÊ¾F
    Uint16 disp0;   // ¹¦ÄÜÂë×éF0, ÏÔÊ¾0
};
struct GROUP_DISPLAY groupDisplay;
void UpdateGroupDisplay(Uint16 group);

Uint16 GroupUpDown(const Uint16 funcCodeGrade[], Uint16 group, Uint16 flag);
void DealUserMenuModeGroupGrade(Uint16 flag);
void GetGroupGrade(Uint16 index);

//-------------------------------------------------
Uint16 checkMenuModeCmd;    // ËÑË÷Ö¸Áî¡£1-¿ªÊ¼ËÑË÷£¬0-ÎÞËÑË÷Ö¸Áî/ËÑË÷Íê³É
Uint16 checkMenuModePara;   // ËÑË÷Ö¸ÁîµÄ²ÎÊý
enum CHECK_MENU_MODE_DEAL checkMenuModeDealStatus;
Uint16 checkMenuModeSerachNone; // ËÑË÷£¬Ã»ÓÐÕÒµ½Óë³ö³§Öµ²»Í¬µÄ¹¦ÄÜÂë£¬±êÖ¾¡£
// 1-Ã»ÓÐÕÒµ½Óë³ö³§Öµ²»Í¬µÄ¹¦ÄÜÂë
// 0-ÕÒµ½ÁËÓë³ö³§Öµ²»Í¬µÄ¹¦ÄÜÂë
void DealCheckMenuModeGroupGrade(Uint16 flag);
//-------------------------------------------------

Uint16 LimitOverTurnDeal(const Uint16 limit[], Uint16 data, Uint16 upper, Uint16 low, Uint16 flag);
void MotorDebugFcDeal(void);
void fghldf(Uint16 dest[], const Uint16 src[], Uint16 length);
void Menu0AddMenuLevel(void);
Uint16 ValidateTuneCmd(Uint16 value, Uint16 motorIndex);




void GroupHideDeal(Uint16 funcCodeGrade[]);


//=====================================================================
//
// ËùÓÐ²Ëµ¥¼¶±ðÏÂ£¬°´ÏÂrun¼üµÄ´¦Àí
// run°´¼üÔÚ ÔËÐÐÃüÁîº¯ÊýÖÐ´¦Àí£¬Ö±½ÓÊ¹ÓÃµ±Ç°µÄ°´¼ü¡£
//
//=====================================================================
LOCALF void MenuOnRun(void)
{
    ;
}


//=====================================================================
//
// ËùÓÐ²Ëµ¥¼¶±ðÏÂ£¬°´ÏÂstop¼üµÄ´¦Àí
// run°´¼üÔÚ ÔËÐÐÃüÁîº¯ÊýÖÐ´¦Àí£¬Ö±½ÓÊ¹ÓÃµ±Ç°µÄ°´¼ü¡£
//
//=====================================================================
LOCALF void MenuOnStop(void)
{
    ;
}



// quick´¦Àí
void MenuOnQuick(void)
{
    Uint16 digit[5];
    enum MENU_MODE menuModeNext[3];

    // ½öÓÐÁã¼¶Ë÷ÒýÔòQUICKÎÞÐ§
    if (!funcCode.code.menuMode)
    {
        menuModeTmp = MENU_MODE_BASE;
        return;
    }
    
    if (MENU_MODE_ON_QUICK == menuModeStatus)   // µÚ1´Î°´¼ü£¬ÏÔÊ¾µ±Ç°Ä£Ê½£»Ö®ºó²Å¸Ä±äÄ£Ê½
    {
        GetNumberDigit1(digit, funcCode.code.menuMode);
        if (digit[0])
		{
            menuModeNext[0] = MENU_MODE_USER;
		}
        else
        {
            menuModeNext[0] = MENU_MODE_CHECK;
        }

        if (digit[1])
		{
            menuModeNext[1] = MENU_MODE_CHECK;
		}
        else
        {
            menuModeNext[1] = MENU_MODE_BASE;
        }

        menuModeNext[2] = MENU_MODE_BASE;

        menuModeTmp = menuModeNext[((Uint16)menuModeTmp) - 1];
        
    }

    menuModeStatus = MENU_MODE_ON_QUICK;
}



LOCALF void Menu1OnUp(void)
{
    Menu1OnUpDown(ON_UP_KEY);
}


LOCALF void Menu2OnUp(void)
{
    Menu2OnUpDown(ON_UP_KEY);
}


LOCALF void Menu3OnUp(void)
{
    Menu3OnUpDown(ON_UP_KEY);
}



LOCALF void Menu1OnDown(void)
{
    Menu1OnUpDown(ON_DOWN_KEY);
}


LOCALF void Menu2OnDown(void)
{
    Menu2OnUpDown(ON_DOWN_KEY);
}


LOCALF void Menu3OnDown(void)
{
    Menu3OnUpDown(ON_DOWN_KEY);
}


//=====================================================================
//
// ËùÓÐ²Ëµ¥¼¶±ðÏÂ£¬°´ÏÂMF.K¼üµÄ´¦Àí
//
//=====================================================================
LOCALF void MenuOnMfk(void)
{
#if DEBUG_F_MFK
    if (tuneCmd)
    {
        return;
    }

    switch (funcCode.code.mfkKeyFunc)
    {
        case FUNCCODE_mfkKeyFunc_SWITCH: // Óë²Ù×÷Ãæ°åÃüÁîÍ¨µÀÇÐ»»
            // F0-00¹¦ÄÜÂëÉè¶¨ÖµÎª²Ù×÷Ãæ°å£¬MF.K¼üµÄ²Ù×÷Ãæ°åÃüÁîÔ´Í¨ÇÐ»»ÃüÁî²»Æð×÷ÓÃ
            if (FUNCCODE_runSrc_PANEL != funcCode.code.runSrc)
            {
                keyFunc = KEY_SWITCH;
            }
            break;

        case FUNCCODE_mfkKeyFunc_REVERSE:   // Õý·´×ªÇÐ»»
            if (FUNCCODE_runSrc_PANEL == runSrc)    // µ±Ç°ÃüÁîÔ´Í¨µÀÎª²Ù×÷Ãæ°å
            {
                keyFunc = KEY_REV;
            }
            break;

        case FUNCCODE_mfkKeyFunc_FWD_JOG:
            keyFunc = KEY_FWD_JOG;
            break;

        case FUNCCODE_mfkKeyFunc_REV_JOG:
            keyFunc = KEY_REV_JOG;
            break;
            
        default:
            break;
    }
#endif
}


//=====================================================================
//
// menu0, Áã¼¶²Ëµ¥
//
//=====================================================================
LOCALF void Menu0OnPrg(void)
{
    if (tuneCmd)
    {
        if (!runFlag.bit.tune)
        {
            menuLevel = MENU_LEVEL_2;    // È¡Ïûµ÷Ð³£¬»Øµ½2¼¶²Ëµ¥
            tuneCmd = 0;
        }
        else
            Menu0AddMenuLevel();
//        return;
    }

#if DEBUG_F_PASSWORD
    else if ((funcCode.code.userPassword)    // ÓÐÓÃ»§ÃÜÂë
            && (menuMode != MENU_MODE_USER)  // µ±Ç°²»ÎªÓÃ»§¶¨ÖÆ²ÎÊýÄ£Ê½
//       || (funcCode.code.userPasswordReadOnly)
        )
    {
        menuLevel = MENU_LEVEL_PWD_HINT;    // ½øÈëPWD_HINT²Ëµ¥£¬ÌáÊ¾½øÐÐÃÜÂëÊäÈë
    }
    else                        // Ã»ÓÐÃÜÂë
#endif
    {
        Menu0AddMenuLevel();
    }

    menuAttri[menuLevel].operateDigit = 0;
}


LOCALF void Menu0OnEnter(void)
{
    if (MENU_MODE_ON_QUICK == menuModeStatus)       // °´ÏÂQUICKºó£¬ÔÙ°´¼üENTER
    {
        MenuModeSwitch();
        return;
    }
}


void MenuModeSwitch(void)
{
    menuModeOld = menuMode;

    menuModeStatus = MENU_MODE_NONE;
        
    // ÇÐ»»²Ëµ¥Ä£Ê½Ê±¼ÇÒägroupºÍgrade
    menuFc[menuMode].group = curFc.group;       // ±£´æold
    menuFc[menuMode].grade = curFc.grade;
    menuMode = menuModeTmp;
    curFc.group = menuFc[menuModeTmp].group;    // »Ö¸´new
    curFc.grade = menuFc[menuModeTmp].grade;

// Èô»Ö¸´Ö®ºóµÄgroupÎª³§¼Ò²ÎÊý×é£¬ÇÒ
// menuMode±»¸Ä±ä£¬³§¼ÒÃÜÂë£¬lock×´Ì¬
    if ((menuModeOld != menuMode)
        && (FC_GROUP_FACTORY == curFc.group)
        )
    {
        factoryPwdStatus = FACTORY_PWD_LOCK;
        curFc.grade = 0;                        // ½øÈëlock×´Ì¬£¬gradeÇåÁã
    }

    MenuModeDeal();
	
// ¸Ä±ämenuModeÊ±£¬3¼¶²Ëµ¥¸ü¸ÄÎª2¼¶²Ëµ¥¡£
    if (menuModeOld == MENU_MODE_USER)
    {
        Menu0OnPrg();
    }
    else if ((menuLevel == MENU_LEVEL_3)
        && (menuModeOld != menuMode)
        )
    {
        menuLevel = MENU_LEVEL_2;
    }
// -C-Ä£Ê½Ã»ÓÐ1¼¶²Ëµ¥
// 0/1¼¶²Ëµ¥ÏÂ¸Ä±äÎª-C-Ä£Ê½£¬½øÈë2¼¶²Ëµ¥
    else if (((menuLevel == MENU_LEVEL_0) || (menuLevel == MENU_LEVEL_1))
        && (MENU_MODE_CHECK == menuMode)
        )
    {
        Menu0OnPrg();
    }
// ÓÃ»§¶¨ÖÆ²Ëµ¥ÎÞ1¼¶²Ëµ¥
// 0/1¼¶²Ëµ¥ÏÂ¸Ä±äÎªÓÃ»§¶¨ÖÆÄ£Ê½£¬½øÈë2¼¶²Ëµ¥
    else if (((menuLevel == MENU_LEVEL_0) || (menuLevel == MENU_LEVEL_1))
        && (MENU_MODE_USER == menuMode)
        )
    {
        Menu0OnPrg();
    }
// 0¼¶²Ëµ¥ÏÂ¸ü¸Ä£¬½øÈë1¼¶²Ëµ¥
    else if (menuLevel == MENU_LEVEL_0)
    {
        Menu0OnPrg();
    }

// ¸Ä±ämenuModeÊ±£¬µ±Ç°²Ù×÷bitÎª0
    if (menuModeOld != menuMode)
    {
        menuAttri[menuLevel].operateDigit = 0;
    }

    
}


LOCALF void Menu0OnUpDown(void)
{
    if (!tuneCmd)
    {
        accDecFrqPrcFlag = ACC_DEC_FRQ_WAIT;
        accDecFrqTicker = 0;
        frqKeyUpDownDelta = upDownDelta;
    }
}


LOCALF void Menu0OnShift(void)
{
    ticker4LowerDisp = 0;       // 0¼¶²Ëµ¥ÏÂ°´¼üshift£¬ÏÔÊ¾×îºó2Î»Êý×Ö

#if DEBUG_F_ERROR_TUNE_USE_SHIFT
// 0¼¶²Ëµ¥ÏÂ£¬ÓÐ¹ÊÕÏÊ±°´¼üshiftÈÔÈ»ÓÐÐ§
    if (MENU0_DISP_STATUS_RUN_STOP != menu0DispStatus)
    {
        menu0DispStatus = MENU0_DISP_STATUS_RUN_STOP;   // ½øÈëÔËÐÐ/Í£»úÏÔÊ¾×´Ì¬
    }
    else
#elif 1
    if ((!tuneCmd)
        && (!errorCode)         // 0¼¶²Ëµ¥ÏÂ£¬ÓÐ¹ÊÕÏÊ±°´¼üshiftÎÞÐ§
        )
#endif
    {
        cycleShiftDeal(1);      // 0¼¶²Ëµ¥ÏÔÊ¾µÄÑ­»·ÒÆÎ»´¦Àí
    }
}


//=====================================================================
//
// menu1, Ò»¼¶²Ëµ¥
//
//=====================================================================
void Menu1OnPrg(void)
{
    menuLevel = MENU_LEVEL_0;
    groupHideChkOkFlag = 0;

    ticker4LowerDisp = 0;   // ²Ëµ¥¼¶±ðÖØÐÂÖÃÎª0Ê±£¬tickerÇåÁã
}


LOCALF void Menu1OnEnter()
{
    if (MENU_MODE_ON_QUICK == menuModeStatus)       // °´ÏÂQUICKºó£¬ÔÙ°´¼üENTER
    {
        MenuModeSwitch();
        return;
    }
    
    menuLevel = MENU_LEVEL_2;
    menuAttri[menuLevel].operateDigit = 0;

    if (FC_GROUP_FACTORY == curFc.group)    // ½øÈëFF×é£¬gradeÖØÖÃ£¬ÔÚÊäÈëÕýÈ·ÃÜÂëºóÔÙ»Ö¸´
        curFc.grade = 0;
}


LOCALF void Menu1OnUpDown(Uint16 flag)
{
    curFc.group = GroupUpDown(funcCodeGradeCurMenuMode, curFc.group, flag);

// ²»¼ÇÒägrade£¬ÔÚÐÞ¸ÄgroupÊ±£¬grade¾ÍÇåÁã
    curFc.grade = 0;
}


LOCALF void Menu1OnShift(void)
{
    ;
}


//=====================================================================
//
// menu2, ¶þ¼¶²Ëµ¥
//
//=====================================================================
LOCALF void Menu2OnPrg(void)
{
    menuLevel = MENU_LEVEL_1;
    
    if ((MENU_MODE_CHECK == menuMode)
        || (MENU_MODE_USER == menuMode)
        )
    {
        //menuLevel = MENU_LEVEL_0;
        Menu1OnPrg();
    }

//  ÍË»Øµ½2¼¶²Ëµ¥(FF-xx -> Fx)
//  factoryPwdStatus ==> lock×´Ì¬
    factoryPwdStatus = FACTORY_PWD_LOCK;
}


LOCALF void Menu2OnEnter(void)
{
    if (MENU_MODE_ON_QUICK == menuModeStatus)       // °´ÏÂQUICKºó£¬ÔÙ°´¼üENTER
    {
        MenuModeSwitch();
        return;
    }

    if ((MENU_MODE_CHECK == menuMode)
        && (checkMenuModeSerachNone)
        )
    {
        return;
    }
    
//     ½øÈë²»ÊÇFF×éµÄ3¼¶²Ëµ¥(FF-xx -> Fx-00£¬enter)
//     factoryPwdStatus ==> lock×´Ì¬
    if (curFc.group != FC_GROUP_FACTORY)
    {
        factoryPwdStatus = FACTORY_PWD_LOCK;
    }

    menuAttri[MENU_LEVEL_2].operateDigit = 0;
    curFc.index = GetGradeIndex(curFc.group, curFc.grade);

    {
        menuLevel = MENU_LEVEL_3;

        menuAttri[MENU_LEVEL_3].operateDigit = 0;
        menu3Number = funcCode.all[curFc.index];

        // È·¶¨¹¦ÄÜÂëÏÔÊ¾Î»Êý
        curFcDispDigits = GetDispDigits(curFc.index);
    }
}


LOCALF void Menu2OnUpDown(Uint16 flag)
{
    if (MENU_MODE_USER == menuMode)
    {
        DealUserMenuModeGroupGrade(flag);
    }
    else if (MENU_MODE_CHECK == menuMode)
    {
        checkMenuModeCmd = 1;
        checkMenuModePara = flag;
    }
    else if (menuAttri[MENU_LEVEL_2].operateDigit >= 3)   // ÐÞ¸ÄcurrentGroup
    {
        Menu1OnUpDown(flag);
    }
    else                        // ÐÞ¸ÄcurrentGrade
    {
        int16 delta = 1;
        Uint16 tmp;

        if ((FC_GROUP_FACTORY == curFc.group) && (FACTORY_PWD_LOCK == factoryPwdStatus)) // ³§¼ÒÃÜÂë
        {
            Menu2OnEnter();     // FFÊ±£¬°´¼üUP/DOWNÒ²¿ÉÒÔ½øÈëÃÜÂëÊäÈë×´Ì¬
            return;
        }

#if 1
        tmp = OverTurnDeal(curFc.grade, funcCodeGradeCurMenuMode[curFc.group] - 1, 0, flag);
        if (curFc.grade == tmp)    // Ã»ÓÐ·­×ª
        {
            if (1 == menuAttri[MENU_LEVEL_2].operateDigit)
                delta = 10;

            if (ON_DOWN_KEY == flag)
                delta = -delta;

            curFc.grade = LimitDeal(0, curFc.grade, funcCodeGradeCurMenuMode[curFc.group] - 1, 0, delta);
        }
        else
        {
            curFc.grade = tmp;
        }
#elif 1
        if (1 == menuAttri[MENU_LEVEL_2].operateDigit)
            delta = 10;

        if (ON_DOWN_KEY == flag)
            delta = -delta;

        curFc.grade += delta;
        if ((int16)curFc.grade >= (int16)(funcCodeGradeCurMenuMode[curFc.group])
        {
            curFc.grade = 0;
        }
        else if ((int16)curFc.grade < 0)
        {
            curFc.grade = funcCodeGradeCurMenuMode[curFc.group] - 1;
        }
#endif
    }
}


LOCALF void Menu2OnShift(void)
{
// ÓÃ»§¶¨ÖÆ
// ·Ç³ö³§Öµ
// ²Ëµ¥Ä£Ê½
// Ã»ÓÐshift
    if (MENU_MODE_BASE == menuMode)
    {
        if (menuAttri[MENU_LEVEL_2].operateDigit == 0)
            menuAttri[MENU_LEVEL_2].operateDigit = 3;
        else if (menuAttri[MENU_LEVEL_2].operateDigit == 1)
            menuAttri[MENU_LEVEL_2].operateDigit = 0;
        else if (menuAttri[MENU_LEVEL_2].operateDigit == 3)
            menuAttri[MENU_LEVEL_2].operateDigit = 1;
    }
}


//=====================================================================
//
// menu3, Èý¼¶²Ëµ¥
//
//=====================================================================
LOCALF void Menu3OnPrg(void)
{
    menuLevel = MENU_LEVEL_2;
    menuAttri[MENU_LEVEL_2].operateDigit = 0;
}


LOCALF void Menu3OnEnter(void)
{
    Uint16 writable = funcCodeAttribute[curFc.index].attribute.bit.writable;

    if (MENU_MODE_ON_QUICK == menuModeStatus)       // °´ÏÂQUICKºó£¬ÔÙ°´¼üENTER
    {
        MenuModeSwitch();
        return;
    }

// ÕýÔÚ²Ù×÷EEPROM£¬²»ÏìÓ¦"±£´æ¹¦ÄÜÂë"£¬µ«ÊÇÏìÓ¦ÆäËû°´¼ü¡£
    if (FUNCCODE_RW_MODE_NO_OPERATION != funcCodeRwMode)
    {
        if ((FACTORY_PWD_INDEX != curFc.index)        // FF-00
            && (ATTRIBUTE_READ_ONLY_ANYTIME != writable)    // (ÈÎºÎÊ±ºò¶¼)Ö»¶ÁµÄ¹¦ÄÜÂë
            )
        {
            return;
        }
    }

// ¶ÔÓÚÔËÐÐÖÐÖ»¶ÁµÄ¹¦ÄÜÂë£¬ÔÚÍ£»úÊ±ÐÞ¸ÄÁË(µ«Î´°´ENTER)£¬ÔÙÔËÐÐ£¬°´ENTER£¬²»ÏìÓ¦¡£
// ¶ÔÓÚÈÎºÎÊ±ºò¶¼Ö»¶ÁµÄ¹¦ÄÜÂë£¬²»ÒªÕâÃ´´¦Àí¡£
    if ((runFlag.bit.run)      // ÔËÐÐ
        && (ATTRIBUTE_READ_ONLY_WHEN_RUN == writable) // ÔËÐÐÊ±Ö»¶Á
        && (funcCode.all[curFc.index] != menu3Number) // Öµ±»¸Ä±ä
        )
    {
        return;
    }

    // U×é¹¦ÄÜÂëÈ«²¿Ö»¶Á
    if (curFc.group >= FUNCCODE_GROUP_U0)
    {
        return;
    }

#if DEBUG_F_RESTORE_COMPANY_PARA_DEAL
    // FP-01²ÎÊý³õÊ¼»¯£¬FF-00³§¼ÒÃÜÂë£¬µ÷Ð³£¬²»Òª±£´æ
    if (GetCodeIndex(funcCode.code.paraInitMode) == curFc.index) // FP-01, ²ÎÊý³õÊ¼»¯
    {
        if (FUNCCODE_paraInitMode_CLEAR_RECORD == menu3Number) // Çå³ý¼ÇÂ¼
        {
            ClearRecordDeal();
        }
        else
        {
            if (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA == menu3Number) // FP-01==1
            {
                funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA;
            }
            else if (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL == menu3Number) // FP-01==3
            {
                //funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL;
            }
            else if (FUNCCODE_paraInitMode_SAVE_USER_PARA == menu3Number) // FP-01==4
            {
                funcCodeRwModeTmp = FUNCCODE_paraInitMode_SAVE_USER_PARA;
            }
            else if (FUNCCODE_paraInitMode_RESTORE_USER_PARA == menu3Number) // FP-01==5
            {
                if ((funcCode.code.saveUserParaFlag1 == USER_PARA_SAVE_FLAG1)
                    && (funcCode.code.saveUserParaFlag2 == USER_PARA_SAVE_FLAG2))
                {
                    // »Ö¸´ÒÑ±£´æµÄÓÃ»§²ÎÊý
                    funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_USER_PARA;
                }
                else
                {
                    // Ö®Ç°Î´½øÐÐÓÃ»§²ÎÊý±£´æ²Ù×÷,¸Ã¹¦ÄÜÎÞÐ§
                    return;
                }
            }
        }
    }
    else
#endif
        if (FACTORY_PWD_INDEX == curFc.index) // FF-00, ³§¼ÒÃÜÂë
//    else if ((DISPLAY_F == curFc.group) && (FACTORY_PWD_LOCK == factoryPwdStatus)) // FF-00, ³§¼ÒÃÜÂë
    {
        if (COMPANY_PASSWORD != menu3Number) // ³§¼ÒÃÜÂë²»ÕýÈ·
        {
            menu3Number = 0;
            menuAttri[MENU_LEVEL_3].operateDigit = 0;
            return;
        }
        else
        {
            factoryPwdStatus = FACTORY_PWD_UNLOCK;    // ³§¼ÒÃÜÂëÕýÈ·£¬½âËø
        }
    }
    else if (GetCodeIndex(funcCode.code.tuneCmd) == curFc.index) // µç»ú1µ÷Ð³
    {
        if (COMM_ERR_PARA == ValidateTuneCmd(menu3Number, MOTOR_SN_1))
            return;
    }
    else if (GetCodeIndex(funcCode.code.motorFcM2.tuneCmd) == curFc.index) // µç»ú2µ÷Ð³
    {
        if (COMM_ERR_PARA == ValidateTuneCmd(menu3Number, MOTOR_SN_2))
            return;
    }
    else if (GetCodeIndex(funcCode.code.motorFcM3.tuneCmd) == curFc.index) // µç»ú3µ÷Ð³
    {
        if (COMM_ERR_PARA == ValidateTuneCmd(menu3Number, MOTOR_SN_3))
            return;
    }
    else if (GetCodeIndex(funcCode.code.motorFcM4.tuneCmd) == curFc.index) // µç»ú4µ÷Ð³
    {
        if (COMM_ERR_PARA == ValidateTuneCmd(menu3Number, MOTOR_SN_4))
            return;
    }
    else if (ATTRIBUTE_READ_ONLY_ANYTIME != writable) // ·Ç ÈÎºÎÊ±ºò¶¼Ö»¶Á µÄ¹¦ÄÜÂë
    // ³ýÉÏÃæ¼¸¸ö¹¦ÄÜÂëÖ®Íâ£¬ÐÞ¸ÄµÄÖµ¶¼ÐèÒª±£´æ
    {
// Ä³Ð©»¥³âµÄ¹¦ÄÜÂë¡£ModifyFunccodeEnter()ÖÐ»á¶ÔfuncCode¸³Öµ¡£
// Ðë·ÅÔÚ ¹¦ÂÊ¸Ä±ä´¦Àí ºÍ »úÐÍ¸Ä±ä´¦Àí ºóÃæ¡£
        if (COMM_ERR_PARA == ModifyFunccodeEnter(curFc.index, menu3Number))
            return;

        //funcCode.all[curFc.index] = menu3Number; // RAM
        SaveOneFuncCode(curFc.index);  // ÕâÊ±²»ÓÃ¿¼ÂÇÓëEEPROMÖÐµÄÖµÊÇ·ñÏàÍ¬¡£
    }

    if (MENU_MODE_USER == menuMode)
    {
        DealUserMenuModeGroupGrade(ON_UP_KEY);
    }
    else if (MENU_MODE_CHECK == menuMode)
    {
        checkMenuModeCmd = 1;
        checkMenuModePara = ON_UP_KEY;
    }
    else if (++curFc.grade >= funcCodeGradeCurMenuMode[curFc.group])
    {
        curFc.grade = 0;
    }

    // µ÷Ð³¹¦ÄÜÂë£¬½øÈëµ÷Ð³×´Ì¬£¬menuLevel¸ÄÎª0¼¶¡£
    if ((tuneCmd) &&
        ((curFc.index == GetCodeIndex(funcCode.code.tuneCmd)) 
        || (curFc.index == GetCodeIndex(funcCode.code.motorFcM2.tuneCmd)) 
        || (curFc.index == GetCodeIndex(funcCode.code.motorFcM3.tuneCmd)) 
        || (curFc.index == GetCodeIndex(funcCode.code.motorFcM4.tuneCmd)) )
        )
    {
        menuLevel = MENU_LEVEL_0;
    }
    else
    {
        menuLevel = MENU_LEVEL_2;
        menuAttri[MENU_LEVEL_2].operateDigit = 0;
    }
}



// È·ÈÏµ÷Ð³ÃüÁîÊÇ·ñÓÐÐ§
Uint16 ValidateTuneCmd(Uint16 value, Uint16 motorsN)
{
    if (motorsN != motorSn)       // ÊÇ·ñÎªµ±Ç°Ñ¡Ôñµç»ú
    {
        return COMM_ERR_PARA;
    }
    
    if (MOTOR_TYPE_PMSM == motorFc.motorPara.elem.motorType)    // pmsm
    {
        if ((FUNCCODE_tuneCmd_PMSM_11 != value) &&
            (FUNCCODE_tuneCmd_PMSM_12 != value) &&
            (FUNCCODE_tuneCmd_PMSM_13 != value)
            )
        {
            return COMM_ERR_PARA;
        }
    }
    else        // Òì²½»ú
    {
        if ((FUNCCODE_tuneCmd_ACI_STATIC != value) &&
            (FUNCCODE_tuneCmd_ACI_WHOLE != value)
            )
        {
            return COMM_ERR_PARA;
        }
    }

    if ((errorCode == ERROR_NONE) && (FUNCCODE_runSrc_PANEL == runSrc))       // ½öÃæ°åÃüÁîÍ¨µÀ£¬²ÅÄÜµ÷Ð³
    {
        tuneCmd = value;
        return COMM_ERR_NONE;
    }
    else
    {
        return COMM_ERR_PARA;
    }
}



LOCALF void Menu3OnUpDown(Uint16 flag)
{
    int16 delta;
    Uint16 flag1 = 1;

    if (curFc.group >= FUNCCODE_GROUP_U0) // U×é£¬ÏÔÊ¾
    {   // U×é£¬3¼¶²Ëµ¥ÏÂ£¬UP/DOWN×Ô¶¯µ½next grade£¬Í¬Ê±ÐèÒª¸üÐÂcurrentIndex
        Menu2OnUpDown(flag);
        curFc.index = GetGradeIndex(curFc.group, curFc.grade);

        flag1 = 0;
    }
#if DEBUG_F_USER_MENU_MODE
    else if (USER_MENU_GROUP == curFc.group)     // FE×é£¬ÓÃ»§¶¨ÖÆ¹¦ÄÜÂë
    {
        if (2 == menuAttri[MENU_LEVEL_3].operateDigit)
        {
            Uint16 group;
            Uint16 funcCodeGrade[FUNCCODE_GROUP_NUM];   // ¶ÑÕ»¹»ÓÃ
            
            memcpy(funcCodeGrade, funcCodeGradeAll, (FUNCCODE_GROUP_NUM));          
            funcCodeGrade[FC_GROUP_FACTORY] = 0;   // ÓÃ»§¶¨ÖÆ²Ëµ¥²»¿ÉÉèÖÃFF×é
            funcCodeGrade[USER_MENU_GROUP] = 0;  // ÓÃ»§¶¨ÖÆ²Ëµ¥²»¿ÉÉèÖÃCC×é

#if DEBUG_F_MOTOR_FUNCCODE
            MotorDebugFcDeal();
            funcCodeGrade[FUNCCODE_GROUP_CF] = motorDebugFc.fc;
            funcCodeGrade[FUNCCODE_GROUP_UF] = motorDebugFc.u;
#endif

            group = GroupUpDown(funcCodeGrade, menu3Number / 100, flag);
            menu3Number = group * 100; // ¸Ä±ä×éÊ±£¬gradeÇåÁã

            flag1 = 0;
        }
    }
#endif

    if (flag1)  // ÕæµÄÐèÒªUP/DOWN
    {
        // Ê®½øÖÆÔ¼Êø
        if (ATTRIBUTE_MULTI_LIMIT_HEX != funcCodeAttribute[curFc.index].attribute.bit.multiLimit)
        {
            delta = decNumber[menuAttri[MENU_LEVEL_3].operateDigit];
        }
        else    // ¶à¸ö¹¦ÄÜÂë£¬Ê®Áù½øÖÆ
        {
            delta = hexNumber[menuAttri[MENU_LEVEL_3].operateDigit];
        }

        if (ON_DOWN_KEY == flag)
            delta = -delta;

        ModifyFunccodeUpDown(curFc.index, &menu3Number, delta);
    }
}


LOCALF void Menu3OnShift(void)
{
    Uint16 max = curFcDispDigits;
    
#if DEBUG_F_USER_MENU_MODE
    if (USER_MENU_GROUP == curFc.group)     // FE×é£¬ÓÃ»§¶¨ÖÆ¹¦ÄÜÂë
    {
        max = 3;
    }
#endif

    if (!menuAttri[MENU_LEVEL_3].operateDigit)
        menuAttri[MENU_LEVEL_3].operateDigit = max - 1;
    else
        menuAttri[MENU_LEVEL_3].operateDigit--;
}


//=====================================================================
//
// ¹¦ÄÜÂëÏÞ·ù´¦Àí
//
// ÊäÈë:
// signal--- ÐèÒªÏÞ·ù´¦ÀíµÄ¹¦ÄÜÂëµÄsignal£¬
// data  --- ÐÞ¸ÄÖ®Ç°µÄÊý¾Ý
// upper --- ¹¦ÄÜÂëÉÏÏÞ
// lower --- ¹¦ÄÜÂëÏÂÏÞ
// delta --- delta>0, UP; delta<0, DOWN; delta==0, ¼ì²âµ±Ç°ÖµÊÇ·ñÔÚÏÞÖµ·¶Î§ÄÚ
//
// ·µ»Ø£ºÐÞ¸ÄÖ®ºóµÄÖµ
//
// ×¢Òâ£º
// ÔÚÓÐ·ûºÅÊ±£¬Òª¿¼ÂÇÒÔÏÂÇé¿ö£º
//   upper = 30000, data = +25000, delta = +10000   ==> data = 30000
// ÔÚÎÞ·ûºÅÊ±£¬Òª¿¼ÂÇÒÔÏÂÇé¿ö£º
//   lower = 5,     data = 4,       delta = -0      ==> data = 5
//   lower = 5,     data = 6,       delta = -10     ==> data = 5
//   upper = 65535, data = 60000,   delta = +10000  ==> data = 65535
//
//=====================================================================
Uint16 LimitDeal(Uint16 signal, Uint16 data, Uint16 upper, Uint16 lower, int16 delta)
{
    int32 data1, upper1, lower1;

    if (signal) // ÓÐ·ûºÅ
    {
        data1 = (int32)(int16)data;
        upper1 = (int32)(int16)upper;
        lower1 = (int32)(int16)lower;
    }
    else        // ÎÞ·ûºÅ
    {
        data1 = (int32)data;
        upper1 = (int32)upper;
        lower1 = (int32)lower;
    }

// Ïàµ±ÓÚ¶ÔÔ­ÖµÒ²½øÐÐ±È½ÏÁË
    data1 += delta;
    if (data1 > upper1)         // ÉÏÏÞ´¦Àí
    {
        data1 = upper1;
    }
    if (data1 < lower1)         // ÏÂÏÞ´¦Àí
    {
        data1 = lower1;
    }

    return (Uint16)data1;
}


//=====================================================================
//
// Ä³Ð©¹¦ÄÜÂëµÄÉè¶¨²»ÄÜÏàÍ¬¡£Ä¿Ç°280ÓÐDI¶Ë×Ó£¬320»¹ÓÐÖ÷¸¨ÆµÂÊÔ´
//
// ÊäÈë£º
// index        -- ÐèÒª½øÐÐ´¦Àí¹¦ÄÜÂëµÄindex
// funcIndex[]  -- ÕâÐ©Éè¶¨²»ÄÜÏàÍ¬¹¦ÄÜÂëµÄindexÊý×é
// number       -- funcIndex[]µÄ³¤¶È
// data         -- µ±Ç°¹¦ÄÜÂë£¬ÏëÉè¶¨µÄÖµ
// upper        -- ¹¦ÄÜÂëÉÏÏÞ
// lower        -- ¹¦ÄÜÂëÏÂÏÞ
// delta        -- ¹¦ÄÜÂëÖµµ±Ç°¿ÉÒÔÔö¼ÓµÄdelta¡£ÕýÊý--UP, ¸ºÊý--DOWN£¬0--½öÅÐ¶ÏÊÇ·ñÓëÆäËû¹¦ÄÜÂëÉè¶¨ÏàÍ¬
//
// ·µ»Ø:
//      delta²»Îª0Ê±£¬·µ»ØÓëÆäËû¹¦ÄÜÂëÉè¶¨²»Í¬µÄÊý¾Ý¡£
//      deltaÎª0Ê±£¬ÏàÍ¬Ôò·µ»Ø0£¬·ñÔò·µ»ØÊäÈëÖµ¡£
//
//=====================================================================
Uint16 NoSameDeal(Uint16 index, const Uint16 funcIndex[], int16 number, int16 data, int16 upper, int16 lower, int16 delta)
{
#if DEBUG_F_NO_SAME
    Uint16 i;

#if 1
    // ·­×ª
    if ((data == upper) && (delta > 0))
    {
        data = lower;       // 0(ÊÇÎÞ¹¦ÄÜ£¬)¿ÉÒÔÖØ¸´¡£
    }
    else if ((data == lower) && (delta < 0))
    {
        data = upper;       // upper²»¿ÉÒÔÖØ¸´£¬»¹Òª½øÐÐ´¦Àí¡£
    }
    else
#endif
    {
        data = LimitDeal(0, data, upper, lower, delta); // Ä¿Ç°¶¼ÊÇÎÞ·ûºÅ
    }

    if (data == lower)      // 0ÊÇÎÞ¹¦ÄÜ£¬¿ÉÒÔÖØ¸´¡£·µ»Ø¡£
        return data;

    for (;;)
    {
        for (i = 0; i < number; i++)    // ¶ÔËùÓÐ²»ÄÜÖØ¸´µÄindex
        {
            if ((funcIndex[i] != index) && (data == funcCode.all[funcIndex[i]])) // ÈôÓëÆäËû¹¦ÄÜÂëÉè¶¨ÏàÍ¬
            {
                if (delta > 0)
                {
                    data += 1;              // Óöµ½ÖØ¸´£¬×Ô¶¯¼Ó1(Ò²Ðí¿ÉÒÔ¸ÄÎªdelta)Ìø¹ý¡£
                    if (data > upper)       // Èô¼Ó1Ö®ºó³¬¹ý×î´óÖµ£¬±äÎª×îÐ¡Öµ¡£
                    {
                        data = lower;       // 0ÊÇÎÞ¹¦ÄÜ£¬¿ÉÒÔÖØ¸´
                    }
                }
                else if (delta < 0)
                {
                    data -= 1;              // Óöµ½ÖØ¸´£¬×Ô¶¯¼õ1(Ò²Ðí¿ÉÒÔ¸ÄÎªdelta)Ìø¹ý
                    if (data < lower)       // Èô¼õ1Ö®ºóÐ¡ÓÚ×îÐ¡Öµ£¬±äÎª×î´óÖµ¡£
                    {
                        data = upper;       // ×î´óÖµ¼ÌÐø±È½Ï
                    }
                }
                else
                {
                    data = lower;
                }

                break;
            }
        }

        if ((i >= number) ||    // Ò»Ö±Ã»ÓÐÖØ¸´
            (data == lower)     // Óöµ½ÖØ¸´£¬×Ô¶¯¼Ó1Ìø¹ý¡£Èô¼Ó1Ö®ºó³¬¹ý×î´óÖµ£¬¸ÄÎª×îÐ¡Öµ¡£
            )
            break;
    }

    return data;
#endif
}


//=====================================================================
//
// 0¼¶²Ëµ¥ÏÔÊ¾µÄÑ­»·ÒÆÎ»º¯Êý¡£
// ÊäÈë:
//      flag -- 1 °´¼üshiftÖ®ºó£¬ µ÷ÓÃ±¾º¯Êý
//              0 ¸Ä±ä¹¦ÄÜÂëÖ®ºó£¬µ÷ÓÃ±¾º¯Êý
//
//
//=====================================================================
LOCALF void cycleShiftDeal(Uint16 flag)
{
    Uint16 *bit = &funcCode.code.dispParaStopBit;
    Uint32 para = funcCode.code.ledDispParaStop;
    Uint16 max = STOP_DISPLAY_NUM;

    if (runFlag.bit.run)
    {
        bit = &funcCode.code.dispParaRunBit;
        para = funcCode.code.ledDispParaRun1 + ((Uint32)funcCode.code.ledDispParaRun2 << 16);
        max = RUN_DISPLAY_NUM;
    }

    if (!para)  // ·ÀÖ¹ÉèÖÃÎª0
    {
        para = 1;
    }

    if ((!flag) && (para & (0x1UL << *bit)))    // ¸Ä±ä¹¦ÄÜÂë£¬ÇÒµ±Ç°ÏÔÊ¾bitÈÔÈ»ÒªÏÔÊ¾
        return;

    do
    {
        if (++(*bit) >= max)
        {
            if (errorCode)  // ÓÐ¹ÊÕÏ
            {
                menu0DispStatus = MENU0_DISP_STATUS_ERROR;  // ½øÈë¹ÊÕÏ/¸æ¾¯ÏÔÊ¾×´Ì¬
            }

            if (tuneCmd)
            {
                menu0DispStatus = MENU0_DISP_STATUS_TUNE;   // ½øÈëµ÷Ð³ÏÔÊ¾
            }

            *bit = 0;
        }
    }
    while (!(para & (0x1UL << *bit)));
}


//=====================================================================
//
// ·­×ª´¦Àíº¯Êý
//
// ÊäÈë:
//      data  -- ¿ÉÄÜÐèÒª·­×ªµÄÊý¾Ý
//      upper -- ÉÏÏÞ
//      lower -- ÏÂÏÞ
//      flag  -- UP/DOWN±êÖ¾
//
// ·µ»Ø£º
//      ·­×ªÖ®ºóµÄÖµ¡£
//
//=====================================================================
LOCALF Uint16 OverTurnDeal(Uint16 data, Uint16 upper, Uint16 lower, Uint16 flag)
{
    if (ON_UP_KEY == flag)
    {
        if (upper == data)
        {
            data = lower;
        }
    }
    else
    {
        if (lower == data)
        {
            data = upper;
        }
    }

    return data;
}

//=====================================================================
//
// 0¼¶²Ëµ¥ÏÂµÄÏÔÊ¾»º³å¸üÐÂº¯Êý, 12msµ÷ÓÃ1´Î
// ÔËÐÐ/Í£»úÊ±LEDÏÔÊ¾£¬ÕýÔÚÐÞ¸ÄÉè¶¨ÆµÂÊÊ±£¬²»ÏÔÊ¾Êý¾Ý×îÇ°ÃæµÄ0
// ÏÔÊ¾µÄÉÁË¸£¬ÔÝÊ±²»Ê¹ÓÃÂË²¨
//
//=====================================================================
LOCALF void UpdateMenu0DisplayBuffer(void)
{
    Uint16 menu0Number;             // 0¼¶²Ëµ¥ÏÔÊ¾µÄÖµ
    union FUNC_ATTRIBUTE attributeMenu0;

    UpdateMenu0DispStatus();

    if (MENU0_DISP_STATUS_RUN_STOP == menu0DispStatus)
    {
        if (bFrqDigital || bFrqDigitalDone4WaitDelay)   // ÕýÔÚÐÞ¸ÄÉè¶¨ÆµÂÊ, »òÕßÐÞ¸ÄÖ®ºó»¹ÔÚÒ»¶¨Ê±¼äÄÚ
        {
            if (!bFrqDigital)
            {
                if (++accDecFrqTicker >= UP_DOWN_DEAL_DONE_TIME / 12)     // _*12ms. ÔÚ0¼¶²Ëµ¥°´¼üUP/DOWN¸Ä±äÆµÂÊÍê³Éºó£¬ÔÝÍ£Ò»»á
                {
                    accDecFrqTicker = 0;
                    bFrqDigitalDone4WaitDelay = 0;
                }
            }

            menu0Number = frqAimDisp;
            attributeMenu0 = funcCodeAttribute[MAX_FRQ_INDEX].attribute;    // ÏÔÊ¾ÊôÐÔÓëMAX_FRQÒ»ÖÂ
            attributeMenu0.bit.point = funcCode.code.frqPoint;
//                menuAttri[MENU_LEVEL_0].winkFlag = 0;        // ÔÚ0¼¶²Ë¥ÏÂ°´ÏÂUP/DOWNÊ±, Ö®ºó_Ê±¼äÒ²²»ÉÁË¸
        }
        else
        {
            Uint16 bitDisp;

            if (runFlag.bit.run) // ÔËÐÐÊ±LEDÏÔÊ¾
            {
                bitDisp = funcCode.code.dispParaRunBit;
            }
            else                      // Í£»úÊ±LEDÏÔÊ¾
            {
                if (funcCode.code.dispParaStopBit >= STOP_DISPLAY_NUM)
                {
                    funcCode.code.dispParaStopBit = 0;
                }
                bitDisp = stopDispIndex[funcCode.code.dispParaStopBit];

                menuAttri[MENU_LEVEL_0].winkFlag = 0xf8; // 0¼¶²Ëµ¥ÏÂÍ£»úÊ±È«ÉÁË¸
            }

            attributeMenu0 = dispAttributeU0[bitDisp];

            // ÔËÐÐÆµÂÊ¡¢Éè¶¨ÆµÂÊÏÔÊ¾£¬Ð¡Êýµã
            if ((DISP_FRQ_RUN == bitDisp) 
                || (DISP_FRQ_AIM == bitDisp)
                || (DISP_FRQ_RUN_FDB == bitDisp)
                )
            {
                attributeMenu0.bit.point = funcCode.code.frqPoint;
            }
            // Ð¡Êýµã
            else if (DISP_OUT_CURRENT == bitDisp)  // Êä³öµçÁ÷
            {
                if (invPara.type > invPara.pointLimit)
                    attributeMenu0.bit.point--;
            }
            else if (DISP_LOAD_SPEED == bitDisp) // ¸ºÔØËÙ¶ÈÏÔÊ¾
            {
                attributeMenu0.bit.point = funcCode.code.speedDispPointPos; // ËÙ¶ÈÏÔÊ¾Ð¡ÊýµãÎ»
            }
            
            menu0Number = funcCode.group.u0[bitDisp];          
            
        }

        UpdateDisplayBufferAttribute(menu0Number, attributeMenu0);
    }
    else if (MENU0_DISP_STATUS_ERROR == menu0DispStatus)    // ¹ÊÕÏ/¸æ¾¯ÏÔÊ¾
    {
        UpdateErrorDisplayBuffer();
    }
    else if (MENU0_DISP_STATUS_TUNE == menu0DispStatus)     // µ÷Ð³ÏÔÊ¾
    {
        UpdateTuneDisplayBuffer();
    }
}


// ¸üÐÂ0¼¶²Ëµ¥ÏÔÊ¾×´Ì¬
void UpdateMenu0DispStatus(void)
{
    static Uint16 errorCodeOld4Menu0;
    static Uint16 tuneCmdOld4Menu0;

    if ((!errorCodeOld4Menu0)
        && errorCode
        )       // ´ÓÎÞ¹ÊÕÏµ½ÓÐ¹ÊÕÏ
    {
        menu0DispStatus = MENU0_DISP_STATUS_ERROR;  // ½øÈë¹ÊÕÏ/¸æ¾¯ÏÔÊ¾×´Ì¬
    }
    errorCodeOld4Menu0 = errorCode;

    if ((!tuneCmdOld4Menu0)
        && tuneCmd
        )
    {
        menu0DispStatus = MENU0_DISP_STATUS_TUNE;   // ½øÈëµ÷Ð³ÏÔÊ¾
    }
    tuneCmdOld4Menu0 = tuneCmd;

    if ((!errorCode)
        && (!tuneCmd)
        )
    {
        menu0DispStatus = MENU0_DISP_STATUS_RUN_STOP;
    }
}


//=====================================================================
//
// 1¼¶²Ëµ¥ÏÂµÄÏÔÊ¾»º³å¸üÐÂº¯Êý
//
//=====================================================================
LOCALF void UpdateMenu1DisplayBuffer(void) // 1¼¶²Ëµ¥ÏÔÊ¾£º@@@FX
{
    UpdateGroupDisplay(curFc.group);

// ÊýÂë¹ÜÏÔÊ¾
    displayBuffer[0] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[1] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[2] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[3] = DISPLAY_CODE[groupDisplay.dispF];
    displayBuffer[4] = DISPLAY_CODE[groupDisplay.disp0];

//    menuAttri[MENU_LEVEL_1].winkFlag = 0x08;    // ÉÁË¸×îºóÒ»Î»£¬ÓëMD320²»Í¬
}


//=====================================================================
//
// 2¼¶²Ëµ¥ÏÂµÄÏÔÊ¾»º³å¸üÐÂº¯Êý
//
//=====================================================================
LOCALF void UpdateMenu2DisplayBuffer(void) // ÏÔÊ¾ FX-XX
{
    Uint16 digit[5];
    Uint16 flag = 0;

    if ((MENU_MODE_CHECK == menuMode)
        && (checkMenuModeSerachNone)
        )
    {
        flag = 1;
    }
    else if ((MENU_MODE_USER == menuMode)
        && (0 == funcCode.code.userCustom[userMenuModeFcIndex])
        )
    {
        flag = 2;
    }

    if (!flag)
    {
        UpdateGroupDisplay(curFc.group);

        GetNumberDigit(digit, curFc.grade, DECIMAL);

        // ÊýÂë¹ÜÏÔÊ¾
        displayBuffer[0] = DISPLAY_CODE[groupDisplay.dispF];
        displayBuffer[1] = DISPLAY_CODE[groupDisplay.disp0];
        displayBuffer[2] = DISPLAY_CODE[DISPLAY_LINE];
        displayBuffer[3] = DISPLAY_CODE[digit[1]];
        displayBuffer[4] = DISPLAY_CODE[digit[0]];

        if (MENU_MODE_CHECK == menuMode)        // ·Ç³ö³§ÖµÄ£Ê½
        {
            displayBuffer[0] = DISPLAY_CODE[DISPLAY_c];// & DISPLAY_CODE[DISPLAY_DOT];
            displayBuffer[1] = DISPLAY_CODE[groupDisplay.dispF];
            displayBuffer[2] = DISPLAY_CODE[groupDisplay.disp0] & DISPLAY_CODE[DISPLAY_DOT];
        }
        else if (MENU_MODE_USER == menuMode)    // ÓÃ»§¶¨ÖÆÄ£Ê½
        {
            displayBuffer[0] = DISPLAY_CODE[DISPLAY_u];// & DISPLAY_CODE[DISPLAY_DOT];
            displayBuffer[1] = DISPLAY_CODE[groupDisplay.dispF];
            displayBuffer[2] = DISPLAY_CODE[groupDisplay.disp0] & DISPLAY_CODE[DISPLAY_DOT];
        }

        menuAttri[MENU_LEVEL_2].winkFlag = 0x01 << (3 + menuAttri[MENU_LEVEL_2].operateDigit);
    }
    else
    {
        // ÊýÂë¹ÜÏÔÊ¾
        if (1 == flag)
        {
            displayBuffer[0] = DISPLAY_CODE[DISPLAY_c] & DISPLAY_CODE[DISPLAY_DOT];
        }
        else
        {
            displayBuffer[0] = DISPLAY_CODE[DISPLAY_u] & DISPLAY_CODE[DISPLAY_DOT];
        }
        
        displayBuffer[1] = DISPLAY_CODE[DISPLAY_N];
        displayBuffer[2] = DISPLAY_CODE[DISPLAY_U];
        displayBuffer[3] = DISPLAY_CODE[DISPLAY_L];
        displayBuffer[4] = DISPLAY_CODE[DISPLAY_L];
    }
}


//=====================================================================
//
// 3¼¶²Ëµ¥ÏÂµÄÏÔÊ¾»º³å¸üÐÂº¯Êý
// ¸üÐÂdisplayBuffer[]
//
//=====================================================================
LOCALF void UpdateMenu3DisplayBuffer(void)
{
    union FUNC_ATTRIBUTE attribute;
    Uint16 tmp = menu3Number;

#if (DEBUG_F_DISP_DIDO_STATUS_SPECIAL)
// DIDO×´Ì¬Ö±¹ÛÏÔÊ¾
    if ((curFc.index == GetCodeIndex(funcCode.group.u0[DISP_DI_STATUS_SPECIAL1])) || 
        (curFc.index == GetCodeIndex(funcCode.group.u0[DISP_DO_STATUS_SPECIAL1]))
        )
    {
        Uint32 value;

        // DIÊäÈë×´Ì¬Ö±¹ÛÏÔÊ¾£¬DI1-DI10, VDI1-VDI5, AI1asDI-AI3asDI
        if (GetCodeIndex(funcCode.group.u0[DISP_DI_STATUS_SPECIAL1]) == curFc.index)
        {
            value = diStatus.a.all & 0x000FFFFF;
        }
        // DOÊäÈë×´Ì¬Ö±¹ÛÏÔÊ¾£¬FMR,RELAY1,RELAY2,DO1,DO2,VDO1-VDO5
        else if (GetCodeIndex(funcCode.group.u0[DISP_DO_STATUS_SPECIAL1]) == curFc.index)
        {
            value = doStatus.a.all & 0x000FFFFF;
        }
        
        UpdateDisplayBufferVisualIoStatus(value);
        
        return;
    }
// DI¹¦ÄÜ×´Ì¬Ö±¹ÛÏÔÊ¾
    else if ((curFc.index == GetCodeIndex(funcCode.group.u0[DISP_DI_FUNC_SPECIAL1])) ||
             (curFc.index == GetCodeIndex(funcCode.group.u0[DISP_DI_FUNC_SPECIAL2]))
             )
    {
        Uint16 high;        // ¸ß8Î»
        Uint32 low;         // µÍ32Î»

        // DI¹¦ÄÜ×´Ì¬Ö±¹ÛÏÔÊ¾1£¬diFunc1-diFunc40
        if (GetCodeIndex(funcCode.group.u0[DISP_DI_FUNC_SPECIAL1]) == curFc.index)
        {
            high = (diFunc.f2.all >> 1) & 0x00FF;
            low = ((diFunc.f2.all & 0x0001) << 31) + (diFunc.f1.all >> 1);
        }
        // DI¹¦ÄÜ×´Ì¬Ö±¹ÛÏÔÊ¾2£¬diFunc41-diFunc80
        else if (GetCodeIndex(funcCode.group.u0[DISP_DI_FUNC_SPECIAL2]) == curFc.index)
        {
            high = 0;
            low = diFunc.f2.all >> 9;
        }
        
        UpdateDisplayBufferVisualDiFunc(high, low);
        
        return;
    }
#endif

    if (FUNCCODE_GROUP_U0 == curFc.group)               // U0£¬ÏÔÊ¾
    {
        attribute = dispAttributeU0[curFc.grade];      
        //funcCode.all[curFc.index] = *pDispValueU0[curFc.grade]; // UpdateU0Data()ÖÐ¸üÐÂ
    }
    
#if DEBUG_F_POSITION_CTRL
    else if (FUNCCODE_GROUP_U0 + 1 == curFc.group)          // U1£¬ÏÔÊ¾
    {
        attribute = dispAttributeU1[curFc.grade];
    }
#endif
#if DEBUG_F_MOTOR_FUNCCODE
    else if (FUNCCODE_GROUP_UF == curFc.group)          // UF£¬ÏÔÊ¾
    {
        attribute.all = UF_VIEW_ATTRIBUTE;
    }
#endif
#if DEBUG_F_PLC_CTRL
    else if (FUNCCODE_GROUP_U0 + 3 == curFc.group)          // U3£¬ÏÔÊ¾
    {
        attribute.all = 0x1040;
    }
#endif
    else    // ¹¦ÄÜÂë×é£¬°üÀ¨U1×é
    {
        attribute = funcCodeAttribute[curFc.index].attribute;
    }

    // ·ÇÃæ°åÃüÁîÍ¨µÀ£¬µ÷Ð³¹¦ÄÜÂë²»ÉÁË¸¡£
    if (((TUNE_CMD_INDEX_1 == curFc.index) 
        || (TUNE_CMD_INDEX_2 == curFc.index)
        || (TUNE_CMD_INDEX_3 == curFc.index)
        || (TUNE_CMD_INDEX_4 == curFc.index)
        )
// 380Ä¿Ç°£¬²»¿ÉÍ¨Ñ¶µ÷Ð³
        && (FUNCCODE_runSrc_PANEL != runSrc)
        )
    {
        attribute.bit.writable = ATTRIBUTE_READ_ONLY_ANYTIME;
    }

    if (IsWritable(attribute))             // µ±Ç°¿ÉÐÞ¸Ä
    {
        menuAttri[MENU_LEVEL_3].winkFlag = 0x01 << (3 + menuAttri[MENU_LEVEL_3].operateDigit);
    }
    else                                    // µ±Ç°²»¿ÉÐÞ¸Ä
    {
        tmp = funcCode.all[curFc.index];    // ËæÊ±¸üÐÂÏÔÊ¾Îª¹¦ÄÜÂëµÄÖµ
    }

    
    // ×î½üÒ»´Î¹ÊÕÏÆµÂÊ
    if (GetCodeIndex(funcCode.code.errorScene3.elem.errorFrq) == curFc.index)
    {
        attribute.bit.point = funcCode.code.errorFrqUnit & 0x000F;
    }
    // µÚ¶þ´Î¹ÊÕÏÆµÂÊ
    else if (GetCodeIndex(funcCode.code.errorScene2.elem.errorFrq) == curFc.index)
    {
        attribute.bit.point = (funcCode.code.errorFrqUnit >> 4) & 0x000F;
    }
    // µÚÒ»´Î¹ÊÕÏÆµÂÊ
    else if (GetCodeIndex(funcCode.code.errorScene1.elem.errorFrq) == curFc.index)
    {
        attribute.bit.point = (funcCode.code.errorFrqUnit >> 8 ) & 0x000F;
    }
    // ¸ºÔØËÙ¶ÈÏÔÊ¾
    else if(GetCodeIndex(funcCode.group.u0[DISP_LOAD_SPEED]) == curFc.index)
    {
        attribute.bit.point = funcCode.code.speedDispPointPos;
    }  
#if DEBUG_ACC_DEC_TIME_POINT
    // ¼Ó¼õËÙÊ±¼äµÄÐ¡Êýµã
    else if ((GetCodeIndex(funcCode.code.accTime1) == curFc.index) ||
        (GetCodeIndex(funcCode.code.accTime2) == curFc.index)  ||
        (GetCodeIndex(funcCode.code.accTime3) == curFc.index)  ||
        (GetCodeIndex(funcCode.code.accTime4) == curFc.index)  ||
        (GetCodeIndex(funcCode.code.jogAccTime) == curFc.index)  ||
        (GetCodeIndex(funcCode.code.decTime1) == curFc.index) ||
        (GetCodeIndex(funcCode.code.decTime2) == curFc.index)  ||
        (GetCodeIndex(funcCode.code.decTime3) == curFc.index)  ||
        (GetCodeIndex(funcCode.code.decTime4) == curFc.index)  ||
        (GetCodeIndex(funcCode.code.jogDecTime) == curFc.index) 
        )
    {
        attribute.bit.point = funcCode.code.accDecTimeUnit;
    }
#endif    
// ÆµÂÊÖ¸Áîµ¥Î»
#if DEBUG_FRQ_POINT
    else if (((attribute.bit.point == 2)  // µ¥Î»Îª0.01Hz»òÊÇF4-12 ¶Ë×ÓUP/DNËÙÂÊ
        && (attribute.bit.unit == 1)) 
        || (GetCodeIndex(funcCode.code.diUpDownSlope)  == curFc.index)
        )
    {
        attribute.bit.point -= (2 - funcCode.code.frqPoint);
    }

#endif    
// ¶î¶¨µçÁ÷£¬¿ÕÔØµçÁ÷£¬¹ÊÕÏµçÁ÷£¬U0-04ÏÔÊ¾µçÁ÷
    else if ((GetCodeIndex(funcCode.code.motorParaM1.elem.ratingCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorParaM1.elem.zeroLoadCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.ratingCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.zeroLoadCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.ratingCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.zeroLoadCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.ratingCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.zeroLoadCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.errorScene3.elem.errorCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.code.errorScene2.elem.errorCurrent) == curFc.index) ||
        (GetCodeIndex(funcCode.group.u0[DISP_OUT_CURRENT]) == curFc.index)         
        )
    {
        if (invPara.type > invPara.pointLimit)
            attribute.bit.point--;
    }	    
// µç»ú²ÎÊý
    else if ((GetCodeIndex(funcCode.code.motorParaM1.elem.statorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorParaM1.elem.rotorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorParaM1.elem.leakInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorParaM1.elem.mutualInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorParaM1.elem.pmsmRs) == curFc.index) ||   // Í¬²½»ú¶¨×Óµç×è
        (GetCodeIndex(funcCode.code.motorParaM1.elem.pmsmLd) == curFc.index) ||   // Í¬²½»údÖáµç¸Ð
        (GetCodeIndex(funcCode.code.motorParaM1.elem.pmsmLq) == curFc.index) ||   // Í¬²½»úqÖáµç¸Ð
        
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.statorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.rotorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.leakInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.mutualInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.pmsmRs) == curFc.index) ||   // Í¬²½»ú¶¨×Óµç×è
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.pmsmLd) == curFc.index) ||   // Í¬²½»údÖáµç¸Ð
        (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.pmsmLq) == curFc.index) ||   // Í¬²½»úqÖáµç¸Ð
       
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.statorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.rotorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.leakInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.mutualInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.pmsmRs) == curFc.index) ||   // Í¬²½»ú¶¨×Óµç×è
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.pmsmLd) == curFc.index) ||   // Í¬²½»údÖáµç¸Ð
        (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.pmsmLq) == curFc.index) ||   // Í¬²½»úqÖáµç¸Ð
       
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.statorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.rotorResistance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.leakInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.mutualInductance) == curFc.index) ||
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.pmsmRs) == curFc.index) ||   // Í¬²½»ú¶¨×Óµç×è
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.pmsmLd) == curFc.index) ||   // Í¬²½»údÖáµç¸Ð
        (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.pmsmLq) == curFc.index)      // Í¬²½»úqÖáµç¸Ð
        )
    {
        if (invPara.type > invPara.pointLimit)
        {
            attribute.bit.point++;
        }
    }
    
    UpdateDisplayBufferAttribute(tmp, attribute);
}


//=====================================================================
//
// ¸ù¾ÝÊôÐÔ£¬¸üÐÂÏÔÊ¾Êý¾Ý»º³åµÄ£º¸ººÅ¡¢Î»Êý¡¢Ð¡ÊýµãºÍµ¥Î»
//
//=====================================================================
LOCALF void UpdateDisplayBufferAttribute(const Uint16 data, union FUNC_ATTRIBUTE attribute)
{
    static Uint16 digit[5];
    int16 i;
    Uint16 digits;              // dataµÄÎ»Êý
    Uint16 bMinus = 0;          // ÊÇ·ñÏÔÊ¾·ûºÅ±êÖ¾£¬1-ÒªÏÔÊ¾¸ººÅ
    Uint16 dataTmp;
    Uint16 a,b;
    Uint16 mode;

// 1. ÏÔÊ¾Î»ÊýºÍÏÔÊ¾¸ººÅ
    // ÓÐ·ûºÅ£¬ÇÒÎª¸ºÊý£¬ÐèÒªÏÔÊ¾¸ººÅ-
    if ((attribute.bit.signal) // ·ûºÅ£¬unsignal->0; signal->1.
        && ((int16)(data) < 0))
    {
        dataTmp = -(int16)(data);
        bMinus = 1;

         // ÎªÓÐ·ûºÅÊý¾ÝÇÒÖµÎª¸ºÇÒÏÔÊ¾Öµ³¬¹ý4Î»
        if ((attribute.bit.point) 
            && ((int16)data < (-9999))
            )
        {
            attribute.bit.point--;
            dataTmp = dataTmp/10;
        }
    }
    else
    {
        dataTmp = data;
    }

// »ñÈ¡Ã¿Ò»Î»µÄÏÔÊ¾Öµ
    a = digit[1];   // ±£´æ
    b = digit[0];

    mode = DECIMAL;
    if (ATTRIBUTE_MULTI_LIMIT_HEX == attribute.bit.multiLimit)  // Ê®Áù½øÖÆÔ¼Êø
    {
        mode = HEX;
    }
    digits = GetNumberDigit(digit, dataTmp, mode);

    if (ticker4LowerDisp)  // ·Ç0¼¶²Ëµ¥£¬»òÕß0¼¶²Ëµ¥ÏÂ×îºó2Î»µ½ÁË¸üÐÂÊ±¼ä£¬»òÕß0¼¶²Ëµ¥  °´¼üshift
    {
        digit[1] = a;
        digit[0] = b;
    }

// ÓÃ»§¶¨ÖÆ¹¦ÄÜÂë×éµÄÏÔÊ¾
#if DEBUG_F_USER_MENU_MODE
    // ÏÔÊ¾ uFx.yz
    if ((USER_MENU_GROUP == curFc.group) &&      // ÓÃ»§¶¨ÖÆ¹¦ÄÜÂë×é
        (MENU_LEVEL_3 == menuLevel))
    {
        Uint16 group;

        digit[4] = DISPLAY_u;

        group = menu3Number / 100;
        UpdateGroupDisplay(group);
        digit[3] = groupDisplay.dispF;
        digit[2] = groupDisplay.disp0;

        attribute.bit.point = 2;
    }
#endif

    if ((MENU_LEVEL_0 == menuLevel) // 0¼¶²Ëµ¥£¬²»ÏÔÊ¾Êý×ÖÇ°ÃæµÄÁã
        || ((MENU_LEVEL_3 == menuLevel) && (curFc.group >= FUNCCODE_GROUP_U0)) // U×é£¬ÏÔÊ¾
            )
    {
        if (mode == HEX)
        {
            if (attribute.bit.displayBits < DISPLAY_8LED_NUM)
            {
                // Ê®Áù½øÖÆÏÔÊ¾¼ÓÇ°×º H.
                digits = attribute.bit.displayBits + 1;
                digit[attribute.bit.displayBits] = DISPLAY_H;    // ÏÔÊ¾H
                attribute.bit.point = attribute.bit.displayBits; // ÏÔÊ¾H.
            }
        }

        if (++ticker4LowerDisp >= DECIMAL_DISPLAY_UPDATE_TIME)  // Ð¡Êýµãºó2Î»¸üÐÂÊ±¼ä£¬_*12ms
        {
            ticker4LowerDisp = 0;
        }
    }
    else //if (MENU_LEVEL_3 == menuLevel)    // 0£¬3¼¶²Ëµ¥²Å»áµ÷ÓÃ±¾º¯Êý
    {
        ticker4LowerDisp = 0;

        // Ê®Áù½øÖÆÊý¾Ý
        if (mode == HEX)
        {
            // ÓÐ¿ÕÓàÊýÂë¹Ü
            if (curFcDispDigits < DISPLAY_8LED_NUM)
            {
                // 3¼¶²Ëµ¥µÄÊ®Áù½øÖÆÊý¾ÝÏÔÊ¾¼ÓÇ°×º H.
                digits = curFcDispDigits + 1;          // Ôö¼ÓÒ»Î»ÏÔÊ¾Î»
                digit[curFcDispDigits] = DISPLAY_H;    // ÏÔÊ¾Êý¾ÝÎªH
                attribute.bit.point = curFcDispDigits; // ÏÔÊ¾Êý¾ÝÎªH.
            }
            // ÊýÂë¹ÜÏÔÊ¾ÒÑÎÞ¿ÕÓà£¬²»ÏÔÊ¾H.
            else
            {
                digits = curFcDispDigits;       // 3¼¶²Ëµ¥
            }
        }
        // Ê®½øÖÆÊý¾Ý
        else
        {
            digits = curFcDispDigits;          // 3¼¶²Ëµ¥
        }
    }
    
    if (attribute.bit.point >= digits)  // ÖÁÉÙÏÔÊ¾µ½Ð¡ÊýµãÎ»ÖÃ
    {
        digits = attribute.bit.point + 1;
    }

    for (i = DISPLAY_8LED_NUM - 1; i >= 0; i--) // ÏÔÊ¾Î»Êý
    {
        if (i < digits)
        {
            displayBuffer[(DISPLAY_8LED_NUM - 1) - i] = DISPLAY_CODE[digit[i]];
        }
        else
        {
            displayBuffer[(DISPLAY_8LED_NUM - 1) - i] = DISPLAY_CODE[DISPLAY_NULL];
        }
    }
    
    if (bMinus)                 // ÏÔÊ¾¸ººÅ-
    {
        int16 tmp = DISPLAY_8LED_NUM - 1 - digits;
        if (tmp < 0)
            tmp = 0;

        if (!digit[4])  // ×î¸ßÎ»Îª0
        {
            displayBuffer[tmp] = DISPLAY_CODE[DISPLAY_LINE];
        }
        else
        {
            ///displayBuffer[(DISPLAY_8LED_NUM - 1)] &= DISPLAY_CODE[DISPLAY_DOT];// ×îºó1Î»LEDÏÔÊ¾Ð¡Êýµã
            //displayBuffer[tmp] = DISPLAY_CODE[DISPLAY_LINE_1];
            //displayBuffer[tmp] &= 0xdf;
            //displayBuffer[tmp] ^= 0x40;
            //displayBuffer[tmp] ^= ~0xBf;
            //displayBuffer[tmp] ^= 0x02;
            attribute.bit.point = 0;
        }
    }


// 2. ÏÔÊ¾Ð¡Êýµã
    if (attribute.bit.point)
    {
        displayBuffer[(DISPLAY_8LED_NUM - 1) - attribute.bit.point] &= DISPLAY_CODE[DISPLAY_DOT];
    }

// 3. ÏÔÊ¾µ¥Î»
    if (attribute.bit.unit & (0x01U << ATTRIBUTE_UNIT_HZ_BIT))   // ÏÔÊ¾µ¥Î»,Hz
    {
        displayBuffer[5] &= LED_CODE[LED_HZ];   // ÁÁ
    }

    if (attribute.bit.unit & (0x01U << ATTRIBUTE_UNIT_A_BIT))    // ÏÔÊ¾µ¥Î»,A
    {
        displayBuffer[5] &= LED_CODE[LED_A];
    }

    if (attribute.bit.unit & (0x01U << ATTRIBUTE_UNIT_V_BIT))    // ÏÔÊ¾µ¥Î»,V
    {
        displayBuffer[5] &= LED_CODE[LED_V];
    }
}


//=====================================================================
//
// ¹ÊÕÏÂëÏÔÊ¾
//
//=====================================================================
void UpdateErrorDisplayBuffer(void)
{
    Uint16 digit[5];

    GetNumberDigit1(digit, errorCode);

        // ÊýÂë¹ÜÏÔÊ¾
    displayBuffer[0] = DISPLAY_CODE[DISPLAY_E];
    displayBuffer[1] = DISPLAY_CODE[DISPLAY_r];
    displayBuffer[2] = DISPLAY_CODE[DISPLAY_r];
    displayBuffer[3] = DISPLAY_CODE[digit[1]];
    displayBuffer[4] = DISPLAY_CODE[digit[0]];

    if ((runFlag.bit.run)    // ¸æ¾¯
        || (ERROR_LEVEL_RUN == errorAttribute.bit.level)
        )
    {
        displayBuffer[0] = DISPLAY_CODE[DISPLAY_NULL];
        displayBuffer[1] = DISPLAY_CODE[DISPLAY_NULL];
        displayBuffer[2] = DISPLAY_CODE[DISPLAY_A];
    }
}


//=====================================================================
//
// µ÷Ð³ÏÔÊ¾
//
//=====================================================================
LOCALF void UpdateTuneDisplayBuffer(void)
{
    // ÊýÂë¹ÜÏÔÊ¾
    displayBuffer[0] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[1] = DISPLAY_CODE[DISPLAY_T];
    displayBuffer[2] = DISPLAY_CODE[DISPLAY_U];
    displayBuffer[3] = DISPLAY_CODE[DISPLAY_N];
    displayBuffer[4] = DISPLAY_CODE[DISPLAY_E];
}


//=====================================================================
//
// ÊäÈë£º
// *data  ÒªÐÞ¸ÄµÄÊý¾ÝµÄµ±Ç°Öµ
//
// Êä³ö£º
// *data  ÐÞ¸ÄÖ®ºóµÄÊý¾Ý
//
// ²ÎÊý£º
// index  ÐÞ¸Ä¹¦ÄÜÂëµÄindex
// delta  ÔöÁ¿¡£ÕýÊý£¬UP£»¸ºÊý£¬DOWN£»0£¬Í¨Ñ¶µ÷ÓÃ¡£
//
// ·µ»Ø£º
// COMM_ERR_NONE
// COMM_ERR_PARA       ÎÞÐ§²ÎÊý
// COMM_ERR_READ_ONLY  ²ÎÊý¸ü¸ÄÎÞÐ§
//
//=====================================================================
Uint16 ModifyFunccodeUpDown(Uint16 index, Uint16 *data, int16 delta)
{
    union FUNC_ATTRIBUTE attribute = funcCodeAttribute[index].attribute;
    Uint16 upper;
    Uint16 lower;
    Uint16 tmp = *data;
    Uint16 flag = COMM_ERR_NONE;
    int16 i;
    int16 flag1 = 0;
    Uint16 multiLimit;

// ²ÎÊý¶ÁÐ´ÌØÐÔ£¬0x-²ÎÊýÖ»¶Á£¬10-ÔËÐÐÖÐÖ»¶Á£¬11-¿ÉÒÔ¶ÁÐ´
    if (IsWritable(attribute))
    {
#if DEBUG_F_USER_MENU_MODE
        if ((USER_MENU_GROUP == curFc.group)     // FE×é£¬ÓÃ»§¶¨ÖÆ¹¦ÄÜÂë
		|| ((index >= GetCodeIndex(funcCode.code.userCustom[0])) 
			&& (index <= GetCodeIndex(funcCode.code.userCustom[FENUM-1])))
		)
        {
            Uint16 group = *data / 100;
			
			if ((group >= FUNCCODE_GROUP_NUM) || (group == USER_MENU_GROUP))
			{
				return COMM_ERR_PARA;
			}
			
            upper = group * 100 + funcCodeGradeAll[group] - 1;
            lower = group * 100 + 0;

			if (upper < lower)
			{
				return COMM_ERR_PARA;
			}
        }
        else
#endif
        {
            upper = funcCodeAttribute[index].upper;
            if (attribute.bit.upperLimit)
                upper = funcCode.all[upper];

            // SÇúÏßµÄÆðÊ¼¶ÎºÍ½áÊø¶ÎÖ®ºÍ×î´óÎª1000.
            // sCurveStartPhaseTime + sCurveEndPhaseTime <= 100.0%
            if (index == GetCodeIndex(funcCode.code.sCurveStartPhaseTime))
            {
                upper = 1000 - funcCode.code.sCurveEndPhaseTime;
            }
            else if (index == GetCodeIndex(funcCode.code.sCurveEndPhaseTime))
            {
                upper = 1000 - funcCode.code.sCurveStartPhaseTime;
            }

            lower = funcCodeAttribute[index].lower;
            if (attribute.bit.lowerLimit)
                lower = funcCode.all[lower];

            // ¸Ã¹¦ÄÜÂëÎª¶à¸ö¹¦ÄÜÂëµÄ×éºÏ
            multiLimit = funcCodeAttribute[index].attribute.bit.multiLimit;

            if ((ATTRIBUTE_MULTI_LIMIT_SINGLE != multiLimit) && (!delta))  // ×éºÏ¹¦ÄÜÂëÇÒÍ¨Ñ¶µ÷ÓÃ
            {
                Uint16 dataDigit[5],upperDigit[5], lowerDigit[5];
                Uint16 bit;
               // const int16 *p = decNumber;
                Uint16 mode = DECIMAL;

                if (ATTRIBUTE_MULTI_LIMIT_HEX == multiLimit)
                {
                    //p = hexNumber;
                    mode = HEX;
                }

				GetNumberDigit(dataDigit, *data, mode);
				GetNumberDigit(upperDigit, upper, mode);
				GetNumberDigit(lowerDigit, lower, mode);

                for (bit = 0; bit < 5; bit++)
                {
                    if ((dataDigit[bit] > upperDigit[bit]) 
                    || (dataDigit[bit] < lowerDigit[bit]))
					{
                        return COMM_ERR_PARA;
					}
                }

                return COMM_ERR_NONE;
            }
                
            
            if ((ATTRIBUTE_MULTI_LIMIT_SINGLE != multiLimit)   // ×éºÏ¹¦ÄÜÂëÇÒ·ÇÍ¨Ñ¶µ÷ÓÃ
                && (delta)
                )
            {
                Uint16 digit[5];
                Uint16 tmp;
                Uint16 bit = menuAttri[MENU_LEVEL_3].operateDigit;
                const int16 *p = decNumber;
                Uint16 mode = DECIMAL;

                if (ATTRIBUTE_MULTI_LIMIT_HEX == multiLimit)
                {
                    p = hexNumber;
                    mode = HEX;
                }

                GetNumberDigit(digit, *data, mode);
                tmp = *data - digit[bit] * (*(p+bit));

                GetNumberDigit(digit, upper, mode);
                upper = tmp + digit[bit] * (*(p+bit));

                GetNumberDigit(digit, lower, mode);
                lower = tmp + digit[bit] * (*(p+bit));
            }

#if DEBUG_F_INV_TYPE_RELATE
            // »úÐÍ²»ÄÜ³¬¹ý·¶Î§
            if (GetCodeIndex(funcCode.code.inverterType) == index)  // FF-01
            {
                if ((100 != delta) && (-100 != delta))   // Ã»ÓÐÔÚÐÞ¸Ä°ÙÎ»
                {
                    Uint16 i;
                    i = *data / 100;

                    upper = invTypeLimitTable[i].upper + (i * 100);
                    lower = invTypeLimitTable[i].lower + (i * 100);
                }
            }
            else 
#endif
            // ×î´óÆµÂÊ£¬²»Í¬ÆµÂÊÖ¸ÁîÐ¡ÊýµãµÄ·¶Î§Ó¦¸Ã²»Í¬¡£
            if (GetCodeIndex(funcCode.code.maxFrq) == index)
            {
                lower = 50 * decNumber[funcCode.code.frqPoint];
            }
        }

// ÉÏÏÂÏÞ´¦Àí
#if DEBUG_F_NO_SAME
//--------------------------------------------------------
// NoSameDeal()
        // Ö÷ÆµÂÊÔ´XºÍ¸¨ÆµÂÊÔ´YµÄÉè¶¨Öµ²»ÄÜÒ»Ñù
        for (i = sizeof(frqSrcFuncIndex) - 1; i >= 0; i--)
        {
            if (frqSrcFuncIndex[i] == index)
            {
                *data = NoSameDeal(index, frqSrcFuncIndex, sizeof(frqSrcFuncIndex), *data, upper, lower, delta);
                flag1 = 1;
            }

            if (flag1)
                break;
        }

        // DI(DI, VDI, AiAsDi)¶Ë×ÓµÄ¹¦ÄÜ£¬²»ÄÜÖØ¸´
        for (i = sizeof(diFuncIndex) - 1; i >= 0; i--)
        {
            if (diFuncIndex[i] == index)
            {
                *data = NoSameDeal(index, diFuncIndex, sizeof(diFuncIndex), *data, upper, lower, delta);
                flag1 = 1;
            }

            if (flag1)  // ÓÐÏàµÈµÄindex£¬Ìø³ö
                break;
        }
//--------------------------------------------------------
#endif
        if (!flag1)
        {
            *data = LimitDeal(attribute.bit.signal, *data, upper, lower, delta);
        }

        if (*data != tmp)       // Í¨Ñ¶µ÷ÓÃÊ±£¬·¢ÏÖÊý¾Ý±»¸ü¸Ä£¬¼´²ÎÊý´íÎó¡£
            flag = COMM_ERR_PARA;
    }
    else
    {
        flag = COMM_ERR_READ_ONLY;
    }

    return flag;
}




//=====================================================================
//
// ²ÎÊý£º
// index  ÒªÐÞ¸Ä¹¦ÄÜÂëµÄindex
// *data  Êý¾Ý
//
// ·µ»Ø£º
// COMM_ERR_NONE
// COMM_ERR_PARA       ÎÞÐ§²ÎÊý
//
//=====================================================================
Uint16 ModifyFunccodeEnter(Uint16 index, Uint16 dataNew)
{
    Uint16 dataOld;
    Uint16 ret = COMM_ERR_NONE;

    dataOld = funcCode.all[index];
    funcCode.all[index] = dataNew;        // ±£´æµ½RAM


    if (dataOld != dataNew)
    {
        // ½øÈëÁËÄ³Ð©¹¦ÄÜÂë£¬»¹ÒªÐÞ¸ÄÒ»Ð©±äÁ¿
        // F0-07, ÆµÂÊÔ´Ñ¡Ôñ
        // F0-08, Ô¤ÖÃÆµÂÊ
        if ((GetCodeIndex(funcCode.code.presetFrq) == index) ||
            (GetCodeIndex(funcCode.code.frqCalcSrc) == index))
        {
            // Êý×ÖÉè¶¨ÆµÂÊÔ´£¬ÐÞ¸ÄÁËÔ¤ÖÃÆµÂÊ£¬ÒªÏàÓ¦ÐÞ¸ÄÉè¶¨ÆµÂÊ¡£½øÈëÔ¤ÖÃÆµÂÊºóenter¾ÍÐÞ¸Ä
            ResetUpDownFrq();
        }
#if DEBUG_F_MOTOR_POWER_RELATE
        else if (GetCodeIndex(funcCode.code.motorParaM1.elem.ratingPower) == index)         // ÐÞ¸Äµç»ú¶î¶¨¹¦ÂÊ£¬Á¢¼´ÐÞ¸ÄÏà¹Ø¹¦ÄÜÂë
        {
            MotorPowerRelatedParaDeal(dataNew, MOTOR_SN_1);
        }
        else if (GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.ratingPower) == index) // ÐÞ¸Äµç»ú¶î¶¨¹¦ÂÊ£¬Á¢¼´ÐÞ¸ÄÏà¹Ø¹¦ÄÜÂë
        {
            MotorPowerRelatedParaDeal(dataNew, MOTOR_SN_2);
        }
        else if (GetCodeIndex(funcCode.code.motorFcM3.motorPara.elem.ratingPower) == index) // ÐÞ¸Äµç»ú¶î¶¨¹¦ÂÊ£¬Á¢¼´ÐÞ¸ÄÏà¹Ø¹¦ÄÜÂë
        {
            MotorPowerRelatedParaDeal(dataNew, MOTOR_SN_3);
        }
        else if (GetCodeIndex(funcCode.code.motorFcM4.motorPara.elem.ratingPower) == index) // ÐÞ¸Äµç»ú¶î¶¨¹¦ÂÊ£¬Á¢¼´ÐÞ¸ÄÏà¹Ø¹¦ÄÜÂë
        {
            MotorPowerRelatedParaDeal(dataNew, MOTOR_SN_4);
        }   
#endif
        else if (GetCodeIndex(funcCode.code.menuMode) == index)  // ÐÞ¸Ä²Ëµ¥²Ù×÷Ä£Ê½
        {
            Uint16 digit[5];
            GetNumberDigit(digit, menu3Number, DECIMAL);
        		menuModeTmp = menuMode;
        		// µ±Ç°²Ëµ¥²Ù×÷Ä£Ê½Ñ¡ÔñÎÞÐ§ºó
        		if (((menuModeTmp == MENU_MODE_USER) && (!digit[0]))
                  || ((menuModeTmp == MENU_MODE_CHECK) && (!digit[1])))
        		{
        			// ¸ü¸Äµ±Ç°²Ëµ¥²Ù×÷Ä£Ê½ÎªÒÑÓÐÑ¡ÔñÄ£Ê½
        			menuModeTmp = MENU_MODE_BASE;
        			MenuModeSwitch();
        		}
        }
        // ¸üÐÂ¹¦ÄÜ²ÎÊý×éÏÔÊ¾Òþ²ØÊôÐÔ
        else if ((GetCodeIndex(funcCode.code.aiaoCalibrateDisp) == curFc.index)
            || (GetCodeIndex(funcCode.code.funcParaView) == curFc.index))
        {
            MenuModeDeal();
        }
#if DEBUG_F_INV_TYPE_RELATE
        else if (GetCodeIndex(funcCode.code.inverterType) == index) // ÐÞ¸Ä±äÆµÆ÷»úÐÍ FF-01£¬Á¢¼´ÐÞ¸ÄÏà¹Ø¹¦ÄÜÂë
        {
            // »úÐÍ²»ÄÜ³¬¹ý·¶Î§
            ret = ValidateInvType();
            if (!ret)  // »úÐÍÓÐÐ§
            {
                InverterTypeRelatedParaDeal();
            }
        }
#endif
#if 0
        else if (GetCodeIndex(funcCode.code.commOverTime) == index) // FA-04, Í¨Ñ¶³¬Ê±Ê±¼ä
        {
            if ((dataNew) && (!dataOld))  // 0->·Ç0
                commTicker = 0;  // ÐÞ¸ÄÁËÍ¨Ñ¶ÑÓÊ±Ê±¼ä£¬ÖØÐÂ¿ªÊ¼¼ÆÊ±¡£
        }
#endif
    }

    // ½öDI5¿ÉÒÔ¶¨ÒåÎªDI_FUNC_APTP_ZERO
    if ((GetCodeIndex(funcCode.code.diFunc[0]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[1]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[2]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[3]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[5]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[6]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[7]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[8]) == index)
        || (GetCodeIndex(funcCode.code.diFunc[9]) == index)
        )
    {
        if (dataNew == DI_FUNC_APTP_ZERO)
        {
            ret = COMM_ERR_PARA;
        }
    }
#if DEBUG_F_GROUP_HIDE
    else if (GetCodeIndex(funcCode.code.funcParaView) == curFc.index)
    {
        MenuModeDeal();
    }
#endif
#if DEBUG_F_MOTOR_FUNCCODE1
    // ÐÔÄÜµ÷ÊÔ¹¦ÄÜÂë
    else if (GetCodeIndex(funcCode.code.motorDebugFc) == curFc.index)
    {
        MenuModeDeal();
    }
#endif
    // AIAOÐ£Õý¹¦ÄÜÂëÒþ²Ø
    else if (GetCodeIndex(funcCode.code.aiaoCalibrateDisp) == curFc.index)
    {
        MenuModeDeal();
    }
    else if ((GetCodeIndex(funcCode.code.ledDispParaRun1) == index) ||
        (GetCodeIndex(funcCode.code.ledDispParaRun2) == index) ||
        (GetCodeIndex(funcCode.code.ledDispParaStop) == index)
        )
    {
        cycleShiftDeal(0);      // 0¼¶²Ëµ¥ÏÔÊ¾µÄÑ­»·ÒÆÎ»´¦Àí
    }
    // ÆµÂÊÖ¸ÁîÐ¡Êýµã
    else if (GetCodeIndex(funcCode.code.frqPoint) == index)
    {
        if (funcCode.code.maxFrq < 50 * decNumber[funcCode.code.frqPoint])
        {
            funcCode.code.maxFrq = 50 * decNumber[funcCode.code.frqPoint];

            // Ä³Ð©¹¦ÄÜÂëÊÇÆäËû¹¦ÄÜÂëÉÏÏÂÏÞµÄ´¦Àí
            LimitOtherCodeDeal(MAX_FRQ_INDEX);   // ×î´óÆµÂÊ
        }
    }
#if (DEBUG_F_POSITION_CTRL)
    // aptpÁãµãÊäÈë¶Ë×Ó£¬Ê¹ÓÃÖÐ¶Ï
    // ÈÏÎªDI5ÎªHDI
    else if (GetCodeIndex(funcCode.code.diFunc[4]) == index)
    {
        // ¸Ä±ä³ÉaptpÁãµãÊäÈë
        if ((dataOld != DI_FUNC_APTP_ZERO) && (dataNew == DI_FUNC_APTP_ZERO))
        {
            InitSetEcap4WithInt();
            InitSetAptpZero();
        }
        else if ((dataOld == DI_FUNC_APTP_ZERO) && (dataNew != DI_FUNC_APTP_ZERO))
        {
            ;
        }
    }
    // FVCµÄPG¿¨Ñ¡Ôñ, 1-QEP1,0-QEP2(À©Õ¹)
    // ÕâÀï½ö½øÐÐÎ»ÖÃ¿ØÖÆPGµÄ³õÊ¼»¯
    else if (GetCodeIndex(funcCode.code.pgParaM1.elem.fvcPgSrc) == index)
    {
        if ((dataOld != FUNCCODE_fvcPgSrc_QEP1) && (dataNew == FUNCCODE_fvcPgSrc_QEP1))
        {
            //InitEQep2Gpio();
            aptpAbsZeroOk = 0;
            pEQepRegsFvc = &EQep1Regs;   // FVCµÄPG¿¨

            EALLOW;
            pEQepRegsFvc->QEPCTL.bit.IEL = 01;      // FVC QEPx Index event latch
            EDIS;

            InitSetPcEQep();
        }
        else if ((dataOld == FUNCCODE_fvcPgSrc_QEP1) && (dataNew != FUNCCODE_fvcPgSrc_QEP1))
        {
            //InitEQep1Gpio();
            aptpAbsZeroOk = 0;
            pEQepRegsPc = &EQep1Regs;    // Î»ÖÃ¿ØÖÆµÄPG¿¨

            EALLOW;
            pEQepRegsFvc->QEPCTL.bit.IEL = 01;      // FVC QEPx Index event latch
            EDIS;

            InitSetPcEQep();
        }
    }
    // Î»ÖÃÖ¸ÁîÂö³åÂß¼­
    else if (GetCodeIndex(funcCode.code.pcPulseLogic) == index)
    {
        // ½«QEP_PC-B·´Ïò
        pEQepRegsPc->QDECCTL.bit.QBP = dataNew;
    }
    // Î»ÖÃÖ¸ÁîÂö³å·½Ê½
    else if (GetCodeIndex(funcCode.code.pcPulseType) == index)
    {
        if (FUNCCODE_pcPulseType_PULSE_AND_DIR == funcCode.code.pcPulseType)    // Âö³å+·½Ïò
        {
            pEQepRegsPc->QDECCTL.bit.QSRC = 01; // Direction-count mode (QCLK = xCLK, QDIR = xDIR)
        }
        else if (FUNCCODE_pcPulseType_QUADRATURE == funcCode.code.pcPulseType)  // 2Â·Õý½»Âö³å
        {
            pEQepRegsPc->QDECCTL.bit.QSRC = 00; // quadrature count mode
        }
    }
    // Î»ÖÃÖ¸ÁîÏàÐò
    else if (GetCodeIndex(funcCode.code.pcPulseSwap) == index)
    {
        pEQepRegsPc->QDECCTL.bit.SWAP = dataNew;
    }
    // ËÙ¶È·´À¡ABÏàÐò
#if 0    // ABÏàÐò½»¸øÐÔÄÜ´¦Àí
    else if (GetCodeIndex(funcCode.code.fvcPgLogic) == index)
    {
        // ½«QEP_ËÙ¶È¿ØÖÆ-AB½»»»
        pEQepRegsFvc->QDECCTL.bit.SWAP = dataNew;
    }
#endif
    // ¸Ä±äÁãµã, aptpAbsZeroOkÇåÁã
    else if (GetCodeIndex(funcCode.code.pcZeroSelect) == index)
    {
        if (dataOld != dataNew)
        {
            aptpAbsZeroOk = 0;
        }
    }
#endif
    else
    {
        LimitOtherCodeDeal(index);      // Ä³Ð©¹¦ÄÜÂëÊÇÆäËû¹¦ÄÜÂëÉÏÏÂÏÞµÄ´¦Àí
    }

    if (COMM_ERR_PARA == ret)           // ¸ü¸ÄÎÞÐ§£¬Ôò»Ö¸´Ö®Ç°µÄÖµ
    {
        funcCode.all[index] = dataOld; // »Ö¸´
    }

    return ret;
}


//=====================================================================
//
// ÏÔÊ¾Êý¾Ý´¦Àí£¬Ä¿Ç°2msµ÷ÓÃ1´Î
//
//=====================================================================
#define OUT_TORQUE_FRQ_DISP_FILTER_TIME 75  // Êä³öÆµÂÊÂË²¨Ê±¼äÏµÊý
LOCALF LowPassFilter torQueFrqDispLpf = LPF_DEFALUTS;
void DispDataDeal(void)
{
// ÔËÐÐÆµÂÊÏÔÊ¾¡£
// ÏÔÊ¾¹¦ÄÜ´«µÝ¸øÐÔÄÜµÄË²Ê±Öµ£¬¶ø²»ÊÇÐÔÄÜµÄ·´À¡ÆµÂÊ(»òÕßÐÔÄÜÊµ¼Ê·¢ËÍµÄÆµÂÊ)
    frqDisp = ABS_INT32(frqDroop);

#if DEBUG_F_PLC_CTRL
    frqPLCDisp = (int16)(frqDroop*10000/maxFrq);
#endif


    if (RUN_MODE_TORQUE_CTRL == runMode)    // ×ª¾Ø¿ØÖÆ
    {
        torQueFrqDispLpf.t = OUT_TORQUE_FRQ_DISP_FILTER_TIME;
        torQueFrqDispLpf.in = ABS_INT32(frqRun);
        torQueFrqDispLpf.calc(&torQueFrqDispLpf);
        frqDisp = torQueFrqDispLpf.out;

#if DEBUG_F_PLC_CTRL
        frqPLCDisp = (int16)(torQueFrqDispLpf.out*10000/maxFrq);
#endif
    }

#if DEBUG_F_PLC_CTRL
     if (funcCode.code.runDir == FUNCCODE_runDir_REVERSE)
     {
        frqPLCDisp = -((int16)frqPLCDisp);
     }
#endif    

// Éè¶¨ÆµÂÊÏÔÊ¾¡£ÌøÔ¾ÆµÂÊÖ®Ç°£¬µã¶¯Ö®ºóµÄÖµ
// ·ÅÔÚUpdateFrqAim()ÖÐ¡£

    pidFuncRefDisp = ((Uint32)funcCode.code.pidDisp * ABS_INT32(pidFunc.ref) + (1 << 14)) >> 15;
    pidFuncFdbDisp = ((Uint32)funcCode.code.pidDisp * ABS_INT32(pidFunc.fdb) + (1 << 14)) >> 15;

    // ¸ºÔØËÙ¶È
    // Í£»úÊ±ÏÔÊ¾Éè¶¨ÆµÂÊ*ÏµÊý
    // ÔËÐÐÊ±ÏÔÊ¾ÔËÐÐÆµÂÊ*ÏµÊý
    if (runFlag.bit.run)
    {
        loadSpeedDisp = (ABS_INT32(frq) * funcCode.code.speedDispCoeff) / 10000;
    }
    else
    {
        loadSpeedDisp = (ABS_INT32(frqAim) * funcCode.code.speedDispCoeff) / 10000;
    }
    
    pulseInFrqDisp = (pulseInFrq + 5) / 10;

    torqueCurrentAct = (Uint32)torqueCurrent * currentPu >> 12;
     
// Êä³ö¹¦ÂÊ¼ÆËã£¬ÏÖÔÚÓÉÇý¶¯Ö±½Ó¼ÆËã
#if 0//DEBUG_F_OUT_POWER
    //+============ Ð¡Êýµã
    outPower = (Uint32)torqueCurrentAct * outVoltageDisp * 135 / (100UL * 1000);   // µ¥Î»Îª0.1KW£¬Ð§ÂÊÄ¬ÈÏÎª1.35/1.732
    itDisp = (Uint32)torqueCurrentAct * 1000 / motorFc.motorPara.elem.ratingCurrent;       // Êä³ö×ª¾Ø
#endif

    funcCode.code.inverterGpTypeDisp = funcCode.code.inverterGpType; // F0-00, GPÀàÐÍÏÔÊ¾

//  ËÙ¶È·´À¡PG¿¨ABÏàÐò£¬ÐÔÄÜ×ö
//    pEQepRegsFvc->QDECCTL.bit.SWAP = funcCode.code.fvcPgLogic;
}


#if 0
//=====================================================================
//
// »Ö¸´³ö³§²ÎÊý£¬»Ö¸´ÖÁRAM
//
//=====================================================================
void RestoreCompanyParaRamDeal(Uint16 i)
{
    Uint16 flag = 0;    // 0, »Ö¸´

    if (indexRestoreExceptSeries < RESTORE_COMPANY_PARA_EXCEPT_NUMBER)
    {
        if ((exceptRestoreSeries[indexRestoreExceptSeries].start <= i) &&
            (i <= exceptRestoreSeries[indexRestoreExceptSeries].end))
        {
            flag = 1;
        }
        if (i >= exceptRestoreSeries[indexRestoreExceptSeries].end)
        {
            indexRestoreExceptSeries++;
        }
    }

    if (indexRestoreExceptSingle < CLEAR_RECORD_NUM)   // ÕâÐ©¹¦ÄÜÂë²»»Ö¸´
    {
        if (clearRecord[indexRestoreExceptSingle] == i)
        {
            flag = 1;
            indexRestoreExceptSingle++;
        }
    }

    if (!flag)
    {
        funcCode.all[i] = funcCodeInit.all[i]; // »Ö¸´³É³ö³§²ÎÊý
    }
}
#endif


// 0¼¶²Ëµ¥ÏÂÔö¼Ó²Ëµ¥¼¶±ð
void Menu0AddMenuLevel(void)
{
    menuLevel = MENU_LEVEL_1;

    if ((MENU_MODE_CHECK == menuMode)
        || (MENU_MODE_USER == menuMode)
        )
    {
        Menu1OnEnter();
    }
}


//=====================================================================
//
// È·¶¨¹¦ÄÜÂëµÄÏÔÊ¾Î»Êý
//
// Êµ¼ÊÏÔÊ¾Î»Êý£¬Ó¦¸ÃÎª:
// 1. ÎÞ·ûºÅ
//    ÉÏÏÞÊýÖµµÄÎ»Êý
// 2. ÓÐ·ûºÅ
//    ÉÏÏÞºÍÏÂÏÞÊýÖµµÄ¾ø¶ÔÖµµÄ´óÖµµÄÎ»Êý
//
//=====================================================================
Uint16 GetDispDigits(Uint16 index)
{
    Uint16 upper;
    Uint16 lower;
    Uint16 value;
    Uint16 digits;
    Uint16 mode;
    Uint16 digit[5];
    const FUNCCODE_ATTRIBUTE *p = &funcCodeAttribute[index];

// »ñµÃÊµ¼ÊµÄÉÏÏÞ

// »ñÈ¡ÉÏÏÞÊýÖµ
    while (p->attribute.bit.upperLimit)
    {
        p = &funcCodeAttribute[p->upper];
    }
    upper = p->upper;
    
    if (!p->attribute.bit.signal)   // ÎÞ·ûºÅ
    {
        value = upper;
    }
    else                            // ÓÐ·ûºÅ
    {
        // »ñÈ¡ÏÂÏÞÊýÖµ
        while (p->attribute.bit.lowerLimit)
        {
            p = &funcCodeAttribute[p->lower];
        }
        lower = p->lower;

        // È¡¾ø¶ÔÖµ
        upper = ABS_INT16((int16)upper);
        lower = ABS_INT16((int16)lower);

        // È¡¾ø¶ÔÖµµÄ´óÖµ
        value = (upper > lower) ? upper : lower;
    }

// »ñÈ¡ÉÏÏÞµÄÎ»Êý
    mode = DECIMAL;

    if (ATTRIBUTE_MULTI_LIMIT_HEX == p->attribute.bit.multiLimit)   // Ê®Áù½øÖÆÔ¼Êø
    {
        mode = HEX;
    }

    digits = GetNumberDigit(digit, value, mode);

    return digits;
}


//=====================================================================
//
// ¸ù¾ÝcurrentGroup£¬¸üÐÂÏÔÊ¾µÄ×é£¬¼´groupDisplay
// F0,¡­,FE,FF,FP,A0,¡­,AF,B0,¡­,BF,C0,¡­,CF,
//
//=====================================================================
void UpdateGroupDisplay(Uint16 group)
{
    Uint16 currentGroupDispF;
    Uint16 currentGroupDisp0;

// F0,¡­,FE,FF,FP,A0,¡­,AF,B0,¡­,BF,C0,¡­,CF,
    currentGroupDispF = DISPLAY_F;  //stamp:MD380_DISPLAY
    //currentGroupDispF = DISPLAY_E;    //
    if (group <= FUNCCODE_GROUP_FP)         // F0-FP
    {
        currentGroupDisp0 = group;

    }
    else if (group <= FUNCCODE_GROUP_CF)    // A0,¡­,AF,B0,¡­,BF,C0,¡­,CF,
    {
        Uint16 tmp1;
        Uint16 tmp2;

        tmp1 = (group - FUNCCODE_GROUP_FP - 1) / 16;
        tmp2 = (group - FUNCCODE_GROUP_FP - 1) % 16;

      currentGroupDispF = DISPLAY_A + tmp1; //MD380_DISPLAY
        //currentGroupDispF = DISPLAY_F + tmp1;//
        currentGroupDisp0 = tmp2;
    }
    else    // U×é
    {
      currentGroupDispF = DISPLAY_U; //MD380_DISPLAY
        //currentGroupDispF = DISPLAY_P;//stamp:
        currentGroupDisp0 = group - FUNCCODE_GROUP_U0;
    }

    groupDisplay.dispF = currentGroupDispF;  
    groupDisplay.disp0 = currentGroupDisp0;  //MD380_DISPLAY

	//if((groupDisplay.dispF == DISPLAY_E)&&(groupDisplay.disp0 == 15)) // ×¢ÊÍµô»Ö¸´ md380 
	//{
       // groupDisplay.dispF = DISPLAY_H;
       // groupDisplay.disp0 = 17;  //display HH
	//}
}


// groupµÄUP/DOWN
Uint16 GroupUpDown(const Uint16 funcCodeGrade[], Uint16 group, Uint16 flag)
{
    group = LimitOverTurnDeal(funcCodeGrade, group, FUNCCODE_GROUP_NUM, FC_START_GROUP, flag);

    return group;
}


// ÓÃ»§¶¨ÖÆ¹¦ÄÜÂë²Ëµ¥Ä£Ê½µÄ´¦Àí
void DealUserMenuModeGroupGrade(Uint16 flag)
{
#if DEBUG_F_USER_MENU_MODE
    userMenuModeFcIndex = LimitOverTurnDeal(funcCode.code.userCustom, userMenuModeFcIndex, FENUM, 0, flag);

// ¸öÎ»¡¢Ê®Î»±íÊ¾ÓÃ»§¶¨ÖÆµÄgrade
// °ÙÎ»¡¢Ç§Î»±íÊ¾ÓÃ»§¶¨ÖÆµÄgroup
    curFc.group = funcCode.code.userCustom[userMenuModeFcIndex] / 100;
    curFc.grade = funcCode.code.userCustom[userMenuModeFcIndex] % 100;
#endif
}


// dataµÄ·¶Î§ÊÇ [low, upper-1]
Uint16 LimitOverTurnDeal(const Uint16 limit[], Uint16 data, Uint16 upper, Uint16 low, Uint16 flag)
{
    Uint16 loopNumber = 0;
    
    do
    {
        data += deltaK[flag];

        if ((int16)data >= (int16)upper)
        {
            data = low;
        }
        else if ((int16)data < (int16)low)
        {
            data = upper - 1;
        }

        loopNumber++;
    }
    while ((!limit[data]) 
        && (loopNumber < upper - low)
        );

    return data;
}


// Ð£Ñé²Ëµ¥
#define CHECK_MENU_MODE_NUMBER_ONCE     70      // 1ÅÄÄÚµÄÑ­»·´ÎÊý
void DealCheckMenuModeGroupGrade(Uint16 flag)
{
#if DEBUG_F_CHECK_MENU_MODE
    int16 delta = 1;
    static Uint16 group;
    static Uint16 grade;
    Uint16 gradeTmp;
    Uint16 loopNumber = 0;
    Uint16 checkMenuModeFcIndex;

    if (CHECK_MENU_MODE_DEAL_CMD == checkMenuModeDealStatus)   // ÐÂµÄËÑË÷Ö¸Áî
    {
        group = curFc.group;
        grade = curFc.grade;
    }
    checkMenuModeDealStatus = CHECK_MENU_MODE_DEAL_CMD;

    if (ON_DOWN_KEY == flag)    // ÏòÏÂËÑË÷
    {
        delta = -delta;
    }

    do
    {
        gradeTmp = grade;
        if (funcCodeGradeCurMenuMode[group] > 1)    // ¸ÃgroupÓÐÐ§
        {
            grade = LimitDeal(0, grade, funcCodeGradeCurMenuMode[group] - 1, 0, delta); // funcCodeGradeAll
        }

        if (grade == gradeTmp)                      // gradeµ½´ïÏÞÖµ£¬Òª¸Ä±ägroup
        {
            group = LimitOverTurnDeal(funcCodeGradeCurMenuMode, group, FUNCCODE_GROUP_U0 - 1, FUNCCODE_GROUP_F0, flag);

            if (ON_UP_KEY == flag)
            {
                grade = 0;
            }
            else
            {
                grade = funcCodeGradeCurMenuMode[group] - 1;    // ¸ÃgroupµÄ×îºóÒ»¸ögrade
            }
        }

        checkMenuModeFcIndex = GetGradeIndex(group, grade);

        // 1ÅÄÄÚ²»ÄÜÕ¼ÓÃÌ«¶àÊ±¼ä
        // ·ñÔò£¬ÏÂÒ»ÅÄÔÙ½øÐÐËÑË÷
        if (++loopNumber >= CHECK_MENU_MODE_NUMBER_ONCE)        
        {
            checkMenuModeDealStatus = CHECK_MENU_MODE_DEAL_SERACHING;
        }

        if ((curFc.grade == grade)      // if Ö®Ç°Ã»ÓÐelse
            && (curFc.group == group)
            )   // ÓÖ»Øµ½Ö®Ç°µÄ¹¦ÄÜÂë£¬ËµÃ÷Ã»ÓÐÆäËüÓë³ö³§Öµ²»Í¬µÄ¹¦ÄÜÂë
        {
            checkMenuModeDealStatus = CHECK_MENU_MODE_DEAL_END_NONE;   // ±éÀúÒ»±é£¬Ã»ÓÐÕÒµ½ÐÂµÄÓë³ö³§Öµ²»Í¬µÄ¹¦ÄÜÂë
            checkMenuModeSerachNone = 1;
        }
        
        // ¿¼ÂÇ³ö³§ÖµÓë»úÐÍÏà¹ØµÄ·Çµç»ú²ÎÊý¹¦ÄÜÂë
        // if Ö®Ç°Ã»ÓÐelse
        if ((funcCode.all[checkMenuModeFcIndex] != GetFuncCodeInit(checkMenuModeFcIndex, 0)) &&
            (funcCodeAttribute[checkMenuModeFcIndex].attribute.bit.writable != 2))
        {
            checkMenuModeDealStatus = CHECK_MENU_MODE_DEAL_END_ONCE;  // ÕÒµ½ÐÂµÄÓë³ö³§Öµ²»Í¬µÄ¹¦ÄÜÂë
            checkMenuModeSerachNone = 0;
        }
    }
    while (CHECK_MENU_MODE_DEAL_CMD == checkMenuModeDealStatus);

#if 0
    if (CHECK_MENU_MODE_DEAL_END_NONE == checkMenuModeDealStatus)  // Ã»ÓÐ¹¦ÄÜÂëÓë³ö³§Öµ²»Í¬£¬ÏÔÊ¾F0-00
    {
        group = 0;
        grade = 0;
    }
#endif

    if (CHECK_MENU_MODE_DEAL_SERACHING != checkMenuModeDealStatus)  // ÒÑ¾­Íê³ÉËÑË÷
    {
        curFc.group = group;
        curFc.grade = grade;

        checkMenuModeCmd = 0;       // Íê³ÉËÑË÷
        checkMenuModeDealStatus = CHECK_MENU_MODE_DEAL_CMD;         // ×¼±¸ÏÂÒ»´ÎËÑË÷
    }
#endif
}



// ²Ëµ¥Ä£Ê½´¦Àí
// ¸üÐÂfuncCodeGradeCurrentMenuMode
void MenuModeDeal(void)
{
    Uint16 digit[5];

    UpdataFuncCodeGrade(funcCodeGradeCurMenuMode);
    GetNumberDigit1(digit, funcCode.code.menuMode);

    switch (menuMode)
    {
        case MENU_MODE_BASE:       // »ù±¾²Ëµ¥£¬Ö÷ÒªÎªÄ¿Ç°320¹¦ÄÜÂë(³ö³§)
            break;       

#if DEBUG_F_USER_MENU_MODE
        case MENU_MODE_USER:        // ÓÃ»§¶¨ÖÆ²Ëµ¥
            if (MENU_MODE_USER != menuModeOld)
            {
                DealUserMenuModeGroupGrade(ON_UP_KEY);
            }
            
            curFc.group = funcCode.code.userCustom[userMenuModeFcIndex] / 100;
            curFc.grade = funcCode.code.userCustom[userMenuModeFcIndex] % 100;
            break;
#endif

#if DEBUG_F_CHECK_MENU_MODE
        case MENU_MODE_CHECK:        // Ð£¶Ô²Ëµ¥£¬½öÏÔÊ¾Óë³ö³§Öµ²»Í¬µÄ¹¦ÄÜÂë
            if (MENU_MODE_CHECK != menuModeOld)
            {
                checkMenuModeCmd = 1;
                checkMenuModePara = ON_UP_KEY;
            }
            break;
#endif
        default:
            break;
    }

}


void UpdataFuncCodeGrade(Uint16 funcCodeGrade[])
{
#if 1
    int16 i;
    Uint16 digit[5];

    memcpy(funcCodeGrade, funcCodeGradeAll, FUNCCODE_GROUP_NUM);
    
    GetNumberDigit(digit, funcCode.code.funcParaView, DECIMAL);  // µÃµ½Ã¿Î»µÄÖµ£¬0 or 1

    // U×éÏÔÊ¾ÊôÐÔ
    if(digit[0] == 0)
    {
        for (i = 0; i < 16; i++)
        {
            funcCodeGrade[FUNCCODE_GROUP_U0 + i] = 0;
        }
    }
    
    // A×éÏÔÊ¾ÊôÐÔ
    if(digit[1] == 0)
    {
        for (i = 0; i < 16; i++)
        {
            funcCodeGrade[FUNCCODE_GROUP_A0 + i] = 0;
        }
    }
    
    // B×éÏÔÊ¾ÊôÐÔ
    if(digit[2] == 0)
    {
        for (i = 0; i < 16; i++)
        {
            funcCodeGrade[FUNCCODE_GROUP_B0 + i] = 0;
        }
    }

    // C×éÏÔÊ¾ÊôÐÔ
    if(digit[3] == 0)
    {
        for (i = 0; i < 16; i++)
        {
            funcCodeGrade[FUNCCODE_GROUP_C0 + i] = 0;
        }
    }

    // ÐÔÄÜµ÷ÊÔÏÔÊ¾ÊôÐÔ
#if DEBUG_F_MOTOR_FUNCCODE
    MotorDebugFcDeal();
    funcCodeGrade[FUNCCODE_GROUP_CF] = motorDebugFc.fc; // CF×é¸öÊý
    funcCodeGrade[FUNCCODE_GROUP_UF] = motorDebugFc.u;  // UF×é¸öÊý
#endif
    
    // AIAOÐ£Õý¹¦ÄÜÂëÏÔÊ¾ÊôÐÔ
    if (funcCode.code.aiaoCalibrateDisp)
    {
        funcCodeGrade[FUNCCODE_GROUP_AE] = AENUM;
    }
#endif

}


//====================================================================
// °´Î»(¸öÎ»¡¢Ê®Î»¡¢°ÙÎ»¡¢Ç§Î»¡¢ÍòÎ»)Þ¸ÄµÄÊ®½øÖÆ¹¦ÄÜÂë£¬×ª»»Îª¶þ½øÖÆ
// Ã¿Î»½öÄÜÎª0, 1
// ÀýÈç£¬¹¦ÄÜÂëA6-06(ÐéÄâVDI¶Ë×Ó¹¦ÄÜÂëÉè¶¨ÓÐÐ§×´Ì¬)£¬
// ¹¦ÄÜÂëÏÔÊ¾Îª 11101£¬×ª»»Îª 29¡£
//====================================================================
Uint16 FcDigit2Bin(Uint16 value)
{
    Uint16 tmp = 0;
    Uint16 digit[5];
    int16 i;

    GetNumberDigit(digit, value, DECIMAL);  // µÃµ½Ã¿Î»µÄÖµ£¬0 or 1

    for (i = 5-1; i >= 0; i--)
    {
        if (digit[i])       //if (1 == digit[i])    // Èç¹ûÎ»Îª1.
        {
            tmp += 1 << i;
        }
    }

    return tmp;
}

//====================================================================
//
// ÐÔÄÜµ÷ÊÔµÄ¹¦ÄÜÂë×éCF¡¢UF×éµÄ¸öÊý´¦Àí
//
//====================================================================
void MotorDebugFcDeal(void)
{
#if DEBUG_F_MOTOR_FUNCCODE
#if DEBUG_F_MOTOR_FUNCCODE1
    Uint16 fc;
    Uint16 u;

    fc = funcCode.code.motorDebugFc % 100;
    u = funcCode.code.motorDebugFc / 100;
    if (fc > CFNUM)
    {
        fc = CFNUM;
    }
    if (u > UFNUM)
    {
        u = UFNUM;
    }

    motorDebugFc.fc = fc;
    motorDebugFc.u = u;
#endif
#endif
}


//====================================================================
//
// ÉÏµçÊ±µÄ²Ëµ¥³õÊ¼»¯
//
//====================================================================
void MenuInit(void)
{
    menuAttri[MENU_LEVEL_0].winkFlag = 0x00F8;
	// Ä¬ÈÏ»áÑ¡ÔñµÚÒ»¼¶Ë÷Òý
    menuModeTmp = MENU_MODE_BASE;
    menuMode = menuModeTmp;
    MenuModeDeal();
    menuLevel = MENU_LEVEL_0;
}


#if DEBUG_F_DISP_DIDO_STATUS_SPECIAL
// ¸üÐÂDIDO×´Ì¬µÄÖ±¹ÛÏÔÊ¾
void UpdateDisplayBufferVisualIoStatus(Uint32 value)
{
    int16 i;
    Uint16 bit[4];

    for (i = 4; i >= 0; i--)
    {
        bit[0] = (value & (0x01 << 0)) >> 0;    // »ñµÃ0/1
        bit[1] = (value & (0x01 << 1)) >> 1;
        bit[2] = (value & (0x01 << 2)) >> 2;
        bit[3] = (value & (0x01 << 3)) >> 3;

        displayBuffer[i] = (bit[0] << 1) |      // 1±íÊ¾ÏÔÊ¾Î»ÖÃ£¬ÔÚÊýÂë¹ÜµÄÎ»ÖÃ
                           (bit[1] << 2) |      // 2±íÊ¾ÏÔÊ¾Î»ÖÃ£¬ÔÚÊýÂë¹ÜµÄÎ»ÖÃ
                           (bit[2] << 5) |
                           (bit[3] << 4) |
                           (~DISPLAY_CODE[DISPLAY_LINE]);
        displayBuffer[i] = ~displayBuffer[i];

        value >>= 4;
    }

    //displayBuffer[5] = LED_CODE[LED_NULL];
}


// DI¹¦ÄÜµÄÏÔÊ¾
// valueH, ¸ß 8Î»
// valueL, µÍ32Î»
void UpdateDisplayBufferVisualDiFunc(Uint16 valueH, Uint32 valueL)
{
    //displayBuffer[5] = LED_CODE[LED_NULL];
    
    displayBuffer[4] = ~(Uint16)((valueL >> 0)  & 0xff);
    displayBuffer[3] = ~(Uint16)((valueL >> 8)  & 0xff);
    displayBuffer[2] = ~(Uint16)((valueL >> 16) & 0xff);
    displayBuffer[1] = ~(Uint16)((valueL >> 24) & 0xff);
    displayBuffer[0] = ~(Uint16)((valueH >> 0)  & 0xff);
}
#endif



#if DEBUG_F_PASSWORD      // ÃÜÂë¡£°üÀ¨ÓÃ»§ÃÜÂë£¬Ö»¶ÁÓÃ»§ÃÜÂë£¬¹¦ÄÜÂë×éÒþ²ØÃÜÂë
void MenuPwdOnPrg(void)
{
    // Èç¹ûÕýÔÚ½øÐÐÃÜÂë¼ì²â£¬°´¼üPRGÍË»Øµ½0¼¶²Ëµ¥¡£
    menuLevel = MENU_LEVEL_0;       // ÖØÐÂ¸´Î»

#if DEBUG_F_GROUP_HIDE
    if (groupHidePwdStatus)         // ÖØÐÂ¸´Î»£¬»Øµ½2¼¶²Ëµ¥ÏÂ
    {
        groupHidePwdStatus = 0;
        menuLevel = MENU_LEVEL_2;
    }
#endif
}

// °´¼üENTER, UP, DOWN£¬¶¼½øÈëMENU_LEVEL_PWD_INPUT
LOCALD void MenuPwdHint2Input(void)
{
    menuLevel = MENU_LEVEL_PWD_INPUT;
    menuAttri[MENU_LEVEL_PWD_INPUT].operateDigit = 0;
    menuPwdNumber = 0;                    // ÃÜÂë³õÊ¼Îª0
}

LOCALD void MenuPwdInputOnEnter(void)
{
    {
        if ((menuPwdNumber == funcCode.code.userPassword) ||    // ÓÃ»§ÃÜÂë
            (menuPwdNumber == SUPER_USER_PASSWORD))             // ³¬¼¶ÃÜÂë
        {
            Menu0AddMenuLevel();
        }
        else
        {
            menuLevel = MENU_LEVEL_PWD_HINT;
        }
    }
}

// ÃÜÂëÌáÊ¾ÏÂ(-----)°´SHIFT¼üÏÔÊ¾ÃÜÂëÃ÷ÎÄ
LOCALD void MenuPwdHintOnShift(void)
{
#if DEBUG_RANDOM_FACPASS      
     facPassViewStatus= FAC_PASS_VIEW;

    // Éú³É³¬¼¶ÃÜÂë
    if (!factoryPwd)
    {
        Uint32 time;
        Uint16 pwd[4];
        time = GetTime();
        factoryPwd = (time >> 16) + (time & 0xFFFF);
        pwd[0] = factoryPwd>>8;
        pwd[1] = SUPER_USER_PASSWORD_SOURCE1&0xFF;
        pwd[2] = SUPER_USER_PASSWORD_SOURCE1>>8;
        pwd[3] = factoryPwd&0xFF;
        superFactoryPass = CrcValueByteCalc(pwd, 4);
    }
#else
    MenuPwdHint2Input();
#endif
}

LOCALD void MenuPwdInputOnUp(void)
{
    MenuPwdInputOnUpDown(ON_UP_KEY);
}



LOCALD void MenuPwdInputOnDown(void)
{
    MenuPwdInputOnUpDown(ON_DOWN_KEY);
}



void MenuPwdInputOnUpDown(Uint16 flag)
{
    int16 delta;

    delta = decNumber[menuAttri[MENU_LEVEL_PWD_INPUT].operateDigit];

    if (ON_DOWN_KEY == flag)
        delta = -delta;

    menuPwdNumber = LimitDeal(0, menuPwdNumber, 65535, 0, delta);
}



LOCALD void MenuPwdInputOnShift(void)
{
    if (menuAttri[MENU_LEVEL_PWD_INPUT].operateDigit == 0)
        menuAttri[MENU_LEVEL_PWD_INPUT].operateDigit = 4;
    else
        menuAttri[MENU_LEVEL_PWD_INPUT].operateDigit--;
}



LOCALD void UpdateMenuPwdHintDisplayBuffer(void)
{  
#if DEBUG_RANDOM_FACPASS    
    if (facPassViewStatus == FAC_PASS_VIEW)
    {
        // ÏÔÊ¾ÃÜÂëÃ÷ÎÄ
        Uint16 digit[5];
        GetNumberDigit(digit, factoryPwd, 0);
        displayBuffer[0] = DISPLAY_CODE[digit[4]];
        displayBuffer[1] = DISPLAY_CODE[digit[3]];
        displayBuffer[2] = DISPLAY_CODE[digit[2]];
        displayBuffer[3] = DISPLAY_CODE[digit[1]];
        displayBuffer[4] = DISPLAY_CODE[digit[0]];
        menuAttri[MENU_LEVEL_PWD_HINT].winkFlag = 0;
    }
    else
#endif        
    {
        // ÏÔÊ¾-----
        displayBuffer[0] = DISPLAY_CODE[DISPLAY_LINE];
        displayBuffer[1] = DISPLAY_CODE[DISPLAY_LINE];
        displayBuffer[2] = DISPLAY_CODE[DISPLAY_LINE];
        displayBuffer[3] = DISPLAY_CODE[DISPLAY_LINE];
        displayBuffer[4] = DISPLAY_CODE[DISPLAY_LINE];
        menuAttri[MENU_LEVEL_PWD_HINT].winkFlag = 0x08; // 320ÉÁË¸×îºóÒ»Î»
    }
    
}



LOCALD void UpdateMenuPwdInputDisplayBuffer(void)
{   // ÓÃ»§ÃÜÂëÊäÈë
    Uint16 digit[5];

    GetNumberDigit(digit, menuPwdNumber, DECIMAL);

// ÊýÂë¹ÜÏÔÊ¾
    displayBuffer[0] = DISPLAY_CODE[digit[4]];
    displayBuffer[1] = DISPLAY_CODE[digit[3]];
    displayBuffer[2] = DISPLAY_CODE[digit[2]];
    displayBuffer[3] = DISPLAY_CODE[digit[1]];
    displayBuffer[4] = DISPLAY_CODE[digit[0]];

    menuAttri[MENU_LEVEL_PWD_INPUT].winkFlag = 0x01U << (3 + menuAttri[MENU_LEVEL_PWD_INPUT].operateDigit);
}

#elif 1

LOCALD void MenuPwdOnPrg(void){}
LOCALD void MenuPwdHint2Input(void){}
LOCALD void MenuPwdInputOnEnter(void){}
LOCALD void MenuPwdInputOnUp(void){}
LOCALD void MenuPwdInputOnDown(void){}
LOCALD void MenuPwdInputOnShift(void){}
LOCALD void UpdateMenuPwdHintDisplayBuffer(void){}
LOCALD void UpdateMenuPwdInputDisplayBuffer(void){}

#endif

LOCALD void MenuPwdHintOnQuick(void)
{
}
LOCALD void MenuPwdInputOnQuick(void)
{
}




