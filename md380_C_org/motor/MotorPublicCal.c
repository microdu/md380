/****************************************************************
文件功能：包含驱动部分与控制方式无关的独立模块
文件版本：
更新日期：

****************************************************************/
#include "MotorPublicCalInclude.h"

/************************************************************/
/********************全局变量定义****************************/
DC_BRAKE_STRUCT			gDCBrake;	//直流制动用变量
BRAKE_CONTROL_STRUCT	gBrake;		//制动电阻控制用变量
SHORT_GND_STRUCT		gShortGnd;
JUDGE_POWER_LOW			gLowPower;	    //上电缓冲判断使用数据结构
/************************************************************
	上电对地短路判断
************************************************************/
void RunCaseShortGnd(void)
{
	switch(gMainStatus.SubStep)
	{
		case 1:
            gMainStatus.PrgStatus.bit.PWMDisable = 1;
			gShortGnd.Comper = SHORT_GND_PERIOD;
			gShortGnd.ocFlag = 0;
			gShortGnd.BaseUDC = gUDC.uDC;
			gShortGnd.ShortCur = 0;

			EALLOW;
			EPwm1Regs.TBPRD = SHORT_GND_PERIOD;
			EPwm1Regs.CMPA.half.CMPA = SHORT_GND_PERIOD;
			EPwm1Regs.CMPB  = EPwm1Regs.TBPRD - gADC.DelayApply;
			EPwm2Regs.TBPRD = SHORT_GND_PERIOD;
			EPwm3Regs.TBPRD = SHORT_GND_PERIOD;


			EPwm1Regs.AQCSFRC.all = 0x08;
			//EPwm1Regs.AQCSFRC.bit.CSFB = 1;	//强制关闭某些桥臂
			EPwm2Regs.AQCSFRC.all = 0x0A;
			//EPwm2Regs.AQCSFRC.bit.CSFA = 2;
			//EPwm2Regs.AQCSFRC.bit.CSFB = 2;
			EPwm3Regs.AQCSFRC.all = 0x0A;
			//EPwm3Regs.AQCSFRC.bit.CSFA = 2;
			//EPwm3Regs.AQCSFRC.bit.CSFB = 2;
			EPwm1Regs.DBCTL.all = 0;
			EPwm2Regs.DBCTL.all = 0;
			EPwm3Regs.DBCTL.all = 0;
			//EPwm1Regs.DBCTL.bit.OUT_MODE = DB_DISABLE;
			//EPwm2Regs.DBCTL.bit.OUT_MODE = DB_DISABLE;
			//EPwm3Regs.DBCTL.bit.OUT_MODE = DB_DISABLE;
			EDIS;

			gMainStatus.SubStep = 2;
			break;

		case 2:
			gMainStatus.SubStep = 3;
			EnableDrive();
			break;

		case 3:
			if((gShortGnd.ocFlag != 0) || 
			  (abs(gShortGnd.ShortCur) > (30 * 32)) ||	//410*2 为峰值电流，与变频器额定电流10%比较
			  (gUDC.uDC > gShortGnd.BaseUDC + 650))	//母线电压上升65V
			{							
				//上电对地短路处理
				DisableDrive();
				gError.ErrorCode.all |= ERROR_SHORT_EARTH;
				gMainStatus.SubStep = 4;
				break;
			}
			
			if(gShortGnd.Comper <= SHORT_GND_CMPR_INC)
			{
				gMainStatus.SubStep = 4;
			}
			else
			{
				gShortGnd.Comper -= SHORT_GND_CMPR_INC;
				EALLOW;
				EPwm1Regs.CMPA.half.CMPA = gShortGnd.Comper;
				EDIS;
			}
			break;

		case 4:
			DisableDrive();
			EALLOW;
			EPwm1Regs.DBCTL.all = 0x0007;
			EPwm2Regs.DBCTL.all = 0x0007;
			EPwm3Regs.DBCTL.all = 0x0007;
			//EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
			//EPwm2Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
			//EPwm3Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
			EPwm1Regs.AQCSFRC.all = 0x0;
			EPwm2Regs.AQCSFRC.all = 0x0;
			EPwm3Regs.AQCSFRC.all = 0x0;
		
			EDIS;
			gMainStatus.StatusWord.bit.ShortGndOver = 1;
			gMainStatus.RunStep = STATUS_STOP;
            gMainStatus.PrgStatus.bit.PWMDisable = 0;
			gMainStatus.SubStep = 0;
			if(gMainStatus.ErrFlag.bit.OvCurFlag == 1)
			{
				gMainStatus.ErrFlag.bit.OvCurFlag = 0;
				EALLOW;
				EPwm1Regs.TZCLR.bit.INT = 1;
				EDIS;
			}
			break;

		default:
			break;
	}	
}

