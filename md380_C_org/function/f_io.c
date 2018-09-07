//======================================================================
//
// IO处理。包括DI, DO, AI, AO, PulseIn, PulseOut.
//
// Time-stamp: <2012-10-22 21:06:51  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_io.h"
#include "f_main.h"
#include "f_runSrc.h"
#include "f_comm.h"
#include "f_menu.h"
#include "f_frqSrc.h"
#include "f_posCtrl.h"
#include "f_error.h"

#if F_DEBUG_RAM        // 仅调试功能，在CCS的build option中定义的宏

#define DEBUG_F_DI              0
#define DEBUG_F_DO              0
#define DEBUG_F_AI              0
#define DEBUG_F_AO              0
#define DEBUG_F_HDI             0
#define DEBUG_F_HDO             0
#define DEBUG_F_TEMPERATURE     0

#elif 1

#define DEBUG_F_DI              1
#define DEBUG_F_DO              1
#define DEBUG_F_AI              1
#define DEBUG_F_AO              1
#define DEBUG_F_HDI             1
#define DEBUG_F_HDO             1

#if !DEBUG_F_POSITION_CTRL
#define DEBUG_F_TEMPERATURE     1
#elif 1
#define DEBUG_F_TEMPERATURE     0
#endif

#endif

//======================================================================
struct DI_FUNC diFunc;          // 当前的DI各个功能的状态
struct DI_FUNC diSelectFunc;    // 是否有DI端子选择了相应的功能
struct DI_STATUS_ALL diStatus;  // 当前的DI端子状态
LOCALF union DI_DELAY_TICKER_UNION diDelayTicker;
//======================================================================


//======================================================================
struct DO_FUNC doFunc;
Uint16  oCurrentCheckFlag;
struct DO_STATUS_ALL doStatus;  // 当前的DO端子状态
union DO_HW_STATUS doHwStatus;  // DO硬件状态
LOCALF union DO_DELAY_TICKER_UNION doDelayTicker;

#define DO_STATUS_VDO   5      // doStatus中，VDO1的bit位置

//#define PULSE_OUT_CTR_PERIOD       (MAX_UINT32 - 2)    // DO3使用ECAP2的APWM时的周期值
#define PULSE_OUT_CTR_PERIOD       (MAX_UINT16 - 2)
//======================================================================


//======================================================================
#define AI_VOLTAGE_CALC_SHIFT   5                   // AI计算过程中的电压的移位，由于LineCalc()的限制，目前最大为5。
#define AI_SAMPLE_Q             16                  // 计算时使用采样值的Q格式，目前为16
#define AI_MAX_SAMPLE1          ((0x0FFFUL-0)<<4) 
#define AI_MAX_SAMPLE           ((0x0FFFUL-3)<<4)   // 当采样值(Q16)大于_时，认为电压为AI_VOLTAGE_INPUT_MAX
//#define AI_MAX_SAMPLE           ((0x0FFFUL-0)<<4)

#define AI_MAX_VOLTAGE_HW       1057    // 硬件的AI输入最大输入电压，即多少电压对应DSP的满量程。单位，10mv
#define AI_MIN_VOLTAGE_HW       (int16)-1057
//#define AI_MAX_VOLTAGE_IDEA     10000   // 理想的AI输入最大输入电压，单位，1mv

LowPassFilter aiLpf[AI_NUMBER] = {LPF_DEFALUTS, LPF_DEFALUTS, LPF_DEFALUTS};
struct AI_DEAL aiDeal[AI_NUMBER];
//======================================================================

//======================================================================
#define AO_PWM_FRQ          10000   // AO的PWM载频, _Hz
#define AO_PWM_PERIOD       (DSP_CLOCK * 1000000UL / AO_PWM_FRQ)

#define AO_MAX_VOLTAGE_IDEA     10000   // 理想的AO最大输出电压，单位，1mv

#define FAN_CONTROL_STOP_DELAY  2500  // 散热器风扇停转延迟时间(5000ms)
#define FAN_CONTROL_START_TEMP  50    // 散热器风扇起动温度
#define FAN_CONTROL_STOP_TEMP   45    // 散热器风扇停转温度

int32 aoFmpValue;            // FMP,AO的输出值
int32 aoFmpMax;              // FMP,AO的输出最大值

//======================================================================

//======================================================================
#define PULSE_IN_ZERO                           1000    // _ms内没有脉冲输入(捕获)，认为脉冲频率为0
#define COUNTER_IN_USE_CAPTURE_TICKER_LIMIT     21000   // 计数器使用CAP捕获次数的频率限值,_Hz

int16 pulseInSet;               // Q15, 100.0% - 1*2^15
Uint32 pulseInFrq;              // 输入脉冲的脉冲频率，单位: 1Hz
Uint32 pulseInFrq1;
Uint16 lineSpeed;

//Uint16 fanControl;

//LOCALF LINE_STRUCT pulseInLine = LINE_STRTUCT_DEFALUTS;
LOCALF LowPassFilter pulseInLpf = LPF_DEFALUTS;
LOCALF Uint32 capturePeriodSum;     // 捕获到captureTicker的时间，CAP定时器的基数为SYSCLK
LOCALF Uint16 captureTicker;        // capturePeriodSum时间内，捕获到的次数
LOCALF Uint16 noCaptureTicker;      // PULSE_IN_CALC_PERIOD时间内没有捕获。一直的累加ticker

LOCALF Uint16 bOverFlowCounterTicker;   // 计数值 溢出标志
LOCALF Uint16 bOverFlowLengthCurrent;   // 当前长度 溢出标志

LOCALF Uint32 oCurrentChkTicker;         //  电流检测计时
Uint16 softOcDoFlag = 0;
//======================================================================



void TemperatureDeal(void);
void GetTemperature(void);
Uint16 GetTemperatureCalc(Uint16 voltage, const Uint16 *p, Uint16 len);
void TemperatureAfterDeal(void);
void TemperatureSensorDeal(void);
Uint16 motorForeOT;                         // 过温预报警标志
Uint16 motorOT;                             // 过温报警标志
Uint16 temperature;                         // 检测到得温度值
Uint16 tickerTempDeal;
Uint32 tempSampleSum;
Uint16 temperatureVoltage;                  // 温度检测的电压, 0-3V
Uint16 temperatureVoltageOrigin;            // 温度检测的电压, 校正前，0-3V
#define TEMPERATURE_CALC_PERIOD     100     // 温度处理周期，_ms
#define TEMPERATURE_CALL_PERIOD     2       // 温度处理函数调用周期，_ms
//======================================================================
//
// Time-stamp: <2007-12-20 14:01:46  Shisheng.Zhi, 0354>
// 
// 温度检测的参数
// DSP采样电压与温度的对应关系表
//
// PT100, PT1000, NTC
//
//======================================================================


#if DEBUG_F_TEMPERATURE
// PT100的参数表，DSP采样电压与温度的对应关系表
const Uint16 voltageTempPT100[] = 
{// 电压mV      温度℃
    1650 	,   //	0
    1675 	,   //	4
    1700 	,   //	8
    1724 	,   //	12
    1749 	,   //	16
    1774 	,   //	20
    1798 	,   //	24
    1823 	,   //	28
    1847 	,   //	32
    1872 	,   //	36
    1896 	,   //	40
    1920 	,   //	44
    1945 	,   //	48
    1969 	,   //	52
    1993 	,   //	56
    2017 	,   //	60
    2041 	,   //	64
    2066 	,   //	68
    2090 	,   //	72
    2114 	,   //	76
    2138 	,   //	80
    2161 	,   //	84
    2185 	,   //	88
    2209 	,   //	92
    2233 	,   //	96
    2257 	,   //	100
    2280 	,   //	104
    2304 	,   //	108
    2328 	,   //	112
    2351 	,   //	116
    2375 	,   //	120
    2398 	,   //	124
    2422 	,   //	128
    2445 	,   //	132
    2469 	,   //	136
    2492 	,   //	140
    2515 	,   //	144
    2538 	,   //	148
    2562 	,   //	152
    2585 	,   //	156
    2608 	,   //	160
    2631 	,   //	164
    2654 	,   //	168
    2677 	,   //	172
    2700 	,   //	176
    2723 	,   //	180
    2746 	,   //	184
    2769 	,   //	188
    2791 	,   //	192
    2814 	,   //	196
    2837 	,   //	200

}; 



// PT1000的参数表，DSP采样电压与温度的对应关系表
const Uint16 voltageTempPT1000[] = 
{// 电压mV      温度℃
    1648	,   //	0
    1673	,   //	4
    1698	,   //	8
    1722	,   //	12
    1748	,   //	16
    1772	,   //	20
    1795	,   //	24
    1820	,   //	28
    1844	,   //	32
    1868	,   //	36
    1891	,   //	40
    1916	,   //	44
    1941	,   //	48
    1964	,   //	52
    1988	,   //	56
    2012	,   //	60
    2036	,   //	64
    2059	,   //	68
    2083	,   //	72
    2108	,   //	76
    2130	,   //	80
    2155	,   //	84
    2177	,   //	88
    2202	,   //	92
    2225	,   //	96
    2249	,   //	100
    2272	,   //	104
    2295	,   //	108
    2317	,   //	112
    2341	,   //	116
    2364	,   //	120
    2387	,   //	124
    2411	,   //	128
    2433	,   //	132
    2456	,   //	136
    2478	,   //	140
    2502	,   //	144
    2524	,   //	148
    2547	,   //	152
    2570	,   //	156
    2592	,   //	160
    2616	,   //	164
    2638	,   //	168
    2661	,   //	172
    2683	,   //	176
    2706	,   //	180
    2729	,   //	184
    2750	,   //	188
    2772	,   //	192
    2795	,   //	196
    2818	,   //	200 
};
#endif


LOCALD void UpdateDoFunc(void);

void  newUpdateDoFunc(Uint16 index);

LOCALD void UpdateFmpAoValue(Uint16 func, Uint16 aoOrFmp);
void PulseOutDeal(void);
void PulseOutCalc(void);
LOCALD void Di2Bin(void);
LOCALD void Do2Bin(void); 
LOCALD void getDiHwStatus(void);
LOCALD void setDOStatus(void);

//=====================================================================
//
// DI 处理函数
// 输入：diHwStatus -- DI的瞬时状态
// 参数：funcCode.code.diFilterTime -- 滤波时间
// 输出：diFunc -- DI端子功能状态
//
// 流程：
// 1. 外部故障常开端子、停机直流制动端子赋值为1，其余diFunc赋值为0。
// 2. 根据DI的瞬时状态、滤波时间，得到DI的稳定状态 diStatus.a。
// 3. 根据diStatus，把diFunc相应位置1。
//
//    
//=====================================================================
Uint32 diLogic;     // 正反逻辑。VDI无此处理
Uint16 vdiSrc;
Uint16 vdiFcSet;
void DiCalc(void)
{
    static Uint16 diXFilterCounter[DI_NUMBER_PHSIC];     // 每个端子的滤波计时器
    int16 i;
    Uint32 tmp;
    Uint32 tmpFc;

    Uint16 fc1[DI_TERMINAL_NUMBER];     // DI端子的功能选择
    Uint32 *pDiSelectFunc = &diSelectFunc.f1.all;
    Uint32 *pDiFunc = &diFunc.f1.all;

    diSelectFunc.f1.all = 0;
    diSelectFunc.f2.all = 0;
    diFunc.f1.all = 0;                   // 不能删除
    diFunc.f2.all = 0;
#if DEBUG_F_POSITION_CTRL   //
    diSelectFunc.f3.all = 0;
    diFunc.f3.all = 0;
#endif

#if DEBUG_F_DI
    Di2Bin();

#if DSP_2803X   // 2803x平台
    getDiHwStatus();
#endif
    asm(" nop");


// 获得 diStatus.a.all。滤波时间
    // 物理DI端子，DI1 -- DI10
    for (i = DI_NUMBER_PHSIC - 1; i >= 0; i--)//382
    {
        tmp = 0x01UL << i;
        
        if ((diStatus.a.all ^ (~diHwStatus.all)) & tmp) // 该DI端子的状态改变了
        {                                           // 注意硬件上，DI已经取反
            // 确认该位DI的状态已经改变(滤波)
            if (++diXFilterCounter[i] >= funcCode.code.diFilterTime / DI_CALC_PERIOD)
            {
                diStatus.a.all ^= tmp;              // 该位DI的状态取反
                diXFilterCounter[i] = 0;
            }
        }
        else    // 没有改变，滤波计时器清零。不能删掉。考虑: 在达到滤波时间之前，又清除了
        {
            diXFilterCounter[i] = 0;
        }

        fc1[i] = funcCode.code.diFunc[i];
    }
    // 虚拟DI端子，VDI1 -- VDI5//359
    for (i = DI_NUMBER_V - 1; i >= 0; i--)
    {
        Uint16 vdiTmp = (0x01U << i);
        Uint16 vdi;
        
        tmp = 0x01UL << (i + DI_NUMBER_PHSIC);

        if (!(vdiSrc & vdiTmp))     // 0：与虚拟DOx内部连接
        {
            vdi = doStatus.c.all >> DO_STATUS_VDO;  // doStatus.c.all的bit6-bit10表示VDO
        }
        else                        // 1: 功能码设定
        {
            vdi = vdiFcSet;
        }

        if (vdi & vdiTmp)
        {
            diStatus.a.all |= tmp;
        }
        else
        {
            diStatus.a.all &= ~tmp;
        }

        fc1[i + DI_NUMBER_PHSIC] = funcCode.code.vdiFunc[i];
    }
    // AI作为DI使用//208
    for (i = DI_NUMBER_AI_AS_DI - 1; i >= 0; i--)
    {
        tmp = 0x01UL << (i + DI_NUMBER_PHSIC + DI_NUMBER_V);

#define AI_AS_DI_LOW    300     // <= 3V
#define AI_AS_DI_HIGH   700     // >= 7V
        if (aiDeal[i].voltage >= AI_AS_DI_HIGH)
        {
            diStatus.a.all |= tmp;
        }
        else if (aiDeal[i].voltage <= AI_AS_DI_LOW)
        {
            diStatus.a.all &= ~tmp;
        }

        fc1[i + DI_NUMBER_PHSIC + DI_NUMBER_V] = funcCode.code.aiAsDiFunc[i];
    }

// 获得diStatus.b.all。DI延迟时间处理//193
    for (i = 3-1; i >= 0; i--)
    {
        int32 delayTickerMax;

        tmp = 0x01UL << i;
        
        // F4-35 DI延迟时间功能码起始
        delayTickerMax = (Uint32)funcCode.code.diDelayTime[i] * (TIME_UNIT_DI_DELAY / DI_CALC_PERIOD);

        if (diStatus.a.all & tmp)   // 该DO输出1(延时前)
        {
            diDelayTicker.all[i].low = 0;
            if (++diDelayTicker.all[i].high >= delayTickerMax)
                diStatus.b.all |= tmp;     // 延时完成后一直输出1
        }
        else
        {
            diDelayTicker.all[i].high = 0;
            if (++diDelayTicker.all[i].low >= delayTickerMax)
                diStatus.b.all &= ~tmp;    // 延时完成后一直输出0
        }
    }
    diStatus.b.all = (diStatus.a.all & 0xFFFFFFF8UL) |  (diStatus.b.all & 0x00000007UL);
    
// 获得diStatus.c.all。正反逻辑处理
    diStatus.c.all = diStatus.b.all ^ diLogic;
    
// 根据diStatus.c.all，进行处理//1253
    for (i = DI_TERMINAL_NUMBER - 1; i >= 0; i--)
    {
        tmp = 0x01UL << i;
        
        tmpFc = 0x01UL << (fc1[i] % 32);            // fc1[]有可能大于32
// code generation 4.13编译，即使fc >= 32，也没有问题。汇编为 LSLL

// 获得diSelectFunc
        *(pDiSelectFunc + fc1[i] / 32) |= tmpFc;    // 是否有DI端子选择了相应的功能。

// 获得diFunc
        if (diStatus.c.all & tmp)                   // 根据DI的稳定的当前状态，置1 or 清零
        {
            *(pDiFunc + fc1[i] / 32) |= tmpFc;
        }
        else
        {
            *(pDiFunc + fc1[i] / 32) &= ~tmpFc;
        }
    }

// 使用pulseIn时，DI5功能不计算。
// 但是，使用pulseIn时，DI5还可同时作为计数器和长度输入。
    if ((DI_FUNC_COUNTER_TICKER_IN != funcCode.code.diFunc[4]) && 
        (DI_FUNC_LENGTH_TICKER_IN != funcCode.code.diFunc[4]) && 
        (   // 使用了Pulse In，DI5功能不计算。
         (FUNCCODE_frqXySrc_PULSE == funcCode.code.frqXSrc) || 
         (FUNCCODE_frqXySrc_PULSE == funcCode.code.frqYSrc) || 
         (FUNCCODE_upperFrqSrc_PULSE  == funcCode.code.upperFrqSrc) || 
         (FUNCCODE_pidSetSrc_PULSE == funcCode.code.pidSetSrc) || 
         (FUNCCODE_pidFdbSrc_PULSE == funcCode.code.pidFdbSrc) || 
         (FUNCCODE_plcFrq0Src_PULSE == funcCode.code.plcFrq0Src)
        )
       )
    {
        *(pDiFunc) &= ~(0x01UL << funcCode.code.diFunc[4]);
    }
#endif
} // DiCalc()


