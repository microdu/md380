/****************************************************************
文件功能: 开环矢量定、转子磁链计算，转子转速估计
文件版本： 
最新更新： 
输入变量：无
输出变量：无
使用外部全局变量：
                 gABVoltSet            //αβ坐标轴下定子实际输出电压
				 gIAlphBetaQ12         //αβ坐标轴下定子采样反馈电流
                 gBasePar              //载波频率生效值,频率基值
                 gMainCmd              //输出同步频率
                 gVCPar.VCSpeedFilter  //F2-07转速滤波系数
				 gMotorExtPer          //标幺化后的电机参数
				 gPhase
				 gVCPar.SvcMode        //SVC优化模式选择
修改外部全局变量：
				 gFluxR                //转子磁场的幅值及相位；
				                       //由于SVC1优化算法，有补偿，不能用于起重飞车保护 
				 gFluxS                //故增加定子磁场的幅值及相位的输出
	             gMainCmd.FreqFeed     //测速或SVC估计得到的转子转速值

开放性能调试用功能码：
      SVC1:
                gTestDataReceive.TestData3 //磁通观测低通滤波截止频率
				gTestDataReceive.TestData2 //磁通观测补偿系数
				gTestDataReceive.TestData4 //M轴电流闭环比例系数
				gTestDataReceive.TestData5 //T轴电流闭环比例系数
				gTestDataReceive.TestData8 //T轴电流给定值平滑滤波系数
****************************************************************/
#include "MotorVCInclude.h"
#include "MotorSvcInclude.h"
#include "MotorEncoder.h"

// // 全局变量定义
FLUX_STRUCT             gFluxR;
FLUX_STRUCT             gFluxS;
SVC_FLUX_CAL_STRUCT		gCalFlux;		
SVC_SPEED_STRUCT        gSVCSpeed;

//SVC 1 专用
ALPHABETA_STRUCT		gABFluxS_LowFilter;		//αβ坐标轴下定子磁通给定值
ALPHABETA_STRUCT		gABFluxS_comp;	    //αβ坐标轴下定子磁通观测补偿值：该变量应定义为全局变量
MT_STRUCT_Q24           gIMTQ24_obs;        //磁通观测坐标系下分解得到的MT轴电流
MT_STRUCT_Q24           gIMTQ12_obs;
/************************************************************
函数输入:无
函数输出:无
调用位置:上电初始化和运行之前
调用条件:异步机SVC运行
函数功能:初始化运行中用到的变量
************************************************************/
void ResetSVC(void)
{
	gABVoltSet.Alph = 0;
	gABVoltSet.Beta = 0;


	gFluxR.Amp = 0;
	gFluxR.Theta = 0;
	gFluxS.Amp = 0;
	gFluxS.Theta = 0;


	gSVCSpeed.SvcRotorSpeed 	= 0;
    gMainCmd.FreqFeed      = 0;            //防止故障后启动出问题
	gSVCSpeed.SvcSynSpeed 	= 0;
    gMainCmd.FreqSynFilter = 0;

	gSVCSpeed.SvcWs 			= 0;
	gSVCSpeed.SvcSignal = 0;
	gMainCmd.FreqReal = 0;						//new reset words
	gSVCSpeed.SvcLastFluxPos = 0;
	gMainCmd.FirstCnt = 0;


	//Added by jxl:用于新增磁通观测补偿值计算
    gABFluxS_LowFilter.Alph = 0;  //Q13:Alph轴定子磁通用电压模型和一阶惯性环节的观测值
    gABFluxS_LowFilter.Beta = 0;  //Q13:Beta轴定子磁通用电压模型和一阶惯性环节的观测值
	gABFluxS_comp.Alph = 0;       //Q13:Alph轴定子磁通补偿值
	gABFluxS_comp.Beta = 0;       //Q13:Beta轴定子磁通补偿值

	gFluxR.Theta = (Uint)((long)gPhase.IMPhase>>16);
}
/*************************************************************
	SVC计算定子磁通矢量和转子磁通矢量
	SVC0 无补偿
	SVC1 有补偿
*************************************************************/
/************************************************************
函数输入:
          gMainCmd.FreqSyn          //实际输出同步频率
		  gIAlphBetaQ12             //静止坐标系采样反馈定子电流
          gIMTSetApply              //MT轴电流实际生效的设定值
		  gABVoltSet                //实际输出电压
		  gPhase.IMPhase            //设定的转子磁场同步角
		  gPhase.StepPhaseApply     //转子磁场同步角累加步长
函数输出:
          gFluxR                    //极坐标系转子磁通观测值
		  gFluxS                    //极坐标系定子磁通观测值
调用功能码：
          gTestDataReceive.TestData3 //SVC1 磁通观测低通滤波截止频率
          gTestDataReceive.TestData2 //SVC1 磁通观测补偿系数

调用位置:AD中断
调用条件:异步机SVC运行，FVC磁通闭环，起重飞车保护
函数功能:电机定转子磁通幅值及相位观测
************************************************************/

