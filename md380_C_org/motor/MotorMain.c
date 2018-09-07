/****************************************************************
ÎÄ¼þ¹¦ÄÜ£ººÍµç»ú¿ØÖÆÏà¹ØµÄ³ÌÐòÎÄ¼þ£¬µç»ú¿ØÖÆÄ£¿éµÄÖ÷Ìå²¿·Ö
ÎÄ¼þ°æ±¾£º 
¸üÐÂÈÕÆÚ£º 

****************************************************************/
#include "MotorInclude.h"
#include "MotorDataExchange.h"
#include "SystemDefine.h"
#include "MotorDefine.h"

// // ÄÚ²¿º¯ÊýÉùÃ÷
void RunCaseDeal05Ms(void);
void RunCaseDeal2Ms(void);
void RunCaseRun05Ms(void);
void RunCaseRun2Ms(void);
void RunCaseStop(void);
void SendDataPrepare(void);		

/************************************************************
    ÐÔÄÜÄ£¿é³õÊ¼»¯³ÌÐò£ºÖ÷Ñ­»·Ç°³õÊ¼»¯ÐÔÄÜ²¿·ÖµÄ±äÁ¿
(ËùÓÐµÈÓÚ0µÄ±äÁ¿ÎÞÐè³õÊ¼»¯)
************************************************************/
void InitForMotorApp(void)
{
	DisableDrive();
	TurnOffFan();
    
// ¹«¹²±äÁ¿³õÊ¼»¯
	gMainStatus.RunStep = STATUS_LOW_POWER;	//Ö÷²½Öè
	gMainStatus.SubStep = 1;				//¸¨²½Öè
	gMainStatus.ParaCalTimes = 0;
	gError.LastErrorCode = gError.ErrorCode.all;
	//gMainStatus.StatusWord.all = 0;
	gCBCProtect.EnableFlag = 1;				//Ä¬ÈÏÆô¶¯Öð²¨ÏÞÁ÷¹¦ÄÜ
	gADC.ZeroCnt = 0;
	gADC.DelaySet = 100;
	gADC.DelayApply = 600;
	gFcCal.FcBak = 50;
	gBasePar.FcSetApply = 50;
	gUVCoff.UDivV = 4096;
    gPWM.gPWMPrd = C_INIT_PRD;

    gMainStatus.StatusWord.all = 0;
    
// Ê¸Á¿Ïà¹Ø±äÁ¿³õÊ¼»¯
    gRotorTrans.Flag = 0;   //Í¬²½»ú³õÊ¼»¯, Ðý±ä²âËÙÓÃµ½
    gFVCSpeed.MTCnt = 0;
    gFVCSpeed.MTLimitTime = 0;
    gFVCSpeed.MSpeedSmooth.LastSpeed = 0;
    gFVCSpeed.MSpeedSmooth.SpeedMaxErr = 1500;
    gFVCSpeed.MTSpeedSmooth.LastSpeed = 0;
    gFVCSpeed.MTSpeedSmooth.SpeedMaxErr = 1500;
    gFVCSpeed.TransRatio = 1000;                  // ²âËÙ´«¶¯±È¹Ì¶¨Öµ
    gPGData.QEPIndex = QEP_SELECT_NONE; 
    gPGData.PGType = PG_TYPE_NULL;                  // ³õÊ¼»¯Îª null£¬
    gPWM.gZeroLengthPhase = ZERO_VECTOR_NONE; 

// µç»úÀàÐÍÏà¹ØµÄ³õÊ¼»¯£¬Ä¬ÈÏÊÇÒì²½»úµç»ú
    gMotorInfo.MotorType = MOTOR_TYPE_IM;
    //gMotorInfo.LastMotorType = gMotorInfo.MotorType;
    gMotorInfo.LastMotorType = MOTOR_NONE;                // ±£Ö¤½øÈëÖ÷³ÌÐòºóÄÜ½øÐÐÏà¹Ø³õÊ¼»¯
    
    //(*EQepRegs).QEINT.all = 0x0;  //È¡ÏûQEPµÄIÐÅºÅÖÐ¶Ï
    
    gPGData.PGMode = 0;
    gFVCSpeed.MDetaPosBak = 0;

    gIPMPos.ZErrCnt = 0	;
    gIPMPosCheck.UvwRevCnt = 0	;
    gIPMInitPos.Flag = 0;

    gIPMInitPos.InitPWMTs = (50 * DSP_CLOCK);	  //500us
    gPWM.PWMModle = MODLE_CPWM;

    gIPMPos.Zcounter  = 0;              // ¼ÇÂ¼½øÈëzÖÐ¶ÏµÄ´ÎÊý£¬ÓÃÓÚ¼àÊÓABZ,UVW ±àÂëÆ÷Í¬²½»ú±æÊ¶²»³É¹¦µÄÎÊÌâ
                                        // ²é¿´uf-25;
//
    ParSend2Ms();
    ParSend05Ms();
}

/************************************************************
Ö÷³ÌÐò²»µÈ´ýÑ­»·£ºÓÃÓÚÖ´ÐÐÐèÒª¿ìËÙË¢ÐÂµÄ³ÌÐò£¬ÒªÇó³ÌÐò·Ç³£¼ò¶Ì
************************************************************/
void Main0msMotor(void)
{
    if(gPGData.PGMode == 0)
    {
    GetMDetaPos();
    GetMTTimeNum();
    }
}