//==================================================================
//
// 变频器运行中(1)
//==================================================================
void Dout1(void)
{
    if (runFlag.bit.run)
    {
        doFunc.f1.bit.run = 1;
    }
}

//==================================================================
//
// 故障输出(故障停机)
//
//==================================================================
void Dout2(void)
{
    if //((ERROR_EXTERNAL == errorCode)      // 外部故障(不自动复位)
       //|| (ERROR_RUN_TIME_OVER == errorCode) // 运行时间到达(不自动复位)
       //|| (ERROR_USER_1 == errorCode)        // 用户故障1(不自动复位)
       //|| (ERROR_USER_2 == errorCode)        // 用户故障2(不自动复位)
       //|| 
       (errorCode                              // 故障且故障自动复位结束   
       && ( errorDealStatus != ERROR_DEAL_OK)  // 故障自动复位慢了一拍
       && (funcCode.code.errAutoRstRelayAct || (errAutoRstNum >= funcCode.code.errAutoRstNumMax))
       )
    {
        // 故障响应不为自动运行
        if (ERROR_LEVEL_RUN != errorAttribute.bit.level)
        {
            doFunc.f1.bit.error = 1;  // 2:故障输出(故障停机)
        }
    }   
}

//==================================================================
//
// 故障输出(故障输出)
//
//==================================================================
void Dout38(void)
{
    if //(  (ERROR_EXTERNAL == errorCode)      // 外部故障(不自动复位)
       //|| (ERROR_RUN_TIME_OVER == errorCode) // 运行时间到达(不自动复位)
       //|| (ERROR_USER_1 == errorCode)        // 用户故障1(不自动复位)
       //|| (ERROR_USER_2 == errorCode)        // 用户故障2(不自动复位)
       //|| 
       (errorCode                              // 故障且故障自动复位结束   
       && ( errorDealStatus != ERROR_DEAL_OK)  // 故障自动复位慢了一拍
       && (funcCode.code.errAutoRstRelayAct || (errAutoRstNum >= funcCode.code.errAutoRstNumMax))      
       )
    {
        doFunc.f2.bit.errorOnStop = 1;      // 38:故障输出
    }   
}

//==================================================================
//
// 故障输出
//
//==================================================================
void Dout41(void)
{
    if //((ERROR_EXTERNAL == errorCode)      // 外部故障(不自动复位)
       //|| (ERROR_RUN_TIME_OVER == errorCode) // 运行时间到达(不自动复位)
       //|| (ERROR_USER_1 == errorCode)        // 用户故障1(不自动复位)
       //|| (ERROR_USER_2 == errorCode)        // 用户故障2(不自动复位)
       //|| 
       (errorCode                              // 故障且故障自动复位结束   
       && ( errorDealStatus != ERROR_DEAL_OK)  // 故障自动复位慢了一拍
       && (errorCode != ERROR_UV)
       && (funcCode.code.errAutoRstRelayAct || (errAutoRstNum >= funcCode.code.errAutoRstNumMax))
       )
    {
        // 故障响应不为自动运行
        if (ERROR_LEVEL_RUN != errorAttribute.bit.level)
        {
            doFunc.f2.bit.errorOnNoUV = 1;  // 2:故障输出(故障停机)
        }
    }   
}

//==================================================================
//
// 3:频率水平检测FDT到达
//
//==================================================================
void Dout3(void)
{
    static Uint16 frqFdtArriveOld;      // FDT频率到达

    if (ABS_INT32(frq) >= funcCode.code.frqFdtValue)
    {
        frqFdtArriveOld = 1;
    }
    else if (ABS_INT32(frq) < ((Uint32)funcCode.code.frqFdtValue * (1000 - funcCode.code.frqFdtLag) / 1000))
    {
        frqFdtArriveOld = 0;
    }
    
    if (!runFlag.bit.run)       // 停机时，应该一直输出无效
    {
        frqFdtArriveOld = 0;
    }
    
    if (frqFdtArriveOld)
    {
        doFunc.f1.bit.frqFdtArrive = 1;
    }
}


//==================================================================
//
// 4:频率到达
//
//==================================================================
void Dout4(void)
{
    if ((ABS_INT32(frq) <= (ABS_INT32(frqAim) + (int32)maxFrq * funcCode.code.frqArriveRange / 1000))
        && (ABS_INT32(frq) >= (ABS_INT32(frqAim) - (int32)maxFrq * funcCode.code.frqArriveRange / 1000))  
        && (runFlag.bit.run)
        && ((int64)frq * frqAim >= 0)
        && (!tuneCmd)  // 调谐时
        )
    {
        doFunc.f1.bit.frqArrive = 1;
    }
}

//==================================================================
//
// 零速运行中
//
//==================================================================
void Dout5(void)
{
    if (!frq)
    {
        if(runFlag.bit.run)  // 处于运行中
        {
            doFunc.f1.bit.zeroSpeedRun = 1; // 5:零速运行中(停机无效)
        }
    }
}

//==================================================================
//
// 零速运行中
//
//==================================================================
void Dout23(void)
{
    if (!frq)
    {
        doFunc.f1.bit.zeroSpeedRun1 = 1;    // 23:零速运行中2(停机有效)
    }
}

//==================================================================
//
// 6:电机过载预报警
//
//==================================================================
void Dout6(void)
{
    if (dspStatus.bit.motorPreOl)
    {
        doFunc.f1.bit.motorPreOl = 1;
    }
}

//==================================================================
//
// 7:变频器过载预报警
//
//==================================================================
void Dout7(void)
{
    if (dspStatus.bit.inverterPreOl)
    {
        doFunc.f1.bit.inverterPreOl = 1;
    }
}

//==================================================================
//
// 8:设定计数脉冲值到达
//
//==================================================================
void Dout8(void)
{
    if (funcCode.code.counterTicker >= funcCode.code.counterSet)
    {
        doFunc.f1.bit.counterSetArrive = 1;
    }

    if (bOverFlowCounterTicker)    // counterTicker溢出
    {
        doFunc.f1.bit.counterSetArrive = 1;
    }
}

//==================================================================
//
// 9:指定计数脉冲值到达
//
//==================================================================
void Dout9(void)
{
    if (funcCode.code.counterTicker >= funcCode.code.counterPoint)
    {
        doFunc.f1.bit.counterPointArrive = 1;
    }

    // 计数脉冲溢出
    if (bOverFlowCounterTicker)    // counterTicker溢出
    {
        doFunc.f1.bit.counterPointArrive = 1;
    }
}

//==================================================================
//
// 10:长度到达
//
//==================================================================
void Dout10(void)
{
    if ((funcCode.code.lengthCurrent >= funcCode.code.lengthSet)
        || bOverFlowLengthCurrent)  // 溢出
    {
        doFunc.f1.bit.lengthArrive = 1;
    }
}

//==================================================================
//
// 11:PLC循环完成
//
//==================================================================
void Dout11(void)
{
    if (bPlcEndOneLoop)
    {
        doFunc.f1.bit.plcEndLoop = 1;
    }
}

//==================================================================
//
// 12:运行时间到达
//
//==================================================================
void Dout12(void)
{
    if ((funcCode.code.runTimeAddup >= funcCode.code.runTimeArriveSet)
        && funcCode.code.runTimeArriveSet)  // 设定运行到达时间时间为0，无效
    {
        doFunc.f1.bit.runTimeArrive = 1;
    }
}

//==================================================================
//
// 13:频率限定中
//
//==================================================================
void Dout13(void)
{
	if (swingFrqLimit)
	{
		doFunc.f1.bit.frqLimit = 1;  // 摆频限定中
	}
}

//==================================================================
//
// 14:转矩限定中
//
//==================================================================
void Dout14(void)
{
    if (TorqueLimitCalc())
    {
        doFunc.f1.bit.torqueLimit = 1;  // 转矩限定中
    }
}

//==================================================================
//
// 运行准备就绪(15)
//
//==================================================================
void Dout15(void)
{
    if ((!errorCode)                    // 没有故障
        && (!bUv)                       // 母线电压不欠压
        && (!diFunc.f1.bit.closePwm)    // 自由停车(运行禁止)端子无效
        )
    {
        doFunc.f1.bit.runReadyOk = 1;
    }
}

//==================================================================
//
// AI1 > AI2(16)
//
//==================================================================
void Dout16(void)
{
    if (ABS_INT16(aiDeal[0].set) > ABS_INT16(aiDeal[1].set))
    {
        doFunc.f1.bit.ai1GreaterThanAi2 = 1;
    }
}

//==================================================================
//
// 上限频率到达(17)
//
//==================================================================
void Dout17(void)
{
	if ((ABS_INT32(frqAim) >= upperFrq) 
      && (ABS_INT32(frq) >= upperFrq))  // 注意，这与是否运行无关(供水)
	{
		doFunc.f1.bit.upperFrqArrive = 1;
	}
}

//==================================================================
//
// 下限频率到达(18)
//
//==================================================================
void Dout18(void)
{
	if ((ABS_INT32(frqAim) <= lowerFrq)
		&& (ABS_INT32(frq) <= lowerFrq)
		&& runFlag.bit.run)
	{
		doFunc.f1.bit.lowerFrqArrive = 1;
	}
}

//==================================================================
//
// 欠压状态输出(19)
//
//==================================================================
void Dout19(void)
{
    if (!dspStatus.bit.uv)
    {
        doFunc.f1.bit.uv = 1;
    }
}

void DoutRSVD(void)
{
}

//==================================================================
//
// 定位完成(21)    
//
//==================================================================
void Dout21(void)
{
#if DEBUG_F_POSITION_CTRL
    if (bPcErrorOk)
    {
        doFunc.f1.bit.pcOk = 1; // DO 输出 定位完成信号
    }
#endif
}

//==================================================================
//
// 定位接近(22)
//
//==================================================================
void Dout22(void)
{
#if DEBUG_F_POSITION_CTRL
    if (bPcErrorNear)
    {
        doFunc.f1.bit.pcNear = 1; // DO 输出 定位接近信号
    }
#endif
}

//==================================================================
//
// 上电时间到达(24)
//
//==================================================================
void Dout24(void)
{
    if ((funcCode.code.powerUpTimeAddup >= funcCode.code.powerUpTimeArriveSet)
        && funcCode.code.powerUpTimeArriveSet)  // 设定运行到达时间时间为0，无效
    {
        doFunc.f1.bit.powerUpTimeArrive = 1;
    }
}

