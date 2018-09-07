//======================================================================
//
// Time-stamp: <2007-12-20 14:01:46  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_funcCode.h"
#include "f_ui.h"               // LED、键盘的头文件
#include "f_io.h"               // IO头文件
#include "f_eeprom.h"           // EEPROM头文件
#include "f_comm.h"             // 串口通讯头文件
#include "f_runSrc.h"           // 命令源头文件
#include "f_frqSrc.h"           // 频率源头文件
#include "f_main.h"


#define RAM_START_ADDRESS 0x008000 // RAM的起始地址
#if F_DEBUG_RAM
#define RAM_SIZE          0x0600   // 分配的RAM(data)空间大小
//#define RAM_SIZE          0x0c00   // 分配的RAM(data)空间大小
//#define RAM_SIZE          0x4000   // 分配的RAM(data)空间大小
#elif 1    // Flash中运行
#define RAM_SIZE          0x1F80   // 分配的RAM(data)空间大小
#endif


typedef struct
{
    Uint16 ms;
    Uint16 s;
    Uint16 min;
    Uint16 hour;
}TIME;

#if F_DEBUG_RAM
//int32 GlobalQ = GLOBAL_Q;
#endif

// 与性能交互的变量
#if !F_DEBUG_RAM
int16 gSendToMotor05MsDataBuff[FUNC_TO_MOTOR_05MS_DATA_NUM];
int16 gSendToMotor2MsDataBuff[FUNC_TO_MOTOR_2MS_DATA_NUM + FUNC_TO_CORE_DEBUG_DATA_NUM];
#endif
int16 gSendToFunctionDataBuff[MOTOR_TO_Func_2MS_DATA_NUM + CORE_TO_FUNC_DISP_DATA_NUM];
int16 gParaIdToFunctionDataBuff[TUNE_DATA_NUM];

int16 gRealTimeToFunctionDataBuff[1];     // 性能==>功能的参数，实时

LOCALF TIME time;
LOCALF Uint16 timerTicker;
LOCALF Uint16 bTimerHalfMs;
LOCALF Uint16 timerTickerHalfMsOld;
LOCALF Uint16 bTimer1ms;
LOCALF Uint16 timerTicker1msOld;
LOCALF Uint16 bTimer2ms;
LOCALF Uint16 timerTicker2msOld;
//LOCALF Uint16 bTimer4ms;
//LOCALF Uint16 timerTicker4msOld;

extern void InitECap1Gpio(void);
extern void InitECap2Gpio(void);
extern void InitSciaGpio(void);
extern void InitSetAdc(void);
extern interrupt void SCI_RXD_isr(void);
extern interrupt void SCI_TXD_isr(void);
LOCALD void TimerDeal(void);
void ramInit(void);
LOCALD void dspInit(void);
LOCALD void functionPeripheralInit(void);
#define EEPROM_INDEX_USE_LENGTH     994     // 最后一个eeprom地址+1
extern Uint16 sizeOfTable_eeprom2Fc;
void main(void)
{
    Uint32 baseTime;
    static Uint16 ticker;
    dspInit();
	sizeOfTable_eeprom2Fc = EEPROM_INDEX_USE_LENGTH;
    InitForFunctionApp();

    baseTime = GetTime();

//    StartCpuTimer0();

    for (;;)
    {
        Main0msFunction();

        if(baseTime - GetTime() >= TIME_100US)
        {
            static Uint16 k;
            //baseTime = GetTime();
            baseTime -= TIME_100US;

            TimerDeal();

            if (++k >= 1)   // 
            {
                k = 0;
//                Main0msFunction();
            }
        }

        if (bTimerHalfMs)
        {
            bTimerHalfMs = 0;
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
            ticker++;
            if (ticker >= 4)
            {
                ticker = 0;
            }

        }

        if (bTimer1ms)
        {
            bTimer1ms = 0;
        }

        if (bTimer2ms)
        {
            bTimer2ms = 0;
            Main2msFunction();
        }
    } // for(;;)
}


