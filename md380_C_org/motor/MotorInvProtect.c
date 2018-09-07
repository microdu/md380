/************************************************************
ÎÄ¼ş¹¦ÄÜ:µç»úºÍ±äÆµÆ÷±£»¤³ÌĞò
ÎÄ¼ş°æ±¾£º 
×îĞÂ¸üĞÂ£º 

*************************************************************/
#include "DSP2803x_EPwm_defines.h"
#include "MotorInvProtectInclude.h"
// // È«¾Ö±äÁ¿¶¨Òå
OVER_LOAD_PROTECT		gOverLoad;
PHASE_LOSE_STRUCT		gPhaseLose;
INPUT_LOSE_STRUCT		gInLose;
LOAD_LOSE_STRUCT		gLoadLose;
FAN_CTRL_STRUCT			gFanCtrl;
Ulong					gBuffResCnt;	//»º³åµç×è±£»¤±äÁ¿
CBC_PROTECT_STRUCT		gCBCProtect;
struct FAULT_CODE_INFOR_STRUCT_DEF  gError;

//±äÆµÆ÷ºÍµç»úµÄ¹ıÔØ±í
Uint const gInvOverLoadTable[9] =      /*±í³¤¶È9£»±í²½³¤9%£»±í×îĞ¡Öµ:115%£»±í×î´óÖµ:187%*/
{
		36000,				//115%±äÆµÆ÷µçÁ÷   		1Ğ¡Ê±¹ıÔØ  
		18000,				//124%±äÆµÆ÷µçÁ÷	  	30·ÖÖÓ¹ıÔØ
        6000,				//133%±äÆµÆ÷µçÁ÷	  	10·ÖÖÓ¹ıÔØ
        1800,				//142%±äÆµÆ÷µçÁ÷	  	3·ÖÖÓ¹ıÔØ 
        600,				//151%±äÆµÆ÷µçÁ÷   		1·ÖÖÓ¹ıÔØ  
        200,				//160%±äÆµÆ÷µçÁ÷   		20Ãë¹ıÔØ   
        120,				//169%±äÆµÆ÷µçÁ÷   		12Ãë¹ıÔØ    
        20,					//178%±äÆµÆ÷µçÁ÷   		6Ãë¹ıÔØ    ¸ÄÎª178% 2S¹ıÔØ
        20,					//187%±äÆµÆ÷µçÁ÷   		2Ãë¹ıÔØ    
};
Uint const gInvOverLoadTableForP[9] =       /*±í³¤¶È9£»±í²½³¤4%£»±í×îĞ¡Öµ:105%£»±í×î´óÖµ:137%*/
{
		36000,				//105%±äÆµÆ÷µçÁ÷   		1Ğ¡Ê±¹ıÔØ  
		15000,				//109%±äÆµÆ÷µçÁ÷	  	25·ÖÖÓ¹ıÔØ
        6000,				//113%±äÆµÆ÷µçÁ÷	  	10·ÖÖÓ¹ıÔØ
        1800,				//117%±äÆµÆ÷µçÁ÷	  	3·ÖÖÓ¹ıÔØ 
        600,				//121%±äÆµÆ÷µçÁ÷   		1·ÖÖÓ¹ıÔØ  
        300,				//125%±äÆµÆ÷µçÁ÷   		30Ãë¹ıÔØ   
        100,				//129%±äÆµÆ÷µçÁ÷   		10Ãë¹ıÔØ    
        30,					//133%±äÆµÆ÷µçÁ÷   		3Ãë¹ıÔØ    
        10,					//137%±äÆµÆ÷µçÁ÷   		1Ãë¹ıÔØ    
};

//±äÆµÆ÷¹ıÔØÀÛ¼ÆÁ¿µÄÏû³ıÏµÊı
Ulong const gInvOverLoadDecTable[12] =
{
        (65536L*60/7),      //0%±äÆµÆ÷µçÁ÷    0.7·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/8),		//10%±äÆµÆ÷µçÁ÷   0.8·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/9),		//20%±äÆµÆ÷µçÁ÷   0.9·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/10),		//30%±äÆµÆ÷µçÁ÷   1.0·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/11),		//40%±äÆµÆ÷µçÁ÷   1.1·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/13),		//50%±äÆµÆ÷µçÁ÷   1.3·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/16),		//60%±äÆµÆ÷µçÁ÷   1.6·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/19),		//70%±äÆµÆ÷µçÁ÷   1.9·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/24),		//80%±äÆµÆ÷µçÁ÷   2.4·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/34),		//90%±äÆµÆ÷µçÁ÷   3.4·ÖÖÓÏû³ı¹ıÔØ
		(65536L*60/56),		//100%±äÆµÆ÷µçÁ÷  5.6·ÖÖÓÏû³ı¹ıÔØ
};

