/***************************************************************************
文件功能：同步机参数辨识，包括初始位置检测，编码器零点位置、方向辨识
文件版本：
最新更新：

***************************************************************************/
//文件功能：
//    1. 同步机磁极位置检测的中断程序和2ms程序， 包括修改ePWM模块实现各相发波；
//    2. 检测磁极位置时顺便辨识出同步机线电感，d、q轴电感；
//    3. 零点位置角的空载辨识，检测编码器方向；
//    4. 零点位置角的带载辨识，(*** 注意需要用户更改编码器方向试探)
//    5. 对编码器的支持包括ABZ，UVW, 旋转变压器；
//       UVW编码器包括uvw信号的零点为直角和uvw信号方向；
//    6. 同步机反电动势的辨识；

#include "MotorInclude.h"

/*******************结构体声明******************************************/
IPM_ZERO_POS_STRUCT		gIPMZero;         //永磁同步电机检测编码器零点位置角的数据结构
IPM_INITPOS_PULSE_STR	gIPMInitPos;      // pmsm 辨识磁极位置
PMSM_EST_BEMF           gEstBemf;         // PMSM辨识反电动势数据
PMSM_EST_PARAM_DATA		gPmParEst;        // 永磁同步机辨识时的和功能的数据交互区


/*******************相关函数声明******************************************/
void InitSetPosTune(void);
void DetectZeroPosOnce(void);
void SynInitPosDetSetTs(void);
void SynCalLabAndLbc(void);
void SynInitPosDetCal(void);


/************************************************************
	初始位置角检测结束后开始辨识编码器零点位置角的初始化程序
************************************************************/
void InitSetPosTune(void)
{
	ResetParForVC();

    gPGDir.ABDirCnt = 0;                    // 复位所有编码器方向
    gPGDir.UVWDirCnt = 0;
    gPGDir.CDDirCnt = 0;
    gPGDir.ABAngleBak = gIPMZero.FeedPos;
    gPGDir.UVWAngleBak = gUVWPG.UVWAngle;
    
	gIPMZero.Flag 		= 0;
	gIPMZero.TotalErr 	= 0;
	gIPMZero.DetectCnt  = 0;
    
	gMainStatus.PrgStatus.all = 0;
}

/*************************************************************
	pm 磁极位置辨识, 需要与中断中程序配合
辨识方法: 脉冲电压法, dq轴电感能顺便得到；
辨识时，由自己发波，处理程序状态字
*************************************************************/
void SynTuneInitPos(void)
{	
	switch(gGetParVarable.IdSubStep)
	{
		case 1:                                             // 初始化变量
			gIPMInitPos.Waite = 0;
            //gIPMInitPos.InitPWMTs	= (50 * DSP_CLOCK);	    //50us
            gIPMInitPos.InitPWMTs	= (50 * DSP_CLOCK);	    //50us
			gGetParVarable.IdSubStep++;
			break;
            
		case 2:                                             // 延时等待20ms, 再初始化相关变量，关PWM发波
			gIPMInitPos.Waite ++;
			if(gIPMInitPos.Waite > 10)			
			{
				gIPMInitPos.Waite = 0;
       			gIPMInitPos.Step = 1;                       // 置静态辨识标志
       			gMainStatus.PrgStatus.bit.PWMDisable = 1;   // 关EPWM 发波
       			gGetParVarable.IdSubStep++;
			}
			break;
            
		case 3:                                             // 等待中断中静态辨识完成...
			if(gIPMInitPos.Step == 0)
			{
				SetIPMPos((Uint)gIPMPos.InitPos);
				SetIPMPos_ABZRef((Uint)gIPMPos.InitPos);
                
				gGetParVarable.IdSubStep++;
			}
			break;
            
		case 4:
			DisableDrive();
			SynCalLdAndLq(gIPMPos.RotorPos);
			IPMCalAcrPIDCoff();
			gGetParVarable.IdSubStep++;
            gIPMInitPos.Waite  = 0;
			break;
            
		case 5:
		    if(gIPMInitPos.Waite < 500)
            {
                gIPMInitPos.Waite  ++;
            }
            else
            {
    			InitSetPWM(); 
    			gGetParVarable.IdSubStep = 1;
    		    gGetParVarable.ParEstMstep++;               //切换到下一辨识步骤

                gMainStatus.PrgStatus.all = 0;
            }
			break;	

        default:
            break;
	}
}

