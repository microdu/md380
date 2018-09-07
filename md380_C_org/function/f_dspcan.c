/*************** (C) COPYRIGHT 2012 Co., Ltd****************
* File Name          : f_dspcan.c
* Author             : 	
* Version            : V0.0.1
* Date               : 08/09/2012
* Description        : DSP CAN×ÜÏßµ×²ãÇý¶¯¿â

********************************************************************************/
#include "DSP28x_Project.h"     							// DSP2803x Headerfile Include File	
//#include "main.h"											// °üº¬Í·ÎÄ¼þ

#include "f_funcCode.h"
#include "f_dspcan.h"



#define DEBUG_F_CAN              1



#if DEBUG_F_CAN

#if (DSP_CLOCK == 100)
	#define		DSPCAN_CLK		100000
#else
	#define		DSPCAN_CLK		30000
#endif

const	CAN_BAUD	eCanBaud[CAN_BAUD_SUM] = {
									{(DSPCAN_CLK/20/20)-1, 3, 14},	// 20Kbps	
									{(DSPCAN_CLK/20/50)-1, 3, 14},	// 50Kbps		
									{(DSPCAN_CLK/20/100)-1, 3, 14},	// 100Kbps	
									{(DSPCAN_CLK/20/125)-1, 3, 14},	// 125Kbps		3+14+ 2 +1 = 20
									{(DSPCAN_CLK/20/250)-1, 3, 14},	// 250Kbps		3+14+ 2 +1 = 20
									{(DSPCAN_CLK/20/500)-1, 3, 14},	// 500Kbps		3+14+ 2 +1 = 20
									{(DSPCAN_CLK/10/1000)-1, 1, 6} //  1Mbps		1+6 + 2 +1 = 10
								};

Uint32 eCanTranEnFlag;// = 0;
Uint32 eCanReEnFlag;// = 0;
	
