/****************************************************************
文件功能：异步机参数辨识
文件版本： 
最新更新： 
	
****************************************************************/
#include "MotorParaIDinclude.h"

// // 文件内部函数声明
interrupt void RrLoInPeriodAndCmpInt(void);

void RsIdentify(void);
void RrLoIdentify(void);
void LmIoIdentify(void);
void SetPwmForRrLoId(void);
void InitRrLoVariable(void);
void EnableTimer(void);
void RrWaitAddGetUdc(void);
void RrGetVoltReBegin(void);
void RrLoCalVaule(void);
void InitLmIoVaribale(void);
void LmIoCalValue(void);

/*******************************************************************
函数功能 转子电阻和漏感辨识时，在周期中断和比较中断中检测电流和电压
********************************************************************/

/************************************************************
	参数辨识阶段(该程序用于检测U相和V相电流检测通道的增益偏差)
	同时检测定子电阻值。
************************************************************/

/****************************************************************
函数功能：定子电阻辨识
*****************************************************************/
void RsIdentify(void)
{
	Uint  m_UData1,m_UData2,m_MaxCur;
	Ulong m_ULong1,m_ULong2;

    m_MaxCur = ((Ulong)gInvInfo.InvCurrent * 3277)>>12;     //80%变频器额定电流
	switch(gGetParVarable.IdSubStep)
	{
		case 1:     // 辨识定子电阻时发波的处理, EPWM模块的设置, 参数的初始化
		    if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
              {
               gUVCoff.Rs_PRD =SHORT_GND_PERIOD_1140; /*1140V定子电阻辩士载频固定1K，其它电压等级4K，2011.5.7 L1082*/
              }
            else if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
              {
               gUVCoff.Rs_PRD =SHORT_GND_PERIOD_690; /*690V定子电阻辩士载频固定1.5K，其它电压等级4K，2011.6.20 L1082 */
              }
            else
              {
               gUVCoff.Rs_PRD =SHORT_GND_PERIOD;
              }  
			gUVCoff.Comper     = gUVCoff.Rs_PRD/2;
			gUVCoff.Number     = 0;
			gUVCoff.TotalU     = 0;
			gUVCoff.TotalV     = 0;
			gUVCoff.TotalI     = 0;
			gUVCoff.TotalIL    = 0;
			gUVCoff.TotalVolt  = 0;
			gUVCoff.TotalVoltL = 0;
            
			EALLOW;
            EPwm1Regs.ETSEL.bit.INTEN = 0;  //定子电阻辨识时，禁止下溢中断
			EPwm1Regs.TBPRD = gUVCoff.Rs_PRD;
			EPwm1Regs.CMPB  = EPwm1Regs.TBPRD - gADC.DelayApply;
			EPwm2Regs.TBPRD = gUVCoff.Rs_PRD;
			EPwm3Regs.TBPRD = gUVCoff.Rs_PRD;

			EPwm1Regs.CMPA.half.CMPA = gUVCoff.Comper;
			EPwm2Regs.CMPA.half.CMPA = gUVCoff.Comper;

		
			EPwm3Regs.AQCSFRC.all = 0x0A;		//关闭3桥臂
			EPwm3Regs.DBCTL.all   = 0x0000;
			EDIS;
            
			gGetParVarable.IdSubStep = 2;               
			break;

		case 2:         // 需多次辨识时延时之后再开启驱动
            if(0 == gUVCoff.IdRsDelay)
            {
			    gGetParVarable.IdSubStep = 3;
			    EnableDrive();
            }
            else
            {
                gUVCoff.IdRsDelay--;
            }
			break;
            
		case 4:
			gUVCoff.Number++;
			if(gUVCoff.Number >= 512)
			{
				gUVCoff.TotalVoltL += gUDC.uDCFilter;
				gUVCoff.TotalIL    += abs(gIUVWQ12.U);
			}
			if(gUVCoff.Number >= 1024)
			{
				gUVCoff.ComperL = gUVCoff.Comper;
				gGetParVarable.IdSubStep = 5;
			}
			break;
			
		case 3:
		case 5:
            
			m_UData1 = (1024UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;//变频器电流大于4倍以上电机额定电流
			if(gGetParVarable.IdSubStep == 5)
            {
                m_UData1 = (4096UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;
                if( 16 < gInvInfo.InvTypeApply )
                {
                    m_UData1 = (3500UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;  //避免检测时报过载
                }
            }
            if( gMotorInfo.Current > m_MaxCur )  //电机额定电流大于变频器额定电流时，以变频器额定电流为准
            {
                m_UData1 = (((long)m_MaxCur) * (long)m_UData1) / gMotorInfo.Current;
            }

			gUVCoff.Number = 0;
			if(abs(gIUVWQ12.U) >= m_UData1)
			{
				gGetParVarable.IdSubStep++;
			}
			else
			{
			    if(gInvInfo.InvTypeApply > 16 )
                {         
                   gUVCoff.Comper += 1;         
                }
                else
                {
				   gUVCoff.Comper += 4;
                }
				if(gUVCoff.Comper >= gUVCoff.Rs_PRD)      // 125us
				{
			        gUVCoff.Comper = gUVCoff.Rs_PRD/2;  //由功能模块判断超时报ERR19
			        // 可以报缺相
			        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                    gError.ErrorInfo[1].bit.Fault1 = 11;
				}
				else
				{
					EALLOW;
					EPwm1Regs.CMPA.half.CMPA = gUVCoff.Comper;
					EPwm2Regs.CMPA.half.CMPA = gUVCoff.Rs_PRD - gUVCoff.Comper;
					EDIS;
				}
			}
			break;

		case 6:
			gUVCoff.TotalU += abs(gIUVWQ12.U);
			gUVCoff.TotalV += abs(gIUVWQ12.V);
			gUVCoff.Number++;
			if(gUVCoff.Number >= 512)
			{
				gUVCoff.TotalVolt += gUDC.uDCFilter;
				gUVCoff.TotalI += abs(gIUVWQ12.U);

				if(gUVCoff.Number >= 1024)
				{
                    DisableDrive();
					gUVCoff.Number = 0;
					m_UData1 = (gUVCoff.TotalU << 4) / (Uint)(gUVCoff.TotalV >> 8);
					gUVCoff.UDivVSave = ((Ulong)m_UData1 * 1000)>>12;

					m_UData2 = gUVCoff.Rs_PRD/2;
					//m_UData2 = C_INIT_PRD/2;
					m_UData1 = (gUVCoff.TotalVoltL<<5)/m_UData2;
					m_ULong1 = ((Ulong)m_UData1 * (Ulong)(gUVCoff.ComperL - m_UData2));
					m_UData1 = (gUVCoff.TotalVolt<<5)/m_UData2;
					m_ULong1 = ((Ulong)m_UData1 * (Ulong)(gUVCoff.Comper  - m_UData2)) - m_ULong1;
                    m_ULong1 =  m_ULong1 * 10;//电压变量一个小数点，电流2个小数点，所有要乘以10
					//m_ULong1 = 电压差(V)×10×2^14 
					m_UData2 = (gUVCoff.TotalI - gUVCoff.TotalIL)>>7;
					m_ULong2 = ((Ulong)m_UData2 * (Ulong)gMotorInfo.Current)>>10;
					//m_ULong2 = 电流差(A)×10×2^4
					while((m_ULong2>>16) != 0)
					{
						m_ULong1 = m_ULong1>>1;
						m_ULong2 = m_ULong2>>1;
					}
					m_UData2 = m_ULong1/(Uint)m_ULong2;
                    m_UData2 = ((Ulong)m_UData2 * (Ulong)1000)>>11;
                    if(0 == gUVCoff.IdRsCnt)
                    {
                        gUVCoff.IdRsBak = m_UData2;
                        gGetParVarable.IdSubStep = 1;
                        gUVCoff.IdRsCnt++;
                        gUVCoff.IdRsDelay = 1000; //第二次辨识之前，延时2s
                    }
                    else
                    {
                        if(MOTOR_TYPE_PM == gMotorInfo.MotorType)
                        {
                           gMotorExtReg.RsPm = m_UData2;
                        }
                        else
                        {
                    	    gMotorExtReg.R1 = m_UData2;
                        }
					    gGetParVarable.IdSubStep = 7;
                    }
				}
			}
			break;
            
		case 7:			
		default:
			DisableDrive();
  			gGetParVarable.IdSubStep = 1;
            gGetParVarable.ParEstMstep++; 
            InitSetPWM();                       //恢复修改的寄存器配置
			break;
	}	
}

/*******************************************************************
函数功能 转子电阻和漏感辨识，静态辨识的一部分
********************************************************************/
void RrLoIdentify(void)
{
    switch( gGetParVarable.IdSubStep )
    {
        case 1:	        //第一步：定子电阻辨识后的延时阶段，该阶段需要检测母线电压；
       	    gRrLoIdentify.WaitCnt++;
            EALLOW;
            PieCtrlRegs.PIEIER1.bit.INTx1 = 0;           //  ADC1INT
            EDIS;
			if( gRrLoIdentify.WaitCnt < 1000 )			//延迟时间2s
			{
			    RrWaitAddGetUdc();
			}
			else
			{
                SetPwmForRrLoId();              // configue ePWM module          			
			    InitRrLoVariable();
                EnableTimer();
                
	            gRrLoIdentify.PwmCompareValue = 200;			//初始脉冲120宽度
	            //额定电流对应的AD采样值。AD采样值最大为32767，右移一位后是16384。额定电流是它的一半
	            gRrLoIdentify.CurrentRatio = 8192L * (long)gMotorInfo.Current / gInvInfo.InvCurrent; 
	            gRrLoIdentify.CurrentRatio = (gRrLoIdentify.CurrentRatio > 8992)?8992:gRrLoIdentify.CurrentRatio;
                gRrLoIdentify.RrL0CycleCnt = 0;
	            gRrLoIdentify.RrAndRsAccValue = 0;
 	            gRrLoIdentify.LoAccValue = 0;                
				gGetParVarable.IdSubStep = 2;
			}
            break;
            
        case 2:                             //寻找合适输出电压值的过程
            
            if(gRrLoIdentify.WaitCnt != 0)	//如果处于等待阶段，则检测母线电压
			{
				gRrLoIdentify.WaitCnt--;
				if(gRrLoIdentify.WaitCnt != 0)
				{
					RrWaitAddGetUdc();
				}   
				else
				{
					InitRrLoVariable();
                    EnableTimer();  
				}
			}
			else
			{                
				if((gRrLoIdentify.CurrentMax - 3200) > gRrLoIdentify.CurrentRatio)
				{	
                    gRrLoIdentify.PwmCompareValue -= 20;
					gRrLoIdentify.WaitCnt = 20;	//若电流比较大则降低电压重新来过
					RrGetVoltReBegin();
				}
				else if(1 == gRrLoIdentify.RrL07PulseOverSig)
				{
					//if(gRrLoIdentify.SampleTimes >= 14)
					//{
						if(gRrLoIdentify.CurrentMax > gRrLoIdentify.CurrentRatio)
						{
							gGetParVarable.IdSubStep = 3;	//若电流到达额定电流，则进入STEP4
						}
						else							//电流未到，则修改电压后重新来过
						{
							if((gRrLoIdentify.CurrentMax * 2) > gRrLoIdentify.CurrentRatio)
							{
									gRrLoIdentify.PwmCompareValue += 50;
							}
							else
							{
									gRrLoIdentify.PwmCompareValue += 100;
							}
							gRrLoIdentify.WaitCnt = 20;	//修正输出电压，再发一次
							RrGetVoltReBegin();
						}
					//}
					//else
					//{
					//	gRrLoIdentify.WaitCnt = 20;
					//	RrGetVoltReBegin();				//比较或周期检测个数不对，重新来过
					//}
			    }
			}
            break;
            
        case 3:		                            //合适电压已经找到，开始几个脉冲的采样与计算
			if(gRrLoIdentify.WaitCnt != 0)	    //如果处于等待阶段，则检测母线电压
			{
				gRrLoIdentify.WaitCnt--;
				if(gRrLoIdentify.WaitCnt != 0)
				{
					RrWaitAddGetUdc();
				}
				else
				{
					InitRrLoVariable();
                    EnableTimer();
				}
			}
			else
			{
				if(1 == gRrLoIdentify.RrL07PulseOverSig)
				{
					if(14 == gRrLoIdentify.SampleTimes)
					{								//开始计算辨识结果
						RrLoCalVaule();
						if(6 <= gRrLoIdentify.RrL0CycleCnt)
						{
							gMotorExtReg.R2 = ((gRrLoIdentify.RrAndRsAccValue - gRrLoIdentify.RrAndRsMax - gRrLoIdentify.RrAndRsMin) / 4) - gMotorExtReg.R1;
							gMotorExtReg.L0 = (gRrLoIdentify.LoAccValue - gRrLoIdentify.LoMax - gRrLoIdentify.LoMin) / 4;
							if((gInvInfo.InvType > 18) && (gInvInfo.InvType < 23))
							{
								gMotorExtReg.R2 = ((long)gMotorExtReg.R2 * 768L)>>10;
							}
							else if(gInvInfo.InvType > 23)
							{
								gMotorExtReg.R2 = ((long)gMotorExtReg.R2 * 666L)>>10;
							}
							gGetParVarable.IdSubStep = 4;		//辨识结果完毕
						}
						else
						{
							gRrLoIdentify.WaitCnt = 20;
							RrGetVoltReBegin();				//没有完成6次采样，需要继续。
						}
					}
					else
					{
						gRrLoIdentify.WaitCnt = 20;
						RrGetVoltReBegin();					//比较或周期检测个数不对，重新来过
					}
				}
			}           
            break;
            
        case 4:
        default:
	        DisableDrive();               
            EALLOW;
            PieCtrlRegs.PIEIER3.all = 1;                    //关闭EPWM2中断
            PieVectTable.EPWM1_INT 	= &EPWM1_zero_isr;		//恢复对中断的修改
            EDIS;               
            gGetParVarable.IdSubStep = 1;
            gGetParVarable.ParEstMstep++;
	        InitSetPWM();         //恢复转子电阻辨识时对外设寄存器的修改
   	        InitSetAdc();
	        SetInterruptEnable();	            
            break;
    }
}

/*******************************************************************
函数功能: 
    重新设置PWM模块，准备转子电阻和漏感的辨识

********************************************************************/
void SetPwmForRrLoId(void)
{
    EALLOW;
    PieVectTable.EPWM1_INT = &RrLoInPeriodAndCmpInt;    // 必须先屏蔽中断，才能更改比较寄存器	
	EPwm1Regs.ETSEL.all = 0x0000A;		                //周期中断
    PieVectTable.EPWM2_INT = &RrLoInPeriodAndCmpInt;    
    EPwm2Regs.ETSEL.all = 0x000C;                       //借用EPWM2的比较中断 
    EPwm2Regs.ETPS.all  = 1;
    
	EPwm1Regs.TBPRD = 12000; 
	EPwm1Regs.TBCTL.bit.CTRMODE = 3;		// 暂时停止TB记数//
#ifdef DSP_CLOCK100
	EPwm1Regs.TBCTL.bit.CLKDIV = 1;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 5;      // PWM时钟周期为0.2us，与老程序一致
#endif
#ifdef DSP_CLOCK60
    EPwm1Regs.TBCTL.bit.CLKDIV = 0;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 6;
#endif
	EPwm1Regs.TBCTR = 0;	
    
	//EPwm1Regs.CMPB = 12000;				//初始脉冲值为200//
	EPwm1Regs.CMPA.half.CMPA = 200;
	EPwm1Regs.CMPCTL.all = 0x0100;		//下溢加载CMP值//
	EPwm1Regs.AQSFRC.all = 0x000C0;		//U相桥上下一直关闭，立即加载//
	EPwm1Regs.AQCSFRC.all = 0x00A;		//U相桥上下一直关闭//	
	EPwm1Regs.DBCTL.all = 0x0000;		//没有死区时间//
	EPwm1Regs.ETCLR.bit.INT = 1;		//清除中断标志//

	EPwm2Regs.TBPRD = 12000; 
    EPwm2Regs.TBCTL.bit.CTRMODE = 3;
#ifdef DSP_CLOCK100
	EPwm2Regs.TBCTL.bit.CLKDIV = 1;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 5;      //PWM时钟周期为0.2us，与老程序一致										/////////////PWM2//////////////
#endif
#ifdef DSP_CLOCK60
    EPwm2Regs.TBCTL.bit.CLKDIV = 0;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 6;
#endif
    EPwm2Regs.TBCTR = 0;
    EPwm2Regs.CMPA.half.CMPA = 200;
    EPwm2Regs.CMPCTL.all = 0x0100;
    EPwm2Regs.AQCTLA.all = 0;
	EPwm2Regs.AQSFRC.all = 0x00C0;	    
	EPwm2Regs.AQCSFRC.all = 0x0006;		//V相上桥一直关闭,下桥一直打开//
	EPwm2Regs.DBCTL.all = 0x0000;		//没有死区时间//
	EPwm2Regs.ETCLR.bit.INT = 1;

#ifdef DSP_CLOCK100
	EPwm3Regs.TBCTL.bit.CLKDIV = 1;
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = 5;      //PWM时钟周期为0.2us，与老程序一致                                        /////////////PWM3//////////////
#endif
#ifdef DSP_CLOCK60
    EPwm3Regs.TBCTL.bit.CLKDIV = 0;
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = 6;
#endif   
	EPwm3Regs.TBPRD = 12000; 
    EPwm3Regs.AQCTLA.all = 0;
	EPwm3Regs.AQSFRC.all = 0x00C0;		    
	EPwm3Regs.AQCSFRC.all = 0x0006;		//W相上桥一直关闭，下桥一直打开//
	EPwm3Regs.DBCTL.all = 0x0000;		//没有死区时间//
    PieCtrlRegs.PIEIER3.all = 3;    
	EDIS;
}

/*******************************************************************
函数功能:
    初始化转子电阻和漏感辨识过程中用到的变量
    
********************************************************************/
void InitRrLoVariable(void)
{
    int m_index;
    
	gRrLoIdentify.WaitCnt = 0;
    for(m_index=0; m_index<14; m_index++)
    {
        gRrLoIdentify.IsSampleValue[m_index] = 0;
    }
	gRrLoIdentify.UdcVoltage = gUDC.uDCFilter;//计算参数辨识时用的母线电压
	gRrLoIdentify.CurrentMax = 0;
	gRrLoIdentify.SampleTimes = 0;
    gRrLoIdentify.RrL07PulseOverSig = 0;
}

/*******************************************************************
函数功能 启动定时器，输出使能
********************************************************************/
void EnableTimer(void)
{
	EALLOW;
	EPwm1Regs.AQCTLA.all = 0x0021;		//下溢时置低，比较时置高
	EPwm1Regs.AQCSFRC.all = 0x008;		//U相下桥一直关闭    
    EPwm1Regs.TBCTR = 0;
    EPwm2Regs.TBCTR = 0;
	EPwm1Regs.TBCTL.bit.CTRMODE = 0;		//单增模式，并启动定时器的工作
	EPwm2Regs.TBCTL.bit.CTRMODE = 0;
	EDIS; 
	EINT;								//开中断    
    EnableDrive();	
}

/*******************************************************************
函数功能: 
    转子电阻和漏感辨识时，在周期中断和比较中断中检测电流和电压
该函数合并了原来的周期和下溢检测过程，需要测试是否对辨识有影响。

********************************************************************/
interrupt void RrLoInPeriodAndCmpInt(void)
{
	long		m_Iu;
   	
   	EALLOW;
   	EPwm1Regs.ETCLR.bit.INT = 1;
    EPwm2Regs.ETCLR.bit.INT = 1;
   	EDIS;
    
	EINT;								
    RrWaitAddGetUdc();
    gRrLoIdentify.UdcVoltage = Filter4(gUDC.uDCFilter, gRrLoIdentify.UdcVoltage);
	m_Iu = (int)(ADC_IU - (Uint)32768);
	m_Iu =  (m_Iu - (long)gExcursionInfo.ErrIu)>>1;	//去除零漂;Q14

	gRrLoIdentify.IsSampleValue[gRrLoIdentify.SampleTimes] = __IQsat(m_Iu, 32767, -32767);  
    if(gRrLoIdentify.CurrentMax < abs(m_Iu))
	{
		gRrLoIdentify.CurrentMax = abs(m_Iu);
	}
	gRrLoIdentify.SampleTimes++;  
    gRrLoIdentify.SampleTimes = __IQsat(gRrLoIdentify.SampleTimes, 15, 0);
    
	if(gRrLoIdentify.SampleTimes >= 14)
	{
		DisableDrive();						//发完7个脉冲，则关闭输出并初始化
        EALLOW;
        EPwm1Regs.TBCTL.bit.CTRMODE = 3;	//停止计数
		EPwm1Regs.TBCTR = 0;
		EPwm1Regs.AQCSFRC.all = 0x00A;		//U相桥上下恢惫乇?/
		EPwm2Regs.TBCTL.bit.CTRMODE = 3;
        EPwm2Regs.TBCTR = 0;

        //SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK = 0;
        //SysCtrlRegs.PCLKCR1.bit.EPWM2ENCLK = 0;
        //SysCtrlRegs.PCLKCR1.bit.EPWM3ENCLK = 0;
        //SysCtrlRegs.PCLKCR1.all &= 0xF8;    // disable ePWM clock
		EDIS;
		gRrLoIdentify.RrL07PulseOverSig = 1;	
    }    
    CBCLimitCurPrepare();
    DINT;
    
   	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;	// Acknowledge this interrupt
}

/*******************************************************************
函数功能 在延时等待过程中检测母线电压
********************************************************************/
void RrWaitAddGetUdc(void)
{
    Uint counter;
    
	EALLOW;
	ADC_RESET_SEQUENCE;		    //复位AD的计数器
	ADC_CLEAR_INT_FLAG;			//首先清除中断标志位
	ADC_START_CONVERSION;		//软件启动AD
	EDIS;

    counter = 0;
	while(ADC_END_CONVERSIN == 0)   // 等待AD转换完成
    {
        counter ++;
        if(counter > 50)
        {
            gError.ErrorCode.all |= ERROR_TUNE_FAIL;    // 调谐故障，退出调谐
            break;
        }
	}
    
    EALLOW;
    ADC_CLEAR_INT_FLAG;         // 清除AD中断
    EDIS;
    
    GetUDCInfo();    
}

void RrGetVoltReBegin(void)
{
	DisableDrive();
    gRrLoIdentify.PwmCompareValue = __IQsat(gRrLoIdentify.PwmCompareValue,2000,1);
    
    EALLOW;
	EPwm1Regs.CMPA.half.CMPA = gRrLoIdentify.PwmCompareValue;
    EPwm2Regs.CMPA.half.CMPA = gRrLoIdentify.PwmCompareValue;

	EPwm1Regs.TBCTL.bit.CTRMODE = 3;		//停止计数
	EPwm1Regs.TBCTR = 0;
	EPwm1Regs.AQCSFRC.all = 0x00A;		//U相桥上下一直关闭//
	EPwm2Regs.TBCTL.bit.CTRMODE = 3;
    EPwm2Regs.TBCTR = 0;

    //SysCtrlRegs.PCLKCR1.all |= 0x03;        // enable ePWM clockS
	EDIS;
}

/*******************************************************************
函数功能:
    根据采样到的电流电压值，计算转子电阻和漏感
********************************************************************/
void RrLoCalVaule(void)
{
    int     m_IdVoltage,m_zeroVoltageTime,m_index,m_CurrMultiple;
    long    m_RrRsCalValue,m_LoCalVAlue,m_IsUdcVoltDiff,m_IsZeroVoltDiff,m_IsUdcVoltageSum,m_IsZeroVoltageSum;
    long    m_Value1,m_Value2;
    m_IsUdcVoltageSum = 0;
    m_IsUdcVoltDiff = 0;
    m_IsZeroVoltageSum = 0;
    m_IsZeroVoltDiff = 0;
	m_IdVoltage = ((long)gRrLoIdentify.UdcVoltage * 43690L)>>16;	//母线电压的2/3，实际电压的10倍;
	m_zeroVoltageTime = 12000 - gRrLoIdentify.PwmCompareValue;		//输出电压为0的时间
    for(m_index=2;m_index<7;m_index++)
        m_IsUdcVoltDiff += ((long)gRrLoIdentify.IsSampleValue[2*m_index]-(long)gRrLoIdentify.IsSampleValue[2*m_index-1]);
    m_IsUdcVoltDiff = abs(m_IsUdcVoltDiff);     //6A  UDC下的电流变化
    for(m_index=2;m_index<7;m_index++)
        m_IsZeroVoltDiff += ((long)gRrLoIdentify.IsSampleValue[2*m_index+1]-(long)gRrLoIdentify.IsSampleValue[2*m_index]);
    m_IsZeroVoltDiff = abs(m_IsZeroVoltDiff);	//6B 电压为0下电流变化
	for(m_index=3;m_index<13;m_index++)
        m_IsUdcVoltageSum += (long)abs(gRrLoIdentify.IsSampleValue[m_index]);
    m_IsUdcVoltageSum = m_IsUdcVoltageSum>>1;   //5*I_udc
    for(m_index=4;m_index<14;m_index++)
        m_IsZeroVoltageSum += (long)abs(gRrLoIdentify.IsSampleValue[m_index]);
    m_IsZeroVoltageSum = m_IsZeroVoltageSum>>1; //5*I_0
		
	m_Value1 = m_IsUdcVoltageSum + ((long long)m_IsZeroVoltageSum * (long long)m_IsUdcVoltDiff * (long long)m_zeroVoltageTime) /
                (m_IsZeroVoltDiff * (long)gRrLoIdentify.PwmCompareValue);
            //实际电流计算方式  AD采样值*2*SQRT(2)*变频器额定电流>>14

    m_Value2 = (m_Value1 * (llong)gInvInfo.InvCurrent)>>14;
            
    #ifdef TMS320F2808
    m_Value2 = ((llong)m_Value2 * 5793L)>>11; //*sqrt(2);
    #else
    m_Value2 = ((llong)m_Value2 * 6372L)>>11; //*sqrt(2); 
    #endif

    m_Value2 = ((llong)m_Value2 * (long)gInvInfo.CurrentCoff) / 1000;	  
    m_RrRsCalValue = ((long)m_IdVoltage * 50000L)/ m_Value2;	 //定子电阻和转子电阻之和
    m_LoCalVAlue = ((long long)m_RrRsCalValue * (long long)m_IsZeroVoltageSum * (long long)m_zeroVoltageTime) / (m_IsZeroVoltDiff * 50000L); //漏感计算 
    m_LoCalVAlue = m_LoCalVAlue>>1;    //漏感计算完毕，假定定转子漏感相等，除以2得到转子漏感


    
	if(0 == gRrLoIdentify.RrL0CycleCnt)//记录最小最大值
	{
        gRrLoIdentify.LoMax = m_LoCalVAlue;
        gRrLoIdentify.LoMin = m_LoCalVAlue;
        gRrLoIdentify.RrAndRsMax = m_RrRsCalValue;
        gRrLoIdentify.RrAndRsMin = m_RrRsCalValue;
	}
	else
	{
	    gRrLoIdentify.LoMax = (m_LoCalVAlue > gRrLoIdentify.LoMax)?m_LoCalVAlue:gRrLoIdentify.LoMax;
        gRrLoIdentify.LoMin = (m_LoCalVAlue < gRrLoIdentify.LoMin)?m_LoCalVAlue:gRrLoIdentify.LoMin;
        gRrLoIdentify.RrAndRsMax = (m_RrRsCalValue > gRrLoIdentify.RrAndRsMax)?m_RrRsCalValue:gRrLoIdentify.RrAndRsMax;
        gRrLoIdentify.RrAndRsMin = (m_RrRsCalValue < gRrLoIdentify.RrAndRsMin)?m_RrRsCalValue:gRrLoIdentify.RrAndRsMin;
	}    
	gRrLoIdentify.RrAndRsAccValue += m_RrRsCalValue;
	gRrLoIdentify.LoAccValue += m_LoCalVAlue;
    gRrLoIdentify.RrL0CycleCnt++;
}

/************************************************************
	自适应计算异步机电流环的调节器增益(按照8KHz载波频率计算)
	m_kp_m = m_l1*m_Fc*m_In*4096.0/(2.6*64.0*m_Un);
	m_ki_m = m_res*m_In*65536.0/(0.65*64*m_Un);
	
	m_kp_t = m_l1*m_Fc*m_In*4096.0/(2.6*64.0*m_Un);
	m_ki_t = m_res*m_In*65536.0/(1.3*64*m_Un);
	
	量刚：电感-mH、电流-A、电压-V、电阻-欧姆、载波频率8KHz
	m_kp_m = m_l1*m_In*197/m_Un;
	m_ki_m = m_res*m_In*1575/m_Un;
	
	m_kp_t = m_l1*m_In*197/m_Un;
	m_ki_t = m_res*m_In*788/m_Un;
	
	量刚：电感-0.01mH、电流-0.01A、电压-V、电阻-0.001欧姆、载波频率8KHz
	m_kp_m = m_l1*m_In/(51*m_Un);
	m_ki_m = m_res*m_In/(64*m_Un) = (m_res*m_In/m_Un)>>6;
	
	m_kp_t = m_l1*m_In/(51*m_Un);
	m_ki_t = m_res*m_In/(128*m_Un) = (m_res*m_In/m_Un)>>7;

	m_l1 = L1[1-(Lm*Lm/L1*L1)] = Lo + Lo*Lm/L1  注意单位应该为0.01mH
************************************************************/
void ImCalKpKi(void)
{
	Ulong tempL;
    Ulong tempL1;
    Ulong temp;

    tempL1 = gMotorExtReg.L0/10 + gMotorExtReg.LM;
    tempL  = gMotorExtReg.L0 * gMotorExtReg.LM / tempL1 ;    // 0.01mH
    tempL += gMotorExtReg.L0;
//
	temp = ((Ulong)tempL * (Ulong)gMotorInfo.CurrentGet) / gMotorInfo.Votage;
    gPmParEst.IdKp = (Ulong)temp * 1290L >> 10; // * 1.26
	gPmParEst.IqKp = gPmParEst.IdKp;
    // 积分增益
	gPmParEst.IdKi = (((Ulong)gMotorExtReg.R1 * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Votage)>>1;    // * 0.504
	gPmParEst.IqKi = gPmParEst.IdKi >> 1;    //
}

/*******************************************************************
函数功能:
    空载电流和互感辨识，动态辨识
********************************************************************/
void LmIoIdentify(void)
{
    //Uint m_AbsFreq;
    switch(gGetParVarable.IdSubStep)
    {
        case 1:            
            InitLmIoVaribale();   //初始化空载辨识参数，主要是WaitCnt等循环变量
            gGetParVarable.IdSubStep++;
            
        case 2:         //空载辨识前的等待阶段，等待转子电阻辨识时电流回零
            gLmIoIdentify.WaitCnt++;
            if( 800 < gLmIoIdentify.WaitCnt )
            {
                gLmIoIdentify.WaitCnt = 0;
                gGetParVarable.IdSubStep++;
			    gGetParVarable.StatusWord = TUNE_ACC;  //开始加速                
                EnableDrive();
                //
                gPGData.imPgEstTick = 0;
                gPGData.imDirAdder = 0;
                gPGData.imFreqErr = 0;
                gPGData.imFrqEncoder = 0;
                gLmIoIdentify.VFOvShock = gVFPar.VFOvShock;
            }
            break;
            
        case 3:         //实际输出频率达到电机额定频率的80%
            if(gMainCmd.FreqSyn >= (gLmIoIdentify.DestinationFreq - 11))
            {

                if(gLmIoIdentify.VFOvShock != 0)
                 { 
                   gLmIoIdentify.VFOvShock--;
                   gVFPar.VFOvShock = gLmIoIdentify.VFOvShock;
                    break;
                  }
                else
                 {
                    if(gLmIoIdentify.WaitCnt < 500)
                    {
                        gLmIoIdentify.WaitCnt ++; 
                        break; 
                    }
                 }
                // 开始辨别编码器
                gPGData.imPgEstTick ++;
                // 辨别方向
                if((long)gFVCSpeed.SpeedEncoder * gMainCmd.FreqSyn < 0)
                {
                    //gPGData.PGDir = (gPGData.SpeedDir ^ 0x01);
                    gPGData.imDirAdder ++;
                }
                else
                {
                    gPGData.imDirAdder --;
                }
                // 辨别编码器测速
                gPGData.imFreqErr += (gMainCmd.FreqSyn - abs(gFVCSpeed.SpeedEncoder));
                gPGData.imFrqEncoder += abs(gFVCSpeed.SpeedEncoder);

                if(gPGData.imPgEstTick > 128)       // 最后判定
                {
                    int maxFrqErr;
                    
                    if(gPGData.imDirAdder > 50)
                    {
                        gPGData.PGDir = (gPGData.SpeedDir ^ 0x01);
                    }
                    else if(gPGData.imDirAdder > -50)
                    {
                        gPGData.PGErrorFlag = 1;        // 认为未接编码器
                    }
                    // else encoder is ok
                    maxFrqErr = (long)gLmIoIdentify.DestinationFreq * 100L >> 10;    // 100/1024 = 10%
                    if((gPGData.imFreqErr >> 7) > maxFrqErr)    // 2^7 = 128
                    {
                        gPGData.PGErrorFlag = 2;        // 测速不吻合，编码器线数出错
                    }
                    if((gPGData.imFrqEncoder>>7) < maxFrqErr)   // 10% AimFrq
                    {
                        gPGData.PGErrorFlag = 1;        // 认为未检测到编码器
                    }

                    gLmIoIdentify.WaitCnt = 0;
                    gLmIoIdentify.lImAccValue = 0;
                    gLmIoIdentify.lIsAccValue = 0;
                    gLmIoIdentify.DataSavedNum = 0;
			        gGetParVarable.IdSubStep++;
                    }
                }
            // ;
            break;          
            
        case 4:                 // 等待中断执行
              //gVFPar.VFOvShock = 0;
			if(gMainCmd.FreqSyn < (gLmIoIdentify.DestinationFreq - 20))
			{
                gGetParVarable.IdSubStep++; //辨识过程中速度低于设定值，异常退出
            }
            
	        if(120 <= gLmIoIdentify.WaitCnt)   
	        {
				gMotorExtReg.LM = (((gLmIoIdentify.LmAccValue * 10L) / gLmIoIdentify.WaitCnt) - (long)gMotorExtReg.L0) / 10;
				gMotorExtReg.I0 = gLmIoIdentify.IoAccValue / gLmIoIdentify.WaitCnt; //机型大于22，电流小数点位数不同
                #ifdef TMS320F28035
                gMotorExtReg.LM = ((long)gMotorExtReg.LM * 7447L) >> 13; //除以1.1
                gMotorExtReg.I0 = ((long)gMotorExtReg.I0 * 9011L) >> 13; //乘以1.1
                #endif
	            //gGetParVarable.StatusWord = TUNE_SUCCESS;    //开始减速
	            gGetParVarable.StatusWord = TUNE_DEC;
                gLmIoIdentify.WaitCnt = 0;
				gGetParVarable.IdSubStep++;                    
			}
            break;
            
        case 5:
            if( 8 > gMainCmd.FreqSyn )
            {
                DisableDrive();
                gGetParVarable.ParEstMstep++;
                gGetParVarable.IdSubStep = 1;

                // 计算电流环kp，ki
                ImCalKpKi();
            }            
            break;
            
    }

}

void InitLmIoVaribale(void)
{
    gLmIoIdentify.WaitCnt = 0;
    gLmIoIdentify.DestinationFreq = ((long)gMotorInfo.FreqPer<<3)/10;  //电机额定频率的80%
    gLmIoIdentify.LmAccValue = 0;
    gLmIoIdentify.IoAccValue = 0;
}

/*******************************************************************
函数功能 计算输入感抗和空载电流，并累加
********************************************************************/

void LmIoCalValue(void)
{
    long    m_ComIm,m_ComLma,m_ComLmb,m_ComLmc;
	m_ComIm  =  gLmIoIdentify.lIsAccValue / 100; //Q16
	m_ComLma = ((gLmIoIdentify.lImAccValue / 200L ) * 1448L)>>10;  //*sqrt(2)
    m_ComLmb = (m_ComIm * m_ComIm)>>13;
	m_ComLmc = ((long)gOutVolt.Volt * m_ComLma) / m_ComLmb;//VOLTAGE*IM*SIN /{[(IM*SIN)^2+(IM*COS)^2] / 2^12}
	m_ComLmc = m_ComLmc * (long)gMotorInfo.Votage * 10L / (long)gInvInfo.InvCurrent;
    m_ComLmc = (m_ComLmc * 739L)>>13;
    m_ComLmc = m_ComLmc * 1000L / (long)gInvInfo.CurrentCoff;    
    m_ComLmb = (m_ComLmc * 10000L) / (long)gMainCmd.FreqReal;   			
	m_ComLmc = ((m_ComIm * (long)gInvInfo.InvCurrent)>>15) * (long)gInvInfo.CurrentCoff / 707L;//((AD*Iv)*Igain*2*2^0.5/1000)>>16
    gLmIoIdentify.LmAccValue += m_ComLmb;
    gLmIoIdentify.IoAccValue += m_ComLmc;
   	gLmIoIdentify.WaitCnt++;
}

/*******************************************************************
函数功能 完成电流的矢量变换，并累加电流值
********************************************************************/
void LmIoInPeriodInt(void)
{
    long    m_lIm,m_lIt,m_lIs;
    int     m_VoltagePhase;
    if(( 4 != gGetParVarable.IdSubStep )||
        ( 120 <= gLmIoIdentify.WaitCnt ))
    {       
        return;
    }
    m_VoltagePhase = gPhase.IMPhase>>16;
   	m_lIm = abs(((long)gExcursionInfo.IvValue * (long)qsin(m_VoltagePhase)                     
        		- (long)gExcursionInfo.IuValue * ((long)qsin(m_VoltagePhase - 21845)))>>15);//Q15 
	m_lIt = abs((long)gExcursionInfo.IvValue * (long)qsin(16384+m_VoltagePhase)
				- (long)gExcursionInfo.IuValue * (long)qsin(m_VoltagePhase - 5461)>>15);//Q15           
	m_lIs = ( 827L * (long)qsqrt(m_lIm * m_lIm + m_lIt * m_lIt))>>10;
    gLmIoIdentify.lIsValue = Filter16(m_lIs,gLmIoIdentify.lIsValue);       
    m_lIs = abs(((long)gLmIoIdentify.lIsValue * (long)qsin(16384 - gIAmpTheta.Theta))>>15);        
    gLmIoIdentify.lIsAccValue += gLmIoIdentify.lIsValue;
	gLmIoIdentify.lImAccValue += m_lIs;
 
    gLmIoIdentify.DataSavedNum++;
    if( 200 <= gLmIoIdentify.DataSavedNum )
    {
        LmIoCalValue();
        gLmIoIdentify.lIsAccValue = 0;
        gLmIoIdentify.lImAccValue = 0;
        gLmIoIdentify.DataSavedNum = 0;
    }    
}

void ChgParForEst(void)
{
    if(MOTOR_TYPE_PM != gMotorInfo.MotorType)
    {
        //gMainCmd.Command.bit.ControlMode = IDC_VF_CTL;
        gCtrMotorType = ASYNC_VF;
       if (500 < gInvInfo.InvTypeSet)
        {
         gBasePar.FcSet = 10;                            // 异步机参数辨识, 1140V载频固定为1.0KHz
        }
       else if (400 < gInvInfo.InvTypeSet)
        {
         gBasePar.FcSet = 20; 
        }
       else
        {
         gBasePar.FcSet = 30;                            // 异步机参数辨识, 载频固定为3.0KHz
        }
         gVFPar.VFWsComp = 0;
         gVFPar.VFLineType = 0;
         gVFPar.VFTorqueUp = 10;
         gPWM.SoftPWMTune = 0;
    }
    else                // pmsm
    {
       if (500 < gInvInfo.InvTypeSet)
       {
        gBasePar.FcSet =  10;                             // 异步机参数辨识, 1140V载频固定为1.0KHz
       }
       else if (400 < gInvInfo.InvTypeSet)
       {
        gBasePar.FcSet = 20; 
       }
       else
       {
        gBasePar.FcSet = (gBasePar.FcSet < 30) ? 30 : gBasePar.FcSet;       // min 3.0KHz
       }
    }
}