/************************************************************
上电初始位置角检测程序(在周期中断－－AD转换结束中断中执行)，
以U上桥和VW下桥打开为例说明：
	1）PWM1B、PWM2A、PWM3A强制关闭（高电平），
	2）初始强制设置PWM1A、PWM2B、PWM3B为高电平
	3）死区和死区补偿为0 (两次检测求取平均值)
	4) 设置gIPMInitPos.Step为1开始启动静止位置调谐, 完成后将其置0
************************************************************/
void SynInitPosDetect(void)
{
	int  m_Cur;
    int  m_Index;

	switch(gIPMInitPos.Step)
	{
		case 1:							                    //初始参数
			gIPMInitPos.PeriodCnt = 0;
			gIPMInitPos.Section   = 0;
			gIPMInitPos.PWMTs 	  = gIPMInitPos.InitPWMTs;
			gIPMInitPos.CurFirst  = 0;

            gIPMInitPos.CurLimit = 5792;        // 4096*sqrt(2) (Q12)
        	if(gInvInfo.InvCurrent < gMotorInfo.Current)
        	{
        	    Ulong temp; 
                temp = (Ulong)gIPMInitPos.CurLimit * gInvInfo.InvCurrent;                
        		gIPMInitPos.CurLimit = temp / gMotorInfo.Current;
        	}
            
        	            
			SynInitPosDetSetTs();
			SynInitPosDetSetPwm(7);

            gIPMInitPos.Step ++;
			break;

		case 2:							                            //检测合适的脉冲宽度
			EnableDrive();
			gIPMInitPos.PeriodCnt++;
			if(gIPMInitPos.PeriodCnt >= 6)	                        // step: 0 - 5
			{
				gIPMInitPos.PeriodCnt = 0;
			}

			switch(gIPMInitPos.PeriodCnt)
			{
				case 0:
					SynInitPosDetSetPwm(7);
					gIPMInitPos.Section = (gIPMInitPos.Section + 1) & 0x01;     // 脉冲方向反向

                    //m_Cur = abs(gExcursionInfo.Iu);
                    m_Cur = abs(gIUVWQ24.U >> 12);
                    if(m_Cur < abs(gIPMInitPos.CurFirst))
                    {
                        m_Cur = abs(gIPMInitPos.CurFirst);
                    }

                    #if 0
                    if(gIPMInitPos.PWMTs >= 60000)  //电机电感太大，或者输出断开，报初始位置角检测故障
                    {
                        if(m_Cur < (gIPMInitPos.CurLimit>>2))
                        {
                            
                            gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                            gError.ErrorInfo[1].bit.Fault1 = 10;
                        }
                        else
                        {
                            gError.ErrorCode.all |= ERROR_INIT_POS;
                            gError.ErrorInfo[3].bit.Fault3 = 5;
                        }
                    }  
                    #else                    
                    if((gIPMInitPos.PWMTs >= 65000) && (m_Cur < (gIPMInitPos.CurLimit>>2)))
                    {
                        gError.ErrorCode.all |= ERROR_INIT_POS;
                        gError.ErrorInfo[3].bit.Fault3 = 1;
                    }
                    #endif

                    if((m_Cur > gIPMInitPos.CurLimit) || (gIPMInitPos.PWMTs >= 60000))
					{
						gIPMInitPos.Section     = 0;						
						gIPMInitPos.PeriodCnt   = 0;
                        gIPMInitPos.InitPWMTs = gIPMInitPos.PWMTs;  // 记录下脉宽供初始位置检测使用
                        gIPMInitPos.Step = 3;                       // 脉宽已检测到,到下一步
					}
					else
					{                        
				        if(m_Cur > (gIPMInitPos.CurLimit >> 1))
                        {
						    gIPMInitPos.PWMTs += (2 * DSP_CLOCK);      //2us
                        }
                        else
                        {
						    gIPMInitPos.PWMTs += (gMotorExtInfo.Poles *DSP_CLOCK *2); 
                        }
						
						SynInitPosDetSetTs();
					}
					break;

				case 3:
					SynInitPosDetSetPwm(7);
					gIPMInitPos.Section = (gIPMInitPos.Section + 1) & 0x01;     // 脉冲方向反向
					//gIPMInitPos.CurFirst = gExcursionInfo.Iu;
					gIPMInitPos.CurFirst = gIUVWQ24.U >> 12;
					break;

				case 1:
				case 4:
					SynInitPosDetSetPwm(gIPMInitPos.Section);                   // 发脉冲
					break;

				default:
					break;
			}
			break;

		case 3:							
		case 4:
			gIPMInitPos.PeriodCnt++;
			if(gIPMInitPos.PeriodCnt >= 3)  // step: 0-2
			{
				gIPMInitPos.PeriodCnt = 0;
			}

			if(gIPMInitPos.Section <= 1)
			{
				m_Cur  = gIUVWQ24.U >> 12;
			}
			else if(gIPMInitPos.Section <= 3)
			{
				m_Cur  = gIUVWQ24.V >> 12;
			}
			else
			{
				m_Cur  = gIUVWQ24.W >> 12;
			}

			switch(gIPMInitPos.PeriodCnt)
			{
				case 0:
					SynInitPosDetSetPwm(7);
					m_Cur = m_Cur - gIPMInitPos.CurFirst;
					m_Index = (gIPMInitPos.Step - 3) * 6 + gIPMInitPos.Section;					
					gIPMInitPos.Cur[m_Index] = abs(m_Cur);

                    gIPMInitPos.Section++;
					if(gIPMInitPos.Section == 6)
					{
						gIPMInitPos.Section = 0;
						gIPMInitPos.Step++;
					}
					break;

				case 1:
					SynInitPosDetSetPwm(gIPMInitPos.Section);
					break;

				case 2:
					gIPMInitPos.CurFirst  = m_Cur;
					break;
			}
			break;

		case 5:							//结束检测
			DisableDrive();
			SynInitPosDetSetPwm(6);
            
			SynCalLabAndLbc();
			SynInitPosDetCal();

            if(gMainStatus.RunStep == STATUS_GET_PAR)
            {
                gIPMInitPos.Step ++; // 进入同步机缺相检测
            }
			else
            {
                gIPMInitPos.Step = 0;
                DisableDrive();
            }
			
			break;

        case 6:
			SynInitPosDetSetTs();
			SynInitPosDetSetPwm(7);
            EnableDrive();

            gIPMInitPos.Step ++;  
            gIPMInitPos.PhsChkStep = 0;
            break;

        case 7:
            switch (gIPMInitPos.PhsChkStep)
            {
                case 0:
                    SynInitPosDetSetPwm(0);     // A+, B-
                    gIPMInitPos.PhsChkStep ++;
                    break;
                case 1:
                case 4:
                    // wait a step
                    gIPMInitPos.PhsChkStep ++;
                    break;
                case 2:
                    SynInitPosDetSetPwm(7);
                    if(abs(gIUVWQ24.U>>12) < (gIPMInitPos.CurLimit>>3))    // U相缺相
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault1 = 10;
                        gIPMInitPos.Step ++;
                    }
                    if(abs(gIUVWQ24.V>>12) < (gIPMInitPos.CurLimit>>3))    // V相缺相
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault1 = 11;
                        gIPMInitPos.Step ++;
                    }
                    gIPMInitPos.PhsChkStep ++;
                    break;
                case 3:
                    SynInitPosDetSetPwm(2);     // B+ C-
                    gIPMInitPos.PhsChkStep ++;
                    break;
                case 5:
                    SynInitPosDetSetPwm(7);
                    if(abs(gIUVWQ24.W>>12) < (gIPMInitPos.CurLimit>>3))    // W相缺相
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault1 = 12;
                        gIPMInitPos.Step ++;
                    }
                    if(abs(gIUVWQ24.V>>12) < (gIPMInitPos.CurLimit>>3))    // V相缺相
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault1 = 11;
                        gIPMInitPos.Step ++;
                    }
                    gIPMInitPos.PhsChkStep ++;
                    break;
                    
                default:
                    DisableDrive();
                    SynInitPosDetSetPwm(6);         // recove PWM regester
                    gIPMInitPos.Step ++;
                    break; 
            }   
            break;

		default:
			gIPMInitPos.Step = 0;           // 中断执行完成
			DisableDrive();
			break;
	}
}