/*******************************************************************************
* º¯ÊýÃû³Æ          : Uint16 InitdspECan(Uint16 baud)
* Èë¿Ú²ÎÊý			: CAN½Ó¿Ú²¨ÌØÂÊ£¬
* ³ö¿Ú				£ºCAN_INIT_TIME	 ³õÊ¼»¯½øÐÐÖÐ
*					  CAN_INIT_SUCC  ³õÊ¼»¯³É¹¦
*					  CAN_INIT_TIMEOUT ³õÊ¼»¯³¬Ê±
*					  CAN_INIT_BAUD_ERR ²¨ÌØÂÊ³ö´í
* ´´½¨	            : 	
* °æ±¾		        : V0.0.1
* Ê±¼ä              : 07/29/2012
* ËµÃ÷				: ³õÊ¼»¯DSP Ecan½Ó¿Ú
********************************************************************************/
#define		IINIT_CAN_TIME				3
Uint16 InitdspECan(Uint16 baud)		// Initialize eCAN-A module
{
	struct ECAN_REGS ECanaShadow;							// ÉùÃ÷Ò»¸öÓ°×Ó¼Ä´æÆ÷£¬Ä³Ð©¼Ä´æÆ÷Ö»ÄÜÊ¹ÓÃ32Î»²Ù×÷
	Uint32 *MsgCtrlPi;										// ³õÊ¼»¯ÒýÓÃÖ¸Õë
	Uint16	i;												// Ñ­»·±äÁ¿
	static	Uint16 con = 0;
	static	Uint16 count = 0;								// ³¬Ê±¼ÆÊýÆ÷
	
	if (baud >= CAN_BAUD_SUM)
		return CAN_INIT_BAUD_ERR;							// ²¨ÌØÂÊ³ö´í
	if (count > IINIT_CAN_TIME)								// ³õÊ¼»¯³¬Ê±³ö´í
		return  CAN_INIT_TIMEOUT;
	
	EALLOW;
	if (con == 0)
	{
		GpioCtrlRegs.GPAPUD.bit.GPIO30 = 0;	    // Enable pull-up for GPIO30 (CANRXA)
		GpioCtrlRegs.GPAPUD.bit.GPIO31 = 0;	    // Enable pull-up for GPIO31 (CANTXA)

	/* Set qualification for selected CAN pins to asynch only */
	// Inputs are synchronized to SYSCLKOUT by default.  
	// This will select asynch (no qualification) for the selected pins.

		GpioCtrlRegs.GPAQSEL2.bit.GPIO30 = 3;   // Asynch qual for GPIO30 (CANRXA)   

	/* Configure eCAN-A pins using GPIO regs*/
	// This specifies which of the possible GPIO pins will be eCAN functional pins.

		GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 1;	// Configure GPIO30 for CANTXA operation
		GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 1;	// Configure GPIO31 for CANRXA operation
	
	/* Configure eCAN RX and TX pins for eCAN transmissions using eCAN regs*/  
		ECANREGS.CANTIOC.bit.TXFUNC = 1;
		ECANREGS.CANRIOC.bit.RXFUNC = 1;  

	/* Configure eCAN for HECC mode - (reqd to access mailboxes 16 thru 31) */
										// HECC mode also enables time-stamping feature
		ECanaShadow.CANMC.all = 0;
		ECanaShadow.CANMC.bit.SRES = 1;
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;			// Èí¼þ¸´Î»CANÄ£¿é
		
		ECanaShadow.CANMC.all = ECANREGS.CANMC.all;			// ¶ÁÈ¡CANÖ÷¿ØÖÆ¼Ä´æÆ÷
		ECanaShadow.CANMC.bit.SCB = 1;						// eCANÄ£Ê½				
		ECanaShadow.CANMC.bit.SUSP = 1;						// ÍâÉè²»ÊÜµ÷ÊÔÓ°Ïì
//		ECanaShadow.CANMC.bit.DBO = 1;						// Ê×ÏÈ ×îµÍÓÐÐ§Î» ¸ß×Ö½ÚÔÚÇ°
		ECanaShadow.CANMC.bit.CCR = 1;						// CPUÇëÇóÐÞ¸Ä²¨ÌØÂÊ»òÈ«¾ÖÆÁ±Î¼Ä´æÆ÷
		ECanaShadow.CANMC.bit.ABO = 1;						// ×Ô¶¯»Ö¸´×ÜÏßÊ¹ÄÜ
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;			// »ØÐ´¿ØÖÆ¼Ä´æÆ÷
		
	/* Initialize all bits of 'Master Control Field' to zero */
	// Some bits of MSGCTRL register come up in an unknown state. For proper operation,
	// all bits (including reserved bits) of MSGCTRL must be initialized to zero
		MsgCtrlPi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGCTRL);	// ÏûÏ¢¿ØÖÆÆ÷Ö¸Õë
		for (i=0; i<32; i++)
		{
			MsgCtrlPi[i<<2] = 0x00000000;					// ÇåÁãËùÓÐÏûÏ¢¿ØÖÆ¼Ä´æÆ÷
		}
		MsgCtrlPi = (Uint32 *)(&ECANLAMS.LAM0);				// Ï¢¿ØÖÆÆ÷Ö¸Õë
		for (i=0; i<32; i++)								// Çå¿ÕËùÓÐÆÁ±Î¼Ä´æÆ÷
		{
			MsgCtrlPi[i] = 0x00000000;						// 
		}
		
	/* 
		ECanaMboxes.MBOX0.MSGCTRL.all = 0x00000000;
		..........
		ECanaMboxes.MBOX31.MSGCTRL.all = 0x00000000;
	*/    
	// TAn, RMPn, GIFn bits are all zero upon reset and are cleared again
	//	as a matter of precaution. 
		ECANREGS.CANTRR.all	= 0xFFFFFFFF;					// ¸´Î»·¢ËÍÇëÇó£¬È¡ÏûÕýÔÚ½øÐÐµÄ·¢ËÍ
		ECANREGS.CANTA.all	= 0xFFFFFFFF;					// ÇåÁã·¢ËÍÏìÓ¦¼Ä´æÆ÷/* Clear all TAn bits */      
		ECANREGS.CANRMP.all = 0xFFFFFFFF;					// ½ÓÊÕÏûÏ¢¹ÒÆð¼Ä´æÆ÷/* Clear all RMPn bits */      
		ECANREGS.CANGIF0.all = 0xFFFFFFFF;					// È«¾ÖÖÐ¶Ï±êÖ¾/* Clear all interrupt flag bits */ 
		ECANREGS.CANGIF1.all = 0xFFFFFFFF;
		ECANREGS.CANOPC.all = 0;							// ËùÓÐÓÊÏä¿É±»¸²¸Ç
	/* Configure bit timing parameters for eCANA
		ECanaShadow.CANMC.all = ECANREGS.CANMC.all;
		ECanaShadow.CANMC.bit.CCR = 1 ;            			// CPUÇëÇóÐÞ¸Ä²¨ÌØÂÊ»òÈ«¾ÖÆÁ±Î¼Ä´æÆ÷
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;
	*/	
		con = 1;											// µÚÒ»½×¶ÎÍê³É
	}
    if (con == 1)
	{
		ECanaShadow.CANES.all = ECANREGS.CANES.all;
		if (ECanaShadow.CANES.bit.CCE != 1 ) 				// Wait for CCE bit to be set..
		{
			count++;
			EDIS;
			return CAN_INIT_TIME;							// ³õÊ¼»¯½øÐÐÖÐ
		}
		else
			con = 2;
	}
	
    if (con == 2)
	{
		ECanaShadow.CANBTC.all = 0;                         // ³õÊ¼»¯²¨ÌØÂÊ
		ECanaShadow.CANBTC.bit.BRPREG = eCanBaud[baud].BRPREG;
		ECanaShadow.CANBTC.bit.TSEG2REG = eCanBaud[baud].TSEG2REG;
		ECanaShadow.CANBTC.bit.TSEG1REG = eCanBaud[baud].TSEG1REG; 
		ECanaShadow.CANBTC.bit.SAM = 0;
		ECANREGS.CANBTC.all = ECanaShadow.CANBTC.all;
		
		ECanaShadow.CANMC.all = ECANREGS.CANMC.all;
		ECanaShadow.CANMC.bit.CCR = 0 ;            			// ²¨ÌØÂÊÉèÖÃÍê³É Set CCR = 0
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;
		con = 3;
    }
	if (con == 3)
	{
		ECanaShadow.CANES.all = ECANREGS.CANES.all;
		if (ECanaShadow.CANES.bit.CCE != 0 ) 				// Wait for CCE bit to be  cleared..
		{
			count++;
			EDIS;
			return CAN_INIT_TIME;		
		}
	}
