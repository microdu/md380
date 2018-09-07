//======================================================================
//
// 命令源。
// 运行命令，frq
//
// Time-stamp: <2012-06-14 16:04:40  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_io.h"
#include "f_comm.h"
#include "f_ui.h"
#include "f_main.h"
#include "f_menu.h"
#include "f_eeprom.h"
#include "f_posCtrl.h"


extern int32 pcRef;
extern int32 pcFdb;

#if F_DEBUG_RAM                             // 仅调试功能，在CCS的build option中定义的宏

#define DEBUG_F_RUN_SRC_DI          0       // 端子命令源
#define DEBUG_F_RUN_SRC_COMM        0       // 通讯命令源
#define DEBUG_F_SWING               0       // 摆频
#define DEBUG_F_TUNE                0       // 调谐
#define DEBUG_F_POWER_OFF_NO_STOP   0       // 瞬停不停
#define DEBUG_F_LOSE_LOAD           0       // 掉载
#define DEBUG_F_LOWER_FRQ           0       // 低于下限频率
#define DEBUG_F_JOG                 0       // jog
#define DEBUG_F_RUN_TIME            0
#define DEBUG_F_START_RUN           0
#define DEBUG_F_STOP_RUN            0
#define DEBUG_F_ACC_DEC_TIME        0
#define DEBUG_F_RUN_POWERUP_TIME    0
#define DEBUG_F_ERROR_RUN_FRQ       0
#define DEBUG_F_DORMANT_DEAL        0

#elif 1

#define DEBUG_F_RUN_SRC_DI          1
#define DEBUG_F_RUN_SRC_COMM        1
#define DEBUG_F_SWING               1
#define DEBUG_F_TUNE                1
#define DEBUG_F_POWER_OFF_NO_STOP   1
#define DEBUG_F_LOSE_LOAD           1
#define DEBUG_F_LOWER_FRQ           1
#define DEBUG_F_JOG                 1
#define DEBUG_F_RUN_TIME            1
#define DEBUG_F_START_RUN           1
#define DEBUG_F_STOP_RUN            1
#define DEBUG_F_ACC_DEC_TIME        1
#define DEBUG_F_RUN_POWERUP_TIME    1
#define DEBUG_F_ERROR_RUN_FRQ       1       // 故障时继续运行频率
#define DEBUG_F_DORMANT_DEAL        1       // 休眠唤醒
#endif

union RUN_CMD runCmd;               // 运行命令字
union RUN_FLAG runFlag;             // 运行标识字
union RUN_STATUS_FIRST_STEP runStatus1Step;  // 运行状态第一拍
union DSP_MAIN_COMMAND dspMainCmd;  // 功能传递给性能的主命令字
union DSP_MAIN_COMMAND1 dspMainCmd1;

union DSP_SUB_COMMAND dspSubCmd;    // 功能传递给性能的辅命令字
union DSP_STATUS dspStatus;         // 性能传递给功能的状态字
union DSP_SUB_STATUS dspSubStatus;  // 辅助状态字

union INVT_STATUS invtStatus;

Uint16 startProtectSrc;             // 启动保护源
Uint16 otherStopSrc;                // otherStop源

// otherStop源
// 自由停车
// plc时间全部为0，或者单次运行结束停机
Uint16 otherStopLowerSrc;

// otherStopLowerSrc源
// 启动时低于启动频率
// 启动时/运行时低于下限频率，且低于下限频率停机

enum RUN_STATUS runStatus;              // 运行状态
enum START_RUN_STATUS startRunStatus;   // 启动时的运行状态
enum STOP_RUN_STATUS stopRunStatus;     // 停机时的运行状态
LOCALF enum RUN_STATUS runStatusOld4LoseLoad;
LOCALF enum RUN_STATUS runStatusOld4Jog;
LOCALF enum RUN_STATUS runStatusOld4POffNoStop;

Uint16 runSrc;          // 运行命令源
Uint32 accFrqTime;      // 加速时间，单位同功能码
Uint32 decFrqTime;      // 减速时间

enum SWING_STATUS swingStatus;  // 摆频状态
LOCALF Uint32 swingDoubleAw;    // 峰峰值，摆幅的2倍
LOCALF int32 swingMaxFrq;       // 摆频上限频率
LOCALF int32 swingMinFrq;       // 摆频下限频率
LOCALF int32 swingJumpFrq;      // 摆频突跳频率
LOCALF Uint32 swingAccTime;     // 摆频加速时间
LOCALF Uint32 swingDecTime;     // 摆频减速时间
Uint16 swingFrqLimit;           // 摆频限定中

Uint16 setRunLostTime;
Uint16 setRunTimeAim;               // 设定定时运行时间
//LOCALF Uint32 setTimeTicker;        // 定时运行计时(S)
//LOCALF Uint32 setTimeSecTicker;     // 定时运行计时(S)
//LOCALF Uint32 setTimeMinTicker;     // 定时运行计时(M)
LOCALF Uint32 runTimeTicker;        // 运行时间计时
LOCALF Uint32 lowerDelayTicker;     // 低于下限频率停机时间计时
LOCALF Uint32 shuntTicker;          // 瞬停不停的电压回升时间计时
LOCALF Uint16 runTimeAddupTicker;   // 累计运行时间计时
LOCALF Uint16 powerUpTimeAddupTicker;   // 累计运行时间计时

Uint16 shuntFlag;    // 瞬停不停标志

Uint16 accDecTimeSrcPlc;        // 当前PLC段的加减速时间选择

#define STOP_REM_NONE           0   // 没有停机记忆，或者停机记忆完成
#define STOP_REM_WAIT           1   // 需要停机记忆
#define STOP_REM_PAUSE_JUDGE    2   // 运行之后暂停，根据是否真正停机判断是否停机记忆
LOCALF Uint16 stopRemFlag;
Uint16 runDirPanelOld;   // 操作面板的old运行方向

LOCALF Uint16 bRunJog;  // 同时有运行和点动命令，且运行中点动使能，置为1.

#define TUNE_STEP_OVER_TIME_MAX     60UL    // 静态调谐限时时间，_s
#define TUNE_RUN_OVER_TIME_MAX      180UL   // 运行时的调谐限时时间，_s
enum TUNE_RUN_STATUS
{
    TUNE_RUN_STATUS_WAIT,           // 给性能发送调谐命令，等待
    TUNE_RUN_STATUS_ACC,            // 动态调谐，加速过程
    TUNE_RUN_STATUS_DEC,            // 动态调谐完成，停机
    TUNE_RUN_STATUS_END             // 调谐完成
};
Uint16 tuneCmd;                 // 调谐
Uint16 saveTuneDataFlag;        // 调谐保存标志，一次调谐过程只保存一次
//Uint16 tunePGflag;              // 参数辨识PG卡标示



//-----------------------------------------------------------
// 休眠唤醒
// 1. 设定频率低于休眠频率时，进入休眠状态，不论有无运行命令进入停机状态。
// 2. 设定频率高于唤醒频率时，响应运行命令。即在有运行命令时，进入运行状态。
// 3. 第一次有运行命令，高于休眠频率，也要响应运行命令。
// 4. 休眠与唤醒的切换，有延时，由功能码"唤醒延迟时间"和"休眠延迟时间"确定。

// 休眠的状态信息
enum DORMANT_STATUS
{
    DORMANT_RESPOND,            // 响应运行命令。高于唤醒频率，或者第一次有运行命令时高于休眠频率。
    DORMANT_NO_RESPOND,         // 不响应运行命令。低于休眠频率
    DORMANT_2                   // 未处于休眠状态，但启动时发现低于休眠频率。
};
enum DORMANT_STATUS dormantStatus;  // 0-没有休眠；1-处于休眠状态；2-未处于休眠状态，但启动时发现低于休眠频率。
// 
LOCALD void DormantDeal(void);

LOCALD void StartRunCtrl(void); // 启动
LOCALD void NormalRunCtrl(void);
LOCALD void StopRunCtrl(void);
LOCALD void JogRunCtrl(void);   // 点动
LOCALD void TuneRunCtrl(void);  // 调谐
LOCALD void ShutDownRunCtrl(void);
LOCALD void LoseLoadRunCtrl(void);
LOCALD void LowerThanLowerFrqDeal(void);
LOCALD void PowerOffNoStopDeal(void);
LOCALD void SwingDeal(void);
LOCALD void UpdateSwingPara(void);
void SaveTuneData(void);

void AccCalc(int32 frq0);

void RunSrcUpdateFrq(void);

extern Uint16 spdLoopOut;

void setTimeRun(void);

