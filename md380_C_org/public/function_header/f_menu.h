#ifndef __F_MENU_H__
#define __F_MENU_H__

#include "f_funcCode.h"

//=================================
enum MENU_MODE
{
    MENU_MODE_NULL,
    MENU_MODE_BASE,         // 1, 基本菜单，主要为目前320功能码(出厂)
    MENU_MODE_USER,         // 4, 用户定制菜单
    MENU_MODE_CHECK        // 5, 校验菜单，仅显示与出厂值不同的功能码
};
//=================================


Uint16 FcDigit2Bin(Uint16 value);



extern enum MENU_MODE menuModeTmp; 
extern enum MENU_MODE menuMode; 

extern Uint16 superFactoryPass;

enum MENU_MODE_OPERATE
{
    MENU_MODE_NONE,         // 
    MENU_MODE_ON_QUICK      // 按下QUICK键
};
extern enum MENU_MODE_OPERATE menuModeStatus;

// 厂家随机密码明文显示
enum FAC_PASS_RANDOM_VIEW_OPERATE
{
    FAC_PASS_NONE,    // 显示密码明文
    FAC_PASS_VIEW     // 不显示
};
extern enum FAC_PASS_RANDOM_VIEW_OPERATE facPassViewStatus;

#define DISP_OUT_CURRENT        4   //  4, 输出电流序号

#define DEBUG_RANDOM_FACPASS    0   // 随机厂家密码

#if F_DEBUG_RAM
#define COMPANY_PASSWORD        1430           // 厂家密码，功能调试时设置密码为1，方便调试
#else
#define COMPANY_PASSWORD        1430          // 厂家密码
#endif

#if DEBUG_RANDOM_FACPASS  
#define SUPER_USER_PASSWORD     superFactoryPass   // 超级用户密码
#else
#define SUPER_USER_PASSWORD     0                  // 超级用户密码
#endif
 
#define SUPER_USER_PASSWORD_SOURCE1    0      // 超级密码来源

#define ON_UP_KEY       ACC_SPEED
#define ON_DOWN_KEY     DEC_SPEED

#define STOP_DISPLAY_NUM 16     // 停机时，LED显示参数的总数
#define RUN_DISPLAY_NUM  32     // 运行时，LED显示参数的总数
#define COMM_PARA_NUM    33     // 通讯读取停机或运行显示参数的个数

#define USER_PARA_SAVE_FLAG1   1
#define USER_PARA_SAVE_FLAG2   0xFFFF - USER_PARA_SAVE_FLAG1

// 将菜单操作封装在一起
typedef struct tagSysMenu
{
    void (*onPrgFunc)();         // 在当前菜单按下 PRG   键的处理函数指针
    void (*onUpFunc)();          // 在当前菜单按下 UP    键的处理函数指针
    void (*onEnterFunc)();       // 在当前菜单按下 ENTER 键的处理函数指针
    void (*onMfkFunc)();         // 在当前菜单按下 MF.K  键的处理函数指针
    void (*onDownFunc)();        // 在当前菜单按下 DOWN  键的处理函数指针
    void (*onShiftFunc)();       // 在当前菜单按下 SHIFT 键的处理函数指针
    void (*onRunFunc)();         // 在当前菜单按下 RUN   键的处理函数指针
    void (*onStopFunc)();        // 在当前菜单按下 STOP  键的处理函数指针
    void (*onQuickFunc)();       // 在当前菜单按下 QUICK 键的处理函数指针

    void (*UpdateDisplayBuffer)(); // 当前菜单下更新显示数据缓冲的函数指针
}sysMenu, *sysMenuHandle;

extern const sysMenu menu[];

#define MENU_LEVEL_NUM  6   // 一共有_级菜单
enum MENU_LEVEL
{        
    MENU_LEVEL_0,           // 0级菜单
    MENU_LEVEL_1,           // 1级菜单
    MENU_LEVEL_2,           // 2级菜单
    MENU_LEVEL_3,           // 3级菜单
    MENU_LEVEL_PWD_HINT,    // 提示输入密码
    MENU_LEVEL_PWD_INPUT    // 输入密码
};
extern enum MENU_LEVEL menuLevel;


struct MENU_ATTRIBUTE
{
    Uint16 operateDigit;    // 各级菜单下，当前操作位. 0-4
    
    Uint16 winkFlag;        // 数码管闪烁控制寄存器, 对应位为1时闪烁.
// bit7,数码管左边第1个(最左边); bit3, 数码管左边第5个(最右边)