/************************************************************
	变频器的上电处理过程：判断母线电压稳定，屏蔽上电缓冲电阻
判断依据：1）母线电压大于欠压点 
       && 2）母线电压没有再增加 
       && 3）持续200ms
		  4）判断母线电压稳定后再延时200ms，进入停机状态。
************************************************************/
void RunCaseLowPower(void)
{
	Uint uDCLowLimt;

	uDCLowLimt = gInvInfo.InvLowUDC + 200;
	if(INV_VOLTAGE_220V == gInvInfo.InvVoltageType)
	{
		uDCLowLimt = gInvInfo.InvLowUDC + 160;
	}

	switch(gMainStatus.SubStep)
	{
		case 2://判断母线电压大于欠压点&&电压没再增加&&持续200ms
			if((gUDC.uDCFilter > uDCLowLimt) && 
			   (gUDC.uDCFilter <= gLowPower.UDCOld))
			{
				if((gLowPower.WaiteTime++) >= 100)
				{
					gExcursionInfo.EnableCount = 199;
					gMainStatus.SubStep ++;	
					gLowPower.WaiteTime = 0;			
				}
			}
			else
			{
				gLowPower.WaiteTime = 0;
				gLowPower.UDCOld = gUDC.uDCFilter;
			}

			break;

		case 3:	//200ms延时
			if(((gLowPower.WaiteTime++) >= 100) && 
			   (gExcursionInfo.EnableCount >= 200))
			{
				gMainStatus.RunStep = STATUS_STOP;
				gMainStatus.SubStep = 1;
                gMainStatus.StatusWord.bit.LowUDC = 1;
				gMainStatus.StatusWord.bit.RunEnable = 1;
        		if((gError.ErrorCode.all & ERROR_LOW_UDC) == ERROR_LOW_UDC)
                {
                    gError.ErrorCode.all = 0;                   //欠压后需要清除故障?
		            if(gMainStatus.ErrFlag.bit.OvCurFlag == 1)  //修改欠压后无法进入过流中断的错误
		            {
			            gMainStatus.ErrFlag.bit.OvCurFlag = 0;
			            EALLOW;
			            EPwm2Regs.TZCLR.bit.OST = 1;
			            EPwm3Regs.TZCLR.bit.OST = 1;
			            EPwm1Regs.TZCLR.bit.OST = 1;
			            EPwm1Regs.TZCLR.bit.INT = 1;
			            EDIS;
		            }     
        		}
				ConnectRelay();				
				gBuffResCnt += 30000;
				gPhase.IMPhase = GetTime() << 28;	//上电后随机选择初始相位
			}
			break;

		default:
			DisConnectRelay();	
            gError.ErrorCode.all |= ERROR_LOW_UDC;				//出错标志
        	gMainStatus.StatusWord.bit.LowUDC = 0;
			gMainStatus.StatusWord.bit.StartStop = 0;
			gLowPower.WaiteTime = 0;
			gLowPower.UDCOld = gUDC.uDCFilter;
			gMainStatus.SubStep = 2;
			break;
	}
}

