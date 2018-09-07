/***************************************************************************
 该文件提供对所有需要使用的外设初始化操作，主要包括：
1、系统时钟初始化
2、搬移部分程序到RAM中运行
3、FLASH初始化
4、为需要使用的外设提供时钟
5、所有GPIO口应用情况初始化
6、初始化定时器
****************************************************************************
注意:

  此处只包含性能部分的初始化,功能模块的初始化在性能初始化完成后执行，调试时
  首先要判断此处的初始化结果，有没有被功能的初始化意外修改
***************************************************************************/
#include "MotorDefine.h"
#include "MotorInclude.h"

#define Device_cal (void   (*)(void))0x3D7C80

/**修改FLASH寄存器的程序需要搬移到FLASH中执行**/
#pragma CODE_SECTION(InitFlash, "ramfuncs");
extern COPY_TABLE prginRAM;

void 	copy_prg(COPY_TABLE *tp);
void   	DisableDog(void);   
void 	InitPll(Uint val);
void	InitFlash(void);
void	InitPeripheralClocks(void);

void 	InitPieCtrl(void);
void 	InitPieVectTable(void);

void 	InitSetGpio(void);
extern interrupt void SCI_RXD_isr(void);
extern interrupt void SCI_TXD_isr(void);

/************************************************************
初始化DSP系统，包括初始化时钟和配置寄存器
************************************************************/
void InitSysCtrl()
{   
    DisableDog();			// Disable the watchdog

    // *IMPORTANT*
    // The Device_cal function, which copies the ADC & oscillator calibration values
    // from TI reserved OTP into the appropriate trim registers, occurs automatically
    // in the Boot ROM. If the boot ROM code is bypassed during the debug process, the
    // following function MUST be called for the ADC and oscillators to function according
    // to specification. The clocks to the ADC MUST be enabled before calling this
    // function.
    // See the device data manual and/or the ADC Reference
    // Manual for more information.
        EALLOW;
        //SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 1; // Enable ADC peripheral clock
        //(*Device_cal)();
        //SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 0; // Return ADC clock to original state
        EDIS;

    // Select Internal Oscillator 1 as Clock Source (default), and turn off all unused clocks to
    // conserve power.
    // Initialize the PLL control: PLLCR and CLKINDIV
    // DSP28_PLLCR and DSP28_CLKINDIV are defined in DSP2803x_Examples.h
    InitPll(DSP_CLOCK/10);	// Change 20MHz clock to 100MHz or 60MHz
   
    copy_prg(&prginRAM);		// Move the program from FLASH to RAM
    InitFlash();				// Initializes the Flash Control registers

    InitPeripheralClocks();	// Initialize the peripheral clocks
}

/************************************************************
初始化中断服务程序
************************************************************/
void InitInterrupt()
{
   DINT;							//Clear all interrupts and initialize PIE vector table:
   InitPieCtrl();  					//Disable PIE
   IER = 0x0000;
   IFR = 0x0000;

   InitPieVectTable();				//Enable PIE

   EALLOW;  						//设置用户服务程序
   //PieVectTable.TINT0 		= &cpu_timer0_isr;		//定时中断	
   PieVectTable.ADCINT1     = &ADC_Over_isr;		//ADC结束中断
   PieVectTable.EPWM1_TZINT = &EPWM1_TZ_isr;		//过流中断
   PieVectTable.EPWM1_INT 	= &EPWM1_zero_isr;		//下溢中断   
   PieVectTable.SCIRXINTA 	= &SCI_RXD_isr; 		//通讯接收中断
   PieVectTable.SCITXINTA 	= &SCI_TXD_isr; 		//通讯发送中断

   EDIS;    	
}