//=====================================================================
//
// 更新运行命令字runCmd
//
// 流程:
//
// 1. 更新运行命令源命令源runSrc。
//
// 2. 根据运行命令给定方式，更新命令字runCmd。
//
// 3. 根据变频器内部逻辑，更新命令字runCmd。
//
//=====================================================================
void UpdateRunCmd(void)
{
    static Uint16 bMfkSwitch;       // MFK切换到面板有效标志
    static Uint16 fwdOld, revOld, fJogOld, rJogOld;
    static Uint16 startProtectSrcOld;
    static Uint16 otherStopSrcOld;
    static Uint16 otherStopLowerSrcOld;
    static Uint16 bJog, bRun;
    static Uint16 jogCmdPulse;      // 脉冲有效的jog，目前仅有通讯点动
    static Uint16 runSrcBak;
    Uint16 jogCmdLevel;             // 电平有效的jog
    Uint16 jogCmd;                  // 总的jog命令
    Uint16 protectCmd;              // 保护标志
    Uint16 fwd, rev, fJog, rJog;

    jogCmdLevel = 0;
    protectCmd = 0;

    fwd = diFunc.f1.bit.fwd;
    rev = diFunc.f1.bit.rev;
    fJog = diFunc.f1.bit.fwdJog;
    rJog = diFunc.f1.bit.revJog;

    if (POWER_ON_WAIT == powerOnStatus)  // 上电还没有准备完成
    {
        fwdOld = fwd;
        revOld = rev;
        fJogOld = fJog;
        rJogOld = rJog;
    }

    // 1. 更新命令源runSrc
    if (KEY_SWITCH == keyFunc)
    {
        bMfkSwitch = !bMfkSwitch;
        keyFunc = 0;
    }
    if (FUNCCODE_mfkKeyFunc_SWITCH != funcCode.code.mfkKeyFunc)
    {
        bMfkSwitch = 0;         // MF.K键的功能不是切换到面板命令，清零
    }

    runSrc = funcCode.code.runSrc;      // 功能码设定
    
    // MF.K键切换到面板命令
    if (bMfkSwitch)                     // MF.K键的切换到面板命令
    {
        runSrc = FUNCCODE_runSrc_PANEL;
    }
    
    // DI或comm与面板切换
    if (diFunc.f1.bit.localOrRemote)        
    {
        if (FUNCCODE_runSrc_PANEL != runSrc)
        {
            runSrc = FUNCCODE_runSrc_PANEL;
        }
#if 0
        else
        {
            runSrc = funcCode.code.runSrc;
        }
#endif
    }
    // DI与comm切换
    else if (diFunc.f2.bit.diOrComm)        
    {
        if (FUNCCODE_runSrc_DI == funcCode.code.runSrc)
        {
            runSrc = FUNCCODE_runSrc_COMM;
        }
        else if (FUNCCODE_runSrc_COMM == funcCode.code.runSrc)
        {
            runSrc = FUNCCODE_runSrc_DI;
        }
    }

    // 运行命令切换时停机
    if (runSrcBak != runSrc)
    {
		runCmd.bit.common0 = 0;
        runCmd.bit.common = 0;
    }
    runSrcBak = runSrc;
    
    // 2. 根据运行命令给定方式，更新命令字runCmd
    runCmd.bit.freeStop = 0;    // 先清除 freeStop, pause, tune
    runCmd.bit.errorReset = 0;
    runCmd.bit.pause = 0;
    
    switch (runSrc)                     // 运行命令通道
    {
        case FUNCCODE_runSrc_PANEL:     // 运行命令给定方式: 操作面板
            jogCmdPulse = 0;
            
            if ((KEY_STOP == keyFunc) || (diFunc.f2.bit.stopPanel))
            {
                runCmd.bit.common0 = 0;             // 停机
            }
            else if (KEY_REV == keyFunc)
            {
                runCmd.bit.dir = ~runCmd.bit.dir;   // 反转
                runDirPanelOld = runCmd.bit.dir;    // 改变方向时才更新runDirPanelOld
                keyFunc = 0;
            }
            else if (KEY_RUN == keyFunc)
            {
                runCmd.bit.common0 = 1;             // 运行
                keyFunc = 0;
            }

            if (!runFlag.bit.run)                   // 停机时才恢复runCmd.bit.dir
            {
                runCmd.bit.dir = runDirPanelOld;    // 没有切换命令源时，没有影响。
            }

            break;
            
#if DEBUG_F_RUN_SRC_DI
        case FUNCCODE_runSrc_DI:                        // 运行命令给定方式: 端子
            jogCmdPulse = 0;
            
            switch (funcCode.code.diControlMode)        // 端子命令方式
            {
                case FUNCCODE_diControlMode_2LINE1:     // 两线式1
                case FUNCCODE_diControlMode_2LINE2:     // 两线式2
                    if (FUNCCODE_diControlMode_2LINE1 == funcCode.code.diControlMode)   // 两线式1
                    {
                        if (fwd && (!rev))              // fwdRun
                        {
                            runCmd.bit.dir = FORWARD_DIR;
                            runCmd.bit.common0 = 1;
                        }
                        else if ((!fwd) && (rev))       // revRun
                        {
                            runCmd.bit.dir = REVERSE_DIR;
                            runCmd.bit.common0 = 1;
                        }
                        else
                        {
                            runCmd.bit.common0 = 0;     // 停机
                        }

                        protectCmd = 1; // 从两线式2到两线式1，也需要要启动保护。
                        // 例如，fwd=0,rev=1,两线式2；然后改为两线式1，应该进入启动保护。
                    }
                    else    // 两线式2
                    {
                        runCmd.bit.common0 = fwd;   // 运行命令
                    
                        if (rev)                // 方向
                            runCmd.bit.dir = REVERSE_DIR;
                        else
                            runCmd.bit.dir = FORWARD_DIR;

                        protectCmd = 2;     // 从两线式1到两线式2，也需要要启动保护。
                        // 例如，fwd=1,rev=1,两线式1；然后改为两线式2，应该进入启动保护。
                    }
#if 0   //+e 改变了两线式端子命令源，是否需要保护? 暂时不保护
                    if ((protectCmdOld != protectCmd) // 改变了两线式端子命令源，包括0->1,0->2,1->2,2->1
                        && (((runFlag.bit.run) && (RUN_STATUS_STOP == runStatus))
                            || (!runFlag.bit.run)
                            )   // 键盘/端子点动在所有命令源都有效
                        )
                        startProtectSrc++;
#endif
                    break;

                case FUNCCODE_diControlMode_3LINE1:         // 三线式1
                case FUNCCODE_diControlMode_3LINE2:         // 三线式2
                    if (fwdOld != fwd)  // 脉冲(上升沿)有效
                    {
                        if (fwd)        // fwd
                        {
                            runCmd.bit.common0 = 1;
                            
                            if (FUNCCODE_diControlMode_3LINE1 == funcCode.code.diControlMode)
                            {
                                runCmd.bit.dir = FORWARD_DIR;
                            }
                        }
                    }

                    if (FUNCCODE_diControlMode_3LINE1 == funcCode.code.diControlMode) // 三线式1
                    {
                        if (revOld != rev)
                        {
                            if (rev)
                            {
                                runCmd.bit.common0 = 1;
                                runCmd.bit.dir = REVERSE_DIR;
                            }
                        }
                    }
                    else        // 三线式2
                    {
                        rev ? (runCmd.bit.dir = REVERSE_DIR) : (runCmd.bit.dir = FORWARD_DIR);
                        #if 0
                        if (rev)          // 方向
                            runCmd.bit.dir = REVERSE_DIR;
                        else
                            runCmd.bit.dir = FORWARD_DIR;    
                        #endif
                    }

                    if (!diFunc.f1.bit.tripleLineCtrl)  // 运行命令
                    {
                        runCmd.bit.common0 = 0;
                    }

                    break;

                default:
                    break;
            }

            break;
#endif

#if DEBUG_F_RUN_SRC_COMM
        case FUNCCODE_runSrc_COMM:                 // 运行命令给定方式: 串口通讯
#if DEBUG_F_PLC_CTRL
            // PLC功能有效时，通讯命令来源于PLC控制
            if (funcCode.code.plcEnable)
            {
               commRunCmd = funcCode.code.plcCmd;
            }
#endif            
            switch (commRunCmd)
            {
#if 1
                case SCI_RUN_CMD_FWD_RUN:
                    if (!runFlag.bit.jog)   // 当前没有点动/运行中点动，不能更改点动的方向
                    {
                        runCmd.bit.common0 = 1;
                        runCmd.bit.dir = FORWARD_DIR;
                    }
                    break;
                    
                case SCI_RUN_CMD_REV_RUN:
                    if (!runFlag.bit.jog)
                    {
                        runCmd.bit.common0 = 1;
                        runCmd.bit.dir = REVERSE_DIR;
                    }
                    break;
                    
#elif 1
                case SCI_RUN_CMD_FWD_RUN:
                    // 当前没有点动/运行中点动，不能更改点动的方向
                    if ((!runFlag.bit.jog) || (jogCmdPulse))    // 通讯引起的点动，然后输入通讯运行命令，要响应。
                    {
                        runCmd.bit.common0 = 1;
                        runCmd.bit.dir = FORWARD_DIR;
                        jogCmdPulse = 0;
                    }
                    break;
                case SCI_RUN_CMD_REV_RUN:
                    if ((!runFlag.bit.jog) || (jogCmdPulse))
                    {
                        runCmd.bit.common0 = 1;
                        runCmd.bit.dir = REVERSE_DIR;
                        jogCmdPulse = 0;
                    }
                    break;
#endif
                case SCI_RUN_CMD_FWD_JOG:
                    jogCmdPulse = RUN_CMD_FWD_JOG;
                    break;
                    
                case SCI_RUN_CMD_REV_JOG:
                    jogCmdPulse = RUN_CMD_REV_JOG;
                    break;
                    
                case SCI_RUN_CMD_FREE_STOP:
                    otherStopSrc++;     // 自由停车
                    runCmd.bit.freeStop = 1;
                    //lint -fallthrough     //通讯自由停车也要清除jogCmdPulse和common0
                    
                case SCI_RUN_CMD_STOP:
                    jogCmdPulse = 0;    // 通讯stop可以停止通讯点动，但不可停止其他点动
                    runCmd.bit.common0 = 0;
                    break;
                    
                case SCI_RUN_CMD_RESET_ERROR:
                    runCmd.bit.errorReset = 1;
                    break;
                    
                default:
                    break;
            }

            break;
#endif

#if 0
        // 上电运行
        case FUNCCODE_runSrc_AUTO_RUN:
            runCmd.bit.common0 = 1;
            break;
#endif            
        default:
            break;
    }

//--------------------------------------------------------------------------------------
// 3. 根据变频器的内部逻辑，更新runCmd
#if DEBUG_F_ACC_DEC_TIME
    // 故障时的启动保护。继续运行的故障，不进入启动保护
    if (((errorCode && (errorAttribute.bit.level != ERROR_LEVEL_RUN))   // 故障
          || (POWER_ON_WAIT == powerOnStatus)                           // 上电还没有准备完成
          )
        )
    {
        startProtectSrc++;
    }

    if (bStopPlc)    // plc时间全部为0，或者单次运行结束停机
    {
        if (runCmd.bit.common0)      // 不启动
        {
            otherStopSrc++;
        }

        bStopPlc = 0;
    }

    if ((diFunc.f1.bit.closePwm)         // 端子自由停车，强制保护
        ||(diFunc.f2.bit.emergencyStop)   // 端子紧急停车
		||(diFunc.f2.bit.stop4dec)        // 减速时间4停车
        || ((FUNCCODE_stopKeyFunc_ALL == funcCode.code.stopKeyFunc) // 停机功能各种控制方式均有效
            && (KEY_STOP == keyFunc)    // 面板命令源，stop也可以停止端子点动。
            )
        )
    {
        otherStopSrc++;
        jogCmdPulse = 0;    // stop停止通讯点动
    }

    if (startProtectSrcOld != startProtectSrc)  // 发现有新的startProtectSrc将bJog和bRun清零
    {
        bJog = 0;
        bRun = 0;

        runCmd.bit.startProtect = 1;
    }

    if ((otherStopSrcOld != otherStopSrc)    // 发现有新的otherStopSrc，将bJog和bRun清零
        || (otherStopLowerSrcOld != otherStopLowerSrc))
    {
        bJog = 0;
        bRun = 0;

        runCmd.bit.otherStop = 1;
    }

    if (!funcCode.code.startProtect) // 启动保护功能码无效。没有必要将bJog和bRun置为1
    {
        runCmd.bit.startProtect = 0;
    }

    if (runCmd.bit.startProtect || runCmd.bit.otherStop)
    {
        if ((fJogOld != fJog)       // 正向点动端子发生变化
            || (rJogOld != rJog)    // 反向点动端子发生变化
            || ((!fJog) && (!rJog)) // 正向点动、反向点动端子都为无效
            )
        {
            bJog = 1;
        }

        if (!bJog)              // jog处于保护状态，启动保护/otherStop
        {
            fJog = 0;
            rJog = 0;
        }

        if (!runCmd.bit.common0)
        {
            bRun = 1;
        }

        if ((!runFlag.bit.common) // 已经停机
            && (FUNCCODE_pidCalcMode_YES == funcCode.code.pidCalcMode)
            && (otherStopLowerSrcOld != otherStopLowerSrc) // 供水模式低于下限频率/启动频率不保护
            )
        {
            otherStopLowerSrcOld = otherStopLowerSrc;

            bRun = 1;
        }

        if (!bRun)              // 处于保护状态
        {
            runCmd.bit.common0 = 0;
        }

        if (!protectCmd)        // 非两线式端子命令源
        {
            if (bJog)
            {
                runCmd.bit.startProtect = 0;
                runCmd.bit.otherStop = 0;
                
                otherStopLowerSrcOld = otherStopLowerSrc;
            }
        }
        else                    // 两线式端子命令源
        {
            if (bRun && bJog)
            {
                runCmd.bit.startProtect = 0;
                runCmd.bit.otherStop = 0;

                otherStopLowerSrcOld = otherStopLowerSrc;
            }
        }
    }
    
    fwdOld = diFunc.f1.bit.fwd;
    revOld = diFunc.f1.bit.rev;
    fJogOld = diFunc.f1.bit.fwdJog;
    rJogOld = diFunc.f1.bit.revJog;
 // protectCmdOld = protectCmd;
    startProtectSrcOld = startProtectSrc;
    otherStopSrcOld = otherStopSrc;
#endif

    commRunCmd = 0;                 // 通讯命令字清零

#if DEBUG_F_PLC_CTRL
    // PLC卡命令清0
   funcCode.code.plcCmd = 0;
#endif

#if DEBUG_F_JOG
    // 更新面板/端子点动命令；通讯点动时，在通讯控制里面更新
    if ((KEY_FWD_JOG == keyFunc) && (runSrc == FUNCCODE_runSrc_PANEL))    // 面板点动，正方向点动
    {
        jogCmdLevel = RUN_CMD_FWD_JOG;
    }
    else if ((KEY_REV_JOG == keyFunc) && (runSrc == FUNCCODE_runSrc_PANEL))   // 面板点动，反方向点动
    {
        jogCmdLevel = RUN_CMD_REV_JOG;
    }
    else if ((fJog) && (runSrc == FUNCCODE_runSrc_DI))  // 端子正点动
    {
        jogCmdLevel = RUN_CMD_FWD_JOG;
    }
    else if ((rJog) && (runSrc == FUNCCODE_runSrc_DI))  // 端子反点动
    {
        jogCmdLevel = RUN_CMD_REV_JOG;
    }
    // 否则，电平点动命令为零
#endif


//---------------------------------------------------------------
// 故障后运行方式
// 自由停机、减速停机、继续运行
    if ((ERROR_LEVEL_FREE_STOP == errorAttribute.bit.level)   // 自由停车
       	|| (((runStatus == RUN_STATUS_TUNE)                  // 调谐 且 存在故障
       	    || (dspMainCmd1.bit.speedTrack)                  // 转速追踪 且存在故障
       	    )
       	    &&  errorCode
       	   )
		)
    {
        if (runCmd.bit.common0)  // 有运行命令
        {
            menu0DispStatus = MENU0_DISP_STATUS_ERROR;  // 进入故障/告警显示状态
        }
        
        runCmd.bit.freeStop = 1;
    }
    else if (ERROR_LEVEL_STOP == errorAttribute.bit.level)  // 按停机方式停机
    {
        if (runCmd.bit.common0)  // 有运行命令
        {
            menu0DispStatus = MENU0_DISP_STATUS_ERROR;      // 进入故障/告警显示状态
        }
        
        runCmd.bit.common0 = 0;
    }
    else if (ERROR_LEVEL_RUN == errorAttribute.bit.level)   // 继续运行
    {
        ;
    }
//---------------------------------------------------------------

// 休眠、唤醒
    DormantDeal();

// common run
    runCmd.bit.common = 0;
    if ((runCmd.bit.common0)  // 有运行命令
        && (DORMANT_RESPOND == dormantStatus)
        )
    {
        Uint16 common = 0;
        
        if ((runFlag.bit.common) || (runFlag.bit.tune)) // 当前正在运行，或者正在tune
        {
            common = 1;
        }
        else if ((!runFlag.bit.jog) && (!runFlag.bit.tune)) // 准备启动
        {
            if (((ABS_INT32(frqAimTmp0) >= funcCode.code.startFrq)  // 启动时，低于启动频率不启动。跳跃频率计算之前。
                 || (runFlag.bit.pid)   // PID(停机不运算)时，即使有启动频率，要保证能启动
                 )
                && ((FUNCCODE_lowerDeal_DELAY_STOP != funcCode.code.lowerDeal) // 以下限频率运行/以零速运行，跳跃频率计算之后。
                    || (ABS_INT32(frqAimTmp) >= lowerFrq) // 设定频率不低于下限频率
                    )
                )
            {
                common = 1;
            }
            else
            {
                if (2 == mainLoopTicker)    // 进入主循环的第1拍，不必处理otherStopLowerSrc(固定的启动保护)
                {
                    otherStopLowerSrc++;
                }
                
                runCmd.bit.common0 = 0;     // 清运行命令
            }
        }
        else
        {
            if (!runFlag.bit.jogWhenRun)    // 运行暂停中的运行中点动，程序也会执行到这里
            {
                runCmd.bit.common0 = 0;
            }
        }

        if (common && 
            (!diFunc.f1.bit.runPause)       // 运行暂停端子无效
            )
        {
            runCmd.bit.common = 1;          // 真正有效的运行命令
        }
    }

// free stop
    if (diFunc.f1.bit.closePwm  // DI端子自由停车，common/jog/tune均有效

//        || errorCode
        || (POWER_ON_WAIT == powerOnStatus)        // 性能正在上电处理，不能发送运行/点动命令
        || (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA == funcCodeRwMode)     // 恢复(部分)出厂设定值
        || (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL == funcCodeRwMode) // 恢复(全部)出厂设定值
//+e        || (FUNCCODE_paraInitMode_RESTORE_USER_PARA == funcCodeRwMode)  // 恢复保存的用户功能码
//        || ((RUN_MODE_TORQUE_CTRL == runMode) && (!runCmd.bit.common0)) // 转矩控制模式下，停机命令
        )
    {
        runCmd.bit.freeStop = 1;          // 若是恢复出厂参数，仅仅是为了不响应运行命令 
    }

    // 紧急停车
    if (diFunc.f2.bit.emergencyStop || diFunc.f2.bit.stop4dec)
    {
        runCmd.bit.common0 = 0;     // common run
        runCmd.bit.common = 0;
    }
    
    if ((diFunc.f1.bit.runPause) && (!runCmd.bit.freeStop))//+e || (extendCmd.bit.runPause))
    {
        runCmd.bit.pause = 1;       // 自由停车时，清除运行暂停命令
    }

	// || (0) //+e 转矩控制, (funcCode.code.torqueControl && (!diFunc.f2.bit.forbid_torque_control))
    //+e (extendCmd.bit.freeStop)
    if ((runFlag.bit.common) 
    	&& (!runCmd.bit.common)
        && (!runFlag.bit.jogWhenRun)                              // 普通停机命令
        && (FUNCCODE_stopMode_FREESTOP == funcCode.code.stopMode) // 停机方式为自由停车
        && (DORMANT_NO_RESPOND != dormantStatus)                  // 不处于休眠状态
        )
    {
        runCmd.bit.freeStop = 1;
    }

    if ((runCmd.bit.freeStop)       // 故障，自由停车端子，恢复出厂参数
        && (!runCmd.bit.pause)      // 运行暂停引起的自由停车，不清运行命令
        )
    {
        runCmd.bit.common0 = 0;     // common run
        runCmd.bit.common = 0;
        jogCmdPulse = 0;            // 清jog命令
        jogCmdLevel = 0;
    }

// jog
    jogCmd = jogCmdLevel | jogCmdPulse; // 相与成一个点动命令

// jog when run, 判断是否运行中点动
    bRunJog = 0;
    if ((jogCmd)
        && (runFlag.bit.common                              // 当前正在运行
            || ((runCmd.bit.pause) && (runCmd.bit.common0)) // 当前正在运行之后的运行暂停状态
            || runCmd.bit.common                            // 同时有运行和点动命令
            )
        )
    {
        if (funcCode.code.jogWhenRun)   // 运行中点动使能
        {
            bRunJog = 1;                     // 同时有运行和点动命令，且运行中点动使能
            
            if ((!dspMainCmd.bit.startBrake) // 直流制动过程中，运行中点动无效
                && (!dspMainCmd.bit.stopBrake)
                && (!dspMainCmd.bit.startFlux) // 启动励磁
                )
            {
                runFlag.bit.jogWhenRun = 1;
            }
            else
            {
                jogCmd = 0;
            }
        }
        else
        {
            jogCmd = 0;         // 不是运行中点动(运行时，运行中点动无效，清点动命令)
        }
    }

#if DEBUG_F_JOG
    if (RUN_CMD_FWD_REV_JOG == jogCmd)  // 既有正向点动命令，又有反向点动命令，认为正向点动
    {
        jogCmd = RUN_CMD_FWD_JOG;
    }

    runCmd.bit.jog = jogCmd;

    if (jogCmdPulse != jogCmd)    // 最终的点动命令不是脉冲点动命令，则清除脉冲点动命令
    {
        jogCmdPulse = 0;
    }
#endif

// 运行暂停的停机记忆
    if (STOP_REM_PAUSE_JUDGE == stopRemFlag)
    {
        if (!runCmd.bit.pause)          // 运行暂停端子无效
        {
            stopRemFlag = STOP_REM_NONE;
        }
        else if (!runCmd.bit.common0)   // 运行暂停状态下真正的停机
        {
            stopRemFlag = STOP_REM_WAIT;
        }
    }
//--------------------------------------------------------------------------------------

}