/*************************************************************
	把转子磁极位置强制设置为Pos的函数
*************************************************************/
void SetIPMPos(Uint Pos)
{
	static Uint m_Pos,m_Cnt;

	m_Pos = Pos;
	m_Cnt = ((Ulong)(m_Pos/gMotorExtInfo.Poles) *(Ulong)gPGData.PulseNum + (2<<13))>>14;

	gIPMPos.RotorPos = m_Pos;
    DINT;
	gIPMPos.QepTotal = m_Cnt;
	gIPMPos.QepBak   = GetQepCnt();
	EINT;
}

/*************************************************************
	把转子磁极位置强制设置为Pos的函数,主要是在ABZ编码器时做参考用
*************************************************************/

void SetIPMPos_ABZRef(Uint Pos)
{
	static Uint m_Pos,m_Cnt;
	
	m_Pos = Pos;
	m_Cnt = ((Ulong)(m_Pos/gMotorExtInfo.Poles) *(Ulong)gPGData.PulseNum + (2<<13))>>14;//将转子位转换为qep对应的计数值

    gIPMPos.ABZ_RotorPos_Ref = m_Pos;
	DINT;
	gIPMPos.ABZ_QepTotal = m_Cnt;
	gIPMPos.ABZ_QepBak   = GetQepCnt();
	EINT;
}


/************************************************************
	LAB、LBC、LCA轴电感计算函数
	gIPMInitPos.Cur保存：IU+、IU-、IV+、IV-、IW+、IW-、
	                     IU+、IU-V+、IV-、IW+、IW-
************************************************************/
void SynCalLabAndLbc(void)
{
	Uint  m_Index,m_Sel;
	Uint  m_DetaI,m_Cur1,m_Cur2,m_UDC;
	Ulong m_LTemp;

	//检测LAB、LBC、LCA
	for(m_Index = 0;m_Index < 3;m_Index++)
	{
		m_Sel = m_Index<<1;
		m_Cur1 = (gIPMInitPos.Cur[m_Sel] <= gIPMInitPos.Cur[m_Sel+1]) ?
						gIPMInitPos.Cur[m_Sel] : gIPMInitPos.Cur[m_Sel+1];
		m_Cur2 = (gIPMInitPos.Cur[m_Sel+6] <= gIPMInitPos.Cur[m_Sel+7]) ?
						gIPMInitPos.Cur[m_Sel+6] : gIPMInitPos.Cur[m_Sel+7];
		m_DetaI = (m_Cur1>>1) + (m_Cur2>>1);

		m_DetaI = ((Ulong)m_DetaI * (Ulong)gMotorInfo.Current) >> 12;	//(单位0.01A or 0.1A)
		m_UDC	= gUDC.uDCFilter - (Ulong)m_DetaI * (Ulong)gMotorExtReg.RsPm / 10000;      // 0.1V
		m_LTemp = (Ulong)gIPMInitPos.PWMTs*2L * 100L/DSP_CLOCK;				            //(单位10ns)
		m_LTemp = (Ulong)m_UDC * (Ulong)m_LTemp;
		//L=((电压/10)*(时间/10^9)/(电流/100))*100 (0.01mH)
		//L = 电压*时间/(电流*100)  (0.01mH)
		gIPMInitPos.LPhase[m_Index] = (m_LTemp)/((Ulong)m_DetaI * 100);
	}
}

/************************************************************
	初始位置计算函数，同时计算绕组电感。
************************************************************/
void SynInitPosDetCal(void)
{
	static int  m_X,m_Y;
	Uint  m_Index;

	for(m_Index = 0;m_Index<12;)
	{
		gIPMInitPos.Cur[m_Index] = gIPMInitPos.Cur[m_Index+1] - gIPMInitPos.Cur[m_Index];
		m_Index = m_Index + 2;
	}
	gIPMInitPos.Cur[0] = (gIPMInitPos.Cur[0] + gIPMInitPos.Cur[0+6]);
	gIPMInitPos.Cur[2] = (gIPMInitPos.Cur[2] + gIPMInitPos.Cur[2+6]);
	gIPMInitPos.Cur[4] = (gIPMInitPos.Cur[4] + gIPMInitPos.Cur[4+6]);

	m_X = gIPMInitPos.Cur[0] - (gIPMInitPos.Cur[2]>>1) - (gIPMInitPos.Cur[4]>>1);
	m_Y = ((long)(gIPMInitPos.Cur[2] - gIPMInitPos.Cur[4]) * 28378L)>>15;
    if(abs(m_X) + abs(m_Y) < (4096/40))
    {
        gError.ErrorCode.all |= ERROR_INIT_POS;
        gError.ErrorInfo[3].bit.Fault3 = 2;
    }//等效于:三相电流偏差之和小于电机额定电流的1/40，认为无法识别磁极初始位置

	gIPMPos.InitPos = (Uint)atan(m_X, m_Y) - 5461;   // 30deg
	gIPMPos.InitAngle_deg = (Ulong)gIPMPos.InitPos * 3600 >> 16;
}