/************************************************************
开启已经使用的中断
************************************************************/
void SetInterruptEnable()
{
   IER |= (M_INT1 | M_INT2 | M_INT3 | M_INT5);	//  Enable interrupts:
   PieCtrlRegs.PIEIER1.bit.INTx1 = 1;           //  ADC1INT
   PieCtrlRegs.PIEIER2.bit.INTx1 = 1;           // 	EPWM1_TZ
   PieCtrlRegs.PIEIER3.bit.INTx1 = 1;           // 	EPWM1_INT
}

/************************************************************
初始化DSP的所有外设，为电机控制和接口做准备
************************************************************/
void InitPeripherals(void)
{
   	InitSetGpio();  
   	InitSetPWM();
   	InitSetAdc();

   	InitCpuTimers();
   	//ConfigCpuTimer(&CpuTimer0, DSP_CLOCK, 1000000L);//100MHz CPU, 1 millisecond Period
   	//StartCpuTimer0();					 //暂时没用的定时器中断
   	StartCpuTimer1();				     //作为主程序的时间基准
}

//---------------------------------------------------------------------------
// Move program from FLASH to RAM size < 65535 words
//---------------------------------------------------------------------------
void copy_prg(COPY_TABLE *tp)
{
	Uint size;
	COPY_RECORD *crp = &tp->recs[0];

	size = (Uint)crp->size - 1;

	MovePrgFrFlashToRam(crp->src_addr, crp->dst_addr, size);
}

//---------------------------------------------------------------------------
// This function initializes the Flash Control registers
//                   CAUTION 
// This function MUST be executed out of RAM. Executing it
// out of OTP/Flash will yield unpredictable results
//---------------------------------------------------------------------------
void InitFlash(void)
{
   EALLOW;
   //Enable Flash Pipeline mode to improve performance of code executed from Flash.
   //FlashRegs.FOPT.bit.ENPIPE = 1;
   FlashRegs.FOPT.all = 0x01;
   //Set the Random Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.RANDWAIT = READ_RAND_FLASH_WAITE;
   //Set the Paged Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.PAGEWAIT = READ_PAGE_FLASH_WAITE;
   //Set the Waitstate for the OTP
   FlashRegs.FOTPWAIT.bit.OTPWAIT = READ_OTP_WAITE;
   
   //Set number of cycles to transition from sleep to standby
   //FlashRegs.FSTDBYWAIT.bit.STDBYWAIT = 0x01FF;       
   //Set number of cycles to transition from standby to active
   //FlashRegs.FACTIVEWAIT.bit.ACTIVEWAIT = 0x01FF;   
   EDIS;
   asm(" RPT #7 || NOP");	
}	

//---------------------------------------------------------------------------
// This function resets the watchdog timer.
//---------------------------------------------------------------------------
void KickDog(void)
{
    EALLOW;
    SysCtrlRegs.WDKEY = 0x0055;
    SysCtrlRegs.WDKEY = 0x00AA;
    EDIS;
}

//---------------------------------------------------------------------------
// This function disables the watchdog timer.
//---------------------------------------------------------------------------
void DisableDog(void)
{
    EALLOW;
    SysCtrlRegs.WDCR= 0x0068;
    EDIS;
}

//---------------------------------------------------------------------------
// This function Enables the watchdog timer.
//---------------------------------------------------------------------------
void EnableDog(void)
{
    EALLOW;
    SysCtrlRegs.WDCR= 0x002A;
    EDIS;
}