//=====================================================================
// 
// 定时运行
//
//=====================================================================
void setTimeRun()
{
	Uint16 i;

    // 定时功能无效
    if (!funcCode.code.setTimeMode)
    {
        // 剩余运行时间为0
        setRunLostTime = 0;
        setRunTimeAim = 0;
        return;
    }
    
    // 定时设定时间量程
    setRunTimeAim = funcCode.code.setTimeValue;

    // 定时时间设定来源
    switch (funcCode.code.setTimeSource)
    {
        case FUNCCODE_plcFrq0Src_AI1:    // AI1\AI2\AI3
        case FUNCCODE_plcFrq0Src_AI2:
        case FUNCCODE_plcFrq0Src_AI3:
            i = funcCode.code.setTimeSource - FUNCCODE_plcFrq0Src_AI1;
            setRunTimeAim = ((int32)ABS_INT32(aiDeal[i].set) * setRunTimeAim + (1 << 14)) >> 15;
            break;

        default:
            break;
    }

    // 设定定时时间不为0
    if (setRunTimeAim > 0)
    {
        // 定时到达(非自动运行)停机
        if (curTime.runTime >= setRunTimeAim) 
        {
            // 定时时间到达停机
            runCmd.bit.common0 = 0;
       		runCmd.bit.common = 0;
            otherStopSrc++;
            setRunLostTime = 0;    // 定时运行剩余时间为0
        }
        else
        {
            setRunLostTime = setRunTimeAim - curTime.runTime;
        }
    }
    else
    {
        setRunLostTime = 0;
    }

}

