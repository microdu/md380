/******************** (C) COPYRIGHT 2012 dn********************
* File Name          : f_comm_DP.c
* Version            : 
* Date               : 
* Description        : profibus_DP function SCI
********************************************************************************/


#include "f_comm_DP.h"
#include "f_main.h"

#define DEBUG_F_DP             1


#if DEBUG_F_DP

#define SCI_M380_DP_DATA_DEFAULTS   \
{                                   \
    &SciaRegs                       \
}

struct SCI_DATA_DP sciM380DpData = SCI_M380_DP_DATA_DEFAULTS;

Uint16 sendData_DP[SEND_DATA_NUMBER];                        // 发送给STM32数据
Uint16 rcvData_DP[RCV_DATA_NUMBER];                          // 接收STM32的数据
enum DP_SCI_COMM_RCV_FLAG dpSciCommRcvFlag;
enum DP_SCI_COMM_SEND_FLAG dpSciCommSendFlag;

DP_PARAMETER dpParameter = {3,5};

Uint16 dataByteNum; // 不同模式下数据传输的字节个数,PPO1-12,PPO2-20,PPO3-4,PPO5-32

#define FIFO_NUM  16;

void InitSciDpBaudRate(struct SCI_DATA_DP *p); 
interrupt void SCI_DP_RXD_isr(void);
interrupt void SCI_DP_TXD_isr(void);


void DpDataDeal(void);
void ErrAgainSend(struct SCI_DATA_DP *p);

void UpdateSciDpFormat(struct SCI_DATA_DP *p);
void GetDataByteNum(void);
 

//=====================================================================
//
// 通讯接收中断函数
//
//=====================================================================
interrupt void SCI_DP_RXD_isr(void)
{
    Uint16 tmp;
    struct SCI_DATA_DP *p = &sciM380DpData;  
    tmp = p->pSciRegs->SCIRXBUF.all; 
    
    if(p->rcvDataJuageFlag == 0)  // 帧头判断标志
    {
#if BUG_SCI_BACK_DATA
        // 测试数据格式
        if(tmp == 0x5A)
        {
            p->frameFlagDp  = 1;      // 不同数据帧                  
            p->rcvDataJuageFlag = 1;
        }
        else 
#endif
        if(tmp == 0xAA)
        {
            p->rcvRigthFlag = 1;      // 接收帧头第一数据正确
            p->rcvData_SCI[0] = tmp;
              
        }
        else if(p->rcvRigthFlag == 1) // 接收帧头第一数据正确
        {   
            p->rcvRigthFlag = 0;      // 清接收帧头第一数据正确
            if(tmp == 0x55)
            {
                p->commDpRcvNumber = 1; // 接收数据计数
                p->frameFlagDp  = 2;    // 不同数据帧                    
                p->rcvDataJuageFlag = 1;
            }              
        }
        else
        {
            p->rcvCrcErrCounter  = 0;
            p->commDpRcvNumber  = 0;   // 接受数据计数
            p->commDpSendNum  = 0;     // 发送的数据计数
            p->commDpSendNumMax  = 0;  // 每次发送的数据个数
            p->rcvDataJuageFlag = 0;       
            p->frameFlagDp = 0;
            p->sciRcvFlag = SCI_RCV_NO;
            UpdateSciDpFormat(&sciM380DpData); 
        }

     }
#if BUG_SCI_BACK_DATA    
    // 测试数据
    if(p->frameFlagDp == 1)
    {
        // 接收一帧数据还没有完成
        if (p->commDpRcvNumber < SCI_SEND_READ_NUMBER)  
        {
            p->rcvData_SCI[p->commDpRcvNumber++] = tmp;
            if(p->commDpRcvNumber >= SCI_SEND_READ_NUMBER)
            {
                p->rcvDataJuageFlag = 0;     // 数据接收完后置0
                p->commDpRcvNumber = 0;      // 接收数据序号置0
                p->sciRcvFlag = SCI_RCV_YES; // 数据接收OK
                p->commDpSendNumMax = SCI_SEND_READ_NUMBER;  // 置发送数据个数
            }
        }                 
    }
    // DP
    else 
#endif
    if(p->frameFlagDp == 2)
    {
       if (p->commDpRcvNumber < (dataByteNum + 4))  // 接收一帧数据还没有完成
       {
           p->rcvData_SCI[p->commDpRcvNumber++] = tmp;
           if(p->commDpRcvNumber >= (dataByteNum + 4))
           {
               p->rcvDataJuageFlag = 0;
               p->commDpRcvNumber = 0;
               p->sciRcvFlag = SCI_RCV_YES;
               p->commDpSendNumMax = (dataByteNum + 4);
           }
       }   
    } 
    PieCtrlRegs.PIEACK.bit.ACK9 = 1;                // Issue PIE ACK
}