/************************************************************
	判断UVW三相是否处于逐波限流状态，并且设置标志
************************************************************/
void CBCLimitCurPrepare(void)
{
	EALLOW;
	if(EPwm1Regs.TZFLG.bit.CBC == 1) 		//表示U相处于逐波限流状态
	{
		EPwm1Regs.TZCLR.bit.CBC = 1;
		gCBCProtect.Flag.bit.CBC_U = 1;
	}
	
	if(EPwm2Regs.TZFLG.bit.CBC == 1) 		//表示V相处于逐波限流状态
	{
		EPwm2Regs.TZCLR.bit.CBC = 1;
		gCBCProtect.Flag.bit.CBC_V = 1;
	}

	if(EPwm3Regs.TZFLG.bit.CBC == 1) 		//硎網相处于逐波限流状态
	{
		EPwm3Regs.TZCLR.bit.CBC = 1;
		gCBCProtect.Flag.bit.CBC_W = 1;
	}
	EDIS;
}
/*************************************************************
	选择三相电流中的最大值
*************************************************************/
Uint MaxUVWCurrent(void)
{
	Uint m_IU,m_IV,m_IW;

	m_IU = abs(gIUVWQ24.U>>12);
	m_IV = abs(gIUVWQ24.V>>12);
	m_IW = abs(gIUVWQ24.W>>12);

	m_IU = (m_IU >= m_IV)?m_IU:m_IV;
	m_IU = (m_IU >= m_IW)?m_IU:m_IW;

	return m_IU;
}
/*************************************************************
	过流中断处理程序（可屏蔽中断，电平触发）

28035 也可能是过压中断，需要软件判断；
28035 在发生过压后，会频繁的进入该中断， 验证这样没有明显的问题
*************************************************************/
void HardWareErrorDeal()
{
#ifdef TMS320F28035 // 需要判断是否过流
    int sum = 0, i;
    
    gMainStatus.ErrFlag.bit.OvCurFlag = 1;		//发生了过流中断的标志

    for(i = 0; i < 10; i++)     // 需要2us， 母线肯定是不变的
    {
        sum += GpioDataRegs.AIODAT.bit.AIO2;
    }
    if(sum < 5)                // io口为低， 判断为过压，
    {
        HardWareOverUDCDeal();
        return;         // 过压在AD中断中处理, 2ms中有处理
    }
    
#else
	gMainStatus.ErrFlag.bit.OvCurFlag = 1;		//发生了过流中断的标志
#endif

    if(gMainStatus.RunStep == STATUS_SHORT_GND)
	{
		gShortGnd.ocFlag = 1;						//上电对地短路
	}
	else if((gError.ErrorCode.all & ERROR_OVER_CURRENT) != ERROR_OVER_CURRENT)
	{
		gError.ErrorCode.all |= ERROR_OVER_CURRENT;
        gError.ErrorInfo[0].bit.Fault1 = 1;
		gLineCur.ErrorShow = MaxUVWCurrent();	//硬件过流，记录故障电流
	}

	EALLOW;
	EPwm1Regs.TZCLR.bit.OST = 1;
	EPwm2Regs.TZCLR.bit.OST = 1;
	EPwm3Regs.TZCLR.bit.OST = 1;
	EDIS;
    
    return;
}

/*************************************************************
	过压中断处理程序（不可屏蔽中断，上升沿触发）
*************************************************************/
void HardWareOverUDCDeal(void)					
{
    /*
	if(gUDC.uDC < gInvInfo.InvUpUDC - 300)
	{
		return;									//母线电压比过压点低30V，避免误报警
	}	                                              //但是AD采样错误，也可能导致不报警
    */
	DisableDrive();								//封锁输出
	if(gMainStatus.RunStep == STATUS_SHORT_GND)
	{
		gShortGnd.ocFlag = 2;						//上电对地短路检测阶段的标志
	}
	else 
	{
		gError.ErrorCode.all |= ERROR_OVER_UDC;				//过压处理
        gError.ErrorInfo[0].bit.Fault2 = 1;
	}
    
    EALLOW;
	EPwm1Regs.TZCLR.bit.OST = 1;
	EPwm2Regs.TZCLR.bit.OST = 1;
	EPwm3Regs.TZCLR.bit.OST = 1;
	EDIS;

}

/************************************************************
直流制动程序：在载波周期中，通过PI调节器计算直流制动下的输出电?
直流制动的时候，只有一相开关管动作
************************************************************/
void RunCaseDcBrake(void)		
{
	int m_BrakeCur;
	
	if(gMainCmd.Command.bit.StartDC == 1)
	{
		//m_BrakeCur = (((long)gComPar.StartDCBrakeCur)<<12)/100;
		m_BrakeCur = gComPar.StartDCBrakeCur * 41;			//4096/100 ~= 41
	}
	else if(gMainCmd.Command.bit.StopDC == 1)
	{
		//m_BrakeCur = (((long)gComPar.StopDCBrakeCur)<<12)/100;
		m_BrakeCur = gComPar.StopDCBrakeCur * 41;			//4096/100 ~= 41
	}

	gDCBrake.Time++;
	if(gDCBrake.Time < 2)		//直流制动的前几拍，按照定子电阻和电流计算电压
	{
		gOutVolt.Volt = ((Ulong)m_BrakeCur * (Ulong)gMotorExtPer.R1)>>16;
		gDCBrake.PID.Total = ((long)gOutVolt.Volt<<16);
	}
	else						//通过PI调节器控制直流制动电流
	{
		gDCBrake.Time = 10;
		gDCBrake.PID.Deta = m_BrakeCur - (int)gLineCur.CurPer;
        gDCBrake.PID.KP   = 1600/16;
		//gDCBrake.PID.KP   = 1600;
		gDCBrake.PID.KI   = 300;
        if( 16 < gInvInfo.InvTypeApply )  //大功率直流制动时KI减小一半，防止电流振荡。
        {
            gDCBrake.PID.KI = 150;
        }
        gDCBrake.PID.QP = 0;
        gDCBrake.PID.QI = 0;
		gDCBrake.PID.KD   = 0;
		gDCBrake.PID.Max  = 4096;
		gDCBrake.PID.Min  = 0;
		PID((PID_STRUCT *)&gDCBrake.PID);
		gOutVolt.Volt = gDCBrake.PID.Out>>16;		
	}
	gOutVolt.VoltApply = gOutVolt.Volt;  	
                                        //直流制动情况下，不修改输出电压相位角, 电压相位不更新
}

