/****************************************************************
文件功能：VF控制相关程序，包括VF曲线计算、转矩提升、转差补偿、抑制振荡等处理
文件版本： 
最新更新： 
	
*************************************************************/

#include "MotorVFInclude.h"
#include "MotorInclude.h"

// 全局变量定义
VF_INFO_STRUCT			gVFPar;		//VF参数
VF_AUTO_TORQUEBOOST_VAR gVFAutoVar;

VF_WS_COMP_STRUCT		gWsComp;
VAR_AVR_STRUCT			gVarAvr;
//VF_CURRENT_CONTROL      gVfCsr;

OVER_SHOCK_STRUCT		gVfOsc;	//抑制振荡用结构变量
THREE_ORDER_FILTER      gOscAmp;

// Vf 失速控制相关变量
VF_OVER_UDC_DAMP        gVfOverUdc;
OVER_CURRENT_DAMP2	    gOvCur2;
VF_FREQ_DEAL            gVfFreq2;

OVER_CURRENT_DAMP	    gOvCur;
OVER_UDC_CTL_STRUCT		gOvUdc;

HVF_OSC_DAMP_STRUCT     gHVfOscDamp;
HVF_OSC_JUDGE_INDEX     gHVfOscIndex;           // HVf 振荡系数的计算
HVF_DB_COMP_OPT         gHVfDeadBandCompOpt; 
MT_STRUCT				gHVfCur;

// 文件内部函数声明
void CalTorqueUp(void);
void ResDropComp(MT_STRUCT * );

int VfOverCurDeal2(int step);
int VfOverUdcDeal2(int step);

void VFOVUdcLimit(void);

/************************************************************
函数输入:无
函数输出:无
调用位置:上电初始化和运行之前
调用条件:无
函数功能:初始化运行中用到的变量
************************************************************/
void VfVarInitiate(void)  //VF运行变量初始化函数
{
    gVFPar.FreqApply = 0;
    gVfFreq2.freqSet = 0;
    
	if(gMainCmd.FreqDesired > 0)		//根据目标频率方向确定初始相位角
	{
		gOutVolt.VoltPhaseApply = 16384;
	}
	else
	{
		gOutVolt.VoltPhaseApply = -16384;
	}		
	gPhase.OutPhase = (int)(gPhase.IMPhase>>16) + gOutVolt.VoltPhaseApply;

//VF自动转矩提升变量初始化
	gVFAutoVar.VfCurrentIs = 0; 
	gVFAutoVar.VfReverseAngle = 0;
    gVFAutoVar.AutoTorquePID.Total = 0;
    gVFAutoVar.AutoTorquePID.Out   = 0;
    gVFAutoVar.VfReverseVolt = 0;
    gVFAutoVar.VfTorqueEnableTime = 0;
    gWsComp.Coff = gVFPar.VFWsComp;
    gWsComp.DelayTime = 0;
    gWsComp.WsCompMT.M = 0;
    gWsComp.WsCompMT.T = 0;
    gWsComp.WsCompMTApply.M = 0;
    gWsComp.CompFreq = 0;
    gVFAutoVar.DestinationVolt = 0;
    gOutVolt.detVfVoltUp = 0;
    gVFAutoVar.VfAutoTorqueEnable = 0;


    gOscAmp.InData1 =   gIMTQ12.M;
    gOscAmp.Indata2 =   gIMTQ12.M;
    gOscAmp.Indata3 =   gIMTQ12.M;   
    gOscAmp.OutData1 =  gIMTQ12.M;
    gOscAmp.OutData2 =  gIMTQ12.M;
    gOscAmp.OutData3 =  gIMTQ12.M;

    gVfOsc.TimesNub = 0;
    gVfOsc.IO = 0;
    gVfOsc.ShockDecrease = 0;

    gVfFreq2.preSpdFlag = gMainCmd.Command.bit.SpeedFlag; 
}

/************************************************************
	减速中持续过电压处理
************************************************************/
void VFOVUdcLimit(void)
{
	int	m_Mid;

	m_Mid = ((Ulong)gOvUdc.Limit * 3932L)>>12;			//0.96
	
	if((gUDC.uDCFilter > m_Mid) && (gOvUdc.StepApply <= 0) )
	{
		gOvUdc.OvUdcLimitTime++;
		if(gOvUdc.OvUdcLimitTime > 10000)
		{
			gOvUdc.OvUdcLimitTime = 10000;
		}
	}
	else
	{
		gOvUdc.OvUdcLimitTime = 0;
	}
}