//==================================================================
//
// 频率水平检测FDT1到达(25)
//
//==================================================================
void Dout25(void)
{
    static Uint16 frqFdt1ArriveOld;     // FDT1频率到达

    if (ABS_INT32(frq) >= funcCode.code.frqFdt1Value)
    {
        frqFdt1ArriveOld = 1;
    }
    else if (ABS_INT32(frq) < ((Uint32)funcCode.code.frqFdt1Value * (1000 - funcCode.code.frqFdt1Lag) / 1000))
    {
        frqFdt1ArriveOld = 0;
    }
    if (!runFlag.bit.run)       // 停机时，应该一直输出无效
    {
        frqFdt1ArriveOld = 0;
    }
    if (frqFdt1ArriveOld)
    {
        doFunc.f1.bit.frqFdtArrive1 = 1;
    }	
}

//==================================================================
//
// 频率到达1(26)
//
//==================================================================
void Dout26(void)
{
    if ((ABS_INT32(frq) <= (funcCode.code.frqArriveValue1 + (int32)maxFrq * funcCode.code.frqArriveRange1 / 1000))
        && (ABS_INT32(frq) >= (funcCode.code.frqArriveValue1 - (int32)maxFrq * funcCode.code.frqArriveRange1 / 1000))
        && runFlag.bit.run
        )
    {
        doFunc.f1.bit.frqArrive1 = 1;
    }
}

//==================================================================
//
// 频率到达2(27)
//
//==================================================================
void Dout27(void)
{
    if ((ABS_INT32(frq) <= (funcCode.code.frqArriveValue2 + (int32)maxFrq * funcCode.code.frqArriveRange2 / 1000))
        && (ABS_INT32(frq) >= (funcCode.code.frqArriveValue2 - (int32)maxFrq * funcCode.code.frqArriveRange2 / 1000))
        && runFlag.bit.run
        )
    {
        doFunc.f1.bit.frqArrive2 = 1;
    }
}

//==================================================================
//
// 任意电流到达1(28)
//
//==================================================================
void Dout28(void)
{
    if ((outCurrentDisp >= (((int32)funcCode.code.currentArriveValue1 - funcCode.code.currentArriveRange1)*motorFc.motorPara.elem.ratingCurrent/1000))
        && (outCurrentDisp <= ((funcCode.code.currentArriveValue1 + funcCode.code.currentArriveRange1)*motorFc.motorPara.elem.ratingCurrent/1000))
        && runFlag.bit.run
        )
    {
        doFunc.f1.bit.currentArrive1 = 1;
    }
}

//==================================================================
//
// 任意电流到达2(29)
//
//==================================================================
void Dout29(void)
{
    if ((outCurrentDisp >= (((int32)funcCode.code.currentArriveValue2 - funcCode.code.currentArriveRange2)*motorFc.motorPara.elem.ratingCurrent/1000))
        && (outCurrentDisp <= ((Uint32)(funcCode.code.currentArriveValue2 + funcCode.code.currentArriveRange2)*motorFc.motorPara.elem.ratingCurrent/1000))
        && runFlag.bit.run
        )
    {
        doFunc.f1.bit.currentArrive2 = 1;
    }	
}

//==================================================================
//
// 定时到达(30)
//
//==================================================================
#define SET_TIME_ARRIVE_TCNT_MAX   250  //500ms
void Dout30(void)
{
    static Uint16 arriveTcnt;    
    if ((!setRunLostTime) && setRunTimeAim)
    {
        arriveTcnt = SET_TIME_ARRIVE_TCNT_MAX;
    }

    if (arriveTcnt > 0)
    {
        doFunc.f1.bit.setTimeArrive = 1;   // DO 输出  定时到达信号
        arriveTcnt--;
    }
}

//==================================================================
//
// AI1超出上下限(31)
//
//==================================================================
void Dout31(void)
{
    if ((ABS_INT16(aiDeal[0].voltage) > funcCode.code.ai1VoltageUpper) || 
        (ABS_INT16(aiDeal[0].voltage) < funcCode.code.ai1VoltageLimit))
    {
        doFunc.f1.bit.ai1limit = 1;
    }
}

//==================================================================
//
// 掉载(32)
//
//==================================================================
void Dout32(void)
{
    if (dspStatus.bit.outAirSwitchOff)
    {
        doFunc.f2.bit.loseLoad = 1;
    }
}

//==================================================================
//
// 转速方向(33)
//
//==================================================================
void Dout33(void)
{
    if ( runFlag.bit.run
    	&& ((frqRun < 0) 
        	|| ((frqRun == 0) && (frqAim < 0)))
		)
    {
        doFunc.f2.bit.speedDir = 1;  // 运行方向为反
    }
}

//==================================================================
//
// 零电流检测(34)
//
//==================================================================
void Dout34(void)
{
    if (!oCurrentCheckFlag)
    {
        if ((runFlag.bit.run) && 
            (outCurrentDisp <= ((Uint32)funcCode.code.oCurrentChkValue*motorFc.motorPara.elem.ratingCurrent/1000))
            )
        {
            // 零电流检测时间
            if (oCurrentChkTicker < ((Uint32)funcCode.code.oCurrentChkTime*TIME_UNIT_CURRENT_CHK / DO_CALC_PERIOD))
            {
                oCurrentChkTicker++;
            }
        }
        else
        {
            oCurrentChkTicker = 0;
        }

        if (oCurrentChkTicker >= ((Uint32)funcCode.code.oCurrentChkTime*TIME_UNIT_CURRENT_CHK / DO_CALC_PERIOD))
        {
            doFunc.f2.bit.oCurrent = 1;
        }

        // 已判断
        oCurrentCheckFlag = 1;
    }
}

//==================================================================
//
// 模块温度预报警(35)
//
//==================================================================
void Dout35(void)
{
    if (funcCode.code.radiatorTemp >= funcCode.code.temperatureArrive)
    {
        doFunc.f2.bit.tempArrive= 1;
    }
}

//==================================================================
//
// 软件过流DO输出(36)
//
//==================================================================
void Dout36(void)
{
    if (softOcDoFlag == 1)
    {
        doFunc.f2.bit.softOc= 1;
    }
    else
    {
        doFunc.f2.bit.softOc= 0;
    }
}

//==================================================================
//
// 下限频率到达(37)
//
//==================================================================
void Dout37(void)
{
	if ((ABS_INT32(frqAim) <= lowerFrq)
		&& (ABS_INT32(frq) <= lowerFrq)) // 注意，这与是否运行有关	
	{
		doFunc.f2.bit.lowerFrqArrive = 1;
	}
}

void Dout39(void)
{
    // 电机过温预报警
    if (motorForeOT)
    {
        doFunc.f2.bit.motorForeOT = 1;
    }
}

void Dout40(void)
{
    // 当前运行时间到达
    if (funcCode.code.setTimeArrive 
        && (curTime.runTime >= funcCode.code.setTimeArrive))
    {
        doFunc.f2.bit.setRunTimeArrive = 1;
    }
}

typedef struct
{
    void (*calc)();       // 函数指针
} DO_FUNC_CAL;

const DO_FUNC_CAL doFuncCal[DO_FUNC_NUMBER] = {
    DoutRSVD,  Dout1,  Dout2,  Dout3,  Dout4,  
    Dout5,     Dout6,  Dout7,  Dout8,  Dout9,  
    Dout10,    Dout11, Dout12, Dout13, Dout14, 
    Dout15,    Dout16, Dout17, Dout18, Dout19, 
    DoutRSVD,  Dout21, Dout22, Dout23, Dout24, 
    Dout25,    Dout26, Dout27, Dout28, Dout29, 
    Dout30,    Dout31, Dout32, Dout33, Dout34, 
    Dout35,    Dout36, Dout37, Dout38, Dout39,
    Dout40,    Dout41,
};

//=====================================================================
//
// DO 处理函数
//
// 根据当前变频器的各种状态/通讯的DO控制，
// 输出DO端子的doStatus。
//
// doStatus的位定义，与通讯控制的DO输出控制的位定义的关系。
// doStatus.a.bit(i)         -- doComm Index[i]
// doStatus.bit0 --   do3  -- doComm.bit4
// doStatus.bit1 -- relay1 -- doComm.bit2
// doStatus.bit2 -- relay2 -- doComm.bit3
// doStatus.bit3 --   do1  -- doComm.bit0
// doStatus.bit4 --   do2  -- doComm.bit1
//
// doHwIndex, 功能码DO顺序与硬件DO顺序的关系
//
//=====================================================================
Uint16 doLogic;
void DoCalc(void)
{
#if DEBUG_F_DO
    Uint32 delayTickerMax;
    Uint16 doCommIndex[DO_TERMINAL_NUMBER] = {4, 2, 3, 0, 1, 5, 6, 7, 8, 9};
    int16 i;

// 更新doFunc    
    doFunc.f1.all = 0;
    doFunc.f2.all = 0;
    oCurrentCheckFlag = 0;

// 根据doFunc，更新doStatus
    doStatus.a.all = 0;
    for (i = DO_TERMINAL_NUMBER - 1; i >= 0; i--)
    {
        Uint16 fc;      // 该DO的功能选择
        Uint16 tmpSci; 
        Uint16 tmp;
        Uint16 flag = 0;

        if (i < DO_NUMBER_PHSIC)   // 物理DO
        {
            fc = funcCode.code.doFunc[i];
    		tmp = 1U << i;
            delayTickerMax = (Uint32)funcCode.code.doDelayTime[i] * (TIME_UNIT_DO_DELAY / DO_CALC_PERIOD);
        }
        else        // 虚拟DO
        {
            Uint16 vdoTmp = i - DO_NUMBER_PHSIC;
            fc = funcCode.code.vdoFunc[vdoTmp];
    		tmp = 1U << (vdoTmp + DO_STATUS_VDO);
            delayTickerMax = (Uint32)funcCode.code.vdoDelayTime[vdoTmp] * (TIME_UNIT_DO_DELAY / DO_CALC_PERIOD);
        }
        
        // 通讯控制
        tmpSci = 1U << doCommIndex[i];

        // 虚拟DO且数据来源为DI
        if ((i >= DO_NUMBER_PHSIC) 
            && (fc == 0))
        {
            flag = (diStatus.c.all>>(i - DO_NUMBER_PHSIC)) & 0x0001;
        }
        else if (DO_FUNC_COMM_CTRL != fc) 
        {
            doComm.all &= ~tmpSci;              // 该DO在doSci对应的位清零。
#if 0
            flag = doFuncCal[fc].calc();
#else
            doFuncCal[fc].calc();
            // 该DO在doSci对应的位清零。
            if(fc > 31)
            {
                if (doFunc.f2.all & (0x1UL << (fc-32)))  // 由doFunc控制
                    flag = 1;   // 该DO输出1
            }
            else
            {
                if (doFunc.f1.all & (0x1UL << fc))  // 由doFunc控制
                    flag = 1;   // 该DO输出1
            }
#endif
        }
        else
        {
            if (doComm.all & tmpSci)            // 通讯控制的该位DO为1
            {
                flag = 1;    
            }
        }

        // DO输出延时处理
        if (flag)   // 该DO输出1(延时前)
        {
            doStatus.a.all |= tmp;
            
            doDelayTicker.all[i].low = 0;
            if (++doDelayTicker.all[i].high >= delayTickerMax)
                doStatus.b.all |= tmp;     // 延时完成后一直输出1
        }
        else
        {
            //doStatus.a.all &= ~tmp;
            
            doDelayTicker.all[i].high = 0;
            if (++doDelayTicker.all[i].low >= delayTickerMax)
                doStatus.b.all &= ~tmp;    // 延时完成后一直输出0
        }
    }

// 正反逻辑
    Do2Bin();   // 更新doLogic
    doStatus.c.all = doStatus.b.all ^ doLogic;

// 转换成硬件DO对应顺序
// DO3(FMR)的控制在FMPDeal()中
    doHwStatus.bit.relay1 = doStatus.c.bit.relay1;      // 控制板继电器
    doHwStatus.bit.relay2 = doStatus.c.bit.relay2;      // 扩展板继电器
    doHwStatus.bit.do1    = doStatus.c.bit.do1;         // DO1
    doHwStatus.bit.do2    = doStatus.c.bit.do2;         // 扩展板DO2
    doHwStatus.bit.error  = doFunc.f1.bit.error;        // 故障指示灯
    doHwStatus.bit.run    = doFunc.f1.bit.run;          // 运行指示灯
    doHwStatus.bit.fan    = (dspStatus.bit.fan | funcCode.code.fanControl);
    //doHwStatus.bit.fan    = dspStatus.bit.fan;          // 风扇控制

#if DEBUG_F_PLC_CTRL
    if (funcCode.code.outPortControl && funcCode.code.plcEnable)
    {
        Uint16 digit1[5];
        Uint16 digit2[5];
        // 读取输出端子控制来源(PLC卡控制还是变频器控制)
        GetNumberDigit1(digit1, funcCode.code.outPortControl);
        // 读取PLC卡控制端子输出信号
        GetNumberDigit1(digit2, funcCode.code.inPortOut);
        
        // FMP为PLC卡控制
        if (digit1[0])
        {
            doStatus.c.bit.do3 = digit2[0];
        }
        
        // 继电器1为PLC卡控制
        if (digit1[1])
        {
            doHwStatus.bit.relay1 = digit2[1];
        }
        
        // DO1为PLC卡控制
        if (digit1[2])
        {
            doHwStatus.bit.do1 = digit2[2];
        }
    }
#endif

#if DSP_2803X
    setDOStatus();
#endif
    
    
#endif
} // DoCalc()


#if DEBUG_F_TEMPERATURE

//========================================================================
//
// 温度检测处理
//
//========================================================================
void TemperatureDeal(void)
{
    // 没有温度传感器
    if (!funcCode.code.motorOtMode)
    {
        motorForeOT = 0;   // 过温预报警清0
        motorOT = 0;       // 过温报警清0
        // 温度检测值清0
        tickerTempDeal = 0;
        tempSampleSum = 0;
        temperature = 0;
        
        if (ERROR_MOTOR_OT == errorOther)
        {
            errorOther = ERROR_NONE;
        }
        return;
    }
    // 温度传感器
    TemperatureSensorDeal();   
    // 获取温度之后的处理
    TemperatureAfterDeal();
}