/************************************************************
	制动电阻控制
************************************************************/
void BrakeResControl(void)
{
    //Uint m_UData, m_UData1, m_UData2, m_UData3;
/* 
// 计算制动电阻开通和关断次数（载波周期判断次数）
    //m_UData = (Ulong)PWM_CLOCK * 1000 / gPWM.gPWMPrd;   //  2ms包含的PWM周期
    //m_UData = 4;
    m_UData = 1;
	m_UData2 = 100 - gComPar.BrakeCoff;
	m_UData3 = (m_UData2<gComPar.BrakeCoff)?m_UData2:gComPar.BrakeCoff;     // min
	m_UData2 = (m_UData2>gComPar.BrakeCoff)?m_UData2:gComPar.BrakeCoff;     // max
	
	if(m_UData3 == 0)	
	{
		m_UData1 = 65535;
		m_UData  = 0;
	}
	else
	{
		m_UData1 = ((Ulong)m_UData * (Ulong)m_UData2)/m_UData3;		
	}

	if(gComPar.BrakeCoff < 50)      // swap m_UData1 with m_UData
	{
		m_UData3 = m_UData1;
		m_UData1 = m_UData;
		m_UData  = m_UData3;
	}
	gBrake.OnCnt  = m_UData1;
	gBrake.OffCnt = m_UData;
*/
    //Uint max, min;

    if(gComPar.BrakeCoff == 100)
    {
        gBrake.OnCnt = 65535;
        gBrake.OffCnt = 0;
    }
    else if(gComPar.BrakeCoff == 0)
    {
        gBrake.OnCnt = 0;
        gBrake.OffCnt = 65535;
    }
    else if(gComPar.BrakeCoff <= 50)
    {
        gBrake.OnCnt = 1;
        gBrake.OffCnt = (100 - gComPar.BrakeCoff) / gComPar.BrakeCoff;  // 这样虽然不精确，但是简单
    }
    else    // 50-100
    {
        gBrake.OnCnt = (gComPar.BrakeCoff) / (100 - gComPar.BrakeCoff);
        gBrake.OffCnt = 1;
    }
    
// 
	if(gBrake.OnCnt == 0)
	{
		TurnOffBrake();
		return;
	}

    gBrake.VoltageLim = gVFPar.ovPoint * gVFPar.ovPointCoff;            // 制动电阻动作点就是过压抑制点调整
    
// 判断本次是应该开通还是关断
	if(gUDC.uDC < (gBrake.VoltageLim + 90))	
	{
		gBrake.Flag &= 0x0FFFC;		//清除0、1bit
		gBrake.Cnt = 0;
	}
	else if((gBrake.Flag & 0x01) == 0)      //开通第一拍，置0、1bit
	{
		if(gUDC.uDC > (gBrake.VoltageLim + 110))	
		{
			gBrake.Flag |= 0x03;		
			gBrake.Cnt = 0;
		}
	}
	else        // on or off
	{
		gBrake.Cnt++;
		if((gBrake.Flag & 0x02) == 0)       // off
		{
			if(gBrake.Cnt > gBrake.OffCnt)
			{
				gBrake.Flag |= 0x02;
				gBrake.Cnt = 0;
			}
		}
		else        // bit1 != 0, switch on
		{
			if(gBrake.Cnt > gBrake.OnCnt)
			{
				gBrake.Flag &= 0x0FFFD;
				gBrake.Cnt = 0;
			}
		}
	}

// 开始执行制动电阻的导通和关断
	if((gBrake.Flag & 0x02) == 0x02)        // bit1
	{
		TurnOnBrake();				//...开通		
	}
	else
	{
		TurnOffBrake();				//...关断		
	}
}

