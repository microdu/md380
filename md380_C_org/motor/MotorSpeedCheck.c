/****************************************************************
文件功能：转速跟踪相关程序
文件版本： 
更新日期： 

****************************************************************/

#include "MotorSpeedCheceInclude.h"

// // 全局变量定义
FEISU_STRUCT			gFeisu;			//转速跟踪用变量
FEISU_STRUCT_NEW        gFeisuNew;
PID_STRUCT				gSpeedCheckPID;
PID_STRUCT				gSpeedCheckPID1;
 
// // 文件内部函数声明 
void SpeedCheckUdcRiseDeal(void);
void SpeedCheckCycleOver(void);
void SpeedCheckSUB1(void);
void SpeedCheckCycleSUB2(void);
void PrepareForSpeedCheck(void);

void Cur_PID(void);
void Cur_PIDonefive(void);
void SaveTheta(void);
 
/************************************************************
	准备进入转速跟踪状态
************************************************************/
void PrepareForSpeedCheck(void)
{
    gFeisu.Case4Sig2.all = 0;	
	gFeisu.Case4Sig1.all = 0;
	gFeisu.AlmostCNT     = 0;
	gFeisu.CycleCnt      = 0;
	gFeisu.GuoduCnt      = 0;
	gFeisu.LowFreqCNT    = 0;
	gFeisu.CurDelta      = 0;
	gFeisu.SpeedCheck    = 0;
	gFeisu.UdcOvCnt      = 0;
	gFeisu.UdcRiseCnt    = 0;
	gFeisu.VoltCheck     = 0;
	gFeisu.VoltCNT       = 0;
	gFeisu.UdcBak        = gUDC.uDC;
	gFeisu.CheckMode     = gComPar.SpdSearchMethod;
	gFeisu.SpeedMaxPu    = (((Ulong)gBasePar.MaxFreq)<<15)/gBasePar.FullFreq;
	gFeisu.Speed5hz      = (500L <<15)/gBasePar.FullFreq01;
    gIMTSetApply.T       = 0;
    gSpeedCheckPID.Total = 0;
	gSpeedCheckPID1.Total = 0;

    gFeisu.Case4Sig1.all = 0x00;
    gFeisu.Case4Sig1.bit.SpdSig = (gFeisu.SpeedLast >= 0) ? 1 : 0;
    gFeisu.CsrWtOver = 0;
    
	if(gFeisu.CheckMode == 2)				//从最大频率跟踪
	{
		gFeisu.Case4Sig1.bit.CHECK_BIT15 = 1;
		//gFeisu.SpeedCheck = gFeisu.Speed5hz;		
        gFeisu.SpeedCheck = (gFeisu.SpeedLast < 0) ? (-gFeisu.SpeedMaxPu) : gFeisu.SpeedMaxPu;
	}
	else if((gFeisu.CheckMode == 1) ||	(abs(gFeisu.SpeedLast) < gFeisu.Speed5hz))
	{	//从零速跟踪或从小的停机频率跟踪							
		// gFeisu.Case4Sig1.bit.CHECK_BIT14 = 0;
		// gFeisu.Case4Sig1.bit.CHECK_BIT15 = 0;
        gFeisu.SpeedCheck = (gFeisu.SpeedLast < 0) ? (-gFeisu.Speed5hz) : gFeisu.Speed5hz;
	}
	else								//从停机
	{
		gFeisu.Case4Sig1.bit.CHECK_BIT14 = 1;
		gFeisu.SpeedCheck = gFeisu.SpeedLast;
	}
	gMainCmd.FreqSyn = gFeisu.SpeedCheck;	    //	set h_freq_syn
	gFeisu.VoltTemp = CalOutVotInVFStatus(gMainCmd.FreqSyn);	    //	cal voltage of VF.


}

