/****************************************************************
ÎÄ¼þ¹¦ÄÜ£º´¦ÀíÍ¬²½»ú±Õ»·Ê¸Á¿¿ØÖÆ²¿·Ö£¬Èõ´ÅµÈ
ÎÄ¼þ°æ±¾£ºÍ¬²½»úÔËÐÐÖ÷³ÌÐòÏà¹Ø
×îÐÂ¸üÐÂ£º

****************************************************************/
//ÎÄ¼þÄÚÈÝ: 
//    1. Í¬²½»úÍ£»ú×´Ì¬Î»ÖÃ¼ì²é£»
//    2. Í¬²½»úÔËÐÐÇ°µÄ³õÊ¼Î»ÖÃÈ·¶¨£»
//    3. Í¬²½»úABZ±àÂëÆ÷´Å¼«Î»ÖÃµÄÀÛ¼Ó¼ì²â£
//    4. Í¬²½»úµç¸Ð¼ÆËã£¬µçÁ÷»·²ÎÊýÕû¶¨£»
//    5. Í¬²½»úµÄÈõ´Å´¦Àí£»
//    6. ABZ/UVW±àÂëÆ÷zÖÐ¶ÏµÄ´¦Àí£»

#include "MotorVCInclude.h"
#include "MotorInclude.h"

//************************************************************
IPM_POSITION_STRUCT		gIPMPos;          //ÓÀ´ÅÍ¬²½µç»úºÍ×ª×Ó½Ç¶ÈÏà¹ØµÄ½á¹¹
PM_FLUX_WEAK            gFluxWeak;
PM_INIT_POSITION        gPMInitPos;
IPM_POS_CHECK_STRUCT	gIPMPosCheck;   //ÓÀ´ÅÍ¬²½µç»úÉÏµç¼ì²âµ±Ç°¾ø¶ÔÎ»ÖÃ½ÇµÄÊý¾Ý½á¹¹
PM_DECOUPLE             gPmDecoup;