//=====================================================================
//
// 通讯发送中断函数
//
// 发送一个字符完成，就进入该中断
//
//=====================================================================
interrupt void SCI_DP_TXD_isr(void)
{    
    struct SCI_DATA_DP *p = &sciM380DpData;
    
#if NO_FIFO    

    if (p->commDpSendNum < p->commDpSendNumMax)           // 发送一帧数据没有完成
    {
         p->pSciRegs->SCITXBUF = p->sendData_SCI[p->commDpSendNum++];
    }                       
    
#else

    Uint16 i;
    if (p->commDpSendNumMax < FIFO_NUM)
    {
        for (i = 0; i < p->commDpSendNumMax; i++)
        {
            p->pSciRegs->SCITXBUF = p->sendData_SCI[i];    
        }
    }
    else
    {
        for (i = 0; i < FIFO_NUM; i++)
        {
            p->pSciRegs->SCITXBUF = p->sendData_SCI[i];    
        }
    }
#endif
    PieCtrlRegs.PIEACK.bit.ACK9 = 1;            // Issue PIE ACK
}


//=====================================================================
//
// 通讯中断发送触发函数
//
//=====================================================================
void inline CommDpStartSend(struct SCI_DATA_DP *p)
{
    p->pSciRegs->SCITXBUF = p->sendData_SCI[0];     // 发送第一个数据
    p->commDpSendNum = 1;                           //给中断发送剩余的数据
}


//=====================================================================
//
// 通讯接收的数据处理函数
//
//=====================================================================
void CommDpRcvDataDeal(struct SCI_DATA_DP *p)
{  
    Uint16 i;
    Uint16 crcValue;
    if (p->sciRcvFlag == SCI_RCV_YES)
    {
        p->sciRcvFlag = SCI_RCV_NO;

#if BUG_SCI_BACK_DATA        
        if (p->frameFlagDp == 1)   // DP卡读配置参数 包括地址和通讯模式
        {                
            crcValue = (p->rcvData_SCI[SCI_SEND_READ_NUMBER - 1] << 8) + p->rcvData_SCI[SCI_SEND_READ_NUMBER - 2]; //高位在后 低位在前
            if (crcValue == CrcValueByteCalc(p->rcvData_SCI,(SCI_SEND_READ_NUMBER-2)))
            {
                // 发送数据处理
                p->frameSendStart.bit.frameType = DSP_TO_DP_PARAMETER;
                p->sendData_SCI[0] = 0x5A;  // 回复读参数的桢头
                p->sendData_SCI[1] = dpParameter.dpAddress;
                p->sendData_SCI[2] = dpParameter.dpDataFormat;
                crcValue = CrcValueByteCalc(p->sendData_SCI,(SCI_SEND_READ_NUMBER-2));
                p->sendData_SCI[SCI_SEND_READ_NUMBER-1] = (crcValue >> 8)&0x00ff;
                p->sendData_SCI[SCI_SEND_READ_NUMBER-2] = (crcValue&0x00ff); 
                CommDpStartSend(&sciM380DpData);                     
            }
            else
            {
                p->rcvCrcErrCounter ++; //校验出错
            }
            
            p->frameFlagDp = 0;  //清状态标志
            
        }
        else 
#endif
        if (p->frameFlagDp == 2) //正常通讯状态 
        {
            crcValue = (p->rcvData_SCI[dataByteNum + 3] << 8) + p->rcvData_SCI[dataByteNum +2];
            if(crcValue == CrcValueByteCalc(p->rcvData_SCI,(dataByteNum + 2)))
            {
                for( i = 0; i < dataByteNum; i++)
                {
                    rcvData_DP[i] = p->rcvData_SCI[i+2];
                    #if BUG_SCI_BACK_DATA
                    sendData_DP[i] = rcvData_DP[i];                             
                    #endif
                }    
                #if BUG_SCI_BACK_DATA
                funcCode.code.dpDataBuffer[1] = (((rcvData_DP[0] << 8)&0xff00) + rcvData_DP[1]);
                funcCode.code.dpDataBuffer[2] = (((rcvData_DP[2] << 8)&0xff00) + rcvData_DP[3]);
                funcCode.code.dpDataBuffer[3] = (((rcvData_DP[4] << 8)&0xff00) + rcvData_DP[5]);
                #endif
                dpSciCommRcvFlag = DP_SCI_COMM_RCV_YES;
            }
            else
            {
                p->rcvCrcErrCounter++;
            }
    
            p->frameFlagDp = 0;  //清状态标志
    
        }

    }
    
    if(p->rcvCrcErrCounter > RCV_CRC_ERR_NUMBER)
    {
        p->rcvCrcErrCounter  = 0;
        p->commDpRcvNumber  = 0;   // 接受数据计数
        p->commDpSendNum  = 0;     //发送的数据计数
        p->commDpSendNumMax  = 0;  //每次发送的数据个数
        p->rcvDataJuageFlag = 0;    
        p->frameFlagDp = 0;
        p->sciRcvFlag = SCI_RCV_NO;
        UpdateSciDpFormat(&sciM380DpData);  // 重启串口
    }
}