//========================================================================
//
// 温度传感器处理
//
//========================================================================
void TemperatureSensorDeal()
{
    // 若无温度传感器
    if (FUNCCODE_tempSenorType_NONE == funcCode.code.motorOtMode)
    {
        return;
    }

    // AI2 温度采样值
    tempSampleSum += aiDeal[2].sample;

    tickerTempDeal++;
    if (tickerTempDeal >= TEMPERATURE_CALC_PERIOD / TEMPERATURE_CALL_PERIOD) // 时间错开
    {
        Uint32 k;
        LINE_STRUCT aiLine = LINE_STRTUCT_DEFALUTS;
        
        Uint16 *p = &funcCode.code.aiCalibrateCurve[2].before1;  // 实测电压      
#define MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA  3300        // 3300mv

        // 获取温度检测的采样电压
        k = (4095L << 4) * tickerTempDeal / MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA;
        temperatureVoltageOrigin = tempSampleSum / k;

        // 电机温度传感器AI2采样校正曲线
        aiLine.mode = 1;    // 不限幅
        aiLine.y1 = ((int32)(int16)(*(p + 0)) * 0x7FFF) / MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA;  // (理想)输入电压1，精密仪器测量电压
        aiLine.x1 = ((int32)(int16)(*(p + 1)) * 0x7FFF) / MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA;  // 变频器采样电压1(校正前)
        aiLine.y2 = ((int32)(int16)(*(p + 2)) * 0x7FFF) / MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA;  // 理想输入电压2
        aiLine.x2 = ((int32)(int16)(*(p + 3)) * 0x7FFF) / MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA;  // 变频器采样电压2(校正前)
        aiLine.x = ((int32)temperatureVoltageOrigin * 0x7FFF) / MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA;
        aiLine.calc(&aiLine);
        if (aiLine.y < 0)    // 计算后可能出现负值，但温度传感器电压不能为负
        {
            aiLine.y =0;
        }

        temperatureVoltage = aiLine.y * MOTOR_T_SENSOR_AI_MAX_VOLTAGE_IDEA / 0x7FFF;

        // 根据采样电压、电压与温度的对应关系表，获得温度
        GetTemperature();

        tickerTempDeal = 0;  // 清零
        tempSampleSum = 0;

        // AI3电压显示
        aiDeal[2].voltageOrigin = temperatureVoltageOrigin;
        aiDeal[2].voltage = temperatureVoltage/10;
    }
}


//========================================================================
//
// 获得温度值
//
//========================================================================
void GetTemperature()
{
    const Uint16 *p;
    Uint16 size;

// 温度传感器类型
    if (FUNCCODE_tempSenorType_PTC100 == funcCode.code.motorOtMode)
    {
        p = voltageTempPT100;
        size = sizeof(voltageTempPT100);
    }
    else if (FUNCCODE_tempSenorType_PTC1000 == funcCode.code.motorOtMode)
    {
        p = voltageTempPT1000;
        size = sizeof(voltageTempPT1000);
    }

    temperature = GetTemperatureCalc(temperatureVoltage, p, size);
}



// 根据采样电压、电压与温度的对应关系表，获得温度
Uint16 GetTemperatureCalc(Uint16 voltage, const Uint16 *p, Uint16 len)
{
    int16 i;
    LINE_STRUCT temperatureLine = LINE_STRTUCT_DEFALUTS;

    for (i = 1; i < len; i++)
    {
        if (voltage < *(p + i))
            break;
    }
    if (i >= len)  // 到达最终
    {
        i = len - 1;
    }

    temperatureLine.mode = 0;
    temperatureLine.x1 = *(p + i - 1);
    temperatureLine.y1 = (i - 1) * 4;
    temperatureLine.x2 = *(p + i);
    temperatureLine.y2 = i * 4;
    temperatureLine.x = voltage;
    temperatureLine.calc(&temperatureLine);
    
    return (temperatureLine.y);
}


//========================================================================
//
// 温度处理
//
//========================================================================
void TemperatureAfterDeal(void)
{
    // 过温预报警
    if (temperature >= funcCode.code.motorOtCoef)
    {
        motorForeOT = 1;
    }
    else
    {
        motorForeOT = 0;
    }

    // 电机过温报警
    if (temperature >= funcCode.code.motorOtProtect)
    {
        motorOT = 1;
    }
    else
    {
        motorOT = 0;
    }

    if (motorOT)
    {
        if (!errorOther)
        {
            errorOther = ERROR_MOTOR_OT;    // 电机过温故障
        }
    }
}
#else
void TemperatureDeal(void)
{
}
#endif

//=====================================================================
//
// AI处理函数
// AI1 -- A2
// AI2 -- B2
//
//=====================================================================
void AiCalc(void)
{
#if DEBUG_F_AI
    LINE_STRUCT aiLine = LINE_STRTUCT_DEFALUTS;
    Uint16 * pAiSetCurve[5] =               // AI设定曲线
    {
        &funcCode.code.curveSet2P1[0],
        &funcCode.code.curveSet2P2[0],
        &funcCode.code.curveSet2P3[0],
        &funcCode.code.curveSet4P1[0],
        &funcCode.code.curveSet4P2[0],
    };
    Uint16 aiFilterTime[AI_NUMBER];
    Uint16 aiNum;
    int32 point;    // AI JUMP的点
    int32 range;    // AI JUMP的范围
    Uint16 *p = &funcCode.code.curveSet2P1[0];  // 临时指针
    int16 i;
    int16 j;
    int32 aiMin;

    if (1 == mainLoopTicker)        // 
    {
        aiLpf[0].out = aiDeal[0].sample;
        aiLpf[1].out = aiDeal[1].sample;
        aiLpf[2].out = aiDeal[2].sample;

        mainLoopTicker = 2;         // 仅性能传递AI的第1拍，如此处理。之后，可正常使用AI
    }

#if F_DEBUG_RAM
#if 0
#define AI1_NUMBER  256
    static Uint16 ai1Array[AI1_NUMBER];
    Uint32 ai1Sum;
#endif

//#define AI1_RESULT  (AdcRegs.ADCRESULT4)    // AI1对应的ADCRESULT
//#define AI2_RESULT  (AdcRegs.ADCRESULT5)    // AI2对应的ADCRESULT
#if 0
#define AI1_RESULT  (AdcRegs.ADCRESULT1)    // AI1对应的ADCRESULT
#define AI2_RESULT  (AdcRegs.ADCRESULT3)    // AI2对应的ADCRESULT
#define AI3_RESULT  (AdcRegs.ADCRESULT5)    // AI2对应的ADCRESULT
    aiDeal[0].sample = AI1_RESULT;                 // AI1的采样值, Q16
    aiDeal[1].sample = AI2_RESULT;                 // AI2的采样值, Q16
    aiDeal[2].sample = AI3_RESULT;                 // AI2的采样值, Q16
#endif    
#if 0
    ai1Sum = 0;
    for (i = AI1_NUMBER-1; i > 0; i--)
    {
        ai1Array[i] = ai1Array[i - 1];
        ai1Sum += ai1Array[i];
    }
    ai1Array[0] = aiDeal[0].sample;
    ai1Sum += ai1Array[0];
    aiDeal[0].sample = ai1Sum / AI1_NUMBER;
#endif
#endif

    aiFilterTime[0] = funcCode.code.ai1FilterTime;
    aiFilterTime[1] = funcCode.code.ai2FilterTime;
    aiFilterTime[2] = funcCode.code.ai3FilterTime;
    
    aiMin = (int32)AI_MIN_VOLTAGE_HW*10;
    for (j = AI_NUMBER - 1; j >= 0; j--)
    //j = 0;
    {
        // AI3为温度传感器时不进行以下处理
        if ((j == AI_NUMBER - 1) 
            && (FUNCCODE_tempSenorType_NONE != funcCode.code.motorOtMode)
            )
        {
            aiMin = 0;
            continue;
        }
// AI滤波
        aiLpf[j].t = aiFilterTime[j] * (TIME_UNIT_AI_PULSE_IN_FILTER / AI_CALC_PERIOD); // 滤波
        aiLpf[j].in = aiDeal[j].sample;      // Q16
        aiLpf[j].calc(&aiLpf[j]);

        aiLine.mode = 0;    // 限幅

        aiLine.x1 = 0;
        aiLine.y1 = aiMin;
        aiLine.x2 = (4095L << 4);
        aiLine.y2 = (int32)AI_MAX_VOLTAGE_HW*10;
        aiLine.x = aiLpf[j].out;
        aiLine.calc(&aiLine);
        aiDeal[j].voltageOrigin = aiLine.y;
        aiMin = 0;
        
// AI校正曲线
#define SIZE_AI_CALIBRATE   sizeof(struct ANALOG_CALIBRATE_CURVE)   // AI校正曲线的size
        p = (&funcCode.code.aiCalibrateCurve[0].before1 + SIZE_AI_CALIBRATE * j);
#undef SIZE_AI_CALIBRATE
        aiLine.mode = 1;    // 不限幅
        aiLine.y1 = (int16)(*(p + 0));       // 精密仪器测量电压(校正后)
        aiLine.x1 = (int16)(*(p + 1));       // 变频器采样电压(校正前) 
        aiLine.y2 = (int16)(*(p + 2));       // 精密仪器测量电压(校正后)
        aiLine.x2 = (int16)(*(p + 3));       // 变频器采样电压(校正前)
        aiLine.x = aiDeal[j].voltageOrigin;  // 采样电压
        aiLine.calc(&aiLine);

        if (aiDeal[j].voltageOrigin == 0)
        {
            aiDeal[j].voltage = 0;   // 采样电压为0时实际电压也为0
        }
        else
        {
            aiDeal[j].voltage = aiLine.y / 10;   // 实际电压
        }
        
// 找AI设定曲线
        aiNum = (funcCode.code.aiCurveSrc >> (j * 4)) & 0x000F;
        p = pAiSetCurve[aiNum - 1];
        if (aiNum >= 4)     // 4点的曲线。曲线4和曲线5为4点曲线
        {
            // 设定曲线的上下限已经保证从小到大
            for (i = 2; i >= 0; i--) // 4点，有3段。确定当前AI电压在3段中的哪一段。
            {
                if (aiDeal[j].voltage  >= (int16)(*(p + i*2))) 
                {
                    break;
                }
            }
            if (i < 0)
            {
                i = 0;
            }
            
            p += i * 2;
        }

// AI设定
        if((aiNum >= 4) && (i > 0))   // 为4点曲线且当前位置不在第一段
        {
            aiLine.mode = 0;    // 限幅
        }
        else
        {
            if((funcCode.code.aiLimitSrc >> (j * 4)) & 0x000F)
            {
                aiLine.mode = 2;    // 限幅且低于下限为0
            }
            else
            {
                aiLine.mode = 0;    // 限幅
            }
        }
        //aiLine.mode = 0;        // 限幅
        aiLine.x1 = ((int32)(int16)(*(p + 0)) * 33553)>>10;  // 优化
        aiLine.y1 = ((int32)(int16)(*(p + 1)) * 33553)>>10;  // 优化
        aiLine.x2 = ((int32)(int16)(*(p + 2)) * 33553)>>10;  // 优化
        aiLine.y2 = ((int32)(int16)(*(p + 3)) * 33553)>>10;  // 优化
        //aiLine.x = aiLine.y;
        aiLine.x = aiDeal[j].voltage * 33553>>10;  // Q10 32.767
        aiLine.calc(&aiLine);
        aiDeal[j].set = aiLine.y;

        //aiDeal[j].set = (int32)aiDeal[j].set * 2 - (32767);

// AI跳跃点
#define SIZE_AI_JUMP sizeof(struct AI_JUMP)     // AI JUMP的size
        p = (&funcCode.code.aiJumpSet[0].point + SIZE_AI_JUMP * j);
        point = (int32)(int16)(*(p + 0)) * 33553 >> 10;   // 设定跳跃点
        range = (int32)(int16)(*(p + 1)) * 33553 >> 10;   // 设定跳跃幅度
        if ((point - range < aiLine.y) && 
            (aiLine.y < point + range)      // 若在跳跃范围之内
            ) 
        {
            aiDeal[j].set = point;
        }
#undef SIZE_AI_JUMP
    }
    
    //aiDeal[2].voltageOrigin = aiLpf[2].out * 20000 / (4095L << 4) - 10000; // 校准前电压
#endif
} // AiCalc()