/*************************************************************
	Í¬²½»úÏÂ±àÂëÆ÷µÄ»ù×¼ÖÐ¶Ïµ½´ïµÄ´¦Àí³ÌÐò(µ÷ÓÃ³ÌÐò)
*************************************************************/
interrupt void PG_Zero_isr(void)
{
    int  errZFlag;
    Uint mPos;
    
	EALLOW;
	(*EQepRegs).QCLR.all = 0x0401;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP5;	            // Acknowledge this interrupt
    EDIS;
    
    if(gIPMZero.zFilterCnt != 0)                        // Z ÐÅºÅµÄÂË²¨´¦Àí£¬ÒªÇóÁ½¸öZ¼ä¸ô³¬¹ý4ms(1e7rpm)
	{
	    //gIPMPos.ZErrCnt ++;
		return;
	}    
 
    
    gIPMZero.zFilterCnt = 8;	                        // 4*0.5ms = 2ms
	gIPMZero.Flag = (gIPMZero.Flag | 0x01);	            // ÖÃZÐÅºÅµ½´ï±êÖ¾
    gIPMPos.Zcounter ++;
    	  
	if(gMainStatus.RunStep != STATUS_GET_PAR)
	{    
	    errZFlag = 0;
		if(gPGData.PGType == PG_TYPE_ABZ)      // ABZ 
		{
			gIPMPos.AbzErrPos = (int)(gIPMPos.RotorZero- gIPMPos.ABZ_RotorPos_Ref);
			if(abs(gIPMPos.AbzErrPos) > (35*65536L/360))                    // abs(30deg)
			{
	            errZFlag ++;
			}                        
		}
        else if(gPGData.PGType == PG_TYPE_UVW)   // UVW, ¼ÈÓÃabzÐÅºÅ¿¨Î»£¬Ò²ÓÃuvwÐÅºÅ¿¨Î»
        {            
            gIPMPos.AbzErrPos = (int)(gIPMPos.RotorPos - gIPMPos.RotorZero);
			if(abs(gIPMPos.AbzErrPos) > (65*65536L/360))                    // abs(60deg)
			{
	            errZFlag ++;
			} 
            
            GetUvwPhase();
            mPos = gUVWPG.UVWAngle + gUVWPG.UvwZeroPos;
            gUVWPG.UvwZIntErr = (int)(gIPMPos.RotorPos  - mPos);
            if(abs(gUVWPG.UvwZIntErr ) > (65L*32767/180))                   // ×î¶àÏàÁÚÇø¼ä
            {
                 errZFlag ++;
            }
            #if 0           // Èç¹ûabz½Ç¶ÈÒÑ¾­Æ«µô£¬»¹Ê¹ÓÃabzÐÅºÅ×öÏÞÎ»£¬½«²»ÄÜÔÚzÐÅºÅ¸´Î»½Ç¶È            
            else    errZFlag = 0;
            #endif            
        }

        if(errZFlag)  
            {
            gIPMPos.ZErrCnt ++;                 // ¸ÉÈÅZÐÅºÅ
            }
        else  
            {

              SetIPMPos(gIPMPos.RotorZero);       // ¸´Î»×ª×Ó½Ç¶È
            }
	}    
    else
	{
		if(TUNE_STEP_NOW == PM_EST_NO_LOAD)              // ¿ÕÔØµ÷Ð³±àÂëÆ÷ÁãµãÎ»ÖÃ½ÇÊ±
		{
			gIPMZero.FeedPos = 0;
			gIPMPos.QepTotal = 0;
			gIPMPos.QepBak   = GetQepCnt();
		}
		else if(TUNE_STEP_NOW == PM_EST_WITH_LOAD)      // ´øÔØ±æÊ¶Ê±
		{
			gIPMPos.PosInZInfo = gIPMPos.RotorPos;

            if(gUVWPG.UvwEstStep == 1)
            {
                gUVWPG.UvwEstStep ++;   // gUVWPG.UvwEstStep==2: uvw est counter start
                gUVWPG.UvwCnt2 = 0;
            }
            else if(gUVWPG.UvwEstStep == 2)
            {
                gUVWPG.UvwCnt2 ++;
                if(gUVWPG.UvwCnt2 >= 3)     // 12¸ö»úÐµÖÜÆÚ
                {
                    gUVWPG.UvwEstStep ++;   // gUVWPG.UvwEstStep==3: int complete
                }
            }
		}

        if(gPGData.PGType == PG_TYPE_UVW)
        {
            GetUvwPhase();
            //gPmParEst.UvwZPos = gUVWPG.UVWAngle;
        }
	}  
}

