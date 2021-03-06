/************************************************************
文件功能:电机和变频器保护程序
文件版本： 
最新更新： 

*************************************************************/
#include "DSP2803x_EPwm_defines.h"
#include "MotorInvProtectInclude.h"
// // 全局变量定义
OVER_LOAD_PROTECT		gOverLoad;
PHASE_LOSE_STRUCT		gPhaseLose;
INPUT_LOSE_STRUCT		gInLose;
LOAD_LOSE_STRUCT		gLoadLose;
FAN_CTRL_STRUCT			gFanCtrl;
Ulong					gBuffResCnt;	//缓冲电阻保护变量
CBC_PROTECT_STRUCT		gCBCProtect;
struct FAULT_CODE_INFOR_STRUCT_DEF  gError;

//变频器和电机的过载表
Uint const gInvOverLoadTable[9] =      /*表长度9；表步长9%；表最小值:115%；表最大值:187%*/
{
		36000,				//115%变频器电流   		1小时过载  
		18000,				//124%变频器电流	  	30分钟过载
        6000,				//133%变频器电流	  	10分钟过载
        1800,				//142%变频器电流	  	3分钟过载 
        600,				//151%变频器电流   		1分钟过载  
        200,				//160%变频器电流   		20秒过载   
        120,				//169%变频器电流   		12秒过载    
        20,					//178%变频器电流   		6秒过载    改为178% 2S过载
        20,					//187%变频器电流   		2秒过载    
};
Uint const gInvOverLoadTableForP[9] =       /*表长度9；表步长4%；表最小值:105%；表最大值:137%*/
{
		36000,				//105%变频器电流   		1小时过载  
		15000,				//109%变频器电流	  	25分钟过载
        6000,				//113%变频器电流	  	10分钟过载
        1800,				//117%变频器电流	  	3分钟过载 
        600,				//121%变频器电流   		1分钟过载  
        300,				//125%变频器电流   		30秒过载   
        100,				//129%变频器电流   		10秒过载    
        30,					//133%变频器电流   		3秒过载    
        10,					//137%变频器电流   		1秒过载    
};

//变频器过载累计量的消除系数
Ulong const gInvOverLoadDecTable[12] =
{
        (65536L*60/7),      //0%变频器电流    0.7分钟消除过载
        (65536L*60/8),		//10%变频器电流   0.8分钟消除过载
        (65536L*60/9),		//20%变频器电流   0.9分钟消除过载
        (65536L*60/10),		//30%变频器电流   1.0分钟消除过载
        (65536L*60/11),		//40%变频器电流   1.1分钟消除过载
        (65536L*60/13),		//50%变频器电流   1.3分钟消除过载
        (65536L*60/16),		//60%变频器电流   1.6分钟消除过载
        (65536L*60/19),		//70%变频器电流   1.9分钟消除过载
        (65536L*60/24),		//80%变频器电流   2.4分钟消除过载
        (65536L*60/34),		//90%变频器电流   3.4分钟消除过载
		(65536L*60/56),		//100%变频器电流  5.6分钟消除过载
};

#define C_MOTOR_OV_TAB_NUM      7
Uint const gMotorOverLoadTable[C_MOTOR_OV_TAB_NUM] =
{
		48000,				//115%电机电流  1小时20分钟过载
		24000,				//125%电机电流  40分钟过载
		9000,				//135%电机电流  15分钟过载 
		3000,				//145%电机电流  5分钟过载
		1200,				//155%电机电流  2分钟过载
		1200,				//165%电机电流  2分钟过载
		1200				//175%电机电流  2分钟过载
};
#define C_MOTOR_OV_MAX_CUR      1750
#define C_MOTOR_OV_MIN_CUR      1150
#define C_MOTOR_OV_STEP_CUR     100

//电机过载累计量的消除系数
Ulong const gMotorOverLoadDecTable[12] =
{
        (65536L*60/30),     //0%电机电流    3.0分钟消除过载
        (65536L*60/40),		//10%电机电流   4.0分钟消除过载
        (65536L*60/50),		//20%电机电流   5.0分钟消除过载
        (65536L*60/60),		//30%电机电流   6.0分钟消除过载
        (65536L*60/70),		//40%电机电流   7.0分钟消除过载
        (65536L*60/80),		//50%电机电流   8.0分钟消除过载
        (65536L*60/90),		//60%电机电流   9.0分钟消除过载
        (65536L*60/100),	//70%电机电流   10.0分钟消除过载
        (65536L*60/110),	//80%电机电流   11.0分钟消除过载
        (65536L*60/120),	//90%电机电流   12.0分钟消除过载
		(65536L*60/130),	//100%电机电流  13.0分钟消除过载
};

// //温度曲线表
Uint const gTempTableP44X[23] =
{
		624,614,603,590,576,561,		//6
		544,525,506,485,464,442,		//6
		419,395,373,350,327,305,		//6
		284,264,244,226,208				//5
};
Uint const gTempTableP8XX[23] =
{
		475,451,426,400,374,348,		//6
		323,299,275,253,232,212,		//6
		193,176,161,146,133,121,		//6
		110,100,91, 83, 76				//5
};
Uint const gTempTableBSMXX[23] =
{
		486,461,435,412,386,361,		//6
		337,313,291,269,248,228,		//6
		209,193,176,161,148,135,		//6
		123,113,103,94, 86				//5		
};
Uint const gTempTableSEMIKON[23] =
{
		558,519,480,451,418,392,		//6
		369,350,331,314,302,288,		//6
		278,269,262,254,247,243,		//6
		237,233,229,226,224				//5
};
Uint const gTempTableWAIZHI[23] =
{
		655,609,563,518,473,430,		//6
		389,350,314,282,251,224,		//6
		199,177,158,140,124,111,		//6
		99, 88, 78, 70, 62				//5		
};

