/***************************************************************************
文件说明： 
文件功能：
文件版本
最新更新：
更新日志：
zhuozhe : linxib
***************************************************************************/
#ifndef MOTOR_PM_INCLUDE_H
#define MOTOR_PM_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif


//同步机初始磁极位置推断方式选择
#define INIT_POS_NULL			0		//硬件检测方式
#define INIT_POS_VOLT_PULSE		1		//电压脉冲法


/*******************结构体定义******************************************/
typedef struct IPM_ZERO_POS_STRUCT_DEF {
	Uint	FeedPos;				// 以Z为基准，编码器累加位置角
	Uint    FeedABZPos;
	Uint	FirstPos;				//
	Uint	Flag;					// BIT0、1、2=1分别表示Z信号到、AB、UVW接线判断完成
	                                // 旋变时: 如果连续两个0.5ms 正角度，产生一个Flag标志
	Uint	zFilterCnt;              // Z 信号的滤波处理，要求两个Z间隔超过4ms
	Uint	Cnt;
	//Uint	ABAngleBak;
	long	QepBak;
	int		DetectCnt;
    //int     UvwCnt;
	long	TotalErr;				//上次下电时候的角度

    Uint    CurLimit;
    Uint    time;
}IPM_ZERO_POS_STRUCT;          //永磁同步电机检测编码器零点位置角的数据结构

typedef struct IPM_INITPOS_PULSE_STRUCT_DEF {
	Uint	Waite;
	Uint	Step;					// 标志， 设置Step为1起动静态辨识，结束后为0
	Uint	Flag;					// 1已经经过磁极初始位置检测的标志
	Uint	PeriodCnt;
	Uint	Section;
	Uint	PWMTs;
	Uint	InitPWMTs;
	Uint	LPhase[3];				// 顺便检测出电机相电感
	Uint    Ld;						// 机型 >22，单位0.001mH，机型 <22，单位0.01mH
	Uint    Lq;						// 机型 >22，单位0.001mH，机型 <22，单位0.01mH
	Uint	CurLimit;
	int		CurFirst;
	int		Cur[12];
    int     PhsChkStep;             // 同步机缺相检测步骤
}IPM_INITPOS_PULSE_STR;        //永磁同步电机上电初始位置检测的数据结构(电压脉冲法)

typedef struct PMSM_EST_PARAM_DEF
{
	Uint    IdKp;               // 电流环pi参数
	Uint    IdKi;
	Uint    IqKp;
	Uint    IqKi;

    Uint    CoderPos_deg;       // 编码器零点位置角；
    Uint    EstZero;            // 带载辨识时使用的零点位置角
    
    //Uint    UvwZeroPhase_deg;   // UVW信号零点位置角；
    Uint    UvwDir;             // 编码器绝对位置方向正反， UVW编码器表示UVW信号正反
    Uint    UvwZeroAng;         // uvw 零点角度
    Uint    UvwZeroAng_deg;

    Uint   UvwZPos;
}PMSM_EST_PARAM_DATA;

typedef struct PMSM_EST_BEMF_DEF
{
    Uint    TuneFreq;           // 记录反电动势辨识运行频率
    Ulong   TotalId1;           // 积分第一点电流值
    Ulong   TotalId2;           // 积分第二点电流值
    Ulong   TotalVq1;           // 积分第一点电压值
    Ulong   TotalVq2;           // 积分第二点电压值

    int     IdSet;              // M 轴电流设定
    int     IdSetFilt;
    int     IqSet;
    int     TuneFreqSet;        // 实时的速度给定
    int     TuneFreqAim;
    Ulong   AccDecTick;         // 加减速时间, 2ms Ticker个数
    int     FreqStep;
    long    FreqRem;
    
    Uint    BemfVolt;           // 反电动势， 标么化即为转子磁链
    Uint    Cnt;
}PMSM_EST_BEMF;


//***************************************************************************
extern IPM_ZERO_POS_STRUCT		gIPMZero;
extern IPM_INITPOS_PULSE_STR	gIPMInitPos;
extern PMSM_EST_PARAM_DATA		gPmParEst;
extern PMSM_EST_BEMF            gEstBemf;


//***************************************************************************
extern void SynInitPosDetSetPwm(Uint Section);
extern void SynTuneInitPos(void);
extern void SynInitPosDetect(void);
extern void SynTunePGZero_No_Load(void);
extern void SynTunePGZero_Load(void);
extern void SetIPMPos(Uint Pos);
extern void SetIPMPos_ABZRef(Uint Pos);
extern void SynTuneBemf(void);


#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

/*===========================================================================*/
// End of file.
/*===========================================================================*/