//=====================================================================
//
// AO处理函数
// AO1, AO2(与FMP二选一)
//
//=====================================================================
void AoCalcChannel(Uint16 channel)
{
#if DEBUG_F_AO
    int32 outCoeff;
    Uint32 tmpAo;
    
#if DSP_2803X
	volatile struct EPWM_REGS *EPwmRegs  = &EPwm6Regs;
#else
    volatile struct ECAP_REGS *pECapRegs = &ECap3Regs;
#endif
       
    LINE_STRUCT aoLine = LINE_STRTUCT_DEFALUTS;
    struct ANALOG_CALIBRATE_CURVE *pAo = &funcCode.code.aoCalibrateCurve[0];

    if (AOFMP_AO1 == channel)           // AO1
    {
        ;
    }
    else //if (AOFMP_AO2 == channel)    // AO2
    {
#if DSP_2803X
        EPwmRegs =  &EPwm4Regs;
#else
        pECapRegs = &ECap2Regs;
#endif
    }

#if DEBUG_F_PLC_CTRL
    if (funcCode.code.plcEnable
        && (AOFMP_AO1 == channel)
        && (funcCode.code.outPortControl >= 10000) 
        )
    {
        aoFmpValue = funcCode.code.ao1Value;
        aoFmpMax = 1000;
    }
    else
#endif        
    {
        UpdateFmpAoValue(funcCode.code.aoFunc[channel], channel);
    }
    outCoeff = (aoFmpValue << 15) / aoFmpMax;   // Q15

    outCoeff = (outCoeff * 1000) / 1047;   // 校正1046V电压
// 功能码的零偏和增益
    outCoeff = ((outCoeff * (int32)(int16)funcCode.code.aoPara[channel-1].gain) * 327 >> 15)
        + (((int32)(int16)funcCode.code.aoPara[channel-1].offset * 2097) >> 6);
    
// AO校正曲线
    // 注意保证不要溢出
    // 校正应该放在功能码修正之后
    if (outCoeff < 0)
    {
        outCoeff = 0;
    }
    else if (outCoeff > (Uint32)(1.5*32768))
    {
        outCoeff = (Uint32)(1.5*32768);
    }

    pAo = &funcCode.code.aoCalibrateCurve[channel-1];
    aoLine.mode = 1;    // 不限幅
    aoLine.x1 = ((int32)(int16)pAo->after1  * 13421 ) >> 12;  // 万用表测量电压
    aoLine.y1 = ((int32)(int16)pAo->before1 * 13421 ) >> 12;  // 理论输出电压
    aoLine.x2 = ((int32)(int16)pAo->after2  * 13421 ) >> 12;  // 万用表测量电压
    aoLine.y2 = ((int32)(int16)pAo->before2 * 13421 ) >> 12;  // 理论输出电压
    aoLine.x = outCoeff;
    aoLine.calc(&aoLine);
    outCoeff = aoLine.y;

    if (outCoeff < 0)
    {
        outCoeff = 0;
    }
    else if (outCoeff >= (1U << 15))
    {
        outCoeff = (1U << 15) + 100;
    }
    // outCoeff > 1，输出100%
    // outCoeff=1时，有1sysclk的低脉冲
    
    tmpAo = (outCoeff * (AO_PWM_PERIOD >> 3)) >> (15 - 3);    // 2^15*11*1250 < 2^31

#if DSP_2803X
    EPwmRegs->TBPRD = AO_PWM_PERIOD;    // 当前周期有效
    EPwmRegs->CMPA.half.CMPA  = tmpAo;            // 下一周期有效
#else
    pECapRegs->CAP1 = AO_PWM_PERIOD;    // 当前周期有效
    pECapRegs->CAP4 = tmpAo;            // 下一周期有效
#endif
      
#endif
} // AoCalcChannel()


//=====================================================================
//
// pulse in 处理函数
// 使用ECAP1。
//
// 1. 采用delta模式。初始化在InitSetEcap4()中。
// 2. 目前的精度(PulseInSample()调用周期100us下，采用判断CAPx方式)：
//     < 8KHz，不计小数，0%%，即显示频率无跳动。
//     < 40KHz，误差 < 1%%。
//     50-40KHz, 大约在 0.5%%-1%%，少数在1.5%%，极个别在2%%。
//     若，DSP的PULSE IN 直接接到DSP的PULSE OUT，误差非常小，即使频率达到500K，也能很好的采样，计算的值基本没有变化。
// 3. 本周期有捕获，利用捕获值进行计算。
// 4. 本周期无捕获，进行估算。
//    在一定时间内没有脉冲输入(捕获)，认为脉冲频率为0。由宏PULSE_IN_ZERO设置，目前设置为1000ms。
// 5. PulseInSample()的调用周期。100us至500us都能比较好的满足精度，高频时500us误差会稍大。
// 6. 所有CAP都在PulseInSample()中进行记录，不再在PulseInCalc()中记录。PulseInCalc()中仅进行计算。
//
//=====================================================================
//#define HDI_CAP (ECap4Regs)
void PulseInCalc(void)
{
#if DEBUG_F_HDI
    LINE_STRUCT pulseInLine = LINE_STRTUCT_DEFALUTS;

    static Uint16 pulseCounterRemainder;
#if 1
    static Uint32 lineSpeedCounter;
    static Uint16 lineSpeedCounterRemainder;
    static Uint32 lineSpeedTicker;
#endif
    static Uint16 lengthTickerInOld;
    static Uint16 counterTickerInOld;
    Uint16 pulseCounter;
    Uint16 frq1;
    Uint16 captureTickerTmp = captureTicker;

    Uint16 counterTickerOld;    // counterTicker备份
    Uint16 counterTickerDelta;
    Uint16 lengthCurrentOld;    // lengthCurrent备份
    Uint16 lengthTickerDelta;

// 这段时间(PULSE_IN_CALC_PERIOD)内没有捕获，noCaptureTicker加1
    if (!captureTicker)
    {
        ++noCaptureTicker;
    }
    
// 前面几个周期和本周期无捕获，进行估算
    if (noCaptureTicker)
    {
        frq1 = ((TIME_UNIT_MS_PER_SEC << 1) / (noCaptureTicker * PULSE_IN_CALC_PERIOD) + 1) >> 1;

        if (pulseInFrq1 > frq1)  // 仅在之前频率大于估算频率时，即频率在减小时，才进行估算。
        {
            pulseInFrq1 = frq1;
        }
    }
    
// 本周期有捕获
    if (captureTicker)
    {
#if (DSP_CLOCK == 100)
        pulseInFrq1 = ((((100UL * 1000000UL) << 5) / capturePeriodSum) * captureTicker + (1 << 4)) >> 5;
#elif (DSP_CLOCK == 60)
        pulseInFrq1 = ((((60UL * 1000000UL) << 5) / capturePeriodSum) * captureTicker + (1 << 4)) >> 5;
#endif
        
        capturePeriodSum = 0;   // 捕获的时间清零
        captureTicker = 0;      // 捕获的脉冲数清零
        
        noCaptureTicker = 0;    // 有捕获，则将noCaptureTicker清零
    }

#if (TIME_UNIT_MS_PER_SEC / PULSE_IN_ZERO > 1) // 能测量的最小频率 <= 1Hz，不需要这个比较顺序。
    if (pulseInFrq1 < TIME_UNIT_MS_PER_SEC / PULSE_IN_ZERO)
    {
        pulseInFrq1 = 0;
    }
#endif

#if 1   // counter的计算放在LPF之前，结果更准确。
    pulseCounter = ((Uint32)pulseInFrq1 * PULSE_IN_CALC_PERIOD + pulseCounterRemainder) / TIME_UNIT_MS_PER_SEC;
    pulseCounterRemainder = ((Uint32)pulseInFrq1 * PULSE_IN_CALC_PERIOD + pulseCounterRemainder) % TIME_UNIT_MS_PER_SEC;

    if (pulseInFrq1 > COUNTER_IN_USE_CAPTURE_TICKER_LIMIT)
    {
        counterTickerDelta = pulseCounter;
        lengthTickerDelta = pulseCounter;
    }
    else    // 采用捕获次数，使其更精确
    {
        counterTickerDelta = captureTickerTmp;
        lengthTickerDelta = captureTickerTmp;
    }
#endif

// pulseInFrq滤波
    pulseInLpf.t = (int32)funcCode.code.pulseInFilterTime * (TIME_UNIT_AI_PULSE_IN_FILTER / PULSE_IN_CALC_PERIOD); // 滤波
    pulseInLpf.in = pulseInFrq1;
    ///
    pulseInLpf.calc(&pulseInLpf);
    pulseInFrq = pulseInLpf.out;

// 计算pulseInSet
    pulseInLine.x1 = (int32)(int16)funcCode.code.curveSet2P4[0] * 10 / 2; // pulseInFrq的单位为1Hz，所以这里要 *10
    pulseInLine.x2 = (int32)(int16)funcCode.code.curveSet2P4[2] * 10 / 2;
    pulseInLine.y1 = ((int32)(int16)funcCode.code.curveSet2P4[1] * 33553) >> 10;   //优化除法
    pulseInLine.y2 = ((int32)(int16)funcCode.code.curveSet2P4[3] * 33553) >> 10;   //优化除法
    
    pulseInLine.x = pulseInFrq / 2;
    pulseInLine.calc(&pulseInLine);
    pulseInSet = pulseInLine.y;

#if 0   // counter的计算放在LPF之后，可能不准确。
    pulseCounter = ((Uint32)pulseInFrq * PULSE_IN_CALC_PERIOD + pulseCounterRemainder) / TIME_UNIT_MS_PER_SEC;
    pulseCounterRemainder = ((Uint32)pulseInFrq * PULSE_IN_CALC_PERIOD + pulseCounterRemainder) % TIME_UNIT_MS_PER_SEC;

    if (pulseInFrq > COUNTER_IN_USE_CAPTURE_TICKER_LIMIT)
    {
        counterTickerDelta = pulseCounter;
        lengthTickerDelta = pulseCounter;
    }
    else
    {
        counterTickerDelta = captureTickerTmp;
        lengthTickerDelta = captureTickerTmp;
    }
#endif

#if 1   //+= 线速度计算
#define LINE_SPEED_CALC_PERIOD  100     // _ms更新一次lineSpeed
    lineSpeedCounter += pulseCounter;
    if (++lineSpeedTicker >= LINE_SPEED_CALC_PERIOD / PULSE_IN_CALC_PERIOD) // _ms更新一次
    {
        lineSpeed = (lineSpeedCounter * 10 * 60 * 10 + lineSpeedCounterRemainder) / funcCode.code.lengthPulsePerMeter;
        lineSpeedCounterRemainder = (lineSpeedCounter * 10 * 60 * 10 + lineSpeedCounterRemainder)
            % funcCode.code.lengthPulsePerMeter;
        
        lineSpeedCounter = 0;
        lineSpeedTicker = 0;
    }
#endif

// 计数器输入
    if (DI_FUNC_COUNTER_TICKER_IN != funcCode.code.diFunc[4]) // DI5没有选择计数器输入
    {
        if (counterTickerInOld != diFunc.f1.bit.counterTickerIn) // 其他DI也可以设置为 计数器输入
        {
            counterTickerDelta = diFunc.f1.bit.counterTickerIn; // 1 or 0
            
            counterTickerInOld = diFunc.f1.bit.counterTickerIn;
        }
        else                    // 计数器输入增量为0
        {
            counterTickerDelta = 0;
        }
    }

    counterTickerOld = funcCode.code.counterTicker;
    funcCode.code.counterTicker += counterTickerDelta; // 不考虑溢出
    bOverFlowCounterTicker = 0;
    if (diFunc.f1.bit.resetCounter)         // 计数器复位
    {
        funcCode.code.counterTicker = 0;
    }
    else if (funcCode.code.counterTicker < counterTickerOld)    // 溢出
    {
        bOverFlowCounterTicker = 1;
    }

// 长度计数输入
    if (DI_FUNC_LENGTH_TICKER_IN != funcCode.code.diFunc[4])  // 其他DI也可以设置为 长度计数输入
    {
        if (lengthTickerInOld != diFunc.f1.bit.lengthTickerIn)
        {
            lengthTickerDelta = diFunc.f1.bit.lengthTickerIn;
            
            lengthTickerInOld = diFunc.f1.bit.lengthTickerIn;
        }
        else
        {
            lengthTickerDelta = 0;
        }
    }

    lengthCurrentOld = funcCode.code.lengthCurrent;
    funcCode.code.lengthCurrent += ((Uint32)lengthTickerDelta * 10 + funcCode.code.lengthTickerRemainder) 
        / funcCode.code.lengthPulsePerMeter; // 更新实际长度。10-lengthPulsePerMeter的单位
    funcCode.code.lengthTickerRemainder = ((Uint32)lengthTickerDelta * 10 + funcCode.code.lengthTickerRemainder) 
        % funcCode.code.lengthPulsePerMeter;
    bOverFlowLengthCurrent = 0;
    if (diFunc.f1.bit.resetLengthCounter)   // 长度复位
    {
        funcCode.code.lengthCurrent = 0;
        funcCode.code.lengthTickerRemainder = 0;
    }
    else if (funcCode.code.lengthCurrent < lengthCurrentOld)    // 溢出
    {
        bOverFlowLengthCurrent = 1;
    }
#endif
} // PulseInCalc()


//=====================================================================
//
// Pulse In 采样函数
// 使用ECAP1。
//
// 1. 捕获的值ECap1Regs.CAPx，是这次捕获与上次捕获之间的delta。
// 即ECap1Regs.ECCTL1.bit.CTRRST1 = EC_DELTA_MODE;
// 
// 2. 由于 PULSE_IN_CALC_PERIOD 时间内或者每次pulse in计算capturePeriodSum, captureTicker清零，
// 所以capturePeriodSum, captureTicker不会溢出。
//
// 3. 有捕获时，将CAP1/CAP2/CAP3/CAP4清零。通过判断CAPx是否为0确定是否有新的捕获。
//
// 4. 于是，本函数可以放在0.5ms周期中，即使放在100us周期也可以。放在100us周期中精度最高。
//
//=====================================================================
void PulseInSample(void)
{
#if DEBUG_F_HDI
    Uint32 ts1, ts2, ts3, ts4;
#if DSP_2803X
    volatile struct ECAP_REGS *pECapRegs = &ECap1Regs;
#else
    volatile struct ECAP_REGS *pECapRegs = &ECap4Regs;
#endif
    ts1 = pECapRegs->CAP1;
    if (ts1)
    {
        pECapRegs->CAP1 = 0;
        captureTicker++;
    }

    ts2 = pECapRegs->CAP2;
    if (ts2)
    {
        pECapRegs->CAP2 = 0;
        captureTicker++;
    }

    ts3 = pECapRegs->CAP3;
    if (ts3)
    {
        pECapRegs->CAP3 = 0;
        captureTicker++;
    }

    ts4 = pECapRegs->CAP4;
    if (ts4)
    {
        pECapRegs->CAP4 = 0;
        captureTicker++;
    }

    capturePeriodSum += ts1 + ts2 + ts3 + ts4;
#endif
}