// //文件内部函数声明
void    SoftWareErrorDeal(void);	
void	TemperatureCheck(void);					//温度检查
void	OutputPhaseLoseDetect(void);			//输出缺相检测
void 	OutputLoseReset(void);		
void	InputPhaseLoseDetect(void);				//输入缺相检测
void	ControlFan(void);						//风扇控制
void	OverLoadProtect(void);					//过载保护
void	CBCLimitCurProtect(void);
void 	SetCBCEnable(void);
void 	SetCBCDisable(void);
void	LoadLoseDetect(void);
void 	BufferResProtect(void);

/************************************************************
	变频器保护处理
************************************************************/
void InvDeviceControl(void)			
{
	//if(gADC.ResetTime < 500)
    {
    //    return;                         //AD采样稳定后开始
    }
	SoftWareErrorDeal();				//出错处理
	TemperatureCheck();					//温度检查
	OutputPhaseLoseDetect();			//输出缺相检测
	InputPhaseLoseDetect();				//输入缺相检测
	ControlFan();						//风扇控制
	OverLoadProtect();					//过载保护

    CBCLimitCurPrepare();					//逐波限流保护程序
	CBCLimitCurProtect();				//逐波限流情况下的过载判断
	
	LoadLoseDetect();					//掉载处理
	BufferResProtect();
}

/*************************************************************
	软件故障处理
停机状态也可能报过流故障、过压故障
*************************************************************/
void SoftWareErrorDeal(void)					
{
	if((gMainStatus.RunStep == STATUS_LOW_POWER) ||                         // 欠压
	   ((gError.ErrorCode.all & ERROR_SHORT_EARTH) == ERROR_SHORT_EARTH))
	{
		gUDC.uDCBigFilter = gUDC.uDCFilter;
		return;										//欠压状态下不判断软件故障
	}
	if((STATUS_STOP == gMainStatus.RunStep) &&
        (gError.LastErrorCode != gError.ErrorCode.all) && 
        (gError.ErrorCode.all != 0))
	{
	    gError.LastErrorCode = gError.ErrorCode.all;
        if(0 != gError.ErrorCode.all)
        {
			gPhase.IMPhase += 0x40000000L;				//故障复位后运行角度发生变化
        }
    }
    
// 开始故障复位
	if(gSubCommand.bit.ErrorOK == 1) 			
	{
		gError.ErrorCode.all = 0;
        gError.ErrorInfo[0].all = 0;
        gError.ErrorInfo[1].all = 0;
        gError.ErrorInfo[2].all = 0;
        gError.ErrorInfo[3].all = 0;
        gError.ErrorInfo[4].all = 0;
		if(gMainStatus.ErrFlag.bit.OvCurFlag == 1)
		{
			gMainStatus.ErrFlag.bit.OvCurFlag = 0;
			//gPhase.IMPhase += 16384;
			EALLOW;
			EPwm2Regs.TZCLR.bit.OST = 1;
			EPwm3Regs.TZCLR.bit.OST = 1;
			EPwm1Regs.TZCLR.bit.OST = 1;
			EPwm1Regs.TZCLR.bit.INT = 1;
			EDIS;
		}
	}

// 开始判断软件故障
	if(gLineCur.CurBaseInv > 10240)			// 2.5倍过流点判断
	// (采样电流为2倍额定电流，所以实际的过流保护没有)
	{
	    DINT;
		DisableDrive();
		gError.ErrorCode.all |= ERROR_OVER_CURRENT;
        gError.ErrorInfo[0].bit.Fault1 = 2;
		gLineCur.ErrorShow = gLineCur.CurPer;
        EINT;
	}
    
	if((gUDC.uDCBigFilter > gInvInfo.InvUpUDC) || //过压判断,使用大滤波电压
	    GetOverUdcFlag())
	{
	    DisableDrive(); //停机也允许报过压，提醒用户输入电压过高
	    gError.ErrorCode.all |= ERROR_OVER_UDC;
        gError.ErrorInfo[0].bit.Fault2 = 2;
	}
	else if(gUDC.uDCBigFilter < gInvInfo.InvLowUDC) //欠压判断,使用大滤波电压
	{
	    DisableDrive();
        gMainStatus.RunStep = STATUS_LOW_POWER;
        gMainStatus.SubStep = 0;
		DisConnectRelay();	
		gError.ErrorCode.all |= ERROR_LOW_UDC;
        gMainStatus.StatusWord.bit.LowUDC = 0;
	}
}

