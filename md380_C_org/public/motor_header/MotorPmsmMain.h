/***************************************************************
文件功能：
文件版本：VERSION 1.0
最新更新：
************************************************************/
#ifndef MOTOR_PMSM_INCLUDE_H
#define MOTOR_PMSM_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif


//**************************************************************************
typedef struct IPM_POS_CHECK_STRUCT_DEF {
	Uint	FirstPos;				//第一次位置
	Uint	Cnt;					//计数器
	long	TotalErr;				//位置偏差累加
	Ulong	TotalErrAbs;			//位置偏差的绝对值累加
	int     UvwStopErr;
    int     UvwStopErr_deg;              // uvw的误差角度
    Uint    UvwRevCnt;              // 被UVW信号修正次数计数器
}IPM_POS_CHECK_STRUCT;         //永磁同步电机上电检测当前绝对位置角的数据结构

typedef struct IPM_POSITION_STRUCT_DEF {
    Uint	InitPosMethod;          //初始磁极位置检测方法
    
	Uint	RotorPos;				//转子位置角标么值
	Uint    ABZ_RotorPos_Ref;
	Uint	RealPos;				//转子实时角度(360.0度范围)

    Uint    InitPos;                // 磁极辨识得到的位置
    Uint    InitAngle_deg;          // 辨识得到的磁极位置，0-360.0deg
	
	Uint	RotorZero;				//编码器零点位置角（65536范围）
	Uint	RotorZeroGet;			//编码器零点位置角(360.0度范围)
	Uint    ZeroPosLast;            // 配合ABZ编码器时手动修改零点位置角

    Uint    PowerOffPos;            // 上次掉电视的角度
	Uint	PowerOffPosDeg;			//上次下电时候的角度(360.0度范围)
	
	Uint	PosInZInfo;				//Z信号到达时候的实时角度
	long	QepBak;					//上一拍QEP累加数值
	long	QepTotal;

    long      ABZ_QepTotal;
    long      ABZ_QepBak;  
    
    Uint    Zcounter;               // 检测到Z信号的个数，方便调试

	Uint    ZErrCnt;			    //Z信号错误计数器
	int     AbzErrPos;   
    int     AbzErrPos_deg;
}IPM_POSITION_STRUCT; //永磁同步电机和转子角度相关的结构

typedef struct PM_INIT_POSITION_DEF
{
	int SubStep;
	int PeriodCnt;
	int Section;	
	int CurFirst;
	int Cur[12];
	int Timer;
	unsigned int PWMTs;
	unsigned int PWMTGet;
	unsigned int PWMTSet;
}PM_INIT_POSITION;

typedef struct PM_FLUX_WEAK_DEF
{
    Uint    Mode;              // 弱磁模式选择 0: 不弱磁；1: 弱磁直接计算；2: 弱磁自动调整
    int     DecoupleMode;      // 解耦模式 0: 不解耦；1: 解耦
    int     CsrGainMode;        // 电流环参数修正模式 0: 不修正，1: 修正
    int     CoefFlux;          // 弱磁系数
	long     VoltCoef;          // voltage coefficient (%)
    long    FluxD;             // d轴总磁链 PhiD = PhiSd + FluxRotor
    int     FluxSd;            // PhiSd = Ld * gIMTQ12.M
	long     IqLpf;
    int     VoltLpf;            // 输出电压滤波值
    int     AbsFrqLpf;
    int     IdSet;
    int     IdMax;              // 弱磁d 轴电流限制 1%
    int     Vd;
    int     Vq;
	int     AdjustId;           // 模式2 调整量
    int     AdjustLimit;        // 模式2 调整量上限
	int     CoefAdj;            // 模式2 调整系数 
    int     CoefKI;             // 弱磁时电流环积分增益调整系数

    //另外增加的变量
    long    VoltMax;
    long    CurrCoef;
    long    Iq;   // Q轴电流 单位为uH ，也可当做Q24格式
    long    Omg;  // 实际角速度，Q10格式
    long    Id;   // 弱磁电流，Q12格式表么值
    long    IqFBLpf;// Q轴滤波电流,Q12格式表么值
    long    IqLimit;
    long    TorqeCurr;//程序中没有使用
    long    AdjustId1;// 程序中没有使用
    int     CoefIdComp;// IS300 中gSendToMotorDataBuff1[14]
    long    IqErr;     // Q轴电流偏差，Q12格式表么值
    int     IqErrAbs;
    long    OmgQ15;   // 实际频率的滤波值
    int     Ratio;   
    int     VoltOut;


    long    data0;
    long    data1;
    long    data2;
    long    data3;
    long    data4;
    long    data5;
    long    data6;
    long    data7;
    long    data8;
    long    data9;
    long    ud;
    long    uq;
   
        
}PM_FLUX_WEAK;

typedef struct PM_DECOUPLE_DEF{
    int Omeg;   // Q15
    int Isd;    // Q12
    int Isq;    // Q12
    int PhiSd;  // Q12      d轴全磁链
    int PhiSq;  // Q12
    int RotVd;  // Q12
    int RotVq;  // Q12
    int EnableDcp;
}PM_DECOUPLE;