void SVCCalFlux_380(void)  //季筱隆修改后算法
{
	long  m_Long;
	Ulong m_ULong;
	int   m_LrDivLm,m_LLou;
    Ulong m_Timer;
	int   m_DetaTimer;
    int     m_Angle,m_Sin,m_Cos;
    ALPHABETA_STRUCT		m_ABFluxS_cmd;		//αβ坐标轴下定子磁通给定值
    ALPHABETA_STRUCT		m_ABFluxR_cmd;		//αβ坐标轴下转子磁通给定值
    ALPHABETA_STRUCT		m_IAlphBeta_cmd;	    //定子两相坐标轴定子电流给定值
    ALPHABETA_STRUCT		m_ABFluxS;		//αβ坐标轴下定子磁通
    ALPHABETA_STRUCT		m_ABFluxR;		//αβ坐标轴下转子磁通
    
	gCalFlux.SampleTime = 25736L / (long)gBasePar.FcSetApply;	//载波周期 PI*2^13 / FC

   	if(0 == gVCPar.SvcMode)  //SVC0
	{
        gCalFlux.FilterTime = (gMainCmd.FreqReal * 839L >> 11) + 300;
        m_Timer = GetTime();
        m_DetaTimer = (((gSVCSpeed.Timer - m_Timer) & 0xFFFFFFul)>>3);
        gSVCSpeed.Timer = m_Timer;
	}
	else                     //SVC1
	{
	    //gCalFlux.FilterTime = 200;
	    gCalFlux.FilterTime = gTestDataReceive.TestData3;
	    gCalFlux.FilterTime = __IQsat(gCalFlux.FilterTime,2000,100);
	}

	m_LrDivLm = ((Ulong)gMotorExtPer.L1 << 14)/gMotorExtPer.LM;               // Lx
	//m_LrDivLm(Q14) = ((Ulong)gMotorExtPer.L1(Q9) << 14)/gMotorExtPer.LM(Q9);               // Lx
	m_LLou = ((Ulong)gMotorExtPer.L0 * 10000)/gBasePar.FullFreq01;    //2*L0
	//m_LLou(Q14)    = ((Ulong)gMotorExtPer.L0(Q14) * 5000(Q0))/gBasePar.FullFreq01(Q0);

/*******************************************/
    //计算静止坐标系的定子磁通给定值对观测得到的定子磁通进行补偿，再计算转子磁通

    //定子磁通观测：低通滤波代替开环积分
	m_Long = (long)gMotorExtPer.R1 * (long)gIAlphBetaQ12.Alph;	//R1为Q16格式
	//m_Long(Q28) = ((llong)gMotorExtPer.R1(Q16) * (llong)gIAlphBetaQ12.Alph(Q12));
	m_Long = (long)gABFluxS_LowFilter.Alph * (long)gCalFlux.FilterTime + m_Long;
	//m_Long(Q28) = (long)gABFluxS_LowFilter.Alph(Q14) * (long)gCalFlux.FilterTime(Q14) + m_Long(Q28);
	m_Long = (((long) gABVoltSet.Alph<<16) - m_Long)>>16;
	//m_Long(Q12) = (((long) gABVoltSet.Alph(Q12)<<16) - m_Long(Q28))>>16;
	m_Long = (long)m_Long * (long)gCalFlux.SampleTime;
	//m_Long(Q25) = (long)m_Long(12) * (long)gCalFlux.SampleTime(13);
	gABFluxS_LowFilter.Alph = (((long)gABFluxS_LowFilter.Alph<<11) + m_Long)>>11;
	//gABFluxS_LowFilter.Alph(Q14) = (((long)gABFluxS_LowFilter.Alph(Q14)<<11) + m_Long(Q25))>>11;

	m_Long = (long)gMotorExtPer.R1 * ((long)gIAlphBetaQ12.Beta);	//R1为Q16格式
	//m_Long(Q28) = ((llong)gMotorExtPer.R1(Q16) * (llong)gIAlphBetaQ12.Beta(12));	//R1为Q16格式
	m_Long = (long)gABFluxS_LowFilter.Beta * (long)gCalFlux.FilterTime + m_Long;
	//m_Long(Q28) = (long)gABFluxS_LowFilter.Beta(Q14) * (long)gCalFlux.FilterTime(Q14) + m_Long(Q28);
	m_Long = (((long)gABVoltSet.Beta<<16) - m_Long)>>16;
	//m_Long(Q12) = (((long)gABVoltSet.Beta(Q12)<<16) - m_Long(Q28))>>16;
	m_Long = (long)m_Long * (long)gCalFlux.SampleTime;
	//m_Long(Q25) = (long)m_Long(Q12) * (long)gCalFlux.SampleTime(Q13);
	gABFluxS_LowFilter.Beta = (((long)gABFluxS_LowFilter.Beta<<11) + m_Long)>>11;
	//gABFluxS_LowFilter.Beta(Q14) = (((long)gABFluxS_LowFilter.Beta(Q14)<<11) + m_LongQ25))>>11;

    //静止坐标系下定子磁通观测值＝低通滤波观测值+补偿值
   	if(0 == gVCPar.SvcMode)  //使用原380算法，以便于和320兼容
    {
         m_ABFluxS.Alph = (long)gABFluxS_LowFilter.Alph; // - ((long)gABFluxS_comp.Alph * (long)gTestDataReceive.TestData2)/100L;
         m_ABFluxS.Beta = (long)gABFluxS_LowFilter.Beta; // - ((long)gABFluxS_comp.Beta * (long)gTestDataReceive.TestData2)/100L;
    }
    else  //SVC1   1 == gVCPar.SvcMode  
    {
        //定子磁通观测值补偿：用给定磁通对低通滤波观测值补偿
       	// 计算αβ轴上的给定转子磁通:DQ->AlphBeta角度准备  
    	    m_Angle = (int)(((long)gPhase.IMPhase>>16)+((long)gPhase.StepPhaseApply>>16));
    	    //m_Angle = (int)((long)gFluxR.Theta+((long)gPhase.StepPhaseApply>>16));
			
    	m_Sin  = qsin(m_Angle);                 //Q15
    	m_Cos  = qsin(16384 - m_Angle);         //Q15

        //计算转子磁通在静止坐标系下的给定值：r/s 变换
    	m_ABFluxR_cmd.Alph = ((((long)m_Cos * ((long)gIMTSetApply.M>>12))>>11)
    	                    * ((long)gMotorExtPer.LM*5000L/(long)gBasePar.FullFreq01))>>11;
    	//m_ABFluxR_cmd.Alph(Q14) = ((((long)m_Cos(Q15) * ((long)gIMTSet.M(Q24)>>12)>>11))
    	//                    * (long)gMotorExtPer.LM(Q9))>>11;
    	m_ABFluxR_cmd.Beta = ((((long)m_Sin * ((long)gIMTSetApply.M>>12))>>11)
    	                    * ((long)gMotorExtPer.LM*5000L/(long)gBasePar.FullFreq01))>>11;
    	//m_ABFluxR_cmd.Beta(Q13) = ((((long)m_Sin(Q15) * ((long)gIMTSet.M(Q24)>>12)>>11))
    	//                    * (long)gMotorExtPer.LM(Q9))>>11;

    	//计算定子电流在静止坐标系下的给定值：r/s变换
        m_IAlphBeta_cmd.Alph = (((long)gIMTSetApply.M>>12) * (long)m_Cos
                              - ((long)gIMTSetApply.T>>12) * (long)m_Sin)>>15;
        //m_IAlphBeta_cmd.Alph(Q12) = (((long)gIMTSetApply.M(Q24)>>12) * (long)m_cos(Q15)
        //                      - ((long)gIMTSetApply.T(Q24)>>12) * (long)m_sin(Q15))>>15;
        m_IAlphBeta_cmd.Beta = (((long)gIMTSetApply.M>>12) * (long)m_Sin
                              + ((long)gIMTSetApply.T>>12) * (long)m_Cos)>>15;
        //m_IAlphBeta_cmd.Beta(Q12) = (((long)gIMTSetApply.M(Q24)>>12) * (long)m_sin(Q15)
        //                      + ((long)gIMTSetApply.T(Q24)>>12) * (long)m_cos(Q15))>>15;

    	//计算定子磁通在静止坐标系下的给定值：转子磁通加漏磁通后折算
    	m_Long = ((long)m_LLou * (long)m_IAlphBeta_cmd.Alph)>>12;
    	//m_Long(Q14) = ((long)m_LLou(Q14) * (long)m_IAlphBeta_cmd.Alph(Q12))>>12;
    	m_Long = (long)m_ABFluxR_cmd.Alph + m_Long;
    	//m_Long(Q14) = (long)m_ABFluxR_cmd.Alph(Q14) + m_Long(Q14);
    	m_Long = (long)m_Long * (long)gMotorExtPer.LM;
    	//m_Long(Q23) = (long)m_Long(Q14) * (long)gMotorExtPer.LM(Q9)
        m_ABFluxS_cmd.Alph = (long)m_Long / (long)gMotorExtPer.L1;
        //m_ABFluxS_cmd.Alph(Q14) = (long)m_Long(Q23) / (long)gMotorExtPer.L1(Q9);

    	m_Long = ((long)m_LLou * (long)m_IAlphBeta_cmd.Beta)>>12;
    	//m_Long(Q14) = ((long)m_LLou(Q14) * (long)m_IAlphBeta_cmd.Beta(Q12))>>12; 
    	m_Long = (long)m_ABFluxR_cmd.Beta + m_Long;
    	//m_Long(Q14) = (long)m_ABFluxR_cmd.Beta(Q14) + m_Long(Q14);
    	m_Long = (long)m_Long * (long)gMotorExtPer.LM;
    	//m_Long(Q23) = (long)m_Long(Q14) * (long)gMotorExtPer.LM(Q9)
        m_ABFluxS_cmd.Beta = (long)m_Long / (long)gMotorExtPer.L1;
        //m_ABFluxS_cmd.Beta(Q14) = (long)m_Long(Q23) / (long)gMotorExtPer.L1(Q9);

    	//对静止坐标系定子磁通给定值做高通滤波，计算补偿值
    	m_Long = (long)m_ABFluxS_cmd.Alph - (long)gABFluxS_comp.Alph;
    	//m_Long(Q14) = m_ABFluxS_cmd.Alph(Q14) - gABFluxS_comp.Alph(Q14);
    	m_Long = ((long)m_Long * (long)gCalFlux.SampleTime)>>12;
    	//m_Long(Q15) = (m_Long(Q14) * gCalFlux.SampleTime(Q13))>>12;
        gABFluxS_comp.Alph += ((long)m_Long * (long)gCalFlux.FilterTime)>>15;
        //gABFluxS_comp.Alph(Q14) += (m_Long(Q15) * gCalFlux.FilterTime(Q14))>>15;

    	m_Long = (long)m_ABFluxS_cmd.Beta - (long)gABFluxS_comp.Beta;
    	//m_Long(Q14) = m_ABFluxS_cmd.Beta(Q14) - gABFluxS_comp.Beta(Q14);
    	m_Long = ((long)m_Long * (long)gCalFlux.SampleTime)>>12;
    	//m_Long(Q15) = (m_Long(Q14) * gCalFlux.SampleTime(Q13))>>12;
        gABFluxS_comp.Beta += ((long)m_Long * (long)gCalFlux.FilterTime)>>15;
        //gABFluxS_comp.Beta(Q14) += (m_Long(Q15) * gCalFlux.FilterTime(Q14))>>15;

         m_ABFluxS.Alph = (long)gABFluxS_LowFilter.Alph + ((long)gABFluxS_comp.Alph * (long)gTestDataReceive.TestData2)/100L;
         m_ABFluxS.Beta = (long)gABFluxS_LowFilter.Beta + ((long)gABFluxS_comp.Beta * (long)gTestDataReceive.TestData2)/100L;
    }

	m_Long = ((long)gIAlphBetaQ12.Alph * (long)m_LLou)<<2;
	//m_Long(Q28) = ((llong)gIAlphBetaQ12.Alph(Q12) * (llong)m_LLou(Q14))<<2;
	m_Long = ((long)m_ABFluxS.Alph * (long)m_LrDivLm) - (long)m_Long;
	//m_Long(Q28) = ((long)m_ABFluxS.Alph(Q14) * (long)m_LrDivLm(Q14)) - m_Long(Q28);
	m_ABFluxR.Alph = m_Long>>14;
	//m_ABFluxR.Alph(Q14) = m_Long(Q28)>>14;

	m_Long = ((long)gIAlphBetaQ12.Beta * (long)m_LLou)<<2;	
	//m_Long(Q28) = ((llong)gIAlphBetaQ12.Beta(Q12) * (llong)m_LLou(Q14))<<2;	
	m_Long = ((long)m_ABFluxS.Beta * (long)m_LrDivLm) - m_Long;
	//m_Long(Q28) = ((long)m_ABFluxS.Beta(Q14) * (long)m_LrDivLm(q14)) - m_Long(q28);
	m_ABFluxR.Beta = m_Long>>14;
	//m_ABFluxR.Beta(q13) = m_Long(q28)>>14;



	m_ULong = (((long)m_ABFluxR.Alph * (long)m_ABFluxR.Alph) + 
	          ((long)m_ABFluxR.Beta * (long)m_ABFluxR.Beta));	
	//tempL = (((long)m_ABFluxR.Alph(Q14) * (long)m_ABFluxR.Alph(Q14)) + 
	//          ((long)m_ABFluxR.Beta(Q14) * (long)m_ABFluxR.Beta(Q14)));	
	gFluxR.Amp = (Ulong)qsqrt(m_ULong);			//计算输出缪狗?
	//gFluxSAmp(Q14) = (Ulong)qsqrt(tempL)Q28));			//计算输出缪狗?
    gFluxR.Theta = atan(m_ABFluxR.Alph,m_ABFluxR.Beta);//计算磁通矢量角度

	m_ULong = (((long)gABFluxS_LowFilter.Alph * (long)gABFluxS_LowFilter.Alph) + 
	          ((long)gABFluxS_LowFilter.Beta * (long)gABFluxS_LowFilter.Beta));	
	//tempL = (((long)gABFluxR.Alph(Q14) * (long)gABFluxR.Alph(Q14)) + 
	//          ((long)gABFluxR.Beta(Q14) * (long)gABFluxR.Beta(Q14)));	
	gFluxS.Amp = (Ulong)qsqrt(m_ULong);			//计算输出缪狗?
	//gFluxSAmp(Q14) = (Ulong)qsqrt(tempL)Q28));			//计算输出缪狗?
    gFluxS.Theta = atan(gABFluxS_LowFilter.Alph,gABFluxS_LowFilter.Beta);//计算磁通矢量角度

    gSVCSpeed.DetaTimer += m_DetaTimer;

}