//=====================================================================
// 
// 上电、运行时间统计
//
//=====================================================================
void runTimeCal(void)
{
#if DEBUG_F_RUN_POWERUP_TIME
    if(++powerUpTimeAddupTicker >= ((Uint16)(TIME_UNIT_MS_PER_SEC / RUN_TIME_CAL_PERIOD)))
    {
        powerUpTimeAddupTicker = 0;

        // 当前上电时间
        if(++curTime.powerOnTimeSec>= TIME_UNIT_SEC_PER_MIN)
        {
            curTime.powerOnTimeSec = 0;   // 当前上电时间秒清0
            curTime.powerOnTimeM++;       // 当前上电时间分++
        }
        
        if (++funcCode.code.powerUpTimeAddupSec >= TIME_UNIT_SEC_PER_HOUR)
        {
            funcCode.code.powerUpTimeAddupSec = 0;  // 掉电时才保存到EEPROM
            funcCode.code.powerUpTimeAddup++;       // 累计上电时间, 单位: h，停机时保存到EEPROM
        }
    }

    // 累计上电时间 达到 设定上电时间，
    if (funcCode.code.powerUpTimeArriveSet // 设定上电时间为0，则定时功能无效
        && (funcCode.code.powerUpTimeAddup >= funcCode.code.powerUpTimeArriveSet)  // 累计运行时间 达到 设定运行时间，
        )
    {
        errorOther = ERROR_POWER_UP_TIME_OVER;      // 上电时间到达
    }
#endif
    
#if DEBUG_F_RUN_TIME
    // 处于运行状态且本次运行时间清零端子无效
    // 如果时间超过运行时间到达后，则运行时间清零端子开始无效
    if ((runFlag.bit.run) 
        && ((!diFunc.f2.bit.clearSetRunTime) 
        || ((curTime.runTime >= funcCode.code.setTimeArrive)
            &&(funcCode.code.setTimeArrive)))
		)
    {
        if (++runTimeAddupTicker >= ((Uint16)(TIME_UNIT_MS_PER_SEC / RUN_TIME_CAL_PERIOD)))
        {
            runTimeAddupTicker = 0;

            if(++curTime.runTimeSec >= TIME_UNIT_SEC_PER_MIN)
            {
                curTime.runTimeSec = 0;   // 当前运行时间秒清0
                curTime.runTimeM++;       // 当前运行时间分++
            }
            
            if (++funcCode.code.runTimeAddupSec >= TIME_UNIT_SEC_PER_HOUR)
            {
                funcCode.code.runTimeAddupSec = 0;  // 掉电时才保存到EEPROM
                funcCode.code.runTimeAddup++;       // 累计运行时间, 单位: h，停机时保存到EEPROM
            }
        } 

    }
    else
    {
        // 清当前运行时间
        curTime.runTimeSec = 0;   
        curTime.runTimeM = 0;       
    }

    // 当前上电时间、运行时间
    curTime.runTime = curTime.runTimeM*10 + curTime.runTimeSec/6;
    curTime.powerOnTime = curTime.powerOnTimeM;

    // 累计运行时间 达到 设定运行时间，
    if (funcCode.code.runTimeArriveSet // 设定运行时间为0，则定时功能无效
        && (funcCode.code.runTimeAddup >= funcCode.code.runTimeArriveSet)  // 累计运行时间 达到 设定运行时间，
        )
    {
        errorOther = ERROR_RUN_TIME_OVER;      // 运行时间到达
    }
   
#endif
}

//=====================================================================
// 
// 根据runCmd，
// (1) 更鼻八彩备ㄆ德蔲rq
// (2) 更新传递给dspMainCmd的run, dspSubCmd.
//
// 备注：
// (1) 当有启动命令时，要能在最短时间之内打开PWM。
//     如，出厂值默认(没有启动直流制动，没有启动频率保持)时，能在给定
//     启动命令的当前周期，就能给性能瞬时频率和开启PWM的命令。
// (2) 运行/点动时，当有停机/自由停机/停止点动的命令时，要能在最短时间
//     内关闭PWM。
//     如，没有停机直流制动等待，没有停机直流制动时，能在给定停机命令
//     的当前周期，就给性能减小的瞬时频率的命令。若停机时间为0，则要
//     在当前周期给出关闭PWM的命令。
//
//=====================================================================
void RunSrcDeal(void)
{
// 更新运行命令
    UpdateRunCmd();

// 更新瞬时频率
    RunSrcUpdateFrq();

#if DEBUG_F_POSITION_CTRL
{
    UniversalDebug(0);
    UniversalDebug(1);
    UniversalDebug(2);
    UniversalDebug(3);
    UniversalDebug(4);
    UniversalDebug(5);
}
#endif
}



//=====================================================================
// 
// 更新加减速时间基准
// 
//=====================================================================
void UpdateBenchTime(void)
{
    switch (funcCode.code.accDecTimeUnit)
    {
        case FUNCCODE_accDecTimeUnit_0POINT:
            timeBench = 1000;
            break;
            
        case FUNCCODE_accDecTimeUnit_1POINT:
            timeBench = 100;
            break;

        default:
            timeBench = 10;
            break;     
    }
}




void RunSrcUpdateFrq(void)
{
    static Uint16 errorLevelOld, runOldFrqFlag;
    static int32 frqOld;
    
#if (DEBUG_F_POSITION_CTRL)
    if (FUNCCODE_pcZeroSelect_ENCODER == funcCode.code.pcZeroSelect) // 编码器z信号作为零位信号
    {
        if ((FUNCCODE_aptpMode_INDEX == funcCode.code.aptpMode)
            && (1 == pEQepRegsFvc->QFLG.bit.IEL)
            )
        {
            aptpAbsZeroOk = 1;
            pcOrigin = pEQepRegsFvc->QPOSILAT; // 原点
        }
        else if ((!aptpAbsZeroOk)
                && (1 == pEQepRegsFvc->QFLG.bit.IEL)
                )
        {
            aptpAbsZeroOk = 1;
            pcOrigin = pEQepRegsFvc->QPOSILAT; // 原点
        }
    }
    else if (FUNCCODE_pcZeroSelect_DI == funcCode.code.pcZeroSelect) // DI端子作为零信号
    {
#if 0
        if (diFunc.f3.bit.aptpZero)
        //if (1)
        {
            pcOrigin = GetCurPos(); // 原点
            aptpAbsZeroOk = 1;
        }
#endif
    }
    else if ((FUNCCODE_aptpMode_ABSOLUTE == funcCode.code.aptpMode)
        && (2 == funcCode.code.pcZeroSelect)
        )
    {
        pcOrigin = 0 - qposcntLastPOff; // 绝对原点
        //pcOrigin = 0;
        aptpAbsZeroOk = 1;
    }
    
    if (aptpAbsZeroOk)
    {
        aptpCurPos = (int32)(GetCurPos() - pcOrigin);
        //pcCurrentPulse = (aptpCurPos / 4);
        pcCurrentPulse = (aptpCurPos / 1);
    }

    //UpdateRcMasterPos();
    RcCutPointDeal();       // 剪切点的处理
#endif
    
    if (runCmd.bit.freeStop)   // 故障码，端子自由停车，通讯自由停车，停机方式为自由停车，正在恢复出厂参数。
    {
        runStatus = RUN_STATUS_SHUT_DOWN;
    }
#if DEBUG_F_TUNE
    // 调谐时，自由停车有效
    else if ((runCmd.bit.common) && (tuneCmd))
    {
        if (RUN_STATUS_WAIT == runStatus) // 等待启动
        {
            runStatus = RUN_STATUS_TUNE;
            saveTuneDataFlag = 0;           // 清零
        }
    }
#endif
    else if (diFunc.f2.bit.brake)   // DI端子直流制动有效
    {                       // 直流制动使能端子，对此没有影响。
        runStatus = RUN_STATUS_DI_BRAKE;
    }
    else if (diFunc.f2.bit.decBrake)
    {
        if (runFlag.bit.run)
        {
            runStatus = RUN_STATUS_DI_BRAKE_DEC;
        }
        else
        {
            runStatus = RUN_STATUS_DI_BRAKE;
        }
    }
#if DEBUG_F_LOSE_LOAD
    // 320没有掉载保护
    else if (dspStatus.bit.outAirSwitchOff
             && runFlag.bit.run
             && funcCode.code.loseLoadProtectMode) // 掉载保护
    {
        if (RUN_STATUS_LOSE_LOAD != runStatus)
        {
            runStatusOld4LoseLoad = runStatus;  // 保存掉载时刻的runStatus
        }
        
        runStatus = RUN_STATUS_LOSE_LOAD;
    }
#endif
#if DEBUG_F_JOG
    else if (runCmd.bit.jog) // 点动，包括运行中点动
    {
        if (RUN_STATUS_JOG != runStatus)
        {
            runStatusOld4Jog = runStatus;       // 保存运行中点动时刻的运行状态
        }
            
        runStatus = RUN_STATUS_JOG;
        
        runFlag.bit.jog = 1;
    }
#endif
    else if (runCmd.bit.common)
    {
        if (RUN_STATUS_WAIT == runStatus) // 等待启动
        {
            runStatus = RUN_STATUS_START;
            startRunStatus = START_RUN_STATUS_INIT;
        }
    }
    
// 根据运行方向,运行方式(点动还是普通运行),跳跃频率，计算设定频率(目标频率frqAim).
// 包括方向处理.
    UpdateFrqAim();

// 瞬停不停处理
    PowerOffNoStopDeal();

#if DEBUG_F_ERROR_RUN_FRQ
    // 运行过程中产生继续运行故障的第一拍记忆当前运行频率
    if (runFlag.bit.run && (ERROR_LEVEL_RUN == errorAttribute.bit.level) && (ERROR_LEVEL_RUN != errorLevelOld))
    {
        runOldFrqFlag = 1;
        frqOld = frq;        
    }
    
    if ((errorCode == ERROR_NONE)
        ||(ERROR_LEVEL_RUN != errorAttribute.bit.level)
        )
    {
        runOldFrqFlag = 0;
    }
    
    // 备份故障级别信息
    errorLevelOld = errorAttribute.bit.level;
    
    // 存在继续运行的故障
    if ((ERROR_LEVEL_RUN == errorAttribute.bit.level)
        && (errorCode != ERROR_LOSE_LOAD)
        )
    {
        // 故障时继续运行频率选择
        switch (funcCode.code.errorRunFrqSrc)
        {
            // 当前运行频率
            case ERR_RUN_FRQ_RUN:
                if (runOldFrqFlag)
                {
                    frqCurAim = frqOld;
                }
                else
                {
                    frqCurAim = frqAim; 
                }
                break;

            // 设定频率
            case ERR_RUN_FRQ_AIM:
                frqCurAim = frqAim; 
                break;

            // 上限频率
            case ERR_RUN_FRQ_UPPER:
                frqCurAim = upperFrq;
                break;

            // 下限频率
            case ERR_RUN_FRQ_LOWER:
                frqCurAim = lowerFrq;
                break;

            // 备用频率
            case ERR_RUN_FRQ_SECOND:
				
                frqCurAim = ((Uint32)funcCode.code.errorSecondFrq * funcCode.code.maxFrq) / 1000;

				if (frqAim < 0)
				{
					frqCurAim = -frqCurAim;
				}
                break;

            default:
                break;
        }
    }
    else
#endif        
    {
        frqCurAim = frqAim;      // 直流制动之前，就给性能传递 
    }

    
#if (DEBUG_F_POSITION_CTRL)
    AccCalc(frqTmp);     // 求加速度
    //AccCalc(frqRun);
#endif

// runStatus处理
    if (!shuntFlag)      // 没有进入瞬停不停，才处理runStatus
    {
        switch (runStatus)
        {
            case RUN_STATUS_START:  // 启动
                StartRunCtrl();
                break;

            case RUN_STATUS_JOG:    // 点动运行
                JogRunCtrl();
                break;

            case RUN_STATUS_TUNE:   // 调谐运行
                TuneRunCtrl();
                break;

            case RUN_STATUS_DI_BRAKE_DEC:  // 端子制动前减速
                // 目标频率为0
                frqCurAim = 0;              
                AccDecFrqCalc(accFrqTime, decFrqTime, funcCode.code.accDecSpdCurve);
                if (!diFunc.f2.bit.decBrake)
                {
                    runStatus = RUN_STATUS_NORMAL;    
                }
                // 小于直流制动起始频率(零速值)
                else if (ABS_INT32(frqTmp) <= funcCode.code.stopBrakeFrq) 
                {
                    // 进入端子直流制动
                    runStatus = RUN_STATUS_DI_BRAKE;          
                }
                break;
                
            case RUN_STATUS_DI_BRAKE:  // 制动
                frqTmp = 0;
                //runCmd.bit.common0 = 1;
                runFlag.bit.run = 1;
                runFlag.bit.common = 1;
                dspMainCmd.bit.run = 1;
                dspMainCmd.bit.stopBrake = 1;
                if ((!diFunc.f2.bit.brake) // DI端子的直流制动命令变为无效之后，关断
                    && (!diFunc.f2.bit.decBrake)
                    )
                {
                    // 有运行命令恢复为正常运行
                    if (runCmd.bit.common)
                    {
                        dspMainCmd.bit.stopBrake = 0;
                        runStatus = RUN_STATUS_NORMAL;
                    }
                    else
                    {
                        runCmd.bit.common0 = 0;
                        runStatus = RUN_STATUS_SHUT_DOWN;
                    }
                }
                break;

            case RUN_STATUS_LOSE_LOAD: // 掉载
                LoseLoadRunCtrl();
                break;
                
            case RUN_STATUS_WAIT:
                break;
                
            case RUN_STATUS_NORMAL:
                break;
                
            case RUN_STATUS_STOP:
                break;
                
            case RUN_STATUS_SHUT_DOWN:
                break;
                
#if DEBUG_F_POSITION_CTRL
            case RUN_STATUS_POS_CTRL:  // 位置控制
                PcRunCtrl();
                break;
#endif
            default:
                break;
        }

        if (RUN_STATUS_NORMAL == runStatus) // normal run
        {
            NormalRunCtrl();
        }
    }

    if (RUN_STATUS_STOP == runStatus)       // 停机
    {
        StopRunCtrl();
    }

    if (RUN_STATUS_SHUT_DOWN == runStatus)  // shutdown, 关断
    {
        ShutDownRunCtrl();
    }

#if 0
    // 参数辨识后PG卡反馈出错信息
    if (tunePGflag)
    {
        // 为闭环运行
        if (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_FVC)
        {
			// 运行时提示一次
            if (tunePGflag == 1)
            {
                // 未接编码器
                errorOther = ERROR_PG_LOST;
			   tunePGflag = 0;
            }
			// 辨识完提示一次
            else if(tunePGflag == 2)
            {
                // 编码器线数设定错误 
                errorOther = ERROR_PG_PARA_ERROR;
			   tunePGflag = 0;
            }            
        }
        else
        {
            if(tunePGflag == 2)
            {
                tunePGflag = 0;
            }
        }
    }

#endif    
    
#if DEBUG_F_POSITION_CTRL
// 给真正的frq赋值
    if (RUN_STATUS_POS_CTRL == runStatus)
    {
        frq = frqPcOut;
        frqFrac = frqPcOutFrac;

        frqCurAim = frq;    // 位置控制时，目标频率与当前频率一致
        frqCurAimFrac = frqFrac;
        
#if 0   // 位置控制时，超过上限频率+5Hz报警
        if ((frq > (int32)upperFrq + 500) // 限幅，防止错误
            || (frq < -(int32)upperFrq - 500)
            )
        {
            errorOther = 97;
        }
#endif
    }
    else
#endif
    {
        frq = frqTmp;
        frqFrac = frqTmpFrac;

        //pcRef = 0;      // 非位置控制，强制为0
        //pcFdb = 0;
    }

#if 0
    if (RUN_STATUS_TUNE != runStatus)   // 调谐时不受此限制
    {
        if (frq > upperFrq) // 限幅，防止错误
        {
            frq = upperFrq;
            frqTmpFrac = 0;
        }
        else if (frq < -(int32)upperFrq)
        {
            frq = -(int32)upperFrq;
            frqTmpFrac = 0;
        }
    }
#endif    

// 停机记忆，故障引起的停机也要记忆
    if (STOP_REM_WAIT == stopRemFlag) // 由于PLC目前停机后，plcStep重置为0，所以也会到达这里。
    {
        if ((FUNCCODE_frqRemMode_STOP_NO == funcCode.code.frqRemMode)   // 停机不记忆，则upDownFrq置0
            && (dormantStatus != DORMANT_NO_RESPOND)                    // 未处于休眠状态
            )
        {
            upDownFrq = 0;
            frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_OFF;
        }
        stopRemFlag = STOP_REM_NONE;
    }
}