//---------------------------------------------------------------------------
// This function initializes the PLLCR register.
//---------------------------------------------------------------------------
void InitPll(Uint16 val)
{
    // select Osc source: external crystal-ocs, and turn off other osc-source
    EALLOW;
    SysCtrlRegs.CLKCTL.bit.XTALOSCOFF = 0;     // Turn on XTALOSC
    SysCtrlRegs.CLKCTL.bit.XCLKINOFF = 1;      // Turn off XCLKIN
    SysCtrlRegs.CLKCTL.bit.OSCCLKSRC2SEL = 0;  // Switch to external clock
    SysCtrlRegs.CLKCTL.bit.OSCCLKSRCSEL = 1;   // Switch from INTOSC1 to INTOSC2/ext clk
    SysCtrlRegs.CLKCTL.bit.WDCLKSRCSEL = 1;    // Switch Watchdog Clk Src to external clock
    SysCtrlRegs.CLKCTL.bit.INTOSC2OFF = 1;     // Turn off INTOSC2
    SysCtrlRegs.CLKCTL.bit.INTOSC1OFF = 1;     // Turn off INTOSC1
    EDIS;

    // configure the PLL
    if (SysCtrlRegs.PLLSTS.bit.MCLKSTS != 1)	// Make sure the PLL is not running in limp mode
    {
        // DIVSEL MUST be 0 before PLLCR can be changed from
        // 0x0000. It is set to 0 by an external reset XRSn
        // This puts us in 1/4
        if (SysCtrlRegs.PLLSTS.bit.DIVSEL != 0)
        {
            EALLOW;
            SysCtrlRegs.PLLSTS.bit.DIVSEL = 0;
            EDIS;
        }

        EALLOW;
        // Before setting PLLCR turn off missing clock detect logic
        SysCtrlRegs.PLLSTS.bit.MCLKOFF = 1;
        SysCtrlRegs.PLLCR.bit.DIV = val;
        EDIS;

        // Optional: Wait for PLL to lock.
        // During this time the CPU will switch to OSCCLK/2 until
        // the PLL is stable.  Once the PLL is stable the CPU will
        // switch to the new PLL value.
        //
        // This time-to-lock is monitored by a PLL lock counter.
        //
        // Code is not required to sit and wait for the PLL to lock.
        // However, if the code does anything that is timing critical,
        // and requires the correct clock be locked, then it is best to
        // wait until this switching has completed.

        // Wait for the PLL lock bit to be set.

        // The watchdog should be disabled before this loop, or fed within
        // the loop via ServiceDog().

        // Uncomment to disable the watchdog
        DisableDog();

        while(SysCtrlRegs.PLLSTS.bit.PLLLOCKS != 1)
        {
          // Uncomment to service the watchdog
          // ServiceDog();
        }

        EALLOW;
        SysCtrlRegs.PLLSTS.bit.MCLKOFF = 0;     // turn off missing clock detect
        EDIS;

        EALLOW;
        SysCtrlRegs.PLLSTS.bit.DIVSEL = 2;      // configure PLLSTS.DIVSEL = 2  (default)
        EDIS;
    }
}

