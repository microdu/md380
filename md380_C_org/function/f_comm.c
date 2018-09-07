//======================================================================
//
// Í¨Ñ¶´¦Àí¡£ModBusÐ­Òé¡£
// 
// Time-stamp: 
//
//======================================================================

#include "f_comm.h"
#include "f_p2p.h"
#include "f_ui.h"

#if F_DEBUG_RAM                     // ½öµ÷ÊÔ¹¦ÄÜ£¬ÔÚCCSµÄbuild optionÖÐ¶¨ÒåµÄºê
#define DEBUG_F_MODBUS          0   // ÊÇ·ñÊ¹ÓÃÍ¨Ñ¶²¿·Ö
#elif 1
#define DEBUG_F_MODBUS          1
#endif

#define COMM_PARITY_MAX            3

enum COMM_STATUS commStatus;    // ´®¿Ú³õÊ¼»¯ÎªµÈ´ý½ÓÊÕ×´Ì¬
COMM_SEND_DATA commSendData;
COMM_RCV_DATA commRcvData;
Uint16 commType;                 // ´®¿ÚÍ¨Ñ¶Ð­ÒéÀàÐÍ
Uint16 commProtocol;             // Í¨Ñ¶Ð­ÒéÊý¾Ý¸ñÊ½
Uint32 commTicker;               // *_0.5ms
Uint32 canLinkTicker;            // *_0.5ms
Uint16 commRunCmd;               // Í¨Ñ¶¿ØÖÆÔËÐÐÃüÁî×Ö
Uint16 aoComm[AO_NUMBER+1];      // Í¨Ñ¶AO¿ØÖÆ. 0-HDO, ÆäËûÎªAO
union DO_SCI doComm;             // Í¨Ñ¶DO¿ØÖÆ
Uint16 userPwdPass4Comm;         // 1-ÓÃ»§ÃÜÂëÍ¨¹ý£¬½öÍ¨Ñ¶FP-01Ê¹ÓÃ
Uint16 companyPwdPass4Comm;      // 1-³§¼ÒÃÜÂëÍ¨¹ý£¬FF×éÊ¹ÓÃ
Uint16 groupHidePassComm;        // 1-¹¦ÄÜ×éÒþ²ØÃÜÂëÍ¨¹ý£¬AF×éÊ¹ÓÃ
LOCALF Uint16 sendFrame[35];                           // ´Ó»úÏìÓ¦Ö¡
LOCALF Uint16 rcvFrame[35];                            // ½ÓÊÕÊý¾ÝÊý×é(Ö÷»úÃüÁîÖ¡)
LOCALF Uint16 commReadData[RTU_READ_DATA_NUM_MAX];     // ¶ÁÈ¡µÄÊý¾Ý
LOCALF union SCI_FLAG sciFlag;                         // SCIÊ¹ÓÃµÄ±êÖ¾

#if DEBUG_F_MODBUS

#if (DSP_CLOCK == 100)
// 100*10^6/4/(_*8)-1
// Êµ¼Ê²¨ÌØÂÊÊÇ 100*10^6/4/(_*8)
#if !DSP_2803X
LOCALF const DSP_BAUD_REGISTER_DATA dspBaudRegData[BAUD_NUM_MAX + 1] =  // LSPCLK = SYSCLKOUT/4
{
    {   3, 0x0028, 0x00af},           // 0, 300bps
    {   6, 0x0014, 0x0057},           // 1, 600bps
    {  12, 0x000a, 0x002b},           // 2, 1200bps
    {  24, 0x0005, 0x0015},           // 3, 2400bps
    {  48, 0x0002, 0x008a},           // 4, 4800bps
    {  96, 0x0001, 0x0044},           // 5, 9600bps
    { 192, 0x0000, 0x00a1},           // 6, 19200bps
    { 384, 0x0000, 0x0050},           // 7, 38400bps
    { 576, 0x0000, 0x0035},           // 8, 57600bps
    {1152, 0x0000, 0x001a},           // 9, 115200bps
    {1280, 0x0000, 0x000E},           // 10,208300bps
    {2560, 0x0000, 0x000b},           // 11,256000bps
    {5120, 0x0000, 0x0005},           // 12,512000bps
};
#endif
#elif (DSP_CLOCK == 60)                               // DSPÔËÐÐÆµÂÊ60MHz

LOCALF const DSP_BAUD_REGISTER_DATA dspBaudRegData[BAUD_NUM_MAX + 1] =  // LSPCLK = SYSCLKOUT/4
{
#if DSP_2803X
    {   3, 0x0000, 0x1869,  0},           // 0, 300bps
    {   6, 0x0000, 0x0C34,  0},           // 1, 600bps
    {  12, 0x0000, 0x0619,  8},           // 2, 1200bps
    {  24, 0x0000, 0x030c,  4},           // 3, 2400bps
    {  48, 0x0000, 0x0185, 10},           // 4, 4800bps
    {  96, 0x0000, 0x00c2,  5},           // 5, 9600bps
    { 192, 0x0000, 0x0060, 11},           // 6, 19200bps
    { 384, 0x0000, 0x002f, 13},           // 7, 38400bps
    { 576, 0x0000, 0x001f,  9},           // 8, 57600bps
    {1152, 0x0000, 0x000f,  4},           // 9, 115200bps
    {1280, 0x0000, 0x0008,  0},           // 10, 208300bps
    {2560, 0x0000, 0x0006, 15},           // 11, 256000bps
    {5120, 0x0000, 0x0002, 11},           // 12, 512000bps
#else
    {   3, 0x0018, 0x0069},           //  0, 300bps
    {   6, 0x000c, 0x0034},           //  1, 600bps
    {  12, 0x0006, 0x0019},           //  2, 1200bps
    {  24, 0x0003, 0x000c},           //  3, 2400bps
    {  48, 0x0001, 0x0085},           //  4, 4800bps
    {  96, 0x0000, 0x00c2},           //  5, 9600bps
    { 192, 0x0000, 0x0060},           //  6, 19200bps
    { 384, 0x0000, 0x002f},           //  7, 38400bps
    { 576, 0x0000, 0x001f},           //  8, 57600bps
    {1152, 0x0000, 0x000f},           //  9, 115200bps
    {1280, 0x0000, 0x0008},           // 10, 208300bps
    {2560, 0x0000, 0x0006},           // 11, 256000bps
    {5120, 0x0000, 0x0002},           // 12,512000bps
#endif
};
#endif

#if DSP_2803X
const Uint16 commParitys[4] = {0x10, 0x0c, 0x04, 0x00};  //[STOP PARITYENA PARITY] 100 011 010 000
#else
const Uint16 commParitys[4] = {0x87, 0x67, 0x27, 0x07};
#endif

