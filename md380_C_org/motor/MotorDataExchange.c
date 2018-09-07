/****************************************************************
文件功能：和功能模块的数据交互程序，单CPU下使用数组拷贝，双CPU下通讯
文件版本： 
最新更新： 
	
****************************************************************/
#include "MotorInclude.h"

// // 全局变量定义
//#define     NUM_2MS_F2M     151     // 功能2ms传递给驱动
#define     NUM_2ms_FUNC_TO_MOTOR           75   //
#define     NUM_2ms_FUNC_TO_MOTOR_debug     40

#define     NUM_2ms_MOTOR_TO_FUNC           30
#define     NUM_2ms_MOTOR_TO_FUNC_debug     30
#define     NUM_05MS_F2M    14      // 功能05ms传递给驱动
#define     NUM_05MS_M2F    5       // 驱动05ms传递给功能
#define     NUM_TUNE_M2F    20      // 驱动调谐时传递给功能(2ms)

int  gSendToMotor05MsDataBuff[NUM_05MS_F2M];
int  gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + NUM_2ms_FUNC_TO_MOTOR_debug];
int  gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC + NUM_2ms_MOTOR_TO_FUNC_debug];
int  gRealTimeToFunctionDataBuff[NUM_05MS_M2F];
int  gParaIdToFunctionDataBuff[NUM_TUNE_M2F];

Uint uReservedData;              // 保留参数
Uint gSoftVersion = SOFT_VERSION;

extern MT_STRUCT_Q24           gIMTQ12_obs;
extern ALPHABETA_STRUCT		gABFluxS_LowFilter;		//αβ坐标轴下定子磁通给定值
extern ALPHABETA_STRUCT		gABFluxS_comp;		//αβ坐标轴下定子磁通给定值

/**************************************************************
	0.5Ms循环中，从功能模块获取需要的所有参数(暂时不传该组参数)
*************************************************************/
void ParSend05Ms(void)
{
    //gRealTimeToFunctionDataBuff[0]	=	gUDC.uDC	;
    //gRealTimeToFunctionDataBuff[1]	=	gLineCur.CurBaseInv	;
    //gRealTimeToFunctionDataBuff[2]	=	gIAmpTheta.anglePF	;
    //gRealTimeToFunctionDataBuff[3]	=	gVfCsr.active	;
    //gRealTimeToFunctionDataBuff[4]	=	gOutVolt.VoltApply	;
}

void ParGet05Ms(void)
{
    gMainCmd.Command.all            =	gSendToMotor05MsDataBuff[0]	;
    gExtendCmd.all                  =	gSendToMotor05MsDataBuff[1]	;
    gGetParVarable.TuneType         =	(TUNE_FLOW_ENUM)gSendToMotor05MsDataBuff[2]	;
    gMotorInfo.MotorType            =	gSendToMotor05MsDataBuff[3]	;
    gMainCmd.FreqSet                =	gSendToMotor05MsDataBuff[4]	;
    gOutVolt.vfSplit                =	gSendToMotor05MsDataBuff[5]	;
    gMotorInfo.Votage               =	gSendToMotor05MsDataBuff[6]	;
    gMotorInfo.CurrentGet           =	gSendToMotor05MsDataBuff[7]	;
    gMotorInfo.Frequency            =	gSendToMotor05MsDataBuff[8]	;
    gVFPar.VFLineType               =	gSendToMotor05MsDataBuff[9]	;

    gVFPar.ovGain                   =    gSendToMotor05MsDataBuff[10]	;
    gVFPar.ovPoint                  =    gSendToMotor05MsDataBuff[11]	;
    gVFPar.ocGain                   =    gSendToMotor05MsDataBuff[12]	;
    gVFPar.ocPoint                  =    gSendToMotor05MsDataBuff[13]	;
}