/************************************************************
ÈÎÎñ:
1. ±àÂëÆ÷²âËÙ¡¢SVCËÙ¶È¼ÆËã£»
2. ËÙ¶È±Õ»·¿ØÖÆ£»
3. ÏòÏÂ¼ÆËãgMainCmd.FreqSetApply£¬ ÏòÉÏ´«µÝgMainCmd.FreqToFunc£»

4. SVC ²»½øÐÐ±àÂëÆ÷²âËÙ£¬ FVCºÍVF»á¼ÆËã±àÂëÆ÷ËÙ¶È£»

************************************************************/
void Main05msMotor(void)
{
  if(ASYNC_SVC == gCtrMotorType)  // SVC
    {
        //gFVCSpeed.SpeedEncoder = 0;
        VCGetFeedBackSpeed();               //±àÂëÆ÷²âËÙ£¬È·±£¼ä¸ô¾ùÔÈ        
    }
            // 2808Ê±£¬SVC±àÂëÆ÷Ò²²âËÙ
    else
    {
        VCGetFeedBackSpeed();               //±àÂëÆ÷²âËÙ£¬È·±£¼ä¸ô¾ùÔÈ        
    }    

    #ifdef MOTOR_SYSTEM_DEBUG
    DebugSaveDeal(3);
    #endif 
}
/************************************************************
Ö÷³ÌÐòµÄµ¥2msÑ­»·£ºÓÃÓÚÖ´ÐÐµç»ú¿ØÖÆ³ÌÐò
Ë¼Â·£ºÊý¾ÝÊäÈë->Êý¾Ý×ª»»->¿ØÖÆËã·¨->¹«¹²±äÁ¿¼ÆËã->×ÔÎÒ±£»¤->¿ØÖÆÊä³ö
Ö´ÐÐÊ±¼ä£º²»±»ÖÐ¶Ï´ò¶ÏµÄÇé¿öÏÂÔ¼120us
************************************************************/
void Main2msMotorA(void)
{

//´Ó¿ØÖÆ°å»ñÈ¡²ÎÊý	
	ParGet2Ms();
    ParGet05Ms();

//Òì²½»ú²ÎÊý±æÊ¶Ê±£¬¶Ô²ÎÊýÉèÖÃÓÐÌØÊâÒªÇó£¬ÐèÒªÓÅ»¯
    if(STATUS_GET_PAR == gMainStatus.RunStep)
    {
        ChgParForEst();
    }
    
//ParameterChange2Ms();
    if(gMainCmd.Command.bit.Start == 0)
    {
        SystemParChg2Ms();
        SystemParChg05Ms();                 // ÔËÐÐÊ±²»×ª»»µÄ²ÎÊý

        ChangeMotorPar();       //µç»ú²ÎÊý×ª»»£¬ ÔËÐÐ²»×ª»»
    }
    RunStateParChg2Ms();
    //RunStateParChg05Ms();

}