void CommDpSendDataDeal(struct SCI_DATA_DP *p)
{
     Uint16 i;
     Uint16 crcValue;
	 if ( dpSciCommSendFlag == DP_SCI_COMM_SEND_YES )	
     {
        dpSciCommSendFlag = DP_SCI_COMM_SEND_NO;
        // 发送数据处理
        p->frameSendStart.bit.frameType = DSP_TO_DP;
        p->sendData_SCI[0] = 0xAA; 
        p->sendData_SCI[1] = 0x55; // 正常通讯的桢头

        for (i = 0; i < dataByteNum; i++)
        {
            p->sendData_SCI[i + 2] = rcvData_DP[i];
        }
        crcValue = CrcValueByteCalc(p->sendData_SCI,(dataByteNum+2));
        p->sendData_SCI[dataByteNum+3] = (crcValue >> 8)&0x00ff;
        p->sendData_SCI[dataByteNum+2] = (crcValue&0x00ff); 
        CommDpStartSend(&sciM380DpData);
    }
}


//===========================================================================
// Function Name  : sciDpDeal
// Description    : DP相关的串口数据处理
// Input          : None
// Output         : None
// Return         : None
//===========================================================================
void SciDpDeal(struct SCI_DATA_DP *p)
{
    
    //更新DP卡的参数	
    dpParameter.dpAddress = 3;     // funcCode.group.c2[1];        // Profibus DP地址
    dpParameter.dpDataFormat = 5;  // funcCode.group.c2[2];    // 数据传送格式选择

    GetDataByteNum();   //根据数据传输格式得到数据传输的字节数
    
    UpdateSciDpFormat(&sciM380DpData); 
    CommDpRcvDataDeal(&sciM380DpData);  

    // DP数据处理函数
    //DpDataDeal(); 
    #if BUG_SCI_BACK_DATA
    if( dpSciCommRcvFlag == DP_SCI_COMM_RCV_YES)
    {
        dpSciCommRcvFlag = DP_SCI_COMM_RCV_NO;      // 接收标志清零
        dpSciCommSendFlag = DP_SCI_COMM_SEND_YES;   // 置发送标志
    }
    #else
    DpDataDeal(); 
    #endif
    CommDpSendDataDeal(&sciM380DpData);

}