    Uint16 winkFlagLed;     // led闪烁控制寄存器. bit0-Led0; bit1-Led1,...
// 可以将winkFlag和winkFlagLed放在一起
};
extern struct MENU_ATTRIBUTE menuAttri[];


// 电压等级与额定电压的关系见 invTypeLimitTable[]
struct INV_PARA
{
    Uint16 type;                // 变频器机型
    
    Uint16 ratingVoltage;       // 变频器的额定电压
    Uint16 volLevel;            // 变频器的电压等级
    Uint16 pointLimit;          // 电流、电机参数的小数点
    Uint16 bitAccDecStart;      // 加减速时间出厂值为大值的起始机型
};
extern struct INV_PARA invPara;


enum MENU0_DISP_STATUS
{
    MENU0_DISP_STATUS_RUN_STOP,     // 运行/停机显示
    MENU0_DISP_STATUS_UP_DOWN,      // up/down时显示
    MENU0_DISP_STATUS_ERROR,        // 故障/告警显示
    MENU0_DISP_STATUS_TUNE          // 调谐显示
};
extern enum MENU0_DISP_STATUS menu0DispStatus;     // 0级菜单的显示状态

extern Uint16 accDecFrqPrcFlag;
extern Uint16 bFrqDigital;
extern Uint16 bFrqDigitalDone4WaitDelay;
extern Uint16 outCurrentDisp;
extern Uint16 outCurrentDispView;
extern Uint16 currentOcDisp;
extern Uint16 frqAimDisp;
extern Uint16 frqPLCDisp;       // PLC可编程卡读取运行
extern Uint16 frqAimPLCDisp;    // PLC卡编程卡读取设定频率
extern Uint16 outVoltageDisp;
extern Uint16 frqRunDisp;
extern Uint16 pcOriginDisp;

extern Uint16 itDisp;
extern Uint16 frqDisp;
extern Uint16 frqXDisp;
extern Uint16 frqYDisp;
extern Uint32 torqueCurrentAct;   // 转矩电流

extern Uint16 * const pDispValueU0[];
extern Uint16 * const pOscValue[];
extern Uint16 const commDispIndex[];

typedef struct
{
    Uint16 flag;                // 0 -- 功能码的index; 1 --
    Uint16 data;                //

    Uint16 signal;              // 有无符号
    Uint16 upper;               // 上限
    Uint16 lower;               // 下限
    int16  delta;               //

    Uint16 index;               // 功能码的index
} LIMIT_DEAL_STRUCT;
extern LIMIT_DEAL_STRUCT limitDealData;

// 上下限处理
Uint16 LimitDeal(Uint16 signal, Uint16 data, Uint16 upper, Uint16 lower, int16 delta);

// 这些功能码的设定不能相同。目前280F有DI端子；320有DI端子，主、辅频率源选择
Uint16 NoSameDeal(Uint16 index, const Uint16 funcIndex[], int16 number, int16 data, int16 upper, int16 lower, int16 delta);

Uint16 ModifyFunccodeUpDown(Uint16 index, Uint16 *data, int16 delta);
Uint16 ModifyFunccodeEnter(Uint16 index, Uint16 data);
Uint16 TorqueBoostDeal(Uint16 power);
void DispDataDeal(void);
void UpdateErrorDisplayBuffer(void);
void ClearRecordDeal(void);

void MenuModeDeal(void);
void UpdataFuncCodeGrade(Uint16 funcCodeGrade[]);

extern const int16 decNumber[];

extern Uint16 loadSpeedDisp;
extern Uint16 pidFuncRefDisp;
extern Uint16 pidFuncFdbDisp;
extern Uint16 pulseInFrqDisp;
extern Uint16 vfSeparateVol;
extern Uint16 vfSeprateVolAim; 

void RestoreCompanyParaOtherDeal(void);
Uint16 GetFuncCodeInit(Uint16 index, Uint16 type);
Uint16 GetFuncCodeInitOriginal(Uint16 index);


// 某些功能码是其他功能码上下限的处理
LOCALD void LimitOtherCodeDeal(Uint16 index);

void MenuInit(void);

void UpdateInvType(void);



extern const union FUNC_ATTRIBUTE dispAttributeU0[];



enum CHECK_MENU_MODE_DEAL
{
    CHECK_MENU_MODE_DEAL_CMD,           // 指令，准备开始
    CHECK_MENU_MODE_DEAL_SERACHING,     // 正在搜索
    CHECK_MENU_MODE_DEAL_END_NONE,      // 搜索全部功能码，没有发现与出厂值不同的功能码
    CHECK_MENU_MODE_DEAL_END_ONCE       // 搜索找到一个
};

extern Uint16 mainLoopTicker;



#endif  // __F_MENU_H__