void Main2msMotorB(void)
{  
  int     m05HzPu;
  int     m20HzPu;
    m20HzPu = (200L<<15) / gBasePar.FullFreq01;

	if(gIPMZero.zFilterCnt)				    // pm Z filter	
	{
		gIPMZero.zFilterCnt--;  
	}
    gMainCmd.FreqSetApply = (long)gMainCmd.FreqSet;
//    gMainCmd.FreqSetApply = Filter2((long)gMainCmd.FreqSet,gMainCmd.FreqSetApply);
    switch(gCtrMotorType)
    {
        case ASYNC_SVC:
            if(gMainStatus.RunStep != STATUS_SPEED_CHECK)          // SVC speed-check
            {   
             	if(0 == gVCPar.SvcMode)  //Ê¹ÓÃÔ­380Ëã·¨£¬ÒÔ±ãÓÚºÍ320¼æÈÝ
                {   
                    m05HzPu = (50L<<15) / gBasePar.FullFreq01;
                    if((abs(gMainCmd.FreqSet)+3) < m05HzPu) 
                    {
                        gMainCmd.FreqSet = 0;
                        gMainCmd.FreqSetApply = 0;
                    }
                    SVCCalRotorSpeed();
                    VcAsrControl();
                    CalWsAndSynFreq();   // ¼ÆËã×ª²îÆµÂÊ
                }
    			else
    			{
                    if((1 == gMainCmd.Command.bit.TorqueCtl)||(1 == gTestDataReceive.TestData1))
					{}
					else
					{
                        m05HzPu = (50L<<15) / gBasePar.FullFreq01;
                        if((abs(gMainCmd.FreqSet)+3) < m05HzPu) 
                        {
                            gMainCmd.FreqSet = 0;
                            gMainCmd.FreqSetApply = 0;
                        }
					}
                    SVCCalRotorSpeed_380();
                    VcAsrControl();
                    CalWsAndSynFreq_380();   // ¼ÆËã×ª²îÆµÂÊ
    			
    			}
                gMainCmd.FreqToFunc = gMainCmd.FreqFeed;
                }
            else
            {
                gMainCmd.FreqSyn = gFeisu.SpeedCheck;
                gMainCmd.FreqToFunc = gFeisu.SpeedCheck;
                
               //IMTSet.M = (long)gMotorExtPer.IoVsFreq <<12;
                gIMTSet.T = 0L <<12;
                gOutVolt.VoltApply = (long)gOutVolt.VoltApply * gFeisu.VoltCheck >>12;
            }
            break;
            
        case ASYNC_FVC:
            VcAsrControl();     // FVC ËÙ¶È»·
            CalWsAndSynFreq();  // ¼ÆËã×ª²îÆµÂÊ
            gMainCmd.FreqToFunc = gMainCmd.FreqFeed;
            break;

        case SYNC_SVC:
        case SYNC_VF:
        case ASYNC_VF:
            if(gMainStatus.RunStep == STATUS_SPEED_CHECK)
            {                
                gOutVolt.Volt           = gFeisu.VoltCheck;
                gMainCmd.FreqSyn      = gFeisu.SpeedCheck;
                gMainCmd.FreqToFunc     = gMainCmd.FreqSyn;
                gVFPar.FreqApply        = gMainCmd.FreqSyn;
                gOutVolt.VoltPhaseApply = (gFeisu.SpeedLast > 0) ? 16384 : -16384;                
            }            
            break;

        case SYNC_FVC:
//            gMainCmd.FreqFeed = gFVCSpeed.SpeedEncoder;
            VcAsrControl();     // synFVC ËÙ¶È»·
            gMainCmd.FreqSyn = gMainCmd.FreqFeed;
            gMainCmd.FreqToFunc = gMainCmd.FreqFeed;
            break;           

        case DC_CONTROL:
            gMainCmd.FreqSyn = 0;
            RunCaseDcBrake();
            gOutVolt.VoltPhaseApply = 0;        // ¿¼ÂÇµ½Í¬²½»ú£¬Êä³öµçÑ¹¶Ô×¼×ª×Ó´Å¼«£¬Ö±Á÷µçÁ÷¾Í»áÔÚ¸Ã·½Ïò
		                                        // ¶¨×Ó´ÅÁ´¾Í»áÔÚ¸Ã·½ÏòÉÏ£»
            gMainCmd.FreqToFunc = 0;		    
            break;

        case RUN_SYNC_TUNE:  // Ä¿°Ö÷ªÊÇÍ¬²½»ú²ÎÊý±æÊ¶
            ;
            // ²ÎÊý±æÊ¶ºó£¬µçÁ÷»·²ÎÊýÐèÒªµ÷ÓÃ±æÊ¶µÃµ½µÄpi²ÎÊý, µÃµ½pi²ÎÊýÇ°µÄ±æÊ¶¹ý³Ì²»»áÊ¹ÓÃµçÁ÷»·
            gImAcrQ24.KP = (long)gPmParEst.IdKp * gBasePar.FcSetApply / 80;                
            gItAcrQ24.KP = (long)gPmParEst.IqKp * gBasePar.FcSetApply / 80;
            gImAcrQ24.KI = gPmParEst.IdKi;
            gItAcrQ24.KI = gPmParEst.IqKi;
            
           
            
            if((TUNE_STEP_NOW == PM_EST_NO_LOAD) ||                 // pm ¿ÕÔØ±æÊ¶±àÂëÆ÷½Ç¶È
                (TUNE_STEP_NOW == PM_EST_BEMF))                      // pm ·´µç¶¯ÊÆ±æÊ¶£¬
            {           
                gMainCmd.FreqSyn = 0;
                gIMTSet.T = 0;

                if(TUNE_STEP_NOW == PM_EST_BEMF)
                {
                    gMainCmd.FreqSyn = gEstBemf.TuneFreqSet;
                    gEstBemf.IdSetFilt = Filter4(gEstBemf.IdSet, gEstBemf.IdSetFilt);
                    gIMTSet.M = (long)gEstBemf.IdSetFilt << 12;       // Q12->Q24
                }
                gMainCmd.FreqToFunc = gMainCmd.FreqSyn;
            }
            
            if(TUNE_STEP_NOW == PM_EST_WITH_LOAD)       // pm ´øÔØ±æÊ¶
            {
                PrepareAsrPar(); 
                CalTorqueLimitPar();
                CalUdcLimitIT();                   //Ê¸Á¿¿ØÖÆµÄ¹ýÑ¹ÒÖÖÆ¹¦ÄÜ
                VcAsrControl();                         // synFVC ËÙ¶È»·
                
                gIMTSet.M = 0;
                gMainCmd.FreqSyn = gMainCmd.FreqFeed;
                gMainCmd.FreqToFunc = gMainCmd.FreqFeed;
            }
                
            break;

        default:
            gMainCmd.FreqSyn = 0;
            gMainCmd.FreqToFunc = 0;
            break;
    }

    //¼ÆËãÔØ²¨ÆµÂÊ
	CalCarrierWaveFreq();

    // ÉèÖÃ¿ØÖÆÄ£Ê½ºÍµç»úÀàÐÍµÄ×éºÏ£¬ÓÃÓÚ¿ØÖÆÂß¼­µÄÇø·Ö
	if(MOTOR_TYPE_PM != gMotorInfo.MotorType)  
    {   
        if(gMainStatus.RunStep != STATUS_GET_PAR)
        {
            gCtrMotorType = (CONTROL_MOTOR_TYPE_ENUM)gMainCmd.Command.bit.ControlMode;
        }
        else        // im tune
        {
            gCtrMotorType = ASYNC_VF;
        }        
    }
    else if(MOTOR_TYPE_PM ==gMotorInfo.MotorType)
    {
        gCtrMotorType = (CONTROL_MOTOR_TYPE_ENUM)(gMainCmd.Command.bit.ControlMode + 10);
    }
    // Ö±Á÷ÖÆ¶¯
    if((1 == gMainCmd.Command.bit.StartDC) || (1 == gMainCmd.Command.bit.StopDC))
    {
        gCtrMotorType = DC_CONTROL;
    }

    //¸ù¾Ý±äÆµÆ÷×´Ì¬·Ö±ð´¦Àí, ¿ÉÄÜ»áÖØÐÂ¸üÐÂ gCtrMotorType(µ«±ØÐëÔÚ¸Ã2msº¯ÊýÖÐ£¬²»È»»áµ¼ÖÂ´íÎó)
	switch(gMainStatus.RunStep)
	{
		case STATUS_RUN:		                    //ÔËÐÐ×´Ì¬£¬Çø·ÖVF/FVC/SVCÔËÐÐ
			RunCaseRun2Ms();
			break;

        case STATUS_STOP:
            RunCaseStop();
            break;

        case STATUS_IPM_INIT_POS:                   //Í¬²½»ú³õÊ¼Î»ÖÃ½Ç¼ì²â½×¶Î
			RunCaseIpmInitPos();            
            break;
            
		case STATUS_SPEED_CHECK:                    //×ªËÙ¸ú×Ù×´Ì¬
		
			if(gComPar.SpdSearchMethod == 3)    RunCaseSpeedCheck();
            else                                RunCaseSpeedCheck();
			break;

		case STATUS_GET_PAR:	                    //²ÎÊý±æÊ¶×´Ì¬£¬ÒÆµ½0.5msÊ±ÒªÍ¬Ê±ÐÞ¸Ä²ÎÊý´«µÝ
			RunCaseGetPar();

            if(TUNE_STEP_NOW == IDENTIFY_LM_IO)
            { 
                 
        		VfOverCurDeal();
        		VfOverUdcDeal();
        		VfFreqDeal();                        // gVFPar.FreqApply

                gMainCmd.FreqToFunc = gVFPar.FreqApply;
                gMainCmd.FreqSetApply = gVFPar.FreqApply;

                gWsComp.CompFreq = 0;       // ×ª²î²¹³¥Îª0£¬ ×ª¾ØÌáÉýÒ²Îª0
                VFSpeedControl();
                CalTorqueUp(); 
                HVfOscDampDeal();             // HVf Õñµ´ÒÖÖÆ£¬ ²úÉúµçÑ¹ÏàÎ»£¬È¡ÏûMD320Õðµ´ÒÖÖÆ·½Ê½£¬2011.5.7 L1082
                gOutVolt.VoltPhaseApply = gHVfOscDamp.VoltPhase;            
                gOutVolt.Volt = gHVfOscDamp.VoltAmp;   
                VFOverMagneticControl();     
            }
			break;

		case STATUS_LOW_POWER:	                    //ÉÏµç»º³å×´Ì¬/Ç·Ñ¹×´Ì¬
			RunCaseLowPower();
			break;
            
		case STATUS_SHORT_GND:	                    //ÉÏµç¶ÔµØ¶ÌÂ·ÅÐ¶Ï×´Ì¬
			RunCaseShortGnd();
			break;
                       
		default:
            gMainStatus.RunStep = STATUS_STOP;      // ÉÏµçµÚÒ»ÅÅ»á³öÏÖ
			break;
	}	
}