/************************************************************
函数输入:标么值频率Q15,多点VF使用频率实际值
函数输出:输出电压Q12,
调用位置:运行状态下2ms循环
调用条件:
函数功能:根据给定频率和VF曲线，计算输出电压
************************************************************/
int CalOutVotInVFStatus(int freq)
{
	Uint m_Freq2,m_MotorFreq2;
	int  m_AbsFreq;
    int  mVolt;
	long m_DetaFreq,m_LowFreq,m_HighFreq,m_LowVolt,m_HighVolt;
    long m_VFLineFreq1,m_VFlineFreq2,m_VFLineFreq3,m_Frequency;
    
	m_AbsFreq = abs(freq);

	//gOutVolt.VoltPhase = 0;    
    gVFAutoVar.DestinationVolt = 0;

//...恒转矩区域

// 线性VF曲线
	if((gVFPar.VFLineType == 0) || (gMainStatus.RunStep == STATUS_SPEED_CHECK))
	{
		mVolt = ((Ulong)m_AbsFreq * 4096L)/gMotorInfo.FreqPer;
	}
//多点VF曲线， 不用转矩提升
	else if(gVFPar.VFLineType == 1)	
	{
	    m_VFLineFreq1 = (long)gVFPar.VFLineFreq1 * (long)gMainCmd.pu2siCoeff;
        m_VFlineFreq2 = (long)gVFPar.VFLineFreq2 * (long)gMainCmd.pu2siCoeff;
        m_VFLineFreq3 = (long)gVFPar.VFLineFreq3 * (long)gMainCmd.pu2siCoeff;
        m_Frequency   = (long)gMotorInfo.Frequency * (long)gMainCmd.pu2siCoeff;    

		if(gMainCmd.FreqReal < m_VFLineFreq1)
		{
			m_LowFreq  = 0;	
			m_HighFreq = m_VFLineFreq1;
			m_LowVolt  = gVFPar.VFTorqueUp;	
			m_HighVolt = gVFPar.VFLineVolt1;
		}
		else if(gMainCmd.FreqReal < m_VFlineFreq2)
		{
			m_LowFreq  = m_VFLineFreq1;
			m_HighFreq = m_VFlineFreq2;
			m_LowVolt  = gVFPar.VFLineVolt1;
			m_HighVolt = gVFPar.VFLineVolt2;
		}
		else if(gMainCmd.FreqReal < m_VFLineFreq3)
		{
			m_LowFreq  = m_VFlineFreq2;
			m_HighFreq = m_VFLineFreq3;
			m_LowVolt  = gVFPar.VFLineVolt2;
			m_HighVolt = gVFPar.VFLineVolt3;
		}
		else
		{
			m_LowFreq  = m_VFLineFreq3;
			m_HighFreq = m_Frequency;//多点VF曲线的最后一段以电机参数结尾
			m_LowVolt  = gVFPar.VFLineVolt3;
			m_HighVolt = 1000;
		}
		m_DetaFreq = gMainCmd.FreqReal - m_LowFreq;
		m_HighFreq = m_HighFreq - m_LowFreq;
		m_HighVolt = m_HighVolt - m_LowVolt;
		m_LowVolt  = (((long)m_LowVolt)<<12)/1000;
		m_HighVolt = (((long)m_HighVolt)<<12)/1000;	//该段的电压范围

		mVolt = (((long)m_HighVolt * (long)m_DetaFreq)/m_HighFreq) + m_LowVolt;
        return (mVolt);     // 多点vf不用转矩提升
	}	
//平方VF曲线
	else if(gVFPar.VFLineType == 2)				
	{
		m_Freq2 = ((Ulong)m_AbsFreq * (Ulong)m_AbsFreq)>>15;
		m_MotorFreq2  = ((Ulong)gMotorInfo.FreqPer * (Ulong)gMotorInfo.FreqPer)>>15;
        
		mVolt = ((Ulong)m_Freq2 * 4096L)/m_MotorFreq2;
	}
// 小平方Vf曲线
    else if((gVFPar.VFLineType >= 3) && (gVFPar.VFLineType <= 8)) // 2, 3, 4, 5
    {
        Uint posFt, posEnd;
        Uint voltFt, voltEnd;
        Uint16 ptV;
        Uint detFrq;

        ptV = gVFPar.VFLineType - 3;
        if(ptV == 3) ptV = 2;   // 1.6
        if(ptV == 5) ptV = 3;   // 1.8
        
        //采用线性拟合的方式计算指数曲线，将0~~1分为128段
        m_Freq2 = (Ulong)m_AbsFreq * 4096 / gMotorInfo.FreqPer;
        
        posFt = m_Freq2>>5;                 //4096分成128段，每段数值为32
        posFt = (posFt < 127) ? posFt : 127;

        posEnd = posFt + 1;
        detFrq = m_Freq2 - (posFt<<5);
        
        voltFt  = gExponentVf[ptV][posFt];    // 2代表1.2次方曲线
        voltEnd = gExponentVf[ptV][posEnd];
        
        mVolt = ((detFrq * (voltEnd - voltFt)) >> 5) + voltFt;
    }
// VF分离电压生成
    else if(gVFPar.VFLineType == 10 || gVFPar.VFLineType == 11)
    {
        mVolt = ((Ulong)gOutVolt.vfSplit << 12) / gMotorInfo.Votage;
    }

// 根据VF类型进行Vf曲线的边界处理
    
    if(m_AbsFreq == 0)          // 0频率输出电压为0， VF分离也是如此
	{
		mVolt = 0;
		return (mVolt);		
	}
    else if(m_AbsFreq >= gMotorInfo.FreqPer)
	{
	    if(gVFPar.VFLineType < 10)                  // 电机控制恒功率区
        {   
		    mVolt = (mVolt > 4096) ? 4096 : mVolt;  
        }
        else// 不然Vf分离的时候会出问题
        {
            mVolt = (mVolt < gOutVolt.MaxOutVolt) ? mVolt : gOutVolt.MaxOutVolt;
        }                        
		return (mVolt);		
	}	
    
    //转矩提升处理
	//CalTorqueUp();		// generate gOutVolt.detVfVoltUp		
	//mVolt += gOutVolt.detVfVoltUp;
	return (mVolt);
}

/*************************************************************
	转矩提升处理，由I*R计算提升电压，提升电压和VF计算电压矢量加
*************************************************************/
void CalTorqueUp(void)
{
	int m_DetaFreq, m_VoltUp, m_ZeroUp, m_MaxFreqUp;
    
// some case Vf_curve needn't torque up compensation
    if((gVFPar.VFLineType == 1) || (gVFPar.VFLineType >= 10) ||
        //(gMainStatus.RunStep == STATUS_SPEED_CHECK) ||
        (gMainStatus.RunStep == STATUS_GET_PAR))
    {
        gOutVolt.detVfVoltUp = 0;
    }
    else
    {
    m_VoltUp = 0;
	m_ZeroUp = (((Ulong)gVFPar.VFTorqueUp)<<12)/1000; //0Hz时候的提升电压
	m_MaxFreqUp = (((Ulong)gVFPar.VFTorqueUpLim)<<15)/gBasePar.FullFreq; //标么值表示的转矩提升截至频率   
	m_DetaFreq = m_MaxFreqUp - abs(gMainCmd.FreqSyn);
    if(( m_DetaFreq > 0 ) && ( 0 == gExtendCmd.bit.SpeedSearch )) //转速跟踪时不作远靥嵘?
    {
	    if((gVFPar.VFTorqueUp == 0)&&(gVFPar.VFOvShock == 0))		//自动转矩提升,在震荡抑制作用时无效 2011.5.14 L1082
	    {
            if( gMainCmd.FreqDesiredReal > 40 )  //设定频率低于0.4HZ，转矩提升无效
			{
                gVFAutoVar.DestinationVolt = gOutVolt.Volt;
    	    	m_VoltUp = (int)(gVFAutoVar.AutoTorquePID.Out >>16);  //电压补偿滞后0.5ms，是否会有影响澹?
			}
            else
            {
                gVFAutoVar.VfTorqueEnableTime = 0;
                m_VoltUp = 0;
            }
	    }
    	else							//手动转矩提升
	    {
            m_VoltUp = ((Ulong)m_DetaFreq * (Ulong)m_ZeroUp)/(Uint)m_MaxFreqUp; 
		}
      }  
    gOutVolt.detVfVoltUp = m_VoltUp; 
    }
    gOutVolt.Volt = gOutVolt.Volt + gOutVolt.detVfVoltUp;
}