//=====================================================================
//
// 启动控制
//
//=====================================================================
extern Uint16 bFrqCurAimChg;
LOCALF void StartRunCtrl(void)
{
    runFlag.bit.run = 1;
    runFlag.bit.common = 1;
    dspMainCmd.bit.run = 1;

    bFrqCurAimChg = 1;  // 启动

    if (!runCmd.bit.common)     // 启动时，有停机命令
    {
        if (START_RUN_STATUS_HOLD_START_FRQ == startRunStatus)
        {
            runStatus = RUN_STATUS_STOP;
            stopRunStatus = STOP_RUN_STATUS_INIT;
        }
        else
            runStatus = RUN_STATUS_SHUT_DOWN;

        runTimeTicker = 0;  // ticker清零
        
        return;
    }
    
#if DEBUG_F_START_RUN
    if (bAntiReverseRun)    // 反转禁止
    {
        runTimeTicker = 0;
        runStatus = RUN_STATUS_NORMAL;
        return;
    }

    switch (startRunStatus)
    {
        case START_RUN_STATUS_SPEED_TRACK:
            if (FUNCCODE_startMode_SPEED_TRACK == funcCode.code.startMode) // 转速跟踪启动
            {
                // 为同步机或异步机闭环矢量时直接启动
                if ((motorFc.motorPara.elem.motorType == MOTOR_TYPE_PMSM)
                    || (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_FVC)
                    )
                {
                    startRunStatus = START_RUN_STATUS_BRAKE;   // 起动制动
                }
                else
                {
                    dspMainCmd1.bit.speedTrack = 1;

                    if (dspStatus.bit.speedTrackEnd)    // 转速跟踪完成
                    {
                        dspMainCmd1.bit.speedTrack = 0;
                        frqAimOld4Dir = frqRun;         // 转速跟踪时也要考虑正反转死区
                        runStatus = RUN_STATUS_NORMAL;
                    }
                    frqTmp = frqRun;                // 更新当前跟踪的频率
                    break;
                } 
            }
            else  if (FUNCCODE_startMode_DIRECT_START == funcCode.code.startMode)
            {
                startRunStatus = START_RUN_STATUS_BRAKE;   // 起动制动
            }
            else   if (FUNCCODE_startMode_FORE_MAG == funcCode.code.startMode)
            {
                //  VF控制模式
                if (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_VF)
                {
                    startRunStatus = START_RUN_STATUS_BRAKE;   // 起动制动
                }
                else
                {
                    startRunStatus = START_RUN_STATUS_PRE_FLUX;  // 预励磁启动
                }
            }
            
        // 起动制动
        case START_RUN_STATUS_BRAKE:
            if(startRunStatus == START_RUN_STATUS_BRAKE)
            {
                if ((++runTimeTicker >= (Uint32)funcCode.code.startBrakeTime
                     * (Uint16)(TIME_UNIT_START_BRAKE / RUN_CTRL_PERIOD))
                    )
                {    
                    runTimeTicker = 0;
                    dspMainCmd.bit.startBrake = 0;  // 清制动命令
                    startRunStatus = START_RUN_STATUS_HOLD_START_FRQ;
                }
                else
                {
                    dspMainCmd.bit.startBrake = 1;  // 置制动命令
                    break;
                }
            }
            
        // 预励磁启动
        case START_RUN_STATUS_PRE_FLUX:
            if(startRunStatus == START_RUN_STATUS_PRE_FLUX)
            {
                if ((++runTimeTicker >= (Uint32)funcCode.code.startBrakeTime
                     * (Uint16)(TIME_UNIT_START_BRAKE / RUN_CTRL_PERIOD))
    				 )
                {
                    runTimeTicker = 0;
                    dspMainCmd.bit.startFlux = 0;  // 清预励磁命令
                    startRunStatus = START_RUN_STATUS_HOLD_START_FRQ;
                }
                else
                {

                    dspMainCmd.bit.startFlux = 1;  // 置预励磁命令
                    break;
                }
            }
            
        // 起动频率保持    
        case START_RUN_STATUS_HOLD_START_FRQ:
            if (bRunJog)   // 若同时有运行和点动命令，且运行中点动使能，退出启动频率
                break;
            
            // 注意，启动频率保持时间时间为0，则从启动频率开始normal run
            frqTmp = (FORWARD_DIR == runFlag.bit.dirFinal) ? (funcCode.code.startFrq) : (-(int32)funcCode.code.startFrq);
            if (++runTimeTicker >= (Uint32)funcCode.code.startFrqTime
                 * (Uint16)(TIME_UNIT_START_FRQ_WAIT / RUN_CTRL_PERIOD))
            {
                // FVC启动，从当前频率开始
                if (FUNCCODE_motorCtrlMode_FVC == motorFc.motorCtrlMode)
                {
                    // 从当前电机反馈速度开始启动
                    frqTmp = frqFdbTmp;  // frqRun;
                }
                
                runTimeTicker = 0;
                runStatus = RUN_STATUS_NORMAL;
            }
            else
            {
                frqCurAim = frqTmp;  // 更新目标频率
                frqCurAimOld = frqCurAim;
            }

            break;

        default:
            break;
    }
#elif 1

    runTimeTicker = 0;
    runStatus = RUN_STATUS_NORMAL;

#endif
}