/* Disable all Mailboxes  */

	con = 0;
	count = 0;
 	ECANREGS.CANME.all = 0;									// Required before writing the MSGIDs

    EDIS;
	
	eCanTranEnFlag = 0;                                     // Çå¿ÕÓÊÏä³õÊ¼»¯±êÖ¾
	eCanReEnFlag = 0;
	return CAN_INIT_SUCC;									// ³õÊ¼»¯³É¹¦ 
}	


/*******************************************************************************
* º¯ÊýÃû³Æ          : void InitTranMbox(Uint16 mbox)
* Èë¿Ú²ÎÊý			: mbox ÓÊÏä±àºÅ 0~31£¬ 
*					  ID	ÏûÏ¢±êÊ¶ID
* ³ö¿Ú				£ºÎÞ
* ´´½¨	            : 	
* °æ±¾		        : V0.0.1
* Ê±¼ä              : 07/29/2012
* ËµÃ÷				: ³õ»¯CAN·¢ËÍÓÊÏä£¬¿É³õÊ¼»¯Îª×Ô¶¯Ó¦´ðÓÊÏä
********************************************************************************/
void InitTranMbox(Uint16 mbox, Uint32 msgid, Uint32 *dataPi)
{
	Uint16 id;
	Uint32 ECanaShadow, *msgIdPi;	                        //Ö¸Õë¸³ÖµÏûÏ¢IDµØÖ·

	id = mbox & 0x1f;
	eCanTranEnFlag |= 1ul <<mbox;							// ÓÊÏä³õÊ¼»¯·¢ËÍ±êÖ¾

	msgIdPi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);
	msgIdPi[id<<2] = msgid;								    // Ð´ÏûÏ¢±êÖ¾£¬È·¶¨ÊÇ·ñÎª×Ô¶¯Ó¦´ðÓÊÏä
	msgIdPi[(id<<2) +1] = 8;
	
	ECanaShadow = ECANREGS.CANMD.all;
	ECanaShadow &= ~(1ul<<id);
	ECANREGS.CANMD.all = ECanaShadow;						// ÇåÁãÉèÖÃÎª·¢ËÍÓÊÏä

	ECanaShadow = ECANREGS.CANME.all;
	ECanaShadow |= 1ul<<id;
	ECANREGS.CANME.all = ECanaShadow;						// Ê¹ÄÜ¶ÔÓ¦ÓÊÏä

	msgIdPi[(id<<2) + 2] = *dataPi++;						// Ð´×Ô¶¯Ó¦´ðÐÅÏ¢µ½
	msgIdPi[(id<<2) + 3] = *dataPi;	
	
}