/************************************************************
温度检查,说明如下:

1、机型小于等于10       (2.2Kw 以下)
	温度曲线:	     1		TABLE_P44X
				2		TABLE_P8XX
				3		TABLE_SEMIKON
				4		TABLE_BSMXX

2、机型在（11～18之间） (11Kw 到 30Kw)
	温度曲线:	     1		TABLE_BSMXX
				2		TABLE_P44X
				3~4		TABLE_SEMIKON

3、机型在（19～26之间） (37Kw 到 200Kw,并且包含37, 不包含200)
	温度采样外置：		TABLE_WAIZHI

4、机型大于等于27		TABLE_BSMXX

5、机型大于等于27时，温度采样电路不同，需要进行3.3V和3V的转换；

6、机型大于等于19的85度保护；其他机型95度保护；

表格排列方式：每4度作一个数据，起始地址数据为12度，数据值为AD采样值
AD采样值：AD_RESULT>>6
************************************************************/
void TemperatureCheck(void)
{
	Uint    * m_pTable;
	Uint    m_TempAD,m_IndexLow,m_IndexHigh,m_Index,m_ErrTemp;
	Uint    m_LimtCnt;
    Uint    mType;

	//...准备温度表,690V的温度采样和保护与380V机型大于27的一致
	if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
    {
        mType = gInvInfo.InvTypeApply + 27;
    }
    else
    {
        mType = gInvInfo.InvTypeApply;
    }

// 保护温度点的确定
    if(mType >= 19)
    {
        m_ErrTemp = 85;
    }
    else
    {
        m_ErrTemp= 95;
    }
    if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
    {
       m_ErrTemp = 70;
       gInvInfo.TempType = 1;  /*1140V 过温点和温度曲线设定 2011.5.7 L1082*/
    }
// 温度曲线的选定
	if(mType <= 10)
	{
		if(gInvInfo.TempType == 1)
		{
			m_pTable = (Uint *)gTempTableP44X;
		}
		else if(gInvInfo.TempType == 2)
		{
			m_pTable = (Uint *)gTempTableP8XX;
		}
		else if(gInvInfo.TempType == 3)
		{
			m_pTable = (Uint *)gTempTableSEMIKON;
		}
		else    
		{
			m_pTable = (Uint *)gTempTableBSMXX;
		}
	}
	else if(mType <= 18)    // [11, 18]
	{
		if(gInvInfo.TempType == 1)
		{
			m_pTable = (Uint *)gTempTableBSMXX;
		}
		else if(gInvInfo.TempType == 2)
		{
			m_pTable = (Uint *)gTempTableP44X;
		}
		else
		{
			m_pTable = (Uint *)gTempTableSEMIKON;
		}
	}
    else if(mType <= 26)    // [19, 26]
    {
        m_pTable = (Uint *)gTempTableWAIZHI;
    }
    else        // mType >= 27
    {
        m_pTable = (Uint *)gTempTableBSMXX;
    }

// 硬件采样，以及采样电路的修正
    m_TempAD = (gTemperature.TempAD>>6);

#ifdef  TMS320F2808
    if(mType >= 27) // 3V和3.3v 采样电路的转换
    {
        m_TempAD = ((long)gTemperature.TempAD * 465)>>15; 
    }
#endif

// 开始查询温度表
	m_IndexLow  = 0;
	m_IndexHigh = 22;
	m_Index = 11;
	if(m_TempAD >= m_pTable[m_IndexLow])
	{	
		mType = 12 * 16;
	}
	else if(m_TempAD <= m_pTable[m_IndexHigh])
	{
		mType = 100 * 16;
	}
	else
	{
		m_LimtCnt = 0;
		while(m_LimtCnt < 7)
		{
			m_LimtCnt++;					//避免死循环
			if(m_TempAD == m_pTable[m_Index])
			{
				mType = (m_Index<<6) + (12 * 16);
				break;
			}
			else if(m_IndexLow+1 == m_IndexHigh)
			{
				mType = (m_IndexLow<<6) + (12 * 16) 
							+ ((m_pTable[m_IndexLow] - m_TempAD)<<6)
				                /(m_pTable[m_IndexLow] - m_pTable[m_IndexHigh]);
				break;
			}
			
			if(m_TempAD > m_pTable[m_Index])
			{
				m_IndexHigh = m_Index;
			}
			else
			{
				m_IndexLow = m_Index;
			}
			m_Index = (m_IndexLow + m_IndexHigh)>>1;
		}
	}
	if(mType - gTemperature.TempBak >= 8)			//温度变化超过0.5度才赋值
	{
		gTemperature.TempBak = mType;
		gTemperature.Temp = mType>>4;
	}

// 开始作温度判断和报警处理
	gTemperature.ErrCnt++;
	if(gTemperature.Temp < m_ErrTemp)
	{	
		gTemperature.ErrCnt = 0;
	}

	if(gTemperature.ErrCnt >= 5)
	{
		gTemperature.ErrCnt = 0;
		gError.ErrorCode.all |= ERROR_INV_TEMPERTURE;		//过热报警
	}

/*
    	//...准备温度表,690V的温度采样和保护与380V机型大于27的一致
	if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)  m_Temp = gInvInfo.InvTypeApply + 27;
     else                                             m_Temp = gInvInfo.InvTypeApply;
	m_TempAD = (gTemperature.TempAD>>6);
	m_ErrTemp = 95;
	if(m_Temp <= 10)
	{
		if(gInvInfo.TempType == 1)
		{
			m_pTable = (Uint *)gTempTableP44X;
		}
		else if(gInvInfo.TempType == 2)
		{
			m_pTable = (Uint *)gTempTableP8XX;
		}
		else if(gInvInfo.TempType == 3)
		{
			m_pTable = (Uint *)gTempTableSEMIKON;
		}
		else
		{
			m_pTable = (Uint *)gTempTableBSMXX;
		}
	}
	else if(m_Temp <= 18)
	{
		if(gInvInfo.TempType == 1)
		{
			m_pTable = (Uint *)gTempTableBSMXX;
		}
		else if(gInvInfo.TempType == 2)
		{
			m_pTable = (Uint *)gTempTableP44X;
		}
		else
		{
			m_pTable = (Uint *)gTempTableSEMIKON;
		}
	}
	else if(m_Temp >= 27)
	{
		m_pTable = (Uint *)gTempTableBSMXX;
        m_TempAD = ((long)gTemperature.TempAD * 465)>>15;   //机型大于26时，温度采样电路不同，需要进行
                                                            //3V和3.3v 采样电路的转换
	}
	else
	{
		m_ErrTemp = 85;				//机型大于等于19的85度保护
		m_pTable = (Uint *)gTempTableWAIZHI;
	}
*/
}