/************************************************************
	设置载波周期寄存器函数
************************************************************/
void SynInitPosDetSetTs(void)
{
	EALLOW;
	EPwm1Regs.TBPRD = gIPMInitPos.PWMTs;
	EPwm1Regs.CMPB  = gIPMInitPos.PWMTs;
	EPwm2Regs.TBPRD = gIPMInitPos.PWMTs;
	EPwm2Regs.CMPB  = gADC.DelayApply;
	EPwm3Regs.TBPRD = gIPMInitPos.PWMTs;
	EDIS;
}

/************************************************************
	初始位置检测阶段，设置PWM的动作寄存器函数，PWM低电平有效
************************************************************/
void SynInitPosDetSetPwm(Uint Section)
{
	Uint m_Section;
    
	m_Section = Section;

	EALLOW;
	switch(m_Section)
	{
		case 0:									// A+, B-
			EPwm1Regs.AQCSFRC.all = 0x08;       
			//pPWMForU->AQCSFRC.bit.CSFB = 2;	
			EPwm2Regs.AQCSFRC.all = 0x02;
			//pPWMForV->AQCSFRC.bit.CSFA = 2;
			EPwm3Regs.AQCSFRC.all = 0x0A;
			//pPWMForW->AQCSFRC.bit.CSFA = 2;
			break;

		case 1:									// A-, B+
			EPwm1Regs.AQCSFRC.all = 0x02;
			//pPWMForU->AQCSFRC.bit.CSFA = 2;	
			EPwm2Regs.AQCSFRC.all = 0x08;
			//pPWMForV->AQCSFRC.bit.CSFB = 2;
			EPwm3Regs.AQCSFRC.all = 0x0A;
			//pPWMForW->AQCSFRC.bit.CSFB = 2;
			break;

		case 2:									// B+, C-
			EPwm1Regs.AQCSFRC.all = 0x0A;
			//pPWMForU->AQCSFRC.bit.CSFA = 2;	
			EPwm2Regs.AQCSFRC.all = 0x08;
			//pPWMForV->AQCSFRC.bit.CSFB = 2;
			EPwm3Regs.AQCSFRC.all = 0x02;
			//pPWMForW->AQCSFRC.bit.CSFA = 2;
			break;

		case 3:									// B-, C+
			EPwm1Regs.AQCSFRC.all = 0x0A;
			//pPWMForU->AQCSFRC.bit.CSFB = 2;	
			EPwm2Regs.AQCSFRC.all = 0x02;
			//pPWMForV->AQCSFRC.bit.CSFA = 2;
			EPwm3Regs.AQCSFRC.all = 0x08;
			//pPWMForW->AQCSFRC.bit.CSFB = 2;
			break;

		case 4:									// A-, C+
			EPwm1Regs.AQCSFRC.all = 0x02;
			//pPWMForU->AQCSFRC.bit.CSFA = 2;	
			EPwm2Regs.AQCSFRC.all = 0x0A;
			//pPWMForV->AQCSFRC.bit.CSFA = 2;
			EPwm3Regs.AQCSFRC.all = 0x08;
			//pPWMForW->AQCSFRC.bit.CSFB = 2;
			break;

		case 5:									// A+, C-
			EPwm1Regs.AQCSFRC.all = 0x08;
			//pPWMForU->AQCSFRC.bit.CSFB = 2;
			EPwm2Regs.AQCSFRC.all = 0x0A;
			//pPWMForV->AQCSFRC.bit.CSFB = 2;
			EPwm3Regs.AQCSFRC.all = 0x02;
			//pPWMForW->AQCSFRC.bit.CSFA = 2;
			break;

		case 6:									//恢复PWM模块的寄存器设置
			EPwm1Regs.DBCTL.all 	= 0x0007;
			EPwm1Regs.AQCTLA.all 	= 0x0090;
			EPwm1Regs.AQCTLB.all 	= 0x00;
			EPwm1Regs.AQCSFRC.all	= 0x00;

			EPwm2Regs.DBCTL.all 	= 0x0007;
			EPwm2Regs.AQCTLA.all 	= 0x0090;
			EPwm2Regs.AQCTLB.all 	= 0x00;
			EPwm2Regs.AQCSFRC.all 	= 0x00;

			EPwm3Regs.DBCTL.all 	= 0x0007;
			EPwm3Regs.AQCTLA.all 	= 0x0090;
			EPwm3Regs.AQCTLB.all 	= 0x00;
			EPwm3Regs.AQCSFRC.all 	= 0x00;
			break;

		default:								//同步机初始位置角检测初始化寄存器
			EPwm1Regs.AQCSFRC.all 	= 0x0A;		
			EPwm1Regs.DBCTL.all 	= 0;			
			EPwm1Regs.AQCTLA.all 	= 0x000C;
			EPwm1Regs.AQCTLB.all 	= 0x000C;

			EPwm2Regs.AQCSFRC.all 	= 0x0A;
			EPwm2Regs.DBCTL.all 	= 0;
			EPwm2Regs.AQCTLA.all 	= 0x000C;
			EPwm2Regs.AQCTLB.all 	= 0x000C;

			EPwm3Regs.AQCSFRC.all 	= 0x0A;
			EPwm3Regs.DBCTL.all 	= 0;
			EPwm3Regs.AQCTLA.all 	= 0x000C;
			EPwm3Regs.AQCTLB.all 	= 0x000C;
			break;
	}
	EDIS;
}

/************************************************************
	同步机空载编码器位置角辨识的时候，检测一次角度
************************************************************/
void DetectZeroPosOnce(void)
{
	Uint m_Data;

    if(gPGData.PGMode == 0)
    {
	    m_Data = gIPMPos.RotorPos - gIPMZero.FeedPos;
    }
    else
    {
        m_Data = gIPMPos.RotorPos - gRotorTrans.RTPos;
    }
    
	if(gIPMZero.DetectCnt == 2000)
	{
		gIPMZero.FirstPos = m_Data;
	}
	else
	{
		gIPMZero.TotalErr += ((long)m_Data - (long)gIPMZero.FirstPos);
	}
	//gIPMZero.DetectCnt++;
}

/************************************************************
	编码器零点位置检测；
* 各种编码器零点位置角度，编码器方向(包括uvw信号方向)一并辨识得出；
* 需要电流闭环配合发M轴电流；
************************************************************/