/************************************************************
     VF自动转矩提升和转差补偿公共变量计算
************************************************************/
void VFWsTorqueBoostComm(void)
{
    int    m_CurrentPro,m_AngleCos,m_ResistanceVolt;
    int    m_RVcos;
    long   m_lTempVar;
    int    m_iTempVar1,m_iTempVar2;
    MT_STRUCT_Q24  m_NewMT;

    m_CurrentPro = gLineCur.CurPer;//gIAmpTheta.Amp    
    m_iTempVar1  = abs(gIMTQ12.T);
    gVFAutoVar.VfCurrentIs = Filter4(m_CurrentPro, gVFAutoVar.VfCurrentIs);

    if( m_iTempVar1 > gVFAutoVar.VfCurrentIs )
	{
        m_iTempVar1 = gVFAutoVar.VfCurrentIs;
	}
    m_AngleCos = (((long)m_iTempVar1)<<14) / gVFAutoVar.VfCurrentIs;  //计算功率因数角的余弦
    m_lTempVar =  ( 0x4000L<<14) - (long)m_AngleCos * (long)m_AngleCos;  
    gVFAutoVar.VfAngleSin = qsqrt(m_lTempVar);  
    
    m_ResistanceVolt = ((Ulong)gMotorExtPer.R1 * (Ulong)gVFAutoVar.VfCurrentIs)>>16;  //计算定子电阻上的压降,Q16表示
    m_RVcos = ((long)m_ResistanceVolt * (long)m_AngleCos)>>14;
    gVFAutoVar.VfRIsSinFai = ((long)m_ResistanceVolt * (long)gVFAutoVar.VfAngleSin)>>14;
    gVFAutoVar.VfRVCosFai  = gOutVolt.VoltApply - 20 - m_RVcos; //输出电压标么值减去20，做为修正
    
    if(0 > gVFAutoVar.VfRVCosFai)
	{
        gVFAutoVar.VfRVCosFai = 0;
    }

    m_iTempVar2 = atan(gVFAutoVar.VfRVCosFai,gVFAutoVar.VfRIsSinFai);    
    gVFAutoVar.VfReverseAngle = Filter128(m_iTempVar2,gVFAutoVar.VfReverseAngle);//计算电压修正角
    m_iTempVar2 = atan( abs(gIMTQ12.M), abs(gIMTQ12.T) );
    if( gVFAutoVar.VfReverseAngle > m_iTempVar2 )
	{
        gVFAutoVar.VfReverseAngle = m_iTempVar2;  //计算的修正角大于功率因数角的余角，就使用功率因数角余角代替
	}

    gWsComp.Coff = gVFPar.VFWsComp;
    if( gWsComp.Coff > 1400 )
    {                           //转差补偿增益大于1400，直接用MT轴电流计算转差补偿量
        gWsComp.Coff = gVFPar.VFWsComp - 500;
        gWsComp.WsCompMT.M = abs( gIMTQ12.M );
        gWsComp.WsCompMT.T = abs( gIMTQ12.T );
    }
    else
    {                          //转差补偿增益低于1400，用修正后的角度重新计算MT轴电流
        m_iTempVar1 = (int)(gPhase.IMPhase>>16) + gVFAutoVar.VfReverseAngle; 
        if( gMainCmd.FreqSet < 0 )
		{
            m_iTempVar1 = (int)(gPhase.IMPhase>>16) - gVFAutoVar.VfReverseAngle;
        }
        
        AlphBetaToDQ((ALPHABETA_STRUCT *)&gIAlphBeta, m_iTempVar1, (MT_STRUCT_Q24 *)&m_NewMT);
        gWsComp.WsCompMT.T = Filter128( abs(m_NewMT.T>>12), gWsComp.WsCompMT.T);
        if( gLineCur.CurPer < 4096 )	//如果电流大于额定电流，不再计算励磁电流
        {       
            gWsComp.WsCompMT.M = Filter8( abs(m_NewMT.M>>12), gWsComp.WsCompMT.M);            
        }
    }
}

/************************************************************
函数输入:根据VF曲线计算的输出电压，Q12;MT坐标变换后的电流,Q12
函数输出:VF低频电压修正量，Q12格式
调用位置:运行状态下2ms循环
调用条件:
函数功能:VF自动转矩提升，根据电流计算电压补偿量
************************************************************/
void VFAutoTorqueBoost(void)
{
    long  m_lTempVar1,m_lVoltDeta;
    int   m_iEsVolt,m_iEsnVolt,m_LowerLimit,m_TempVolt;
    
    m_lTempVar1  = (long)gVFAutoVar.VfRIsSinFai * (long)gVFAutoVar.VfRIsSinFai;
    m_lTempVar1 += (long)gVFAutoVar.VfRVCosFai * (long)gVFAutoVar.VfRVCosFai;
    m_iEsVolt    = qsqrt(m_lTempVar1);   //修正后的输出电压减去定子电阻压降的余弦，与定子电阻压降的正弦的平方根
    m_iEsnVolt   = ((long)gOutVolt.VoltApply * (long)gVFAutoVar.VfAngleSin)>>14;//不加修正的输出电压乘以功率因数角的正弦   				
    if(15 >= gInvInfo.InvTypeApply)        m_LowerLimit = 50;//根据机型计算PI调节器的下限，避免起动缓慢。
    else if(27 >= gInvInfo.InvTypeApply)   m_LowerLimit = 35;
    else                                     m_LowerLimit = 0;

    m_LowerLimit = (long)(1000 - gMainCmd.FreqReal) * (long)m_LowerLimit / 1000; //类似手动提升，按频率计算对应下限，截至频率认为是10HZ

    m_TempVolt = m_iEsnVolt;  			//根据当前运行频率和电阻，决定反电动势计算方式
    if(27 < gInvInfo.InvTypeApply) m_TempVolt = m_iEsVolt;  //大功率启动时第一个电流峰值很大，容易报过载 
    
    if(gMainCmd.FreqReal > 150)         //  1.5Hz以上, 下限为零；
    {
        m_LowerLimit = 0;				
        m_TempVolt = m_iEsVolt;

        if(gMainCmd.FreqReal < 2000)	//大于1.5HZ,小于20HZ
        {       
            m_TempVolt = m_iEsnVolt;
            if(gMotorExtInfo.R1 < (2000 * gMotorExtInfo.UnitCoff))//定子电阻小于2欧姆
            {   
                m_TempVolt = m_iEsVolt;
                if(gMainCmd.FreqReal < 950)//定子电阻小于2欧姆，运行频率低于9.5Hz
                {   
                    m_TempVolt = (m_iEsnVolt + m_iEsVolt) >>1;
                }            
            }
        }
    }
    
    if((gMotorExtInfo.R1 < (2000 * gMotorExtInfo.UnitCoff)) && (350 > gVFAutoVar.VfTorqueEnableTime))             
    {      
        gVFAutoVar.VfTorqueEnableTime++;
        m_TempVolt = m_iEsVolt;
    }
    m_lVoltDeta = ((long)(gVFAutoVar.DestinationVolt - m_TempVolt))<<5; 	// 避免精度损失，用Q5表示偏差
    m_TempVolt = __IQsat(m_lVoltDeta,32767,-32767);
    gVFAutoVar.VfReverseVolt = Filter32(m_TempVolt, gVFAutoVar.VfReverseVolt);
    
    if(19 < gInvInfo.InvTypeApply)
    {
       gVFAutoVar.AutoTorquePID.Deta = (long)gVFAutoVar.VfReverseVolt>>5;
    }
    else if(13 < gInvInfo.InvTypeApply)
    {
       gVFAutoVar.AutoTorquePID.Deta = ((long)gVFAutoVar.VfReverseVolt>>3) & 0xFFFC;
    }
    else
    {
       gVFAutoVar.AutoTorquePID.Deta = ((long)gVFAutoVar.VfReverseVolt>>1) & 0xFFF0;
    }
    gVFAutoVar.AutoTorquePID.Max  = 1228 ; 			//30% Torque Boost
    gVFAutoVar.AutoTorquePID.Min  = m_LowerLimit;
    gVFAutoVar.AutoTorquePID.KP   = 56;  		//PID函数实现上与老版本有所不同
    gVFAutoVar.AutoTorquePID.KI   = 49;
    gVFAutoVar.AutoTorquePID.QP = 0;
    gVFAutoVar.AutoTorquePID.QI = 0;
    PID((PID_STRUCT *)&gVFAutoVar.AutoTorquePID); 
 }