void ParSend2Ms(void)
{
    gSendToFunctionDataBuff[0]	=	gMainStatus.StatusWord.all	;
    gSendToFunctionDataBuff[1]	=	gGetParVarable.StatusWord	;
    gSendToFunctionDataBuff[2]	=	gError.ErrorCode.ErrorCodeStruct.ErrorCode1	;
    gSendToFunctionDataBuff[3]	=	gError.ErrorCode.ErrorCodeStruct.ErrorCode2	;
    gSendToFunctionDataBuff[4]	=	gError.ErrorInfo[0].all	;
    gSendToFunctionDataBuff[5]	=	gError.ErrorInfo[1].all	;
    gSendToFunctionDataBuff[6]	=	gError.ErrorInfo[2].all	;
    gSendToFunctionDataBuff[7]	=	gError.ErrorInfo[3].all	;
    gSendToFunctionDataBuff[8]	=	gError.ErrorInfo[4].all	;
    gSendToFunctionDataBuff[9]	=	gLineCur.ErrorShow	;
    gSendToFunctionDataBuff[10]	=	gMainCmd.FreqToFunc	;
    gSendToFunctionDataBuff[11]	=	gOutVolt.VoltDisplay	;
    gSendToFunctionDataBuff[12]	=	gTemperature.Temp	;
    gSendToFunctionDataBuff[13]	=	gMotorInfo.Current	;
    gSendToFunctionDataBuff[14]	=	gSoftVersion	;
    gSendToFunctionDataBuff[15]	=	gAI.gAI1	;
    gSendToFunctionDataBuff[16]	=	gAI.gAI2	;
    gSendToFunctionDataBuff[17]	=	gAI.gAI3	;
    gSendToFunctionDataBuff[18]	=	gUDC.uDCFilter	;
    gSendToFunctionDataBuff[19]	=	gLineCur.CurTorque	;
    gSendToFunctionDataBuff[20]	=	gLineCur.CurPerShow	;
    gSendToFunctionDataBuff[21]	=	gIPMPos.RealPos	;
    gSendToFunctionDataBuff[22]	=	uReservedData	;
    gSendToFunctionDataBuff[23]	=	gFVCSpeed.SpeedEncoder;
    gSendToFunctionDataBuff[24] =   gPowerTrq.InvPower_si;
    gSendToFunctionDataBuff[25] =   gPowerTrq.TrqOut_pu;
    gSendToFunctionDataBuff[26] =   gRotorTrans.AbsRotPos;              // 
    gSendToFunctionDataBuff[27] =   gPowerTrq.anglePF;
    gSendToFunctionDataBuff[28] =   gIPMPos.Zcounter;
    gSendToFunctionDataBuff[29] =   gHVfOscIndex.oscIndex;      // u0- 59

// UF 参数组
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC + 0 ] =   gIMTSetQ12.M;                       // uf-00
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC + 1 ] =   gIMTQ12.M;                         // uf-01
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	2 ] =   gIMTSetQ12.T;                      // uf-02
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	3 ] =   gIMTQ12.T;                         // uf-03
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	4 ] =   gIAmpTheta.PowerAngle;                 // uf-04
    
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	5 ] =   gMainCmd.FreqFeed;                 // uf-05  
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	6 ] =   gFVCSpeed.MTTime;     // uf-06
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	7 ]	=	gFVCSpeed.MTPulseNum        ;   // uf-07
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	8 ]	=	gIAmpTheta.Amp 	;   // uf-08
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	9 ]	=	gLineCur.CurBaseInv	;   // uf-09
    
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	10]	=	gMainCmd.FreqSet	;               // uf-10
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	11] =   gFVCSpeed.SpeedEncoder;           // uf-11
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	12]	=	gIMTQ12_obs.M 	;           // uf-12    
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	13]	=	gIMTQ12_obs.T 	;           // uf-13
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	14]	=	gRatio	;   // uf-14
    
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	15]	=	gUMTSet.M	;   // uf-15
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	16]	=	gUMTSet.T 	;   // uf-16
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	17]	=	gUVWPG.UVWAngle ;   // uf-17
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	18]	=	gOutVolt.VoltPhaseApply;   // uf-18
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	19]	=	gIPMPos.RotorPos	;   // uf-19
    
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	20]	=   gABFluxS_LowFilter.Alph;              // uf-20
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	21]	=   gABFluxS_LowFilter.Beta;              // uf-21
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	22]	=   gPhase.IMPhase>>16;        // uf-22
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	23]	=   gFluxR.Theta ;                  // uf-23 进入z中断的个数，
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	24]	=	gFluxR.Amp	;                   // uf-24
    
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	25]	=	gIPMPosCheck.UvwRevCnt	;           // uf-25
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	26]	=	gIUVWQ12.U;   // uf-26
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	27]	=	gIPMPos.ABZ_QepTotal	;   // uf-27
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	28]	=	gCpuTime.CpuCoff0Ms	;               // uf-28
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	29]	=	gCpuTime.CpuBusyCoff	;           // uf-29
}

