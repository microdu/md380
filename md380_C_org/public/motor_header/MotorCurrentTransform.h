/***************************************************************
文件功能：电流转换使用的结构声明
文件版本：
最新更新：
************************************************************/
#ifndef MOTOR_CURRENT_INCLUDE_H
#define  MOTOR_CURRENT_INCLUDE_H
#ifdef  __cplusplus
extern "C" {
#endif

//********************************************************
#include "SystemDefine.h"
#include "MotorDefine.h"

typedef struct I_STRUCT_DEF{       /*chzq*/
	Uint16     iuPrd;      
	Uint16     ivPrd;
    Uint16 	   iuZero;   
    Uint16     ivZero; 
    Uint16     iuZeroOld;
    Uint16     ivZeroOld;
    Uint16     iuAver;      //long
    Uint16     ivAver;      //long
    Uint16     iuApply;
    Uint16     ivApply;
}I_STRUCT;	//周期中断与下溢中断的采样数据结构
typedef struct MT_STRUCT_DEF{
	int  	M;      // Q12
	int  	T;
}MT_STRUCT;	//MT轴系下的电流、电压结构
typedef struct MT_STRUCT_Q24_DEF{
    long    M;
    long    T;
}MT_STRUCT_Q24;
typedef struct ALPHABETA_STRUCT_DEF{
	long  	Alph;
	long  	Beta;
}ALPHABETA_STRUCT;//定子两相坐标轴电流、电压结构
typedef struct ADC_STRUCT_DEF{					
	Uint	DelaySet;			//ADC采样延时时间(0.1us单位)
	Uint	DelayApply;			//ADC采样延时时间(PWM定时器周期单位)
	//Uint  	ResetTime;			//ADC已经启动的次数
	long	ZeroTotal;
	int		ZeroCnt;
	int		Comp;
}ADC_STRUCT;	//定子三相坐标轴电流
typedef struct UDC_STRUCT_DEF {
	int		DetaUdc;
	Uint	uDCBak;
	Uint 	uDC;				//母线电压			单位0.1V
	Uint 	uDCFilter;			//大滤波母线电压	单位0.1V
	Uint 	uDCBigFilter;		//过压、欠压判断用母线电压	单位0.1V
	Uint 	Coff;				//母线电压计算系数
	Uint    uDCADCoff;          //AD采样值与实际值之间的系数
	Uint    uDcCalMax;          //计算最大电压使用的母线电压
}UDC_STRUCT;	//母线电压数据
typedef struct IUVW_SAMPLING_STRUCT_DEF{					
	long  	U;					//Q24格式，以电机额定电流为标么值基值
	long  	V;
	long  	W;
	long	UErr;				//U相毛刺滤波
	long 	VErr;				//V相毛刺滤波
	long    Coff;				//采样电流转换为标么值电流的系数
}IUVW_SAMPLING_STRUCT;	//电流采样结构
typedef struct UVW_STRUCT_DEF_Q24{					
	long  	U;					//Q24格式，以电机额定值为标么值基值
	long  	V;
	long  	W;
}UVW_STRUCT_Q24;	//定子三相坐标轴电流
typedef struct UVW_STRUCT_DEF{					
	int  	U;					//Q12格式，以电机额定值为标么值基值
	int  	V;
	int  	W;
}UVW_STRUCT;	//定子三相坐标轴电流

typedef struct LINE_CURRENT_STRUCT_DEF{
	Uint  	CurPer;				//Q12 以电机电流为基值表示的线电流有效值
	Uint  	CurBaseInv;			//Q12 以变频器电流为基值表示的线电流有效值
	Ulong  	CurPerFilter;		//Q12 以电机电流为基值表示的线电流有效值
	Uint  	CurPerShow;			//Q12 以电机电流为基值表示的线电流有效值
	int    CurTorque;          //Q12 以电机电流为基值表示的转矩电流
	Uint  	ErrorShow;			//过流时刻记录的线电流有效值

    Uint    Cur_Ft4;
}LINE_CURRENT_STRUCT;//计算过程中使用的线电流表示
typedef struct AMPTHETA_STRUCT_DEF{
	Uint  	Amp;				//Q12
	int  	Theta;				//Q15
	int     ThetaFilter;        //Q15 ----DBcomp
	int     PowerAngle;         // 功率因素角
	
    int     CurTmpM;
    int     CurTmpT;
}AMPTHETA_STRUCT;//极坐标表示的电流、电压结构

/************************************************************/
/*******************供外部引用函数声明***********************/
void AlphBetaToDQ(ALPHABETA_STRUCT* , int , MT_STRUCT_Q24* );


#ifdef __cplusplus
}
#endif  /* extern "C" */

#endif
/*===========================================================================*/
// End of file.
/*===========================================================================*/






