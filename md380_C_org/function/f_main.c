//======================================================================
//
// 功能主文件。包括:
// 1、上电时功能的初始化，主要为功能码的读取
// 2、2ms,0.5ms,0ms的功能函数
// 3、故障处理
// 4、性能、功能的交互
// 5、电机参数的更新
//
// Time-stamp: <2012-03-14 14:08:23  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_main.h"
#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_eeprom.h"
#include "f_io.h"
#include "f_menu.h"
#include "f_ui.h"
#include "f_comm.h"
#include "f_posCtrl.h"
#include "f_osc.h"
#include "f_canlink.h"
#include "f_p2p.h"


#define PRODUCT_VERSION      38000       // 产品版本号
#define SOFTWARE_VERSION     73    // 软件版本0.73


#if F_DEBUG_RAM

#define DEBUG_F_FC_INIT_OVER        0   // 功能码读取ok处理

#define DEBUG_F_CHK_FUNC_CODE       0   // 检查功能码是否有错误，同时自动获得表格，eeprom2Fc[], anybus2Fc[]

#elif 1

#define DEBUG_F_FC_INIT_OVER        1

//----以下为调试---------
#define DEBUG_F_CHK_FUNC_CODE       0  // 检查功能码是否有错误，同时自动获得表格，eeprom2Fc[], anybus2Fc[]

#endif



// AIAO出厂校正是否完成
#define AIAO_CHK            (0x5A5A)



Uint16 memoryValue;
void getMomeryValue(void);
#define MEMORY_ADDRESS funcCode.code.memoryAddr  // 查看内存地址



#if 0
Uint16 dataDigit;  // 默认为是5位
Uint16 kk;
Uint16 jj;
Uint16 upper1;
Uint16 lower1;
Uint16 index;
#endif


#define START_DSP_DELAY_TIME_MS     300     // 启动延时时间，ms

#define WAIT_POWER_ON_TIME          10000       // 进入主程序之后，等待_*0.5ms开始响应欠压处理和运行命令
enum POWER_ON_STATUS powerOnStatus;             // 上电时的状态

enum FUNC_CODE_INIT_STATUS
{
    FUNCCODE_INIT_STATUS_VERIFY_EEPROM,
    FUNCCODE_INIT_STATUS_NEW_EEPROM,
    FUNCCODE_INIT_STATUS_OLD_EEPROM,
    FUNCCODE_INIT_STATUS_OVER
};
LOCALF enum FUNC_CODE_INIT_STATUS funcCodeInitStatus;  // 功能码的初始化过程


Uint16 driveCoeff[2][4];

extern Uint32 allTime;
extern Uint32 upTimes;

//-------------------------------------------
union MOTOR_PARA motorPara;
struct MOTOR_FC motorFc;            // 第1/2/3/4电机功能码，包括电机参数、控制参数
enum MOTOR_SN motorSn;              // 电机选择，目前第1/2/3/4电机
extern void UpdateMotorPara(void);
//-------------------------------------------

Uint16 aiaoChckReadOK;          // 读取的AIAO校正出厂值正常

Uint16 mainLoopTicker;
// 用来判断功能部分是否已经接收到性能传递的AI采样值。
// 0-未进入主循环，或者，功能部分进入主循环的第1拍还没有完成。
// 1-功能部分进入主循环的第1拍已经完成。
// 2-功能部分，已经接收到性能传递的AI采样值。之后，可以正常使用AI。
// 
// 现象:
//
// 若频率源为AI，命令源为端子两线式1，启动频率非零。运行命令端子上电前就有效。
// 则上电后不能启动，必须运行命令端子先无效再有效，才能启动。
//
// 原因:
//
// 进入主循环之后，性能才传递AI的采样值，直到功能的第2拍才传递。
// 进入主循环的第1拍，由于性能没有传递AI采样值，于是设定频率为0，低于启动频率。
// 而低于启动频率/下限频率时，基于安全考虑，会有启动保护，就不能启动。
//
// 处理:
//
// 在功能接收到性能传递的AI时(进入主循环的第2拍)，AI不滤波(即AI初值直接赋值)，
// 这样可避免受到AI滤波时间的影响；AI的采样值也会立即更新为设定值，
// 显示上也没有渐变过程。
// 


extern void InitSetFvcIndexIntEQep(void);

LOCALD void FuncVariableInit(void);


LOCALD void InitFuncCode(void);
void InitDspFunc(void);


// 更新U0组显示数据
extern void UpdateU0Data(void);

extern void SoftVersionDeal(void);


extern void UpdateDataCore2Func(void);
extern void UpdateDataFunc2Core0p5ms(void);
extern void UpdateDataFunc2Core2ms(void);
extern void TemperatureDeal(void);


Uint32 timeBase;
Uint32 timeBase05msOld;
Uint32 ticker05msFuncInternal;  // 两个0.5ms函数之间的时间
Uint32 ticker05msFuncIn[4];
Uint16 ticker05msFuncInternalDisp[4];

Uint32 ticker05msFuncEach;      // 每个0.5ms调用函数的执行时间
Uint32 ticker05msFuncEa[4];
Uint16 ticker05msFuncEachDisp[4];
void CodeRunMonitor(void);

void Main05msFunctionEachStart(void);
void Main05msFunctionA(void);
void Main05msFunctionB(void);
void Main05msFunctionC(void);
void Main05msFunctionD(void);
void Main05msFunctionEachEnd(void);



