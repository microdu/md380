/****************************************************************

****************************************************************/

#ifndef MOTOR_STRUCT_DEFINE_H
#define  MOTOR_STRUCT_DEFINE_H
#ifdef __cplusplus
extern "C"[
#endif

// // 共用结构体的定义
struct MAIN_COMMAND_STRUCT_DEF{
   Uint    Start:1;					// 1 起动； 0 停机
   Uint    StartDC:1;				// 1 起动直流制动
   Uint    StopDC:1;				// 1 停机直流制动命令   
   Uint    ControlMode:2;           // 0－SVC，1－FVC，2－VF
   Uint    PreExcFlux:1;            // 1 预励磁启动
   Uint	   TorqueCtl:1;				// 1 转矩控制标志   
   Uint    SpeedFlag:2;             // 0－恒速，1－加速，2－减速
   Uint    IntegralDiscrete:1;      // 1－积分分离
};

union MAIN_COMMAND_UNION_DEF {
   Uint   all;
   struct MAIN_COMMAND_STRUCT_DEF  	bit;
};

struct MAIN_COMMAND_EXTEND_STRUCT_DEF{
   Uint    QepIndex:2;              // 0－使用EQP2测速,1－使用QEP1测速，2－PULSE脉冲输入
   Uint    SpeedRev:1;				// 1 码盘方向反向标志(//rt 次处字暂时不用)
   Uint    DeadCompMode:3;           //0－无死区补偿；1－AD死区补偿 2－优化死区补偿 2011.5.7 L1082
   Uint    ModulateType:1;          //0－异步调制；1 －同步调制
   Uint    Reserved:1;             //0－保留
   Uint    Reserved1:1;             //保留
   Uint    FreqUint:2;              //频率指令单位。0－1Hz；1－0.1Hz；2－0.01Hz
   Uint    SpeedSearch:2;			// 0－转速跟踪无效，1－转速跟踪模式1，2－转速跟踪模式2
   Uint    ShortGnd:1;				// 1－上电对地短路检测标志   
};

typedef union MAIN_COMMAND_EXTEND_UNION_DEF{
    Uint all;
    struct MAIN_COMMAND_EXTEND_STRUCT_DEF bit;
}MAIN_COMMAND_EXTEND_UNION;

struct SUB_COMMAND_STRUCT_DEF{
   Uint    ErrorOK:1;				// 1 故障处理完毕标志
   Uint    OutputLost:1;			// 1 输出缺相检测使能
   Uint    InputLost:1;				// 1 输入缺相保护使能
   Uint    MotorOvLoad:1;			// 1 电机过载保护使能
   Uint	   LoadLose:1;				// 1 输出掉载保护使能标志
   Uint    NoStop:1;				// 1 瞬停不停使能
   Uint	   CBCEnable:1;				// 1 逐波限流功能使能标志
   Uint    VarFcByTem:1;			// 1 载波频率随温度调整(固定为1)
   Uint    FanNoStop:1;				// 1 停机直流制动等待时间内风扇运行标志  
}; 
typedef union SUB_COMMAND_UNION_DEF {
   Uint   all;
   struct SUB_COMMAND_STRUCT_DEF  	bit;
}SUB_COMMAND_UNION;

struct SEND_STATUS_STRUCT_DEF{
   Uint    RunEnable:1;				// 1 初始化完成，可以运行标志
   Uint    LowUDC:1;				// 0 母线电压欠压故障标志
   Uint    StartStop:1;				// 1 运行/停机状态标志 (现在该标志没有用)//rt
   Uint    ShortGndOver:1;			// 1 对地短路检测完毕标志    
   Uint    SpeedSearchOver:1;		// 1 转速跟踪结束标志
   Uint    PerOvLoadInv:1;			// 1 变频器过载预报警标志
   Uint    PerOvLoadMotor:1;		// 1 电机过载预报警标志
   Uint    FanControl:1;			// 1 风扇运行
   Uint    OutOff:1;				// 1 变频器输出空开断开标志
}; 

typedef union SEND_STATUS_UNION_DEF {
   Uint   all;
   struct SEND_STATUS_STRUCT_DEF  	bit;
}SEND_STATUS_UNION;

// // 以下为基本信息数据结构 
typedef struct INV_STRUCT_DEF {
	Uint 	InvTypeApply;			    //实际使用的机型（P型机 = G型机-1）
	Uint    InvTypeSet;                 //功能传递的机型，包含电压等级信息
	Uint    InvVoltageType;             //变频器电压等级信息
	Uint 	InvType;				    //用户设置的机型，去除电压等级信息，范围在00－99
	Uint    GpTypeSet;                  // 功能码设定的GP
	Uint    GPType;                     // 1 G型机，2 P型机
	Uint 	InvCurrent;				    //查表得到的变频器电流，	单位由机型确定
	Uint    InvCurrentOvload;           //变频器过载电流，	单位0.01A
	Uint    InvCurrForP;                //P型机使用的额定电流       过载保护用电流
	Uint 	InvVolt;				    //查表得到的变频器电压，	单位1V
	Uint 	CurrentCoff;			    //变频器电流矫正系数  ，	单位0.1%
	Uint 	UDCCoff;				    //变频器母线电压矫正系数，	单位0.1%
	Uint 	TempType;				    //变频器温度曲线选择
	Uint 	InvUpUDC;				    //母线过压点				单位0.1V
	Uint    InvLowUdcStad;              // 未校正的欠压点
	Uint 	InvLowUDC;				    //欠压点
	Uint    LowUdcCoff;                 //欠压点校正系数
	Uint 	BaseUdc;				    //母线电压基准 380V机器为537.4V
}INV_STRUCT;                         //变频器硬件信息结构

// // 使用枚举变量区分电机类型和控制模式的组合
typedef enum CONTROL_MOTOR_TYPE_ENUM_DEF{
    ASYNC_SVC,                          // 
    ASYNC_FVC,                          // 
    ASYNC_VF,                           // 异步机SVC控制为0，FVC为1，VF为2
    
    SYNC_SVC = 10,                      // 
    SYNC_FVC,
    SYNC_VF,                            // 同步机SVC为0，FVC为1，VF为2 
    
    DC_CONTROL = 20,                    // 直流制动为20
    RUN_SYNC_TUNE                       // 参数辨识时需要电机运行的状态，主要是同步机使用
}CONTROL_MOTOR_TYPE_ENUM;  

typedef struct MOTOR_STRUCT_DEF {
    Uint    MotorType;              //0－普通异步电机；1－变频异步电机；2－永磁同步电机
    Uint    LastMotorType;          //存储上一拍功能传递的电机机型，用于某些在电机类型改变时，需要修正的变量
	Uint 	Power;					//电机功率					单位0.1KW
	Uint 	Votage;					//电机电压					单位1V
	Uint 	CurrentGet;				//功能传递的电机电流			单位由机型确定
	Uint 	Frequency;				//电机频率					
	Uint 	FreqPer;				//标么值电机频率
	Uint 	Current;				//驱动程序选用的电流基值(可能和实际电机电流不等)					
	Uint	CurBaseCoff;			//电流基值的放大倍数
	Uint    Motor_HFreq;            // 电机额定频率的60%
    Uint    Motor_LFreq;            // 电机额定频率的40%
}MOTOR_STRUCT;                   //电机基本信息结构

typedef struct MOTOR_EXTERN_STRUCT_DEF {
    Uint    UnitCoff;                           //电机参数转换系数 机型小于22，为1；大于22，为10
	Uint 	R1;						            //定子相电阻		 机型小于22，单位0.001欧姆，大于22，0.0001欧姆
	Uint 	R2;						            //转子相电阻		 机型小于22，单位0.001欧姆，大于22，0.0001欧姆
	Uint 	L0;						            //漏感			 机型小于22，单位0.01mH，大于22，单位0.001mH
	Uint 	LM;						            //互感			 机型小于22，单位0.1mH，大于22，单位0.01mH
	Uint 	I0;						            //空载电流		 机型小于22，单位0.01A，大于22，单位0.1A
	Uint    IoVsFreq;                           //弱磁区反比速度变化的空载电流，只计算它的标么值
	Uint 	Rpm;					            //电机转速		 单位1rpm
    Uint    RatedComp;	                        //额定转差率       0.01Hz, pu
	Uint 	Poles;					            //电机极数
	Uint 	L1;						            //定子相电感		 机型小于22，单位0.1mH，大于22，单位0.01mH
	Uint 	L2;						            //转子相电感		 机型小于22，单位0.1mH，大于22，单位0.01mH
    Uint    RsPm;                               // 同步机定子电阻
	Uint 	LD;						            //同步机D轴电感     机型 <22，单位0.01mH，机型 >22，单位0.001mH
	Uint 	LQ;						            //同步机Q轴电感     机型 <22，单位0.01mH，机型 >22，单位0.001mH
	Uint    BemfVolt;                               // 同步机反电动势电压， 可以计算出转子磁链
	Uint    FluxRotor;                              // 同步机转子磁链  Q12
    Uint    FluxRotor1;
    Uint    ItRated;                                // 额定力矩电流，pm im共用
    Uint    FluxLeakCurveGain;
}MOTOR_EXTERN_STRUCT;   //电机扩展信息结构

struct ERROR_FLAG_SIGNAL {
    Uint16  OvCurFlag:1;                            // bit0=1表示发生了过流中断
    Uint16  OvUdcFlag:1;                            // bit1=1表示发生了过压中断
	Uint16  Res:14;                                 // 保留    
};

union ERROR_FLAG_SIGNAL_DEF {
   Uint16                	all;
   struct ERROR_FLAG_SIGNAL  bit;
};

typedef struct PRG_STATUS_STRUCT_DEF{
   Uint    PWMDisable:1;			                        //BIT0=1 表示不发送PWM波；
   Uint    ACRDisable:1;			                        //BIT1=1 表示不计算电流环；
   Uint    ASRDisable:1;			                        //BIT2=1 表示不计算速度环；
}PRG_STATUS_STRUCT;                                     //程序运行控制状态字的bit安排

typedef union PRG_STATUS_UNION_DEF {                // 程序状态字
   Uint   all;
   PRG_STATUS_STRUCT bit;
}PRG_STATUS_UNION;

typedef struct RUN_STATUS_STRUCT_DEF {
	Uint 	RunStep;				                        //主步骤
	Uint 	SubStep;				                        //辅步骤
	Uint    ParaCalTimes;                                   //用于控制上电后只计算一次的参数转换。
	PRG_STATUS_UNION 	PrgStatus;				            //程序控制状态字	
	union   ERROR_FLAG_SIGNAL_DEF	ErrFlag;									
    SEND_STATUS_UNION		StatusWord;	
}RUN_STATUS_STRUCT;                                     //变频器运行状态结构


typedef struct BASE_COMMAND_STRUCT_DEF {
	union MAIN_COMMAND_UNION_DEF Command;	            //主命令字结构
	int 	FreqSet;				                    // 功能传递的设定频率
	int     FreqSetApply;                               // 驱动实际使用的设定频率
	int		FreqSetBak;				                    //最新的非0给定速度
	int 	FreqSyn;			                        //实际速度（同步速度）
    int     FreqSynFilter;                              //实际输出同步频率的滤波值，用于弱磁区计算
	int     FreqWs;                                   // 矢量时计算的转差
	int 	FreqDesired;			                    //目标速度
	int 	VCTorqueLim;			                    //VC转矩限定
	int     FreqToFunc;                                 //反馈给功能模块的速度，标么值表示
	                                                    // 现在反馈速度分为变频器运行频率和编码器测速
    int     FreqFeed;
	long 	FreqReal;				                    // 实际设定频率(非标么值表示),单位0.01Hz
	long    FreqDesiredReal;                            // 实际目标频率(非标么值表示),单位0.01Hz	
    long    FreqSetReal;                                // 实际设定频率(非标么值表示),单位0.01Hz	
                                                        // 功能传递的频率值转换为0.01Hz时的系数
    long    FreqRealFilt;
                                                        
	Uint    pu2siCoeff;                          //标么值频率基值单位与程序中使用的实际频率单位不同，存在一个转换系数
	                                                    // 1Hz 转化为功能小数点的系数              
	Uint    si2puCoeff;                          //程序中的实际频率转换为标么值频率，需要的校正系数。    
	Uint	FirstCnt;                            //用于启动时的特殊处理，延迟计数值

    //Uint	SpeedFalg;				//加减速标志
}BASE_COMMAND_STRUCT;                              //实时修改的命令结构

// // 以下为和电机控制相关设定参数定义数据结构 
typedef struct BASE_PAR_STRUCT_DEF {
	Ulong 	FullFreq01;				                    // Full freq, SI, 0.01Hz;
	Uint    FullFreq;                                   // SI, 小数点与功能基值一致，用于将功能传递的实际值转换为标么值
	Uint 	MaxFreq;				                    //最大频率 format: point
	Uint 	FcSet;					                    //设定载波频率		
	Uint 	FcSetApply;				                    //实际载波频率		
}BASE_PAR_STRUCT;	                                //基本运行信息结构

typedef struct COM_PAR_INFO_STRUCT_DEF {
	Uint 	StartDCBrakeCur;		                    //起动直流制动电流
	Uint 	StopDCBrakeCur;			                    //停机直流制动电流
	Uint 	BrakeCoff;				                    //制动电阻使用率
	Uint 	MotorOvLoad;			                    //电机过载保护增益
	Uint 	PerMotorOvLoad;			                    //电机过载保护预警系数
	Uint 	SpdSearchMethod;		                    //速度搜索方式
	Uint 	SpdSearchTimeSet;                           //功能传递速度搜索快慢
	Uint 	SpdSearchTime;			                    //实际速度搜索快慢
}COM_PAR_INFO_STRUCT;	                            //和运行方式无关的参数设置结构

typedef struct MOTOR_POWER_TORQUE_DEF{
    long    InvPowerPU;                             // 变频器输出功率标么值
    int     InvPower_si;                            // 变频器输出功率
    int     TrqOut_pu;                              // 输出转矩 0.1%

    Uint    rpItRated;                              // 额定力矩电流的倒数 Q12    
    int     anglePF;
    int     Cur_Ft4;
}MOTOR_POWER_TORQUE;

// // 公共模块使用的数据结构
typedef struct CPU_TIME_STRUCT_DEF {
	Ulong 	MFA2msBase;
    Ulong 	MFB2msBase;
    Ulong 	MFC2msBase;
    Ulong 	MFD2msBase;
    Ulong   Motor05MsBase;
	Ulong	ADCIntBase;
	Ulong	PWMIntBase;
	Uint 	MFA2ms;
    Uint 	MFB2ms;
    Uint 	MFC2ms;
    Uint 	MFD2ms;
	Uint    Motor05Ms;
	Uint	ADCInt;				                    //ADC中断执行时间
	Uint	PWMInt;				                    //PWM中断执行时间
    Uint    tmpBase;
    Uint    tmpTime;
    Uint    CpuCoff0Ms;
    Uint    tmp0Ms;
    Uint    Det05msClk;
    Uint    CpuBusyCoff;                            // cpu时间片执行度系数
}CPU_TIME_STRUCT;	    //统计模块执行时间的数据结构

// // 驱动部分调试用数据结构
typedef struct MOTOR_DEBUG_DATA_RECEIVE_STRUCT_DEF{
    int     TestData0;
    int     TestData1;
    int     TestData2;
    int     TestData3;
    int     TestData4;
    int     TestData5;
    int     TestData6;
    int     TestData7;
    int     TestData8;
    int     TestData9;
    int     TestData10;
    int     TestData11;
    int     TestData12;
    int     TestData13;
    int     TestData14;
    int     TestData15;
    int     TestData16;
    int     TestData17;
    int     TestData18;
    int     TestData19;
    int     TestData20;
    int     TestData21;
    int     TestData22;
    int     TestData23;
    int     TestData24;
    int     TestData25;
    int     TestData26;
    int     TestData27;
    int     TestData28;
    int     TestData29;
    int     TestData30;
    int     TestData31;
    int     TestData32;
    int     TestData33;
    int     TestData34;
    int     TestData35;
    int     TestData36;
    int     TestData37;
    int     TestData38;
    int     TestData39;
}MOTOR_DEBUG_DATA_RECEIVE_STRUCT;   //性能调试数据接收

typedef struct MOTOR_DEBUG_DATA_DISPLAY_STRUCT_DEF{
    int     DisplayData1;
    int     DisplayData2;    
    int     DisplayData3;
    int     DisplayData4;
    int     DisplayData5;    
    int     DisplayData6;
    int     DisplayData7;
    int     DisplayData8;    
    int     DisplayData9;
    int     DisplayData10;
    int     DisplayData11;
    int     DisplayData12;    
    int     DisplayData13;
    int     DisplayData14;
    int     DisplayData15;    
    int     DisplayData16;
    int     DisplayData17;
    int     DisplayData18;    
    int     DisplayData19;
    int     DisplayData20; 
    int     DisplayData21;
    int     DisplayData22;    
    int     DisplayData23;
    int     DisplayData24;
    int     DisplayData25;    
    int     DisplayData26;
    int     DisplayData27;
    int     DisplayData28;    
    int     DisplayData29;
    int     DisplayData30;     
}MOTOR_DEBUG_DATA_DISPLAY_STRUCT;   //性能调试数据显示

// // 结构体定义结束

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif

