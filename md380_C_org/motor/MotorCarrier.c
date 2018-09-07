/****************************************************************
文件功能：载频和载波周期相关计算,包括调制方式的确定
文件版本:
更新日期： 

****************************************************************/
#include "MotorInclude.h"
#include "MotorEncoder.h"
#include "MotorPwmInclude.h"

// 全局变量定义
FC_CAL_STRUCT			gFcCal;
SYN_PWM_STRUCT			gSynPWM;
ANGLE_STRUCT			gPhase;		//角度结构
DEAD_BAND_STRUCT		gDeadBand;

// 同步调制频率分段表、载波比表、每载波周期步长表
Uint const gSynFreqTable[10] =
{
		75,		113, 	169, 	250, 	355, 
//NC=	80		60		42		30		20
		535, 	854, 	1281, 	1922, 	65535
//NC=	12		9		6		3
};

Uint const gSynNcTable[10] = 
{
		80,		60,		42,		30,		20,
		12,		9,		6,		3,		1
};

Ulong const gSynStepAngleTable[10] = 	//65536*65536/N
{
	53687091,	71582788,	102261126,	143165576,	214748365,	
	357913941,	477218588,	715827883,	1431655765, 4294967295 
};

// 文件内部函数声明
void AsynPWMAngleCal(Ulong);
void SynPWMAngleCal(void);

/************************************************************
计算载波频率(2ms)：
输入:   gBasePar.FcSet， gTemperature.Temp；
输出:   gBasePar.FcSetApply；

1. 载频随温度降低的下限是1K，设定频率低于1K不调整；
2. 机型小于12，载频不大于12K，小于15，载频不大于10K，机型15以上，最大载频8K；
   最小载频0.8K，运行频率小于8Hz，载频最高5KHz；
3. VC运行的载频范围是2K到8K；
4. 载频调整时每40ms调整0.1KHz；
************************************************************/
void CalCarrierWaveFreq(void)
{
	Uint	m_TempLim;
    Uint    m_MaxFc;
    Uint    m_MinFc;

// 确定载波频率
	gFcCal.Time++;

	gFcCal.Cnt += gBasePar.FcSetApply;              //载频低的时候，调整的慢一些
	if(gFcCal.Cnt < 100)				
	{
		return;
	}
	gFcCal.Cnt = 0;
	
    //...此处作FC和温度关系的计算
	m_TempLim = 70;
    if(gInvInfo.InvTypeApply >= 19)
    {
        m_TempLim = 70;
    }

	if((gSubCommand.bit.VarFcByTem != 0) &&         //载频随温度调整功能选择
	   (gMainStatus.RunStep != STATUS_GET_PAR) &&   // 调谐时，载波不调整
	   (gTemperature.Temp > (m_TempLim-5)) &&       //温度大于（限制值-5）度
	   (10 < gBasePar.FcSet))                       //载频随温度降低的下限是1K，设定频率低于1K不调整
	{
		if(gFcCal.Time >= 10000)				    //每20秒调整一次
		{
			gFcCal.Time = 0;
			if(gTemperature.Temp <= m_TempLim)	    //载波频率逐渐回升
			{
				gFcCal.FcBak += 5;
			}
			else if(gTemperature.Temp >= (m_TempLim+5))
			{
				gFcCal.FcBak -= 5;				    //载波频率逐渐降低
				gFcCal.FcBak = (gFcCal.FcBak<10) ? 10 : gFcCal.FcBak;
			}
		}
	}
	else										    //载波频率不需调整情况
	{
		gFcCal.Time = 0;
		gFcCal.FcBak = gBasePar.FcSet;
	}
    gFcCal.FcBak = (gFcCal.FcBak > gBasePar.FcSet) ? gBasePar.FcSet : gFcCal.FcBak;
    //根据当前的过载情况减小载波频率，超过过载的50％后开始降低载波频率，到4KHz
	if(gOverLoad.InvTotal.half.MSW < 16000)
	{ 
		gFcCal.FcLimitOvLoad = gBasePar.FcSet*10;
	}
	else if(gOverLoad.InvTotal.half.MSW > 18000)
	{
		gFcCal.FcLimitOvLoad--;
		if(gFcCal.FcLimitOvLoad < 400)	gFcCal.FcLimitOvLoad = 400;
	}
	if(gFcCal.FcBak > (gFcCal.FcLimitOvLoad/10))
	{
		gFcCal.FcBak = (gFcCal.FcLimitOvLoad/10);
	}
//...开始作FC的最大和最小限制
	m_MinFc = 8;								    //最小0.8KHz载波频率
	m_MaxFc = 120;								    //最大12KHz载波频率
	if(gInvInfo.InvTypeApply > 15)		
	{
		m_MaxFc = 80;							    //大于15机型最大8KHz载波频率
	}
	else if(gInvInfo.InvTypeApply > 12)	
	{
		m_MaxFc = 100;							    //大于12机型最大10KHz载波频率
	}
	if(gMainCmd.Command.bit.ControlMode != IDC_VF_CTL)
	{
		m_MaxFc = (m_MaxFc>80)?80:m_MaxFc;		    //VC运行8KHz最大载波频率限制
		m_MinFc = 20;							    //VC运行2KHz最小载波频率限制
	}
	if(gMainCmd.FreqReal < 800)					
	{
		m_MaxFc = (m_MaxFc>50)?50:m_MaxFc;		    //8Hz以下5KHz最大载波频率限制
	}
    if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
    {
		m_MaxFc = (m_MaxFc>10)?10:m_MaxFc;		    //1140V最大载频1K，最小0.5K
		m_MinFc = 8;							    //2011.5.7 L1082
    }
    else if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
    {
        m_MaxFc = (m_MaxFc>40)?40:m_MaxFc;		    //690V最大载频4K，最小0.8K
		m_MinFc = 8; 
    }
	gFcCal.FcBak = __IQsat(gFcCal.FcBak,m_MaxFc,m_MinFc);


//...开始调整载波频率，每40ms调整0.1KHz
	if(gFcCal.FcBak > gBasePar.FcSetApply)
	{
		gBasePar.FcSetApply ++;
	}
	else if(gFcCal.FcBak < gBasePar.FcSetApply)
	{
		gBasePar.FcSetApply --;
	}

// 同步调制与异步调制的选择
	gSynPWM.AbsFreq = gMainCmd.FreqReal/100;
    // 自动切换
	if((IDC_VF_CTL != gMainCmd.Command.bit.ControlMode)||
	   (gExtendCmd.bit.ModulateType == 0)	|| 
	   (gSynPWM.AbsFreq < gSynFreqTable[0]-10))     // 65Hz
	{
		gSynPWM.ModuleApply = 0;
	}
	else
	{
		if((gSynPWM.ModuleApply == 0) && (gSynPWM.AbsFreq > gSynFreqTable[0]+10))
		{
			gSynPWM.ModuleApply = 1;
			gSynPWM.Index       = 0;
			gSynPWM.Flag        = 0;
		}
		else if((gSynPWM.ModuleApply >= 1) && (gSynPWM.AbsFreq < gSynFreqTable[0]-10))
		{
			gSynPWM.ModuleApply = 0;
		}
	}
    if(gSynPWM.ModuleApply == 0)
    {
        gSynPWM.FcApply = ((Ulong)gBasePar.FcSetApply)<<9;
    }
// 根据运行频率计算输出电压相位角度变化步长 : gPhase.StepPhase
	if(gSynPWM.ModuleApply == 1)		
	{
		SynPWMAngleCal();
	}
	else								
	{		
		AsynPWMAngleCal(gSynPWM.FcApply);
	}
// DPWM 和 CPWM 的选择
    // CPWM或者DPWM调制方式自动切换，遵循280的自动切换原则
	if((gBasePar.FcSetApply <= 15) || ( 1 == gExtendCmd.bit.ModulateType) ||(gCtrMotorType != ASYNC_VF)	)	
	{
		gPWM.PWMModle = MODLE_CPWM;			//中频电机希望全程CPWM调制
	}
	else if( gMainCmd.FreqReal > gPWM.PwmModeSwitchLF + 300) /*CPWM，DPWM切换通过频率点2011.5.7 L1082*/
	{
        gPWM.PWMModle = MODLE_DPWM;
	}
	else if( gMainCmd.FreqReal < gPWM.PwmModeSwitchLF)
	{
        gPWM.PWMModle = MODLE_CPWM;
	}

}

/************************************************************
0.5ms调用:
	根据当前载波频率和运行频率计算载波周期内相位变化步长，
	区分同步调制和异步调制

1. 在VC，SVC控制时使用异步调制；
2. 运行频率小于75Hz使用异步调制；
3. 手动选择异步调制有效；
4. 只有在Vf控制，并且运行频率大于75Hz时，并且手动选择同步调制才切换到同步调制；
5. 75Hz切换时，存在10Hz的滞环；
************************************************************/

/************************************************************
	异步调制情况下根据当前载波频率和运行频率计算载波周期内相位变化步长
Input:    gFCApply  gSpeed.SpeedGivePer
OutPut:   gPhase.StepPhase  gPhase.CompPhase gPWMTc
TC/2 = 250000/Fc  
StepPhase = (TC/2)*f*Maxf*1125900 >> 32
************************************************************/
void AsynPWMAngleCal(Ulong FcApply)
{
	Uint  m_HalfTc,m_AbsFreq,m_Fc;

	Ulong m_LData = (Ulong)PWM_CLOCK_HALF * 10000L * 512L;
	if(FcApply > 61440)
	{
		m_Fc = FcApply>>1;
		m_LData = m_LData>>1;
	}
	else
	{
		m_Fc = FcApply;
	}
	m_HalfTc = (Uint)(m_LData/m_Fc);

	m_AbsFreq  = abs(gMainCmd.FreqSyn);
    m_LData = ((Ullong)m_AbsFreq * (Ullong)gBasePar.FullFreq01 * 439804651L / m_Fc)>>16;

	DINT;
	if(gMainCmd.FreqSyn >= 0)
	{
		gPhase.StepPhase = m_LData;
		gPhase.CompPhase = (m_LData>>15) + COM_PHASE_DEADTIME;
	}
	else
	{
		gPhase.StepPhase = -m_LData;
		gPhase.CompPhase = -(m_LData>>15) - COM_PHASE_DEADTIME;
	}
    
	gPWM.gPWMPrd = m_HalfTc;
	EINT;
}

/************************************************************
	同步调制下计算载波周期和每载波周期的角度变化步长
************************************************************/
void SynPWMAngleCal(void)
{
	Uint  m_AbsFreq,m_HalfTc,m_Index;
	Ulong m_Fc,m_Long,m_LData;

	m_Index = 0;
	while(gSynPWM.AbsFreq >= gSynFreqTable[m_Index]) 	
	{
		m_Index++;
		if(m_Index >= 10)	break;
	}

	m_Index--;
	if((m_Index < gSynPWM.Index) && 
	   (gSynPWM.AbsFreq <= gSynFreqTable[gSynPWM.Index]-10))
	{
		gSynPWM.Flag  = 1;					//表示频率减小、载波频率突然增加的过渡过程
		gSynPWM.Index = m_Index;
	}
	else if((m_Index > gSynPWM.Index) && 
	        (gSynPWM.AbsFreq > gSynFreqTable[gSynPWM.Index+1]+10))
	{
		gSynPWM.Flag  = 2;					//表示频率增加、载波频率突然减小的过渡过程
		gSynPWM.Index = m_Index;
	}

	m_AbsFreq  = abs(gMainCmd.FreqSyn);
	m_Long = ((Ullong)gBasePar.FullFreq01 * (Ullong)m_AbsFreq)>>7;
	m_Long = ((Ullong)m_Long * (Ullong)gSynNcTable[gSynPWM.Index] / 100)>>8;
    
	m_Fc = (((Ullong)m_Long<<9))/100;			//当前载波频率

	if(gSynPWM.Flag == 1)
	{
		gSynPWM.FcApply += 100;				//每载波周期变化约0.02KHz载波频率
		if(gSynPWM.FcApply > m_Fc)
		{
			gSynPWM.Flag = 0;
		}
	}
	else if(gSynPWM.Flag == 2)
	{
		gSynPWM.FcApply -= 100;
		if(gSynPWM.FcApply < m_Fc)
		{
			gSynPWM.Flag = 0;
		}
	}
	else
	{
		gSynPWM.FcApply = m_Fc;
	}

	if(gSynPWM.Flag != 0)
	{
		AsynPWMAngleCal(gSynPWM.FcApply);
		return;
	}
    m_LData = (Ulong)PWM_CLOCK_HALF * 1000000L;
	m_HalfTc = m_LData/m_Long;

	DINT;
	if(gMainCmd.FreqSyn >= 0)
	{
		gPhase.StepPhase = gSynStepAngleTable[gSynPWM.Index];
		gPhase.CompPhase = (gSynStepAngleTable[gSynPWM.Index]>>15) + COM_PHASE_DEADTIME;
	}
	else
	{
		gPhase.StepPhase = -gSynStepAngleTable[gSynPWM.Index];
		gPhase.CompPhase = -(gSynStepAngleTable[gSynPWM.Index]>>15) - COM_PHASE_DEADTIME;
	}

	gPWM.gPWMPrd = m_HalfTc;
	EINT;
}