//=====================================================================
//
// 变频器功能部分初始化，包括：
// 功能涉及到的片内外设
// 功能码初始化
// 参数初始化
//
//=====================================================================
void InitForFunctionApp(void)
{
#if DEBUG_F_CHK_FUNC_CODE
    extern void ValidateFuncCode(void);

// 判断功能码的 个数、属性个数等是否一致，
// 判断功能码的EEPROM地址，anybus地址是否有错误
// 自动获取 eeprom2Fc[] , anybus2Fc[] 数组.
// 仅调试时使用
    ValidateFuncCode();
#endif

// 初始化功能涉及到的片内外设
    InitDspFunc();

// 初始化功能码
    InitFuncCode();

// 某些初始化应该在功能码正常读取之后进行

// 参数初始化
    FuncVariableInit();
}


//=====================================================================
//
// 初始化功能涉及到的片内外设
//
//=====================================================================
extern void I2cDealBeforeInit(void);
void InitDspFunc(void)
{
    EALLOW;
#if DSP_2803X       // 2803x还是2808平台
    SysCtrlRegs.PCLKCR0.bit.ECANAENCLK = 1;    // eCAN-A
    SysCtrlRegs.PCLKCR1.bit.EPWM4ENCLK = 1;    // ePWM4
    SysCtrlRegs.PCLKCR1.bit.EPWM5ENCLK = 1;    // ePWM5
    SysCtrlRegs.PCLKCR1.bit.EPWM6ENCLK = 1;    // ePWM5
    SysCtrlRegs.PCLKCR1.bit.ECAP1ENCLK = 1;    // eCAP1
    SysCtrlRegs.PCLKCR0.bit.LINAENCLK = 1;     // LIN-A
#else
    SysCtrlRegs.PCLKCR1.bit.ECAP3ENCLK = 1;     // eCAP3
    SysCtrlRegs.PCLKCR1.bit.ECAP4ENCLK = 1;     // eCAP4
    SysCtrlRegs.PCLKCR1.bit.EQEP2ENCLK = 1;     // eQEP2
    SysCtrlRegs.PCLKCR0.bit.ECANAENCLK = 1;     // eCAN-A
#endif

   
    EDIS;
    
// SPI，显示、键盘、DIDO
    InitSpiaGpio();
    InitSetSpiForFunc();

// EEPROM，24LC08/24LC32
    I2cDealBeforeInit();    // 若在操作EEPROM时DSP复位，I2C需要处理
    InitI2CGpio();
    InitSetI2ca();  

// SCI
#if 1
    InitSciaGpio();
    InitSetScia();
#elif 1
    InitScibGpioDp();
    InitSetSciDp(&sciM380DpData);
#endif

// AO1
#if DSP_2803X
    InitEPwm6Gpio(); 
    InitSetEPWM6();  
#else
    InitECap3Gpio();
    InitSetEcap3();
#endif


#if DSP_2803X
// DI/DO
    InitDIGpio();
    InitDOGpio();
#endif

// HDI
#if DSP_2803X
    InitECap1Gpio();
    InitSetEcap1();
#else
    InitECap4Gpio();
    InitSetEcap4();
#endif

// PULSE OUT, AO2，DO3(FM)
#if DSP_2803X
    InitEPwm4Gpio(); // AO2
    InitSetEPWM4();  // AO2
    InitEPwm5Gpio(); // HDO1/FM
    InitSetEPWM5();  // HDO1/FM
#else
    InitECap2Gpio();
    InitSetEcap2();
#endif
    
// CAN

}