void Main2msMotorC(void)
{
    InvCalcPower();     // ¹¦ÂÊ¡¢×ª¾ØµÄ¼ÆËã
    VfOscIndexCalc();
    
//±äÆµÆ÷×ÔÉí¼ì²âºÍ±£»¤	
	InvDeviceControl();			
}

void Main2msMotorD(void)
{
//µçÁ÷ÁãÆ¯¼ì²â£¬ADÁãÆ¯ºÍÏßÐÔ¶È¼ì²â	
	GetCurExcursion();				    

//×¼±¸ÐèÒª´«ËÍ¸ø¿ØÖÆ°åµÄÊý¾Ý
    SendDataPrepare(); 
    
//°ÑÊµÊ±Êý¾Ý´«ËÍ¸ø¿ØÖÆ°å	
	ParSend2Ms();
    ParSend05Ms();

    gCpuTime.CpuBusyCoff = (Ulong)gCpuTime.Det05msClk * 655 >> 16;  // div100
    gCpuTime.CpuCoff0Ms = gCpuTime.tmp0Ms;
    gCpuTime.tmp0Ms = 0;
// End

    #ifdef MOTOR_SYSTEM_DEBUG
    DebugSaveDeal(2);
    #endif 
}

/*************************************************************
	Îª¹¦ÄÜÄ£¿é×¼±¸ÐèÒªµÄËùÓÐ²ÎÊý
*************************************************************/
void SendDataPrepare(void)		
{
    Uint tempU;
    int   mAiCounter;
    Ulong mTotal1;
    Ulong mTotal2;
    Ulong mTotal3;
    Uint   mRatio;
    
	///////////////////////////////////////////////Í£»úÊ±ºòÏÔÊ¾µçÁ÷Îª0´¦Àí
	if((gMainStatus.RunStep == STATUS_LOW_POWER) ||
	   (gMainStatus.RunStep == STATUS_STOP) ||
	   (gLineCur.CurBaseInv < (4096/50))    ||          //¼ì²âµçÁ÷Ð¡ÓÚ±äÆµÆ÷¶î¶¨µçÁ÷2%£¬ÏÔÊ¾0
	   (1 == gMainStatus.StatusWord.bit.OutOff ))	
	{
		gLineCur.CurPerShow = 0;
        gLineCur.CurTorque  = 0;
	}
	else
	{
		gLineCur.CurPerShow = gLineCur.CurPerFilter >> 7;
        gLineCur.CurTorque  = Filter32(abs(gIMTQ12.T), gLineCur.CurTorque);
	}
    
	//Í¬²½»ú½Ç¶È×ª»»
	tempU = (Uint)((int)gRotorTrans.RTPos + gRotorTrans.PosComp);
    gRotorTrans.RtRealPos = ((Ulong)tempU * 3600L + 10) >> 16;
	if(gMotorInfo.MotorType == MOTOR_TYPE_PM)
    {   
	    gIPMPos.RealPos = ((Ulong)gIPMPos.RotorPos * 3600L + 10) >> 16;
    }
    
    // ai ²ÉÑù´¦Àí
    DINT;
    mTotal1 = gAI.ai1Total;
    mTotal2 = gAI.ai2Total;
    mTotal3 = gAI.ai3Total;
    mAiCounter = gAI.aiCounter;
    
    gAI.ai1Total = 0;
    gAI.ai2Total = 0;
    gAI.ai3Total = 0;
    gAI.aiCounter = 0;
    EINT;
    
    gAI.gAI1 = mTotal1 / mAiCounter;
    gAI.gAI2 = mTotal2 / mAiCounter;
    gAI.gAI3 = mTotal3 / mAiCounter;

    // ¼ÆËãÊµ¼ÊÊä³öµçÑ¹
    mRatio = __IQsat(gRatio, 4096, 0);                              // Ã»ÓÐ¹ýµ÷ÖÆ
    mRatio= (Ulong)mRatio * gUDC.uDC / gInvInfo.BaseUdc;            // ÒÔ±äÆµÆ÷¶î¶¨µçÑ¹Îª»ùÖµ
    gOutVolt.VoltDisplay = Filter4(mRatio, gOutVolt.VoltDisplay);
}

