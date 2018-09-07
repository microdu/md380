/****************************************************************
文件功能: 异步机和同步机参数辨识公共变量和函数定义
文件版本： 
最新更新： 

*************************************************************/
#include "MotorParaIDinclude.h"
#include "MotorPmsmParEst.h"
#include "MotorEncoder.h"

// // 全局变量定义
UV_BIAS_COFF_STRUCT		 gUVCoff;
MOTOR_EXTERN_STRUCT		 gMotorExtReg;	            //电机扩展信息（电机参数辨识得到的数据）
MOTOR_PARA_EST           gGetParVarable;
IDENTIFY_RRLO_VARIABLE   gRrLoIdentify;
IDENTIFY_LMIO_VARIABLE   gLmIoIdentify;

// // 异步机静态辨识顺序
PAR_EST_MAIN_STEP const AsynParEstStatic[IDENTIFY_PROGRESS_LENGTH] =
{
    IDENTIFY_RS,
    IDENTIFY_RR_LO,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END
};

// // 异步机完整辨识顺序
PAR_EST_MAIN_STEP const AsynParEstRotor[IDENTIFY_PROGRESS_LENGTH] =   
{
    IDENTIFY_RS, 
    IDENTIFY_RR_LO,
    IDENTIFY_LM_IO,
    IDENTIFY_END,
    IDENTIFY_END
};

// // 同步机空载辨识顺序
PAR_EST_MAIN_STEP const pmTuneProgNoLoad[IDENTIFY_PROGRESS_LENGTH] = 
{
    IDENTIFY_RS,
    PM_EST_POLSE_POS,
    PM_EST_NO_LOAD,
    PM_EST_BEMF,
    //IDENTIFY_END,
    //IDENTIFY_END,
    //IDENTIFY_END,
    IDENTIFY_END
};

// // 同步机带载辨识顺序
PAR_EST_MAIN_STEP const pmTuneProgLoad[IDENTIFY_PROGRESS_LENGTH] = 
{
    IDENTIFY_RS,
    PM_EST_POLSE_POS,
    PM_EST_WITH_LOAD,
    //IDENTIFY_END,
    //IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END
};

PAR_EST_MAIN_STEP const noTuneProgress[IDENTIFY_PROGRESS_LENGTH] =      
{
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END
};

PAR_EST_MAIN_STEP const debugTuneProcess[IDENTIFY_PROGRESS_LENGTH] = 
{
    PM_EST_BEMF,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END
};

// // 内部函数声明
void EndOfParIdentify(void);

/****************************************************************
    函数功能：参数辨识主循环结构控制
    
*****************************************************************/
void RunCaseGetPar(void)
{    
// 故障或者功能停止调谐则停止
	if((gError.ErrorCode.all != 0) || (gMainCmd.Command.bit.Start == 0))
	{
		DisableDrive();
        DINT;                                       // 暂时停止中断， 等中断重新设置
        
        gGetParVarable.ParEstContent[gGetParVarable.ParEstMstep] = IDENTIFY_END;
        gGetParVarable.StatusWord = TUNE_FINISH;
	}
    
// 继续调谐
    switch(gGetParVarable.ParEstContent[gGetParVarable.ParEstMstep])
    {
       case IDENTIFY_RS:
           RsIdentify();
           break;
           
       case IDENTIFY_RR_LO:
            RrLoIdentify();
            break;
            
       case IDENTIFY_LM_IO:
            LmIoIdentify();
            break;
            
       case PM_EST_POLSE_POS:
			SynTuneInitPos();
            break;
            
	   case PM_EST_NO_LOAD:
			SynTunePGZero_No_Load();           
			break;

        case PM_EST_WITH_LOAD:
            SynTunePGZero_Load();
            break;

        case PM_EST_BEMF:
            SynTuneBemf();
            break;
            
       default: 
            EndOfParIdentify();
            break;            
    }
    
    ParSendTune();    
}