//=====================================================================
//
// 功能码初始化，且AI、DI、PulseIn开始更新
//
// 1。刚开始，EEPROM读写正在进行。
// 2。在输入端子(F2组)功能码的读写没有完成时，所有DI、AI、PulseIn的计算无效。
// 3。当输入端子功能码的读写完成时，DI、AI、PulseIn的计算就有效。
// 4。由于功能码的读写是从0开始，所以，从输入端子功能码的读写完成，到
//    全部功能码完成的这段时间，足够DI、AI、PulseIn的更新。
//
//=====================================================================
LOCALF void InitFuncCode(void)
{
    Uint32 baseTime;
    Uint16 readAIAOChck = AIAO_CHK_CHECK;
	Uint16 fcIndex;
    Uint16 readIndex;
    aiaoChckReadOK = AIAO_CHK_READ_NULL;
//    Uint16 initTicker = 0;

    baseTime = GetTime();    
    while ((!errorEeprom)   // 发生EEPROM故障，立即退出读取进入主循环，显示err
           && ((FUNCCODE_INIT_STATUS_OVER != funcCodeInitStatus)
//               || (initTicker < START_DSP_DELAY_TIME_MS / 2)
               )
           )
    {
        switch (funcCodeInitStatus)
        {
            case FUNCCODE_INIT_STATUS_VERIFY_EEPROM: // 判断EEPROM是否第一次上电使用，EEPROM校验字1 和 EEPROM校验字2
                funcCodeRead.index = GetEepromIndexFromFcIndex(EEPROM_CHECK_INDEX1);
                funcCodeRead.number = 2;
                EepromDeal();   // 这个地方仅仅是I2C总线状态处理
                if (FUNCCODE_READ_RET_OK == ReadFuncCode(&funcCodeRead))
                {
                    if ((EEPROM_CHECK1 == funcCodeRead.data[0])
                        && (EEPROM_CHECK2 == funcCodeRead.data[1]))
                    {
                        funcCodeInitStatus = FUNCCODE_INIT_STATUS_OLD_EEPROM; // EEPROM已经使用
                    }
                    else
                    {
                        funcCodeInitStatus = FUNCCODE_INIT_STATUS_NEW_EEPROM; // EEPROM第一次使用
                        readAIAOChck = AIAO_CHK_CHECK;
                    }
                }
                break;

            case FUNCCODE_INIT_STATUS_NEW_EEPROM:
                // 读取AIAO出厂校正值
                if (readAIAOChck == AIAO_CHK_CHECK)
                {
                    // 读取AIAO校正字值
                    readIndex = GetEepromIndexFromFcIndex(AI_AO_CHK_FLAG);
                    funcCodeRead.index = readIndex;
                    funcCodeRead.number = 1;
                    EepromDeal();
                    if (FUNCCODE_READ_RET_OK == ReadFuncCode(&funcCodeRead))
                    {
                        // 为0XA5表示已进行工装校正
                        if (funcCodeRead.data[0] == AIAO_CHK_WORD)
                        {
                            // 读取工装校正值
                            readAIAOChck = AIAO_CHK_START;
                            readIndex = GetEepromIndexFromFcIndex(AI_AO_CALIB_START);
                        }
                        else
                        {
                            // 直接开始初始化EEPROM
                            readAIAOChck = AIAO_CHK_END;
                        }
                    }
                }
                else if (readAIAOChck == AIAO_CHK_START)
                {
                    static Uint16 flag = 0;
                    funcCodeRead.index = readIndex;
                    funcCodeRead.number = 2;
                    EepromDeal();
                    if (FUNCCODE_READ_RET_OK == ReadFuncCode(&funcCodeRead))
                    {
						fcIndex = GetFcIndexFromEepromIndex(readIndex);
                        funcCode.all[fcIndex] = funcCodeRead.data[0];
                        funcCode.all[fcIndex+1] = funcCodeRead.data[1];
                        
                        readIndex += 2;
                        flag++;

                        // AI3不需要校正
                        if (flag == 4)
                        {
                            readIndex += 4;
                        }

                        // 校正到AO1时结束
                        if (readIndex > GetEepromIndexFromFcIndex(AI_AO_CALIB_STOP))
                        {
                            aiaoChckReadOK = AIAO_CHK_READ_OK;  // 校验值正确
                            readAIAOChck = AIAO_CHK_END;        // 结束读取
                        }
                    }
                }
                else //if (readAIAOChck == AIAO_CHK_END)
                {
                    funcCodeRwModeTmp = FUNCCODE_RW_MODE_WRITE_ALL;
                    EepromDeal();
                    if (!funcCodeRwModeTmp)  // 全部功能码和掉电保存变量出厂值写入到EEPROM中
                    {
                        funcCodeInitStatus = FUNCCODE_INIT_STATUS_OVER;

                        // 存储EEPROM校验、AIAO校验
                        funcCode.code.eepromCheckWord1 = EEPROM_CHECK1;
                        funcCode.code.eepromCheckWord2 = EEPROM_CHECK2;
                        funcCode.code.aiaoChkWord = FUNCCODE_READ_RET_OK;
                        SaveOneFuncCode(EEPROM_CHECK_INDEX1);
                        SaveOneFuncCode(EEPROM_CHECK_INDEX2);
                        SaveOneFuncCode(AI_AO_CHK_FLAG);
                    }
                }
                break;

            case FUNCCODE_INIT_STATUS_OLD_EEPROM:
                funcCodeRwModeTmp = FUNCCODE_RW_MODE_READ_ALL;
                EepromDeal();
                if (!funcCodeRwModeTmp)  // 全部功能码和掉电保存变量已经读到RAM中。
                {
                    funcCodeInitStatus = FUNCCODE_INIT_STATUS_OVER;
                }
                break;

            default:
                break;
        } // switch (funcCodeInitStatus)

        PulseInSample();

        if (baseTime - GetTime() >= (TIME_1MS << 1)) // 2ms
        {
            spiData.out.a.bit.dispCode = (displayBuffer[scanStep] & 0x00ff); // 显示初始化的值, -H-C-
            DisplayScanPrepare();       // 显示扫描准备

//            initTicker++;
            baseTime = GetTime();
            //baseTime -= (TIME_1MS << 1);

            EepromOperateTimeDeal();
            
            UpdateDataCore2Func();      // 需要性能传递AI的采样值
            DisplayScan();              // 显示扫描
        }
    }

// 软件版本处理
    SoftVersionDeal();

#if DEBUG_F_FC_INIT_OVER
// 判断EEPROM中的值是否在上下限之内
    if (FUNCCODE_INIT_STATUS_OVER == funcCodeInitStatus) // 读取功能码完毕
    {
        int16 i;
        Uint16 upper, lower;
        Uint16 tmp;
        int16 j;        // limitedByOtherCodeIndex[]的下标
		const FUNCCODE_ATTRIBUTE *maxFrqAttribute = &funcCodeAttribute[MAX_FRQ_INDEX];

        // 生成 limitedByOtherCodeIndex 表
        j = 0;          // limitedByOtherCodeIndex[]的下标
        for (i = FNUM_PARA - 1; i >= 0; i--)    // 没有包括掉电保存和故障记录
        {
            const FUNCCODE_ATTRIBUTE *pAttribute = &funcCodeAttribute[i];

            if (pAttribute->attribute.bit.upperLimit)
            {
                limitedByOtherCodeIndex[j++] = i;

                upper = funcCode.all[pAttribute->upper];
            }
            else
            {
                upper = pAttribute->upper;
            }

            if (pAttribute->attribute.bit.lowerLimit)
            {
                // 若上限受其他功能码限制，limitedByOtherCodeIndex前面已经处理
                if (!pAttribute->attribute.bit.upperLimit)
                {
                    limitedByOtherCodeIndex[j++] = i;
                }

                lower = funcCode.all[pAttribute->lower];
            }
            else
            {
                lower = pAttribute->lower;
            }

            tmp = LimitDeal(pAttribute->attribute.bit.signal, funcCode.all[i], upper, lower, 0);
            if (funcCode.all[i] != tmp)
            {
                funcCode.all[i] = tmp;
                SaveOneFuncCode(i);
            }

            // 读取功能码完毕之后，更新IO口的输入状态
            DiCalc();
            PulseInCalc();
            AiCalc();
        }
        limitedByOtherCodeIndexNum = j; // 保存上下限受其他功能码限制的功能码总数

		// 最大频率的下限值特殊处理
        upper = maxFrqAttribute->upper;
        lower = 50 * decNumber[funcCode.code.frqPoint];
        tmp = LimitDeal(maxFrqAttribute->attribute.bit.signal, funcCode.all[MAX_FRQ_INDEX], upper, lower, 0);
        if (funcCode.all[MAX_FRQ_INDEX] != tmp)
        {
            funcCode.all[MAX_FRQ_INDEX] = tmp;
            SaveOneFuncCode(MAX_FRQ_INDEX);
        }

    }
    else
#endif
        if (errorEeprom)   // EEPROM故障
    {
        errorCode = ERROR_EEPROM;
        UpdateErrorDisplayBuffer();

        for (;;)    // 上电时EEPROM读写错误，一直显示err，不进入主循环
        {
            if (baseTime - GetTime() >= (TIME_1MS << 1))
            {
                spiData.out.a.bit.dispCode = (displayBuffer[scanStep] & 0x00ff);
                DisplayScanPrepare();

                baseTime = GetTime();
                PulseInCalc();  // 仅起延时作用
                AiCalc();       // 延时

                DisplayScan();
            }
        }
    }
}