/************************************************************
	转速跟踪状态处理
************************************************************/
void RunCaseSpeedCheck(void)
{
	Uint m_UData;
    
    //gComPar.SpdSearchMethod = 2;    // 0: 停机频率开始；1: 0速追踪；2: 最高速追踪；
    //gComPar.SpdSearchTime = 20;

	if((gMainCmd.Command.bit.Start == 0) ||     //停机命令
	   (gError.ErrorCode.all != 0))			    //故障
	{ 
		gMainStatus.RunStep = STATUS_STOP;
		gMainStatus.SubStep = 1;
		gMainStatus.StatusWord.bit.StartStop = 0;
		DisableDrive();
		return;
	}
	gMainStatus.StatusWord.bit.StartStop = 1;

    if(ASYNC_SVC == gCtrMotorType)
    {
        //SVCCalRotorSpeed();           // 0.5ms有计算
        gFeisu.VoltCheckAim = 4096;
    }
    else    // VF
    {
        gFeisu.VoltTemp = CalOutVotInVFStatus(gFeisu.SpeedCheck);
        CalTorqueUp();
        gFeisu.VoltTemp += gOutVolt.detVfVoltUp;
	    gFeisu.VoltCheckAim = (((Ulong)gMotorExtPer.R1 * 2048L)>>15) + gFeisu.VoltTemp;
        gComPar.SpdSearchTime = gComPar.SpdSearchTimeSet;
        // 从零速跟踪时，加大目标电压，防止误判断
        if(gInvInfo.InvTypeApply > 13)         
        {   
           gComPar.SpdSearchTime = (long)gComPar.SpdSearchTimeSet * 10L/(gInvInfo.InvTypeApply + 10);
           if(gFeisu.CheckMode == 1)
            {
              gFeisu.VoltCheckAim = (((Ulong)gMotorExtPer.R1 * 5125L)>>15) + gFeisu.VoltTemp;
            }
        }        
    }
    
/////////////////////////////////////////////////////////////////
//已经搜索出速度，对电压过度处理， 进入追踪拖到
	if(gFeisu.Case4Sig1.bit.CHECK_GET == 1)	
	{				
		if(gFeisu.VoltCheck > (gFeisu.VoltCheckAim + 3))
		{	
			gFeisu.VoltCheck -= 2;
			gFeisu.GuoduCnt = 0;
		}
		else if(gFeisu.VoltCheck < (gFeisu.VoltCheckAim - 3)) //开环矢量需要使用目标电压值	
		{	
			gFeisu.VoltCheck += 2;
			gFeisu.GuoduCnt = 0;

		}
		gFeisu.GuoduCnt++;
		if(gFeisu.GuoduCnt > 400)  			//voltage equals with * for 1 second 
		{
			gMainStatus.RunStep = STATUS_RUN;		//速度搜速完毕
			gMainStatus.SubStep = 1;
			gMainStatus.StatusWord.bit.SpeedSearchOver = 1;
			gFeisu.Case4Sig1.all=0;
		}
		SpeedCheckSUB1();
		return;
	}
    
/////////////////////////////////////////////////////////////////
// 速度搜索阶段，输出电压的产生(gFeisu.VoltCheck)
	//gSpeedCheckPID.KP   = 6400;       //current pid control
	gSpeedCheckPID.KP = 200; 
	gSpeedCheckPID.KI = 400;
    gSpeedCheckPID.QP = 0;
    gSpeedCheckPID.QI = 0;
	gSpeedCheckPID.KD = 0;
	gSpeedCheckPID.Max = 4096;
	gSpeedCheckPID.Min = 0;
  #if 0
	if(gInvInfo.InvCurrent < gMotorInfo.Current)
	{
		gFeisu.CurDelta = ((Ulong)gInvInfo.InvCurrent * 5125L) / gMotorInfo.Current;
		gFeisu.CurDelta = (gFeisu.CurDelta > 5125)?5125:gFeisu.CurDelta;
	}
	else
  #endif
	{
		gFeisu.CurDelta = 5125;					//130% rate current
	}
	gSpeedCheckPID.Deta = gFeisu.CurDelta - (int)gLineCur.CurPer;
	PID((PID_STRUCT *)&gSpeedCheckPID);
	gFeisu.VoltCheck = (gSpeedCheckPID.Out>>16);
    
///////////////////////////////////////////////////////////////
// 速度搜索接近结束, 也是过渡过程
	if(gFeisu.Case4Sig2.bit.DelayOver == 1)	// delay (bit7)is over
	{
		gFeisu.VoltCNT++;
		if(gFeisu.VoltCNT > 100)		
		{
			gFeisu.Case4Sig1.bit.CHECK_GET = 1; // speed search complete
		}
		SpeedCheckSUB1();
		return;
	}
    
	if(gFeisu.Case4Sig2.bit.AlmostOver == 1) //almost over
	{
		gFeisu.AlmostCNT++;
		if((gFeisu.AlmostCNT > 1000) || 
		   ( (gFeisu.Case4Sig2.bit.TwoCyclesOver == 0) &&       // 已经搜索了两遍
		     (gFeisu.VoltCheck < gFeisu.VoltCheckAim) ) )       // 
		{
			gFeisu.Case4Sig2.bit.DelayOver = 1;
		}
		SpeedCheckSUB1();
		return;
	}
	gFeisu.AlmostCNT = 0;

// 稍作修正，根据电压判断是否完成搜索
	m_UData = abs(gFeisu.SpeedCheck)>>8;                        
	m_UData = m_UData + 512;
	m_UData = ((Ulong)m_UData * (Ulong)gFeisu.VoltCheck)>>9;
	if(m_UData > gFeisu.VoltCheckAim)
	{
		gFeisu.Case4Sig2.bit.AlmostOver =1;
		SpeedCheckSUB1();
		return;
	}

	gFeisu.LowFreqCNT=0;
//////////////////////////////////////////////////////////////
// 频率搜索阶段，分从0速搜索和从最大速搜索
	if(gFeisu.Case4Sig1.bit.UdcHigh == 0)  //change freqency
	{
	// 产生搜索步长
		if(((gLineCur.CurPer + 1000) < gFeisu.CurDelta) &&  	// 等待电流闭环到位
		    (gFeisu.CsrWtOver == 0))                            // 只让第一次有效
		{
			gFeisu.Ger4A = 0;
		}
		else								
		{
			gFeisu.Ger4A = gComPar.SpdSearchTime + 1;
            gFeisu.CsrWtOver = 1;
		}
        
    // search form speed-0
		if(((gFeisu.Case4Sig1.all & 0x0C000) == 0) ||       // 
		   ((gFeisu.Case4Sig1.all & 0x0C000) == 0x0C000))
		{
			if(gFeisu.Case4Sig1.bit.SpdSig == 0)	// negative
			{
				gFeisu.SpeedCheck -= gFeisu.Ger4A;
			}
			else								    // positive
			{
				gFeisu.SpeedCheck += gFeisu.Ger4A;
			}

			if(abs(gFeisu.SpeedCheck) >= gFeisu.SpeedMaxPu)
			{
				if(gFeisu.CycleCnt == 0)    // 第一个搜索周期
				{
					gFeisu.CycleCnt = 0x1111;
					gFeisu.Case4Sig1.all ^= 0x2000;         // 频率方向反向(搜索到正反方向)
					if(gFeisu.Case4Sig1.bit.SpdSig == 1)	
					{
						gFeisu.SpeedCheck = gFeisu.Speed5hz;
					}
					else									
					{
						gFeisu.SpeedCheck = -gFeisu.Speed5hz;
					}
					SpeedCheckCycleSUB2();
				}
				else                       // 第二个搜索周期结束处理
				{
					SpeedCheckCycleOver();	
				}
			}
		}
    //search from f_max or stop freq
		else
		{
			if(gFeisu.Case4Sig1.bit.SpdSig == 1)	//positive
			{
				gFeisu.SpeedCheck -= gFeisu.Ger4A;
				if(gFeisu.SpeedCheck < 0)
				//if(gFeisu.SpeedCheck < gFeisu.Speed5hz) //rd
				{
					if(gFeisu.CycleCnt == 0)
					{
						gFeisu.CycleCnt = 0x1111;
						gFeisu.Case4Sig1.bit.SpdSig = 0;
						gFeisu.SpeedCheck = -gFeisu.SpeedMaxPu;
						SpeedCheckCycleSUB2();
					}
					else
					{
						SpeedCheckCycleOver();
					}
				}
			}
			else								
			{									//negtive
				gFeisu.SpeedCheck += gFeisu.Ger4A;
				if(gFeisu.SpeedCheck > 0)     //rd
				//if(gFeisu.SpeedCheck > (-(int)gFeisu.Speed5hz))
				{
					if(gFeisu.CycleCnt == 0)
					{
						gFeisu.CycleCnt = 0x1111;
						gFeisu.Case4Sig1.bit.SpdSig = 1;
						gFeisu.SpeedCheck = gFeisu.SpeedMaxPu;
						SpeedCheckCycleSUB2();
					}
					else
					{
						SpeedCheckCycleOver();
					}
				}
			}
		}        
    //the following: decide whether the UDC is high or not
		gFeisu.UdcDelta = gUDC.uDCFilter - gFeisu.UdcOld;
		gFeisu.UdcOld   = gUDC.uDCFilter;
		if((gUDC.uDCFilter - 800) > gFeisu.UdcBak)      // bus voltage rised much more: 80.0V
		{
			//gFeisu.Ger4A = (gComPar.SpdSearchTime << 3)+5;
			if((gFeisu.CheckMode != 1)||(gMainCmd.FreqReal> 1000))
			{
				gFeisu.Case4Sig1.bit.UdcHigh = 1;
				gFeisu.Ger4A = 400;
			}
			else
			{
    			gFeisu.Ger4A = 0;
			}
			SpeedCheckUdcRiseDeal();
			SpeedCheckSUB1();
			return;
		}
		else
		{
			if(gFeisu.UdcDelta > 0)		// bus rised a little
			{
				gFeisu.UdcRiseCnt++;
				if(gFeisu.UdcRiseCnt >= 1000)
				{
					gFeisu.Ger4A=(gComPar.SpdSearchTime << 3)+5;
					gFeisu.Case4Sig1.bit.UdcHigh=1;
					SpeedCheckUdcRiseDeal();
					SpeedCheckSUB1();
					return;
				}
				else
				{		
					SpeedCheckSUB1();
					return;
				}
			}
			else            // bus decreased
			{
				gFeisu.Case4Sig1.bit.UdcHigh=0;
				gFeisu.UdcRiseCnt=0;
				SpeedCheckSUB1();
				return;
			}
		}
	}
	else    //gFeisu.Case4Sig1.bit.UdcHigh == 1  //UDC is too high		
	{
		gFeisu.UdcOvCnt ++;
		if(gFeisu.UdcOvCnt > 1)
		{
			gFeisu.UdcOvCnt = 0;
			gFeisu.UdcDelta = gUDC.uDCFilter - gFeisu.UdcOld;
			gFeisu.UdcOld   = gUDC.uDCFilter;
			if(gFeisu.UdcDelta > 0)
			{
				gFeisu.Ger4A = (gComPar.SpdSearchTime << 3)+5;
				SpeedCheckUdcRiseDeal();
			}
			else if(gUDC.uDCFilter - gFeisu.UdcBak <= 350)	
			{
				gFeisu.Case4Sig2.bit.AlmostOver = 1;
			}
		}
		SpeedCheckSUB1();
		return;
	}
}