/************************************************************
	参数辨识需要的参数的初始化
	
************************************************************/
void PrepareParForTune(void)
{
    int m_index;
    PAR_EST_MAIN_STEP *m_PIdentifyFlow;
    
    //所有参数辨识过程中返回的变量，都要预先赋值，否则会导致对应的功能码出错
	gUVCoff.UDivVSave   = gUVCoff.UDivVGet;
    
	gMotorExtReg.R1     = gMotorExtInfo.R1;         // IM motor
    gMotorExtReg.R2     = gMotorExtInfo.R2;
    gMotorExtReg.L0     = gMotorExtInfo.L0;
    gMotorExtReg.LM     = gMotorExtInfo.LM;
    gMotorExtReg.I0     = gMotorExtInfo.I0;

    gMotorExtReg.RsPm   = gMotorExtInfo.RsPm;       // PM motor
    gMotorExtReg.LD     = gMotorExtInfo.LD;         
    gMotorExtReg.LQ     = gMotorExtInfo.LQ;
    gEstBemf.BemfVolt   = gMotorExtInfo.BemfVolt;               // PM 转子磁链 %

    gPGData.PGDir               = gPGData.SpeedDir;
    gPGData.PGErrorFlag         = 0;
    gPmParEst.CoderPos_deg      = gIPMPos.RotorZeroGet;
    gPmParEst.UvwDir            = gUVWPG.UvwDir;
    gPmParEst.UvwZeroAng_deg       = gUVWPG.UvwZeroPos_deg;

    gPmParEst.IdKp = gVCPar.AcrImKp;
    gPmParEst.IdKi = gVCPar.AcrImKi;
    gPmParEst.IqKp = gVCPar.AcrItKp;
    gPmParEst.IqKi = gVCPar.AcrItKi;
    
    gGetParVarable.ParEstMstep = 0;
    gGetParVarable.StatusWord = TUNE_INITIAL;
    gGetParVarable.IdSubStep = 1;                               // 子过程步骤
    gUVCoff.IdRsCnt = 0;
    gUVCoff.IdRsDelay = 0;    

    gIPMZero.DetectCnt = 0;                 // must be initiated
    gGetParVarable.QtEstDelay = 0;
            
    switch(gGetParVarable.TuneType)
    {
        case TUNE_IM_STATIC:
            m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)AsynParEstStatic;
            break;
            
        case TUNE_IM_ROTOR:
            m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)AsynParEstRotor;
            break;
            
        case TUNE_PM_COMP_LOAD:
			m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)pmTuneProgLoad;
			break;
            
        case TUNE_PM_COMP_NO_LOAD:
            m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)pmTuneProgNoLoad;
            break;

        case TUNE_PM_PARA_temp:         //rt debug
            m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)debugTuneProcess;
			break;
            
		default:            
             m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)noTuneProgress;
            break;
    }
    
    for(m_index=0; m_index < IDENTIFY_PROGRESS_LENGTH; m_index++)
    {
        gGetParVarable.ParEstContent[m_index] = *(m_PIdentifyFlow + m_index);
    }
    
    InitRrLoVariable();                             //初始化转子辨识的变量，主要是WaitCnt循环变量
    
}

/*******************************************************************
    函数功能: 结束参数辨识，恢复对外部模块的修改，准备运行
    延迟推出，避免退出参数辨识后反复进入
********************************************************************/
void EndOfParIdentify(void)
{ 
    if(gGetParVarable.QtEstDelay == 0)
    {
        gGetParVarable.ParEstMstep = 0;
        gMainStatus.PrgStatus.all = 0;
        if(TUNE_FINISH != gGetParVarable.StatusWord)
        {
            gGetParVarable.StatusWord = TUNE_SUCCESS;
        }
    	DisableDrive();
        
        EALLOW;  						                //设置用户服务程序
        PIE_VECTTABLE_ADCINT = &ADC_Over_isr;		    //ADC结束中断--INT1
        PieVectTable.EPWM1_TZINT = &EPWM1_TZ_isr;		//过流中断--INT2
        PieVectTable.EPWM1_INT 	= &EPWM1_zero_isr;		//下溢中断--INT3
        PieCtrlRegs.PIEIER3.bit.INTx2 = 0;              //关闭EPWM2中断
        EDIS;
        
    	InitSetPWM();
       	InitSetAdc();
        SetInterruptEnable();	                        // 如果辨识项目中途退出，中断有可能是关闭的，须在此打开
       	EINT;   							    
       	ERTM;
    
    }
    else if(gGetParVarable.QtEstDelay >= 5)
    {
        if(gPGData.PGErrorFlag && gMainCmd.Command.bit.ControlMode == IDC_FVC_CTL)
        {
            gError.ErrorCode.all |= ERROR_ENCODER;      // 异步机辨识时速度反馈出错
            gError.ErrorInfo[4].bit.Fault1 = 8 +gPGData.PGErrorFlag;   // 9: 未接编码器   
                                                                       // 10:编码器线数错误
            gPGData.PGErrorFlag = 0;
        }   

        gMainStatus.RunStep = STATUS_STOP;
    	gGetParVarable.IdSubStep = 1;
    }

    gGetParVarable.QtEstDelay ++;
}