void SynTunePGZero_No_Load(void)
{
	int	 m_Dir1, m_Dir2, m_Dir3;

    gCtrMotorType = RUN_SYNC_TUNE;
    
	switch(gGetParVarable.IdSubStep)
	{
		case 1:            
            if(gIPMZero.DetectCnt == 0)                     // 初始化相关变量
            {           
    			InitSetPosTune();
    			//gMainStatus.PrgStatus.bit.ASRDisable = 1;       //禁止速度环调节
                
    			//gPGData.PGDir.all = 0;                               // 辨识时先强制方向为0
    			gPGData.PGDir = 0;
                gPmParEst.UvwDir = 0;
                EALLOW;
                EQepRegs->QDECCTL.all = 0;
                EDIS;             

    			gIPMZero.DetectCnt ++;
                gIMTSet.M = 0L <<12;
                gIMTSet.T = 0;
                gIPMZero.CurLimit = 4096UL * (Ulong)gMotorInfo.CurrentGet / gMotorInfo.Current;
            	if(gInvInfo.InvCurrent < gMotorInfo.Current)
            	{
            	    Ulong temp; 
                    temp = (Ulong)gIPMZero.CurLimit * gInvInfo.InvCurrent;                
            		gIPMZero.CurLimit = temp / gMotorInfo.Current;
            	}
                break;
            }
            else if(gIPMZero.DetectCnt == 1)    // 延迟一个2ms再开驱动
            {
                gIPMZero.DetectCnt ++;
                EnableDrive();
            }
            
            if(gIMTSet.M < ((long)gIPMZero.CurLimit <<12))      // M 轴电流上升过程处理, 200ms
            {
                gIMTSet.M += (50L << 12);
            }
            else
            {
                gIMTSet.M = (long)gIPMZero.CurLimit << 12;
                gIPMZero.DetectCnt = 0;
                
                gPGDir.ABDirCnt = 0;
                gPGDir.UVWDirCnt = 0;
                gPGDir.RtDirCnt = 0;

                gGetParVarable.IdSubStep++;
            }
			break;
            
		case 2:                                             // 辨别正反方向            
            gIPMPos.RotorPos += (gMotorExtInfo.Poles * 11);  
            gIPMZero.DetectCnt++;
            
			m_Dir1 = JudgeABDir();											
   			m_Dir2 = JudgeUVWDir();
            m_Dir3 = JudgeRTDir();                 // 判断旋变的正反方向
            
            if(gIPMZero.DetectCnt > 6000)                   //转一圈后判断接线
            {
                if(gPGData.PGMode == 0)     // ABZ, UVW
                {
                    if(m_Dir1 == DIR_ERROR)
                  	{
                        gError.ErrorCode.all |= ERROR_ENCODER;
                        gError.ErrorInfo[4].bit.Fault1 = 1;         // abz 信号方向出错
    				}                
                    else if(gPGData.PGType == PG_TYPE_UVW)
                    {
                        if(m_Dir2 == DIR_ERROR)
                        {
                            gError.ErrorCode.all |= ERROR_ENCODER;
                            gError.ErrorInfo[4].bit.Fault1 = 2;         // uvw 信号方向出错
                        }
                        if(m_Dir2 == DIR_BACKWARD)    
    					{
                            gPmParEst.UvwDir = ((gPmParEst.UvwDir+1)&0x01) ;            // UVW信号硬件不用操作
      
                        }
                    }

                    if(m_Dir1 == DIR_BACKWARD)                  //AB方向取反
                    {
                        EALLOW;
                        gPGData.PGDir = 1;
                        EQepRegs->QDECCTL.all = 0x0400;
                    	EDIS;
                    }
                    gGetParVarable.IdSubStep = 3;
                }
                else                            // Rotor Transformer
                {
                    if(m_Dir3 == DIR_ERROR)
                    {
                        gError.ErrorCode.all |= ERROR_ENCODER;
                        gError.ErrorInfo[4].bit.Fault1 = 3;         // 旋变方向反馈出错
                    }
                    else if(m_Dir3 == DIR_BACKWARD)
                    {
                        gPGData.PGDir = 1;
                    }
                    gGetParVarable.IdSubStep = 4;           // 区别于ABZ编码器
                }
                
                gIPMZero.DetectCnt = 0;
                gIPMZero.Flag = 0;
			}
			break;
            
		case 3:                                             // 等待Z信号, 旋变辨识时就无需此步了
			gIPMPos.RotorPos += (gMotorExtInfo.Poles * 22);  
            gIPMZero.DetectCnt++;
            if(gIPMZero.DetectCnt > 6000)                   // 725个机械角度
            {
                gError.ErrorCode.all |= ERROR_ENCODER;
                gError.ErrorInfo[4].bit.Fault1 = 4;         // 未侦测到z信号
            }
            if(1 == (gIPMZero.Flag & 0x01))                 // Z 信号到
            {
                //gUVWPG.UvwZeroPhase = gIPMPos.RotorPos - gUVWPG.UVWAngle;   // 让角度正常溢出
                //gPmParEst.PreUvwZPos = gPmParEst.UvwZPos;
                gUVWPG.TotalErr = 0;
                gIPMZero.DetectCnt = 0;
                gGetParVarable.IdSubStep++;
                gUVWPG.TuneFlag = 0;
            }
			break;
            
		case 4:                                             // 开始辨识Z信号零点角和UVW信号零点角
            gIPMPos.RotorPos += (gMotorExtInfo.Poles * 6);   // 约5RPM的速度检测零点位置角

			gIPMZero.DetectCnt++;
			if(gIPMZero.DetectCnt >= 2000)
			{
				DetectZeroPosOnce();
			}
            
            if(gPGData.PGType == PG_TYPE_UVW && gIPMZero.DetectCnt < 5461)
            {
                gUVWPG.lastAgl = gUVWPG.NewAgl;
                gUVWPG.NewAgl = gUVWPG.UVWAngle;
                if(((gUVWPG.lastAgl >= 60072)&&(gUVWPG.lastAgl <= 60074))
                    &&(( gUVWPG.NewAgl >=5460)&&(gUVWPG.NewAgl <=5462))
                   )
                {
                    gUVWPG.ErrAgl = gIPMPos.RotorPos;
                    gUVWPG.TuneFlag = 1;
                }                   
            }
            else if(gIPMZero.DetectCnt == 5461)
            {
               gPmParEst.UvwZeroAng = gUVWPG.ErrAgl;
                gPmParEst.UvwZeroAng_deg = ((Ulong)gPmParEst.UvwZeroAng * 3600L +10) >>16;
            }                    
                
			if(gIPMZero.DetectCnt > 6096)
			{
				gIPMPos.RotorZero = gIPMZero.FirstPos + (int)(gIPMZero.TotalErr>>12);
                gPmParEst.CoderPos_deg = ((Ulong)gIPMPos.RotorZero * 3600L + 10)>>16;
                gIPMPos.RotorZeroGet = gPmParEst.CoderPos_deg;
                
				gIPMZero.DetectCnt = 0;
				gGetParVarable.IdSubStep ++;                
			}
			break;
            
		case 5:                             // 等待磁极稳定在 m_RotorPosLast 保存时的位置
                                            // 该过程 gIPMPos.RotorPos 不变
			if(gIPMZero.DetectCnt < 500)
			{
				gIPMZero.DetectCnt++;       
			}
			else
			{   
			    if(gPGData.PGMode == 0)     // 设定QEP
                {         
                    SetIPMPos(gIPMPos.RotorPos);
                    SetIPMPos_ABZRef(gIPMPos.RotorPos);
                }
                gIPMZero.DetectCnt = 0;
                gGetParVarable.IdSubStep ++;
			}
			break;
            
		case 6:
			DisableDrive();
            if(gIPMZero.DetectCnt < 500)
            {
                gIPMZero.DetectCnt ++;
            }
            else
            {
    			gIPMInitPos.Flag = 0;                   //rt 空载辨识后可以直接进入运行，位置更准确
    			gMainStatus.PrgStatus.all = 0;    
    			gGetParVarable.IdSubStep ++;
            }
			break;
            
		case 7:
		default:
			gGetParVarable.IdSubStep = 1;
		    gGetParVarable.ParEstMstep++;       //切换到下一辨识步骤
			break;
	}
}