#if DSP_2803X
#define HDO_PRD     (EPwm5Regs.TBPRD)
#define HDO_CMP     (EPwm5Regs.CMPB)
#define HDO_CTR     (EPwm5Regs.TBCTR)
#endif
//=====================================================================
//
// FMP处理函数
// 
//=====================================================================
void FMPDeal(void)
{
#if (DEBUG_F_AO || DEBUG_F_HDO || DEBUG_F_DO)

#if DSP_2803X
    static Uint16 fmOutSelectOld;

    if (fmOutSelectOld != funcCode.code.fmOutSelect) // F5-00  多功能端子输出选择
    {
        fmOutSelectOld = funcCode.code.fmOutSelect;
        EPwm5Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;    // TBCLK = SYSCLK
        EPwm5Regs.TBCTL.bit.CLKDIV = TB_DIV1;
        HDO_CTR = 0;
    }

    if (FUNCCODE_fmOutSelect_PULSE == funcCode.code.fmOutSelect)    // HDO
    {
        PulseOutCalc();
    }
    else if (FUNCCODE_fmOutSelect_DO == funcCode.code.fmOutSelect)  // DO
    {
        HDO_CTR = 0;        // 将counter设置成period以下。
        HDO_PRD = PULSE_OUT_CTR_PERIOD;     // period

        if (doStatus.c.bit.do3)                 // 周期为0
        {
            HDO_CMP = PULSE_OUT_CTR_PERIOD + 1;  // CMP >= PERIOD + 1, 强制输出1
        }
        else                        // 频率为0，频率小于一定值
        {
            HDO_CMP = 0;            // CMP = 0, 强制输出0
        }
    }
#if 0  // 2803x平台中AO2与FMR是分离的
    else //if (FUNCCODE_fmOutSelect_AO == funcCode.code.fmOutSelect)   // AO2
    {
        AoCalcChannel(AOFMP_AO2);
    }
#endif
#else
    static Uint16 fmOutSelectOld;

    if (fmOutSelectOld != funcCode.code.fmOutSelect) // F5-00  多功能端子输出选择
    {
        ECap2Regs.TSCTR = 0;
        fmOutSelectOld = funcCode.code.fmOutSelect;
    }

    if (FUNCCODE_fmOutSelect_PULSE == funcCode.code.fmOutSelect)    // HDO
    {
        PulseOutDeal();
    }
    else if (FUNCCODE_fmOutSelect_DO == funcCode.code.fmOutSelect)  // DO
    {
        // DO3(FM)的输出，是使用APWM输出的
        ECap2Regs.TSCTR = 0;                        // 将counter设置成period以下。
        ECap2Regs.CAP1 = PULSE_OUT_CTR_PERIOD;      // period
        
        if (doStatus.c.bit.do3)                     // FM(DO3)输出选择
        {
            ECap2Regs.CAP2 = PULSE_OUT_CTR_PERIOD + 1;    // CMP >= PERIOD + 1, 强制输出1
        }
        else
        {
            ECap2Regs.CAP2 = 0;                     // CMP = 0, 强制输出0
        }
    }
    else //if (FUNCCODE_fmOutSelect_AO == funcCode.code.fmOutSelect)   // AO2
    {
        AoCalcChannel(AOFMP_AO2);
    }
#endif
#endif
}

      
void PulseOutCalc(void)
{
#if DEBUG_F_HDO
#if DSP_2803X
    static Uint16 periodOld = MAX_UINT16;
    static Uint16 beforeFrq = 200;
    static Uint16 CLOCK_BENCH = 1;
    Uint16 period;
    Uint64 tmp;
    Uint16 newCompare;
    Uint16 oldCompare;
    Uint16 HDOCTR;

    Uint16 curFrq;

#if DEBUG_F_PLC_CTRL
    if (funcCode.code.plcEnable 
        && ((funcCode.code.outPortControl >= 11000) 
            || ((funcCode.code.outPortControl < 10000) && (funcCode.code.outPortControl >= 1000)))
        )
    {
        aoFmpMax = 1000;
        aoFmpValue = funcCode.code.fmpValue;
    }
    else
#endif
    {
        UpdateFmpAoValue(funcCode.code.aoFunc[AOFMP_FMP], AOFMP_FMP);
    }
    curFrq = aoFmpValue * funcCode.code.fmpOutMaxFrq / aoFmpMax;
    HDOCTR = HDO_CTR;
    
    // 高低频输出切换(2KHZ为切换点)
    if ((curFrq >= 200) && (beforeFrq < 200))
    {
        // 低频切换到高频
        if (HDOCTR > 292)
        {
            HDOCTR = 0xFFFF;
        }
        else
        {
            HDOCTR = HDOCTR * 224;
        }
        
        if (periodOld > 292)
        {
            periodOld = 0xFFFF;
        }
        else
        {
            periodOld = periodOld * 224;
        }
        
        //EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;    // TBCLK = SYSCLK
        //EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;
        CLOCK_BENCH = 1;
    }
    else if ((curFrq < 200) && (beforeFrq >= 200))
    {
        // 高频切换到低频
        HDOCTR = HDOCTR / 224;
        periodOld = periodOld / 224;

        //EPwm4Regs.TBCTL.bit.HSPCLKDIV = 7;          // TBCLK = SYSCLK/28
        //EPwm4Regs.TBCTL.bit.CLKDIV = 4;
        CLOCK_BENCH = 224;
    }

    tmp  = (Uint64)(DSP_CLOCK/10) * 1000000UL * aoFmpMax / (aoFmpValue * funcCode.code.fmpOutMaxFrq);
    tmp /= CLOCK_BENCH;
    if ((tmp > MAX_UINT16) || (!aoFmpValue)) // 大于MAX_UINT16，或者outValue=0，一直输出0频率
    {
        tmp = MAX_UINT16;
    }
    period = tmp;
 
    if (period)
        period--;

    oldCompare = periodOld >> 1;
    newCompare = period >> 1;              // 默认占空比50%

    if (((MAX_UINT16 - 1) != period) && period)   // 周期为0，或者达到2^16-1
    {
        // HDOCTR = HDO_CTR;
        // 频率改变
        if (period != HDO_PRD)
        {
            // 频率从高变成低
            if (periodOld < period)     
            { 
                if (HDOCTR > oldCompare)
                {
                    HDOCTR = HDOCTR - oldCompare + newCompare;  // 注意无符号数的运算
                }		
            }
            // 频率从低变成高(周期由大变小)
            else                        
            {   
                // 计时处于后半周期且超过新的半周期
                if (HDOCTR > newCompare + oldCompare)
                {
                    // 停止当前周期重新开始计算
                    HDOCTR = 0;
                }
                // 计时处于后半周期且未超过新的半周期
                else if (HDOCTR > oldCompare)
                {
                    // 直接以当前计时值开始新的后半周期计算
                    HDOCTR = HDOCTR - oldCompare + newCompare;
                }
                // 计时处于前半周期且计时值大于新的半周期
                else if (HDOCTR > newCompare)
                {
                    // 立即进入后半周期
                    HDOCTR = newCompare;
                }
                // 计时处于前半周期且计时值小于新的半周期, 直接以当前计时值开始新的前半周期计算
            } 
            
            if (HDOCTR > period)
	        {
	            HDOCTR = 0;
	        }

	        DINT;   
            EALLOW;
			if (CLOCK_BENCH == 1)
            {
                EPwm5Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;    // TBCLK = SYSCLK
                EPwm5Regs.TBCTL.bit.CLKDIV = TB_DIV1;
            }
            else
            {
                EPwm5Regs.TBCTL.bit.HSPCLKDIV = 7;          // TBCLK = SYSCLK/28
                EPwm5Regs.TBCTL.bit.CLKDIV = 4;
            }
            HDO_PRD = period;            // 再赋周期值period
			HDO_CMP = newCompare;        // 先赋比较值compare
			HDO_CTR = HDOCTR;
     
            EDIS;
	        EINT;
        }
        
    }
    // 周期为0，或者达到2^16-1
    else  
    {
        HDO_CTR = 0;        // 将counter设置成period以下。
        HDO_PRD = PULSE_OUT_CTR_PERIOD;     // period

        if (!period)                // 周期为0
        {
            HDO_CMP = PULSE_OUT_CTR_PERIOD + 1;  // CMP >= PERIOD + 1, 强制输出1
        }
        else                        // 频率为0，频率小于一定值
        {
            HDO_CMP = 0;            // CMP = 0, 强制输出0
        }
    }

    periodOld = period;
    beforeFrq = curFrq;
#endif
#endif
}



// pulse out
// 使用ECAP2
//
// 无论改变频率的时刻在前半个周期，还是在后半个周期，都立即开始新的频率。
//#define HDO_PRD     (ECap2Regs.CAP1)
//#define HDO_CMP     (ECap2Regs.CAP2)
//#define HDO_CTR     (ECap2Regs.TSCTR)
#if !DSP_2803X
void PulseOutDeal(void)
{
#if DEBUG_F_HDO
#if !DSP_2803X
    static Uint32 periodOld = MAX_UINT32;
    Uint32 period;
    Uint64 tmp;

    Uint32 newCompare;
    Uint32 oldCompare;

// 考虑某些极限情况。如fmpOutMaxFrq = 1，aoFmpMax = 63000，aoFmpValue = 10

#if DEBUG_F_PLC_CTRL
    if (funcCode.code.plcEnable 
        && ((funcCode.code.outPortControl >= 11000) 
            || ((funcCode.code.outPortControl < 10000) && (funcCode.code.outPortControl >= 1000)))
        )
    {
        aoFmpMax = 1000;
        aoFmpValue = funcCode.code.fmpValue;
    }
    else
#endif
    {
        UpdateFmpAoValue(funcCode.code.aoFunc[AOFMP_FMP], AOFMP_FMP);
    }

// fmpOutMaxFrq单位，0.01kHz
    tmp  = (Uint64)(DSP_CLOCK/10) * 1000000UL * aoFmpMax / (aoFmpValue * funcCode.code.fmpOutMaxFrq);
    
    if ((tmp > MAX_UINT32) || (!aoFmpValue)) // 大于MAX_UINT32，或者outValue=0，一直输出0频率
    {
        tmp = MAX_UINT32;
    }
    
    period = tmp;
    if (period)
        period--;

    oldCompare = periodOld >> 1;
    newCompare = period >> 1;              // 默认占空比50%
//    newCompare = (period * 3277) >> 15;   // 占空比10%
//    newCompare = (period * 1966) >> 16;   // 占空比3%

    if (((MAX_UINT32-1) != period) && period)   // 周期为0，或者达到2^32-1
    {
        ECap2Regs.CAP2 = newCompare;    // 先赋比较值compare
        ECap2Regs.CAP1 = period;        // 再赋周期值period

        if (periodOld < period)     // 频率从高变成低，周期从短变长
        {
            DINT;                   // 这时应该关闭全局中断，也可暂停计数器
            
            /*
            * 无论改变频率的时刻在前半个周期
            * 还是在后半个周期
            * 都立即开始新的频率。
            */
            if (ECap2Regs.TSCTR > oldCompare)
                ECap2Regs.TSCTR = ECap2Regs.TSCTR - oldCompare + newCompare; // 注意无符号数的运算
                
            EINT;                   // 重新开启全局中断
        }
        else if (periodOld > period)// 频率从低变成高，周期从长变短
        {
            DINT;                   // 这时应该关闭全局中断，也可暂停计数器
            
            /* 
            * 改变频率的时刻在后半个周期
            */
            // 处于后半个周期的时间超过新的半周期时间，
            if (ECap2Regs.TSCTR > newCompare + oldCompare)
            {
                // 立即进入新的前半周期。
                ECap2Regs.TSCTR = 0;
            }
            // 处于后半个周期的时间没有超过新的半周期时间，
            else if (ECap2Regs.TSCTR > oldCompare)
            {
                // 立即开始新的频率
                ECap2Regs.TSCTR = ECap2Regs.TSCTR - oldCompare + newCompare;
            }
            
            /*
            * 改变频率的时刻在前半个周期
            */
            // 处于前半个周期的时间超过新的半周期时间，
            else if (ECap2Regs.TSCTR > newCompare)
            {
                // 立即进入新的后半周期
                ECap2Regs.TSCTR = newCompare;
            }
            // 处于前半个周期的时间没有超过新的半周期时间，立即开始新的频率。

            // 重新开启全局中断
            EINT;                   
        }
        // 周期值(输出脉冲频率)未改变，不更改周期值。

        if (ECap2Regs.TSCTR > period)
        {
            ECap2Regs.TSCTR = 0;
        }
    }
    else
    {
        ECap2Regs.TSCTR = 0;        // 将counter设置成period以下。
        ECap2Regs.CAP1 = PULSE_OUT_CTR_PERIOD;     // period

        if (!period)                // 周期为0
        {
            ECap2Regs.CAP2 = PULSE_OUT_CTR_PERIOD + 1; // CMP >= PERIOD + 1, 强制输出1
        }
        else                        // 频率为0，频率小于一定值
        {
            ECap2Regs.CAP2 = 0;     // CMP = 0, 强制输出0
        }
    }

    periodOld = period;
#endif
#endif
} // FMPDeal()
#endif