typedef struct PM_FW_IN_DEF
{
	int  Time;			//弱磁控制时间间隔,0.5MS
	int  Bemf;			//1mv/(rad/s),反电动势系数的峰值
	int  Udc;			//0.1V，母线电压
	int  R;				//1mohm,相电阻
	int  IsSetMax;      //设定的最大输出相电流有效值，以电机额定电流为基值，Q12
	int  IsAsrSetMax;	//设定的速度环PI调节最大相电流有效值，以电机额定电流为基值，Q12
	int  IqSet;			//Q轴设定电流有效值，以电机额定电流为基值，Q12
	int  IqFeed;		//Q轴反馈电流有效值，以电机额定电流为基值，Q12	
	int  IdSet;			//Q轴设定电流有效值，以电机额定电流为基值，Q12
	int  IdFeed;		//Q轴反馈电流有效值，以电机额定电流为基值，Q12
	int  VolCsrOut;		//电流环PI调节输出的线电压有效，以电机额定线电压为基值，Q12
	int  UdCsrOut;		//电流环D轴PI调节输出的线电压有效，以电机额定线电压为基值，Q12
	int  UqCsrOut;		//电流环Q轴PI调节输出的线电压有效，以电机额定线电压为基值，Q12

	int  IsAsrOut;		//速度环PI调节输出的相电流有效值，以电机额定电流为基值，Q12
	int  MotorPoles;	//电机磁极对数
	int  FullFreq;		//0.1HZ，电机基准频率
	int  FreqFeed;		//电机反馈速度，以FullFreq为基值，Q15
	int  MotorVol;		//1V,电机额定电压
	long MotorCurr;		//0.01A,电机额定电流

	int  MaxSetUdc;		//0.1V,设定的最大输出母线电压，为0时输出直流母线电压
	int  RatioSet;		//弱磁控制时输出电压设定值，用于确定弱磁电流
						//以变频器能输出的最大电压为基值，Q12
						//必须小于4096，推荐值3700（Udc==540V)
						//母线电压越小该值应该设的越小
	long KpId;			//弱磁电流调节KP

	long MaxDelId;		//每次最大减小ID给定的数值;
	int  AdjuMode;		//自整定模式，0：不整定，1：整定最大转矩电流,
						//2:整定最大弱磁电流，3：整定最大转矩电流、弱磁电流

	int  UdForIqMax;	//弱磁控制时Q轴输出电压设定值,用于确定最大转矩电流
						//以变频器能输出的最大电压为基值，Q12
						//必须大于RatioSet，推荐值3900（Udc==540V)
						//母线电压越小该值应该设的越小

	long KpIqMax;		//最大转矩电流减小调节KP
	long KpIqMax1;		//最大转矩电流增大调节KP
							
	int  UqForIdMax;	//弱磁控制时Q轴输出电压设定值，用于确定最大弱磁电流
						//以变频器能输出的最大电压为基值，Q12
						//推荐值100（Udc==540V)，母线电压越小该值应该设的越大
	long KpIdMax;		//最大弱磁电流调节KP
	int  CsrMaxVolt;	//电流环D、Q轴最大输出电压
	
}PM_FW_IN;

typedef struct PM_FW_DEF
{	
	
	int  IdMax;
	int  IdMax1;
	int  PhiPerLd;
	int  UdcLpf;
	int  UdLpf;
	int  UqLpf;
	int  RatioLpf;
	int  UdForIqMax;
	long  RatioLpf1;
	int  CurPerLpf;
	int  TorqPerAmp;
	int  IqErrLpf;
	int  IdErrLpf;
	int  IdForTorq;
	
	long AbsOmgPer;
	long Omg;
	long OmgLpf;
	long CurrCoef;
	long CoefIqMax;
	
	long AdId;
	long AdIdIntg;

	long AdIqMax;	
	long AdIqMaxIntg;
	long AdIqMaxIntgMax;
	long AdIqMaxIntgMin;
	long AdIdMaxIntg;
	long TorqueCurr;
	long TorqueCurrMax;
	long MinUqLpf;
	long IqMax;
	long IqMax1;
	long PowerLpf;
	long TorqueEst;	
	long Ld;
	long Lq;
	long Lq1;
	int  TorqRevi;
	int  TReviCoef1;
	int  PosComp;
	int  MaxPosComp;
	int  MpcBack;
	int  MinPosComp;
	long IqSetLpf;
	int  UqCompStat;
	int  UqRatio;
	int  imSet;
	int  itSet;
	int  umSet;
	int  utSet;
	int  utSetLpf;
}PM_FW;
typedef struct PM_FW_OUT_DEF
{	
	
	int  IdSet;			//电流环D轴给定值，以电机额定电流为基值，Q12
	int  IqSet;			//电流环Q轴给定值，以电机额定电流为基值，Q12
	int  IsLimit;	   //速度环PI调节输出限定值，以电机额定电流为基值，Q12
	int  UqComp;		//Q轴补偿电压，以电机额定电压为基值，Q12
    int  PosComp;
	int  ClearKID;
}PM_FW_OUT;


//**************************************************************************
extern IPM_POSITION_STRUCT		gIPMPos;
extern PM_INIT_POSITION         gPMInitPos;
extern PM_FLUX_WEAK             gFluxWeak;
extern PM_DECOUPLE              gPmDecoup;
extern IPM_POS_CHECK_STRUCT	    gIPMPosCheck;
extern PM_FW_IN				   gPMFwIn;
extern PM_FW				   gPMFwCtrl;
extern PM_FW_OUT			   gPMFwOut;

//**************************************************************************
extern void SynCalRealPos(void);
extern void IPMCheckInitPos(void);
extern void PmChkInitPosRest(void);
extern void RunCaseIpmInitPos(void);
extern void SynCalLdAndLq(Uint m_Pos);
extern void IPMCalAcrPIDCoff(void);
extern void PmFluxWeakDeal();
extern void PmDecoupleDeal(void);
extern void PMClacFluxWeakId(void);
extern void PMSetFwPara1(void);
extern void PMSetFwPara(void);
#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================