/************************************************************
	带载编码器零点位置辨识
* 需要闭环矢量运行；
* 需要功能配合给出加减速过程频率给定
* 目标频率， 加减速时间由用户设置；
* 编码器方向需要用户设置，如不能成功，需要反向试探；
* uvw编码器时，uvw信号的方向不用设置，可以直接辨识得出；
************************************************************/
void SynTunePGZero_Load(void)
{
	int	 m_Dir1,m_Dir2, m_Dir3;

    gCtrMotorType = RUN_SYNC_TUNE;
	switch(gGetParVarable.IdSubStep)
	{
		case 1:
            InitSetPosTune();
            // 需要根据脉冲电压法检测的磁极位置准备一个零点位置角，闭环矢量才能运行
            if(gPGData.PGMode == 0)
            {
                ;// ABZ, UVW
            }
            else
            {
                gPmParEst.EstZero = gIPMPos.InitPos - gRotorTrans.RTPos;
                gPmParEst.CoderPos_deg = ((Ulong)gPmParEst.EstZero * 3600L + 10)>>16;
            }
                        
            gGetParVarable.IdSubStep ++;
            break;

        case 2:			
			//gTune.StatusWord = SUB_TUNE_PM_LOAD_ZERO_ACC;             //更新调谐状态字，开始加速
            gGetParVarable.StatusWord = TUNE_ACC;    //开始加速
            gIPMZero.DetectCnt = 0;
                        
            EnableDrive();            
            gGetParVarable.IdSubStep++;
            gIPMZero.time = 0 ;
			break;
            
		case 3:                                 // 等待加速完成并且收到Z信号
            gIPMZero.time ++ ;
            if(gIPMZero.time >= 6000)    // 等待大约12秒左右没有收到Z信号报故障20
            {
                gError.ErrorCode.all |= ERROR_ENCODER;
                gError.ErrorInfo[4].bit.Fault1 = 5;  
            }
			if(speed_CON && 1 == (gIPMZero.Flag & 0x01))
            {
                gIPMZero.DetectCnt++;
                if(gIPMZero.DetectCnt > 500)     //等待1sec进入恒速阶段
                {
                    gIPMZero.DetectCnt = 0;
                    gPGDir.ABDirCnt = 0;
                    gPGDir.UVWDirCnt = 0;
                    gPGDir.RtDirCnt = 0;

                    gUVWPG.TotalErr = 0;
                    gUVWPG.UvwCnt = 0;
                    gUVWPG.UvwEstStep = 0;
                    
                    gIPMZero.Flag &= 0xFFFE;
                    gGetParVarable.IdSubStep++;
                }
            }            
			break;
            
		case 4:                                 //恒速阶段 ---- 判断编码器正反方向
		
            if(gPGData.PGMode == 0)
            {
                m_Dir1 = JudgeABDir();											
   			    m_Dir2 = JudgeUVWDir();
                if(gPGData.PGType == PG_TYPE_UVW && abs(gPGDir.UVWDirCnt) > 2) 
                {   
                    if(m_Dir2 == DIR_BACKWARD)  // UVW 编码器带载辨识时可以辨识UVW 信号的方向
                    {
                        gPmParEst.UvwDir = (gPmParEst.UvwDir+1)&0x01;  // UVW 方向反向
                    }
                    else if(m_Dir2 == DIR_ERROR)
                    {
                        gError.ErrorCode.all |= ERROR_ENCODER;
                        gError.ErrorInfo[4].bit.Fault1 = 6;         // 带载辨识uvw方向反馈出错
                    }
                }

                if(abs(gPGDir.ABDirCnt) > 3)
                {
                    if(gPGData.PGType != PG_TYPE_UVW)
                    {
                        gGetParVarable.IdSubStep++;     //
                    }
                    else if(abs(gPGDir.UVWDirCnt)> 3)
                    {
                        gGetParVarable.IdSubStep++;     //
                    }
                }
               gUVWPG.lastAgl = 0;
               gUVWPG.NewAgl = 0;
               gUVWPG.TuneFlag = 0;
            }
            else if(gPGData.PGMode == 1)
            {
                m_Dir3 = JudgeRTDir();
                if(abs(gPGDir.RtDirCnt) > 5)
                {
                    gIPMZero.DetectCnt = 0;
                    gGetParVarable.IdSubStep++;         //
                }
            }
            break;
            
        case 5:                     //恒速阶段 ---- ABZ、旋变 零点角度辨识

            if(gPGData.PGType == PG_TYPE_UVW && gUVWPG.UvwEstStep == 0)
            {
                gUVWPG.UvwEstStep ++;           // gUVWPG.UvwEstStep==1, main2ms loop start
            }
            else if(gPGData.PGType == PG_TYPE_UVW && gUVWPG.UvwEstStep == 2)
            {
                GetUvwPhase();
                gUVWPG.lastAgl = gUVWPG.NewAgl;
                gUVWPG.NewAgl = gUVWPG.UVWAngle;
                if(((gUVWPG.lastAgl >= 60072)&&(gUVWPG.lastAgl <= 60074))
                    &&(( gUVWPG.NewAgl >=5460)&&(gUVWPG.NewAgl <=5462))
                   )
                {
                    gUVWPG.ErrAgl = gIPMPos.RotorPos;
                    gUVWPG.TuneFlag = 1;
                }  
                else if(((gUVWPG.lastAgl >= 5460)&&(gUVWPG.lastAgl <= 5462))
                          &&(( gUVWPG.NewAgl >=60072)&&(gUVWPG.NewAgl <=60074))
                    )
                {
                    gUVWPG.ErrAgl = gIPMPos.RotorPos;
                    gUVWPG.TuneFlag = 1;
                }

                //或者磁极角度是递减的
                
            }
            else if(gPGData.PGType == PG_TYPE_UVW && gUVWPG.UvwEstStep == 3)
            {
               // gPmParEst.UvwZeroAng = (Uint)(gUVWPG.TotalErr /gUVWPG.UvwCnt);
               gPmParEst.UvwZeroAng = gUVWPG.ErrAgl;
                gPmParEst.UvwZeroAng_deg = ((Ulong)gPmParEst.UvwZeroAng * 3600L +10) >>16;
                gUVWPG.UvwEstStep ++;           // gUVWPG.UvwEstStep==4, uvw est complete totally
            }
    
            if(1 == (gIPMZero.Flag & 0x01))             // 一个机械角度周期, total==16
            {
                gIPMZero.Flag &= 0xFFFE;
    			if(gIPMZero.DetectCnt == 0)
    			{
    				gIPMZero.FirstPos = gIPMPos.PosInZInfo;
    				gIPMZero.TotalErr = 0;
    			}
    			else if(gIPMZero.DetectCnt < 5)
    			{				
    				gIPMZero.TotalErr += (int)(gIPMPos.PosInZInfo - gIPMZero.FirstPos);
    			}
                else if(gIPMZero.DetectCnt == 5)
                {                    
                    gIPMPos.RotorZero = gIPMZero.FirstPos + (int)(gIPMZero.TotalErr >>4);
                    gPmParEst.CoderPos_deg = ((Ulong)gIPMPos.RotorZero * 3600L + 10)>>16;
                    gIPMPos.RotorZeroGet = gPmParEst.CoderPos_deg;
                }
                gIPMZero.DetectCnt++;                
            }   

            if(gIPMZero.DetectCnt > 5)
            {
                if((gPGData.PGType != PG_TYPE_UVW) ||
                    (gPGData.PGType == PG_TYPE_UVW && gUVWPG.UvwEstStep >=4))
                {
                    gGetParVarable.StatusWord = TUNE_DEC;    //开始减速
                    gGetParVarable.IdSubStep++;
                }
            }
		    break;
            
		case 6:                                 // 减速阶段
			if(abs(gMainCmd.FreqSet) <= 10)
            {
                gIPMZero.DetectCnt++;
                if(gIPMZero.DetectCnt > 250)
                {
                    gIPMZero.DetectCnt = 0;
					DisableDrive();
                    
                    gGetParVarable.IdSubStep++;
                }
            }
			break;
            
		case 7:
            if(gIPMZero.DetectCnt < 500)
            {
                gIPMZero.DetectCnt ++;
            }
            else
            {
    			gIPMInitPos.Flag = 0;               //rt 
    			SetIPMPos(gIPMPos.RotorPos);
                SetIPMPos_ABZRef(gIPMPos.RotorPos);

                gMainStatus.PrgStatus.all = 0;
    			gGetParVarable.ParEstMstep++; //切换到下一辨识步骤
    			gGetParVarable.IdSubStep = 1;
            }                
			break;

		default:
			;
			break;
	}
}