/*************************************************************
	随机PWM处理，使载波周期和输出相位生效

随机PWN启用的条件是：
   异步调制 + 载频1k-6k之间 + Vf控制 + 手动选择启用；
*************************************************************/
void SoftPWMProcess(void)
{
	Uint  m_Coff;

	if((IDC_VF_CTL != gMainCmd.Command.bit.ControlMode) ||            // 只有Vf才启动随机PWM		
        (gPWM.SoftPWMTune == 0) || (gSynPWM.ModuleApply >= 1) ||     // 同步调制下不启用随机PWM     
	   (gBasePar.FcSetApply < 10) || (gBasePar.FcSetApply > 60)) // 载波频率1K～6K之间有效
	{
	    gPWM.gPWMPrdApply = gPWM.gPWMPrd;	        		
   	    gPhase.StepPhaseApply = gPhase.StepPhase;	
		return;
	}
    
    m_Coff = 3277;
    if( 30 < gBasePar.FcSetApply )
    {
        m_Coff = 1966;
    }
	m_Coff = (gPWM.SoftPWMTune * m_Coff)>>4;    //随机PWM的调节宽度10对应50%;大于3K时，则对应30%
	gPWM.SoftPWMCoff = gPWM.SoftPWMCoff + 29517 * GetTime();    //产生一个Q15格式的随机数
	m_Coff = 4096 + (((long)gPWM.SoftPWMCoff * (long)m_Coff)>>15);

 	gPWM.gPWMPrdApply = ((Ulong)gPWM.gPWMPrd * (Ulong)m_Coff)>>12;
    gPhase.StepPhaseApply = (Ulong)(gPhase.StepPhase>>12) * (Ulong)m_Coff;
}


/************************************************************
	计算输出电压的相位角：VF情况下直接累加
************************************************************/
void CalOutputPhase(void)
{ 
	//if(DC_CONTROL == gCtrMotorType) 	//直流制动
	//{
	    // 电压幅度在2ms计算，异步机无需考虑电压相位；
	    // 同步机暂时按M轴方向发电压， 电压角度增量为0；
	    // gPhase.OutPhase 不更新
		//return;
	//}

    // 同步机获取转子位置，进行转子磁场定向
    if(MOTOR_TYPE_PM == gMotorInfo.MotorType)
    {
		if(0 == gPGData.PGMode)                         // 增量式 uvw, Abz
		{
			SynCalRealPos();                            // 同步机辨识的时候不更新 gIPMPos.RotorPos
			// 非空载辨识时:
			            
        	if((gMainStatus.RunStep != STATUS_GET_PAR) ||
        	   (gMainStatus.RunStep == STATUS_GET_PAR && TUNE_STEP_NOW != PM_EST_NO_LOAD)        	   
        	   )
        	{
        	    if(STATUS_IPM_INIT_POS != gMainStatus.RunStep)
                {   
            		gIPMPos.RotorPos = gIPMZero.FeedPos;            // 空载辨识的时候gIPMPos.RotorPos自动累加        		
                    gIPMPos.ABZ_RotorPos_Ref = gIPMZero.FeedABZPos;
               }
            }
		}
		else if(gPGData.PGType == PG_TYPE_RESOLVER)             // 非增量式, 旋变
		{
			if(gMainStatus.RunStep != STATUS_GET_PAR)       //同步机空载辨识不更新gIPMPos.RotorPos   
			{
				gIPMPos.RotorPos = gRotorTrans.RTPos + gIPMPos.RotorZero + gRotorTrans.PosComp;
			}
            else if(gMainStatus.RunStep == STATUS_GET_PAR && TUNE_STEP_NOW != PM_EST_NO_LOAD)
            {            
                gIPMPos.RotorPos = gRotorTrans.RTPos + gPmParEst.EstZero;// + gRotorTrans.PosComp;
            }
            else            // 旋变空载辨识时， 角度由辨识累加
            {
                ;   // ??
            }
		}
        
        // 转子磁场定向
        if(TUNE_STEP_NOW == PM_EST_BEMF)           // 反电动势辨识， 由功能给定转速计算相位增加量
        {
            gPhase.IMPhase += gPhase.StepPhaseApply;
        }
        else
        {
            gPhase.IMPhase  = ((long)gIPMPos.RotorPos) << 16;	// 正常运行
        }
	}

    if((gMotorInfo.MotorType != MOTOR_TYPE_PM) ||  // 异步机相位角累加		
        (gCtrMotorType == SYNC_VF || gCtrMotorType == SYNC_SVC))    // 同步机VF 和SVC
	{
		gPhase.IMPhase += gPhase.StepPhaseApply;
	}
    
	gPhase.OutPhase = (int)(gPhase.IMPhase>>16) + gOutVolt.VoltPhaseApply;
}