/************************************************************
	抑制振荡处理
************************************************************/
/*void OverShockControl(void)
{
    int m_DelayTime,m_DestCurrent;
    int gainOsc;

    gainOsc = gVFPar.VFOvShock;
    gVfOsc.IMFilter = gIMTQ12.M;
    
    gVfOsc.TimesNub++;
    if( 2000 < gVfOsc.TimesNub )
    {
        gVfOsc.TimesNub = 2001;
    }

//抑制振荡模式为0或2时，DPWM调制时取消抑制振荡
    if((gVfOsc.oscMode == 0) && (MODLE_DPWM == gPWM.PWMModle)) 
    {
        gVfOsc.ShockDecrease++;
        if(800 < gVfOsc.ShockDecrease)
        {
            gVfOsc.ShockDecrease = 808;
        }
        
        m_DelayTime = gVfOsc.ShockDecrease>>3;
        
        if(gVFPar.VFOvShock <= m_DelayTime)
        {
            gainOsc = 0;
        }
        else
        {
            gainOsc = gVFPar.VFOvShock - m_DelayTime;
        }
    }
    else
    {
        gVfOsc.ShockDecrease = 0;
    }

// 无需抑制振荡
	if((gainOsc == 0) || (gMainCmd.FreqReal <= 50) )//|| speed_DEC) // dec    // 0.50Hz
	{										
		gVfOsc.pid.Out -= gVfOsc.pid.Out>>5;
		gVfOsc.pid.Total = 0;
		return;
	}
    gVfOsc.pid.KP   = (gainOsc<<3);     // 280xp版本是左移7位， 由于PID函数中有左移4位
	gVfOsc.pid.KI   = 0;			    //没有积分
	gVfOsc.pid.KD   = 0;
    gVfOsc.pid.QP = 0;
    gVfOsc.pid.QI = 0;
	gVfOsc.pid.Max  = (gainOsc<<2)+100;
	gVfOsc.pid.Min  = -gVfOsc.pid.Max;
    if(gVfOsc.TimesNub < 1500) //避免大功率低速运行时电流始终嵌位在零
    {
        gVfOsc.IO = Filter256(gMotorExtPer.I0, gVfOsc.IO);
    }
//抑制振荡模式为1或2或3时，依赖设定的空载电流
    else// if(( 1500 < gVfOsc.TimesNub ) && (gVfOsc.oscMode >= 1))
    {
        gVfOsc.IO = gMotorExtPer.I0;
    }

	gVfOsc.pid.Deta = gVfOsc.IO - gVfOsc.IMFilter;    
	PID((PID_STRUCT *)&gVfOsc.pid);
    gVfOsc.OscVolt = gVfOsc.pid.Out >> 16; 
	gOutVolt.Volt += gVfOsc.OscVolt;
    if(gVFPar.FreqApply == 0)       // 输出频率为0, 不包含转差补偿
    {
        gOutVolt.Volt  = 0;         // disabel torque-up part and osc-damp part
    }
	gOutVolt.Volt = __IQsat(gOutVolt.Volt, 32767, 0);
    
}
*/
/************************************************************
	转差补偿处理
************************************************************/
void VFWSCompControl(void)
{
    int m_WsWindage, m_WsCurrentIm;

    m_WsWindage = 0;
    if(( gMainCmd.FreqDesiredReal < 40  ) ||
        ( 16 < gInvInfo.InvTypeApply)) 	//设定频率小于0.4Hz,或者机型大于16
 	{	
        gWsComp.CompFreq = 0;
        gMainCmd.FreqSyn = gMainCmd.FreqSetApply;
        return;
	}

    if(speed_CON && (0 != gVFPar.VFWsComp)) 
    {
        if( gWsComp.DelayTime > 100 )
        {
            gWsComp.WsCompMTApply.T = abs(gWsComp.WsCompMT.T);

            if(( 0 == gVFPar.VFOvShock ) && ( 20 <= gBasePar.FcSetApply ))
            {
                gWsComp.WsCompMTApply.M = Filter16(gWsComp.WsCompMT.M, gWsComp.WsCompMTApply.M);
                m_WsCurrentIm = abs(gWsComp.WsCompMTApply.M);
            }
            else
    		{
                m_WsCurrentIm = gMotorExtPer.I0;  //抑制振动有效，或者载频低于2K，励磁电流使用空载电流
            }

            if( 0 == m_WsCurrentIm )	m_WsCurrentIm = 1;
                   
            m_WsWindage = (3038L) * (long)gWsComp.WsCompMTApply.T / (long)m_WsCurrentIm;
            m_WsWindage = (long)m_WsWindage * (long)gWsComp.Coff / 1000L;
            if( m_WsWindage > 0x2000 )	m_WsWindage = 0x2000; 
            if( m_WsWindage < 0 )		m_WsWindage = 0;//最大补偿两倍额定转差
                    
            m_WsWindage = ( (long)m_WsWindage * (long)gMotorExtPer.RatedComp )>>12;
        }
        else	
        {
        	gWsComp.DelayTime++;   //稳态后需要延时0.2s开始转差补偿 
	    }
                      
    }
    else
    {
    	gWsComp.DelayTime = 0;  //加减速过程中，不加转差补偿
    }

    gWsComp.FilterCoff = 100;    //200ms滤波
    if(( (gWsComp.CompFreq>>16) + 2 ) < m_WsWindage ) //滤波允许偏差为2
    {
        gWsComp.CompFreq = (( (long)m_WsWindage <<16 ) - gWsComp.CompFreq ) / (long)( gWsComp.FilterCoff + 1 ) + gWsComp.CompFreq;
    }
    else if(( (gWsComp.CompFreq>>16) - 2 ) >m_WsWindage )
    {
       gWsComp.CompFreq = ( (long long)gWsComp.CompFreq - ( (long long)m_WsWindage <<16 )) * (long long)gWsComp.FilterCoff / 
                             (long)( gWsComp.FilterCoff + 1) + ((long)m_WsWindage <<16);
    }
    else
    {
       gWsComp.CompFreq = ((long)m_WsWindage)<<16;
    }
}