/*******************************************************************************
* º¯ÊýÃû³Æ          : void InitReMbox(Uint16 mbox, union CANMSGID_REG msgid, union CANLAM_REG lam)
* Èë¿Ú²ÎÊý			: mbox ÓÊÏä±àºÅ 0~31£¬bit7 ¡°1¡± ½ÓÊÕÔ¶³ÌÖ¡ ¡°0¡±ÕÍÖ?bit6 "1"¸²¸Ç±£»¤
*					  msgid	ÏûÏ¢±êÊ¶ID
*					  lam	½ÓÊÕÆÁ±Î¼Ä´æÆ÷
* ³ö¿Ú				£ºÎÞ
* ´´½¨	            : 	
* °æ±¾		        : V0.0.1
* Ê±¼ä              : 07/29/2012
* ËµÃ÷				: ³õ»¯CAN½ÓÊÕÓÊÏä
********************************************************************************/
void InitRecMbox(Uint16 mbox, Uint32 msgid, Uint32 lam)
{
	Uint16 id;
	Uint32 ECanaShadow,  *pi;								// = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);
	
	id = mbox & 0x1f;
	eCanReEnFlag |= 1ul << id;
	
	pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);
	pi[id<<2] = msgid;										// ÏûÏ¢±êÊ¶¼Ä´æÆ÷
	if ((mbox & 0x80) == 0x80)								// ·¢ËÍÔ¶³ÌÖ¡´«Êä³õÊ¼»¯
		pi[(id<<2) +1] = 1<<4 | 8;							// ÏûÏ¢¿ØÖÆ¼Ä´æÆ÷
	else
		pi[(id<<2) +1] = 8;
		
	ECanaShadow = ECANREGS.CANOPC.all;
	if ( (mbox & 0x40) == 0x40 )							// Ê¹ÄÜ¸²¸Ç±£»¤¼ì²é£¬Ö÷³õÊ¼»¯ÖÐÒÑ¾­½«ËùÓÐÓÊÏä½ûÖ¹¸²¸Ç±£»¤
		ECanaShadow |= 1ul<<id;
	else
		ECanaShadow &= ~(1ul<<id);
	ECANREGS.CANOPC.all = ECanaShadow;
		
	ECanaShadow = ECANREGS.CANMD.all;						// ÖÃ¡°1¡±ÉèÖÃÎª½ÓÊÕÓÊÏä
	ECanaShadow |= 1ul<<id;
	ECANREGS.CANMD.all = ECanaShadow;						// 
	
	ECanaShadow = ECANREGS.CANME.all;
	ECanaShadow |= 1ul<<id;
	ECANREGS.CANME.all = ECanaShadow;						// Ê¹ÄÜ¶ÔÓ¦ÓÊÏä
	
	pi = (Uint32 *)(&ECANLAMS.LAM0);						// ÅäÖÃ½ÓÊÕÆÁ±Î¼Ä´æÆ÷
	pi[id] = lam;
}


/*******************************************************************************
* º¯ÊýÃû³Æ          : Uint16 eCanDataTran(Uint16 mbox, Uint16 len, Uint32 msgid, Uint32 *dataPi)
* Èë¿Ú²ÎÊý			: mbox ÓÊÏä±àºÅ 0~31£¬
*					  ID	ÏûÏ¢±êÊ¶ID			Ö»°üº¬ÓÐÐ§IDÎ»
* ³ö¿Ú				£ºCAN_MBOX_NUM_ERROR		ÓÊÏäºÅ³ö´í£¬¸ÃÓÊÏäÎ´±»³õÊ¼»¯Îª·¢ËÍÓÊÏä
*					  CAN_MBOX_BUSY				ÓÊÏäÃ¦
*					  CAN_MBOX_TRAN_SUCC		·¢ËÍ³É¹¦
* ´´½¨	            : 	
* °æ±¾		        : V0.0.1
* Ê±¼ä              : 08/25/2012
* ËµÃ÷				: Ö¸¶¨ÓÊÏä·¢ËÍÊý¾Ý£¬ÓÊÏä±ØÐë±»³õÊ¼»¯Îª·¢ËÍÓÊÏä
********************************************************************************/
Uint16 eCanDataTran(Uint16 mbox, Uint16 len, Uint32 msgid, Uint32 *dataPi)
{
	Uint32 ECanaShadow, *pi;
	mbox &= 0x1f;
	if ( (eCanTranEnFlag & (1ul << mbox)) != (1ul << mbox) )
	{
		return (CAN_MBOX_NUM_ERROR);						// CANÓÊÏäºÅ³ö´í£¬ÓÊÏäÎ´³õÊ¼»¯
	}
	
	if (ECANREGS.CANTRS.all & (1ul << mbox))				// ¼ì²éÉÏ´Î·¢ËÍÊÇ·ñÍê³É£¬·¢ËÍÇëÇó±êÖ¾ÖÃÎ»
	{
		return (CAN_MBOX_BUSY);								// CANÓÊÏäÃ¦
	}
	
	ECANREGS.CANTA.all = 1ul << mbox;						// Çå¿Õ·¢ËÍÏìÓ¦±êÖ¾
	
	pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);				// Ð´ID£¬Ð´Êý¾Ý
	
	msgid &= ~(0x7ul<<29);									// Çå³ý¸ßÈýÎ»
	msgid |= pi[mbox<<2] & (0x7ul << 29);					// ²»ÐÞ¸ÄIDÅäÖÃÎ»
	
	ECanaShadow = ECANREGS.CANME.all;
	ECanaShadow &= ~(1ul<<mbox);
	ECANREGS.CANME.all = ECanaShadow;						// ½ûÖ¹¶ÔÓ¦ÓÊÏä
	
	pi[mbox<<2] = msgid;									// ÖØÐ´ID
	pi[(mbox<<2) + 1] = len;
	pi[(mbox<<2) + 2] = *dataPi++;							// Ð´Êý¾Ý
	pi[(mbox<<2) + 3] = *dataPi;
	
	ECanaShadow |= 1ul<<mbox;
	ECANREGS.CANME.all = ECanaShadow;						// Ê¹ÄÜ¶ÔÓ¦ÓÊÏä	

	ECANREGS.CANTRS.all = 1ul << mbox;						// Ê¹ÄÜ·¢ËÍ
	return (CAN_MBOX_TRAN_SUCC);
}