/***********************************************************************
    Í¬²½»úÍ£»ú×´Ì¬ÏÂ½Ç¶ÈÏÞ¶¨´¦Àí,ÅÐ¶Ïif(gMotorInfo.Type == MOTOR_TYPE_PM)
	²Åµ÷ÓÃ
************************************************************************/
void IPMCheckInitPos(void)
{
	Uint m_Pos;
	int  m_Deta;
    Uint uvwPos;
    int  uvwPowOn;
    
	//½ÓÊÕÉÏ´Îµôµç½Ç¶È£¬²¢ÇÒÊ¶±ð±£´æ½Ç¶ÈÊÇ·ñºÏÀí
	uvwPowOn = 0;
    if(gMainStatus.ParaCalTimes == 0)  //ÉÏµçÖ»¼ÆËãÒ»´ÎµÄ²ÎÊý×ª»»
	{
        gMainStatus.ParaCalTimes = 1;        
        
	    gIPMPos.PowerOffPosDeg = ((Ulong)gIPMPos.PowerOffPos << 16)/3600;	//Ê¶±ð³õÊ¼½Ç¶È,»ñÈ¡ÉÏ´ÎÏÂµç½Ç¶È
	    SetIPMPos((Uint)gIPMPos.PowerOffPosDeg);
        SetIPMPos_ABZRef((Uint)gIPMPos.PowerOffPosDeg);

        PmChkInitPosRest(); 
        uvwPowOn ++;
        gIPMPos.ZeroPosLast = gIPMPos.RotorZero;
    }
    
    // UVW±àÂëÆ÷ÔÚÍ£»úÊ±ÓÃUVWÐÅºÅ×öÐ£Õý   
	if(gPGData.PGType != PG_TYPE_UVW)
	{
        return;
	}
    #if 0

    if(gIPMPosCheck.Cnt == 0)	                                //µÚÒ»¸öÊý¾Ý
	{
		gIPMPosCheck.FirstPos = gIPMPos.RotorPos;
		gIPMPosCheck.Cnt++;
	}
	else if(gIPMPosCheck.Cnt <= 128)					        //ÀÛ¼ÓÎó²î½Ç
	{
		m_Deta = (int)(gIPMPos.RotorPos - gIPMPosCheck.FirstPos);
		gIPMPosCheck.TotalErr += m_Deta;
		gIPMPosCheck.TotalErrAbs += abs(m_Deta);
		gIPMPosCheck.Cnt++;
    }

   // if((gIPMPosCheck.Cnt > 128) ||          // ÅÐ¶Ï256ms
     //   (uvwPowOn == 1))                      // µÚÒ»´ÎÉÏµç    
    if((gIPMPosCheck.Cnt > 128) ||          // ÅÐ¶Ï256ms
    (uvwPowOn == 1))                      // µÚÒ»´ÎÉÏµç 
	{
		if((gIPMPosCheck.TotalErrAbs>>7) < ((5L<<15)/180))//rt             //rt ½Ç¶ÈÏà²îÆ½¾ùÖµÐ¡ÓÚ3¶È
		{
		    GetUvwPhase();
			m_Pos = (Uint)((int)gIPMPosCheck.FirstPos + (gIPMPosCheck.TotalErr>>7));
            uvwPos = gUVWPG.UVWAngle + gUVWPG.UvwZeroPos;

            gIPMPosCheck.UvwStopErr = (int)(m_Pos - uvwPos);
			if(abs(gIPMPosCheck.UvwStopErr) > (36L*32768L/180))   // ÀíÂÛÎó²î×î´ó30deg, µ«ÊÇuvwÐÅºÅ²»Ò»¶¨ÍêÈ«¾ùÔÈ
            {
				SetIPMPos(uvwPos);	        //d
				SetIPMPos_ABZRef(uvwPos);
                gIPMPosCheck.UvwRevCnt ++;
			}            
		}
        //else        // µç»úÃ»ÓÐÍ£ÎÈ           
		PmChkInitPosRest();                      	        //ÖØÐÂ¿ªÊ¼Ê¶±ð
	}
    #endif
    #if 1
    GetUvwPhase();

    if(abs(gFVCSpeed.SpeedTemp)<100)
    {
         if(gIPMPosCheck.Cnt > 128) 
        {
            if((gIPMPosCheck.TotalErrAbs>>7) < 5461)
            {
                m_Pos = (Uint)((int)gIPMPosCheck.FirstPos + (gIPMPosCheck.TotalErr>>7));
                
                uvwPos = m_Pos + gUVWPG.UvwZeroPos;

                gIPMPosCheck.UvwStopErr = (int)(gIPMPos.RotorPos - uvwPos);
    			if(abs(gIPMPosCheck.UvwStopErr) > (36L*32768L/180))   // ÀíÂÛÎó²î×î´ó30deg, µ«ÊÇuvwÐÅºÅ²»Ò»¶¨ÍêÈ«¾ùÔÈ
                {
    				SetIPMPos(uvwPos);	        //d
    				//SetIPMPos_ABZRef(uvwPos);
                    //gIPMPosCheck.UvwRevCnt ++;
    			}            
            }
            PmChkInitPosRest();   
        }
        else if(gIPMPosCheck.Cnt == 0)
        {
            gIPMPosCheck.FirstPos = gUVWPG.UVWAngle;
		    gIPMPosCheck.Cnt++;
        }
        else
        {
            m_Deta = (int)(gUVWPG.UVWAngle - gIPMPosCheck.FirstPos);
    		gIPMPosCheck.TotalErr += m_Deta;
    		gIPMPosCheck.TotalErrAbs += abs(m_Deta);
    		gIPMPosCheck.Cnt++;
        }
    }
    #endif
}