//=====================================================================
//
// 功能部分的变量初始化
//
//=====================================================================
LOCALF void FuncVariableInit(void)
{
#if DEBUG_F_MOTOR_FUNCCODE
    int16 i;
#endif

    funcCode.code.inverterGpTypeDisp = funcCode.code.inverterGpType; // F0-00, GP类型显示

    UpdateInvType();    // 获得实际机型(变频器功率)，变频器额定电压

    frqCalcSrcOld = funcCode.code.frqCalcSrc;


//=====================================================================
// 掉电保存参数的恢复

// 数字设定频率UP/DOWN掉电记忆
    // X或者Y为掉电记忆
    if ((funcCode.code.frqXSrc == FUNCCODE_frqXySrc_FC_P_OFF_REM) ||
        (funcCode.code.frqYSrc == FUNCCODE_frqXySrc_FC_P_OFF_REM))
    {
        upDownFrqInit = (int16)funcCode.code.upDownFrqRem;
    }
    else
    {
        upDownFrqInit = 0;
    }

// 同步机转子位置
    pmsmRotorPos = funcCode.code.pmsmRotorPos;

    frqAimTmp0 = funcCode.code.presetFrq; // 目前不需要初始化，因为FrqSetAimSrcDeal()根据presetFrqOld会计算
//    presetFrqOld = funcCode.code.presetFrq;

// PLC掉电记忆
    if (FUNCCODE_plcPowerOffRemMode_REM == (funcCode.code.plcPowerOffRemMode%10)) // PLC掉电记忆
    {
        plcStep = funcCode.code.plcStepRem;
        plcStepRemOld = plcStep;

        plcTime = ((Uint32)funcCode.code.plcTimeHighRem << 16) + funcCode.code.plcTimeLowRem;
        plcTimeRemOld = plcTime;
    }

    //+= funcCode.code.extendType = ;
//=====================================================================


#if DEBUG_F_POSITION_CTRL
    InitPosCtrl();
#endif

#if 0
    displayBuffer[5] = LED_CODE[LED_NULL];          // LED全灭
    //displayBuffer[5] = LED_CODE[LED_ALL];          // LED全亮
    spiData.out.a.bit.dispCode = (displayBuffer[scanStep] & 0x00ff); // 重新更新显示段码。否则，由于在功能主函数中，
                                                    // 显示缓冲和显示扫描的先后顺序，scanStep更新时，
                                                    // curDisplayCode可能没有更新，于是显示乱码。
#elif 0
    displayBuffer[0] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[1] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[2] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[3] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[4] = DISPLAY_CODE[DISPLAY_NULL];
    displayBuffer[5] = LED_CODE[LED_NULL];
    scanStep = 5;
    spiData.out.a.bit.dispCode = (displayBuffer[scanStep] & 0x00ff);
#elif 1
    scanStep = 5;
#endif

    dspMainCmd1.bit.shortGnd = funcCode.code.shortCheckMode;    // 上电对地短路保护功能
    dspMainCmd1.bit.pgLocation = motorFc.pgPara.elem.fvcPgSrc;

#if DEBUG_F_MOTOR_FUNCCODE
#if !F_DEBUG_RAM

    // 性能的掉电记忆
    for (i = REM_P_OFF_MOTOR - 1; i >= 0; i--)  // 性能需要的掉电记忆参数
    {
        gSendToFunctionDataBuff[i + MOTOR_TO_Func_2MS_DATA_NUM] = funcCode.code.remPOffMotorCtrl[i];
    }  
#endif
#endif

    UpdateDataFunc2Core0p5ms();           // 在到达主循环之前更新交互数据
    UpdateDataFunc2Core2ms();

#if 0
//===========================
    //for (i = FNUM_PARA - 1; i >= 0; i--)    // 没有包括掉电保存和快捷菜单
    for (i = 0; i < FNUM_PARA; i++)
    {
        kk = 0;
        index = i;
#if 0
        upper = funcCodeAttribute[index].upper;
        if (funcCodeAttribute[index].attribute.bit.upperLimit)
        {
            index = upper;
            upper = funcCodeAttribute[index].upper;
        }
#elif 1
        while (funcCodeAttribute[index].attribute.bit.upperLimit)
        {
            index = funcCodeAttribute[index].upper;
            if (++kk >= 5)
            {
                asm(" nop");
                break;
            }
        }
        upper1 = funcCodeAttribute[index].upper;
        asm(" nop");
#endif

        if (upper1 >= 10000)
            dataDigit = 5;
        else if (upper1 >= 1000)
            dataDigit = 4;
        else if (upper1 >= 100)
            dataDigit = 3;
        else if (upper1 >= 10)
            dataDigit = 2;
        else
            dataDigit = 1;

        if (funcCodeAttribute[index].attribute.bit.point >= dataDigit) // 至少显示到小数点位置
            dataDigit = funcCodeAttribute[index].attribute.bit.point + 1;

        if (dataDigit != funcCodeAttribute[index].attribute.bit.displayBits)
        {
            jj++;
            asm(" nop");
        }
    }
    asm(" nop");
#endif

    //ECap3Regs.TSCTR= 0;

// 菜单初始化
    MenuInit();
}