//=====================================================================
//
// normal运行控制
//
//=====================================================================
LOCALF void NormalRunCtrl(void)
{
    if (!runCmd.bit.common) // 运行中有停机命令
    {
        //+e if(function.f_code.torque_control&&(!(di_func[3]&0x20)))
        //     frq_set=(int)speed_run*(long)(int)(function.f_code.maxfrq+2000)/0x7fff;
        if (runMode == RUN_MODE_TORQUE_CTRL)
        {
            frq = frqRun;
        }
        
        swingStatus = SWING_NONE;    // 摆频状态清为无摆频状态
        runStatus = RUN_STATUS_STOP;
        stopRunStatus = STOP_RUN_STATUS_INIT;
        return;
    }

// 是否正反转
    if (runFlag.bit.dirReversing) // 正在反向
    {
        frqCurAim = 0;
    }
    else
    {
//+==        frqCurAim = frqAim;

#if DEBUG_F_LOWER_FRQ
        // 判断是否低于下限频率
        LowerThanLowerFrqDeal();
#endif

#if DEBUG_F_SWING
// 判断是否摆频
        if (((frq == frqAim) // 达到设定频率，进入SwingDeal()判断是否需要摆频
             || (SWING_NONE != swingStatus))      // 已经进入摆频
            && (!bAntiReverseRun)                 // 没有处于反转禁止状态
            && (runMode != RUN_MODE_TORQUE_CTRL)  // 不为转矩控制
            )
        {
            SwingDeal();// 摆频
        }
#endif
    }

// 加减速
    if (!diFunc.f1.bit.forbidAccDecSpd)
    {
#if DEBUG_F_SWING
        if (SWING_NONE != swingStatus) // 正在摆频
        {
            AccDecFrqCalc(swingAccTime, swingDecTime, ACC_DEC_LINE);
        }
        else
#endif
        {
            AccDecFrqCalc(accFrqTime, decFrqTime, funcCode.code.accDecSpdCurve);
        }
    }
}


//=====================================================================
//
// 停机控制
//
//=====================================================================
LOCALF void StopRunCtrl(void)
{
    int32 tmp;
    Uint16 enableStopBrake = 1;

    if (runCmd.bit.common)     // 停机时，有启动命令
    {
        if (STOP_RUN_STATUS_WAIT_BRAKE == stopRunStatus)
        {
            //dspMainCmd.bit.run = 1; 不必要
            runStatus = RUN_STATUS_START; //+= 由于停机直流制动等待时间是关断PWM，所以重新开始整个过程
            startRunStatus = START_RUN_STATUS_INIT;
        }
        else
        {
            dspMainCmd.bit.stopBrake = 0;
            runStatus = RUN_STATUS_NORMAL;
        }

        runTimeTicker = 0;  // ticker清零

        return;
    }
    
#if DEBUG_F_STOP_RUN
    switch (stopRunStatus)
    {
        case STOP_RUN_STATUS_DEC_STOP:  // 减速停车
            frqCurAim = 0;              // 目标频率为0
            AccDecFrqCalc(accFrqTime, decFrqTime, funcCode.code.accDecSpdCurve);

            if ((!funcCode.code.ovGain)    // 过压失速，过流失速增益都为0
                && (!funcCode.code.ocGain) // 且为VF运行
                && (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_VF)
                ) 
            {
                tmp = frqTmp;
            }
            else    // 过压失速，过流失速
            {
                tmp = frqRun;
            }
            
            if (((ABS_INT32(tmp) <= funcCode.code.stopBrakeFrq) // 小于直流制动起始频率(零速值)
			    || (ABS_INT32(tmp) < 50))
//                && funcCode.code.stopBrakeFrq                        // 且停机直流制动起始频率不为0
// 停机直流制动起始频率可以为0，例如位能性负载，可能需要在停机时报轴
//                && funcCode.code.stopBrakeCurrent                    // 停机直流制动电流不为0
// 与之前保持一致
//                && funcCode.code.stopBrakeTime                       // 且停机直流制动时间不为0
                && enableStopBrake)                    // DI端子的停机直流制动使能端子有效(默认有效)
            {
                stopRunStatus = STOP_RUN_STATUS_WAIT_BRAKE;          // 进入直流制动等待
                // 直接进入直流制动等待，不用到下一个周期才进入
            }
            else
            {
                if ((!frqTmp)    // 功能给定频率为0，且性能给定的实际转速小于1Hz，关断
                    && (ABS_INT32(tmp) < 50)  // 性能反馈小于_
                    )
                    runStatus = RUN_STATUS_SHUT_DOWN;
                
                break;
            }
            //lint -fallthrough

        case STOP_RUN_STATUS_WAIT_BRAKE:    // 停机直流制动等待
            frqTmp = 0;
            if ((!enableStopBrake) // 停机直流制动使能端子无效，立即退出
                || (++runTimeTicker >= (Uint32)funcCode.code.stopBrakeWaitTime  // 停机直流制动等待时间
                    * (Uint16)(TIME_UNIT_WAIT_STOP_BRAKE / RUN_CTRL_PERIOD)))
            {
                runTimeTicker = 0;
                dspMainCmd.bit.run = 1;
                dspSubCmd.bit.fanRunWhenWaitStopBrake = 0;
                stopRunStatus = STOP_RUN_STATUS_BRAKE;
            }
            else
            {
                dspMainCmd.bit.run = 0;     // 封锁PWM
                dspSubCmd.bit.fanRunWhenWaitStopBrake = 1;  // 停机直流制动等待时间内风扇运行标志
                break;
            }
            //lint -fallthrough
            
        case STOP_RUN_STATUS_BRAKE:     // 直流制动
            if ((!enableStopBrake) // 停机直流制动使能端子无效，立即退出
                || (++runTimeTicker >= (Uint32)funcCode.code.stopBrakeTime // 停机直流制动时间
                    * (Uint16)(TIME_UNIT_STOP_BRAKE / RUN_CTRL_PERIOD)))
            {
                //runTimeTicker = 0;
                runStatus = RUN_STATUS_SHUT_DOWN;
            }
            else
            {
                dspMainCmd.bit.stopBrake = 1;
            }
            
            break;

        default:
            break;
    }
#elif 1
    frqCurAim = 0;              // 目标频率为0
    AccDecFrqCalc(accFrqTime, decFrqTime, funcCode.code.accDecSpdCurve);
    if ((!frqTmp)    // 功能给定频率为0，且性能给定的实际转速小于1Hz，关断
            && (ABS_INT32(frqRun) < 50)  // 性能反馈小于_
        )
        runStatus = RUN_STATUS_SHUT_DOWN;
#endif
}


//=====================================================================
//
// 点动运行控制
//
//=====================================================================
LOCALF void JogRunCtrl(void)
{
#if DEBUG_F_JOG
    runFlag.bit.run = 1;
    runFlag.bit.jog = 1;

    dspMainCmd.bit.run = 1;

    if (runFlag.bit.dirReversing) // 点动引起的反向，正反转死区时间也有效。正在反向
    {
        frqCurAim = 0;
    }
#if 0//+==
    else
    {
        frqCurAim = frqAim;
    }
#endif

    if (!runCmd.bit.jog)
    {
        if (runFlag.bit.jogWhenRun) // 运行中点动，恢复到点动前的运行状态。与runFlag.bit.common无关.
        {
            runFlag.bit.jog = 0;
            runFlag.bit.jogWhenRun = 0;

            runStatus = runStatusOld4Jog;

            return;             // 这时不能使用点动加减速时间。(考虑点动加减速时间为0)
        }
        else
        {
            if (!frqTmp)    // 点动命令取消，降到0频率，才关断PWM
            {
                runStatus = RUN_STATUS_SHUT_DOWN;
            }
            
            frqCurAim = 0;      // 取消点动命令，点动停机
        }
    }

    if (!diFunc.f1.bit.forbidAccDecSpd) // 点动时，加减速禁止也要起作用
    {
        AccDecFrqCalc(funcCode.code.jogAccTime, funcCode.code.jogDecTime, ACC_DEC_LINE);
    }
#endif
}


//=====================================================================
//
// 调谐控制
// 
// 调谐过程中，停机
// 1. 减速停机。且功能码设置的停机直流制动不起作用。
// 2. 若功能码为自由停机，则自由停机
// 3. 若通过端子自由停机，则自由停机
// 4. 若调谐过程中有故障，则自由停机
//
//=====================================================================
#define TUNE_STEP_WAIT      0           // 正在辨识，性能没有任何指令
#define TUNE_STEP_ACC       50          // 辨识过程中加速
#define TUNE_STEP_DEC       51          // 辨识过程中减速
#define TUNE_STEP_END       100         // 辨识结束。保存结果，减速停机。
#define PMSM_TUNE_SPEED     10UL        // 同步机带载调谐速度
Uint16 motorCtrlTuneStatus;             // 参数辨识状态字，性能传递
LOCALF void TuneRunCtrl(void)
{
#if DEBUG_F_TUNE
    Uint16 motorCtrlTuneStatusTmp = motorCtrlTuneStatus;
    static Uint16 stopFlag = 0;

#if 0
    // motorCtrlTuneStatus在调谐之后才更新
    if (runFlag.bit.tune)
    {
        motorCtrlTuneStatusTmp = motorCtrlTuneStatus;
    }
#endif

    runFlag.bit.run = 1;
    runFlag.bit.tune = 1;

    dspMainCmd.bit.run = 1;

    if (++runTimeTicker >= TUNE_RUN_OVER_TIME_MAX * (Uint16)(TIME_UNIT_MS_PER_SEC / RUN_CTRL_PERIOD)) // 限时
    {
        errorOther = ERROR_TUNE;
        return;
    }

    if (saveTuneDataFlag)       // 已经保存参数，进入减速停机状态
    {
        motorCtrlTuneStatusTmp = TUNE_STEP_DEC;
    }

    if (!runCmd.bit.common)     // 调谐命令取消(stop)，减速停车退出调谐。
    {
        stopFlag = 1;
    }
    // 取消调谐
    if (stopFlag)
    {
        motorCtrlTuneStatusTmp = TUNE_STEP_DEC;
    }
	
    switch (motorCtrlTuneStatusTmp)
    {
        case TUNE_STEP_WAIT:
            break;

        case TUNE_STEP_ACC:
            if (MOTOR_TYPE_PMSM == motorFc.motorPara.elem.motorType) 
            {
                // 同步电机带载调谐最高转速为10RPM
                if (motorFc.motorPara.elem.ratingSpeed > PMSM_TUNE_SPEED)
                {
                    frqCurAim = PMSM_TUNE_SPEED * motorFc.motorPara.elem.ratingFrq / motorFc.motorPara.elem.ratingSpeed;  // 同步电机按10RPM转速运行
                }
                else
                {
                    frqCurAim = motorFc.motorPara.elem.ratingFrq;
                }
            }
            else
            {
                frqCurAim = (Uint32)motorFc.motorPara.elem.ratingFrq * 4 / 5; // 电机额定频率的80%
            }

            // 同步电机相序,异步机不需要
            if ((FUNCCODE_runDir_REVERSE == funcCode.code.runDir)
                && (MOTOR_TYPE_PMSM == motorFc.motorPara.elem.motorType))
            {
                frqCurAim = -frqCurAim;
            }
            
            AccDecFrqCalc(accFrqTime, decFrqTime, FUNCCODE_accDecSpdCurve_LINE);
            break;

        case TUNE_STEP_END:
            saveTuneDataFlag = 1;           // 表示已经保存
            //tunePGflag = PGErrorFlag;
            SaveTuneData();                 // 只能保存一次

        case TUNE_STEP_DEC:
            frqCurAim = 0;
            AccDecFrqCalc(accFrqTime, decFrqTime, FUNCCODE_accDecSpdCurve_LINE);
            if (ABS_INT32(frqTmp) < 50)   // 反馈速度已经很低
            {
                if (TUNE_STEP_DEC != motorCtrlTuneStatus)   // 性能传递减速命令，不停机。
                {
                    runCmd.bit.common0 = 0;     // 需要清零。之前调谐版本，没有使用该bit
                    runStatus = RUN_STATUS_SHUT_DOWN;
                    stopFlag = 0;
                }
            }
            break;
            
        default:
            break;
    }
#endif
}