/************************************************************
VF过励磁理：
	减速过程中，已经计算出调制系数后，根据母线电压放大调制系数
输入变量: 调制系数 gRatio;
输出变量: 调制系数 gRatio;
************************************************************/
void VFOverMagneticControl()
{
	Uint    m_UDCDesired;
    Uint    m_AVR;
    //Uint    m_Ratio;

    if((gMainStatus.RunStep == STATUS_SPEED_CHECK) ||   // 转速追踪不需要过励磁
        (gVFPar.VFLineType >= 10))                      // Vf分离不需要过励磁
    {
        gOutVolt.VoltApply = gOutVolt.Volt;
        return;
    }
    
// 减速过程中的过励磁处理
	m_UDCDesired = gInvInfo.BaseUdc;
	if(speed_DEC)
	{
		m_UDCDesired = gUDC.uDC;
	}

	if(abs(gVarAvr.UDCFilter - m_UDCDesired) < 10)
	{
		gVarAvr.UDCFilter = m_UDCDesired;
	}
	else if(gVarAvr.UDCFilter > m_UDCDesired)
	{
		gVarAvr.UDCFilter --;
	}
	else
	{
		gVarAvr.UDCFilter += 3;		
		//gVarAvr.UDCFilter = Filter4(m_UDCDesired, gVarAvr.UDCFilter) + 10;
	}
	gVarAvr.UDCFilter = (gVarAvr.UDCFilter < gInvInfo.BaseUdc)?gInvInfo.BaseUdc:gVarAvr.UDCFilter;
    //gVarAvr.UDCFilter = __IQsat(gVarAvr.UDCFilter, 32767, gInvInfo.BaseUdc);
    
	m_AVR = (((long)gVarAvr.UDCFilter)<<12)/gInvInfo.BaseUdc - 4096;
	m_AVR = ((long)m_AVR * (long)gVarAvr.CoffApply)>>6;
    gOutVolt.VoltApply = ((long)m_AVR + 4096L) * gOutVolt.Volt>> 12;

}

/************************************************************
VF 控制过程:
    频率生成:   gMainCmd.FreqSetApply
    电压生成:   gOutVolt.Volt
************************************************************/
void VFSpeedControl()
{
    int slipComp;
//    int mVolt;

//计算VF输出频率， 转差补偿
    slipComp = gWsComp.CompFreq >>16;
    slipComp = (gMainCmd.FreqSet > 0) ? slipComp : (-slipComp);
    gMainCmd.FreqSyn = gVFPar.FreqApply + slipComp;
    
//计算VF输出电压
    gOutVolt.Volt = CalOutVotInVFStatus(gMainCmd.FreqSyn);
    
 
}

    
// 沿用280xp的失速控制
void VfOverCurDeal()
{
	int  m_Cur,m_DetaCur,m_Coff;
	int	 m_LimHigh,m_LimLow,m_MaxStep,m_AddStep;
	int	 m_Data;

	//电流当前状态判断
	m_Cur = gLineCur.CurBaseInv;
	m_Coff = gVFPar.ocGain;

// 判断加减速第一拍
gVfFreq2.spedChg = (gVfFreq2.preSpdFlag != gMainCmd.Command.bit.SpeedFlag) ? 1 : 0;
gVfFreq2.preSpdFlag = gMainCmd.Command.bit.SpeedFlag; 

	//if(0x8000 == (gMainCmd.SpeedFalg & 0x8000))	//加减速标志改变的第一拍
	if(gVfFreq2.spedChg)
	{
		//gMainCmd.SpeedFalg &= 0x7FFF;
		gOvCur.Flag = 0;
		gOvCur.StepLowLim = 1;
		gOvCur.StepLow = 1;
		gOvCur.StepHigh = 0;
		//if((m_Coff < 10) && ((gMainCmd.SpeedFalg & 0x7FFF) != C_SPEED_FLAG_CON))
		if((m_Coff < 10) && (!speed_CON))
		{
			gOvCur.StepLowLim = (20 - m_Coff)<<3;
			gOvCur.StepLow = gOvCur.StepLowLim;
		}
	}

	m_DetaCur = m_Cur - gOvCur.CurBak;
	gOvCur.LowFreq = (((long)(m_Coff + 50) * 10)<<15)/gBasePar.FullFreq01;
	if(m_Coff < 20)		
	{
		gOvCur.SubStep = 1;
		m_AddStep = (20 - m_Coff);//>>1;				//过流抑制小于20用于小惯性负载
	}
	else
	{
		gOvCur.SubStep = 2;
		m_AddStep = 1;
	}

	if(m_Coff < 40)
	{
		gOvCur.MaxStepLow = 10000 / 6 / m_Coff;
		gOvCur.MaxStepHigh = 10000;
	}
	else
	{
		m_Data = (13200-100 * m_Coff);
		gOvCur.MaxStepLow = m_Data/ 6 / m_Coff + 10;
		gOvCur.MaxStepHigh = m_Data>>4;
	}
    if(15 == gInvInfo.InvTypeApply) /*15KW机型做特别处理，限制电流*2011.5.8 L1082*/
     {
       gOvCur.CurLim = ((long)170L <<12) / 100 ;
     }
    else
     {
       gOvCur.CurLim = ((long)gVFPar.ocPoint <<12) / 100 ;
      }
    
	m_LimLow  = ((long)gOvCur.CurLim * 3605L)>>12;
	m_LimHigh = ((long)gOvCur.CurLim * 3891L)>>12;

	if(m_Cur < m_LimLow)					//小于0.88限流点
	{
		gOvCur.StepHigh = 0;
		if((gOvCur.Flag & 0x01) == 0)
		{
			gOvCur.StepLow += m_AddStep;
			gOvCur.StepLowLim += m_AddStep;
		}
		if(abs(gMainCmd.FreqSyn) < gOvCur.LowFreq)	
		{
			m_MaxStep = gOvCur.MaxStepLow;
		}
		else
		{
			m_MaxStep = gOvCur.MaxStepHigh;
		}
		if(gOvCur.StepLowLim > m_MaxStep)
		{
			gOvCur.StepLowLim -= (gOvCur.StepLowLim - m_MaxStep)>>4;
		}
		//gOvCur.StepLowLim = (gOvCur.StepLowLim > m_MaxStep)?m_MaxStep:gOvCur.StepLowLim;
		gOvCur.StepLow = (gOvCur.StepLow > gOvCur.StepLowLim)?gOvCur.StepLowLim:gOvCur.StepLow;

		gOvCur.StepApply = gOvCur.StepLow;
		gOvCur.Flag &= 0x7FFF;				//电流小于0.88限流点标志
	}
	else if(m_Cur < m_LimHigh)				//0.88～0.95限流点之间
	{
		if(m_DetaCur < 0)	
		{
			gOvCur.StepHigh --;
			gOvCur.StepHigh = (gOvCur.StepHigh<-5000)?-5000:gOvCur.StepHigh;
		}
		if(gOvCur.StepHigh >= 0)
		{
			gOvCur.StepLow -= gOvCur.SubStep;
			gOvCur.StepLow = (gOvCur.StepLow < 0)?0:gOvCur.StepLow;
		}
		else
		{
			gOvCur.StepLow = (gOvCur.StepLow < (-gOvCur.StepHigh))?(-gOvCur.StepHigh):gOvCur.StepLow;
		}
		gOvCur.StepApply = -gOvCur.StepHigh;
		gOvCur.Flag |= 0x8000;				//电流大于0.88限流点标志
	}
	else									//大于0.95限流点
	{
		gOvCur.StepLow -= gOvCur.SubStep;
		gOvCur.StepLow = (gOvCur.StepLow < 0)?0:gOvCur.StepLow;
		if(m_DetaCur < 0)	gOvCur.StepHigh = 0;
		else
		{
			gOvCur.StepHigh += (4 + (m_Coff>>4));
			gOvCur.StepHigh = (gOvCur.StepHigh>10000)?10000:gOvCur.StepHigh;
		}

		gOvCur.StepApply = -gOvCur.StepHigh;
		gOvCur.Flag |= 0x8000;				//电流大于0.88限流点标志
	}
	gOvCur.CurBak = m_Cur;
	gOvCur.Flag &= 0xFFFE;					//清除步长太大标志

}