/************************************************************
	输出缺相检测
************************************************************/
void OutputPhaseLoseDetect(void)			
{
	Uint m_U,m_V,m_W;
	Uint m_Max,m_Min;
    
	if((gSubCommand.bit.OutputLost == 0) ||	
	   (gMainCmd.FreqReal < 80) ||					//0.8Hz以下不检测
	   (gMainCmd.Command.bit.StartDC == 1) ||		//直流制动状态下不检测
	   (gMainCmd.Command.bit.StopDC  == 1) ||
	   ((gMainStatus.RunStep != STATUS_RUN) && 		//不是运行或者速度搜索阶段
	    (gMainStatus.RunStep != STATUS_SPEED_CHECK)))
	{
		OutputLoseReset();
		return;
	}    

	if((gPhaseLose.Time < (Ulong)DSP_CLOCK * 1000000L) && (gPhaseLose.Cnt < 50000))
	{
		return;
	}

	m_U = gPhaseLose.TotalU/gPhaseLose.Cnt;
	m_V = gPhaseLose.TotalV/gPhaseLose.Cnt;
	m_W = gPhaseLose.TotalW/gPhaseLose.Cnt;

	m_Max = (m_U > m_V) ? m_U : m_V;
	m_Min = (m_U < m_V) ? m_U : m_V;
	m_Max = (m_Max > m_W) ? m_Max : m_W;
	m_Min = (m_Min < m_W) ? m_Min : m_W;

	if((m_Max > 500) && (m_Max/m_Min > 10))
	{
		gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
        gPhaseLose.errMaxCur = m_Max;
        gPhaseLose.errMinCur = m_Min;
	}
	OutputLoseReset();
}

void OutputLoseAdd(void)		//输出缺相检测累加电流处理
{
	gPhaseLose.TotalU += abs(gIUVWQ24.U >> 12);
	gPhaseLose.TotalV += abs(gIUVWQ24.V >> 12);
	gPhaseLose.TotalW += abs(gIUVWQ24.W >> 12);

    gPhaseLose.Time   += gPWM.gPWMPrdApply;
    //gPhaseLose.Time += 2000L * DSP_CLOCK;
	gPhaseLose.Cnt++;
}
void OutputLoseReset(void)		//输出缺相检测复位寄存器处理
{
	gPhaseLose.Cnt = 0;
	gPhaseLose.TotalU = 0;
	gPhaseLose.TotalV = 0;
	gPhaseLose.TotalW = 0;
	gPhaseLose.Time   = 0;
}

/************************************************************
	输入缺相检测
缺相检测原理: 
    继电器故障信号与缺相信号复合， 正常的时候一直为高；
    若PL一直为低，则继电器没有吸合；
    若PL为方波，则缺相；    
2808Dsp PL信号与VOE(硬件过压)信号在硬件上复合在一起，VOE有效时，采样为低(0)；
28035Dsp的VOE信号是单独的，缺相采样为PL信号；
18.5kw 以上才有缺相电路
************************************************************/
void InputPhaseLoseDetect(void)			
{
	if((gSubCommand.bit.InputLost == 0) ||                      //运行或转速跟踪状态才检测
	   ((STATUS_RUN!=gMainStatus.RunStep)&&(STATUS_SPEED_CHECK!=gMainStatus.RunStep))||
	   (gInvInfo.InvTypeApply < gInLose.ForeInvType))
	{
		gInLose.Cnt = 0;
		gInLose.UpCnt = 0;
		gInLose.ErrCnt = 0;
		gInLose.CntRes = 0;
		gInLose.UpCntRes = 0;
		return;
	}
    
	if(PL_INPUT_HIGH)           // PL是高电平		
	{
		gInLose.UpCnt ++;
		gInLose.UpCntRes ++;
	}
    
	if(gInLose.UpCntRes != 0)	// 持续500ms的PL低电平判断为继电器故障
	{
		gInLose.CntRes++;	
		if(gInLose.CntRes >= 250)       // 500ms
		{
			if(gInLose.UpCntRes >= 249)
			{
			    gError.ErrorCode.all |= ERROR_RESISTANCE_CONTACK;
			}
			gInLose.CntRes = 0;
			gInLose.UpCntRes = 0;
		}
	}

	gInLose.Cnt++;	
	if(gInLose.Cnt < 500)       //  缺相检测1sec一个循环 
    {
        return;
    }

	if((gInLose.UpCnt > 5) && (gInLose.UpCnt < 485))    // 1sec内PL存在低电平，为缺相方波
	{
		gInLose.ErrCnt++;
		if(gInLose.ErrCnt >= 2)
		{
			gError.ErrorCode.all |= ERROR_INPUT_LACK_PHASE;
			gInLose.ErrCnt = 0;
		}
	}
	else
	{
		gInLose.ErrCnt = 0;
	}
	gInLose.Cnt = 0;
	gInLose.UpCnt = 0;
}