/************************************************************
    Çø·ÖÇý¶¯·½Ê½£¬Ö÷ÒªÍê³ÉÎª05msËÙ¶È»·¿ØÖÆ×¼±¸ºÃ²ÎÊý£»
    
************************************************************/
void RunCaseRun2Ms(void)
{
    //EnableDrive();
    if(gMainCmd.Command.bit.Start == 0)         // ½áÊøÔËÐÐ
    {
        gMainStatus.RunStep = STATUS_STOP;
        RunCaseStop();
        return;
    }

    gMainStatus.StatusWord.bit.StartStop = 1;        
    // Îª×ªËÙ×·×Ù×¼±¸²ÎÊý
    gFeisu.SpeedLast = (gMainCmd.FreqSyn) ? gMainCmd.FreqSyn : gFeisu.SpeedLast; 
    
    switch(gCtrMotorType)
    {
        case ASYNC_SVC:  //Òì²½»úÊ¸Á¿¿ØÖÆ            
        case ASYNC_FVC:
            CalTorqueLimitPar();                // ¼ÆËã×ª¾ØÉÏÏÞºÍ×ª¾Ø¿ØÖÆ
            PrepareAsrPar();
            PrepImCsrPara();
            CalIMSet();							// Àø´ÅµçÁ÷¸ø¶¨      
            CalUdcLimitIT();                   //Ê¸Á¿¿ØÖÆµÄ¹ýÑ¹ÒÖÖÆ¹¦ÄÜ
            break;
            
        case ASYNC_VF:  //Òì²½»úºÍÍ¬²½»úVF¿ØÖÆ,ÔÝ±²»Çø·Ö?
        case SYNC_VF:            
            VFWsTorqueBoostComm();				//×ª²î²¹³¥ºÍ×ª¾ØÌáÉý¹«¹²±äÁ¿¼ÆËã¡£
            VFWSCompControl();					//×ª²î²¹³¥´¦Àí(µ÷ÕûF)
            VFAutoTorqueBoost();        
            #if 1                           // ¼õËÙÏÞÖÆ¹¦ÄÜÆµÂÊÔö¼Ó¸ø¶¨£¬âÑù¼Â´íÎó
            if(speed_DEC &&(abs(gMainCmd.FreqSet) > abs(gVFPar.tpLst)))
            {
              gMainCmd.FreqSet = gVFPar.tpLst;
            }
            gVFPar.tpLst = gMainCmd.FreqSet;
            #endif
    		VfOverCurDeal();				        //¹ýÁ÷ÒÖÖÆ´¦Àí(µ÷ÕûF)
    		VfOverUdcDeal();					    //¹ýÑ¹ÒÖÖÆ´¦Àí(µ÷ÕûF)
    		VfFreqDeal();                           // gVFPar.FreqApply

            gMainCmd.FreqToFunc = gVFPar.FreqApply;
            gMainCmd.FreqSetApply = gVFPar.FreqApply;

            VFSpeedControl();
            CalTorqueUp(); 
            
            HVfOscDampDeal();             // HVf Õñµ´ÒÖÖÆ£¬ ²úÉúµçÑ¹ÏàÎ»
            gOutVolt.VoltPhaseApply = gHVfOscDamp.VoltPhase;            
            gOutVolt.Volt = gHVfOscDamp.VoltAmp;
            
            VFOverMagneticControl();               
            break;
            
        case SYNC_SVC:
            break;
            
        case SYNC_FVC:
            CalTorqueLimitPar();
            CalUdcLimitIT();                   //Ê¸Á¿¿ØÖÆµÄ¹ýÑ¹ÒÖÖÆ¹¦ÄÜ// ¼ÆËã×ª¾ØÉÏÏÞºÍ×ª¾Ø¿ØÖÆ
            PrepareAsrPar();
            //PrepareCsrPara();
            PrepPmsmCsrPrar();
            
            PmFluxWeakDeal();                   // pm Èõ´Å´¦Àí
            break;

        case DC_CONTROL:
            ;
            break;
                        
        default:            
            break;         
    }
}