/*******************************************************************************
* º¯ÊýÃû³Æ          : Uint16 eCanDataRec(Uint16 mbox, Uint32 *dataPi)
* Èë¿Ú²ÎÊý			: mbox ÓÊÏä±àºÅ 0~31£¬
*					  *	dataPi ½ÓÊÕ»º´æ
* ³ö¿Ú				£ºCAN_MBOX_NUM_ERROR		ÓÊÏäºÅ³ö´í£¬¸ÃÓÊÏäÎ´±»³õÊ¼»¯Îª·¢ËÍÓÊÏä
*					  CAN_MBOX_EMPTY			½ÓÊÕÓÊÏä¿Õ
*					  CAN_MBOX_REC_SUCC			½ÓÊÕÊý¾Ý³É¹¦
*					  CAN_MBOX_REC_OVER			½ÓÊÕÊý¾ÝÒç³ö
* ´´½¨	            : 	
* °æ±¾		        : V0.0.1
* Ê±¼ä              : 08/25/2012
* ËµÃ÷				: ½ÓÊÕÊý¾Ý½ÓÊÕ»º´æÇø
********************************************************************************/
Uint16 eCanDataRec(Uint16 mbox, Uint32 *dataPi)
{
	Uint32 *pi;
	
	mbox &= 0x1f;
//	if ( (eCanReEnFlag & (1ul << mbox)) != (1ul << mbox))
//	{
//		return (CAN_MBOX_NUM_ERROR);						// CANÓÊÏäºÅ³ö´í£¬ÓÊÏäÎ´³õÊ¼»¯
//	}
	if (ECANREGS.CANRMP.all & (1ul << mbox) )				// ¼ì²éÊÇ·ñÓÐ½ÓÊÕÏûÏ¢¹ÒÆð
	{
		pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);			
		*dataPi++ = pi[mbox<<2];							//  ¶ÁID£¬¶ÁÊý¾Ý
		*dataPi++ = pi[(mbox<<2) + 2];	
		*dataPi++ = pi[(mbox<<2) + 3];
        *dataPi   = pi[(mbox<<2) + 1] & 0xf;                // ¶ÁÈ¡½ÓÊÕÊý¾Ý³¤¶È

//		ECanaShadow = 1ul<<mbox;
		
		if (ECANREGS.CANRML.all & (1ul << mbox))			// ¼ì²éÓÊÏäÊÇ·ñ±»¸²¸Ç¹ý
		{
			ECANREGS.CANRMP.all = 1ul<<mbox;				// Çå³ýÏûÏ¢¹ÒÆð¼Ä´æÆ÷
			return (CAN_MBOX_REC_OVER);
		}	
		else
		{
			ECANREGS.CANRMP.all = 1ul<<mbox;				// Çå³ýÏûÏ¢¹ÒÆð¼Ä´æÆ÷
			return (CAN_MBOX_REC_SUCC);		
		}
	}
	else
	{
		return (CAN_MBOX_EMPTY);							// CANÓÊÏä¿Õ£¬ÎÞ¿É¶ÁÈ¡Êý¾Ý		
	}
}




#elif 1



#endif