/************************************************************
	风扇控制
1）	上电缓冲状态结束后1秒钟内，风扇不运行；
2）	运行状态，风扇运行；
3）	直流制动等待期间，风扇运行
4）	？槲露鹊陀�40度，风扇停止；温度高于42度风扇运行；40～42度之间不变化。
5)	风扇启动后至少10秒才关闭
************************************************************/
void ControlFan(void)						
{
	if(gMainStatus.RunStep == STATUS_LOW_POWER)
	{
		TurnOffFan();
		gFanCtrl.EnableCnt = 0;
		return;
	}

	gFanCtrl.EnableCnt++;
	if(gFanCtrl.EnableCnt < 500)
	{
		TurnOffFan();
		return;
	}
	gFanCtrl.EnableCnt = 500;

	if((gMainCmd.Command.bit.Start == 1) ||
	   (gTemperature.Temp > 42) ||
	   (gSubCommand.bit.FanNoStop == 1))
	{
		gFanCtrl.RunCnt = 0;
		TurnOnFan();
	}
	else if(gTemperature.Temp < 40)
	{
		gFanCtrl.RunCnt++;
		if(gFanCtrl.RunCnt >= 5000)
		{
			gFanCtrl.RunCnt = 5000;
			TurnOffFan();
		}
	}
}