// Ä³Ð©¹¦ÄÜÂë£¬Í¨Ñ¶²»ÄÜ½øÐÐW
#define COMM_NO_W_FC_0  GetCodeIndex(funcCode.code.tuneCmd)              // µ÷Ð³
#define COMM_NO_W_FC_1  GetCodeIndex(funcCode.code.menuMode)             // ²Ëµ¥Ä£Ê½
#define COMM_NO_W_FC_2  GetCodeIndex(funcCode.code.motorFcM2.tuneCmd)    // µ÷Ð³
#define COMM_NO_W_FC_3  GetCodeIndex(funcCode.code.motorFcM3.tuneCmd)    // µ÷Ð³
#define COMM_NO_W_FC_4  GetCodeIndex(funcCode.code.motorFcM4.tuneCmd)    // µ÷Ð³
#define COMM_NO_W_FC_5  GetCodeIndex(funcCode.code.funcParaView)         // ¹¦ÄÜ²Ëµ¥Ä£Ê½ÊôÐÔ
// Ä³Ð©¹¦ÄÜÂë£¬Í¨Ñ¶²»ÄÜ½øÐÐR
#define COMM_NO_R_FC_0  GetCodeIndex(funcCode.code.userPassword)         // ÓÃ»§ÃÜÂë
// Ä³Ð©¹¦ÄÜÂë£¬Í¨Ñ¶²»ÄÜ½øÐÐRW
#define COMM_NO_RW_FC_0 GetCodeIndex(funcCode.code.userPasswordReadOnly) // Ö»¶ÁÓÃ»§ÃÜÂë
#define COMM_READ_CURRENT_FC GetCodeIndex(funcCode.group.u0[4])          // Í¨Ñ¶¶ÁÈ¡µçÁ÷


const Uint16 COMM_ERR_INDEX[8] = {10, 8, 2, 7, 6, 9, 12, 13};

LOCALD void CommRcvDataDeal(void);
LOCALD void CommSendDataDeal(void);
LOCALD void resetLinSci(void);
LOCALD void closeRTX(void);
LOCALD void setRxConfig(void);
LOCALD void setTxConfig(void);
LOCALD void commErrorDeal(void);
LOCALD void commStatusDeal(void);
LOCALD void CommDataReRcv(Uint16 tmp);
LOCALD void CommDataSend(void);
LOCALD Uint16 CommRead(Uint16, Uint16);
LOCALD Uint16 CommRwFuncCode(Uint16, Uint16, Uint16 rwMode);
LOCALD void inline CommStartSend(void);

#define COMM_READ_FC    0       // Í¨Ñ¶¶Á¹¦ÄÜÂë
#define COMM_WRITE_FC   1       // Í¨Ñ¶Ð´¹¦ÄÜÂë

// ÖÐ¶Ï
#if DSP_2803X
void SCI_RXD_isr(void);
void SCI_TXD_isr(void);
interrupt void Lina_Level0_ISR(void);    // LIN-SCI ÖÐ¶Ï
interrupt void Lina_Level1_ISR(void);    // LIN-SCI ÖÐ¶Ï
#else
interrupt void SCI_RXD_isr(void);       // SCIÖÐ¶Ï
interrupt void SCI_TXD_isr(void);       // SCIÖÐ¶Ï
#endif


// RS485µÄ½ÓÊÕ·¢ËÍÇÐ»»
#if DSP_2803X
#define RTS (GpioDataRegs.GPBDAT.bit.GPIO39)
#else
#define RTS (GpioDataRegs.GPADAT.bit.GPIO27)
#endif
#define RS485_R     0
#define RS485_T     1


//===========================================================
//
// Í¨Ñ¶Ð­Òé
//
//===========================================================
#define COMM_SET_VALUE_ADDR     0x1000      // Í¨Ñ¶Éè¶¨Öµ
#define COMM_CMD1_ADDR          0x2000      // Í¨Ñ¶¿ØÖÆÃüÁî1£¬¿ØÖÆÃüÁî£¬ÆôÍ£µÈ
#define COMM_STATUS_ADDR        0x3000      // ±äÆµÆ÷ÔËÐÐ×´Ì¬
#define COMM_DO_ADDR            0x2001      // DO¿ØÖÆ
#define COMM_HDO_ADDR           0x2004      // HDO¿ØÖÆ
#define COMM_AO1_ADDR           0x2002      // AO1¿ØÖÆ
#define COMM_AO2_ADDR           0x2003      // AO2¿ØÖÆ
#define COMM_INV_ERROR          0x8000      // ±äÆµÆ÷¹ÊÕÏ

#define COMM_KEYBORD_TEST       0xFFFF      // ¼üÅÌ²âÊÔµØÖ·


// Í¨Ñ¶Ð­ÒéÊý¾Ý²Ù×÷º¯Êý
// Ä¿Ç°½öÓÐMODBUSºÍPROFIBUS
// ÊÇ·ñ¿¼ÂÇ½«ÊÖ³Ö²Ù×÷Æ÷µÄÐ­ÒéÒ²Îª¸Ã×ö·¨£¿
#define PROTOCOL_NUM    2
const protocolDeal protocolFunc[PROTOCOL_NUM] =
{
// MODBUS
    { ModbusRcvDataDeal,       ModbusStartDeal,      
      UpdateModbusCommFormat,  ModbusSendDataDeal,
      ModbusCommErrCheck,
    },
    
// PROFIBUS
    { ProfibusRcvDataDeal,     ProfibusStartDeal,
      UpdateProfibusCommFormat,  ProfibusSendDataDeal,  
      ProfibusCommErrCheck,
    }
};


//=====================================================================
//
// Í¨Ñ¶´¦Àíº¯Êý
//
//=====================================================================
void SciDeal(void)
{
    Uint16 commErrFlag;

    // ¸üÐÂ´®¿ÚÅäÖÃ
    UpdateSciFormat();

    // ÃÜÂëÍ¨¹ýÓÐÐ§Ê±¼ä
    if (commTicker >= (Uint32)30 * TIME_UNIT_MS_PER_SEC * 2)    // 30sÖ®ºó£¬·ÃÎÊÈ¨Ê§Ð§
    {
        userPwdPass4Comm = 0;
        companyPwdPass4Comm = 0;
        groupHidePassComm = 0;
    }

    // Í¨Ñ¶³ö´í¼ì²â
	// MODBUS¼ì²â·½Ê½Îª´®¿ÚÍ£¶ÙÊ±¼ä³¬¹ýÍ¨Ñ¶Ó¦´ð³¬Ê±Ê±¼äÉèÖÃ
	// PROFIBUS¼ì²â·½Ê½ÎªCRCÐ£Ñé³ö´í´ÎÊý´ïµ½10´ÎÒÔÉÏ(PROFIBUSÊÇ·ñÓÐ±ØªÕâÑù´¦Àí)
    commErrFlag = protocolFunc[commType].CommErrCheck();

    // Í¨Ñ¶³ö´í
    if (commErrFlag)
    {
        // Í¨Ñ¶³ö´í´¦Àí
        commErrorDeal();     // ±¨¹ÊÕÏ²¢ÖÃ´®¿ÚÎª½ÓÊÕ
    }
    else
    {
        // Í¨Ñ¶¹ý³Ì´¦Àí
        commStatusDeal();
    }
}