//===========================================================================
// Function Name  : ErrAgainSend
// Description    : CRC校验出错 数据重发
// Input          : DP相关数据输入结构体数据
// Output         : None
// Return         : None
//===========================================================================
void ErrAgainSend(struct SCI_DATA_DP *p)
{
     Uint16 crcValue;
     Uint16 i;
     
     p->frameSendStart.bit.frameType = ERR_FRAME;
     p->sendData_SCI[0] = p->frameSendStart.all;

     for(i=0; i<dataByteNum; i++)
     {
        p->sendData_SCI[i+1] = 0x55;
     }
     crcValue = CrcValueByteCalc(p->sendData_SCI,(dataByteNum+1));
     p->sendData_SCI[dataByteNum+2] = (crcValue >> 8)&0x00ff;
     p->sendData_SCI[dataByteNum+1] = (crcValue&0x00ff); 

     CommDpStartSend(&sciM380DpData); 
}

//=====================================================================
//
// 通讯初始化
//
//=====================================================================
void InitSetSciDp(struct SCI_DATA_DP *p)
{ 
#if NO_FIFO 
    p->pSciRegs->SCICTL1.all = 0x0001;   // SCI软件复位，低有效
    p->pSciRegs->SCICTL2.all = 0x00C2;   
    p->pSciRegs->SCICCR.all = 0x0087;    // 2 stop bit, No loopback, No parity,8 bits,async mode,idle-line
    p->pSciRegs->SCIPRI.bit.FREE = 1;
    p->pSciRegs->SCICTL1.all = 0x0023;   // 接收中断                     
    p->pSciRegs->SCICTL2.all = 0x00C3;   // 开启发送中断
    p->pSciRegs->SCICCR.all = 0x0087;    // 无奇偶校验时有2个停止位，Modbus协议要求    
    InitSciDpBaudRate(&sciM380DpData);   // 初始化串口波特率
#else
    p->pSciRegs->SCICTL1.all = 0x0001;   // SCI软件复位，低有效
    p->pSciRegs->SCICTL2.all = 0x0082;   
    p->pSciRegs->SCIFFTX.all = 0xE060;
    p->pSciRegs->SCICCR.all = 0x0087;    // 2 stop bit, No loopback, No parity,8 bits,async mode,idle-line
    p->pSciRegs->SCIPRI.bit.FREE = 1;
    p->pSciRegs->SCICTL1.all = 0x0023;   // 接收中断                     
    p->pSciRegs->SCICTL2.all = 0x0083;   // 开启发送中断  
    p->pSciRegs->SCICCR.all = 0x0087;         
    InitSciDpBaudRate(&sciM380DpData);    //初始化串口波特率
#endif
}


//=====================================================================
//
// sci通讯参数修改函数
//
//=====================================================================
void UpdateSciDpFormat(struct SCI_DATA_DP *p)
{   
    if (p->pSciRegs->SCIRXST.bit.RXERROR)       // 出现接收故障时处理
    {
        InitSetSciDp(&sciM380DpData);  
    }

    p->pSciRegs->SCICCR.all = 0x0087;       // 无奇偶校验时有2个停止位，Modbus协议要求    
    InitSciDpBaudRate(&sciM380DpData);      // 初始化串口波特率
}



void InitSciaGpioDp(void)
{
    EALLOW;
    
    GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    // Enable pull-up for GPIO28 (SCIRXDA)
    GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;    // Enable pull-up for GPIO29 (SCITXDA)
    
    GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  // Asynch input GPIO28 (SCIRXDA)
    
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   // Configure GPIO28 for SCIRXDA operation
    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   // Configure GPIO29 for SCITXDA operation

    EDIS;

    // 通讯控制使用中断，初始化
    EALLOW;
    PieVectTable.SCIRXINTA = SCI_DP_RXD_isr;
	PieVectTable.SCITXINTA = SCI_DP_TXD_isr;
    EDIS;
	IER |= M_INT9;   	            //  Enable interrupts:
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;
	PieCtrlRegs.PIEIER9.bit.INTx2 = 1;
}