/************************************************************
	变频器和电机过载保护
************************************************************/
void OverLoadProtect(void)				
{
	Ulong m_LDeta = 0;
	Uint m_Cur,m_Data,m_Index,m_CurBaseInv;
	Uint m_TorCurBottom,m_TorCurUpper,m_TorCurStep,m_TorCurData;
    Uint *m_TorCurLine;

    m_CurBaseInv = gLineCur.CurBaseInv;
    if( 28 <= gInvInfo.InvTypeApply )
    {
        //m_CurBaseInv = (s32)gLineCur.CurBaseInv * (s32)gInvInfo.InvCurrent / gInvInfo.InvOlCurrent;
        m_CurBaseInv = (long)gLineCur.CurBaseInv * (long)gInvInfo.InvCurrent / gInvInfo.InvCurrForP;
    }
    
	gOverLoad.FilterInvCur = Filter16(m_CurBaseInv,gOverLoad.FilterInvCur);
	gOverLoad.FilterMotorCur = Filter16(gLineCur.CurPer,gOverLoad.FilterMotorCur);
	gOverLoad.FilterRealFreq = Filter16(gMainCmd.FreqReal,gOverLoad.FilterRealFreq);

	if(gMainStatus.RunStep == STATUS_LOW_POWER)
	{
		gOverLoad.InvTotal.all = 0;
		gOverLoad.MotorTotal.all = 0;
		gOverLoad.Cnt = 0;
		gMainStatus.StatusWord.bit.PerOvLoadInv = 0;
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
		return;
	}

	gOverLoad.Cnt++;
	if(gOverLoad.Cnt < 5)		
	{
        return;		    //每10ms判断一次
	}
	gOverLoad.Cnt = 0;

	////////////////////////////////////////////////////////////////
	//选择过载曲线

    if(1 == gInvInfo.GPType)        //G型机过载曲线
    {
        m_TorCurLine    = (Uint *)gInvOverLoadTable;
        m_TorCurBottom  = 1150;
        m_TorCurUpper   = 1870;
        m_TorCurStep    = 90;
        m_TorCurData    = 20;
    }
    else                            //P型机过载曲线
    {       
        m_TorCurLine    = (Uint *)gInvOverLoadTableForP;
        m_TorCurBottom  = 1050;
        m_TorCurUpper   = 1370;
        m_TorCurStep    = 40;
        m_TorCurData    = 10;
    }

	////////////////////////////////////////////////////////////////
	//开始判断变频器的过载
	m_Cur = ((Ulong)gOverLoad.FilterInvCur * 1000L) >> 12;
	if(m_Cur < m_TorCurBottom)
    {
		if(gOverLoad.InvTotal.half.MSW < 10)
		{
			gOverLoad.InvTotal.all = 0;
		}
        else if(gMainStatus.RunStep == STATUS_STOP)
		{
			gOverLoad.InvTotal.all -= gInvOverLoadDecTable[0];  
		}
		else if(m_Cur < 1000)       /*小于变频器额定电流，按照当前电流大小消除过载累计量*/
		{
			gOverLoad.InvTotal.all -= gInvOverLoadDecTable[m_Cur/100 + 1];
		}
	}
	else
	{
		if(gOverLoad.FilterRealFreq < 500)		//电流 = 电流/[0.9*(f/5)+0.1]
		{
			m_Data = gOverLoad.FilterRealFreq * 13 + 26214;
			m_Cur  = (((Ulong)m_Cur)<<15)/m_Data;
		}
		if(m_Cur >= m_TorCurUpper)
		{
			m_Data = m_TorCurData;
		}
		else
		{
			m_Index = (m_Cur - m_TorCurBottom)/m_TorCurStep;
			m_Data = *(m_TorCurLine + m_Index) -
			         (((long)(*(m_TorCurLine + m_Index)) - (*(m_TorCurLine + m_Index + 1))) * 
					  (long)(m_Cur - m_TorCurBottom - m_Index * m_TorCurStep))/m_TorCurStep;
		}
		m_LDeta = ((Ulong)3600<<16)/(Uint)m_Data;
		gOverLoad.InvTotal.all += m_LDeta;
	
		if(gOverLoad.InvTotal.half.MSW >= 36000)
		{
			gOverLoad.InvTotal.half.MSW = 36000;
			//AddOneError(ERROR_INV_OVER_LOAD,1);

			gMainStatus.StatusWord.bit.PerOvLoadInv = 0;
			gError.ErrorCode.all |= ERROR_INV_OVER_LAOD;
            gError.ErrorInfo[1].bit.Fault1 = 2;
		}
	}
    //变频器过载预报警处理
	if(((gOverLoad.InvTotal.all + m_LDeta * 1000UL)>>16) > 36000)
	{
		gMainStatus.StatusWord.bit.PerOvLoadInv = 1;
	}
	else
	{
		gMainStatus.StatusWord.bit.PerOvLoadInv = 0;
	}

	////////////////////////////////////////////////////////////////
	//开始判断电机的过载
	//if(gMainCmd.SubCmd.bit.MotorOvLoadEnable == 0)
	if(gSubCommand.bit.MotorOvLoad == 0)
	{
		gOverLoad.MotorTotal.all = 0;
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
		return;
	}
	//m_Cur = ((Ulong)gOverLoad.FilterMotorCur * 1000L)>>12;
	//m_Cur = ((Ulong)m_Cur * 100L)/gBasePar.MotorOvLoad;
	//m_LDeta = (Ulong)m_Cur * (Ulong)gMotorInfo.CurBaseCoff;
	m_Cur = ((Ulong)gOverLoad.FilterMotorCur * 1000L)>>12;
	m_Cur = ((Ulong)m_Cur * 100L)/gComPar.MotorOvLoad;          // 根据过载保护系数计算出保护电流，
                                                            	//然后用该保护电流查询过载保护曲线
	m_LDeta = (Ulong)m_Cur * (Ulong)gMotorInfo.CurBaseCoff;
	if(m_LDeta >= (C_MOTOR_OV_MAX_CUR * 256L))
	{
		m_Cur = C_MOTOR_OV_MAX_CUR;
	}
	else
	{
		m_Cur = m_LDeta>>8;
	}

	if(m_Cur < C_MOTOR_OV_MIN_CUR)
	{
		if(gOverLoad.MotorTotal.half.MSW < 10)
		{
			gOverLoad.MotorTotal.all = 0;
		}
        else if(gMainStatus.RunStep == STATUS_STOP)
		{
			gOverLoad.MotorTotal.all -= gMotorOverLoadDecTable[0];  
		}
		else if(m_Cur < 1000)                   /*小于100%额定电流按照电流消除电机过载*/
		{
			gOverLoad.MotorTotal.all -= gMotorOverLoadDecTable[m_Cur/100 + 1];  
		}
	}
	else
	{
		if(m_Cur >= C_MOTOR_OV_MAX_CUR)
		{
			m_Data = gMotorOverLoadTable[C_MOTOR_OV_TAB_NUM - 1];
		}
		else
		{
			m_Index = (m_Cur - C_MOTOR_OV_MIN_CUR)/C_MOTOR_OV_STEP_CUR;
			m_Data = gMotorOverLoadTable[m_Index] -
			         ((long)(gMotorOverLoadTable[m_Index] - gMotorOverLoadTable[m_Index+1]) * 
					  (long)(m_Cur - C_MOTOR_OV_MIN_CUR - m_Index * C_MOTOR_OV_STEP_CUR))/C_MOTOR_OV_STEP_CUR;
		}
		m_LDeta = ((Ulong)3600<<16)/(Uint)m_Data;
		gOverLoad.MotorTotal.all += m_LDeta;

		if(gOverLoad.MotorTotal.half.MSW > 36000)
		{
			gOverLoad.MotorTotal.half.MSW = 36000;
			//AddOneError(ERROR_MOTOR_OVER_LOAD,1);
			gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
			gError.ErrorCode.all |= ERROR_MOTOR_OVER_LOAD;
		}		
	}
    //电机过载预报警处理   
	//if(gOverLoad.MotorTotal.half.MSW > gBasePar.PerMotorOvLoad * 360)
	if(gOverLoad.MotorTotal.half.MSW > gComPar.PerMotorOvLoad * 360)
	{
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 1;
	}
	else
	{
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
	}
}