//=====================================================================
//
// Í¨Ñ¶½ÓÊÕÖÐ¶Ïº¯Êý
//
//=====================================================================
#if DSP_2803X
void SCI_RXD_isr(void)
#else
interrupt void SCI_RXD_isr(void)
#endif
{
    Uint16 tmp;

#if DSP_2803X
    tmp = LinaRegs.SCIRD;
#else
    tmp = SciaRegs.SCIRXBUF.all;
#endif
    
    // Êý¾Ý½ÓÊÕÖ¡Í·ÅÐ¶Ï
    if (protocolFunc[commType].StartDeal(tmp))
    {
		// ÎªÕý³£µÄ½ÓÊÕÊý¾Ý  0-ÎÞÐ§   1-¹ã²¥µØÖ·    2-±¾»úµØÖ·
		if (commRcvData.rcvFlag)
		{
	        // ·ÇÖ¡Í·Í¨Ñ¶Êý¾Ý½ÓÊÕ
	        CommDataReRcv(tmp);
		}
    }
    
    commTicker = 0;                     // ÓÐ½ÓÊÕÊý¾Ý£¬ÖØÐÂ¼ÆÊ±
    PieCtrlRegs.PIEACK.bit.ACK9 = 1;    // Issue PIE ACK
}


//=====================================================================
//
// Í¨Ñ¶·¢ËÍÖÐ¶Ïº¯Êý
//
// ·¢ËÍÒ»¸ö×Ö·ûÍê³É£¬¾Í½øÈë¸ÃÖÐ¶Ï
//
//=====================================================================
#if DSP_2803X
void SCI_TXD_isr(void)
#else
interrupt void SCI_TXD_isr(void)
#endif
{
	// Í¨Ñ¶·¢ËÍÊý¾Ý
    CommDataSend();   
    commTicker = 0;                     // ·¢ËÍÒ»¸ö×Ö·ûÍê³É£¬ÖØÐÂ¼ÆÊ±                        
    PieCtrlRegs.PIEACK.bit.ACK9 = 1;    // Issue PIE ACK
}


#if DSP_2803X
// ¸ßÓÅÏÈ¼¶ÖÐ¶Ï
interrupt void Lina_Level0_ISR(void)
{
	Uint32 LinL0IntVect;  

	LinL0IntVect = LinaRegs.SCIINTVECT0.all;

	// ½ÓÊÕÖÐ¶Ï
	if(LinL0IntVect == 11)
	{
		SCI_RXD_isr();
	}
	//  ·¢ËÍÖÐ¶Ï
	else if(LinL0IntVect == 12)
	{
		SCI_TXD_isr();
	}
    // other
    else
    {
        PieCtrlRegs.PIEACK.bit.ACK9 = 1;
    }
}

//Low priority BLIN ISR.  Just a placeholder.
interrupt void Lina_Level1_ISR(void)
{
	PieCtrlRegs.PIEACK.bit.ACK9 = 1; 
}
#endif


//=====================================================================
//
// Í¨Ñ¶½ÓÊÕµÄÊý¾Ý´¦Àíº¯Êý
//
//=====================================================================
LOCALD void CommRcvDataDeal(void)
{
    Uint16 writeErr;

	// ²»Í¬Ð­ÒéµÄ½ÓÊÕÊý¾ÝÐÅÏ¢½âÎö
	// ½âÎö²Ù×÷ÃüÁî¡¢µØÖ·¡¢Êý¾ÝµÈÐÅÏ¢
    protocolFunc[commType].RcvDataDeal();
    // ÇåSCI±êÖ¾
    sciFlag.all = 0;

     // ¹ã²¥Ä£Ê½
    if (!commRcvData.slaveAddr)  
    {
		// ¹ã²¥Ð´²Ù×÷
        if ((SCI_CMD_WRITE == commRcvData.commCmd)
          || (SCI_CMD_WRITE_RAM == commRcvData.commCmd)
           )
        {
			// ÖÃÐ´±êÖ¾
            sciFlag.bit.write = 1;
        }
    }
    else //if (RTUslaveAddress == funcCode.code.commSlaveAddress) // ±¾»úµØÖ·ÅÐ¶Ï
    {
		// CRCÐ£Ñé
        if (commRcvData.crcRcv != CrcValueByteCalc(rcvFrame, commRcvData.crcSize))  // CRCÐ£Ñé¹ÊÕÏÅÐ¶Ï
        {         
            sciFlag.bit.crcChkErr = 1;                      // ÖÃÎ»£ºCRCErr¡¢send
			commRcvData.rcvCrcErrCounter++;                 // ¼ÇÂ¼CRCÐ£Ñé³ö´í´ÎÊý
        }
        else if (SCI_CMD_READ == commRcvData.commCmd)       // ¶ÁÃüÁî²Ù×÷
        {
            sciFlag.bit.read = 1;                           // ÖÃÎ»£ºread¡¢send
        }
        else if ((SCI_CMD_WRITE == commRcvData.commCmd) || (SCI_CMD_WRITE_RAM == commRcvData.commCmd))      // Ð´ÃüÁî²Ù×÷
        {
            sciFlag.bit.write = 1;			                        
        }
        else
        {   
            sciFlag.bit.cmdErr = 1;                                                 // ÃüÁî´íÎó
        }
    }

    // Ð´Êý¾Ý´¦Àí
    if (sciFlag.bit.write)
    {

#if 0
        // Ð´EEPROM
        if (SCI_CMD_WRITE == commRcvData.commCmd)
        {
            commRcvData.commCmdSaveEeprom = SCI_WRITE_WITH_EEPROM;
        }
#endif
        writeErr = CommWrite(commRcvData.commAddr, commRcvData.commData);  
        // Ð´Ê§°Ü
        if (writeErr)
        {
			// ±êÊ¾Ð´Ê§°Ü¹ÊÕÏ
            sciFlag.all |= (0x0001 << COMM_ERR_INDEX[writeErr - 1]);
        }  
    }
}