//=====================================================================
// 
// 掉载运行控制
//
//=====================================================================
LOCALF void LoseLoadRunCtrl(void)
{
#if DEBUG_F_LOSE_LOAD

    if ((!runCmd.bit.common) && (!runCmd.bit.jog)) // 有停机命令
    {
        runStatus = RUN_STATUS_STOP;
        stopRunStatus = STOP_RUN_STATUS_INIT;
    }
    // 已经进入掉载保护，仅当性能没有掉载标志时才推出掉载保护。
    // 如，在掉载保护时，修改掉载保护功能无效，不能退出掉载保护
    else  if (!dspStatus.bit.outAirSwitchOff)
    {
        runStatus = runStatusOld4LoseLoad;
    }
    else
    {
        int32 tmp;

        if (GetErrorAttribute(ERROR_LOSE_LOAD) == ERROR_LEVEL_RUN)
        {
            tmp = ((int32)motorFc.motorPara.elem.ratingFrq * 7) / 100; // 电机额定电流的7%，应该高于性能的阈值

            if (ABS_INT32(frqAim) > tmp)    // 设定频率大于阈值
            {
                frqTmp = (FORWARD_DIR == runFlag.bit.dirFinal) ? (tmp) : (-tmp);
            }
            else                            // 否则，为设定频率
            {
                frqTmp = frqAim;
            }
        }
    }
#endif
}


//=====================================================================
// 
// 关断PWM控制
//
//=====================================================================
LOCALF void ShutDownRunCtrl(void)
{
    dspMainCmd.bit.run = 0;         // dspMainCmd.all &= 0xf330;
    dspMainCmd1.bit.speedTrack = 0;
    dspMainCmd.bit.stopBrake = 0;
    dspMainCmd.bit.startBrake = 0;
    dspMainCmd.bit.startFlux = 0;
    dspMainCmd.bit.accDecStatus = 0;
    tuneCmd = 0;
    
//+==    if (!dspStatus.bit.run) // 性能真正的关断了PWM。该标志可以不使用。
    {
        Uint16 pause = runCmd.bit.pause;

        if (runFlag.bit.common)    // common运行停机，包括故障停机
        {
            if (!pause)                                 // 非运行暂停引起的停机
            {
                stopRemFlag = STOP_REM_WAIT;            // 停机标志为1
            }
            else
            {
                stopRemFlag = STOP_REM_PAUSE_JUDGE;     // 运行之后，运行暂停了。
            }
        }

        if (runFlag.bit.common      // common运行停机
            && (!errorCode)         // 非故障停机
//            && (!runFlag.bit.jog) 非点动
            )
        {
            // 非暂停且停机不记忆
            if ((!pause) && (FUNCCODE_plcStopRemMode_REM != (funcCode.code.plcPowerOffRemMode/10)))     // 非运行暂停，清零。目前没有停机记忆。
            {
                plcStep = 0;
                plcTime = 0;
            }
            else
            {
                plcStepRemOld = plcStep;    // 运行暂停，保存plcStep和plcTime
                plcTimeRemOld = plcTime;
            }
        }

        runTimeTicker = 0;      // ticker清零
        lowerDelayTicker = 0;
        shuntTicker = 0;

        runFlag.bit.run = 0;    // 只清一部分标志
        runFlag.bit.common = 0;
        runFlag.bit.jog = 0;
        runFlag.bit.tune = 0;
        runFlag.bit.jogWhenRun = 0;
        runFlag.bit.accDecStatus = 0;
        runFlag.bit.servo = 0;
        
        frqTmp = 0;                 // 运行频率清零
        frqTmpFrac = 0;             // 小数也要清零
        swingStatus = SWING_NONE;   // 摆频初始化为无摆频状态

        frqCurAimOld = 0;           // S曲线使用
        accTimeOld = 0;             // S曲线使用
        decTimeOld = 0;             // S曲线使用
        frqLine.remainder = 0;      // 清零

        runStatus = RUN_STATUS_WAIT; // 停机完成，等待再次启动
    }
}


//=====================================================================
//
// 更新摆频参数
// 
// 更新摆频的:
// 上限值、下限值(swingMaxFrq,swingMinFrq)
// 对应的加减速时间(swingAccTime,swingDecTime),从0到最大频率的加减速时间
// 摆频的跳频幅度(swingJumpFrq)
// 
//=====================================================================
LOCALF void UpdateSwingPara(void)
{
#if DEBUG_F_SWING
    int32 swingAwBase;      // 摆幅的基准频率，摆幅的基准量
    int32 swingAw;
    Uint32 tmp;
    
	swingFrqLimit = 0;
	
    if (FUNCCODE_swingBaseMode_AGAIN_MAXFRQ == funcCode.code.swingBaseMode)
    {
        swingAwBase = maxFrq;               // 正值
    }
    else
    {
        swingAwBase = ABS_INT32(frqAim); // 正值
    }

    swingAw = (swingAwBase * funcCode.code.swingAmplitude) / 1000;  // 正值

    swingMaxFrq = ABS_INT32(frqAim) + swingAw;   // 摆频上限频率，正值
    if (swingMaxFrq > upperFrq)     // 上限频率限制
    {
    	swingFrqLimit = 1;
        swingMaxFrq = upperFrq;     // 正值
    }

    swingMinFrq = ABS_INT32(frqAim) - swingAw;   // 摆频下限频率，正值
    if (swingMinFrq < lowerFrq)     // 下限频率限制。考虑与中心频率方向不同
    {
    	swingFrqLimit = 1;
        swingMinFrq = lowerFrq;     // 正值
    }

    swingJumpFrq = (swingAw * funcCode.code.swingJumpRange) / 1000; // 要有符号，否则中心频率为负时，突跳频率有错误

    if (REVERSE_DIR == runFlag.bit.dirFinal)            // 中心频率为负方向
    {
        swingMaxFrq = -swingMaxFrq; // 负值
        swingMinFrq = -swingMinFrq;
        swingJumpFrq = -swingJumpFrq;
    }

    swingDoubleAw = swingAw << 1;
//    swingDoubleAw = ABS_INT32(swingMaxFrq - swingMinFrq); 目前不处理这种情况: swingMaxFrq,swingMinFrq达到上下限的情况

    tmp = ((Uint32)maxFrq * funcCode.code.swingPeriod)
        / (swingDoubleAw - swingJumpFrq) * TIME_UNIT_SWING_PERIOD / timeBench;
    swingAccTime = (tmp * funcCode.code.swingRiseTimeCoeff) / 1000;
    swingDecTime = tmp - swingAccTime;
#endif
}


//=====================================================================
//
// 摆频处理，更新：
// 摆频状态。
// 
//=====================================================================
LOCALF void SwingDeal(void)
{
#if DEBUG_F_SWING
    static int32 frqAimOld;
    static Uint32 frqDeltaOldMax; // 当frqAim改变幅度 > 刚进入摆频时swingDoubleAw的1/2^5，先退出摆频

    UpdateSwingPara();              // 更新摆频参数

    if (funcCode.code.swingPeriod       // 摆频周期不为0
        && funcCode.code.swingAmplitude // 摆频幅度不为0
        && (!diFunc.f1.bit.swingPause)  // DI端子的摆频暂停无效
        )
    {
        if (SWING_NONE == swingStatus)       // 刚刚进入摆频，设置为上升阶段
        {
            frqAimOld = frqAim;   // 刚进入摆频，更新frqAimOld
            frqDeltaOldMax = swingDoubleAw >> 5;
            //frqDeltaOldMax = 0; //+= 摆频的退出和进入，还可以优化

            swingStatus = SWING_UP;
        }
    }
    else
    {
        swingStatus = SWING_NONE; // 退出摆频
        return;
    }
    
    if (ABS_INT32(frqAimOld - frqAim) > frqDeltaOldMax) // 目标频率发生较大改变，退出摆频     
    {
        swingStatus = SWING_NONE; // 退出摆频
        return;
    }
    
#if 0   // frqAim = 0时，若摆频幅度相对与最大频率，也要进行摆频
    if (!frqAim)
    {
        frqCurAim = 0;
    }
    else
#endif
    if (SWING_UP == swingStatus)    // 摆频的上升段
    {
        frqCurAim = swingMaxFrq;

        if (frqTmp == swingMaxFrq)
        {
            swingStatus = SWING_DOWN;
//            frqCurAim = swingMinFrq;

            frqTmp -= swingJumpFrq;
        }
    }

    if (SWING_DOWN == swingStatus)    // 摆频的下降段
    {
        frqCurAim = swingMinFrq;
        
        if (frqTmp == swingMinFrq)
        {
            swingStatus = SWING_UP;
            frqCurAim = swingMaxFrq;    // 要同时更新，因为在本函数之后才会调用AccDecFrqCalc()函数。
            
            frqTmp += swingJumpFrq;
        }
    }
#endif
}


//=====================================================================
//
// 运行过程中低于下限频率处理
//
//
//=====================================================================
LOCALF void LowerThanLowerFrqDeal(void)
{
#if DEBUG_F_LOWER_FRQ
    if (ABS_INT32(frqAim) < lowerFrq)
    {
        // 低于下限频率延时停机过程中，目标频率也应该是下限频率
        if (!bAntiReverseRun)   // 没有处于反转禁止
        {
            if (FUNCCODE_lowerDeal_RUN_ZERO == funcCode.code.lowerDeal)    // 低于下限频率以零速运行
            {
                frqCurAim = 0;
            }
			// 延时停机或以下限频率运行
			else
			{
                frqCurAim = (FORWARD_DIR == runFlag.bit.dirFinal) ? (lowerFrq) : (-(int32)lowerFrq);
            }
			
        }

        if (FUNCCODE_lowerDeal_DELAY_STOP == funcCode.code.lowerDeal)
        {
            // 还在加速过程中，还没有达到下限频率，发现设定频率低于下限频率，不延时直接停机
            if (ABS_INT32(frq) < lowerFrq)
            {
                lowerDelayTicker = 0;
                otherStopLowerSrc++;     //+==

                runStatus = RUN_STATUS_STOP;
                stopRunStatus = STOP_RUN_STATUS_INIT;
            }
            // 否则，从运行频率达到下限频率开始计时，延时停机
            else if (ABS_INT32(frq) == lowerFrq)
            {
                {
                    lowerDelayTicker = 0;
                    otherStopLowerSrc++;//+==

                    runStatus = RUN_STATUS_STOP;
                    stopRunStatus = STOP_RUN_STATUS_INIT;
                }
            }
        }
        else if (FUNCCODE_lowerDeal_RUN_LOWER == funcCode.code.lowerDeal)
        {
            lowerDelayTicker = 0;
        }
        else
        {
            ;
        }
    }
    else
    {
        lowerDelayTicker = 0;
    }
#endif
}