//--------------------------------------------------------------------------
// This function initializes the clocks to the peripheral modules.
//---------------------------------------------------------------------------
void InitPeripheralClocks(void)
{
   EALLOW;

   //SysCtrlRegs.HISPCP.all = 0x0001;	//50MHz for ADC / 30MHz
   SysCtrlRegs.LOSPCP.all = 0x0002;	//25MHz for SCI and SPI / 15MHz

   //SysCtrlRegs.XCLK.bit.XCLKOUTDIV = 2;	//Set ClockOut Pin = 100MHz/60MHZ
   SysCtrlRegs.XCLK.all = 0x02;

   //SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 1;    // ADC
   //SysCtrlRegs.PCLKCR0.bit.I2CAENCLK = 1;   // I2C

   //SysCtrlRegs.PCLKCR0.bit.SPIAENCLK = 1;     // SPI-A
   //SysCtrlRegs.PCLKCR0.bit.SPIBENCLK = 0;     // SPI-B
   //SysCtrlRegs.PCLKCR0.bit.SPICENCLK = 0;     // SPI-C
   //SysCtrlRegs.PCLKCR0.bit.SPIDENCLK = 0;     // SPI-D

   //SysCtrlRegs.PCLKCR0.bit.SCIAENCLK = 1;     // SCI-A
   //SysCtrlRegs.PCLKCR0.bit.SCIBENCLK = 0;     // SCI-B

   //SysCtrlRegs.PCLKCR0.bit.ECANAENCLK = 0;    // eCAN-A
   //SysCtrlRegs.PCLKCR0.bit.ECANBENCLK = 0;    // eCAN-B   

   //SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;   // Enable TBCLK within the ePWM

   SysCtrlRegs.PCLKCR0.all = 0x051C;

   //SysCtrlRegs.PCLKCR1.bit.ECAP1ENCLK = 1;  // eCAP1
   //SysCtrlRegs.PCLKCR1.bit.ECAP2ENCLK = 1;  // eCAP2
   //SysCtrlRegs.PCLKCR1.bit.ECAP3ENCLK = 0;  // eCAP3
   //SysCtrlRegs.PCLKCR1.bit.ECAP4ENCLK = 0;  // eCAP4
   
   //SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK = 1;  // ePWM1
   //SysCtrlRegs.PCLKCR1.bit.EPWM2ENCLK = 1;  // ePWM2
   //SysCtrlRegs.PCLKCR1.bit.EPWM3ENCLK = 1;  // ePWM3
   //SysCtrlRegs.PCLKCR1.bit.EPWM4ENCLK = 0;  // ePWM4
   //SysCtrlRegs.PCLKCR1.bit.EPWM5ENCLK = 0;  // ePWM5
   //SysCtrlRegs.PCLKCR1.bit.EPWM6ENCLK = 0;  // ePWM6
  
   //SysCtrlRegs.PCLKCR1.bit.EQEP1ENCLK = 1;  // eQEP1
   //SysCtrlRegs.PCLKCR1.bit.EQEP2ENCLK = 0;  // eQEP2

   SysCtrlRegs.PCLKCR1.all = 0x4307;

   EDIS;
}

//--------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
void InitPieCtrl(void)
{
	Uint * m_Point1,* m_Point2;
	Uint   m_Index;

    DINT;    
    
    PieCtrlRegs.PIECTRL.bit.ENPIE = 0;	// Disable the PIE
	// Clear all PIEIER registers:
	m_Point1 = (Uint *)&PieCtrlRegs.PIEIER1.all;
	m_Point2 = (Uint *)&PieCtrlRegs.PIEIFR1.all;
	for(m_Index = 0;m_Index<12;m_Index++)
	{
		*(m_Point1++) = 0;
		*(m_Point2++) = 0;
	}
}	

/************************************************************
所有误操作的中断处理
************************************************************/
int gErrorIntCnt;
interrupt void rsvd_ISR(void)      
{
	gErrorIntCnt++;				//监控是否有错误进入中断的情况
}

//--------------------------------------------------------------------------
//初始化的时候首先把所有的中断服务程序 都初始化为默认服务程序
//---------------------------------------------------------------------------
void InitPieVectTable(void)
{
	int16	i;
	PINT *pPieTable = (void *) &PieVectTable;
		
	EALLOW;	
	for(i=0; i < 128; i++)
	{
		*pPieTable++ = rsvd_ISR;	
	}
	EDIS;

	// Enable the PIE Vector Table
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;	
			
}