//=====================================================================
//
// Êý¾Ý½ÓÊÕ´¦Àíºó¹ÊÕÏÕûÀí
//
//=====================================================================
Uint16 SciErrCheck(void)
{
    Uint16 readErr;
	Uint16 operateErr;
	
	// ³õÖµÖÃÎÞ¹ÊÕÏ
	operateErr = COMM_ERR_NONE;
    
    // Í¨Ñ¶¶ÁÃüÁî
    if (sciFlag.bit.read)               // Í¨Ñ¶²ÎÊý¶ÁÈ¡´¦Àí
    {
        if(commRcvData.commData > RTU_READ_DATA_NUM_MAX)    // ×î´ó¶ÁÈ¡12¸öÊý¾Ý
        {
            sciFlag.bit.paraOver = 1;   //  ²ÎÊý´íÎó¹ÊÕÏ
        }
        else
        {
            readErr = CommRead(commRcvData.commAddr, commRcvData.commData);
            if (readErr)
            {
				// Êý¾Ý¶ÁÈ¡. Èç¹û´íÎó£¬ÖÃ¹ÊÕÏÎ»£¬²»ÐèÒªÕæÕýµÄ¶ÁÈ¡
                sciFlag.all |= (0x0001 << COMM_ERR_INDEX[readErr - 1]);                  
            }
        }
    }

	// ¹ÊÕÏÐÅÏ¢´¦Àí
    if (sciFlag.bit.pwdErr)                 // ÃÜÂë´íÎó£ºErr01
    {
        operateErr = COMM_ERR_PWD;
    }
    else if (sciFlag.bit.cmdErr)            // ¶ÁÐ´ÃüÁî´íÎó£ºErr02
    {
        operateErr = COMM_ERR_CMD;
    }
    else if (sciFlag.bit.crcChkErr)         // CRCÐ£Ñé´íÎó: Err03
    {
        operateErr = COMM_ERR_CRC;
    }
    else if (sciFlag.bit.addrOver)          // ¹¦ÄÜÂëÎÞÐ§µØÖ·£ºErr04
    {
        operateErr = COMM_ERR_ADDR;
    }
    else if (sciFlag.bit.paraOver)          // ¹¦ÄÜÂëÎÞÐ§²ÎÊý£ºErr05
    {
        operateErr = COMM_ERR_PARA;
    }
    else if (sciFlag.bit.paraReadOnly)      // ²ÎÊý¸ü¸ÄÎÞÐ§£ºErr06
    {
        operateErr = COMM_ERR_READ_ONLY;
    }
    else if (sciFlag.bit.systemLocked)      // ÏµÍ³Ëø¶¨£º·µ»Ø0x0007
    {
        operateErr = COMM_ERR_SYSTEM_LOCKED;
    }
#if 1   // Ä¿Ç°eeprom´¢´æ»úÖÆÏÂ£¬²»»áÓÐ¸Ã´íÎó£¬µ«±£ÏÕÆð¼û£¬»¹ÊÇ±£Áô
    else if (sciFlag.bit.saveFunccodeBusy)  // ÕýÔÚ´æ´¢²ÎÊý£º·µ»Ø0x0008
    {
        operateErr = COMM_ERR_SAVE_FUNCCODE_BUSY;
    }
#endif

	return operateErr;
}


//=====================================================================
//
// Í¨Ñ¶·¢ËÍÊý¾Ý´¦Àíº¯Êý
//
//=====================================================================
LOCALF void CommSendDataDeal(void)
{
    int16 error;
    Uint16 crcSend;

    // ÅÐ¶ÏÍ¨Ñ¶¶ÁÐ´²Ù×÷¹ÊÕÏ
    error = SciErrCheck();

    // ×¼±¸Ð­Òé·¢ËÍÊý¾Ý
    protocolFunc[commType].SendDataDeal(error);

    // ×¼±¸CRCÐ£ÑéÊý¾Ý
    crcSend = CrcValueByteCalc(sendFrame, commSendData.sendNumMax - 2);
    sendFrame[commSendData.sendNumMax - 2] = crcSend & 0x00ff;    // CRCµÍÎ»ÔÚÇ°
    sendFrame[commSendData.sendNumMax - 1] = crcSend >> 8;
}


//=====================================================================
//
// Í¨Ñ¶Ð´Êý¾Ýº¯Êý
// (Í¨Ñ¶Ò²¿ÉÒÔÐÞ¸Äµç»ú²ÎÊý)
//
//=====================================================================
Uint16 CommWrite(Uint16 addr, Uint16 data)
{
    Uint16 errType = COMM_ERR_NONE;   // ¶ÁÈ¡¹¦ÄÜÂë´íÎóÀàÐÍ
    Uint16 high = (addr & 0xF000);
    static  Uint16 canOpen = 0;
    
// °´¼ü²âÊÔ
    if (COMM_KEYBORD_TEST == addr)
    {
        if (data == 1)
        {
            // ¿ªÊ¼°´¼üÅÐ¶Ï²âÊÔ
            keyBordTestFlag = 1;
            keyBordValue = 0;
        }
    }
// Í¨Ñ¶Éè¶¨Öµ(Í¨Ñ¶ÆµÂÊÉè¶¨)
    else if (COMM_SET_VALUE_ADDR == addr)
    {
        if ((-10000 <= (int16)data) && ((int16)data <= 10000))
        {
            funcCode.code.frqComm = data;
        }
        else
        {
            errType = COMM_ERR_PARA;
        }
    }
// µã¶ÔµãÍ¨Ñ¶Éè¶¨Öµ    
    else if (COMM_P2P_COMM_ADDRESS_DATA == addr)
    {
        if ((-10000 <= (int16)data) && ((int16)data <= 10000))
        {
            p2pData.P2PRevData = data;
        }
        else
        {
            errType = COMM_ERR_PARA;
        }
    }
// Í¨Ñ¶¿ØÖÆÃüÁî´¦Àí
    else if (COMM_CMD1_ADDR == addr)                 
    {
        if (data == 8)
        {
            if (canOpen == 0)
            {
                commRunCmd = 1;
                canOpen = 1;
            }
            else
            {
                commRunCmd = 2;
                canOpen = 0;                
            }
        }else if ((1 <= data) && (data <= 7)) // ½ö0001-0007ÃüÁî
        {
            commRunCmd = data;
        }
        else
        {
            commRunCmd = 0;
            errType = COMM_ERR_PARA;
        }
    }
// DO¿ØÖÆ
    else if (COMM_DO_ADDR == addr)            
    {
        doComm.all = data;
    }
// HDO¿ØÖÆ
    else if (COMM_HDO_ADDR == addr)            
    {
        if (data <= 0x7FFF)
        {
            aoComm[0] = data;
        }
        else
        {
            errType = COMM_ERR_PARA;
        }
    }
// AO1¿ØÖÆ
    else if (COMM_AO1_ADDR == addr)            
    {
        if (data <= 0x7FFF)
        {
            aoComm[1] = data;
        }
        else
        {
            errType = COMM_ERR_PARA;
        }
    }
// AO2¿ØÖÆ
    else if (COMM_AO2_ADDR == addr)            
    {
        if (data <= 0x7FFF)
        {
            aoComm[2] = data;
        }
        else
        {
            errType = COMM_ERR_PARA;
        }
    }
// Ð´¹¦ÄÜÂë
    else if ((high == 0x0000)      // Fx-RAM
             || (high == 0xF000)   // Fx
             || (high == 0xA000)   // Ax
             || (high == 0x4000)   // Ax-RAM
             || (high == 0xB000)   // Bx
             || (high == 0x5000)   // Bx-RAM
             || (high == 0xC000)   // Cx
             || (high == 0x6000)   // Cx-RAM
             || ((addr & 0xFF00) == 0x1F00)          // FP£¬1Fxx
             || ((addr & 0xFF00) == 0x7300)   // U3
        ) 
    {
        errType = CommRwFuncCode(addr, data, COMM_WRITE_FC);
    }
// µØÖ·Ô½½ç
    else
    {
        errType = COMM_ERR_ADDR;
    }

    return errType;
}