//=====================================================================
//
// 瞬停不停
//
// 0 - 没有使能，或者已经完成
// 1 - 正在
//
//=====================================================================

#define OFF_THRESHOLD_STOP_DEC_VOL_RANGE  50   // 瞬停减速 停止减速电压阀值
LOCALF void PowerOffNoStopDeal(void)
{
#if DEBUG_F_POWER_OFF_NO_STOP

    Uint16 ratingUdc;           // 220V/380V时的标准(额定)母线电压

    if ((!funcCode.code.pOffTransitoryNoStop)    // 瞬停不停功能码选择
        || ((!runCmd.bit.common) && (!runCmd.bit.jog))) // 无运行/点动命令，不进入瞬停不停
    {
        shuntTicker = 0;
        shuntFlag = 0;

        return;
    }

    runFlag.bit.run = 1;

    ratingUdc = (Uint32)invPara.ratingVoltage * 14482 >> 10;    // 标准母线电压

    // 母线电压低于阈值电压
    // 瞬时停电，当母线电压下降到_时，开始减速以减缓母线电压的下降。
#if DSP_2803X 
    if (generatrixVoltage < (Uint32)funcCode.code.pOffThresholdVol * ratingUdc * 131  >> 17)
#else
    if (generatrixVoltage < (Uint32)funcCode.code.pOffThresholdVol * ratingUdc / 1000)
#endif
    {
        if (!shuntFlag)
        {
            runStatusOld4POffNoStop = runStatus;
        }
        shuntFlag = 1;
        shuntTicker = 0;
    }
    else    // 母线电压高于阈值电压
    {
        if (shuntFlag)          // 处于瞬停不停的电压回升阶段
        {
            if (++shuntTicker >= (Uint32)funcCode.code.pOffVolBackTime
                * (Uint16)(TIME_UNIT_P_OFF_VOL_BACK / RUN_CTRL_PERIOD)) // 电压回升且持续保持_时间，认为瞬停后电压已经恢复正常
            {
                shuntTicker = 0;
                shuntFlag = 0;
                runStatus = runStatusOld4POffNoStop;    // 恢复为瞬停不停前的状态
            }
        }
        else
        {
            shuntTicker = 0;
        }
    }

    if (shuntFlag)          // 正在瞬停不停，减速以减缓母线电压的下降
    {                       // 在电压回升判断时间内，也要减速以减缓母线电压的下降
        frqCurAim = 0;      // 减速的目标频率为0

        // 母线电压超过 瞬停判断电压+0.5*标准母线电压时，停止减速
        // 防止减速过压
        if (generatrixVoltage < ((Uint32)funcCode.code.pOffTransitoryFrqDecSlope * ratingUdc / 1000))
        {
            AccDecFrqCalc(accFrqTime, funcCode.code.decTime4, funcCode.code.accDecSpdCurve);
        }
        
        // 减速到0时停机
        if ((0 == frqRun)
            && (funcCode.code.pOffTransitoryNoStop == 2)  // 停机
			)
        {
            otherStopSrc++;
        }
    }

#endif
}


//=====================================================================
// 
// 加减速时间计算，单位0.01s
// 
//=====================================================================
void AccDecTimeCalc(void)
{
    Uint16 accDecTimeSrc;       // 加减速时间选择
	static Uint16 stop4DecFlag;
    
#if DEBUG_F_ACC_DEC_TIME
    if (RUN_MODE_TORQUE_CTRL == runMode)
    {
        accFrqTime = 0;  // 转矩控制加速时间为0
        decFrqTime = 0;  // 转矩控制减速时间为0
    }
    else
    {
        if ((runFlag.bit.plc)       // PLC运行，且DI端子没有选择选减速时间选择功能
            && (!diSelectFunc.f1.bit.accDecTimeSrc)
            )
        {
            accDecTimeSrc = accDecTimeSrcPlc;
        }
        else
        {
            accDecTimeSrc = diFunc.f1.bit.accDecTimeSrc;
    		
    		// 加减速时间1/2切换
            // 端子未选择加减速时间
            if (!diSelectFunc.f1.bit.accDecTimeSrc)
            {
    			// 加速
    			if (runFlag.bit.accDecStatus == ACC_SPEED)
    			{
    				if (ABS_INT32(frq) >= funcCode.code.accTimefrqChgValue)
    				{
    					accDecTimeSrc = 0;
    				}
    				else
    				{
    					accDecTimeSrc = 1;
    				}
    			}
    			// 减速
    			else if (runFlag.bit.accDecStatus == DEC_SPEED)
    			{
    				if (ABS_INT32(frq) >= funcCode.code.decTimefrqChgValue)
    				{
    					accDecTimeSrc = 0;
    				}
    				else
    				{
    					accDecTimeSrc = 1;
    				}
    			}
    			// 其它
    			else
    			{
    				accDecTimeSrc = 0;
    			}
            }
            
        }

        if ((MOTOR_SN_1 != motorSn) &&      // 非第1电机
            (0 != motorFc.accDecTimeMotor))
        {
            accDecTimeSrc = motorFc.accDecTimeMotor - 1;
        }

    // DI的加减速时间选择端子对PLC也起作用
    // PLC运行。PLC加减速时间有功能码可以选择。在FrqPlcSetDeal()中有计算
        if (3 == accDecTimeSrc)
        {
            accFrqTime = funcCode.code.accTime4;
            decFrqTime = funcCode.code.decTime4;
        }
        else if (2 == accDecTimeSrc)
        {
            accFrqTime = funcCode.code.accTime3;
            decFrqTime = funcCode.code.decTime3;
        }
        else if (1 == accDecTimeSrc)
        {
            accFrqTime = funcCode.code.accTime2;
            decFrqTime = funcCode.code.decTime2;
        }
        else //if (0 == accDecTimeSrc)
#endif
        {
            accFrqTime = funcCode.code.accTime1;
            decFrqTime = funcCode.code.decTime1;
        }
    }
    // 更新加减速时间基准


    // 外部端子停车时以减速时间4为减速时间停机(置标志)
    if (diFunc.f2.bit.stop4dec)
    {
        stop4DecFlag = 1;
    }

    // 停机后紧急停机时间有效标志清零
    if (!runFlag.bit.run)
    {
        stop4DecFlag = 0;
    }

    // 紧急停机时间
    if (stop4DecFlag)
    {
         decFrqTime = funcCode.code.decTime4;
    }


    // 紧急停车时减速时间为0
    if (diFunc.f2.bit.emergencyStop)
    {
        decFrqTime = 0;
    }
    
    UpdateBenchTime();
}

//=====================================================================
// 
// 休眠、唤醒处理
// 
//=====================================================================
void DormantDeal(void)
{
#if DEBUG_F_DORMANT_DEAL    
    int32 tmp = ABS_INT32(frqAimTmp);
    static Uint32 ticker;

    if (!runCmd.bit.common0)                        // 无运行命令
    {
        ticker = 0;
        dormantStatus = DORMANT_RESPOND;            // 退出休眠状态
    }

    if (!runFlag.bit.run)                           // 当前停机
    {
        if (DORMANT_NO_RESPOND != dormantStatus)    // 当前不处于休眠状态
        {
            if (tmp >= funcCode.code.dormantFrq)    // 启动时高于休眠频率(不用高于唤醒频率)，立即响应
            {
                dormantStatus = DORMANT_RESPOND;
            }
            else                                    // 低于休眠频率
            {
                dormantStatus = DORMANT_2;
            }
        }
        else                                        // 当前处于休眠状态，要高于唤醒频率才响应
        {
            if (tmp >= funcCode.code.wakeUpFrq)     // 高于唤醒频率
            {
                if (++ticker >= (Uint32)funcCode.code.wakeUpTime * TIME_UNIT_WAKE_UP / RUN_CTRL_PERIOD)
                {
                    dormantStatus = DORMANT_RESPOND;// 退出休眠状态

                    ticker = 0;
                }
            }
            else
            {
                ticker = 0;
            }
        }
    }
    else if ((runFlag.bit.common) && (DORMANT_NO_RESPOND != dormantStatus))   // 当前处于运行状态(非点动、非调谐)
    {
        if (tmp < funcCode.code.dormantFrq)             // 低于休眠频率
        {
            if (++ticker >= (Uint32)funcCode.code.dormantTime * TIME_UNIT_DORMANT / RUN_CTRL_PERIOD)
            {
                dormantStatus = DORMANT_NO_RESPOND;     // 进入休眠状态

                ticker = 0;
            }
        }
        else
        {
            ticker = 0;
        }
    }
#endif    
}



#if DEBUG_F_POSITION_CTRL
//=====================================================================
//
// 加速度计算
// 
// 
//
//=====================================================================
#define ACCEL_NUM     10
int32 accelArray[ACCEL_NUM];
int32 accelTickerArray[ACCEL_NUM];
int32 accel;    // 加速度，已经乘以2^8了
Uint16 accelDisp;
void AccCalc(int32 frq0)
{
    int16 i;
    int32 accelSum;
    static int32 frqOld;
    static int16 k;
    int16 tickerSum;
    static Uint16 runOld;

    // 开始运行时保存数据，且全部清零
    if ((!runOld) && (runFlag.bit.run))
    {
        for (i = ACCEL_NUM - 1; i >= 0; i--)    // 清零
        {
            accelArray[i] = 0;
            accelTickerArray[i] = 0;
        }
        
        k = 0;
        frqOld = 0;
        accel = 0;  // 加速度也要清零
        accelSum = 0;
        tickerSum = 0;
    }
    runOld = runFlag.bit.run;

    if (!runFlag.bit.run)
    {
        accel = 0;
        return;
    }

    k++;
    if (frq0 != frqOld) // frq改变
    {
        accelSum = 0;
        tickerSum = 0;
        for (i = ACCEL_NUM - 1; i > 0; i--)
        {
            accelArray[i] = accelArray[i-1];
            accelSum += accelArray[i];
            
            accelTickerArray[i] = accelTickerArray[i-1];
            tickerSum += accelTickerArray[i];
        }
        accelArray[0] = frq0 - frqOld;
        accelSum += accelArray[0];
        accelTickerArray[0] = k;
        tickerSum += accelTickerArray[0];
        
        accel = (accelSum << 8) / tickerSum;

        k = 0;
    }
    else if (k >= 200)
    {
        accel = 0;
        //k = 0;

        for (i = ACCEL_NUM - 1; i >= 0; i--)    // 清零
        {
            accelArray[i] = 0;
            //accelTickerArray[i] = 0;
        }
    }

    accelDisp = accel * 500 >> 8;

#if 0
    if (ABS_INT32((int16)accelDisp) > 900)
    {
        asm(" nop");
    }
#endif

#if 0
    if (ABS_INT32(accel) < 100)
    {
        for (i = ACCEL_NUM - 1; i >= 0; i--)    // 清零
        {
            accelArray[i] = 0;
            accelTickerArray[i] = 0;
        }
        accelArray[0] = accel * ACCEL_NUM;
        accelTickerArray[0] = 1;
    }
#endif

#if 0
    if (ABS_INT32(frqTmp) < 50)
    {
        accel = accelArray[0];
    }
#endif

    frqOld = frq0;
}
#endif