/************************************************************
    Æô¶¯µç»úÔËÐÐÇ°µÄÊý¾Ý³õÊ¼»¯´¦Àí£¬Îªµç»úÔËÐÐ×¼±¸³õÊ¼²ÎÊý

************************************************************/
void PrepareParForRun(void)
{
// ¹«¹²±äÁ¿³õÊ¼»¯
    gMainStatus.StatusWord.bit.StartStop = 0;
    gMainStatus.StatusWord.bit.SpeedSearchOver = 0;

	gMainStatus.PrgStatus.all = 0;
	gMainStatus.PrgStatus.bit.ACRDisable = 1;    
    gGetParVarable.StatusWord = TUNE_INITIAL;
    gVarAvr.UDCFilter = gInvInfo.BaseUdc;
	gMainCmd.FreqSyn = 0;
	gMainCmd.FreqReal = 0;
	gOutVolt.Volt = 0;
	gOutVolt.VoltApply = 0;
	gRatio = 0;
	gCurSamp.U = 0;
	gCurSamp.V = 0;
	gCurSamp.W = 0;
	gCurSamp.UErr = 600L<<12;
	gCurSamp.VErr = 600L<<12;
    gIUVWQ24.U = 0;
    gIUVWQ24.V = 0;
    gIUVWQ24.W = 0;
	gIUVWQ12.U = 0;
	gIUVWQ12.V = 0;
	gIUVWQ12.W = 0;
	gLineCur.CurPerShow = 0;
    gLineCur.CurTorque  = 0;
	gLineCur.CurBaseInv = 0;
	gLineCur.CurPer = 0;
	gLineCur.CurPerFilter = 0;
	gIMTQ12.M = 0;
	gIMTQ12.T = 0;
    gIMTQ24.M = 0;
    gIMTQ24.T = 0;
	gDCBrake.Time = 0;
    gPWM.gZeroLengthPhase = ZERO_VECTOR_NONE;
    gIAmpTheta.ThetaFilter = gIAmpTheta.Theta;

// Vf Ïà¹Ø¶¼³õÊ¼»¯
    VfVarInitiate();
    
// Ê¸Á¿Ïà¹Ø±äÁ¿³õÊ¼»¯
    gSpeedFilter.Max = 32767;	//ËÙ¶ÈÂË²¨		
    gSpeedFilter.Min = 3277;
    gSpeedFilter.Output = 0;
    //if((IDC_SVC_CTL == gMainCmd.Command.bit.ControlMode) ||
    //    (IDC_FVC_CTL == gMainCmd.Command.bit.ControlMode))
    if(gMainCmd.Command.bit.ControlMode != IDC_VF_CTL)
    {
  	    ResetParForVC();  //VFµ÷ÓÃ¸Ãº¯ÊýÓÐÎÊÌâ£¬ÒòÎªËü¸Ä±äÁËÊä³öµçÑ¹ºÍÆµÂÊ
    }
    
//Í¬²½»ú¿ØÖÆÏà¹Ø±äÁ¿
	gFluxWeak.AdjustId = 0;

//×ªËÙ¸ú×Ù·½Ê½2±äÁ¿³õÊ¼»¯
	gFeisuNew.gWs_out = 0;
	gFeisuNew.t_DetaTime = 0;
	gFeisuNew.stop_time = 0;
	gFeisuNew.inh_mag = 0;
	gFeisuNew.ang_amu =0;
	gFeisuNew.jicicg  =0;
	gFeisuNew.jicics=0;
}

/************************************************************
	ÇÐ»»µ½Í£»ú×´Ì¬(¹«ÓÃ×Óº¯Êý)
************************************************************/
void TurnToStopStatus(void)
{
	DisableDrive();
	gMainStatus.RunStep = STATUS_STOP;
	gMainStatus.SubStep = 1;
}

/*******************************************************************
    Í£»ú×´Ì¬µÄ´¦Àí
********************************************************************/
void RunCaseStop(void)
{
//Í£»ú·âËøÊä³ö
	DisableDrive();	    
	PrepareParForRun();
    gMainCmd.FreqToFunc = 0; 

//µÈ´ýÁãÆ¯¼ì²âÍê³É
	if(gMainStatus.StatusWord.bit.RunEnable != 1)
    {
        return;
    }

//ÅÐ¶ÏÊÇ·ñÐèÒª¶ÔµØ¶ÌÂ·¼ì²â
	if((1 == gExtendCmd.bit.ShortGnd) && (gMainStatus.StatusWord.bit.ShortGndOver == 0))
	{
		gMainStatus.RunStep = STATUS_SHORT_GND;
		gMainStatus.SubStep = 1;        // ÖØÐÂ½øÐÐ¶ÔµØ¶ÌÂ·¼ì²â
		return;
	}
	else
	{
		gMainStatus.StatusWord.bit.ShortGndOver = 1;
	}

// Í¬²½»úÍ£»úÊ±Î»ÖÃÐ£Ñé
    if(gMotorInfo.MotorType == MOTOR_TYPE_PM)
	{
		IPMCheckInitPos();                  
	}

//ÅÐ¶ÏÊÇ·ñÐèÒªÆð¶¯µç»ú
	if(gMainCmd.Command.bit.Start == 1)	
	{
    #ifdef MOTOR_SYSTEM_DEBUG
        if(gTestDataReceive.TestData16)         // Cf-15
        {
            ResetDebugBuffer();
        }
    #endif
        // 
        PmChkInitPosRest();                     // Í¬²½»úÍ£»úÎ»ÖÃÐ£Ñé¸´Î»

	    // ²ÎÊý±æÊ¶
	    if(TUNE_NULL != gGetParVarable.TuneType)
	    {
		    gMainStatus.RunStep = STATUS_GET_PAR;
            PrepareParForTune();
		    return;			
	    }
        //×ªËÙ¸ú×ÙÆð¶¯
		if(0 != gExtendCmd.bit.SpeedSearch)	
		{
			gMainStatus.RunStep = STATUS_SPEED_CHECK;
			gMainStatus.SubStep = 1;
			PrepareForSpeedCheck();
			EnableDrive();
            return;
		}
        //Í¬²½»úÊ¶±ð´Å¼«³õÊ¼Î»ÖÃ½Ç½×¶Î
    	if((gIPMInitPos.Flag == 0) &&
		    (gMotorInfo.MotorType == MOTOR_TYPE_PM) && 
		    (gPGData.PGType == PG_TYPE_ABZ))
		{
			gMainStatus.RunStep = STATUS_IPM_INIT_POS;
			gMainStatus.SubStep = 1;
            gIPMInitPos.Step = 0;

            return;
		}
        
        // else ...STATUS_RUN
		gMainStatus.RunStep = STATUS_RUN;
		gMainStatus.PrgStatus.all = 0;            
		gMainStatus.SubStep = 1;      
        
		EnableDrive();

        RunCaseRun2Ms();        // ÓÅ»¯Æô¶¯Ê±¼ä£¬ÔÚ¸ÃÅÄ¾ÍÄÜ·¢²¨
	}
}