//=====================================================================
//
// 时间/软件定时器处理
//
//=====================================================================
LOCALF void TimerDeal()
{
    timerTicker++;

// 软件定时器
    if ((timerTicker - timerTicker1msOld) >= 1 * 10) // 1ms
    {
        timerTicker1msOld = timerTicker;
        bTimer1ms = 1;

        if (++time.ms >= 1000)    // 计时
        {
            time.ms = 0;
            if (++time.s >= 60)
            {
                time.s = 0;
                if (++time.min >= 60)
                {
                    time.min = 0;
                    time.hour++;
                }
            }
        }
    }

// 软件定时器
    if ((timerTicker - timerTicker2msOld) >= 2 * 10) // 2ms
    {
        timerTicker2msOld = timerTicker;
        bTimer2ms = 1;
    }

    if ((timerTicker - timerTickerHalfMsOld) >= 5) // 0.5ms
    {
        timerTickerHalfMsOld = timerTicker;
        bTimerHalfMs = 1;
    }

#if F_DEBUG_RAM   // 仅调试功能，在CCS的build option中定义的宏
    AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;   // Reset SEQ1
    AdcRegs.ADCTRL2.bit.SOC_SEQ1 = 1;   // sofware start ADC
#endif
}


//=====================================================================
//
// DSP init
//
//=====================================================================
LOCALF void dspInit()
{
// Step 1. Initialize System Control:
// PLL, WatchDog, enable Peripheral Clocks
// This example function is found in the DSP280x_SysCtrl.c file.
    InitSysCtrl();

// Step 2. Initalize GPIO:
// This example function is found in the DSP280x_Gpio.c file and
// illustrates how to set the GPIO to it's default state.
// InitGpio();  // Skipped for this example
// Setup only the GP I/O only for SPI-A functionality
// This function is found in DSP280x_Spi.c
#if 0
    InitSpiaGpio();
    InitI2CGpio();
    InitSciaGpio();
#endif

// Step 3. Clear all interrupts and initialize PIE vector table:
// Disable CPU interrupts
    DINT;

// Initialize PIE control registers to their default state.
// The default state is all PIE interrupts disabled and flags
// are cleared.
// This function is found in the DSP280x_PieCtrl.c file.
    InitPieCtrl();

// Disable CPU interrupts and clear all CPU interrupt flags:
    IER = 0x0000;
    IFR = 0x0000;

// Initialize the PIE vector table with pointers to the shell Interrupt
// Service Routines (ISR).
// This will populate the entire table, even if the interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in DSP280x_DefaultIsr.c.
// This function is found in DSP280x_PieVect.c.
//    InitPieVectTable();
// Enable the PIE Vector Table
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;

// Interrupts that are used in this example are re-mapped to
// ISR functions found within this file.
#if 0
    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.TINT0 = cpu_timer0_isr;
    EDIS;    // This is needed to disable write to EALLOW protected registers
#endif

// Step 4. Initialize all the Device Peripherals:
// This function is found in DSP280x_InitPeripherals.c
// InitPeripherals(); // Not required for this example

    functionPeripheralInit();

    InitCpuTimers();

// Configure CPU-Timer 0 to interrupt every second:
// 100MHz CPU Freq, 1 second Period (in uSeconds)
//   ConfigCpuTimer(&CpuTimer0, 100, 2000);   // 粗略测试，5.5ms刷新一次显示，就有闪烁感。
// 粗略测试，5.5ms刷新一次显示，就有闪烁感。
#if 0
#if (DSP_CLOCK == 100)
    ConfigCpuTimer(&CpuTimer0, 100.0, 100.0); // 100us
#elif (DSP_CLOCK == 60)
    ConfigCpuTimer(&CpuTimer0, 60.0, 6.0);// 100us
//    ConfigCpuTimer(&CpuTimer0, 60.0, 1000000.0);
#endif
//    StartCpuTimer0();
#endif

    StartCpuTimer1();                    // 作为主程序的时间基准

// Step 5. User specific code:
// Copy time critical code and Flash setup code to RAM
// This includes the following ISR functions: InitFlash();
// The  RamfuncsLoadStart, RamfuncsLoadEnd, and RamfuncsRunStart
// symbols are created by the linker. Refer to the cmd file.
   MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);

// Call Flash Initialization to setup flash waitstates
// This function must reside in RAM
   InitFlash();

// Enable CPU INT1 which is connected to CPU-Timer 0:
//    IER |= M_INT1;

// Enable TINT0 in the PIE: Group 1 interrupt 7
//    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

// 通讯控制使用中断，初始化
#if 1
    EALLOW;
    PieVectTable.SCIRXINTA 		= SCI_RXD_isr;
	PieVectTable.SCITXINTA 		= SCI_TXD_isr;
    EDIS;
	IER |= M_INT9;   	            //  Enable interrupts:
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;
	PieCtrlRegs.PIEIER9.bit.INTx2 = 1;
#endif

//++====
//    InitDa();

// Enable global Interrupts and higher priority real-time debug events:
    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global realtime interrupt DBGM
}