//=====================================================================
//
// 更新FMP(Pulse Out)，AO的输出值，输出最大值
//
// 输出:
//      aoFmpValue      FMP,AO的输出值
//      aoFmpMax        FMP,AO的输出最大值
//
// 备注：
// 1、输出电流。停机时，由于outCurrent可能不为0，所以会有这种情况出现:
// 显示电流为0，但PULSE OUT会有不等的脉冲输出。
// 性能配合的话可以消除这种情况。
// 
// 2、当F3-00(多功能端子输出选择)改变时，需要在APWM的周期值赋值之后，
// 将计数器TSCTR清零。
//
//=====================================================================
LOCALF void UpdateFmpAoValue(Uint16 func, Uint16 aoOrFmp)
{
#if (DEBUG_F_AO || DEBUG_F_HDO)
    switch (func)
    {
        case AO_FMP_FUNC_FRQ_SET:       // 运行频率
            aoFmpValue = frqDisp;
            //aoFmpValue = frq;
            //+= aoFmpValue = frqRun;
            aoFmpMax = maxFrq;
            break;

        case AO_FMP_FUNC_FRQ_AIM:            // 设定频率
            aoFmpValue = frqAim; 
            aoFmpMax = maxFrq;
            break;

        case AO_FMP_FUNC_OUT_CURRENT:           // 输出电流
            aoFmpValue = outCurrentDisp;        // 实际电流
            aoFmpMax = motorFc.motorPara.elem.ratingCurrent << 1;   // 2倍电机额定电流
            break;

        // 0~200.0%对应0~10V
        case AO_FMP_FUNC_OUT_TORQUE:    // 输出转矩
            aoFmpValue = ABS_INT16((int16)itDisp);
            aoFmpMax = 2000;            // 2倍电机额定转矩
            break;    

        case AO_FMP_FUNC_OUT_POWER:     // 输出功率
            aoFmpValue = outPower >> 1;
            aoFmpMax = motorFc.motorPara.elem.ratingPower;
            break;

        case AO_FMP_FUNC_OUT_VOLTAGE:       // 输出电压
            aoFmpValue = outVoltage;        // Q12
            aoFmpMax = (int32)(1.2*4096);   // 1.2倍变频器额定电压
            break;

        case AO_FMP_FUNC_PULSE_IN:          // PULSE脉冲输入
            aoFmpValue = pulseInFrq / 2;    // 1Hz
            aoFmpMax = 50000;               // 50kHz
            break;
            
        case AO_FMP_FUNC_AI1:           // AI1
        case AO_FMP_FUNC_AI2:           // AI2
        case AO_FMP_FUNC_AI3:           // AI3
            aoFmpValue = aiDeal[func - AO_FMP_FUNC_AI1].voltage;    // 0.01V
            aoFmpMax = 1000;                                        // 最大10.00V
            break;
            
        case AO_FMP_FUNC_LENGTH:        // 长度
            aoFmpValue = funcCode.code.lengthCurrent;
            aoFmpMax = funcCode.code.lengthSet;
            break;

        case AO_FMP_FUNC_COUNTER:       // 计数值
            aoFmpValue = funcCode.code.counterTicker;
            aoFmpMax = funcCode.code.counterSet;
            break;

        case AO_FMP_FUNC_COMM:          // 通讯控制输出
            aoFmpValue = aoComm[aoOrFmp];
            aoFmpMax = 0x7FFF;
            break;
            
        case AO_FMP_FUNC_SPEED:         // 电机运行转速
            aoFmpValue = frqRun;
            aoFmpMax = maxFrq;
            break;

        case AO_FMP_FUNC_OUT_CURRENT_1:
            aoFmpValue = outCurrentDisp;        // 实际电流
            aoFmpMax = 10000;                   // 1000A 2倍电机额定电流
             break;

        case AO_FMP_FUNC_OUT_VOLTAGE_1:
            aoFmpValue = generatrixVoltage;
            aoFmpMax = 10000;                   // 满量程为1000.0V
            break;

        // 输出转矩(带方向) -200.0%~200.0%对应0~10V
        case AO_FMP_FUNC_OUT_TORQUE_DIR:
            aoFmpValue = (int16)(itDisp) + 2000;
            aoFmpMax = 4000;                    // 2倍电机额定转矩
            break;
            
        default:
            break;
    }

    aoFmpValue = ABS_INT32(aoFmpValue);
    aoFmpMax = ABS_INT32(aoFmpMax);
    
    if (aoFmpValue > aoFmpMax)      // 限幅
    {
        aoFmpValue = aoFmpMax;
    }
#endif
}


//=====================================================================
//
// 相当于一条线段，已知两端点(x1,y1), (x2,y2), 求第三点(x, y)的y
// 请确保: x1 < x2
// 输出：Q4格式
//
// 1000 * 2^6 = 64000 < 65536，不会溢出。
// pulseInFrq目前最大为50000Hz，也不会溢出。
//
//=====================================================================
void LineCalc(LINE_STRUCT *p)
{
#if 0   // 若x1 < x2. 当x > x1, y = y2; 当x <= x1, y = y1.
    if (p->x <= p->x1)
        p->y = p->y1;
    else if (p->x >= p->x2)
        p->y = p->y2;
    else
        p->y = ((((int32)(p->x - p->x1) << 15) / (p->x2 - p->x1)) * (p->y2 - p->y1)
                 + ((int32)p->y1 << 15) + (1L << 14)) >> 15;
#elif 1
    int32 tmp;
    tmp = ((((int32)(p->x - p->x1) << 15) / (p->x2 - p->x1)) * (p->y2 - p->y1)
                 + ((int32)p->y1 << 15) + (1L << 14)) >> 15;
    
    // 若x1 < x2. 当x > x1, y = y2; 当x <= x1, y = y1.
    if((p->mode == 2) && (p->x < p->x1))    // 限幅且低于下限时为0
        p->y = 0;
    else if (p->x <= p->x1)
        p->y = p->y1;
    else if (p->x >= p->x2)
        p->y = p->y2;
    else
        p->y = tmp;
    
    if (p->mode == 1)    // 1，表示不限幅
        p->y = tmp;
#elif 1   //+e 可从数学上反应x1 < x2的情况。但暂不这样
    int32 max, min;

    max = MAX(p->x1, p->x2);
    min = MIN(p->x1, p->x2);

    if (p->x <= min)
        p->y = p->y1;
    else if (p->x >= max)
        p->y = p->y2;
    else
        p->y = ((((int32)(p->x - p->x1) << 15) / (p->x2 - p->x1)) * (p->y2 - p->y1)
                 + ((int32)p->y1 << 15) + (1L << 14)) >> 15;
#endif
}


//=====================================================================
//
// 低通滤波器函数
// 注意:
//
//=====================================================================
void LpfCalc(LowPassFilter *p)
{
#if 0   // 这一种效果好像差一点，原因以后再查找. 输出可能超过32767, 65535。有bug
    int32 tmp;
    int16 t1 = p->t * TIME_UNIT_AI_PULSE_IN_FILTER;

    p->outOld = p->out;
    if (!p->t)
    {
        p->out = p->in;
    }
    else
    {
        tmp = ((int32)p->in - p->outOld + p->remainder) * AI_CALC_PERIOD;
        p->out = tmp / t1 + p->outOld;
        p->remainder = tmp % t1;
    }
/* #elif 0                                         */
/*     int32 tmp;                                  */
/*                                                 */
/*     p->outOld = p->out;                         */
/*     if (!p->t)                                  */
/*     {                                           */
/*         p->out = p->in;                         */
/*     }                                           */
/*     else                                        */
/*     {                                           */
/*         tmp = p->in + p->remainder - p->outOld; */
/*         p->out = tmp / p->t + p->outOld;        */
/*         p->remainder = tmp % p->t;              */
/*     }                                           */
#elif 1
    int32 tmp;
    int32 out;

    p->outOld = p->out;
    if (!p->t)
    {
        p->out = p->in;
    }
    else
    {
        tmp = ((int32)p->in - p->outOld + p->remainder) * AI_CALC_PERIOD;
        out = tmp / (p->t * 1) + p->outOld;
 
            
            
        p->out = tmp / (p->t * 1) + p->outOld;
        p->remainder = tmp % (p->t * 1);
    }
#endif
}

////////////////////////////////////////////////////////////////
//
//  2803x AO2
//
////////////////////////////////////////////////////////////////
void InitSetEPWM4(void)
{
#if (DEBUG_F_AO)  
#if DSP_2803X
    EALLOW;
    EPwm4Regs.TBPRD = 0;                    // Period = 601 TBCLK counts
    EPwm4Regs.CMPA.half.CMPA = 0;           // Compare A = 350 TBCLK counts
    EPwm4Regs.TBPHS.all = 0;                // Set Phase register to zero
    EPwm4Regs.TBCTR = 0;                    // clear TB counter
    EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;  // UP_Count Mode
    EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;     // Phase loading disabled
    EPwm4Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;    // TBCLK = SYSCLK
    EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR = Zero
    EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR = Zero
    EPwm4Regs.AQCTLA.bit.ZRO = AQ_SET;
    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    //EPwm4Regs.AQCTLB.bit.ZRO = AQ_SET;
    //EPwm4Regs.AQCTLB.bit.CBU = AQ_CLEAR;
    EDIS;
#endif
#endif
}

///////////////////////////////////////////////
//
// 2803x平台HDO1
//
///////////////////////////////////////////////
void InitSetEPWM5(void)
{
 #if ( DEBUG_F_HDO || DEBUG_F_DO)
 #if DSP_2803X
    EALLOW;
    EPwm5Regs.TBPRD = 0;                   // Period = 601 TBCLK counts
    EPwm5Regs.CMPB = 0;                    // Compare B = 200 TBCLK counts
    EPwm5Regs.TBPHS.all = 0;               // Set Phase register to zero
    EPwm5Regs.TBCTR = 0;                   // clear TB counter
    
    EPwm5Regs.TBCTL.bit.FREE_SOFT = 2;
    
    EPwm5Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;  // UP_Count Mode
    EPwm5Regs.TBCTL.bit.PHSEN = TB_DISABLE;     // Phase loading disabled
    EPwm5Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;
    EPwm5Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;
    EPwm5Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;    // TBCLK = SYSCLK
    EPwm5Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm5Regs.CMPCTL.bit.SHDWAMODE = CC_IMMEDIATE;
    EPwm5Regs.CMPCTL.bit.SHDWBMODE = CC_IMMEDIATE;
    EPwm5Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR = Zero
    EPwm5Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR = Zero
    //EPwm5Regs.AQCTLA.bit.ZRO = AQ_SET;
    //EPwm5Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm5Regs.AQCTLB.bit.ZRO = AQ_CLEAR;
    EPwm5Regs.AQCTLB.bit.CBU = AQ_SET;

    EDIS;
#endif
#endif
}

////////////////////////////////////////////////////////////////
//
//  2803x AO1
//
////////////////////////////////////////////////////////////////
void InitSetEPWM6(void)
{
#if (DEBUG_F_AO)  
#if DSP_2803X
    EALLOW;
    EPwm6Regs.TBPRD = 0;                // Period = 601 TBCLK count
    EPwm6Regs.CMPA.half.CMPA = 0;          // Compare A = 350 TBCLK counts
    EPwm6Regs.TBPHS.all = 0;               // Set Phase register to zero
    EPwm6Regs.TBCTR = 0;                   // clear TB counter
    EPwm6Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;  // UP_Count Mode
    EPwm6Regs.TBCTL.bit.PHSEN = TB_DISABLE;     // Phase loading disabled
    EPwm6Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm6Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;
    EPwm6Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;    // TBCLK = SYSCLK
    EPwm6Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm6Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm6Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm6Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR = Zero
    EPwm6Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR = Zero
    EPwm6Regs.AQCTLA.bit.ZRO = AQ_SET;
    EPwm6Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    //EPwm6Regs.AQCTLB.bit.ZRO = AQ_SET;
    //EPwm6Regs.AQCTLB.bit.CBU = AQ_CLEAR;
    EDIS;
#endif
#endif
}

// PULSE OUT, AO，DO3(FM)
#if !DSP_2803X
void InitSetEcap2(void)
{
#if (DEBUG_F_AO || DEBUG_F_HDO || DEBUG_F_DO)
#if !DSP_2803X
// Setup APWM mode on CAP2, set period and compare registers
    ECap2Regs.ECCTL1.bit.FREE_SOFT = 2;  // halt DSP时，该计数器还在运行
    ECap2Regs.ECCLR.all = 0xFFFF;        // Clear pending interrupts
    ECap2Regs.ECCTL2.all = 0x0216;

    //ECap2Regs.CAP1 = 0xffffffff;         // Set Period value
    ECap2Regs.CAP1 = 0;
    ECap2Regs.CAP2 = 0;                  // Set Compare value，初始化输出0
#endif
#endif
}
#endif

////////////////////////////////////////////////////////////////
//
//  2808 AO1
//
////////////////////////////////////////////////////////////////
#if !DSP_2803X
void InitSetEcap3(void)
{
#if DEBUG_F_AO
#if !DSP_2803X
    ECap3Regs.ECCTL1.bit.FREE_SOFT = 2;  // halt DSP时，该计数器还在运行
    
    ECap3Regs.ECCLR.all = 0xFFFF;        // Clear pending interrupts
    ECap3Regs.ECCTL2.all = 0x0216;

    ECap3Regs.CAP1 = 0;                  // Set Period value
    ECap3Regs.CAP2 = 0;                  // Set Compare value，初始化输出0
#endif
#endif
}
#endif

/////////////////////////////////////////////
//
// PULSE IN(2803x)
//
/////////////////////////////////////////////
void InitSetEcap1(void)
{
#if DEBUG_F_HDI
#if DSP_2803X
    ECap1Regs.ECEINT.all = 0x0000;             // Disable all capture interrupts
    ECap1Regs.ECCLR.all = 0xFFFF;              // Clear all CAP interrupt flags   
    ECap1Regs.ECCTL1.all = 0x8000;             // Disable CAP1-CAP4 register loads, halt DSP时，该计数器还在运行
    ECap1Regs.ECCTL2.bit.TSCTRSTOP = EC_FREEZE;// Make sure the counter is stopped

    ECap1Regs.ECCTL1.all = 0x81AA;
    ECap1Regs.ECCTL2.all = 0x0096;
#endif
#endif
}

/////////////////////////////////////////////
//
// PULSE IN(2808)
//
/////////////////////////////////////////////
#if !DSP_2803X
void InitSetEcap4(void)
{
#if DEBUG_F_HDI || DEBUG_F_POSITION_CTRL
    ECap4Regs.ECEINT.all = 0x0000;             // Disable all capture interrupts
    ECap4Regs.ECCLR.all = 0xFFFF;              // Clear all CAP interrupt flags   
    ECap4Regs.ECCTL1.all = 0x8000;             // Disable CAP1-CAP4 register loads, halt DSP时，该计数器还在运行
    ECap4Regs.ECCTL2.bit.TSCTRSTOP = EC_FREEZE;// Make sure the counter is stopped

    ECap4Regs.ECCTL1.all = 0x81AA;
    ECap4Regs.ECCTL2.all = 0x0096;
#endif
}
#endif