#define C_MOTOR_OV_TAB_NUM      7
Uint const gMotorOverLoadTable[C_MOTOR_OV_TAB_NUM] =
{
		48000,				//115%µç»úµçÁ÷  1Ğ¡Ê±20·ÖÖÓ¹ıÔØ
		24000,				//125%µç»úµçÁ÷  40·ÖÖÓ¹ıÔØ
		9000,				//135%µç»úµçÁ÷  15·ÖÖÓ¹ıÔØ 
		3000,				//145%µç»úµçÁ÷  5·ÖÖÓ¹ıÔØ
		1200,				//155%µç»úµçÁ÷  2·ÖÖÓ¹ıÔØ
		1200,				//165%µç»úµçÁ÷  2·ÖÖÓ¹ıÔØ
		1200				//175%µç»úµçÁ÷  2·ÖÖÓ¹ıÔØ
};
#define C_MOTOR_OV_MAX_CUR      1750
#define C_MOTOR_OV_MIN_CUR      1150
#define C_MOTOR_OV_STEP_CUR     100

//µç»ú¹ıÔØÀÛ¼ÆÁ¿µÄÏû³ıÏµÊı
Ulong const gMotorOverLoadDecTable[12] =
{
        (65536L*60/30),     //0%µç»úµçÁ÷    3.0·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/40),		//10%µç»úµçÁ÷   4.0·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/50),		//20%µç»úµçÁ÷   5.0·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/60),		//30%µç»úµçÁ÷   6.0·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/70),		//40%µç»úµçÁ÷   7.0·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/80),		//50%µç»úµçÁ÷   8.0·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/90),		//60%µç»úµçÁ÷   9.0·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/100),	//70%µç»úµçÁ÷   10.0·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/110),	//80%µç»úµçÁ÷   11.0·ÖÖÓÏû³ı¹ıÔØ
        (65536L*60/120),	//90%µç»úµçÁ÷   12.0·ÖÖÓÏû³ı¹ıÔØ
		(65536L*60/130),	//100%µç»úµçÁ÷  13.0·ÖÖÓÏû³ı¹ıÔØ
};

// //ÎÂ¶ÈÇúÏß±í
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

// //ÎÄ¼şÄÚ²¿º¯ÊıÉùÃ÷
void    SoftWareErrorDeal(void);	
void	TemperatureCheck(void);					//ÎÂ¶È¼ì²é
void	OutputPhaseLoseDetect(void);			//Êä³öÈ±Ïà¼ì²â
void 	OutputLoseReset(void);		
void	InputPhaseLoseDetect(void);				//ÊäÈëÈ±Ïà¼ì²â
void	ControlFan(void);						//·çÉÈ¿ØÖÆ
void	OverLoadProtect(void);					//¹ıÔØ±£»¤
void	CBCLimitCurProtect(void);
void 	SetCBCEnable(void);
void 	SetCBCDisable(void);
void	LoadLoseDetect(void);
void 	BufferResProtect(void);

/************************************************************
	±äÆµÆ÷±£»¤´¦Àí
************************************************************/
void InvDeviceControl(void)			
{
	//if(gADC.ResetTime < 500)
    {
    //    return;                         //AD²ÉÑùÎÈ¶¨ºó¿ªÊ¼
    }
	SoftWareErrorDeal();				//³ö´í´¦Àí
	TemperatureCheck();					//ÎÂ¶È¼ì²é
	OutputPhaseLoseDetect();			//Êä³öÈ±Ïà¼ì²â
	InputPhaseLoseDetect();				//ÊäÈëÈ±Ïà¼ì²â
	ControlFan();						//·çÉÈ¿ØÖÆ
	OverLoadProtect();					//¹ıÔØ±£»¤

    CBCLimitCurPrepare();					//Öğ²¨ÏŞÁ÷±£»¤³ÌĞò
	CBCLimitCurProtect();				//Öğ²¨ÏŞÁ÷Çé¿öÏÂµÄ¹ıÔØÅĞ¶Ï
	
	LoadLoseDetect();					//µôÔØ´¦Àí
	BufferResProtect();
}