//---------------------------------------------------------------------------
// Set GPIO 口的复用、上拉下拉、同步
//---------------------------------------------------------------------------
void InitSetGpio(void)
{
   EALLOW;
   //GPIO口上拉选择
   //GpioCtrlRegs.GPAPUD.all = 0x00000000L;	// 与功能相关的8个管脚需要去掉上拉，否则会引起低电平错误。由功能自己完成
   GpioCtrlRegs.GPAPUD.all = 0x00000640;	// 功能需要的io口不要改变
   GpioCtrlRegs.GPBPUD.all = 0x0;			// 其它管脚都上拉

   //GPIO口功能选择：设置使用的外设
   GpioCtrlRegs.GPAMUX1.all = 0x05000555L;
   //GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;   // Configure GPIO0 as EPWM1A
   //GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;   // Configure GPIO1 as EPWM1B
   //GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;   // Configure GPIO2 as EPWM2A
   //GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;   // Configure GPIO3 as EPWM2B
   //GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;   // Configure GPIO4 as EPWM3A
   //GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;   // Configure GPIO5 as EPWM3B
   //GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 1;  // Configure GPIO12 as TZ1(OC)
   //GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 1;  // Configure GPIO13 as TZ2
   GpioCtrlRegs.GPAMUX2.all = 0x00004500L;
   //GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 1;    //eQEP1A
   //GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 1;    //eQEP1B
   //GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 1;    //eQEP1I

   //GPIO数据先初始化为需要的值
   GpioDataRegs.GPADAT.all = 0x00000980L;
   //GpioDataRegs.GPADAT.bit.GPIO7  = 1;        
   //GpioDataRegs.GPADAT.bit.GPIO8  = 1;        
   //GpioDataRegs.GPADAT.bit.GPIO11 = 1;        
   GpioDataRegs.GPBDAT.all = 0x00000000L;
   
   //GPIO口方向选择1：复位为输入
   GpioCtrlRegs.GPADIR.all = 0x00000980L;
   //GpioCtrlRegs.GPADIR.bit.GPIO7  = 1;        //DRIVE(PWM输出允许信号)
   //GpioCtrlRegs.GPADIR.bit.GPIO8  = 1;        //制动电阻控制
   //GpioCtrlRegs.GPADIR.bit.GPIO11 = 1;        //继电器控制	
   GpioCtrlRegs.GPBDIR.all = 0x00;
   //GPIO口同步选择
   GpioCtrlRegs.GPAQSEL1.all = 0x57000000L;    	// GPIO0-GPIO15 Synch to SYSCLKOUT 
   //GpioCtrlRegs.GPAQSEL1.bit.GPIO7 = 0;     	// Synch to SYSCLKOUT GPIO7 (CAP2)
   //GpioCtrlRegs.GPAQSEL1.bit.GPIO12 = 1;     	// Synch to SYSCLKOUT GPIO7 (CAP2)
   //GpioCtrlRegs.GPAQSEL1.bit.GPIO13 = 1;     	// Synch to SYSCLKOUT GPIO7 (CAP2)
   //GpioCtrlRegs.GPAQSEL1.bit.GPIO14 = 1;     	// Synch to SYSCLKOUT GPIO7 (CAP2)
   //GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 1;     	// Synch to SYSCLKOUT GPIO7 (CAP2)
   GpioCtrlRegs.GPAQSEL2.all = 0x0300803FL;    	// GPIO16-GPIO31 Synch to SYSCLKOUT
   //GpioCtrlRegs.GPAQSEL2.bit.GPIO16 = 3; 		// Asynch input GPIO16 (SPISIMOA)
   //GpioCtrlRegs.GPAQSEL2.bit.GPIO17 = 3; 		// Asynch input GPIO17 (SPISOMIA)
   //GpioCtrlRegs.GPAQSEL2.bit.GPIO18 = 3; 		// Asynch input GPIO18 (SPICLKA)
   //GpioCtrlRegs.GPAQSEL2.bit.GPIO24 = 0;    	// Synch to SYSCLKOUT GPIO24 (CAP1)
   //GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  	// Asynch input GPIO28 (SCIRXDA)
   //GpioCtrlRegs.GPAQSEL2.bit.GPIO23 = 2;      // Z-interrupt
   GpioCtrlRegs.GPBQSEL1.all = 0x0000000FL;    	// GPIO32-GPIO34 Synch to SYSCLKOUT 
   //GpioCtrlRegs.GPBQSEL1.bit.GPIO32 = 3;  	// Asynch input GPIO32 (SDAA)
   //GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 3;  	// Asynch input GPIO33 (SCLA)

   GpioCtrlRegs.AIOMUX1.bit.AIO2 = 0;       // GPAIO2
   GpioCtrlRegs.AIODIR.bit.AIO2 = 0;        // input

    EDIS;
}	