/*************************************************************
	同步机反电动势辨识
* 需要电流闭环配合发M轴电流；
* 两点电流分为为: IdSet = 1500, 3000(Q12)；
* 理论上记录d轴电流，约等于线电流，q轴电流很小；
* 理论上记录q轴电压，约等于相(线)电压，d轴电压很小；
* 简化放出: Uq ≈ w(Ld * Id + Phi_r);

* 加减速的加减速过程频率指令 由辨识程序产生；
* 目标频率，加减速时间由功能产生，如下:
    目标频率:   40% 电机额定转速；
    加减速时间: 机型22以下: 30sec；机型22以上: 50sec；
*************************************************************/
void SynTuneBemf()
{
    Ulong temp1, temp2, temp3;
    Ulong fluxRotor;

    gCtrMotorType = RUN_SYNC_TUNE;       //
    
    switch(gGetParVarable.IdSubStep)
    {
        case 1:
            gEstBemf.TotalId1 = 0;
            gEstBemf.TotalId2 = 0;
            gEstBemf.TotalVq1 = 0;
            gEstBemf.TotalVq2 = 0;

            gEstBemf.IdSet = 1500;      // Q12;
            gEstBemf.IqSet = 0;
            gEstBemf.IdSetFilt = 0;            
            if(gInvInfo.InvCurrent < gMotorInfo.Current)
            {
                temp1 = (Ulong)gEstBemf.IdSet * gInvInfo.InvCurrent;
                gEstBemf.IdSet = temp1 / gMotorInfo.Current;
            }
            ResetParForVC();

           
            gEstBemf.TuneFreqAim = (long)gMotorInfo.FreqPer * 2L / 5L;      // 40% 电机额定频率
            gEstBemf.FreqRem = 0;                                           // 频率步长余数清零
            gEstBemf.AccDecTick = (gInvInfo.InvTypeApply <= 20) ? (30L*1000L/2) : (60L*1000L/2);
                                                                    // 机型22以下: 30sec 加速到额定频率
                                                                    // 机型22以上: 60sec 加速到额定频率
            gEstBemf.TuneFreqSet = 0;
            //EnableDrive();
            gGetParVarable.IdSubStep ++;
            break;
            
        case 2:                        // 处于加速过程
            EnableDrive();
            //if(speed_CON)
            gEstBemf.FreqStep = ((long)gMotorInfo.FreqPer + gEstBemf.FreqRem) / gEstBemf.AccDecTick;
            gEstBemf.FreqRem = ((long)gMotorInfo.FreqPer + gEstBemf.FreqRem) % gEstBemf.AccDecTick;
            
            if(gEstBemf.TuneFreqSet < gEstBemf.TuneFreqAim)
            {
                gEstBemf.TuneFreqSet += gEstBemf.FreqStep;
            }
            else
            {
                gEstBemf.TuneFreqSet = gEstBemf.TuneFreqAim;
                gEstBemf.Cnt ++;              
            }
            if(gEstBemf.Cnt > 500)
            {
                gEstBemf.Cnt = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 3:                         // 积分电流和电压----1
            gEstBemf.Cnt ++;
            if(gEstBemf.Cnt <= 2048)
            {
                gEstBemf.TotalId1 += gLineCur.CurPer;           // 电机电流为基值
                gEstBemf.TotalVq1 += gOutVolt.VoltApply;        // 电机电压为基值
            }
            else
            {
                gEstBemf.TotalId1 = gEstBemf.TotalId1 >> 11;
                gEstBemf.TotalVq1 = gEstBemf.TotalVq1 >> 11;

                gEstBemf.TuneFreq = abs(gMainCmd.FreqSyn);
                gEstBemf.Cnt = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 4:                     // 修改电流值
            gEstBemf.IdSet = 3000;
            if(gInvInfo.InvCurrent < gMotorInfo.Current)
            {
                temp1 = (Ulong)gEstBemf.IdSet * gInvInfo.InvCurrent;
                gEstBemf.IdSet = temp1 / gMotorInfo.Current;
            }
            gEstBemf.Cnt ++;
            if(gEstBemf.Cnt > 500)
            {
                gEstBemf.Cnt = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 5:                         // 积分电流电压2
            gEstBemf.Cnt ++;
            if(gEstBemf.Cnt <= 2048)
            {
                gEstBemf.TotalId2 += gLineCur.CurPer;           // 电机电流为基值
                gEstBemf.TotalVq2 += gOutVolt.VoltApply;        // 电机电压为基值
            }
            else
            {
                gEstBemf.TotalId2 = gEstBemf.TotalId2 >> 11;
                gEstBemf.TotalVq2 = gEstBemf.TotalVq2 >> 11;

                gEstBemf.Cnt = 0;
                //gGetParVarable.StatusWord = TUNE_DEC;
                gEstBemf.TuneFreqAim = 0;
                gEstBemf.FreqRem = 0;     
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 6:                         // 等待减速
            gEstBemf.FreqStep = ((long)gMotorInfo.FreqPer + gEstBemf.FreqRem) / gEstBemf.AccDecTick;
            gEstBemf.FreqRem = ((long)gMotorInfo.FreqPer + gEstBemf.FreqRem) % gEstBemf.AccDecTick;
            
            if(gEstBemf.TuneFreqSet > gEstBemf.TuneFreqAim)
            {
                gEstBemf.TuneFreqSet -= gEstBemf.FreqStep;
            }
            else
            {
                gEstBemf.TuneFreqSet = 0;
                DisableDrive();
                gEstBemf.Cnt  = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;
            
        case 7:
            if(gEstBemf.Cnt < 500)          // 等待停机停稳
            {
                gEstBemf.Cnt ++;
            }
            else
            {        
                gEstBemf.Cnt = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 8:                         // 计算反电动势系数，完成辨识           
            temp1 = (Ulong)gEstBemf.TotalVq2 * gEstBemf.TotalId1;
            temp2 = (Ulong)gEstBemf.TotalVq1 * gEstBemf.TotalId2;
            temp3 = (gEstBemf.TotalId2 - gEstBemf.TotalId1) * gEstBemf.TuneFreq;
            fluxRotor = (((Ullong)temp2 - (Ullong)temp1) <<15) /temp3; // Q: 12+12-15 + 15-12 = Q12

            temp3 = fluxRotor * abs(gMotorInfo.FreqPer) >> 12;        // Q15
            gEstBemf.BemfVolt = temp3 * gMotorInfo.Votage * 10L >> 15;              // 0.1 V
            
            gMainStatus.PrgStatus.all = 0;
            gGetParVarable.IdSubStep = 1;
			gGetParVarable.ParEstMstep ++; //切换到下一辨识步骤

            SetIPMPos(gIPMPos.RotorPos);
            SetIPMPos_ABZRef(gIPMPos.RotorPos);
			break;

        default:
            break;
    }            
}