void InitScibGpioDp(void)
{

    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    // Enable pull-up for GPIO28 (SCIRXDA)
    GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;    // Enable pull-up for GPIO29 (SCITXDA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  // Asynch input GPIO28 (SCIRXDA)
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   // Configure GPIO28 for SCIRXDA operation
    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   // Configure GPIO29 for SCITXDA operation
    EDIS;
    // 通讯控制使用中断，初始化
    EALLOW;
    PieVectTable.SCIRXINTA = SCI_DP_RXD_isr;
	PieVectTable.SCITXINTA = SCI_DP_TXD_isr;
    EDIS;
	IER |= M_INT9;   	            //  Enable interrupts:
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;
	PieCtrlRegs.PIEIER9.bit.INTx2 = 1;
}

//=====================================================================
//
// 初始化串口波特率
// 波特率=100*10^6/4/((BAUD+1)*8)
//
//=====================================================================
void InitSciDpBaudRate(struct SCI_DATA_DP *p)
{
#if 0
    switch(funcCode.group.c2[3])	//串口波特率
    {                    
         case SCI_BAUD_RATE1:             
            p->pSciRegs->SCIHBAUD = 0x0000;
            p->pSciRegs->SCILBAUD = 0x001a; //115200bps 
            break;
            
         case SCI_BAUD_RATE2:             
            p->pSciRegs->SCIHBAUD = 0x0000;
            p->pSciRegs->SCILBAUD = 0x000e; //208333bps 
            break;
            
         case SCI_BAUD_RATE3:             
            p->pSciRegs->SCIHBAUD = 0x0000;
            p->pSciRegs->SCILBAUD = 0x000b; //256000bps 
            break;
            
		 case SCI_BAUD_RATE4:             
            p->pSciRegs->SCIHBAUD = 0x0000; 
            p->pSciRegs->SCILBAUD = 0x0005; //512000bps,modefied by sjw 2009-12-09
            break;

         default: 
            break; 
    }
#elif 1
			p->pSciRegs->SCIHBAUD = 0x0000;
            p->pSciRegs->SCILBAUD = 0x000e; //208333bps 
#endif
}



//=====================================================================
//
// 根据数据传输格式获取数据传输字节个数
//
//=====================================================================
const Uint16 dataByteNums[5] = {12, 20, 0, 4, 32};
void GetDataByteNum(void)
{
    dataByteNum = dataByteNums[dpParameter.dpDataFormat - 1];
}


Uint16 dpTest = 2;
void DpDataDeal(void)
{
    // 没有接收到新的数据
    if (DP_SCI_COMM_RCV_NO == dpSciCommRcvFlag)
    {
        return;
    }
#if 0
    for (i = 0; i < RCV_DATA_NUMBER; i++)
    {
        sendData_DP[i] = rcvData_DP[i] + dpTest;
    }
#elif 0	//modefied by sjw 2009-12-24 for test
    ethDpPara.rcvFlag = 1;
    for (i = 0; i < 16; i++)
    {       
       ethDpPara.rcv[i] = (rcvData_DP[2*i+1]&0x00ff)  + (rcvData_DP[2*i]<< 8&0xff00); //PLC发送的命令 高字节在前，低字节在后
    }
    ethDpDeal();
    for (i = 0; i < ETH_PLC_RCV_LENGTH; i++)
    {    
        sendData_DP[2*i] = ((ethDpPara.reply[ i ] >> 8)&0x00ff);
        sendData_DP[2*i + 1] = (ethDpPara.reply[ i ]&0x00ff);//DSP反馈的命令 高字节在前，低字节在后
    }
#endif

    dpSciCommRcvFlag = DP_SCI_COMM_RCV_NO;      // 接收标志清零
    dpSciCommSendFlag = DP_SCI_COMM_SEND_YES;   // 置发送标志
}



#elif 1


void InitScibGpioDp(void){}
void InitSetSciDp(struct SCI_DATA_DP *p){}
void SciDpDeal(struct SCI_DATA_DP *p){}


#endif


