//=====================================================================
//
// 变频器功能处理函数
//
// 说明:
// 1. ErrorDeal()放在DO之前，DO放在ErrorReset()之前，
// 可以避免，当errorCode一直存在，又不停故障复位时，
// DO的error输出一会有一会没有的情况。
//
// 2. ErrorReset()放在UpdateDisplayBuffer()之后。
// 可以避免，当errorCode一直存在，又进行故障复位时，
// 不必要的显示闪烁。
// 
// 3. ErrorDeal()放在RunSrcDeal()之后，处理欠压时运行报警的问题。
//
// 4. DisplayScanPrepare()与DisplayScan()之间，大约有1800个SYSCLK，同时起了
// SPI接收和发送之间(RCK从0变为1)的延时作用。
// RCK的切换需要延时一段时间(1us实验OK)。
//
//=====================================================================
void Main2msFunction(void)
{
//    DINT; // 测试使用
    

//    EINT;
}


//=====================================================================
//
// 变频器功能处理函数(0ms调用周期)
//
//=====================================================================
void Main0msFunction(void)
{
// PulseInSample()可以放在100us的循环中
	PulseInSample();            // PULSE IN的采样。计算在PulseInCalc().
	
//    SciDpDeal(&sciM380DpData);  //modefied by sjw 2009-12-24

#if DEBUG_F_POSITION_CTRL
    pulseInSamplePcQep();
#endif
}

#if 0
//=====================================================================
//
// 变频器功能处理函数(0.5ms调用周期)
//
//=====================================================================
Uint32 allTime;
Uint32 upTimes;
Uint32 maxTime;
Uint32 minTime;
void Main05msFunction(void)
{
    static Uint16 ticker;
    Uint32 timeBase;

    timeBase = GetTime();
    ticker05msFuncInternal = timeBase05msOld - timeBase;
    timeBase05msOld = timeBase;

    //Main05msFunctionEachStart();    // 每个0.5ms都调用
    
    if (0 == ticker)
    {   
        Main05msFunctionA();        // 2ms中的第0个0.5ms，即0ms-0.5ms
    }
    else if (1 == ticker)
    {
        Main05msFunctionB();        // 2ms中的第1个0.5ms，即0.5ms-1.0ms
    }
    else if (2 == ticker)
    {
        Main05msFunctionC();        // 2ms中的第2个0.5ms，即1.0ms-1.5ms
    }
    else if (3 == ticker)
    {
        Main05msFunctionD();        // 2ms中的第3个0.5ms，即1.5ms-2.0ms     
    }
    Main05msFunctionEachEnd();      // 每个0.5ms都调用
    
    ticker05msFuncEach = timeBase - GetTime();

    if (3 == ticker)
    {
        allTime = 0;
        allTime += ticker05msFuncEa[0]; 
        allTime += ticker05msFuncEa[1]; 
        allTime += ticker05msFuncEa[2]; 
        allTime += ticker05msFuncEa[3];
		
		if (maxTime < allTime)
		{
			maxTime = allTime;
		}

		if (minTime > allTime)
		{
			minTime = allTime;
		}
    }
    

    ticker05msFuncEa[ticker] = ticker05msFuncEach;
    ticker05msFuncIn[ticker] = ticker05msFuncInternal;

    ticker++;
    if (ticker >= 4)
    {
        ticker = 0;
    }
}

#endif



// 0.5ms调用
// 2ms中的每个0.5ms都调用，放在最前
void Main05msFunctionEachStart(void)
{
    static Uint16 waitPowerOnTicker;

    timeBase = GetTime();
    ticker05msFuncInternal = timeBase05msOld - timeBase;
    timeBase05msOld = timeBase;
    
    if ((dspStatus.bit.runEnable)                   // 性能初始化完成
        && (dspStatus.bit.uv)                       // 且不欠压
        )
    {
        powerOnStatus = POWER_ON_CORE_OK;           // 上电准备OK
    }

    if (++waitPowerOnTicker >= WAIT_POWER_ON_TIME)  // 进入主循环的时间达到_时间
    {
        if (POWER_ON_WAIT == powerOnStatus)
        {
            powerOnStatus = POWER_ON_FUNC_WAIT_OT;  // 功能的等待时间超时
        }
    }
    
    commTicker++;                   // 串口的停顿时间

    canLinkTicker++;                // 

   	EepromDeal();                   // EEPROM处理，即功能码的读写
}