void ParGet2Ms(void)
{    
    gSubCommand.all              =	gSendToMotor2MsDataBuff[0]	;  
    gMainCmd.FreqDesired         =	gSendToMotor2MsDataBuff[1]	;  
    gBasePar.MaxFreq             =	gSendToMotor2MsDataBuff[2]	;  
    gBasePar.FcSet               =	gSendToMotor2MsDataBuff[3]	;  
    gMotorInfo.Power             =	gSendToMotor2MsDataBuff[4]	;  
    gMotorExtInfo.Rpm            =	gSendToMotor2MsDataBuff[5]	;  
    gMotorExtInfo.R1             =	gSendToMotor2MsDataBuff[6]	;  
    gMotorExtInfo.R2             =	gSendToMotor2MsDataBuff[7]	;  
    gMotorExtInfo.L0             =	gSendToMotor2MsDataBuff[8]	;  
    gMotorExtInfo.LM             =	gSendToMotor2MsDataBuff[9]	;  
    gMotorExtInfo.I0             =	gSendToMotor2MsDataBuff[10]	;  
    gMotorExtInfo.RsPm           =	gSendToMotor2MsDataBuff[11]	;  
    gMotorExtInfo.LD             =	gSendToMotor2MsDataBuff[12]	;  
    gMotorExtInfo.LQ             =	gSendToMotor2MsDataBuff[13]	;  
    gVCPar.ASRKpLow              =	gSendToMotor2MsDataBuff[14]	;  
    gVCPar.ASRTILow              =	gSendToMotor2MsDataBuff[15]	;  
    gVCPar.ASRKpHigh             =	gSendToMotor2MsDataBuff[16]	;  
    gVCPar.ASRTIHigh             =	gSendToMotor2MsDataBuff[17]	;  
    gVCPar.ASRSwitchLow          =	gSendToMotor2MsDataBuff[18]	;  
    gVCPar.ASRSwitchHigh         =	gSendToMotor2MsDataBuff[19]	;  
    gVCPar.VCWsCoff              =	gSendToMotor2MsDataBuff[20]	;  
    gVCPar.VCSpeedFilter         =	gSendToMotor2MsDataBuff[21]	;  
    gPGData.PulseNum             =	gSendToMotor2MsDataBuff[22]	;  
    gVCPar.VcOverExc             =	gSendToMotor2MsDataBuff[23]	;  
    gLoadLose.ChkLevel           =  gSendToMotor2MsDataBuff[24]	; 
    gLoadLose.ChkTime            =  gSendToMotor2MsDataBuff[25]	; 
    gVCPar.VCTorqueLim           =	gSendToMotor2MsDataBuff[26]	;  
    gVFPar.VFTorqueUp            =	gSendToMotor2MsDataBuff[27]	;  
    gVFPar.VFTorqueUpLim         =	gSendToMotor2MsDataBuff[28]	;  
    gVFPar.VFLineFreq1           =	gSendToMotor2MsDataBuff[29]	;  
    gVFPar.VFLineVolt1           =	gSendToMotor2MsDataBuff[30]	;  
    gVFPar.VFLineFreq2           =	gSendToMotor2MsDataBuff[31]	;  
    gVFPar.VFLineVolt2           =	gSendToMotor2MsDataBuff[32]	;  
    gVFPar.VFLineFreq3           =	gSendToMotor2MsDataBuff[33]	;  
    gVFPar.VFLineVolt3           =	gSendToMotor2MsDataBuff[34]	;  
    gVFPar.VFWsComp              =	gSendToMotor2MsDataBuff[35]	;  
    gVFPar.VFOverExc             =	gSendToMotor2MsDataBuff[36]	;  
    gVFPar.VFOvShock             =	gSendToMotor2MsDataBuff[37]	;  
    gComPar.StartDCBrakeCur      =	gSendToMotor2MsDataBuff[38]	;  
    gComPar.StopDCBrakeCur       =	gSendToMotor2MsDataBuff[39]	;  
    gComPar.BrakeCoff            =	gSendToMotor2MsDataBuff[40]	;  
    gComPar.MotorOvLoad          =	gSendToMotor2MsDataBuff[41]	;  
    gComPar.PerMotorOvLoad       =	gSendToMotor2MsDataBuff[42]	;  
    gPWM.SoftPWMTune             =	gSendToMotor2MsDataBuff[43]	;  
    gADC.DelaySet                =	gSendToMotor2MsDataBuff[44]	;  
    gInvInfo.LowUdcCoff          =	gSendToMotor2MsDataBuff[45]	;  
    gPGData.PGTypeGetFromFun     =	gSendToMotor2MsDataBuff[46]	;  
    gFVCSpeed.TransRatio       =	gSendToMotor2MsDataBuff[47]	;  
    gInvInfo.InvTypeSet          =	gSendToMotor2MsDataBuff[48]	;  
    gInvInfo.GpTypeSet           =	gSendToMotor2MsDataBuff[49]	;  
    gInvInfo.TempType            =	gSendToMotor2MsDataBuff[50]	;   
    gInvInfo.UDCCoff             =	gSendToMotor2MsDataBuff[51]	;  
    gInvInfo.CurrentCoff         =	gSendToMotor2MsDataBuff[52]	;  
    gUVCoff.UDivVGet             =	gSendToMotor2MsDataBuff[53]	;  
    gVCPar.SvcMode               =	gSendToMotor2MsDataBuff[54]	;  
    gDeadBand.DeadTimeSet        =	gSendToMotor2MsDataBuff[55]	; 
    gComPar.SpdSearchMethod      =	gSendToMotor2MsDataBuff[56]	;  
    gComPar.SpdSearchTimeSet     =	gSendToMotor2MsDataBuff[57]	;  
    gIPMPos.PowerOffPos          =	gSendToMotor2MsDataBuff[58]	;       // 上次掉电同步机角度
    gRotorTrans.Poles            =	gSendToMotor2MsDataBuff[59]	;   
    gMotorExtInfo.BemfVolt       =	gSendToMotor2MsDataBuff[60]	;  
    gVCPar.AcrImKp               =	gSendToMotor2MsDataBuff[61]	;  
    gVCPar.AcrImKi               =	gSendToMotor2MsDataBuff[62]	;  
    gVCPar.AcrItKp               =	gSendToMotor2MsDataBuff[63]	;  
    gVCPar.AcrItKi               =	gSendToMotor2MsDataBuff[64]	;   
    gPGData.SpeedDir             =	gSendToMotor2MsDataBuff[65]	;  
    gIPMPos.RotorZeroGet         =	gSendToMotor2MsDataBuff[66]	;    
    gPWM.PwmModeSwitchLF       =    gSendToMotor2MsDataBuff[67] ;
    gUVWPG.UvwDir                           =	gSendToMotor2MsDataBuff[68]	;  
    gUVWPG.UvwZeroPos_deg                   =	gSendToMotor2MsDataBuff[69]	;  
    gFluxWeak.Mode                          =	gSendToMotor2MsDataBuff[70]	;  
    gFluxWeak.CoefFlux                      =	gSendToMotor2MsDataBuff[71]	;  
    gFluxWeak.IdMax                         =	gSendToMotor2MsDataBuff[72]	;  
    gFluxWeak.CoefAdj                       =	gSendToMotor2MsDataBuff[73]	;  
    gFluxWeak.CoefKI                        =	gSendToMotor2MsDataBuff[74]	;  
    
// 
    gTestDataReceive.TestData0       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 0 ]	;  
    gTestDataReceive.TestData1       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 1 ]	;  
    gTestDataReceive.TestData2       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 2 ]	;  
    gTestDataReceive.TestData3       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 3 ]	;  
    gTestDataReceive.TestData4       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 4 ]	;  
    gTestDataReceive.TestData5       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 5 ]	;  
    gVFPar.SVCTorqueUp               =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 6 ]	; // SVC励磁电流提升
    gVFPar.SVCTorqueUpLim            =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 7 ]	; // SVC励磁电流提升截止频率