/*************************************************************
	Èí¼ş¹ÊÕÏ´¦Àí
Í£»ú×´Ì¬Ò²¿ÉÄÜ±¨¹ıÁ÷¹ÊÕÏ¡¢¹ıÑ¹¹ÊÕÏ
*************************************************************/
void SoftWareErrorDeal(void)					
{
	if((gMainStatus.RunStep == STATUS_LOW_POWER) ||                         // Ç·Ñ¹
	   ((gError.ErrorCode.all & ERROR_SHORT_EARTH) == ERROR_SHORT_EARTH))
	{
		gUDC.uDCBigFilter = gUDC.uDCFilter;
		return;										//Ç·Ñ¹×´Ì¬ÏÂ²»ÅĞ¶ÏÈí¼ş¹ÊÕÏ
	}
	if((STATUS_STOP == gMainStatus.RunStep) &&
        (gError.LastErrorCode != gError.ErrorCode.all) && 
        (gError.ErrorCode.all != 0))
	{
	    gError.LastErrorCode = gError.ErrorCode.all;
        if(0 != gError.ErrorCode.all)
        {
			gPhase.IMPhase += 0x40000000L;				//¹ÊÕÏ¸´Î»ºóÔËĞĞ½Ç¶È·¢Éú±ä»¯
        }
    }
    
// ¿ªÊ¼¹ÊÕÏ¸´Î»
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

// ¿ªÊ¼ÅĞ¶ÏÈí¼ş¹ÊÕÏ
	if(gLineCur.CurBaseInv > 10240)			// 2.5±¶¹ıÁ÷µãÅĞ¶Ï
	// (²ÉÑùµçÁ÷Îª2±¶¶î¶¨µçÁ÷£¬ËùÒÔÊµ¼ÊµÄ¹ıÁ÷±£»¤Ã»ÓĞ)
	{
	    DINT;
		DisableDrive();
		gError.ErrorCode.all |= ERROR_OVER_CURRENT;
        gError.ErrorInfo[0].bit.Fault1 = 2;
		gLineCur.ErrorShow = gLineCur.CurPer;
        EINT;
	}
    
	if((gUDC.uDCBigFilter > gInvInfo.InvUpUDC) || //¹ıÑ¹ÅĞ¶Ï,Ê¹ÓÃ´óÂË²¨µçÑ¹
	    GetOverUdcFlag())
	{
	    DisableDrive(); //Í£»úÒ²ÔÊĞí±¨¹ıÑ¹£¬ÌáĞÑÓÃ»§ÊäÈëµçÑ¹¹ı¸ß
	    gError.ErrorCode.all |= ERROR_OVER_UDC;
        gError.ErrorInfo[0].bit.Fault2 = 2;
	}
	else if(gUDC.uDCBigFilter < gInvInfo.InvLowUDC) //Ç·Ñ¹ÅĞ¶Ï,Ê¹ÓÃ´óÂË²¨µçÑ¹
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
ÎÂ¶È¼ì²é,ËµÃ÷ÈçÏÂ:

1¡¢»úĞÍĞ¡ÓÚµÈÓÚ10       (2.2Kw ÒÔÏÂ)
	ÎÂ¶ÈÇúÏß:	     1		TABLE_P44X
				2		TABLE_P8XX
				3		TABLE_SEMIKON
				4		TABLE_BSMXX

2¡¢»úĞÍÔÚ£¨11¡«18Ö®¼ä£© (11Kw µ½ 30Kw)
	ÎÂ¶ÈÇúÏß:	     1		TABLE_BSMXX
				2		TABLE_P44X
				3~4		TABLE_SEMIKON

3¡¢»úĞÍÔÚ£¨19¡«26Ö®¼ä£© (37Kw µ½ 200Kw,²¢ÇÒ°üº¬37, ²»°üº¬200)
	ÎÂ¶È²ÉÑùÍâÖÃ£º		TABLE_WAIZHI

4¡¢»úĞÍ´óÓÚµÈÓÚ27		TABLE_BSMXX

5¡¢»úĞÍ´óÓÚµÈÓÚ27Ê±£¬ÎÂ¶È²ÉÑùµçÂ·²»Í¬£¬ĞèÒª½øĞĞ3.3VºÍ3VµÄ×ª»»£»

6¡¢»úĞÍ´óÓÚµÈÓÚ19µÄ85¶È±£»¤£»ÆäËû»úĞÍ95¶È±£»¤£»

±í¸ñÅÅÁĞ·½Ê½£ºÃ¿4¶È×÷Ò»¸öÊı¾İ£¬ÆğÊ¼µØÖ·Êı¾İÎª12¶È£¬Êı¾İÖµÎªAD²ÉÑùÖµ
AD²ÉÑùÖµ£ºAD_RESULT>>6
************************************************************/
void TemperatureCheck(void)
{
	Uint    * m_pTable;
	Uint    m_TempAD,m_IndexLow,m_IndexHigh,m_Index,m_ErrTemp;
	Uint    m_LimtCnt;
    Uint    mType;

	//...×¼±¸ÎÂ¶È±í,690VµÄÎÂ¶È²ÉÑùºÍ±£»¤Óë380V»úĞÍ´óÓÚ27µÄÒ»ÖÂ
	if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
    {
        mType = gInvInfo.InvTypeApply + 27;
    }
    else
    {
        mType = gInvInfo.InvTypeApply;
    }

// ±£»¤ÎÂ¶ÈµãµÄÈ·¶¨
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
       gInvInfo.TempType = 1;  /*1140V ¹ıÎÂµãºÍÎÂ¶ÈÇúÏßÉè¶¨ 2011.5.7 L1082*/
    }
// ÎÂ¶ÈÇúÏßµÄÑ¡¶¨
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

// Ó²¼ş²ÉÑù£¬ÒÔ¼°²ÉÑùµçÂ·µÄĞŞÕı
    m_TempAD = (gTemperature.TempAD>>6);

#ifdef  TMS320F2808
    if(mType >= 27) // 3VºÍ3.3v ²ÉÑùµçÂ·µÄ×ª»»
    {
        m_TempAD = ((long)gTemperature.TempAD * 465)>>15; 
    }
#endif

// ¿ªÊ¼²éÑ¯ÎÂ¶È±í
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
			m_LimtCnt++;					//±ÜÃâËÀÑ­»·
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
	if(mType - gTemperature.TempBak >= 8)			//ÎÂ¶È±ä»¯³¬¹ı0.5¶È²Å¸³Öµ
	{
		gTemperature.TempBak = mType;
		gTemperature.Temp = mType>>4;
	}

// ¿ªÊ¼×÷ÎÂ¶ÈÅĞ¶ÏºÍ±¨¾¯´¦Àí
	gTemperature.ErrCnt++;
	if(gTemperature.Temp < m_ErrTemp)
	{	
		gTemperature.ErrCnt = 0;
	}

	if(gTemperature.ErrCnt >= 5)
	{
		gTemperature.ErrCnt = 0;
		gError.ErrorCode.all |= ERROR_INV_TEMPERTURE;		//¹ıÈÈ±¨¾¯
	}

/*
    	//...×¼±¸ÎÂ¶È±í,690VµÄÎÂ¶È²ÉÑùºÍ±£»¤Óë380V»úĞÍ´óÓÚ27µÄÒ»ÖÂ
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
        m_TempAD = ((long)gTemperature.TempAD * 465)>>15;   //»úĞÍ´óÓÚ26Ê±£¬ÎÂ¶È²ÉÑùµçÂ·²»Í¬£¬ĞèÒª½øĞĞ
                                                            //3VºÍ3.3v ²ÉÑùµçÂ·µÄ×ª»»
	}
	else
	{
		m_ErrTemp = 85;				//»úĞÍ´óÓÚµÈÓÚ19µÄ85¶È±£»¤
		m_pTable = (Uint *)gTempTableWAIZHI;
	}
*/
}