/*************************************************************
	开环矢量下辨识转子速度
*************************************************************/
void SVCCalRotorSpeed(void)
{
	int     m_DetaTimer;
	int    m_Speed;
	int     m_LowVolt;
	long		m_Long;  

	DINT;  
    m_DetaTimer = gSVCSpeed.DetaTimer;
    gSVCSpeed.DetaPhase = gFluxR.Theta - gSVCSpeed.SvcLastFluxPos;
	gSVCSpeed.SvcLastFluxPos = gFluxR.Theta;
    gSVCSpeed.DetaTimer = 0; 
	EINT;
 
	if(gMainCmd.FirstCnt < 10)		//启动需要特殊处理 = 30*500us = 15.0ms
	{
		gMainCmd.FirstCnt +=1;
		gSVCSpeed.SvcRotorSpeed = 0;
		gSVCSpeed.SvcSynSpeed	= 0;
		return;
	}

	//通过转子磁通矢量的角度计算转子磁通速度（同步速度）
	m_Speed = ((Ulong)abs(gSVCSpeed.DetaPhase) * 25000L) / gBasePar.FullFreq01;
    if(gSVCSpeed.DetaPhase < 0)
	{
		m_Speed = -m_Speed;
	}
	m_Speed = ((long)m_Speed * (DSP_CLOCK*250L))/m_DetaTimer;//修正2ms时间
	gSVCSpeed.SvcSynSpeed = Filter16((int)m_Speed, gSVCSpeed.SvcSynSpeed) ;

	m_LowVolt = 300;			//低速需要特殊处理  //18V
	if(gInvInfo.InvTypeApply >= 22)
    {
        m_LowVolt = 80;     // 7V
    }
	if(gRatio < m_LowVolt)
    {
        gSVCSpeed.SvcSynSpeed = gMainCmd.FreqSyn;
    }
    if(gMotorExtPer.I0 > (gIMTSetApply.M>>12))
	{
	    m_Long = ((long)gMotorExtPer.R2<<11)/gMotorExtPer.I0;	//计算转差频率
	}
    else
	{
	    m_Long = ((long)gMotorExtPer.R2<<11)/(gIMTSetApply.M>>12);	//计算转差频率
	}
	//UData = ((long)gMotorExtPer.R2<<11)/gMotorExtPer.I0;	//计算转差频率
	m_Long = ((long)m_Long<<11)/gMotorExtPer.L1;
	m_Long =  ((long)m_Long * (long)gIMTQ12.T)>>14;          // 这个地方用的是实际的T轴电流
	gSVCSpeed.SvcWs = ((long)m_Long * (long)gVCPar.VCWsCoff)/100;
	//计算转子速度
	m_Speed = gSVCSpeed.SvcSynSpeed - gSVCSpeed.SvcWs;
        if(m_Speed==0)
        {
            gSVCSpeed.SvcRotorSpeed = m_Speed;
    	    //return;
        }
        if(gSVCSpeed.SvcSignal==0)
        {
            if(gMainCmd.FreqSyn==0)	
            {
                gSVCSpeed.SvcRotorSpeed = 0;
    	        //return;
            }
            else
            {
                gSVCSpeed.SvcSignal=1;
            }
        }
     
       if(((long)gMainCmd.FreqSet * (long)m_Speed) <0)
        {
            m_Speed = 0;
        }
	    //gSVCSpeed.SvcRotorSpeed = m_Speed;
    	if(gVCPar.VCSpeedFilter <= 1)
    	{
    		gSVCSpeed.SvcRotorSpeed = m_Speed;	
    	}
    	else                        //F2-07的滤波处理
    	{
        	m_Speed =  (long)gSVCSpeed.SvcRotorSpeed * (gVCPar.VCSpeedFilter-1L) + 2L * m_Speed;
        	gSVCSpeed.SvcRotorSpeed = m_Speed / (gVCPar.VCSpeedFilter + 1L);
    	}
        gMainCmd.FreqFeed = gSVCSpeed.SvcRotorSpeed;
	}