//    gTestDataReceive.TestData6       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 6 ]	;  
//    gTestDataReceive.TestData7       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 7 ]	; 
    gTestDataReceive.TestData8       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 8 ]	; 
    gTestDataReceive.TestData9       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 9 ]	; 
    gTestDataReceive.TestData10      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 10]	; 
    gMotorExtPer.FluxLeakCurveGain   =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 11]	; 
    //gTestDataReceive.TestData11      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 11]	; 
    gTestDataReceive.TestData12      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 12]	; 
    gTestDataReceive.TestData13      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 13]	; 
    gTestDataReceive.TestData14      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 14]	; 
    gTestDataReceive.TestData15      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 15]	; 
    gTestDataReceive.TestData16      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 16]	; 
    gTestDataReceive.TestData17      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 17]	; 
    gTestDataReceive.TestData18      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 18]	; 
    gTestDataReceive.TestData19      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 19]	; 
    gTestDataReceive.TestData20      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 20]	; 
    gTestDataReceive.TestData21      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 21]	; 
    gTestDataReceive.TestData22      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 22]	; 
    gTestDataReceive.TestData23      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 23]	; 
    gTestDataReceive.TestData24      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 24]	; 
    gTestDataReceive.TestData25      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 25]	; 
    gTestDataReceive.TestData26      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 26]	; 
    gTestDataReceive.TestData27      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 27]	; 
    gTestDataReceive.TestData28      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 28]	; 
    gTestDataReceive.TestData29      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 29]	; 
    gTestDataReceive.TestData30      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 30]	; 
    gTestDataReceive.TestData31      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 31]	; 
    gTestDataReceive.TestData32      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 32]	; 
    gTestDataReceive.TestData33      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 33]	; 
    gTestDataReceive.TestData34      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 34]	; 
    gTestDataReceive.TestData35      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 35]	; 
    gTestDataReceive.TestData36      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 36]	; 
    gTestDataReceive.TestData37      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 37]	; 
    gTestDataReceive.TestData38      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 38]	; 
    gTestDataReceive.TestData39      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 39]	; 
       
}