/************************************************************
函数输入:无
函数输出:无
调用位置:主循环之前
调用条件:无
函数功能:初始化DSP的AD采样模块
************************************************************/
void InitSetAdc(void)
{
    int waite;
   
    EALLOW;
        SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 1;
        (*Device_cal)();  
    EDIS;
    
	//============================================================================================
    //复位
    for(waite = 0;waite<5000;waite++){}      //延时1ms   
    //------------------------------------------------------------------------------------------------
    AdcRegs.ADCCTL1.bit.RESET     = 0x1;     // 复位ADC模块
    asm(" RPT #28 || NOP");                  // 需延时两个ADCCLK；延时28个空周期  
    for(waite = 0;waite<5000;waite++){}      // 延时1ms        

    EALLOW;
        AdcRegs.ADCCTL1.bit.ADCBGPWD  = 1;      // Power ADC BG
        AdcRegs.ADCCTL1.bit.ADCREFPWD = 1;      // Power reference
        AdcRegs.ADCCTL1.bit.ADCPWDN   = 1;      // Power ADC
        AdcRegs.ADCCTL1.bit.ADCENABLE = 1;      // Enable ADC
        AdcRegs.ADCCTL1.bit.ADCREFSEL = 0;      // Select interal BG
	EDIS;
    
    for(waite = 0;waite<25000;waite++){}        // 延时5ms
    
	//以下开始设置ADC的控制寄存器、转换通道选择寄存器等
    EALLOW;    
	AdcRegs.ADCOFFTRIM.all = 0;
    AdcRegs.INTSEL1N2.all = 0x002C;     //通道12转换完成产生中断ADCINT1
    AdcRegs.ADCSAMPLEMODE.all = 0;          // 采样模式为0: 顺序采样(非同时采样)
	AdcRegs.ADCCTL1.bit.INTPULSEPOS	= 1;	//ADCINT1 trips after AdcResults latch

    //********************************
    AdcRegs.ADCSOC0CTL.all  = 0x2BC8;    //ePWM1 ADCSOCA触发，转换ADCINB7-GND，采样间隔8个周期
    AdcRegs.ADCSOC1CTL.all  = 0x2808;    //ADCINA0-IU
    AdcRegs.ADCSOC2CTL.all  = 0x28C8;    //ADCINA3-IW
    AdcRegs.ADCSOC3CTL.all  = 0x2948;    //ADCINA5-UDC
    AdcRegs.ADCSOC4CTL.all  = 0x2A48;    //ADCINB1-TEMP
    AdcRegs.ADCSOC5CTL.all  = 0x2A08;    //ADCINB0-IV
    AdcRegs.ADCSOC6CTL.all  = 0x2B48;    //ADCINB5-AI1
    AdcRegs.ADCSOC7CTL.all  = 0x29C8;    //ADCINA7-AI2
    AdcRegs.ADCSOC8CTL.all  = 0x2848;    //ADCINA1-AI3
    AdcRegs.ADCSOC9CTL.all  = 0x2988;    //ADCINA6-UU4      //uvw-U
    AdcRegs.ADCSOC10CTL.all = 0x2A88;    //ADCINB2-UU5      //uvw-V
    AdcRegs.ADCSOC11CTL.all = 0x2B08;    //ADCINB4-UU6      //uvw-W
    AdcRegs.ADCSOC12CTL.all = 0x2B88;    //ADCINB6-UU7 
    AdcRegs.ADCSOC13CTL.all = 0x2AC8;    //ADCINB3-PL   //用数字端口实现

	ADC_CLEAR_INT_FLAG;   //ADC 中断复位, 必须的.
    EDIS;    
}	

//===========================================================================
// End of file.
//===========================================================================