// 0.5ms调用 750
// 2ms中的每个0.5ms都调用，放在最后
void Main05msFunctionEachEnd(void)
{
	static Uint16 ticker = 0;
    
    ticker05msFuncEach = timeBase - GetTime();
    ticker05msFuncEa[ticker] = ticker05msFuncEach * 3413 >> 11;     // * 10 / 6
    ticker05msFuncIn[ticker] = ticker05msFuncInternal* 3413 >> 11;  // * 10 / 6
    
    ticker++;
    if (ticker >= 4)
    {
        ticker = 0;
    }
}


// 0.5ms调用
// 2ms中的第0个0.5ms，即0ms-0.5ms
extern void vfSeparateDeal(void);
void Main05msFunctionA(void)
{
    asm(" nop");

    Main05msFunctionEachStart();    // 每个0.5ms都调用
    
    DisplayScanPrepare();       // 130.准备显示扫描

    UpdateDataCore2Func();      // 680.更新性能传递给功能的交互数据(即时传递)
	
    DiCalc();                   // 2500
    PulseInCalc();              // 820

    Main05msFunctionEachEnd();
}



// 0.5ms调用
// 2ms中的第1个0.5ms，即0.5ms-1.0ms
void Main05msFunctionB(void)
{
    Main05msFunctionEachStart();    // 每个0.5ms都调用
    
    AiCalc();                   // 2200

    UpdateMotorPara();          // 更新电机参数 (600)
    
#if DEBUG_F_POSITION_CTRL
    pulseInCalcPcEQep();        // 扩展PG卡的脉冲频率采样
    PcMiscDeal();               // 位置控制的一些杂项处理
#endif

    getMomeryValue();
    
    DisplayScan();              // 显示扫描。与DisplayScanPrepare()之间的程序，同时起了延时的作用。

    KeyScan();                  // 键盘扫描(67)
    KeyProcess();               // 键盘(菜单)处理(47)
    P2PDataDeal();
#if DSP_2803X
    CanlinkFun();
    #if 0   // 串口操作长期打开
    if ( (funcCode.code.commProtocolSec < CANLINK) && (funcCode.code.plcEnable != 1) )
    #endif
    {
    	SciDeal();                      // 串口通讯处理(CANopen、PROFIBUS-DP、MODBUS)
    }
    OscSciFunction();                   // 后台通讯处理(15)
#else

    CanlinkFun();
    if(funcCode.code.commMaster)
	{
		OscSciFunction();               // 后台通讯处理(15)
	}
	else
	{

        if ( (funcCode.code.commProtocolSec < CANLINK) && (funcCode.code.plcEnable != 1) )
        {
        	SciDeal();                  // 通讯处理---modefied by sjw 2009-12-24  306
        }
	}
#endif
    
    FrqSrcDeal();               // 频率源 1100

    vfSeparateDeal();           // VF分离的处理(35)

    Main05msFunctionEachEnd();

}



// 0.5ms调用
// 2ms中的第2个0.5ms，即1.0ms-1.5ms
void Main05msFunctionC(void)
{
    asm(" nop");

    Main05msFunctionEachStart();    // 每个0.5ms都调用

	AccDecTimeCalc();           // 加减速时间计算 145
	
    runTimeCal();              // 上电时间、运行时间统计 100

    setTimeRun();              // 定时运行时间统计 65

	RunSrcDeal();               // 命令源   1300

	TorqueCalc();              // 更新设定转矩 80

	UpdateDataFunc2Core0p5ms(); // 更新功能传递给性能的交互数据  153

    EepromOperateTimeDeal();    // EEPROM时间处理，判断是否超时  40

    ErrorDeal();                // 更新errorCode 527

    AoCalcChannel(AOFMP_AO1);   // AO1  510
#if DSP_2803X 
    AoCalcChannel(AOFMP_AO2);   // AO2  530
#endif
    FMPDeal();                  // FMP处理 710

    Main05msFunctionEachEnd();

}



// 0.5ms调用
// 2ms中的第3个0.5ms，即1.5ms-2.0ms
void Main05msFunctionD(void)
{
    Main05msFunctionEachStart();    // 每个0.5ms都调用
    
    DoCalc();                   // DO  1590

    TemperatureDeal();          // 电机过温传感器
    
    UpdateU0Data();              // 330

    UpdateDisplayBuffer();      // 更新数据显示缓冲  440

    WinkDeal();                 // 闪烁处理   110

    ErrorReset();               // 故障处理    90

    UpdateDataFunc2Core2ms();   // 900

    CodeRunMonitor();           // 30

    if (0 == mainLoopTicker)    // 进入主循环的第1拍。从下一拍开始才考虑启动频率/下限频率的启动保护
    {
        mainLoopTicker = 1;
    }

    Main05msFunctionEachEnd();

}



//#define CODE_RUN_DISPLAY_UPDATE_TIME        200     // _ms，程序运行时间显示更新
#define CODE_MAX_RUN_DISPLAY_UPDATE_TIME    5000    // _ms，多长时间重新更新最长时间
Uint16 CODE_RUN_DISPLAY_UPDATE_TIME = 200;
// 程序执行时间监控
void CodeRunMonitor(void)
{
    static Uint16 ticker;

    if (++ticker >= CODE_RUN_DISPLAY_UPDATE_TIME / FUNC_DEAL_PERIOD)
    {
        ticker = 0;

        CODE_RUN_DISPLAY_UPDATE_TIME += 2;
        if (CODE_RUN_DISPLAY_UPDATE_TIME >= 300)
        {
            CODE_RUN_DISPLAY_UPDATE_TIME = 200;
        }

        ticker05msFuncEachDisp[0] = ticker05msFuncEa[0] * 3276 >> 15;
        ticker05msFuncEachDisp[1] = ticker05msFuncEa[1] * 3276 >> 15;
        ticker05msFuncEachDisp[2] = ticker05msFuncEa[2] * 3276 >> 15;
        ticker05msFuncEachDisp[3] = ticker05msFuncEa[3] * 3276 >> 15;

        ticker05msFuncInternalDisp[0] = ticker05msFuncIn[0] * 3276 >> 15;
        ticker05msFuncInternalDisp[1] = ticker05msFuncIn[1] * 3276 >> 15;
        ticker05msFuncInternalDisp[2] = ticker05msFuncIn[2] * 3276 >> 15;
        ticker05msFuncInternalDisp[3] = ticker05msFuncIn[3] * 3276 >> 15;

    }
}