void VfOverUdcDeal()
{
	int m_High,m_Mid,m_Low,m_Coff;
	int	m_Udc,m_DetaU,m_Add1,m_Add2;

	m_Udc = gUDC.uDC;
	m_Coff = gOvUdc.CoffApply;


	
	if((!speed_DEC) ||     // acc or cons
	    (gVfFreq2.spedChg) ||               // 1st step of acc or dec
	    (m_Coff == 0))
	{
		//gMainCmd.SpeedFalg &= 0x7FFF;
		gOvUdc.StepApply = 0;
        gOvUdc.LastStepApply = 0;
        gOvUdc.AccTimes = 0;
		gOvUdc.Flag = 0;
		gOvUdc.StepBak = 0;
		gOvUdc.ExeCnt = m_Coff;
		gOvUdc.UdcBak = m_Udc;
		gOvUdc.CoffApply = gVFPar.ovGain;
		gOvUdc.FreqMax = gVFPar.FreqApply;
		gOvUdc.OvUdcLimitTime = 0;
		return;
	}

	if(abs(gVFPar.FreqApply) > abs(gOvUdc.FreqMax))
	{
		gVFPar.FreqApply = gOvUdc.FreqMax;
		gOvUdc.StepApply = 0;
		gOvUdc.Flag = 0;
		gOvUdc.StepBak = 0;
		return;
	}	

    
	m_High = gOvUdc.Limit;
	m_Mid = ((Ulong)gOvUdc.Limit * 3932L)>>12;			//0.96
	//if(gInvInfo.InvTypeApply < MAX_220V_INV)	m_Low = 3522;
	//else										m_Low = 6100;
	m_Low = 6100;

    // if((m_Udc <= m_Mid) && ((gOvUdc.Flag & 0x01) == 0x01))	
    /*if((m_Udc <= m_Mid) || ((gOvUdc.Flag & 0x01) == 0x01))	
    {
        gOvUdc.ExeCnt = 200;
    }*/
    if((m_Udc <= m_Mid) && ((gOvUdc.Flag & 0x01) == 0x01))	
	{
		gOvUdc.StepApply = gOvUdc.StepBak;
		gOvUdc.Flag &= 0xFFFE;					//清除单拍设置的标志
		return;
	}

	gOvUdc.ExeCnt += 6;
	if(gOvUdc.ExeCnt < m_Coff)		
	{
		gOvUdc.StepApply = 0;
		//gOvUdc.Flag &= 0xFFFE;					//清除单拍设置的标志
		return;
	}
    else gOvUdc.ExeCnt= 0;

	m_DetaU = m_Udc - gOvUdc.UdcBak;
	gOvUdc.UdcBak = m_Udc;

	if((m_Udc <= m_Mid) && ((gOvUdc.Flag & 0x01) == 0x01))	
	{
		gOvUdc.StepApply = gOvUdc.StepBak;
		gOvUdc.Flag &= 0xFFFE;					//清除单拍设置的标志
		return;
	}
    
	m_Add2 = 2;
	m_Add1 = 1;
	if(m_Coff < 10)	
	{
		m_Add1 = 1 + (10 - m_Coff);
		m_Add2 = m_Add1<<1;
	}

	if(m_Udc <= m_Low)
	{
	    gOvUdc.FreqMax = gVFPar.FreqApply;
		if(m_DetaU < 0)			gOvUdc.StepBak += m_Add2;
		else					gOvUdc.StepBak += m_Add1;
	}
	else if(m_Udc <= m_Mid)
	{
		if(m_DetaU < 0)			gOvUdc.StepBak += m_Add1;
	}
	else if(m_Udc <= m_High)
	{
		if(m_DetaU > 5)			
		{
			gOvUdc.StepBak -= 1;
			gOvUdc.Flag |= 0x02;
			gOvUdc.CoffAdd++;				//超过电压阀值时准备增加抑制增益
		}
		else if(m_DetaU < -5)	gOvUdc.StepBak += m_Add1;
	}
	else
	{
		if(m_DetaU > 3)			
		{
			gOvUdc.StepBak -= 1;
			gOvUdc.Flag |= 0x02;
			gOvUdc.CoffAdd += 2;				//超过电压阀值时准备增加抑制增益
		}
	}
	gOvUdc.CoffAdd = (gOvUdc.CoffAdd > 100)?100:gOvUdc.CoffAdd;
	gOvUdc.StepBak = (gOvUdc.StepBak > 20000)?20000:gOvUdc.StepBak;
	gOvUdc.StepBak = (gOvUdc.StepBak < -20000)?-20000:gOvUdc.StepBak;

	gOvUdc.StepApply = gOvUdc.StepBak;
	if(((gOvUdc.Flag & 0x02) == 0) &&		//电压降低后才增加抑制增益
	   (gOvUdc.CoffAdd != 0) && 
	   (gVFPar.ovGain >= 10))		//过压抑制增益小于10不处理
	{	
		gOvUdc.CoffApply += 6;
		gOvUdc.CoffApply = (gOvUdc.CoffApply > 200)?200:gOvUdc.CoffApply;
		gOvUdc.CoffAdd -= 6;
		//gOvUdc.CoffAdd --;
		gOvUdc.CoffAdd = (gOvUdc.CoffAdd < 0)?0:gOvUdc.CoffAdd;
	}
	gOvUdc.Flag &= 0xFFFC;					//清除单拍设置的标志
}