//=====================================================================
//
// Í¨Ñ¶¶ÁÊý¾Ýº¯Êý
//
//=====================================================================
LOCALD Uint16 CommRead(Uint16 addr, Uint16 data)
{
    int16 i;
    Uint16 high = (addr & 0xF000);
    Uint16 low = (addr & 0x00FF);
    Uint16 errType = COMM_ERR_NONE;
	
// Í¨Ñ¶¶ÁÈ¡Í£»ú»òÔËÐÐÏÔÊ¾²ÎÊý
// ¿ÉÒÔÁ¬Ðø¶ÁÈ¡¶à¸ö²ÎÊý
    if ((addr & 0xFF00) == 0x1000)      // Í£»ú/ÔËÐÐ²ÎÊý
    {
        if (low + data > COMM_PARA_NUM)
        {
            errType = COMM_ERR_ADDR;
        }
        else
        {
            for (i = 0; i < data; i++)
            {
                commReadData[i] = funcCode.group.u0[commDispIndex[i + low]];
                
                // ÎªÍ¨Ñ¶¶ÁÈ¡µçÁ÷
                if ((i + low) == DISP_OUT_CURRENT)
                {
                    // Í¨Ñ¶¶ÁÈ¡µçÁ÷·Ö±æÂÊÎª0.1A
                    if (funcCode.code.commReadCurrentPoint)
                    {
                        commReadData[i] = commReadData[i] / 10;
                    }
                }
            }
        }
    }
// ¶ÁÈ¡±äÆµÆ÷ÔËÐÐ×´Ì¬
    else if (COMM_STATUS_ADDR == addr)         
    {
        if (data > 0x01)
        {
            errType = COMM_ERR_PARA;
        }
        else if (runFlag.bit.run)
        {
            if (FORWARD_DIR == runFlag.bit.dir) // F0-12Ö®Ç°µÄ·½Ïò
            {
                commReadData[0] = 0x0001;
            }
            else
            {
                commReadData[0] = 0x0002;
            }
        }
        else
        {
            commReadData[0] = 0x0003;
        }
    }
// ¶ÁÈ¡¹ÊÕÏ
    else if (COMM_INV_ERROR == addr)     
    {
        if (data > 0x01)
        {
            errType = COMM_ERR_PARA;
        }
        else
        {
            commReadData[0] = errorCode;
        }
    }
// ¶ÁÈ¡°´¼ü²âÊÔÖµ    
    else if (COMM_KEYBORD_TEST == addr)    
    {
        if (data > 0x01)
        {
            errType = COMM_ERR_PARA;
        }
        else
        {
            if (keyBordValue == 0x01FF)
            {
                commReadData[0] = 1;
            }
            else
            {
                commReadData[0] = 0;
            }
        }
    }
// ¶ÁÈ¡¹¦ÄÜÂë
    else if ((high == 0xF000) ||     // Fx, ¶ÁÈ¡¹¦ÄÜÂëÖµ
             (high == 0xA000) ||     // Ax
             (high == 0xB000) ||     // Bx
             (high == 0xC000) ||     // Cx
             (high == 0x7000) ||     // U0
             ((addr & 0xFF00) == 0x1F00)    // FP
        )
    {
        errType = CommRwFuncCode(addr, data, COMM_READ_FC);
    }
// µØÖ·Ô½½ç
    else    
    {
        errType = COMM_ERR_PARA;
    }

    return errType;
}