/*************************************************************
	电流死区补偿极性判断
*************************************************************/
void CalDeadBandComp(void)
{
	int   phase,m_Com;
	//int	  DetaPhase;

	if(gMainCmd.FreqReal <= 40000)	    // 400.00Hz
	{
		m_Com = gDeadBand.Comp;
	}
	else 
	{
		m_Com = (int)(((long)gDeadBand.Comp * (long)(gMainCmd.FreqReal - 40000))>>15);
	}
   
	if((gMotorInfo.MotorType == MOTOR_TYPE_PM) && (gDeadBand.InvCurFilter < 512))
	{
	    m_Com = ((long)m_Com * gDeadBand.InvCurFilter)>>9;
	}
    
	if((gMainCmd.Command.bit.StartDC == 1) || (gMainCmd.Command.bit.StopDC == 1))
        
	{
        m_Com = 0;
	}
    
    gIAmpTheta.ThetaFilter = gIAmpTheta.Theta;/*死区补偿为AD中补偿，MD320 0.5MS补偿取消，角度滤波取消 2011.5.7 L1082 */

    
	phase = (int)(gPhase.IMPhase>>16) + gIAmpTheta.ThetaFilter + gPhase.CompPhase + 16384;
 #if 1
   gDeadBand.CompU = (phase <= 0) ? m_Com : (-m_Com);

	phase -= 21845;
	gDeadBand.CompV = (phase <= 0) ? m_Com : (-m_Com);

	phase -= 21845;
	gDeadBand.CompW = (phase <= 0) ? m_Com : (-m_Com);
 
#else        
    if(phase <= 0)
	{
		if((phase >= -32504)&&(phase <= -264))//32404,364
		{
			gDeadBand.CompU = m_Com;
		}
		else
		{
			gDeadBand.CompU = 0;
		}
	}
	else
	{
		if((phase <= 32504)&&(phase >= 264))
		{
			gDeadBand.CompU = (-m_Com);
		}
		else
		{
			gDeadBand.CompU = 0;
		}
	}

	phase -= 21845;

	if(phase <= 0)
	{
		if((phase >= -32504)&&(phase <= -264))
		{			
			gDeadBand.CompV = m_Com;
		}
		else
		{
			gDeadBand.CompV = 0;
		}
	}
	else
	{
		if((phase <= 32504)&&(phase >= 264))
		{			
			gDeadBand.CompV = (-m_Com);
		}
		else
		{
			gDeadBand.CompV = 0;
		}
	}
	

	phase -= 21845;

	if(phase <= 0)
	{
		if((phase >= -32504)&&(phase <= -264))
		{			
			gDeadBand.CompW = m_Com;			
		}
		else
		{
			gDeadBand.CompW = 0;
		}
	}
	else
	{
		if((phase <= 32504)&&(phase >= 264))
		{
			gDeadBand.CompW = (-m_Com);			
		}
		else
		{
			gDeadBand.CompW = 0;
		}
	}
 #endif
 }
#if 0
// 异步机SVC 的死区补偿
void ImSvcDeadBandComp()
{
    ;
}

void ImFvcDeadBandComp()
{
    ;
}

void PmFvcDeadBandComp()
{
    ;
}
#endif