/************************************************************
	转速跟踪子程序, 得到输出频率和电压
outputVar:  
frequency -> gMainCmd.FreqSyn;
voltage   -> gOutVolt.VoltApply;
************************************************************/
void SpeedCheckSUB1(void)
{
    // gFeisu.SpeedCheck 和 gFeisu.VoltCheck已经准备好；
    // 等待主程序选择

    gMainCmd.FreqSyn  = gFeisu.SpeedCheck;
	gMainCmd.FreqSet       = gFeisu.SpeedCheck;
	gMainCmd.FreqDesired   = gFeisu.SpeedCheck;
    
    if(IDC_VF_CTL == gMainCmd.Command.bit.ControlMode)
    {
        gFeisu.VoltTemp = CalOutVotInVFStatus(gMainCmd.FreqSyn);		// 为计算目标电压
    	//gOutVolt.VoltPhaseApply = gOutVolt.VoltPhase;
    	if(gMainCmd.FreqSetBak > 0)
        {
            gOutVolt.VoltPhaseApply = 16384;
        }
    	else if(gMainCmd.FreqSetBak < 0)
        {
            gOutVolt.VoltPhaseApply = -16384;      
        }
    	gOutVolt.VoltApply      = gFeisu.VoltCheck;
    }

}

/************************************************************
	转速跟踪子程序 ---------搜索速度，update gFeisu.SpeedCheck
************************************************************/
void SpeedCheckUdcRiseDeal(void)
{
//	if((gFeisu.Case4Sig1.bit.CHECK_BIT14 == 1)||
//	   (gFeisu.Case4Sig1.bit.CHECK_BIT15 == 1))
//	{
//		gFeisu.Ger4A = -gFeisu.Ger4A;
//	}
	if(gFeisu.SpeedCheck >= 0)	
	{
		gFeisu.SpeedCheck += abs(gFeisu.Ger4A);
	}
	else
	{
		gFeisu.SpeedCheck -=abs( gFeisu.Ger4A);
	}

	if(((gFeisu.Case4Sig1.all & 0x0C000) == 0x4000)||
	   ((gFeisu.Case4Sig1.all & 0x0C000) == 0x8000))
	{
		if((gFeisu.Case4Sig1.bit.SpdSig == 1) && (gFeisu.SpeedCheck < 0))
		{
			gFeisu.SpeedCheck = 0;
		}
		if((gFeisu.Case4Sig1.bit.SpdSig == 0) && (gFeisu.SpeedCheck > 0))
		{
			gFeisu.SpeedCheck = 0;
		}
	}
	if(((gFeisu.Case4Sig1.all & 0x0C000) == 0x4000) && 
	   (gFeisu.SpeedLast >= 0) &&
	   (gFeisu.SpeedCheck > gFeisu.SpeedLast))
	{
		gFeisu.SpeedCheck = gFeisu.SpeedLast;
	}
	else if(((gFeisu.Case4Sig1.all & 0x0C000) == 0x4000) &&
	        (gFeisu.SpeedLast < 0) &&
	        (gFeisu.SpeedCheck < gFeisu.SpeedLast))
	{
		gFeisu.SpeedCheck = gFeisu.SpeedLast;
	}
	else if(((gFeisu.Case4Sig1.all & 0x0C000) != 0x4000) && 
	        (gFeisu.SpeedCheck > gFeisu.SpeedMaxPu))
	{
		gFeisu.SpeedCheck = gFeisu.SpeedMaxPu;
	}
	else if(((gFeisu.Case4Sig1.all & 0x0C000) != 0x4000) && 
	        (gFeisu.SpeedCheck < -gFeisu.SpeedMaxPu))
	{
		gFeisu.SpeedCheck = -gFeisu.SpeedMaxPu;
	}
}

/************************************************************
	转速跟踪子程序
************************************************************/
void SpeedCheckCycleOver(void)          // 搜索不成功速度处理	
{
	if(gMainCmd.FreqDesired > 0)	            
	{
		gFeisu.SpeedCheck = (gFeisu.Speed5hz >> 3);
	}
	else	
	{
		gFeisu.SpeedCheck = -(gFeisu.Speed5hz >> 3);
	}
	gFeisu.Case4Sig2.all |= 0x8040;
	gFeisu.VoltCheck      = 0;
	gSpeedCheckPID.Total  = 0;
	gFeisu.Case4Sig1.bit.CHECK_GET = 1;
}

/************************************************************
	转速跟踪子程序 ------- 变量清零
************************************************************/
void SpeedCheckCycleSUB2(void)
{
	gFeisu.VoltCheck     = 0;
	gSpeedCheckPID.Total = 0;
}