// Í¨Ñ¶¶ÁÐ´¹¦ÄÜÂë
LOCALD Uint16 CommRwFuncCode(Uint16 addr, Uint16 data, Uint16 rwMode)
{
    Uint16 index;
    Uint16 group;
    Uint16 grade;
    Uint16 gradeAdd;
    int16 i;
    Uint16 tmp;
    Uint16 highH;
    Uint16 funcCodeGradeComm[FUNCCODE_GROUP_NUM];
    Uint16 errType = COMM_ERR_NONE;

    highH = (addr & 0xF000);

    
// »ñÈ¡group, grade
    group = (addr >> 8) & 0x0F;
    grade = addr & 0xFF;

#if 0
    if ((0x0000 == highH) || (0xF000 == highH))         // Fx
    {
        ;
    }
    else
#endif        
    if ((0xA000 == highH) || (0x4000 == highH))       // Ax
    {
        group += FUNCCODE_GROUP_A0;
    }
    else if ((0xB000 == highH) || (0x5000 == highH))  // Bx
    {
        group += FUNCCODE_GROUP_B0;
    }
    else if ((0xC000 == highH) || (0x6000 == highH))  // Cx
    {
        group += FUNCCODE_GROUP_C0;
    }
    else if ((addr & 0xFF00) == 0x1F00)  // FP
    {
        group = FUNCCODE_GROUP_FP;
    }
    else if (0x7000 == highH)            // U0
    {
        group += FUNCCODE_GROUP_U0;
    }
    

// ¸üÐÂÍ¨Ñ¶Çé¿öÏÂ£¬¶ÔÃ¿Ò»group£¬ÓÃ»§¿ÉÒÔ²Ù×÷µÄgrade¸öÊý
    UpdataFuncCodeGrade(funcCodeGradeComm);
        
// ÅÐ¶Ïgroup, gradeÊÇ·ñºÏÀí
    if (COMM_READ_FC == rwMode)     // Í¨Ñ¶¶Á¹¦ÄÜÂë
    {
        gradeAdd = data - 1;
    }
    else        // Í¨Ñ¶Ð´¹¦ÄÜÂë
    {
        gradeAdd = 0;
    }

    if (grade + gradeAdd >= funcCodeGradeComm[group]) // ³¬¹ý½çÏÞ
    {
        errType = COMM_ERR_ADDR;
        return errType;
    }

    index = GetGradeIndex(group, grade);    // ¼ÆËã¹¦ÄÜÂëÐòºÅ
    
    if ((COMM_NO_RW_FC_0 == index) ||       // Ä³Ð©¹¦ÄÜÂë£¬Í¨Ñ¶²»ÄÜ½øÐÐRW
        //(COMM_NO_RW_FC_1 == index) ||
        ((COMM_WRITE_FC == rwMode) &&       // Ä³Ð©¹¦ÄÜÂë£¬Í¨Ñ¶²»ÄÜ½øÐÐW
         ((COMM_NO_W_FC_0 == index) ||
          (COMM_NO_W_FC_1 == index) ||
          (COMM_NO_W_FC_2 == index) ||
          (COMM_NO_W_FC_3 == index) ||
          (COMM_NO_W_FC_4 == index) ||
          (COMM_NO_W_FC_5 == index)       
          )
         ) ||
        ((COMM_READ_FC == rwMode) &&
         ((COMM_NO_R_FC_0 == index)         // Ä³Ð©¹¦ÄÜÂë£¬Í¨Ñ¶²»ÄÜ½øÐÐR
          )
         ) 
        )
    {
        errType = COMM_ERR_ADDR;            // ÎÞÐ§µØÖ·
        return errType;
    }

    // ÃÜÂëÐ£Ñé
    if (COMM_WRITE_FC == rwMode) // Ð´
    {
        // U3×é
        if (group == FUNCCODE_GROUP_U3)
        {
            // Ð´ÈëU3 PLC¿¨¼àÊÓ²ÎÊý×éÐÅÏ¢
            funcCode.all[index] = data;
            return errType;
        }
        else if (GetCodeIndex(funcCode.code.factoryPassword) == index) // FF-00/0F-00, ³§¼ÒÃÜÂë
        {
            if (COMPANY_PASSWORD == data)   // ³§¼ÒÃÜÂëÕýÈ·
            {
                companyPwdPass4Comm = 1;
            }
            else
            {
                errType = COMM_ERR_PARA;        // ÎÞÐ§²ÎÊý
            }
            
            return errType;
        }
        else if (GetCodeIndex(funcCode.code.userPassword) == index) // ÓÃ»§ÃÜÂë
        {
            if (data == funcCode.code.userPassword)
            {
                sciFlag.bit.pwdPass = 1;
                userPwdPass4Comm = 1;
            }
            else
            {
                errType = COMM_ERR_PWD;
            }
            
            return errType;
        }

    }

    if ((group == FC_GROUP_FACTORY) && (!companyPwdPass4Comm)) // FF×é
    {
        errType = COMM_ERR_SYSTEM_LOCKED;   // ÏµÍ³(³§¼Ò¹¦ÄÜÂë)Ëø¶¨     
        return errType;
    }

    if ((group == FUNCCODE_GROUP_AF) && (!groupHidePassComm))  // AF×é
    {
        errType = COMM_ERR_SYSTEM_LOCKED;   // ÏµÍ³(³§¼Ò¹¦ÄÜÂë)Ëø¶¨     
        return errType;
    }

    if (COMM_READ_FC == rwMode)     // Í¨Ñ¶¶Á¹¦ÄÜÂë
    {
        for (i = 0; i < data; i++, index++)
        {
            commReadData[i] = funcCode.all[index];  // U0×éÒ²¿ÉÒÔ

            // ÎªÍ¨Ñ¶¶ÁÈ¡µçÁ÷
            if (COMM_READ_CURRENT_FC == index)
            {
                // Í¨Ñ¶¶ÁÈ¡µçÁ÷·Ö±æÂÊÎª0.1A
                if (funcCode.code.commReadCurrentPoint)
                {
                    commReadData[i] = commReadData[i] / 10;
                }
            }
        }
        return errType;     // ºóÃæ¶¼ÊÇÍ¨Ñ¶Ð´¹¦ÄÜÂëµÄ´¦Àí
    }

    tmp = ModifyFunccodeUpDown(index, &data, 0);
    if (COMM_ERR_PARA == tmp)
    {
        errType = COMM_ERR_PARA;
    }
    else if (COMM_ERR_READ_ONLY == tmp)
    {
        errType = COMM_ERR_READ_ONLY;
    }
    else
    {
        if (GetCodeIndex(funcCode.code.paraInitMode) == index) // ¹¦ÄÜÂë³õÊ¼»¯
        {
            if (!userPwdPass4Comm)
            {
                errType = COMM_ERR_SYSTEM_LOCKED;
            }
            else if (FUNCCODE_RW_MODE_NO_OPERATION != funcCodeRwMode) // ÕýÔÚ²Ù×÷¹¦ÄÜÂë
            {
                errType = COMM_ERR_SAVE_FUNCCODE_BUSY;
            }
            else if (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA == data) // »Ö¸´³ö³§²ÎÊý(²»°üº¬µç»ú²ÎÊý)
            {
                funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA;
            }
            else if (FUNCCODE_paraInitMode_SAVE_USER_PARA == data)    // ±£´æÓÃ»§²ÎÊý
            {
                funcCodeRwModeTmp = FUNCCODE_paraInitMode_SAVE_USER_PARA;
            }
            else if (FUNCCODE_paraInitMode_RESTORE_USER_PARA == data) // »Ö¸´ÓÃ»§±£´æµÄ²ÎÊý
            {

                if ((funcCode.code.saveUserParaFlag1 == USER_PARA_SAVE_FLAG1)
                    && (funcCode.code.saveUserParaFlag2 == USER_PARA_SAVE_FLAG2))
                {
                    funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_USER_PARA;
                }
                else
                {
                    errType = COMM_ERR_PARA;
                }
            }
            else if (FUNCCODE_paraInitMode_CLEAR_RECORD == data) // Çå³ý¼ÇÂ¼ÐÅÏ¢
            {
                ClearRecordDeal();
            }
            
            else if (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL == data) // »Ö¸´³ö³§²ÎÊý(°üº¬µç»ú²ÎÊý)
            {
                //funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL;
            }
            return errType;
        }
        
        if (COMM_ERR_PARA == ModifyFunccodeEnter(index, data))
        {
            errType = COMM_ERR_PARA;
        }
        else
        {
            if ((highH >= 0xA000)                                            // Îª±£´æEEPROMµØÖ·
                && (commRcvData.commCmdSaveEeprom == SCI_WRITE_WITH_EEPROM)      // ÓÐ±£´æEEPROMÃüÁî
			)
            {
                if (FUNCCODE_RW_MODE_NO_OPERATION == funcCodeRwMode)
                {
                    SaveOneFuncCode(index);
                }
                else                            // ÕýÔÚ²Ù×÷EEPROM£¬²»ÏìÓ¦
                {
                    errType = COMM_ERR_SAVE_FUNCCODE_BUSY;
                }
            }
            
        }
    }
    
    return errType;
}


//=====================================================================
//
// Í¨Ñ¶ÖÐ¶Ï·¢ËÍ´¥·¢º¯Êý
//
//=====================================================================
LOCALF void inline CommStartSend(void)
{
#if DSP_2803X
    LinaRegs.SCITD = sendFrame[0];     // ·¢ËÍµÚÒ»¸öÊý¾Ý
#else
    SciaRegs.SCITXBUF = sendFrame[0];     // ·¢ËÍµÚÒ»¸öÊý¾Ý
#endif
}


//=====================================================================
//
// Í¨Ñ¶Êý¾Ý½ÓÊÕÅäÖÃ
//
//=====================================================================
#if DSP_2803X
void resetLinSci(void)
{
    LinaRegs.SCIGCR0.bit.RESET = 0;
    LinaRegs.SCIGCR0.bit.RESET = 1;
    LinaRegs.SCIGCR1.bit.SWnRST = 0; 
}


void closeRTX(void)
{
    LinaRegs.SCIGCR1.bit.RXENA = 0;
    LinaRegs.SCIGCR1.bit.TXENA = 0;
}


void setRxConfig(void)
{
    LinaRegs.SCIGCR1.bit.RXENA = 1;
    LinaRegs.SCIGCR1.bit.TXENA = 0;
}


void setTxConfig(void)
{ 
    LinaRegs.SCIGCR1.bit.TXENA = 1;
    LinaRegs.SCIGCR1.bit.RXENA = 0;
}
#endif


//====================================================================
//
// Í¨Ñ¶³ö´í´¦Àí
//
//===================================================================
void commErrorDeal(void)
{
    errorOther = ERROR_COMM;
    errorInfo = COMM_ERROR_MODBUS;
    commStatus = SCI_RECEIVE_DATA;  // ÖÃÎª½ÓÊÕ×´Ì¬
    RTS = RS485_R;                  // RTSÖÃÎª½ÓÊÕ

    #if DSP_2803X
    EALLOW;        
    setRxConfig();
    EDIS;
    #else
    SciaRegs.SCICTL1.all = 0x0021;  // ½ÓÊÕ
    SciaRegs.SCICTL2.all = 0x00C2;  // ¿ªÆô½ÓÊÕÖÐ¶Ï
    #endif
}


