/***************************************************************
文件功能：模拟量采样
文件版本：
最新更新：
************************************************************/
#ifndef MOTORINFO_COLLECT_INCLUDE_H
#define  MOTORINFO_COLLECT_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "MotorInclude.h"

// // 结构体定义 
typedef struct CUR_EXCURSION_STRUCT_DEF{
	long	TotalIu;
	long	TotalIv;
	int		Iu;                             // 去零漂前
	int		Iv;
    long    IuValue;                        //用于参数辨识
    long    IvValue;                        //用于参数辨识
	int		ErrIu;				            //U相零漂大小
	int		ErrIv;				            //V相零漂大小
	int  	Count;
	int  	EnableCount;
	int  	ErrCnt;
}CUR_EXCURSION_STRUCT;                  //检测零漂使用的结构

typedef struct TEMPLETURE_STRUCT_DEF{
	Uint	TempAD;				            //AD获取值，由于温度查表
	Uint	Temp;				            //用度表示的实际温度值
	Uint	TempBak;			            //用度表示的实际温度值
	Uint	ErrCnt;
}TEMPLETURE_STRUCT;                     //和变频器温度相关的数据结构

typedef struct AI_STRUCT_DEF {
	Uint 	gAI1;
	Uint 	gAI2;
    Uint    gAI3;

    Ulong   ai1Total;
    Ulong   ai2Total;
    Ulong   ai3Total;
    int     aiCounter;
}AI_STRUCT;	//

// // 供外部引用变量声明 
extern I_STRUCT				    ISamp;
extern ADC_STRUCT				gADC;		//ADC数据采集结构
extern UDC_STRUCT				gUDC;		//母线电压数据
extern IUVW_SAMPLING_STRUCT	    gCurSamp;
extern UVW_STRUCT				gIUVWQ12;	//定子三相坐标轴电流
extern UVW_STRUCT_Q24           gIUVWQ24;   //Q24格式的三相定子电流
extern ALPHABETA_STRUCT		    gIAlphBeta;	//定子两相坐标轴电流
extern ALPHABETA_STRUCT		    gIAlphBetaQ12;	//定子两相坐标轴电流
extern MT_STRUCT				gIMTQ12;    //MT轴系下的电流
extern MT_STRUCT                gIMTSetQ12;
extern MT_STRUCT_Q24            gIMTQ24;
extern AMPTHETA_STRUCT			gIAmpTheta;	//极坐标表示的电流
extern LINE_CURRENT_STRUCT		gLineCur;	
extern CUR_EXCURSION_STRUCT	    gExcursionInfo;//检测零漂使用的结构
extern TEMPLETURE_STRUCT		gTemperature;
extern AI_STRUCT				gAI;

extern PID_STRUCT               gWspid;

// // 供外部引用函数声明 
void GetCurExcursion(void);
void GetUDCInfo(void);
void ADCProcess(void);
void GetCurrentInfo(void);

#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================