void ParSendTune(void)
{
    gParaIdToFunctionDataBuff[0]	=	gMotorExtReg.R1	;
    gParaIdToFunctionDataBuff[1]	=	gMotorExtReg.R2	;
    gParaIdToFunctionDataBuff[2]	=	gMotorExtReg.L0	;
    gParaIdToFunctionDataBuff[3]	=	gMotorExtReg.LM	;
    gParaIdToFunctionDataBuff[4]	=	gMotorExtReg.I0	;
    
    gParaIdToFunctionDataBuff[5]	=	gMotorExtReg.RsPm ;
    gParaIdToFunctionDataBuff[6]	=	gMotorExtReg.LD	;
    gParaIdToFunctionDataBuff[7]	=	gMotorExtReg.LQ	;
    gParaIdToFunctionDataBuff[8]	=	uReservedData	;
    gParaIdToFunctionDataBuff[9]	=	gEstBemf.BemfVolt	;
    
    gParaIdToFunctionDataBuff[10]	=	gPmParEst.IdKp	;
    gParaIdToFunctionDataBuff[11]	=	gPmParEst.IdKi	;
    gParaIdToFunctionDataBuff[12]	=	gPmParEst.IqKp	;
    gParaIdToFunctionDataBuff[13]	=	gPmParEst.IqKi	;
    gParaIdToFunctionDataBuff[14]	=	gPMInitPos.PWMTSet	;
    
    gParaIdToFunctionDataBuff[15]	=	gPGData.PGDir	;
    gParaIdToFunctionDataBuff[16]	=	gPmParEst.CoderPos_deg	;
    gParaIdToFunctionDataBuff[17]	=	gUVCoff.UDivVSave	;
    gParaIdToFunctionDataBuff[18]	=	gPmParEst.UvwDir	;
    gParaIdToFunctionDataBuff[19]	=	gPmParEst.UvwZeroAng_deg	;
}