/************************************************************

************************************************************/
void PmChkInitPosRest(void)
{
	gIPMPosCheck.Cnt = 0;               // 
    
	gIPMPosCheck.TotalErr = 0;
	gIPMPosCheck.TotalErrAbs = 0;	
}

/************************************************************
	ÓÀ´ÅÍ¬²½»ú´Å¼«³õÊ¼Î»ÖÃ½Ç¼ì²â½×¶Î
************************************************************/
void RunCaseIpmInitPos(void)
{
	if((gError.ErrorCode.all != 0) || 
	   (gMainCmd.Command.bit.Start == 0))
	{
		DisableDrive();
        //if(gIPMPos.InitPosMethod == INIT_POS_VOLT_PULSE)
        //{
	    SynInitPosDetSetPwm(6);		    //Í¬²½»ú²ÎÊý±æÊ¶»Ö¸´¼Ä´æÆ÷ÉèÖÃ
        //}
		gIPMInitPos.Step = 0;
		TurnToStopStatus();
		return;
	}

    switch(gMainStatus.SubStep)
    {
        case 1:
            if(gIPMInitPos.Step == 0)
            {
                gIPMInitPos.Step = 1;
                //gIPMInitPos.InitPWMTs = (50 * DSP_CLOCK);	  //500us
                gMainStatus.PrgStatus.bit.PWMDisable = 1;

                gMainStatus.SubStep ++;
            }
            else
            {
                gError.ErrorCode.all |= ERROR_PROGRAM_LOGIC;
                gError.ErrorInfo[4].bit.Fault1 = 1;
            }
            break;

        case 2:
            if(gIPMInitPos.Step == 0)           // ÖÐ¶Ï±æÊ¶Íê³É      
        	{ 
    			SetIPMPos((Uint)gIPMPos.InitPos);
                SetIPMPos_ABZRef((Uint)gIPMPos.InitPos);

        		if(abs((int)(gIPMPos.PowerOffPosDeg - gIPMPos.RotorPos)) < 3641)
        		{
        			//SetIPMPos((Uint)gIPMPos.PowerOffPosDeg);	    //rt
        		}

                gIPMInitPos.Flag = 1;
        		DisableDrive();
        		gMainStatus.SubStep ++;             //¿ÉÒÔ¶àµÈÒ»ÅÄ£¬ÈÃPWM»Ö¸´Íê³É
        	}   //else waiting interrupt deal
            break;
            
        case 3:
            	InitSetPWM();
   	            InitSetAdc();
                SetInterruptEnable();	            // Èç¹û±æÊ¶ÏîÄ¿ÖÐÍ¾ÍË³ö£¬ÖÐ¶ÏÓÐ¿ÉÄÜÊÇ¹Ø±ÕµÄ£¬ÐëÔÚ´Ë´ò¿ª
                gMainStatus.SubStep ++; 
                break;
                
        case 4:
            PrepareParForRun();
            gMainStatus.RunStep = STATUS_RUN;
            gMainStatus.SubStep = 0;
            gMainStatus.PrgStatus.all = 0;		//ËùÓÐ¿ØÖÆÓÐÐ§
            EnableDrive();
            break;
            
        default:
            gError.ErrorCode.all |= ERROR_PROGRAM_LOGIC;
            gError.ErrorInfo[4].bit.Fault1 = 2;
            
            break;   
    }
}