void InitDIGpio(void)
{
#if DSP_2803X     // 2803x还是2808平台
#if DEBUG_F_DI
    EALLOW;
    GpioCtrlRegs.GPBMUX1.bit.GPIO42 = 0;    // Configure GPIO42, DI
    GpioCtrlRegs.GPBDIR.bit.GPIO42 = 0;     // input
    GpioDataRegs.GPBDAT.bit.GPIO42 = 0;     // 

    GpioCtrlRegs.GPBMUX1.bit.GPIO43 = 0;    // Configure GPIO43, DI
    GpioCtrlRegs.GPBDIR.bit.GPIO43 = 0;     // input
    GpioDataRegs.GPBDAT.bit.GPIO43 = 0;     // 

    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;    // Configure GPIO27, DI
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 0;     // input
    GpioDataRegs.GPADAT.bit.GPIO27 = 0;     // 

    GpioCtrlRegs.GPBMUX1.bit.GPIO40 = 0;    // Configure GPIO40, DI
    GpioCtrlRegs.GPBDIR.bit.GPIO40 = 0;     // input
    GpioDataRegs.GPBDAT.bit.GPIO40 = 0;     // 

    //GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 0;    // Configure GPIO19, DI
    //GpioCtrlRegs.GPADIR.bit.GPIO19 = 0;     // input
    //GpioDataRegs.GPADAT.bit.GPIO19 = 0;     // 

    GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 0;    // Configure GPIO25, DI
    GpioCtrlRegs.GPADIR.bit.GPIO25 = 0;     // input
    GpioDataRegs.GPADAT.bit.GPIO25 = 0;     // 

    // DI7
    GpioCtrlRegs.GPBMUX1.bit.GPIO44 = 0;    // Configure GPIO44, DI
    GpioCtrlRegs.GPBDIR.bit.GPIO44 = 0;     // input
    GpioDataRegs.GPBDAT.bit.GPIO44 = 0;     // 

    GpioCtrlRegs.GPBMUX1.bit.GPIO35 = 0;    // Configure GPIO35, DI
    GpioCtrlRegs.GPBDIR.bit.GPIO35 = 0;     // input
    GpioDataRegs.GPBDAT.bit.GPIO35 = 0;     //

    GpioCtrlRegs.GPBMUX1.bit.GPIO36 = 0;    // Configure GPIO36, DI
    GpioCtrlRegs.GPBDIR.bit.GPIO36 = 0;     // input
    GpioDataRegs.GPBDAT.bit.GPIO36 = 0;     // 

    GpioCtrlRegs.GPBMUX1.bit.GPIO38 = 0;    // Configure GPIO38, DI
    GpioCtrlRegs.GPBDIR.bit.GPIO38 = 0;     // input
    GpioDataRegs.GPBDAT.bit.GPIO38 = 0;     //
    EDIS;
#endif 
#endif
}

void InitDOGpio(void)
{
#if DSP_2803X     // 2803x还是2808平台
#if DEBUG_F_DO
    EALLOW;
    // D01
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;    // Configure GPIO34, DO
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;     // output
    GpioDataRegs.GPBDAT.bit.GPIO34 = 1;     // 

    // DO2
    GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 0;    // Configure GPIO24, DO
    GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;     // output
    GpioDataRegs.GPADAT.bit.GPIO24 = 1;     // 
    
    // relay1
    GpioCtrlRegs.GPBMUX1.bit.GPIO41 = 0;    // Configure GPIO41, DO
    GpioCtrlRegs.GPBDIR.bit.GPIO41 = 1;     // output
    GpioDataRegs.GPBDAT.bit.GPIO41 = 1;     // 为1表示无效

    // relay2
    GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 0;    // Configure GPIO26, DO
    GpioCtrlRegs.GPADIR.bit.GPIO26 = 1;     // output
    GpioDataRegs.GPADAT.bit.GPIO26 = 1;     // 

    // FAN
    GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37, FAN
    GpioCtrlRegs.GPBDIR.bit.GPIO37 = 1;     // output
    GpioDataRegs.GPBDAT.bit.GPIO37 = 1;     // 

    //HDO1
    //GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;    // Configure GPIO09, DO
    //GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;     // output
    //GpioDataRegs.GPADAT.bit.GPIO9 = 0;     // 
    EDIS;
#endif    
#endif
}


void InitECap2Gpio(void)
{
#if (DEBUG_F_AO || DEBUG_F_HDO || DEBUG_F_DO)
#if !DSP_2803X
EALLOW;
   GpioCtrlRegs.GPAPUD.bit.GPIO7 = 1;      // Disable pull-up on GPIO7 (CAP2)
   GpioCtrlRegs.GPAQSEL1.bit.GPIO7 = 0;    // Synch to SYSCLKOUT GPIO7 (CAP2)
   GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 3;     // Configure GPIO7 as CAP2
EDIS;
#endif
#endif
}


//=============================================
//
// 2803x平台HDO1
//
//=============================================
void InitEPwm5Gpio(void)
{
#if (DEBUG_F_HDO || DEBUG_F_DO)
#if DSP_2803X
EALLOW;
   GpioCtrlRegs.GPAPUD.bit.GPIO9 = 1;      // Disable pull-up on GPIO9 (ePWM5)
   GpioCtrlRegs.GPAQSEL1.bit.GPIO9 = 0;    // Synch to SYSCLKOUT GPIO9 (ePWM5)
   GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 1;     // 00:GPIO9   01:EPWM5B  10:LINTXA  11:Reserved
   GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;
   
EDIS;
#endif
#endif
}

//=============================================
//
// 2803x平台AO2
//
//=============================================
void InitEPwm4Gpio(void)
{
#if (DEBUG_F_AO)
#if DSP_2803X
   EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO6 = 1;     // Disable pull-up on GPIO6 (ePWM4)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO6 = 0;   // Synch to SYSCLKOUT GPIO6 (ePWM4)
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;    // 00:GPIO6   01:EPWM4A  10:SPIDTED  11:ECAP2
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;
    EDIS;
#endif
#endif
}

//=============================================
//
// 2803x平台AO1
//
//=============================================
void InitEPwm6Gpio(void)
{
#if (DEBUG_F_AO)
#if DSP_2803X
EALLOW;
   GpioCtrlRegs.GPAPUD.bit.GPIO10 = 1;     // Disable pull-up on GPIO10 (ePWM6)
   GpioCtrlRegs.GPAQSEL1.bit.GPIO10 = 0;   // Synch to SYSCLKOUT GPIO10 (ePWM6)
   GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 1;    // 00:GPIO10   01:EPWM6A  10:SPIDTED  11:ECAP2
   GpioCtrlRegs.GPADIR.bit.GPIO10 = 1;
EDIS;
#endif
#endif
}


//=============================================
//
// PULSE IN(2808)
//
//=============================================
void InitECap3Gpio(void)
{
#if DEBUG_F_AO
#if !DSP_2803X
EALLOW;
   GpioCtrlRegs.GPAPUD.bit.GPIO9 = 0;      // Enable pull-up on GPIO9 (CAP3)
   GpioCtrlRegs.GPAQSEL1.bit.GPIO9 = 0;    // Synch to SYSCLKOUT GPIO9 (CAP3)
   GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 3;     // Configure GPIO9 as CAP3
EDIS;
#endif
#endif
}


//=============================================
//
// PULSE IN(2803x)
//
//=============================================
void InitECap1Gpio(void)
{
#if (DEBUG_F_HDI || DEBUG_F_DI)
#if DSP_2803X
   EALLOW;
   GpioCtrlRegs.GPAPUD.bit.GPIO19 = 0;   // Enable pull-up on GPIO19 (CAP1)
   GpioCtrlRegs.GPAQSEL2.bit.GPIO19 = 0; // Synch to SYSCLKOUT GPIO19 (CAP1)
   GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 3;  // Configure GPIO19 as CAP1 
   EDIS;
#endif
#endif
}   

//=============================================
//
// PULSE IN(2808)
//
//=============================================
void InitECap4Gpio(void)
{
#if DEBUG_F_HDI || DEBUG_F_POSITION_CTRL
#if !DSP_2803X
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO11 = 0;   // Enable pull-up on GPIO11 (CAP4)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO11 = 0; // Synch to SYSCLKOUT GPIO11 (CAP4)
    GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 3;  // Configure GPIO11 as CAP4
    EDIS;
#endif
#endif
}   

// 为了减小时间
// 仅在功能码改变时，才转换成bin
LOCALF void Di2Bin(void)
{
#if DEBUG_F_DI
    static Uint16 diLogicOld[2];
    static Uint16 diLogicAiAsDiOld;
    static Uint16 vdiSrcOld;
    static Uint16 vdiFcSetOld;
    
    if ((diLogicOld[0] != funcCode.code.diLogic[0]) ||
        (diLogicOld[1] != funcCode.code.diLogic[1]) ||
        (diLogicAiAsDiOld != funcCode.code.diLogicAiAsDi)
        )
    {
        diLogicOld[0] = funcCode.code.diLogic[0];
        diLogicOld[1] = funcCode.code.diLogic[1];
        diLogicAiAsDiOld = funcCode.code.diLogicAiAsDi;
        
        diLogic = FcDigit2Bin(funcCode.code.diLogic[0]) + 
        ((FcDigit2Bin(funcCode.code.diLogic[1])) << 5) + 
        ((Uint32)(FcDigit2Bin(funcCode.code.diLogicAiAsDi)) << (DI_NUMBER_PHSIC + DI_NUMBER_V));
    }

    if (vdiSrcOld != funcCode.code.vdiSrc)
    {
        vdiSrcOld = funcCode.code.vdiSrc;
        
        vdiSrc = FcDigit2Bin(funcCode.code.vdiSrc);
    }

    if (vdiFcSetOld != funcCode.code.vdiFcSet)
    {
        vdiFcSetOld = funcCode.code.vdiFcSet;
        
        vdiFcSet = FcDigit2Bin(funcCode.code.vdiFcSet);
    }
#endif
}

//=====================================================================
//
// 读I端子状态(2803x平台)
//
//=====================================================================
LOCALF void getDiHwStatus()
{
#if DEBUG_F_DI
#if DSP_2803X
	diHwStatus.all = 0;
    diHwStatus.bit.di1 |= GpioDataRegs.GPBDAT.bit.GPIO42;
    diHwStatus.bit.di2 |= GpioDataRegs.GPBDAT.bit.GPIO43;
    diHwStatus.bit.di3 |= GpioDataRegs.GPADAT.bit.GPIO27;
    diHwStatus.bit.di4 |= GpioDataRegs.GPBDAT.bit.GPIO40;
    diHwStatus.bit.di5 |= GpioDataRegs.GPADAT.bit.GPIO19;
    diHwStatus.bit.di6 |= GpioDataRegs.GPADAT.bit.GPIO25;
    diHwStatus.bit.di7 |= GpioDataRegs.GPBDAT.bit.GPIO44;
    diHwStatus.bit.di8 |= GpioDataRegs.GPBDAT.bit.GPIO35;
    diHwStatus.bit.di9 |= GpioDataRegs.GPBDAT.bit.GPIO36;
    diHwStatus.bit.di10 |= GpioDataRegs.GPBDAT.bit.GPIO38;   
#endif    
#endif
}

//=====================================================================
//
// 设置DO端子状态(2803x平台)
//
//=====================================================================
LOCALF void setDOStatus()
{
#if DEBUG_F_DO
#if DSP_2803X
EALLOW;
    //GpioDataRegs.GPBDAT.bit.GPIO34 = doHwStatus.bit.do1;
    //GpioDataRegs.GPADAT.bit.GPIO24 = doHwStatus.bit.do2;    
	//GpioDataRegs.GPBDAT.bit.GPIO37 = !doHwStatus.bit.fan; 		
    //GpioDataRegs.GPADAT.bit.GPIO26 = !doHwStatus.bit.relay2;  
	//GpioDataRegs.GPBDAT.bit.GPIO41 = !doHwStatus.bit.relay1; 
	
	doHwStatus.bit.do1 ? (GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1): (GpioDataRegs.GPBSET.bit.GPIO34 = 1);
	doHwStatus.bit.do2 ? (GpioDataRegs.GPACLEAR.bit.GPIO24 = 1): (GpioDataRegs.GPASET.bit.GPIO24 = 1);

	doHwStatus.bit.fan ? (GpioDataRegs.GPBCLEAR.bit.GPIO37 = 1): (GpioDataRegs.GPBSET.bit.GPIO37 = 1); //MD380 风扇控制  

	doHwStatus.bit.relay1 ? (GpioDataRegs.GPBCLEAR.bit.GPIO41 = 1): (GpioDataRegs.GPBSET.bit.GPIO41 = 1);
	doHwStatus.bit.relay2 ? (GpioDataRegs.GPACLEAR.bit.GPIO26 = 1): (GpioDataRegs.GPASET.bit.GPIO26 = 1);
EDIS;
#endif
#endif
}

// 为了减小时间
// 仅在功能码改变时，才转换成bin
LOCALF void Do2Bin(void)
{
#if DEBUG_F_DO
    static Uint16 doLogicOld;
    static Uint16 vdoLogicOld;

    if ((doLogicOld != funcCode.code.doLogic) ||
        (vdoLogicOld != funcCode.code.vdoLogic)
        )
    {
        doLogicOld = funcCode.code.doLogic;
        vdoLogicOld = funcCode.code.vdoLogic;
        
        doLogic = FcDigit2Bin(funcCode.code.doLogic) + 
            ((FcDigit2Bin(funcCode.code.vdoLogic)) << 5);
    }
#endif
}