/*************************************************************
	¼ÆËãµ÷ÖÆÏµÊý£º´ÓÊä³öµçÑ¹¼ÆËãµ÷ÖÆÏµÊý
*************************************************************/
void CalRatioFromVot(void)
{
	Uint	m_Ratio;
    
    // ¼ÆËãµ÷ÖÆÏµÊý
    if( 1 == gMainStatus.StatusWord.bit.OutOff )   //Êä³öµôÔØ
    {
	    gOutVolt.VoltApply = 287;                       
    } 
	
	m_Ratio = ((Ulong)gOutVolt.VoltApply * (Ulong)gMotorInfo.Votage)/gInvInfo.InvVolt;	
    //gOutVolt.VoltDisplay = (m_Ratio > 4096) ? 4096 : m_Ratio;
	m_Ratio = ((Ulong)m_Ratio<<2) * gInvInfo.BaseUdc / gUDC.uDC >> 2;  // µçÑ¹µÍÊ±×î¶àÐ¡1
	gRatio = (m_Ratio > 8192) ? 8192 : m_Ratio;
}

/*************************************************************
    ÖÜÆÚÖÐ¶Ï£ºÍê³ÉÄ£ÄâÁ¿²ÉÑù¡¢µçÁ÷¼ÆËã¡¢VCµçÁ÷»·¿ØÖÆµÈ²Ù×÷

×¢Òâ:¸Ãº¯ÊýÔÚ²ÎÊý±æÊ¶ÖÐÒ²ÓÐÊ¹ÓÃ£¬¶ÔËüµÄÐÞ¸Ä£¬ÐèÒª¼ì²éÊÇ·ñÓ°Ïì¿ÕÔØ±æÊ¶
*************************************************************/
void ADCOverInterrupt()
{
//    Uint tempU;
    
    if(gPGData.PGType == PG_TYPE_RESOLVER)          // Ðý±äÐ¾Æ¬¿ªÊ¼²ÉÑù
	{	
		//GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;       //GPIO34ÉÏÉýÑØÆô¶¯Ðý±äÎ»ÖÃ½ÓÊÕ
		RT_SAMPLE_START;
        //gRotorTrans.IntCnt ++;          // ¼ÇÂ¼²ÉÑù´ÎÊý
	}

    if(GetOverUdcFlag())                    //¹ýÑ¹´¦Àí
    {
       HardWareOverUDCDeal();				
    }

// »ñÈ¡Ä£ÄâÁ¿
	//ADCProcess();							//ADC½ÃÕý´¦Àí
	GetUDCInfo();							//»ñÈ¡Ä¸ÏßµçÑ¹²ÉÑùÊý¾Ý    
	GetCurrentInfo();						//»ñÈ¡²ÉÑùµçÁ÷, ÒÔ¼°ÎÂ¶È¡¢Ä¸ÏßµçÑ¹µÄ²ÉÑù
	
	ChangeCurrent();						//¼ÆËã¸÷ÖÖ³¡ºÏÏÂµÄµçÁ÷Á¿
	OutputLoseAdd();						//ÀÛ¼ÓÓÃÓÚÊä³öÈ±ÏàÅÐ¶ÏµÄµçÁ÷

// µç»úÔËÐÐ´¦Àí£¬µçÁ÷»·¿ØÖÆ£¬Íê³ÉÊä³öµçÑ¹£¬¼ÆËãµ÷ÖÆÏµÊý
    switch(gCtrMotorType)
    {
        case ASYNC_SVC:                                     //Òì²½»ú¿ª»·Ê¸Á¿¿ØÖÆ
            CalcABVolt();
            SVCCalFlux_380();        

	    	if(0 == gVCPar.SvcMode)  //Ê¹ÓÃÔ­380Ëã·¨£¬ÒÔ±ãÓÚºÍ320¼æÈÝ
            {
                SvcCalOutVolt();     // ¼ÆËãµçÑ¹Éè¶¨
			}
			else
			{
       	        AlphBetaToDQ((ALPHABETA_STRUCT*)&gIAlphBeta,gFluxR.Theta, &gIMTQ24_obs);
                //gIMTQ24_obs.T = ((llong)gIMTQ24_obs.T>>12) * ((llong)gIMTQ24_obs.M>>12) / (llong)gMotorExtPer.I0;
                //gIMTQ24_obs.T(Q12) = (llong)gIMTQ24_obs.T(Q24) * (llong)gMotorExtPer.I0(Q12) / (llong)gIMTQ24_obs.M(Q24);
                gIMTQ12_obs.M = (gIMTQ24_obs.M>>12);
                gIMTQ12_obs.T = (gIMTQ24_obs.T>>12);

               SvcCalOutVolt_380();     // ¼ÆËãµçÑ¹Éè¶¨
			   //VCCsrControl_380();							        //±Õ»·Ê¸Á¿ITºÍIMµ÷½Ú

			}
            break;
            
        case ASYNC_FVC:                                     //Òì²½»ú±Õ»·Ê¸Á¿¿ØÖÆ
            CalcABVolt();
            SVCCalFlux_380();
		    VCCsrControl();							        //±Õ»·Ê¸Á¿ITºÍIMµ÷½Ú
            break;
            
        case ASYNC_VF:                                      //Òì²½»úºÍÍ¬²½»úVF¿ØÖÆ,ÔÝÊ±²»Çø·Ö?
        case SYNC_VF:

            break;
            
        case SYNC_SVC:
            ;
            break;
            
        case SYNC_FVC:
            PmDecoupleDeal();
            VCCsrControl();
            break;
            
        case RUN_SYNC_TUNE:
            ;                                                   // ½èÓÃÍ¬²½»úµçÁ÷»·
            if(TUNE_STEP_NOW == PM_EST_NO_LOAD ||
                TUNE_STEP_NOW == PM_EST_BEMF ||
                TUNE_STEP_NOW == PM_EST_WITH_LOAD)
            {                
                PmDecoupleDeal();
                VCCsrControl();
            }
            break;
            
        default:            
            break;         
    }  	
   if(DEADBAND_COMP_280 == gExtendCmd.bit.DeadCompMode)  
    {
         CalDeadBandComp();
    }
   else if(DEADBAND_COMP_380== gExtendCmd.bit.DeadCompMode)
    {
         HVfDeadBandComp();
    }
   else  
    {
     gDeadBand.CompU = 0;
     gDeadBand.CompV = 0;
     gDeadBand.CompW = 0;  /*ËÀÇø²¹³¥Ä£Ê½Îª0Ê±¡£²»½øÐÐËÀÇø²¹³¥ 2011.5.7 L1082*/       
    }
     
// Í¬²½»ú²ÎÊý±æÊ¶£¬»òÕßABZ±àÂëÆ÷µÚÒ»´ÎÉÏµç³õÊ¼Î»ÖÃ¼ì²â
    if((STATUS_GET_PAR ==gMainStatus.RunStep || gMainStatus.RunStep ==STATUS_IPM_INIT_POS)
        && (MOTOR_TYPE_PM == gMotorInfo.MotorType) 
        && (gIPMInitPos.Step != 0))
	{
		SynInitPosDetect();
	}
// Í¬²½»ú´øÔØ±æÊ¶
    else if((STATUS_GET_PAR == gMainStatus.RunStep) && 
             (TUNE_STEP_NOW == PM_EST_WITH_LOAD) &&
             (gUVWPG.UvwEstStep == 2))
    {
        GetUvwPhase();
        gUVWPG.TotalErr += (long)(gIPMPos.RotorPos - gUVWPG.UVWAngle);
        gUVWPG.UvwCnt ++;
    }
//Òì²½»ú²ÎÊý±æÊ¶¼ÆËã¿ÕÔØµçÁ÷³ÌÐò
	else if((STATUS_GET_PAR == gMainStatus.RunStep) &&
             (MOTOR_TYPE_PM != gMotorInfo.MotorType))
	{
		LmIoInPeriodInt();					
	}
#ifndef MOTOR_SYSTEM_DEBUG

#endif
    
    if(gPGData.PGMode == 0)
    {
        GetMDetaPos();
       GetMTTimeNum();
    }
    else if(gPGData.PGType == PG_TYPE_RESOLVER)  // Ðý±äÐÅºÅ²ÉÑùÍê³É£¬µÈ´ýÏÂÒæÖÐ¶Ï¶ÁÈ¡
	{
		//GpioDataRegs.GPBSET.bit.GPIO34 = 1;
		RT_SAMPLE_END;
        
        #ifdef TMS320F28035
        ROTOR_TRANS_RD = 1;
        ROTOR_TRANS_SCLK   = 1;
        #endif
    }

#ifdef MOTOR_SYSTEM_DEBUG       //rt debug
    #ifdef TMS320F2808
    DebugSaveDeal(0);
    #endif
#endif 
}