/*************************************************************
	Í¬²½»úÏÂ ABZ±àÂëÆ÷Ê±ÓÃQEP¼ÆËãÊµÊ±Î»ÖÃ½Ç£¬
¿¼ÂÇÁËÄÜ¹»Ö§³Ö×î¶à32767Âö³åÊýµÄ±àÂëÆ÷
ouputVar: gIPMZero.FeedPos

?? Èç¹û´ËÊ±±»ZÖÐ¶ÏÖÐ¶ÏÄØ?
*************************************************************/
void SynCalRealPos(void)
{
    long    mQepPos;
    long    mCntMod;       // È¡Ä£Öµ
    long    m_ABZCntMod;
	long    m_4Pluse;
    long    m_DetaCnt;
	int     m_Pos;
    int     m_ABZPos;
    int     mSign;

    long  m_ABZQepPos;
    long  m_ABZDetaCnt;
    
    DINT;
    mQepPos = GetQepCnt();		
    m_DetaCnt = mQepPos - gIPMPos.QepBak;
    gIPMPos.QepBak = mQepPos;
    gIPMPos.QepTotal += m_DetaCnt;
    
	m_4Pluse = ((long)gPGData.PulseNum)<<2;
	mCntMod = gIPMPos.QepTotal % m_4Pluse;
	m_Pos = (mCntMod<<14)/(int)gPGData.PulseNum;			
	gIPMZero.FeedPos = m_Pos * gMotorExtInfo.Poles;						//µç½Ç¶È
    

    if(gPGData.PGType == PG_TYPE_ABZ)
    {
        m_ABZQepPos = GetQepCnt();
        m_ABZDetaCnt = m_ABZQepPos - gIPMPos.ABZ_QepBak;
        gIPMPos.ABZ_QepBak = m_ABZQepPos;
        gIPMPos.ABZ_QepTotal += m_ABZDetaCnt;

        m_4Pluse = ((long)gPGData.PulseNum)<<2;
	    m_ABZCntMod = gIPMPos.ABZ_QepTotal % m_4Pluse;
        m_ABZPos = (m_ABZCntMod<<14)/(int)gPGData.PulseNum;
        gIPMZero.FeedABZPos = m_ABZPos* gMotorExtInfo.Poles;
        
    }
    EINT;
    
    if(labs(gIPMPos.QepTotal) > (50L*4L* gPGData.PulseNum))       // ´óÓÚ50È¦£¬¼õÈ¥30È¦£¬·Àtota±¥ºÍ
    {
        mSign = (gIPMPos.QepTotal > 0) ? 1 : -1;
        gIPMPos.QepTotal -= (30L*4L* gPGData.PulseNum * mSign);
    }
    if(labs(gIPMPos.ABZ_QepTotal)>(50L*4L* gPGData.PulseNum))
    {
        mSign = (gIPMPos.ABZ_QepTotal > 0) ? 1 : -1;
        gIPMPos.ABZ_QepTotal -= (30L*4L* gPGData.PulseNum * mSign);
    }

    
}

/************************************************************
	LD¡¢LQÖáµç¸Ð¼ÆËãº¯Êý
	LAB = A-B*Cos(2*Theta-4*pi/3)	:= gIPMInitPos.LPhase[0]
	LBC = A-B*Cos(2*Theta)		     := gIPMInitPos.LPhase[1]
	LCA = A-B*Cos(2*Theta+4*pi/3)	:= gIPMInitPos.LPhase[2]
************************************************************/
void SynCalLdAndLq(Uint m_Pos)
{
	int m_Cos1,m_Cos2;
	int m_CoffA,m_CoffB;
	Uint m_Angle;

	m_Angle = (m_Pos<<1);
	m_Cos1 = qsin(16384 - (int)m_Angle);        // cos(2*theta)
	m_Angle += 43691;
	m_Cos2 = qsin(16384 - (int)m_Angle);        // cos(2*theta + 4*pi/3)

	m_CoffB = gIPMInitPos.LPhase[2] - gIPMInitPos.LPhase[1];
	m_CoffB = ((long)m_CoffB<<15)/(m_Cos1 - m_Cos2);
	m_CoffA = gIPMInitPos.LPhase[1] + (((long)m_CoffB * (long)m_Cos1)>>15);

	m_CoffA = m_CoffA>>1;
	m_CoffB = m_CoffB>>1;
	gIPMInitPos.Ld = m_CoffA - m_CoffB;
	gIPMInitPos.Lq = m_CoffA + m_CoffB;

	//gMotorExtInfo.LD = gIPMInitPos.Ld;
	//gMotorExtInfo.LQ = gIPMInitPos.Lq;
    gMotorExtReg.LD = gIPMInitPos.Ld;
    gMotorExtReg.LQ = gIPMInitPos.Lq;
}