void SVCCalRotorSpeed_380(void)
{
	Ulong    m_ULong;  //add by jxl
	long    m_Long;  //add by jxl
	int     m_IntData;
//	int     m_DetaPos;
	long    m_Speed;
//	int     m_LowVolt;
//	int		m_UData;
    //int     temp;
        //注：此滤波大小决定了动态响应速度，但目前为保证弱磁区的稳定性，需大滤波，因此采用了逐渐增加滤波的方式
	    //在需要急加速的应用场合，可合理减小滤波
        //后期需采用新的滤波算法
        m_IntData = (gSVCSpeed.SvcSynSpeed < gMainCmd.FreqSyn) ? 1 : -1;
        m_IntData = (gSVCSpeed.SvcSynSpeed == gMainCmd.FreqSyn) ? 0 : m_IntData;
        //temp2 = 1;  //消滤波静差
        m_ULong = (((Ulong)gMotorInfo.Frequency<<15)/gBasePar.FullFreq01)>>1;
	    //取0.5倍的额定频率
        m_ULong = ((Ulong)m_ULong<<7)/(abs(gMainCmd.FreqSyn));
	    if(m_ULong>1024) 
    	{
	        m_ULong = 1024;
    	}
        gSVCSpeed.SvcSynSpeed=(((long)gSVCSpeed.SvcSynSpeed*(1024-m_ULong)+(long)gMainCmd.FreqSyn*m_ULong)>>10) + m_IntData;
	    //gSVCSpeed.SvcSynSpeed = Filter16((int)gMainCmd.FreqSyn, gSVCSpeed.SvcSynSpeed) + m_IntData ;

   if(gMainCmd.FirstCnt < 10)		//启动需要特殊处理 = 50*500us = 25.0ms
	{
		gMainCmd.FirstCnt +=1;
		gSVCSpeed.SvcRotorSpeed = 0;
		gSVCSpeed.SvcSynSpeed	= 0;
		return;
	}
	else if(gMainCmd.FirstCnt < 1000)  //SVC1 为起重程序保留，以预留磁场建立时间
	{
		gMainCmd.FirstCnt +=1;
	}
	else
	{
		gMainCmd.FirstCnt = 1000;

	}

    //用观测同步角分解得到的转矩电流和电机空载电流计算滑差
    //受低频磁通观测影响，引入了零漂造成的波动；加大磁通观测的时间常数可以压制零漂，但稳态精度受损！
    //对低频发电状态不利！

    m_Long = ((long)gMotorExtPer.R2<<11)/(long)((gIMTSet.M>>12));	//计算转差频率
    //m_Long = ((long)gMotorExtPer.R2<<11)/(long)((gIMTQ24_obs.M>>12));	//计算转差频率
    //m_UData(Q15) = ((Ulong)gMotorExtPer.R2(Q16)<<11)/gMotorExtPer.I0(Q12);	//计算转差频率
    m_Long = ((long)m_Long<<11)/(long)gMotorExtPer.L1;
    //m_UData(Q17) = ((long)m_UData(Q15)<<11)/gMotorExtPer.L1(Q9);
    m_Long =  ((long)m_Long * (long)(gIMTQ24_obs.T>>12))>>14;          // 这个地方用的是实际的T轴电流
    //m_UData(Q15) =  ((long)m_UData(Q17) * (long)gIMTQ12.T(Q12))>>14;          // 这个地方用的是实际的T轴电流

	gSVCSpeed.SvcWs = ((long)m_Long * (long)gVCPar.VCWsCoff)/100;
	     

	//计算转子速度
	//原380代码：估计转子转速=估计同步速-估计滑差；
	//新代码（Jxl）：估计转子转速＝给定同步速-估计滑差；
	gSVCSpeed.SvcRotorSpeed = gSVCSpeed.SvcSynSpeed - gSVCSpeed.SvcWs;

        if(gSVCSpeed.SvcSignal==0)
        {
            if(gMainCmd.FreqSyn==0)	
            {
                gSVCSpeed.SvcRotorSpeed = 0;
    	        //return;
            }
            else
            {
                gSVCSpeed.SvcSignal=1;
            }
        }
    
   	if(gVCPar.VCSpeedFilter <= 1)  //无转速滤波
	{
		gSVCSpeed.SvcRotorSpeed = gSVCSpeed.SvcRotorSpeed;	
	}
	else                        //F2-07的滤波处理
	{
       	m_Speed =  (long)gSVCSpeed.SvcRotorSpeed * (gVCPar.VCSpeedFilter-1L) + 2L * m_Speed;
       	gSVCSpeed.SvcRotorSpeed = m_Speed / (gVCPar.VCSpeedFilter + 1L);
	}
    gMainCmd.FreqFeed = gSVCSpeed.SvcRotorSpeed;
 }