/************************************************************
	处于逐波限流状态下的过载保护
单管持续逐波限流时间超过500ms保护，对应250个2ms
************************************************************/
void CBCLimitCurProtect(void)
{
	int     m_CurU,m_CurV,m_CurW,m_Coff;
	int	    m_Max,m_Add,m_Sub;
    Uint    m_Limit;
    
	if(gSubCommand.bit.CBCEnable == 1)
	{
		if(gCBCProtect.EnableFlag == 0) SetCBCEnable();	//启用逐波限流功能
	}
	else
	{
		if(gCBCProtect.EnableFlag == 1)  SetCBCDisable();//取消逐波限流功能
	}
		
	//开始分别计算三相电流绝对值的积分
	m_Coff = (((long)gMotorInfo.Current)<<10) / gInvInfo.InvCurrent;
	m_CurU = (int)(((long)gIUVWQ12.U * (long)m_Coff)>>10);
	m_CurU = abs(m_CurU);
	m_CurV = (int)(((long)gIUVWQ12.V * (long)m_Coff)>>10);
	m_CurV = abs(m_CurV);
	m_CurW = (int)(((long)gIUVWQ12.W * (long)m_Coff)>>10);
	m_CurW = abs(m_CurW);
    
	//开始判断是否有一相持续大电流超过5秒
	if(m_CurU > 9267)	gCBCProtect.CntU++;             // 9267 = 4096 * 1.414 * 1.6
	else				gCBCProtect.CntU = gCBCProtect.CntU>>1;
	gCBCProtect.CntU = (gCBCProtect.CntU > 3000)?3000:gCBCProtect.CntU;

	if(m_CurV > 9267)	gCBCProtect.CntV++;
	else				gCBCProtect.CntV = gCBCProtect.CntV>>1;
	gCBCProtect.CntV = (gCBCProtect.CntV > 3000)?3000:gCBCProtect.CntV;

	if(m_CurW > 9267)	gCBCProtect.CntW++;
	else				gCBCProtect.CntW = gCBCProtect.CntW>>1;
	gCBCProtect.CntW = (gCBCProtect.CntW > 3000)?3000:gCBCProtect.CntW;
    
	if(gMainCmd.FreqReal > 20)
	{
		gCBCProtect.CntU = 0;
		gCBCProtect.CntV = 0;
		gCBCProtect.CntW = 0;
	}

	if((gCBCProtect.CntU > 2500) || 		//单相大电流报过载, 任何一相持续5000ms
	   (gCBCProtect.CntV > 2500) || 
	   (gCBCProtect.CntW > 2500) )
	{
		gError.ErrorCode.all |= ERROR_INV_OVER_LAOD;
        gError.ErrorInfo[1].bit.Fault1 = 1;
	}
    
	if(gCBCProtect.EnableFlag == 0)
	{
		gCBCProtect.TotalU = 0;
		gCBCProtect.TotalV = 0;
		gCBCProtect.TotalW = 0;
		return;
	}
    
    // 根据温度和设定CBC时间点确定CBC时间
    m_Limit = 500;        //逐波限流 500MS，取消功能设置 2011.5.7 L1082；
   /*
    if(gTemperature.Temp < 40)  // 40度
    {
        m_Limit = gCBCProtect.maxCBCTime * 100;     // 单位转换: 0.1sec -> 1ms
    }
    else if(gTemperature.Temp > 60)
    {
        m_Limit = gCBCProtect.minCBCTime * 100;
    }
    else
    {
        m_Limit = (gCBCProtect.maxCBCTime - gCBCProtect.minCBCTime) * 100;
        m_Limit = (long)m_Limit * (60 - gTemperature.Temp)/20 + gCBCProtect.minCBCTime * 100;
    }
    m_Limit *= 3;       // 由于3相同时关断， 
                        // 28035 肯定是三相同时计数
    */
    
	m_Add = 2;
	m_Sub = 1;
	if(gMainStatus.RunStep == STATUS_STOP)
    {
        m_Sub = 2;
    }

	if(gCBCProtect.Flag.bit.CBC_U == 1)   //计算U相电流的积分
	{
		gCBCProtect.TotalU += m_Add;
	}
	else //if(m_CurU < 8688)					//小于1.5倍变频器峰值电流后累加值递减 : 4096*1.5*sqrt(2)
	{
		gCBCProtect.TotalU -= m_Sub;
	}

	if(gCBCProtect.Flag.bit.CBC_V == 1) 	//计算V相电流的积分
	{
		gCBCProtect.TotalV += m_Add;
	}
	else //if(m_CurV < 8688)					//小于1.5倍变频器电流后累加值递减
	{
		gCBCProtect.TotalV -= m_Sub;
	}

	if(gCBCProtect.Flag.bit.CBC_W == 1) 	//计算W相电流的积分
	{
		gCBCProtect.TotalW += m_Add;
	}
	else //if(m_CurW < 8688)					//小于1.5倍变频器电流后累加值递减
	{
		gCBCProtect.TotalW -= m_Sub;
	}

	gCBCProtect.Flag.all = 0;

	//电流积分值限幅
	gCBCProtect.TotalU = __IQsat(gCBCProtect.TotalU, m_Limit, 0);
	gCBCProtect.TotalV = __IQsat(gCBCProtect.TotalV, m_Limit, 0);
	gCBCProtect.TotalW = __IQsat(gCBCProtect.TotalW, m_Limit, 0);

	m_Max = (gCBCProtect.TotalU > gCBCProtect.TotalV) ? gCBCProtect.TotalU : gCBCProtect.TotalV;
	m_Max = (m_Max > gCBCProtect.TotalW) ? m_Max : gCBCProtect.TotalW;
    if(m_Max >= m_Limit)      //逐波限流报40号故障
    {
        gError.ErrorCode.all |= ERROR_TRIP_ZONE;
    }
}