/************************************************************
	×ÔÊÊÓ¦¼ÆËãµçÁ÷»·µÄµ÷½ÚÆ÷ÔöÒæ(°´ÕÕ8KHzÔØ²¨ÆµÂÊ¼ÆËã)
	m_kp_m = m_ld*m_Fc*m_In*4096.0/(2.6*64.0*m_Un);
	m_ki_m = m_res*m_In*65536.0/(1.3*64*m_Un);
	m_kp_t = m_lq*m_Fc*m_In*4096.0/(2.6*64.0*m_Un);
	m_ki_t = m_res*m_In*65536.0/(1.3*64*m_Un);
	
	Á¿¸Õ£ºµç¸Ð-mH¡¢µçÁ÷-A¡¢µçÑ¹-V¡¢µç×è-Å·Ä·¡¢ÔØ²¨ÆµÂÊ8KHz
	m_kp_m = m_ld*m_In*197/m_Un;
	m_ki_m = m_res*m_In*788/m_Un;
	m_kp_t = m_lq*m_In*197/m_Un;
	m_ki_t = m_ki_m
	
	Á¿¸Õ£ºµç¸Ð-0.01mH¡¢µçÁ÷-0.01A¡¢µçÑ¹-V¡¢µç×è-0.001Å·Ä·¡¢ÔØ²¨ÆµÂÊ8KHz
	m_kp_m = m_ld*m_In/(51*m_Un);
	m_ki_m = m_res*m_In/(128*m_Un) = (m_res*m_In/m_Un)>>7;
	m_kp_t = m_lq*m_In/(51*m_Un);
	m_ki_t = m_ki_m
************************************************************/
void IPMCalAcrPIDCoff(void)
{
    Uint temp;
    
#if 1
    // gain of axis-d
	temp = ((Ulong)gMotorExtReg.LD * (Ulong)gMotorInfo.CurrentGet) / gMotorInfo.Votage;
    gPmParEst.IdKp = (Ulong)temp * 1290L >> 10; // * 1.26
    // gain of axis-q
	temp = ((Ulong)gMotorExtReg.LQ * (Ulong)gMotorInfo.CurrentGet) / gMotorInfo.Votage;
    gPmParEst.IqKp = (Ulong)temp * 1290L >> 10; // * 1.26
    // »ý·ÖÔöÒæ
	gPmParEst.IdKi = (((Ulong)gMotorExtReg.RsPm * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Votage)>>(1);    // * 0.504
	gPmParEst.IqKi = gPmParEst.IdKi;    //
#else       //d
;   gPmParEst.IdKp = 1000;
    gPmParEst.IdKi = 1000;
    gPmParEst.IqKp = 1000;
    gPmParEst.IqKi = 1000;
#endif
   
}

void PrepPmsmCsrPrar()
{     
    long    ImKp, ImKi, ItKp, ItKi, temp;
    int     Nf;
    int     sGain;  // »ý·Öµ÷Õû±¶Êý
    int     tempFreq;

    tempFreq = (gBasePar.FcSetApply > 40) ? gBasePar.FcSetApply : 40;   // ÊÔÑé·¢´óµçÁ÷»·ÔöÒæ
    
// Í¬²½»ú¸ù¾ÝÔØ²¨µ÷ÕûµçÁ÷»·²ÎÊý
	ImKp = (long)gVCPar.AcrImKp * tempFreq  / 60;
    ImKi = gVCPar.AcrImKi;
    
    ItKp = (long)gVCPar.AcrItKp * tempFreq  / 60;
    ItKi = gVCPar.AcrItKi;

    //gFluxWeak.CsrGainMode = 1;      //d 
    //if(gFluxWeak.CsrGainMode == 0)    // Í¬²½»úµçÁ÷»·piÔöÒæ²»ÐÞÕý         //rt
    gImAcrQ24.KP = ImKp;
    gItAcrQ24.KP = ItKp;

   /*		
// Í¬²½»úÈõ´ÅÊ±£¬pi²ÎÊýµ÷Õû
    // ¸ù¾Ý ÔØ²¨±Èµ÷½Ú±ÈÀýÔöÒæ
    if(gFluxWeak.AbsFrqLpf < gMotorInfo.FreqPer)
    {
        Nf = 40;
    }
    else
    {
        temp = (gBasePar.FullFreq01/100) * gFluxWeak.AbsFrqLpf >>15;        // si-1Hz
        Nf = ((long)gBasePar.FcSetApply * 100) / temp;                      // ÔØ²¨±È¼ÆËã
    }
    if(Nf >= 40)
    {
        gImAcrQ24.KP = ImKp;
        gItAcrQ24.KP = ItKp;
    }
    else if(Nf >= 20)
    {
        gImAcrQ24.KP = ImKp * 40L / Nf;
        gItAcrQ24.KP = ItKp * 40L / Nf;
    }
    else //(Nf <20)
    {
        gImAcrQ24.KP = ImKp * 2;
        gItAcrQ24.KP = ItKp * 2;
    }    */
    // ¸ù¾ÝÔËÐÐÆµÂÊµ÷½Ú»ý·ÖÔöÒæ
    if(gFluxWeak.AbsFrqLpf <= gMotorInfo.FreqPer)
    {
        gImAcrQ24.KI = ImKi;
        gItAcrQ24.KI = ItKi;
    }
    else    // gFluxWeak.FreqLpf < 32767
    {
        sGain = gFluxWeak.CoefKI;
        temp = 32767L - gMotorInfo.FreqPer;
        temp = (((long)gFluxWeak.AbsFrqLpf -gMotorInfo.FreqPer)<<5) / temp;       // Q5
        gImAcrQ24.KI = ((1L<<5) + temp * sGain) * ImKi >>5;            // d Öá×î´óÐÞÕýÔö¼Ó1±¶
        gItAcrQ24.KI = ((1L<<5) + (temp * sGain<<2)) * ItKi >>5;       // q Öá×î´óÐÞÕýÔö¼Ó4±¶
    }
}

/**************************************************************************************
   pm Èõ´Å¼ÆËã, ¸ø³öM ÖáµçÁ÷Éè¶¨£»
   
**************************************************************************************/
void PmFluxWeakDeal()
{
    int     frq;
    int     minFrq;
    long    mVd;
	//long	mVq;
    long    temp1, temp2;
    int     maxCur;
    
    gFluxWeak.IqLpf   = Filter4(gIMTSet.T>>12, gFluxWeak.IqLpf);
    gFluxWeak.VoltLpf = Filter4(gOutVolt.VoltApply, gFluxWeak.VoltLpf);

    //minFrq = (long)gMotorInfo.FreqPer * 3 >> 2;               // 75% ¶î¶¨µç»úÆµÂÊÒÔÏÂ²»Èõ´Å
    minFrq = 5;                                          // ±£Ö¤Êä³öÆµÂÊ·Ç0
    frq = (abs(gMainCmd.FreqSyn) > minFrq) ? abs(gMainCmd.FreqSyn) : minFrq;
    gFluxWeak.AbsFrqLpf = Filter4(frq, gFluxWeak.AbsFrqLpf);

//...
    // ÐèÒªÈõ´Å
    temp1 = (long)gFluxWeak.AbsFrqLpf * gMotorExtPer.LQ >> 9;                   // Q15
    mVd   = (long)gFluxWeak.IqLpf * temp1 >> 15;                                // Q12
    gFluxWeak.Vd = __IQsat(mVd, 32767, -32767);

    temp1 = (long)gOutVolt.MaxOutVolt - 400L; 
    temp2 = temp1 * temp1 - (long)gFluxWeak.Vd * gFluxWeak.Vd;                  // Q24
    temp2 = __IQsat(temp2, 0x7FFFFFFF, 0);
    gFluxWeak.Vq   = qsqrt(temp2);                                                       // Q12
    
    temp2 = ((long)gFluxWeak.Vq<<15)/gFluxWeak.AbsFrqLpf - gMotorExtPer.FluxRotor;   // Q12
    gFluxWeak.FluxSd = __IQsat(temp2, 0, -32767);
    temp1 = (((long)gFluxWeak.FluxSd <<15) /gMotorExtPer.LD) >>6;                 // Q12                         // Q12
    gFluxWeak.IdSet = temp1 * gFluxWeak.CoefFlux / 100L;                        // Q12

    maxCur = 4096L * gFluxWeak.IdMax /100;              // Q12
    gFluxWeak.IdSet = __IQsat(gFluxWeak.IdSet, 0, -maxCur);
   

    if(gFluxWeak.Mode == 2)     // Èõ´ÅÄ£Ê½2£¬ ÔÙ½øÐÐÐÞÕý
    {                
        gFluxWeak.CoefAdj = __IQsat(gFluxWeak.CoefAdj, 1000, 10);    // 10% - 1000%
        gFluxWeak.AdjustLimit = (long)maxCur * 100L / gFluxWeak.CoefAdj;
        
        gFluxWeak.AdjustId += (gOutVolt.MaxOutVolt - gFluxWeak.VoltLpf) / 4;
        gFluxWeak.AdjustId = __IQsat(gFluxWeak.AdjustId, gFluxWeak.AdjustLimit, -gFluxWeak.AdjustLimit);

        temp1 = (long)gFluxWeak.AdjustId * gFluxWeak.CoefAdj / (100L);    // ´Ó2msµ½05ms£¬»ý·Ö¼õÈõ4±¶
        gFluxWeak.IdSet += temp1;
        gFluxWeak.IdSet = __IQsat(gFluxWeak.IdSet, 0, -maxCur);
    }

    // Ñ¡ÔñÈõ´ÅµçÁ÷
    gIMTSet.M = (gFluxWeak.Mode) ? ((long)gFluxWeak.IdSet << 12) : 0;
    return;
}

// Í¬²½»ú½âñî¼ÆËã£¬¼ÆËãÁ½ÏàÐý×ªµç¶¯ÊÆ
void PmDecoupleDeal()
{
    long temp;
    
    gPmDecoup.Omeg = Filter2(gMainCmd.FreqSyn, gPmDecoup.Omeg);
    gPmDecoup.Isd  = Filter2((gIMTQ24.M>>12), gPmDecoup.Isd);
    gPmDecoup.Isq  = Filter2((gIMTQ24.T>>12), gPmDecoup.Isq);

    temp = (long)gPmDecoup.Isd * gMotorExtPer.LD >> 9;                        // Q12
    gPmDecoup.PhiSd = temp + gMotorExtPer.FluxRotor;                          // Q12
    gPmDecoup.RotVq = (long)gPmDecoup.Omeg * gPmDecoup.PhiSd >> 15;        // Q12

    gPmDecoup.PhiSq = (long)gPmDecoup.Isq * gMotorExtPer.LQ >> 9;            // Q12
    gPmDecoup.RotVd = - (long)gPmDecoup.Omeg * gPmDecoup.PhiSq >> 15;      // Q12
}