/*************************************************************
ÏÂÒçÖÐ¶Ï£ºÓÃÓÚ·¢ËÍPWM¡£

×¢Òâ:¸Ãº¯ÊýÔÚ²ÎÊý±æÊ¶ÖÐÒ²ÓÐÊ¹ÓÃ£¬¶ÔËüµÄÐÞ¸Ä£¬ÐèÒª¼ì²éÊÇ·ñÓ°Ïì¿ÕÔØ±æÊ¶
*************************************************************/
void PWMZeroInterrupt()
{    
    CalRatioFromVot();// ¼ÆËãµ÷ÖÆÏµÊý   
    SoftPWMProcess();//Ëæ»úPWM´¦Àí£¬»ñÈ¡ÔØ²¨ÖÜÆÚºÍ½Ç¶È²½³¤						
   	CalOutputPhase();//¼ÆËãÊä³öÏàÎ»						

    BrakeResControl();      //ÖÆ¶¯µç×è¿ØÖÆ

// ¸ù¾Ý¿ØÖÆ·½Ê½ PWM ·¢²¨, ²ÎÊý±æÊ¶Ê±¿ÉÄÜ²»ÐèÒªÕâÀï·¢²¨
    if(gMainStatus.PrgStatus.bit.PWMDisable)
    {
        asm(" NOP");
    }
	else
	{	
	    OutPutPWMVF();      // SVPWM
	    //OutPutPWMVC();      // SPWM
	}
    
    if(gPGData.PGMode == 0)
    {
        GetMDetaPos();
        GetMTTimeNum();
    }
    else if(gPGData.PGType == PG_TYPE_RESOLVER)
    {
        GetRotorTransPos();
     //    RotorTransSamplePos();
        gRotorTrans.IntCnt ++;          // ¼ÇÂ¼²ÉÑù´ÎÊý
    }
    
#ifdef MOTOR_SYSTEM_DEBUG       //rt debug
    #ifdef TMS320F2808
    DebugSaveDeal(1);
    #endif
#endif 
}