/************************************************************
	Êä³öÈ±Ïà¼ì²â
************************************************************/
void OutputPhaseLoseDetect(void)			
{
	Uint m_U,m_V,m_W;
	Uint m_Max,m_Min;
    
	if((gSubCommand.bit.OutputLost == 0) ||	
	   (gMainCmd.FreqReal < 80) ||					//0.8HzÒÔÏÂ²»¼ì²â
	   (gMainCmd.Command.bit.StartDC == 1) ||		//Ö±Á÷ÖÆ¶¯×´Ì¬ÏÂ²»¼ì²â
	   (gMainCmd.Command.bit.StopDC  == 1) ||
	   ((gMainStatus.RunStep != STATUS_RUN) && 		//²»ÊÇÔËĞĞ»òÕßËÙ¶ÈËÑË÷½×¶Î
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

void OutputLoseAdd(void)		//Êä³öÈ±Ïà¼ì²âÀÛ¼ÓµçÁ÷´¦Àí
{
	gPhaseLose.TotalU += abs(gIUVWQ24.U >> 12);
	gPhaseLose.TotalV += abs(gIUVWQ24.V >> 12);
	gPhaseLose.TotalW += abs(gIUVWQ24.W >> 12);

    gPhaseLose.Time   += gPWM.gPWMPrdApply;
    //gPhaseLose.Time += 2000L * DSP_CLOCK;
	gPhaseLose.Cnt++;
}
void OutputLoseReset(void)		//Êä³öÈ±Ïà¼ì²â¸´Î»¼Ä´æÆ÷´¦Àí
{
	gPhaseLose.Cnt = 0;
	gPhaseLose.TotalU = 0;
	gPhaseLose.TotalV = 0;
	gPhaseLose.TotalW = 0;
	gPhaseLose.Time   = 0;
}

/************************************************************
	ÊäÈëÈ±Ïà¼ì²â
È±Ïà¼ì²âÔ­Àí: 
    ¼ÌµçÆ÷¹ÊÕÏĞÅºÅÓëÈ±ÏàĞÅºÅ¸´ºÏ£¬ Õı³£µÄÊ±ºòÒ»Ö±Îª¸ß£»
    ÈôPLÒ»Ö±ÎªµÍ£¬Ôò¼ÌµçÆ÷Ã»ÓĞÎüºÏ£»
    ÈôPLÎª·½²¨£¬ÔòÈ±Ïà£»    
2808Dsp PLĞÅºÅÓëVOE(Ó²¼ş¹ıÑ¹)ĞÅºÅÔÚÓ²¼şÉÏ¸´ºÏÔÚÒ»Æğ£¬VOEÓĞĞ§Ê±£¬²ÉÑùÎªµÍ(0)£»
28035DspµÄVOEĞÅºÅÊÇµ¥¶ÀµÄ£¬È±Ïà²ÉÑùÎªPLĞÅºÅ£»
18.5kw ÒÔÉÏ²ÅÓĞÈ±ÏàµçÂ·
************************************************************/
void InputPhaseLoseDetect(void)			
{
	if((gSubCommand.bit.InputLost == 0) ||                      //ÔËĞĞ»ò×ªËÙ¸ú×Ù×´Ì¬²Å¼ì²â
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
    
	if(PL_INPUT_HIGH)           // PLÊÇ¸ßµçÆ½		
	{
		gInLose.UpCnt ++;
		gInLose.UpCntRes ++;
	}
    
	if(gInLose.UpCntRes != 0)	// ³ÖĞø500msµÄPLµÍµçÆ½ÅĞ¶ÏÎª¼ÌµçÆ÷¹ÊÕÏ
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
	if(gInLose.Cnt < 500)       //  È±Ïà¼ì²â1secÒ»¸öÑ­»· 
    {
        return;
    }

	if((gInLose.UpCnt > 5) && (gInLose.UpCnt < 485))    // 1secÄÚPL´æÔÚµÍµçÆ½£¬ÎªÈ±Ïà·½²¨
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
	·çÉÈ¿ØÖÆ
1£©	ÉÏµç»º³å×´Ì¬½áÊøºó1ÃëÖÓÄÚ£¬·çÉÈ²»ÔËĞĞ£»
2£©	ÔËĞĞ×´Ì¬£¬·çÉÈÔËĞĞ£»
3£©	Ö±Á÷ÖÆ¶¯µÈ´ıÆÚ¼ä£¬·çÉÈÔËĞĞ
4£©	£¿éÎÂ¶ÈµÍÓÚ40¶È£¬·çÉÈÍ£Ö¹£»ÎÂ¶È¸ßÓÚ42¶È·çÉÈÔËĞĞ£»40¡«42¶ÈÖ®¼ä²»±ä»¯¡£
5)	·çÉÈÆô¶¯ºóÖÁÉÙ10Ãë²Å¹Ø±Õ
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
	±äÆµÆ÷ºÍµç»ú¹ıÔØ±£»¤
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
        return;		    //Ã¿10msÅĞ¶ÏÒ»´Î
	}
	gOverLoad.Cnt = 0;

	////////////////////////////////////////////////////////////////
	//Ñ¡Ôñ¹ıÔØÇúÏß

    if(1 == gInvInfo.GPType)        //GĞÍ»ú¹ıÔØÇúÏß
    {
        m_TorCurLine    = (Uint *)gInvOverLoadTable;
        m_TorCurBottom  = 1150;
        m_TorCurUpper   = 1870;
        m_TorCurStep    = 90;
        m_TorCurData    = 20;
    }
    else                            //PĞÍ»ú¹ıÔØÇúÏß
    {       
        m_TorCurLine    = (Uint *)gInvOverLoadTableForP;
        m_TorCurBottom  = 1050;
        m_TorCurUpper   = 1370;
        m_TorCurStep    = 40;
        m_TorCurData    = 10;
    }

	////////////////////////////////////////////////////////////////
	//¿ªÊ¼ÅĞ¶Ï±äÆµÆ÷µÄ¹ıÔØ
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
		else if(m_Cur < 1000)       /*Ğ¡ÓÚ±äÆµÆ÷¶î¶¨µçÁ÷£¬°´ÕÕµ±Ç°µçÁ÷´óĞ¡Ïû³ı¹ıÔØÀÛ¼ÆÁ¿*/
		{
			gOverLoad.InvTotal.all -= gInvOverLoadDecTable[m_Cur/100 + 1];
		}
	}
	else
	{
		if(gOverLoad.FilterRealFreq < 500)		//µçÁ÷ = µçÁ÷/[0.9*(f/5)+0.1]
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
    //±äÆµÆ÷¹ıÔØÔ¤±¨¾¯´¦Àí
	if(((gOverLoad.InvTotal.all + m_LDeta * 1000UL)>>16) > 36000)
	{
		gMainStatus.StatusWord.bit.PerOvLoadInv = 1;
	}
	else
	{
		gMainStatus.StatusWord.bit.PerOvLoadInv = 0;
	}

	////////////////////////////////////////////////////////////////
	//¿ªÊ¼ÅĞ¶Ïµç»úµÄ¹ıÔØ
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
	m_Cur = ((Ulong)m_Cur * 100L)/gComPar.MotorOvLoad;          // ¸ù¾İ¹ıÔØ±£»¤ÏµÊı¼ÆËã³ö±£»¤µçÁ÷£¬
                                                            	//È»ºóÓÃ¸Ã±£»¤µçÁ÷²éÑ¯¹ıÔØ±£»¤ÇúÏß
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
		else if(m_Cur < 1000)                   /*Ğ¡ÓÚ100%¶î¶¨µçÁ÷°´ÕÕµçÁ÷Ïû³ıµç»ú¹ıÔØ*/
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
    //µç»ú¹ıÔØÔ¤±¨¾¯´¦Àí   
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
	´¦ÓÚÖğ²¨ÏŞÁ÷×´Ì¬ÏÂµÄ¹ıÔØ±£»¤
µ¥¹Ü³ÖĞøÖğ²¨ÏŞÁ÷Ê±¼ä³¬¹ı500ms±£»¤£¬¶ÔÓ¦250¸ö2ms
************************************************************/
void CBCLimitCurProtect(void)
{
	int     m_CurU,m_CurV,m_CurW,m_Coff;
	int	    m_Max,m_Add,m_Sub;
    Uint    m_Limit;
    
	if(gSubCommand.bit.CBCEnable == 1)
	{
		if(gCBCProtect.EnableFlag == 0) SetCBCEnable();	//ÆôÓÃÖğ²¨ÏŞÁ÷¹¦ÄÜ
	}
	else
	{
		if(gCBCProtect.EnableFlag == 1)  SetCBCDisable();//È¡ÏûÖğ²¨ÏŞÁ÷¹¦ÄÜ
	}
		
	//¿ªÊ¼·Ö±ğ¼ÆËãÈıÏàµçÁ÷¾ø¶ÔÖµµÄ»ı·Ö
	m_Coff = (((long)gMotorInfo.Current)<<10) / gInvInfo.InvCurrent;
	m_CurU = (int)(((long)gIUVWQ12.U * (long)m_Coff)>>10);
	m_CurU = abs(m_CurU);
	m_CurV = (int)(((long)gIUVWQ12.V * (long)m_Coff)>>10);
	m_CurV = abs(m_CurV);
	m_CurW = (int)(((long)gIUVWQ12.W * (long)m_Coff)>>10);
	m_CurW = abs(m_CurW);
    
	//¿ªÊ¼ÅĞ¶ÏÊÇ·ñÓĞÒ»Ïà³ÖĞø´óµçÁ÷³¬¹ı5Ãë
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

	if((gCBCProtect.CntU > 2500) || 		//µ¥Ïà´óµçÁ÷±¨¹ıÔØ, ÈÎºÎÒ»Ïà³ÖĞø5000ms
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
    
    // ¸ù¾İÎÂ¶ÈºÍÉè¶¨CBCÊ±¼äµãÈ·¶¨CBCÊ±¼ä
    m_Limit = 500;        //Öğ²¨ÏŞÁ÷ 500MS£¬È¡Ïû¹¦ÄÜÉèÖÃ 2011.5.7 L1082£»
   /*
    if(gTemperature.Temp < 40)  // 40¶È
    {
        m_Limit = gCBCProtect.maxCBCTime * 100;     // µ¥Î»×ª»»: 0.1sec -> 1ms
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
    m_Limit *= 3;       // ÓÉÓÚ3ÏàÍ¬Ê±¹Ø¶Ï£¬ 
                        // 28035 ¿Ï¶¨ÊÇÈıÏàÍ¬Ê±¼ÆÊı
    */
    
	m_Add = 2;
	m_Sub = 1;
	if(gMainStatus.RunStep == STATUS_STOP)
    {
        m_Sub = 2;
    }

	if(gCBCProtect.Flag.bit.CBC_U == 1)   //¼ÆËãUÏàµçÁ÷µÄ»ı·Ö
	{
		gCBCProtect.TotalU += m_Add;
	}
	else //if(m_CurU < 8688)					//Ğ¡ÓÚ1.5±¶±äÆµÆ÷·åÖµµçÁ÷ºóÀÛ¼ÓÖµµİ¼õ : 4096*1.5*sqrt(2)
	{
		gCBCProtect.TotalU -= m_Sub;
	}

	if(gCBCProtect.Flag.bit.CBC_V == 1) 	//¼ÆËãVÏàµçÁ÷µÄ»ı·Ö
	{
		gCBCProtect.TotalV += m_Add;
	}
	else //if(m_CurV < 8688)					//Ğ¡ÓÚ1.5±¶±äÆµÆ÷µçÁ÷ºóÀÛ¼ÓÖµµİ¼õ
	{
		gCBCProtect.TotalV -= m_Sub;
	}

	if(gCBCProtect.Flag.bit.CBC_W == 1) 	//¼ÆËãWÏàµçÁ÷µÄ»ı·Ö
	{
		gCBCProtect.TotalW += m_Add;
	}
	else //if(m_CurW < 8688)					//Ğ¡ÓÚ1.5±¶±äÆµÆ÷µçÁ÷ºóÀÛ¼ÓÖµµİ¼õ
	{
		gCBCProtect.TotalW -= m_Sub;
	}

	gCBCProtect.Flag.all = 0;

	//µçÁ÷»ı·ÖÖµÏŞ·ù
	gCBCProtect.TotalU = __IQsat(gCBCProtect.TotalU, m_Limit, 0);
	gCBCProtect.TotalV = __IQsat(gCBCProtect.TotalV, m_Limit, 0);
	gCBCProtect.TotalW = __IQsat(gCBCProtect.TotalW, m_Limit, 0);

	m_Max = (gCBCProtect.TotalU > gCBCProtect.TotalV) ? gCBCProtect.TotalU : gCBCProtect.TotalV;
	m_Max = (m_Max > gCBCProtect.TotalW) ? m_Max : gCBCProtect.TotalW;
    if(m_Max >= m_Limit)      //Öğ²¨ÏŞÁ÷±¨40ºÅ¹ÊÕÏ
    {
        gError.ErrorCode.all |= ERROR_TRIP_ZONE;
    }
}

/*************************************************************
	¿ªÆôÖğ²¨ÏŞÁ÷¹¦ÄÜ
*************************************************************/
void SetCBCEnable(void)
{
	gCBCProtect.EnableFlag = 1;
    EALLOW;
    
#ifdef TMS320F2808	
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
	EPwm1Regs.TZSEL.bit.CBC3 = TZ_ENABLE;		// EPWM1µÄÖğ²¨ÏŞÁ÷
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
	¹Ø±ÕÖğ²¨ÏŞÁ÷¹¦ÄÜ
*************************************************************/
void SetCBCDisable(void)
{
	gCBCProtect.EnableFlag = 0;

	EALLOW;

#ifdef TMS320F2808
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
	EPwm1Regs.TZSEL.bit.CBC3 = TZ_DISABLE;		// EPWM1µÄÖğ²¨ÏŞÁ÷
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
	Êä³öµôÔØ´¦Àí

ÔÚÔËĞĞ×´Ì¬£¬µçÁ÷Ğ¡ÓÚµç»ú¶î¶¨µçÁ÷5%£¬²¢³ÖĞø100msÈÏÎªÊÇµôÔØ
Í¬²½»ú²»×öµôÔØ±£»¤
*************************************************************/
void LoadLoseDetect(void)
{
	Uint m_Limit,m_LimitFreq;

	//µôÔØÅĞ¶Ï·§ÖµÎªµç»ú¶î¶¨µçÁ÷µÄ5%¡£
	m_Limit = (gLoadLose.ChkLevel * (Ulong)gMotorInfo.CurrentGet) / gMotorInfo.Current << 2; // *4096/1000
	m_Limit = (long)m_Limit * 1447L >> 10;              // * sqrt(2)
	m_Limit = (m_Limit < 20) ? 20 : m_Limit;
	m_LimitFreq = ((Ulong)gMotorInfo.FreqPer * 1638) >> 15;

	if((gMotorInfo.MotorType == MOTOR_TYPE_PM)              // Í¬²½»ú²»×öµôÔØ¼ì²â
        ||(gSubCommand.bit.LoadLose != 1)
        || (abs(gMainCmd.FreqSyn) < m_LimitFreq)			//µç»ú¶î¶¨ÆµÂÊ5%ÒÔÏÂ²»¼ì²â
	    || (gMainCmd.Command.bit.StartDC == 1)
	    || (gMainCmd.Command.bit.StopDC  == 1)				//Ö±Á÷ÖÆ¶¯×´Ì¬²»¼ì²â
	    || (gMainStatus.RunStep != STATUS_RUN)				//·ÇÔËĞĞ×´Ì¬²»¼ì²â
	    || (abs(gIUVWQ12.U) >= m_Limit)                     //ÈÎºÎÒ»ÏàµçÁ÷´óÓÚ200 (5%) ÈÏÎª²»µôÔØ
	    || (abs(gIUVWQ12.V) >= m_Limit) 
	    || (abs(gIUVWQ12.W) >= m_Limit) 
	    )							
	{
		gLoadLose.ErrCnt = 0;
		gMainStatus.StatusWord.bit.OutOff = 0;
		return;
	}
	
	gLoadLose.ErrCnt++;
    if((gLoadLose.ErrCnt<<1) > (gLoadLose.ChkTime*100))     // µôÔØ¼ì³öÊ±¼äÈ·¶¨
	//if(gLoadLose.ErrCnt > 50)
	{
		gLoadLose.ErrCnt = 50;
        gError.ErrorCode.all |= ERROR_LOAD_LOST;
		gMainStatus.StatusWord.bit.OutOff = 1;
	}
}
/*************************************************************
	»º³åµç×è±£»¤´¦Àí
	
³ÖĞøµÄ½øÈëÇ·Ñ¹×´Ì¬£¬ÈÏÎªÊÇ»º³åµç×è¹ÊÕÏ
*************************************************************/
void BufferResProtect(void)
{
	if(gBuffResCnt >= 150000)			//»º³åµç×è±£»¤´¦Àí
	{
		gError.ErrorCode.all |= ERROR_RESISTER_HOT;
	}
	gBuffResCnt--;	
	gBuffResCnt = __IQsat(gBuffResCnt,200000,0);					
}