// 获取16位数的每一位
// mode: 0--十进制, 1--十六进制
// 结果放在 digit[]中
// 返回该数字的位数
Uint16 GetNumberDigit(Uint16 digit[5], Uint16 number, Uint16 mode)
{
    if (!mode)          // 十进制
    {
        GetNumberDigit1(digit, number);
    }
    else                // 十六进制
    {
        GetNumberDigit2(digit, number);
    }

    if (digit[4])
            return 5;
    else if (digit[3])
        return 4;
    else if (digit[2])
        return 3;
    else if (digit[1])
        return 2;
    else //if (digit[0])
        return 1;
}

// 十进制
void GetNumberDigit1(Uint16 digit[5], Uint16 number)
{
    Uint16 tmp;

    tmp = (Uint32)number * 52429 >> 19; // 52428.8 = 1/10*2^19
    digit[0] = number - tmp * 10;
    number = tmp;
    
    tmp = (Uint32)number * 52429 >> 19;
    digit[1] = number - tmp * 10;
    number = tmp;

    tmp = (Uint32)number * 52429 >> 19;
    digit[2] = number - tmp * 10;
    number = tmp;

    tmp = (Uint32)number * 52429 >> 19;
    digit[3] = number - tmp * 10;
    number = tmp;

    tmp = (Uint32)number * 52429 >> 19;
    digit[4] = number - tmp * 10;
    number = tmp;
}


// 十六进制
void GetNumberDigit2(Uint16 digit[5], Uint16 number)
{
    Uint16 tmp;

    tmp = number >> 4;
    digit[0] = number - (tmp << 4);
    number = tmp;
    
    tmp = number >> 4;
    digit[1] = number - (tmp << 4);
    number = tmp;

    tmp = number >> 4;
    digit[2] = number - (tmp << 4);
    number = tmp;

    tmp = number >> 4;
    digit[3] = number - (tmp << 4);
    number = tmp;

    // 赋值0,防止产生随机值
    digit[4] = 0;
}


// 软件版本处理
void SoftVersionDeal(void)
{
    funcCode.code.productVersion = PRODUCT_VERSION;    // 产品号
    funcCode.code.funcSoftVersion = SOFTWARE_VERSION;  // 功能版本号

    // 显示功能与性能较大的版本号
    if (funcCode.code.funcSoftVersion < funcCode.code.motorSoftVersion)
    {
        funcCode.code.softVersion = funcCode.code.motorSoftVersion;
    }
    else
    {
        funcCode.code.softVersion = funcCode.code.funcSoftVersion;
    }
}


#if DEBUG_F_CHK_FUNC_CODE
// 检查功能码是否有错误，
// 同时自动获得表格，eeprom2Fc[], anybus2Fc[]
Uint16 sizeOfGroup;
Uint16 sizeOfGrade;
Uint16 sizeOfAll;
Uint16 sizeOfFuncCodeAttribute;
Uint16 sizeOfEeprom;
Uint16 sizeOfPara;
Uint16 errorFuncCode;
void ValidateFuncCode(void);


Uint16 eeprom2FcAutoGenerate[EEPROM_INDEX_USE_LENGTH];  // 自动生成的eeprom2Fc[]
extern const Uint16 fcNoAttri2Eeprom[];
Uint16 sizeOfTable_eeprom2Fc = EEPROM_INDEX_USE_LENGTH;
Uint16 maxEepromAddr;
Uint16 append;
void ValidateEepromAddr(void);
void GetTableEeprom2Fc(void);
void CheckEeprom2Fc(void);


// 判断功能码的 个数、属性个数等是否一致
// 还应该判断每组的个数是否一致
// funcCodeAttribute[((Uint16)((&(funcCode.group. bb [0] )) - (&(funcCode.all[0]))))]
// 
void ValidateFuncCode(void)
{
    // 判断 FUNCCODE_GROUP 和 FUNCCODE_CODE 是否一致
    sizeOfGroup = sizeof(struct FUNCCODE_GROUP);
    sizeOfGrade = sizeof(struct FUNCCODE_CODE);
    if (sizeOfGroup != sizeOfGrade)
    {
        asm(" ESTOP0");
    }

    // 判断 FUNCCODE_ALL 与 FUNCCODE_CODE 是否一致
    sizeOfAll = sizeof(FUNCCODE_ALL);
    if (sizeOfAll != sizeOfGrade)
    {
        asm(" ESTOP0");
    }

    // 判断属性个数是否一致
    //sizeOfFuncCodeAttribute = sizeof(funcCodeAttribute) / sizeof(FUNCCODE_ATTRIBUTE);
    sizeOfPara = FNUM_PARA;
    sizeOfEeprom = FNUM_EEPROM;
    if (funcCodeAttribute[FNUM_PARA-1].upper == 0)  // 数组funcCodeAttribute[]可能不对
    {
        asm(" ESTOP0");
    }
    if (sizeOfFuncCodeAttribute != sizeOfPara)
    {
        asm(" nop");
        //asm(" ESTOP0");
    }
    
#if 0
    // 判断U0组显示属性个数是否一致
    if (sizeof(dispAttributeU0) != U0NUM)
    {
        asm(" nop");
        asm(" ESTOP0");
    }

    // 判断U2组显示属性个数是否一致
    if (sizeof(dispAttributeU2) != U2NUM)
    {
        asm(" nop");
        asm(" ESTOP0");
    }

    // 判断U3组显示属性个数是否一致
    if (sizeof(dispAttributeU3) != U3NUM)
    {
        asm(" nop");
        asm(" ESTOP0");
    }
#endif

    append = CHK_NUM + REM_NUM;

// 判断EEPROM地址是否正确
    ValidateEepromAddr();
// 获得 eeprom2Fc[]
    GetTableEeprom2Fc();
// 检验数组 eeprom2Fc[] 是否正确
    CheckEeprom2Fc();

    errorCode = errorFuncCode;
    if (errorFuncCode)
    {
        for (;;)
            ;
    }

// 正常，进入无限循环，不真正操作功能码
    for (;;)
    {
        asm(" nop");
        asm(" ESTOP0");
    }
}