/*************************************************************
	开启逐波限流功能
*************************************************************/
void SetCBCEnable(void)
{
	gCBCProtect.EnableFlag = 1;
    EALLOW;
    
#ifdef TMS320F2808	
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
	EPwm1Regs.TZSEL.bit.CBC3 = TZ_ENABLE;		// EPWM1的逐波限流
	EPwm1Regs.TZSEL.bit.CBC4 = TZ_ENABLE;
        
	EPwm2Regs.TZSEL.bit.CBC2 = TZ_ENABLE;       // EPWM2
    EPwm2Regs.TZSEL.bit.CBC3 = TZ_ENABLE;
    EPwm2Regs.TZSEL.bit.CBC4 = TZ_ENABLE;

    EPwm3Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
    EPwm3Regs.TZSEL.bit.CBC3 = TZ_ENABLE;
	EPwm3Regs.TZSEL.bit.CBC4 = TZ_ENABLE;       // EPWM3
#else
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
    EPwm2Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
    EPwm3Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
#endif

	EDIS;
}
/*************************************************************
	关闭逐波限流功能
*************************************************************/
void SetCBCDisable(void)
{
	gCBCProtect.EnableFlag = 0;

	EALLOW;

#ifdef TMS320F2808
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
	EPwm1Regs.TZSEL.bit.CBC3 = TZ_DISABLE;		// EPWM1的逐波限流
	EPwm1Regs.TZSEL.bit.CBC4 = TZ_DISABLE;
        
	EPwm2Regs.TZSEL.bit.CBC2 = TZ_DISABLE;       // EPWM2
    EPwm2Regs.TZSEL.bit.CBC3 = TZ_DISABLE;
    EPwm2Regs.TZSEL.bit.CBC4 = TZ_DISABLE;

    EPwm3Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
    EPwm3Regs.TZSEL.bit.CBC3 = TZ_DISABLE;
	EPwm3Regs.TZSEL.bit.CBC4 = TZ_DISABLE;       // EPWM3
#else
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
    EPwm2Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
    EPwm3Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
#endif

	EDIS;
}

/*************************************************************
	输出掉载处理

在运行状态，电流小于电机额定电流5%，并持续100ms认为是掉载
同步机不做掉载保护
*************************************************************/
void LoadLoseDetect(void)
{
	Uint m_Limit,m_LimitFreq;

	//掉载判断阀值为电机额定电流的5%。
	m_Limit = (gLoadLose.ChkLevel * (Ulong)gMotorInfo.CurrentGet) / gMotorInfo.Current << 2; // *4096/1000
	m_Limit = (long)m_Limit * 1447L >> 10;              // * sqrt(2)
	m_Limit = (m_Limit < 20) ? 20 : m_Limit;
	m_LimitFreq = ((Ulong)gMotorInfo.FreqPer * 1638) >> 15;

	if((gMotorInfo.MotorType == MOTOR_TYPE_PM)              // 同步机不做掉载检测
        ||(gSubCommand.bit.LoadLose != 1)
        || (abs(gMainCmd.FreqSyn) < m_LimitFreq)			//电机额定频率5%以下不检测
	    || (gMainCmd.Command.bit.StartDC == 1)
	    || (gMainCmd.Command.bit.StopDC  == 1)				//直流制动状态不检测
	    || (gMainStatus.RunStep != STATUS_RUN)				//非运行状态不检测
	    || (abs(gIUVWQ12.U) >= m_Limit)                     //任何一相电流大于200 (5%) 认为不掉载
	    || (abs(gIUVWQ12.V) >= m_Limit) 
	    || (abs(gIUVWQ12.W) >= m_Limit) 
	    )							
	{
		gLoadLose.ErrCnt = 0;
		gMainStatus.StatusWord.bit.OutOff = 0;
		return;
	}
	
	gLoadLose.ErrCnt++;
    if((gLoadLose.ErrCnt<<1) > (gLoadLose.ChkTime*100))     // 掉载检出时间确定
	//if(gLoadLose.ErrCnt > 50)
	{
		gLoadLose.ErrCnt = 50;
        gError.ErrorCode.all |= ERROR_LOAD_LOST;
		gMainStatus.StatusWord.bit.OutOff = 1;
	}
}
/*************************************************************
	缓冲电阻保护处理
	
持续的进入欠压状态，认为是缓冲电阻故障
*************************************************************/
void BufferResProtect(void)
{
	if(gBuffResCnt >= 150000)			//缓冲电阻保护处理
	{
		gError.ErrorCode.all |= ERROR_RESISTER_HOT;
	}
	gBuffResCnt--;	
	gBuffResCnt = __IQsat(gBuffResCnt,200000,0);					
}