void VfFreqDeal()
{
	int  m_Deta,m_Step,m_StepOI,m_StepOU,m_Freq,m_Dir;//m_Flag;
	int VfStepSet;

	VFOVUdcLimit();
	VfStepSet = abs((int)abs(gMainCmd.FreqSet) - (int)abs(gVFPar.FreqApply));    
	m_StepOI = gOvCur.StepApply;		
	m_StepOU = gOvUdc.StepApply;	
    
	//m_Flag = (gMainCmd.SpeedFalg & 0x7FFF);
    //if(C_SPEED_FLAG_DEC == m_Flag)
    if(speed_DEC)
    {
       m_StepOI = (m_StepOI>1)?m_StepOI:1;  //解决过流抑制引起的无法停机的问题
       if(0 != gOvUdc.StepApply)
       {
            if( (((long)gOvUdc.LastStepApply * (long)gOvUdc.StepApply) < 0) &&
                (gOvUdc.StepApply < 0))
                gOvUdc.AccTimes++;
            if( 10 < gOvUdc.AccTimes )
            {
                gOvUdc.OvUdcLimitTime = 6000;
                gOvUdc.AccTimes = 11;
            }
            gOvUdc.LastStepApply = gOvUdc.StepApply;
       }
    }
	if(gVFPar.ocGain == 0)		m_StepOI = 32767;
	if((gVFPar.ovGain == 0) || 
	   (gOvUdc.OvUdcLimitTime > 5000) ||
	   (!speed_DEC) ||
	   (abs(gVFPar.FreqApply) - abs(gMainCmd.FreqDesired) < 37))
	{
		m_StepOU = 32767;		
	}

	if(m_StepOI < m_StepOU)	
	{
		m_Step = m_StepOI;
 		gOvUdc.Flag |= 1;
	}
	else
	{
		m_Step = m_StepOU;
		gOvCur.Flag |= 1;
	}

	if(m_Step == 32767)
	{
		gVFPar.FreqApply = gMainCmd.FreqSet;
		gOvCur.Flag |= 1;
		gOvUdc.Flag |= 1;
		return;		
	}

    if((speed_DEC) && (VfStepSet <= m_StepOU))
    {
        gOvUdc.StepBak = VfStepSet;
        gOvUdc.StepApply = VfStepSet;
        gOvUdc.Flag |= 1;
    }
    
	if(speed_DEC)
	{
		m_Step = - m_Step;
	}
	m_Freq = abs(gVFPar.FreqApply) + m_Step;


	m_Dir = gMainCmd.FreqSet;
	if(speed_DEC)
	{
		m_Dir = gVFPar.FreqApply;
	}
	if(m_Dir < 0)	m_Freq = - m_Freq;
	
	if(((long)m_Freq * (long)gVFPar.FreqApply) < 0)	m_Freq = 0;

	m_Deta = abs(m_Freq) - abs(gMainCmd.FreqSet);
	if(speed_DEC)
	{
		m_Deta = - m_Deta;
		if(gVFPar.FreqApply == 0)		m_Deta = 0;		//最低减速到0
	}
	if(m_Deta >= 0)	
	{
		gVFPar.FreqApply = gMainCmd.FreqSet;
		gOvCur.Flag |= 1;
		gOvUdc.Flag |= 1;
	}
	else
	{
		gVFPar.FreqApply = m_Freq;
	}
}

/************************************************************
HVF:
    计算振荡系数，可用在判断振荡程度；
************************************************************/
void VfOscIndexCalc()
{
    int temp;
    int curPhase;
    
    if((gCtrMotorType != ASYNC_VF) ||
        (gMainCmd.Command.bit.Start == 0))
    {
        gHVfOscIndex.oscIndex = 0;
        return;
    }

    gHVfOscIndex.AnglePowerFactor = Filter4(abs(gIAmpTheta.PowerAngle), gHVfOscIndex.AnglePowerFactor);
    //gHVfOscIndex.wCntRltm = 1000L*100L*2L / freqRun;        // active window, 2* T_run
    gHVfOscIndex.wCntRltm = 1000L*100L*2L / gMainCmd.FreqReal;
    curPhase = gHVfOscIndex.AnglePowerFactor;
    
    gHVfOscIndex.wCntUse ++;    
    if(gHVfOscIndex.wCntUse < gHVfOscIndex.wCntRltm)
    {
        gHVfOscIndex.maxAngle = (gHVfOscIndex.maxAngle < curPhase) ? curPhase : gHVfOscIndex.maxAngle;
        gHVfOscIndex.minAngle = (gHVfOscIndex.minAngle > curPhase) ? curPhase : gHVfOscIndex.minAngle;
    }
    else                            // update osc-index
    {
        gHVfOscIndex.wCntUse = 0;
        gHVfOscIndex.oscIndex = (long)(gHVfOscIndex.maxAngle - gHVfOscIndex.minAngle) *180L >>15;        // diff(phi) / 90deg * 100%

        gHVfOscIndex.maxAngle = curPhase;
        gHVfOscIndex.minAngle = curPhase;
    }
    
}