//=====================================================================
//
// RAM数据全部清零
//
//=====================================================================
#if 0
void ramInit()
{
    Uint16 *p = (Uint16 *)RAM_START_ADDRESS;

    for (; p < (Uint16 *)(RAM_START_ADDRESS + RAM_SIZE); p++) // 初始化RAM，全部清零
        *p = 0;
}
#else
void ramInit()                  // RAM数据全部清零
{
    int16 i;
    Uint16 *p = (Uint16 *)RAM_START_ADDRESS;

    for (i = 0; i < RAM_SIZE; i++) // 初始化RAM，全部清零
        *(p++) = 0;
}
#endif


//=====================================================================
//
// 把程序从flash拷贝到ram中
//
//=====================================================================
//void MemCopy(const Uint16 *SourceAddr, const Uint16* SourceEndAddr, Uint16* DestAddr)
void MemCopy(Uint16 *SourceAddr, Uint16* SourceEndAddr, Uint16* DestAddr)
{
    while(SourceAddr < SourceEndAddr)
    {
       *DestAddr++ = *SourceAddr++;
    }
    return;
}

//=====================================================================
//
// 功能部分的外设初始化函数
//
//=====================================================================
LOCALF void functionPeripheralInit()
{
    InitSetAdc();
}


#if 0
interrupt void cpu_timer0_isr(void)
{    
//    DaDeal();

    CPUPerformanceBase++;
    
    // Acknowledge this interrupt to receive more interrupts from group 1
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
#endif




#if 1//F_DEBUG_RAM        // 仅调试功能，在CCS的build option中定义的宏
void InitSetAdc(void)
{
#if DEBUG_F_AI
    int16 waite;

    AdcRegs.ADCTRL3.all = 0x00E0;  // Power up bandgap/reference/ADC circuits

    for (waite = 0;waite < 5000; waite++)
    {
        ;
    }

    //以下开始设置ADC的控制寄存器、转换通道选择寄存器等
    AdcRegs.ADCTRL3.bit.ADCCLKPS = 1;
    AdcRegs.ADCTRL1.bit.CPS = 1;            // 设置ADC时钟为12.5MHz = 50MHz/2/2 100
                                            // 设置ADC时钟为7.5MHz = 30MHz/2/2  60

    AdcRegs.ADCTRL1.bit.ACQ_PS = 1;         // 采样时间 10*ADCCLK
    AdcRegs.ADCTRL1.bit.CONT_RUN = 0;       // 设置为启停模式
    AdcRegs.ADCTRL1.bit.SEQ_CASC = 1;       // 级联模式
    AdcRegs.ADCTRL3.bit.SMODE_SEL = 1;      // 设置为同步采样模式
    AdcRegs.ADCTRL2.bit.EPWM_SOCA_SEQ1 = 1; // 允许SOCA起动ADC A
    AdcRegs.ADCMAXCONV.all = 0x3;           // 最大转换8*2通道

    AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0;    // ADCINA0 & ADCINB0
    AdcRegs.ADCCHSELSEQ1.bit.CONV01 = 1;    // ADCINA1 & ADCINB1
    AdcRegs.ADCCHSELSEQ1.bit.CONV02 = 2;    // ADCINA2 & ADCINB2
    AdcRegs.ADCCHSELSEQ1.bit.CONV03 = 3;    // ADCINA3 & ADCINB3

    AdcRegs.ADCCHSELSEQ2.bit.CONV04 = 4;    // ADCINA4 & ADCINB4
    AdcRegs.ADCCHSELSEQ2.bit.CONV05 = 5;    // ADCINA5 & ADCINB5
    AdcRegs.ADCCHSELSEQ2.bit.CONV06 = 6;    // ADCINA6 & ADCINB6
    AdcRegs.ADCCHSELSEQ2.bit.CONV07 = 7;    // ADCINA7 & ADCINB7
#endif
}
#endif  // #if F_DEBUG_RAM




#if 1//F_DEBUG_RAM
//=====================================================================
//
// 开方函数
//
//=====================================================================
Uint16 qsqrt(Uint32 data)
{
	Uint32 b = 0;
	Uint32 tmp;

    if (data)
    {
        tmp = data / 2;
		for (;;)
		{
			b = data / tmp;
			if (ABS_INT32((int32)(tmp - b)) > 1)
                tmp = (tmp + b) / 2;
			else
                break;
		}
    }
    
    return b;
}
#endif


#if 0
Uint32 sqrt64(const Uint64 x)
{
    int16 i;
    Uint64 rc = 0;
    Uint64 t;

    for (i = 31; i >= 0; i--)
    {
        t = rc + (1UL << i);
        if ((Uint64)t * t <= x)
            rc = t;
    }

    return rc;
}
#endif



//===========================================================================
// No more.
//===========================================================================