//--------------------------------------------------------
// 判断EEPROM地址是否正确
void ValidateEepromAddr(void)
{
    int16 i;
    int16 j;

// 是否有重复
    for (i = 0; i < FNUM_PARA; i++)
    {
        // funcCodeAttribute[]中的EEPROM地址是否有重复
        for (j = 0; j < FNUM_PARA; j++)
        {
            if ((i != j) 
                && (funcCodeAttribute[i].eepromIndex == funcCodeAttribute[j].eepromIndex)
                )
            {
                asm(" ESTOP0");
            }
        }
        
        // funcCodeAttribute[], 与fcNoAttri2Eeprom[]中的地址是否有重复
        for (j = 0; j < append; j++)
        {
            if (funcCodeAttribute[i].eepromIndex == fcNoAttri2Eeprom[j]) 
            {
                asm(" ESTOP0");
            }
        }
    }

    // fcNoAttri2Eeprom[]中的EEPROM地址是否有重复
    for (i = 0; i < append; i++)
    {
        for (j = 0; j < append; j++)
        {
            if ((i != j)
                && (fcNoAttri2Eeprom[i] == fcNoAttri2Eeprom[j])
                )
            {
                asm(" ESTOP0");
            }
        }
    }
    
// 获得EEPROM地址的最大值
// 判断max是否超过 EEPROM_INDEX_USE_LENGTH
    maxEepromAddr = 0;
    for (i = 0; i < append; i++)
    {
        if (maxEepromAddr < fcNoAttri2Eeprom[i])
            maxEepromAddr = fcNoAttri2Eeprom[i];
    }
    for (i = 0; i < FNUM_PARA; i++)
    {
        if (maxEepromAddr < funcCodeAttribute[i].eepromIndex)
            maxEepromAddr = funcCodeAttribute[i].eepromIndex;
    }
    // 判断max是否超过 EEPROM_INDEX_USE_LENGTH
    // 若超过，则 EEPROM_INDEX_USE_LENGTH 要相应更改!!
    if (maxEepromAddr >= EEPROM_INDEX_USE_LENGTH)
    {
        asm(" ESTOP0");
    }
}



// 获得eeprom2Fc[]
#define EEPROM2FC_NONE      FUNCCODE_RSVD4ALL_INDEX     // 保留地址
// 对应关系表
// y = eeprom2Fc[i]
// i, 数组下标，----该功能码在EEPROM的位置
// y, 数组的值，----功能码的序号
//
// 自动生成
void GetTableEeprom2Fc(void)
{
    int16 i;
    int16 j;

    // 获得数组 eeprom2fc[]
    for (i = 0; i <= EEPROM_INDEX_USE_LENGTH; i++)
    {
        for (j = 0; j < FNUM_PARA; j++)
        {
            if (i == funcCodeAttribute[j].eepromIndex)
            {
                eeprom2FcAutoGenerate[i] = j;
                break;
            }
        }
        
        if (j == FNUM_PARA) // funcCodeAttribute[].eepromIndex没有
        {
            for (j = 0; j < append; j++)
            {
                if (i == fcNoAttri2Eeprom[j])
                {
                    eeprom2FcAutoGenerate[i] = j + FNUM_PARA;
                    break;
                }
            }

            if (j == append)    // fcNoAttri2Eeprom[]也没有
            {
                eeprom2FcAutoGenerate[i] = EEPROM2FC_NONE;
                //...
            }
        }
    }
}



// 校验数组 eeprom2Fc[] 是否正确
void CheckEeprom2Fc(void)
{
    int16 i;
    
    for (i = 0; i < FNUM_PARA; i++) // 首先检查 A0-DF
    {
        if (eeprom2FcAutoGenerate[funcCodeAttribute[i].eepromIndex] != i)  // 不等
        {
            asm(" ESTOP0");
        }
    }
    
    // CHK,REM,U1
    for (i = 0; i < append; i++)
    {
        if (eeprom2FcAutoGenerate[fcNoAttri2Eeprom[i]] != i + GetCodeIndex(funcCode.group.fChk[0]))  // 不等
        {
            asm(" ESTOP0");
        }
    }
}
//--------------------------------------------------------

#endif

// 获得任意内存地址值
void getMomeryValue(void)
{
	Uint16* a;
	a = (Uint16*)MEMORY_ADDRESS;
    memoryValue = *a;
}