// 异步机 HVf 的死区补偿
void HVfDeadBandComp()
{
	int   phase,m_Com;
	long  tempL;
    int   tempSect;

    int   phase_sect;
    int   phase_sect_pre;
    int   temp;
    int   gain;

// 确定补偿量
	if(gMainCmd.FreqReal <= 40000)      m_Com = gDeadBand.Comp;
	else    m_Com = (int)(((long)gDeadBand.Comp * (long)(gMainCmd.FreqReal - 40000))>>15); 
	if((gMainCmd.Command.bit.StartDC == 1) || (gMainCmd.Command.bit.StopDC == 1))	
	{
        m_Com = 0;
	}
    
// 判断电流极性
    gIAmpTheta.ThetaFilter = gIAmpTheta.Theta;

    gHVfDeadBandCompOpt.CurPhaseFeed_pre = gHVfDeadBandCompOpt.CurPhaseFeed;
	gHVfDeadBandCompOpt.CurPhaseFeed = (int)(gPhase.IMPhase>>16) + gIAmpTheta.ThetaFilter + gPhase.CompPhase + 16384;
    gHVfDeadBandCompOpt.CurPhaseStepFed = gHVfDeadBandCompOpt.CurPhaseFeed - gHVfDeadBandCompOpt.CurPhaseFeed_pre;// pos or neg

    if(gMainCmd.FreqReal <= 150 || gMainCmd.FreqReal >= gHVfDeadBandCompOpt.DbOptActHFreq) // 1.50Hz, 12.00Hz
    {
        gHVfDeadBandCompOpt.PhaseFwdFedCoeff = 0;    //__IQsat(gTestDataReceive.TestData6, 128, 0);           // filter coeff
    }
    else if(gMainCmd.FreqReal < 800)    // 8.00Hz
    {
        gHVfDeadBandCompOpt.PhaseFwdFedCoeff = 100;
    }
    else if(gMainCmd.FreqReal < 1200)   // 12.00Hz
    {
        temp = gHVfDeadBandCompOpt.DbOptActHFreq - gHVfDeadBandCompOpt.DbOptActLFreq + 1;     
        tempL = gHVfDeadBandCompOpt.DbOptActHFreq - gMainCmd.FreqReal;
        gHVfDeadBandCompOpt.PhaseFwdFedCoeff = (long)100L * tempL / temp;
    }

    gHVfDeadBandCompOpt.StepPhaseSet = gPhase.StepPhase >> 16;
    
    gHVfDeadBandCompOpt.PhaseFwdFedCoeff = 100;
    tempL = (long)gHVfDeadBandCompOpt.CurPhaseStepFed * (128 -gHVfDeadBandCompOpt.PhaseFwdFedCoeff);
    tempL += (long)gHVfDeadBandCompOpt.StepPhaseSet * gHVfDeadBandCompOpt.PhaseFwdFedCoeff;
    gHVfDeadBandCompOpt.CurPhaseStepPredict = tempL >> 7;
    
    tempSect = gHVfDeadBandCompOpt.CurPhaseFeed / 10922;        // 60deg
    tempSect = __IQsat(tempSect, 5, 0);
    phase_sect = gHVfDeadBandCompOpt.CurPhaseFeed - tempSect * 10922;           // present pos

    tempSect = gHVfDeadBandCompOpt.CurPhaseFeed_pre / 10922;
    tempSect = __IQsat(tempSect, 5, 0);
    phase_sect_pre = gHVfDeadBandCompOpt.CurPhaseFeed_pre - tempSect * 10922;   // previous pos

    if((phase_sect_pre <= 5461 && phase_sect >= 5461) ||            // sample point
        (phase_sect_pre >= 5461 && phase_sect <= 5461))
    {
        gHVfDeadBandCompOpt.CurPhasePredict = gHVfDeadBandCompOpt.CurPhaseFeed;
    }
    else           // go to predict
    {
        gHVfDeadBandCompOpt.CurPhasePredict += gHVfDeadBandCompOpt.CurPhaseStepPredict;
    }

    phase = gHVfDeadBandCompOpt.CurPhasePredict;

    gHVfDeadBandCompOpt.DbCompCpwmWidth = 1;

    if(abs(phase) < gHVfDeadBandCompOpt.DbCompCpwmWidth) gDeadBand.CompU = 0;
    else if(phase < 0)                          gDeadBand.CompU = m_Com;
    else if(phase > 0)                          gDeadBand.CompU = -m_Com;

	phase -= 21845;
    if(abs(phase) < gHVfDeadBandCompOpt.DbCompCpwmWidth) gDeadBand.CompV = 0;
    else if(phase < 0)                          gDeadBand.CompV = m_Com;
    else if(phase > 0)                          gDeadBand.CompV = -m_Com;

	phase -= 21845;
    if(abs(phase) < gHVfDeadBandCompOpt.DbCompCpwmWidth) gDeadBand.CompW = 0;
    else if(phase < 0)                          gDeadBand.CompW = m_Com;
    else if(phase > 0)                          gDeadBand.CompW = -m_Com;
}

void HVfOscDampDeal()
{
    long tempL;
   // int  tempLg;
    int  tempVolt;
    gHVfOscDamp.CurMagSet = gMotorExtPer.IoVsFreq;
    tempVolt = (long)gMotorExtPer.R1 * (gHVfOscDamp.CurMagSet - (gIMTQ24.M>>12)) >>15;
    //gHVfOscDamp.VoltSmSet = (long)tempVolt * (int)gHVfOscDamp.OscDampGain /10L;
    gHVfOscDamp.VoltSmSet = (long)tempVolt * (int)gVFPar.VFOvShock /10L;

    gHVfOscDamp.VoltAmp = gOutVolt.Volt; // 已加转矩提升 
    gHVfOscDamp.VoltPhase = atan(gHVfOscDamp.VoltSmSet, gHVfOscDamp.VoltAmp);

    if(gMainCmd.FreqSyn < 0)      // 反转
    {
        gHVfOscDamp.VoltPhase = - gHVfOscDamp.VoltPhase;
    }
}

/*********************************************************************
consider the stator resitance in low-frequency;
reconstruct the M-T current

Q-current:      Q12
Q-resistance:   Q16
Q-voltage:      Qxx
**********************************************************************/
void HVfCurReDecomp()
{
    int     m_CosPhi;     // Q15, angle phi is the angle of power factor
    int     m_SinPhi;     // Q15,
    long    m_AntiVolt;
    long    m_ResVolt;

    int m_phi2;
    int mDir;

    long temp1;
    long temp2;
    long temp3;
    
    // prepare reistance value, in p.u.
    // pi/2: 16384
    //m_CosPhi = qsin(gIAmpTheta.Theta);
    m_CosPhi = qsin(16384 - gIAmpTheta.PowerAngle);
    m_SinPhi = qsin(gIAmpTheta.PowerAngle);

    m_ResVolt = (long)((Ulong)gLineCur.CurPer * gMotorExtPer.R1 >> 16);   // voltage: Q12
    m_ResVolt = (m_ResVolt < gOutVolt.VoltApply) ? m_ResVolt : gOutVolt.VoltApply;
    
    temp1 = (long)gOutVolt.VoltApply * gOutVolt.VoltApply >> 12;
    temp2 = m_ResVolt * m_ResVolt >> 12;
    temp3 = gOutVolt.VoltApply * m_ResVolt >> 12;
    temp3 = temp3 * m_CosPhi >> 14;
    
    m_AntiVolt = (temp1 + temp2 - temp3) << 12;
    gOutVolt.antiVolt = qsqrt(m_AntiVolt);

    temp1 = ((long)gOutVolt.VoltApply * m_CosPhi >> 15) - m_ResVolt;
    temp2 = (long)gOutVolt.VoltApply * m_SinPhi >> 15;
    m_phi2 = atan((int)temp1, (int)temp2);

    // *generate results
    mDir = (gMainCmd.FreqSyn >= 0) ? 1 : -1;
    gHVfCur.M = (int)((long)gLineCur.CurPer * qsin(m_phi2) >> 15) * mDir;
    gHVfCur.T = (int)((long)gLineCur.CurPer * qsin(16384-m_phi2) >> 15) * mDir;

}