//====================================================================
//
// Í¨Ñ¶×´Ì¬´¦Àí
//
//===================================================================
void commStatusDeal(void)
{
    switch (commStatus)
    {
        case SCI_RECEIVE_OK:
            CommRcvDataDeal();

            if (commRcvData.slaveAddr)              // ·Ç¹ã²¥²Å·µ»Ø
            {
                CommSendDataDeal();                 // ·¢ËÍÊý¾Ý×¼±¸
                commStatus = SCI_SEND_DATA_PREPARE; // ½ÓÊÕ´¦ÀíÍê³É£¬×¼±¸·¢ËÍ
            }
            else                                    // ¹ã²¥£¬DSPÏìÓ¦Ö®ºó²»·¢ËÍ£¬¼ÌÐø½ÓÊÕ
            {
                commStatus = SCI_RECEIVE_DATA;
                break;
            }
            
        case SCI_SEND_DATA_PREPARE:
            if ((commTicker >= commRcvData.delay)               // Ó¦´ðÑÓ³Ù
                && (commTicker > commRcvData.frameSpaceTime))   // MODBUSÎª3.5¸ö×Ö·ûÊ±¼ä
            {
                RTS = RS485_T;                          // RTSÖÃÎª·¢ËÍ
                #if DSP_2803X
                EALLOW;            
                setTxConfig();
                EDIS;
                #else                    
                SciaRegs.SCICTL1.all = 0x0022;          // ·¢ËÍ
                SciaRegs.SCICTL2.all = 0x00C1;          // ¿ªÆô·¢ËÍÖÐ¶Ï
                #endif
                commStatus = SCI_SEND_DATA;
                commSendData.sendNum = 1;               // µ±Ç°·¢ËÍÊý¾Ý¸öÊýÖÃÎª1
                CommStartSend();                        // ¿ªÊ¼·¢ËÍ
            }
            break;

        case SCI_SEND_OK:
            #if DSP_2803X
			if (LinaRegs.SCIFLR.bit.TXEMPTY)
            #else
            if (SciaRegs.SCICTL2.bit.TXEMPTY)   // Transmitter empty flag, ÕæÕý·¢ËÍÍê±Ï
            #endif
            {
                commStatus = SCI_RECEIVE_DATA;  // ·¢ËÍÍê±Ïºó£¬ÖÃÎª½ÓÊÕ×´Ì¬
                RTS = RS485_R;                  // RTSÖÃÎª½ÓÊÕ
                #if DSP_2803X
                EALLOW;
                setRxConfig();
                EDIS;
                #else
                SciaRegs.SCICTL1.all = 0x0021;  // ½ÓÊÕ
                SciaRegs.SCICTL2.all = 0x00C2;  // ¿ªÆô½ÓÊÕÖÐ¶Ï
                #endif
            }
            break;

        default:
            break;
    }
}


//====================================================================
//
// Í¨Ñ¶Êý¾Ý½ÓÊÕ´¦Àí
//
//===================================================================
void CommDataReRcv(Uint16 tmp)
{
    if (commRcvData.rcvNum< commRcvData.rcvNumMax)  // ½ÓÊÕÒ»Ö¡Êý¾Ý»¹Ã»ÓÐÍê³É
    {
        rcvFrame[commRcvData.rcvNum++] = tmp;
    }

    if (commRcvData.rcvNum >= commRcvData.rcvNumMax) // ½ÓÊÕÒ»Ö¡Êý¾ÝÍê³É
    {
        // PROFIBUSÖÃÖ¡Í·ÅÐ¶Ï±êÖ¾
        commRcvData.rcvDataJuageFlag = 0;
        
        if (2 == commRcvData.rcvFlag)                // ±¾»úµØÖ·²Å·µ»Ø(DSPÇÐ»»³É·¢ËÍ×´Ì¬)
        {
            #if DSP_2803X
            EALLOW;
            closeRTX();                  // ¹Ø±Õ·¢ËÍ½ÓÊÕ
            EDIS;
            #else
            SciaRegs.SCICTL1.all = 0x0004;      // ¹Ø±Õ·¢ËÍ½ÓÊÕ£¬sleep
            SciaRegs.SCICTL2.all = 0x00C0;      // ¹Ø±Õ½ÓÊÕ·¢ËÍÖÐ¶Ï
            #endif
        }
        
        commStatus = SCI_RECEIVE_OK;            // ±êÖ¾½ÓÊÕÊý¾ÝÍê³É
        commRcvData.rcvFlag = 0;
    }
}


//====================================================================
//
// Í¨Ñ¶Êý¾Ý·¢ËÍ´¦Àí
//
//===================================================================
void CommDataSend(void)
{
	 // ·¢ËÍÒ»Ö¡Êý¾ÝÃ»ÓÐÍê³É
    if (commSendData.sendNum< commSendData.sendNumMax)          
    {
#if DSP_2803X
        LinaRegs.SCITD = sendFrame[commSendData.sendNum++];
#else
        SciaRegs.SCITXBUF = sendFrame[commSendData.sendNum++];
#endif
    }
	// ·¢ËÍÒ»Ö¡Êý¾ÝÈ«²¿Íê³É
    else if (commSendData.sendNum >= commSendData.sendNumMax)     
    {
		// ±êÖ¾·¢ËÍÊý¾ÝÍê³É
        commStatus = SCI_SEND_OK;               
    }
}


//=====================================================================
//
// Í¨Ñ¶³õÊ¼»¯
//
//=====================================================================
void InitSetScia(void)
{
    // Ó¦¸Ã·ÅÔÚÇ°Ãæ
    commStatus = SCI_RECEIVE_DATA;              // ½ÓÊÕ×´Ì¬   
#if DSP_2803X
EALLOW;
    // reset
	resetLinSci();

	LinaRegs.SCIGCR1.bit.SLEEP = 0;
    LinaRegs.SCIFLR.bit.TXWAKE = 0;  
    LinaRegs.SCIFLR.bit.TXEMPTY = 1;
    LinaRegs.SCIFLR.bit.TXRDY = 1;
	// ÅäÖÃÎªÊý¾Ý½ÓÊÕ
    setRxConfig(); 

	LinaRegs.SCIGCR1.bit.TIMINGMODE = 1; //Asynchronous Timing
	LinaRegs.SCIGCR1.bit.CLK_MASTER = 1; //Enable SCI Clock
	LinaRegs.SCIGCR1.bit.CONT = 1;		 //Continue on Suspend
	if (funcCode.code.commProtocolSec == CAN_OPEN)          // ÎÞÐ£Ñé(8-N-2)
        LinaRegs.SCIGCR1.all = (LinaRegs.SCIGCR1.all&0xFFFFFFE3)^commParitys[0];
    else
        LinaRegs.SCIGCR1.all = (LinaRegs.SCIGCR1.all&0xFFFFFFE3)^commParitys[funcCode.code.commParity];
    LinaRegs.SCISETINT.bit.SETRXINT = 1;
    LinaRegs.SCISETINT.bit.SETTXINT = 1;
    LinaRegs.SCIFORMAT.bit.CHAR = 0x7;
    LinaRegs.SCIGCR1.bit.SWnRST = 1; 
       
EDIS;
#else
    SciaRegs.SCICTL1.all = 0x0001;              // SCIÈí¼þ¸´Î»£¬µÍÓÐÐ§
    SciaRegs.SCICTL2.all = 0x00C2;
    SciaRegs.SCICCR.all = 0x0087;               // 2 stop bit, No loopback, No parity,8 bits,async mode,idle-line
    SciaRegs.SCIPRI.bit.FREE = 1;
    SciaRegs.SCICTL1.all = 0x0021;              // Relinquish SCI from Reset
#endif
}


//=====================================================================
//
// SCIÍ¨Ñ¶²ÎÊýÐÞ¸Äº¯Êý
//
//=====================================================================
void UpdateSciFormat(void)
{
    Uint16 digit[5];
    Uint16 tmp;
    
	// »ñµÃÍ¨Ñ¶²¨ÌØÂÊ
	if (funcCode.code.commProtocolSec == CAN_OPEN)
        GetNumberDigit1(digit, 8);                          // Ê¹ÓÃCAN_OPEN½Ó¿Ú,57600bps²¨ÌØÂÊ
    else
        GetNumberDigit1(digit, funcCode.code.commBaudRate);
    
    // ÎªPROFIBUS
    if (funcCode.code.commProtocolSec == PROFIBUS)          // CANOPENÓëModbus¼æÈÝ
    {
        commType = PROFIBUS;
		tmp = digit[commType] + 9;
    }
    // ÎªMODBUS
    else 
    {
        commType = MODBUS;
		tmp = digit[commType];
    }
    
    // »ñµÃÍ¨Ñ¶Êý¾Ý´«ËÍ¸ñÊ½
    if (funcCode.code.commProtocolSec == CAN_OPEN)
        GetNumberDigit1(digit, 0);                          // ¸öÎ»£¬±ê×¼ModbusÐ­Òé
    else        
        GetNumberDigit1(digit, funcCode.code.commProtocol);

    commProtocol = digit[commType];
    
	// ¸üÐÂÍ¨Ñ¶ÅäÖÃÎÄ¼þ
    protocolFunc[commType].UpdateCommFormat(dspBaudRegData[tmp].baudRate);
    
 // ³öÏÖ½ÓÊÕ¹ÊÕÏÊ±´¦Àí    
#if DSP_2803X
	if ( LinaRegs.SCIFLR.bit.BRKDT
        || LinaRegs.SCIFLR.bit.PE 
        || LinaRegs.SCIFLR.bit.OE
        || LinaRegs.SCIFLR.bit.FE)
#else
    if (SciaRegs.SCIRXST.bit.RXERROR)      
#endif
    {
        InitSetScia();   // ³õÊ¼»¯SCI¼Ä´æÆ÷
    }
#if DSP_2803X

EALLOW;
    if (funcCode.code.commProtocolSec == CAN_OPEN)          // ÎÞÐ£Ñé(8-N-2)
        LinaRegs.SCIGCR1.all = (LinaRegs.SCIGCR1.all&0xFFFFFFE3)^commParitys[0];
    else
        LinaRegs.SCIGCR1.all = (LinaRegs.SCIGCR1.all&0xFFFFFFE3)^commParitys[funcCode.code.commParity];
    
    LinaRegs.BRSR.bit.SCI_LIN_PSH = dspBaudRegData[tmp].high;
    LinaRegs.BRSR.bit.SCI_LIN_PSL = dspBaudRegData[tmp].low;
    LinaRegs.BRSR.bit.M = dspBaudRegData[tmp].M;
EDIS;
#else
    if (funcCode.code.commProtocolSec == CAN_OPEN)
        SciaRegs.SCICCR.all = commParitys[0];               // ÎÞÐ£Ñé(8-N-2)
    else
        SciaRegs.SCICCR.all = commParitys[funcCode.code.commParity];
    
    SciaRegs.SCIHBAUD = dspBaudRegData[tmp].high;
    SciaRegs.SCILBAUD = dspBaudRegData[tmp].low;
    SciaRegs.SCICTL1.bit.SWRESET = 1;
#endif

}


#if 1//F_DEBUG_RAM    // ½öµ÷ÊÔ¹¦ÄÜ£¬ÔÚCCSµÄbuild optionÖÐ¶¨ÒåµÄºê
void InitSciaGpio(void)
{
#if DSP_2803X  // 2803xÆ½Ì¨
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO14 = 0;		// Enable pull-up for GPIO14 (LIN TX)
    GpioCtrlRegs.GPAPUD.bit.GPIO15 = 0;		// Enable pull-up for GPIO15 (LIN RX)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 3;  // Asynch input GPIO15 (LINRXA)
    GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 2;   // Configure GPIO14 for LIN TX operation (2-Enable,0-Disable)
    GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 2;   // Configure GPIO15 for LIN RX operation (2-Enable,0-Disable)
    GpioCtrlRegs.GPBPUD.bit.GPIO39 = 0;    
    GpioCtrlRegs.GPBMUX1.bit.GPIO39 = 0;        // Configure GPIO39, RTS
    GpioCtrlRegs.GPBDIR.bit.GPIO39 = 1;         // output
    GpioDataRegs.GPBDAT.bit.GPIO39 = RS485_R;   // Receive
    EDIS;
// Í¨Ñ¶¿ØÖÆÊ¹ÓÃÖÐ¶Ï£¬³õÊ¼»¯
    EALLOW;
    PieVectTable.LIN0INTA = &Lina_Level0_ISR;
    PieVectTable.LIN1INTA = &Lina_Level1_ISR;
    EDIS;
	IER |= M_INT9;   	                 // Enable interrupts:
	PieCtrlRegs.PIEIER9.bit.INTx3=1;     // PIE Group 9, INT3
	PieCtrlRegs.PIEIER9.bit.INTx4=1;     // PIE Group 9, INT4	
#else
    EALLOW;    
    GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    // Enable pull-up for GPIO28 (SCIRXDA)
    GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;    // Enable pull-up for GPIO29 (SCITXDA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  // Asynch input GPIO28 (SCIRXDA)
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   // Configure GPIO28 for SCIRXDA operation
    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   // Configure GPIO29 for SCITXDA operation
    GpioCtrlRegs.GPAPUD.bit.GPIO27 = 0;    
    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;        // Configure GPIO27, RTS
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;         // output
    GpioDataRegs.GPADAT.bit.GPIO27 = RS485_R;   // Receive
    EDIS;
    // Í¨Ñ¶¿ØÖÆÊ¹ÓÃÖÐ¶Ï£¬³õÊ¼»¯
    EALLOW;
    PieVectTable.SCIRXINTA = SCI_RXD_isr;
	PieVectTable.SCITXINTA = SCI_TXD_isr;
    EDIS;
	IER |= M_INT9;   	            //  Enable interrupts:
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;
	PieCtrlRegs.PIEIER9.bit.INTx2 = 1;
 #endif   
	
}
#endif  // #if F_DEBUG_RAM

#else   // #if DEBUG_F_MODBUS
void InitSetScia(void){}
void InitSciaGpio(void){}
void SciDeal(void){}
void UpdateSciFormat(void){}
interrupt void SCI_RXD_isr(void){}
interrupt void SCI_TXD_isr(void){}
Uint16 CommWrite(Uint16 addr, Uint16 data){}
Uint16 CommRead(Uint16 addr, Uint16 data){}
#endif











