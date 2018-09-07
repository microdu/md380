/***************************************************************
文件功能：和电机,变频器保护相关的数据结构定义，变量申明
文件版本：
最新更新：
************************************************************/
#ifndef MOTORINV_PROTECT_INCLUDE_H
#define MOTORINV_PROTECT_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "MotorInclude.h"
#include "SystemDefine.h"
#include "MotorDefine.h"

/******************故障代码定义*************************///
//故障代码
#define  ERROR_OVER_CURRENT         (1L<< (1-1))      // 1 过流-2
#define  ERROR_OVER_UDC             (1L<< (2-1))      // 2 过压-5
#define  ERROR_RESISTER_HOT         (1L<< (3-1))      // 3 缓冲电阻过热-8
#define  ERROR_LOW_UDC              (1L<< (4-1))      // 4 欠压-9
#define  ERROR_INV_OVER_LAOD        (1L<< (5-1))      // 5 变频器过载-10

#define  ERROR_MOTOR_OVER_LOAD      (1L<< (6-1))      // 6 电机过载-11
#define  ERROR_INPUT_LACK_PHASE     (1L<< (7-1))      // 7 输入缺相-12
#define  ERROR_OUTPUT_LACK_PHASE    (1L<< (8-1))      // 8 输出缺相-13
#define  ERROR_INV_TEMPERTURE       (1L<< (9-1))      // 9 变频器过热-14
#define  ERROR_RESISTANCE_CONTACK   (1L<< (10-1))     // 10 缓冲电阻吸合故障-17

#define  ERROR_CURRENT_CHECK        (1L<< (11-1))     // 11 电流零飘检测错误-18
#define  ERROR_SHORT_EARTH          (1L<< (12-1))     // 12 对地短路检测故障-23
#define  ERROR_LOAD_LOST            (1L<< (13-1))     // 13 输出掉载－30
#define  ERROR_TRIP_ZONE            (1L<< (14-1))     // 14 逐波限流故障-40
#define  ERROR_INIT_POS			    (1L<< (15-1)) 	  // 15 初始位置不吻合故障-92
                                                      //    同步机磁极位置判断失败
#define  ERROR_SPEED_LOSE           (1L<< (16-1))     // 16 同步机编码器零点位置角辨识失败94(取消)
#define  ERROR_PROGRAM_LOGIC        (1L<< (17-1))     // 17 程序内部逻辑错位-99
#define  ERROR_TUNE_FAIL            (1L<< (18-1))     // 18 调谐失败 - ??(19)

#define  ERROR_ENCODER              (1L<< (19-1))     // 19 信号编码器故障

/*
ERROR_ENCODER 反馈信息说明 
infor = :
1	// （空载辨识） ab信号方向辨识出错；
2	// （空载辨识） uvw信号方向辨识出错；
3	// （空载辨识） 旋变信号方向辨识出错；
4	// （空载辨识） 没有侦测到Z信号（或旋变零点位置）；

5	// （带载辨识） ab信号方向辨识出错；
6	// （带载辨识） uvw信号方向辨识出错；
7	// （空载辨识） 旋变信号方向辨识出错；

9	异步机辨识编码器未接；
10	异步机辨识编码器线数错误；

11      uvw信号反短线；
12	uvw逻辑错误（编码器或者pg卡损坏）；
*/

/************************************************************/
/***********************结构体定义***************************/
struct FAULT_INFOR_MODE_STRUCT_DEF{
    Uint    Fault1:4;                       //故障信息只能设定为0－15之间的数
    Uint    Fault2:4;
    Uint    Fault3:4;
    Uint    Fault4:4;
}; //定义各种故障的停机处理方式
typedef union FAULT_INFOR_MODE_UNION_DEF{
    Uint    all;
    struct  FAULT_INFOR_MODE_STRUCT_DEF bit;
}FAULT_STOP_MODE_UNION;
struct INV_ERROR_CODE_STRUCT{
    Uint    ErrorCode1;
    Uint    ErrorCode2;
};
union FAULT_CODE_UNION_DEF{
    Uint32  all;
    struct INV_ERROR_CODE_STRUCT  ErrorCodeStruct;
};
struct FAULT_CODE_INFOR_STRUCT_DEF{
    union FAULT_CODE_UNION_DEF ErrorCode;       //故障代码.故障信息存放方式参见故障定义，按bit位存放
    FAULT_STOP_MODE_UNION       ErrorInfo[5];    //故障信息.用于帮助定位故障位置.每一故障的故障信息由4个bit位组成.存放顺序同
                                                 //故障代码中的故障排列顺序一致。
	Uint32                      LastErrorCode;   //上次故障代码，用于输出电压相位切换    
};
typedef struct OVER_LOAD_PROTECT_DEF{
   int		Cnt;
   long		FilterRealFreq;
   int		FilterInvCur;
   int		FilterMotorCur;
   BIT32_GROUP		InvTotal;
   BIT32_GROUP		MotorTotal;
}OVER_LOAD_PROTECT; 	//过载保护数据结构
typedef struct PHASE_LOSE_STRUCT_DEF{
   Ulong	Time;
   Ulong	TotalU;
   Ulong	TotalV;
   Ulong	TotalW;
   Uint		Cnt;
   Uint     errMaxCur;
   Uint     errMinCur;
}PHASE_LOSE_STRUCT; 	//输出缺相判断程序
typedef struct INPUT_LOSE_STRUCT_DEF{
   Uint		Cnt;
   Uint		UpCnt;          // PL高电平计数器
   Uint		ErrCnt;
   Uint		CntRes;         // 充当时间计数器
   Uint		UpCntRes;
   Uint     ForeInvType;    //开始缺相保护的初始机型
}INPUT_LOSE_STRUCT; 	//输入缺相判断程序
typedef struct LOAD_LOSE_STRUCT_DEF {
	Uint 	ErrCnt;
    Uint    ChkLevel;       // 掉载检测水平
    Uint    ChkTime;        // 掉载检出时间
}LOAD_LOSE_STRUCT;	//输出掉载检测数据结构
typedef struct FAN_CTRL_STRUCT_DEF{
   Uint		EnableCnt;	//判断上电后至少1秒才能够开始启动
   Uint		RunCnt;		//判断运行时间至少10秒
}FAN_CTRL_STRUCT; 	//风扇控制程序使用结构变量
struct CBC_FLAG_STRUCT{
    Uint16  CBC_U:1;
    Uint16  CBC_V:1;
    Uint16  CBC_W:1;
    Uint16  RESV:13;
};
typedef union CBC_FLAG_UNION_DEF {
   Uint16                  all;
   struct CBC_FLAG_STRUCT  bit;
}CBC_FLAG_UNION;
typedef struct CBC_PROTECT_STRUCT_DEF{
   CBC_FLAG_UNION	Flag;	//BIT0/1/2为1分别表示UVW三相处于逐波限流状态
   int		TotalU;
   int		TotalV;
   int     	TotalW;
   int     	Total;
   int		EnableFlag;
   int		CntU;		//电流大于1.6倍峰值电流后的累加
   int		CntV;
   int     	CntW;

   Uint     maxCBCTime;     // 60deg温度对应的CBC时间(单管)
   Uint     minCBCTime;     // 40deg温度对应的CBC时间(单管)
}CBC_PROTECT_STRUCT; 	//逐波限流保护数据结构

/************************************************************/
/*******************供外部引用变量声明***********************/
extern OVER_LOAD_PROTECT		gOverLoad;
extern PHASE_LOSE_STRUCT		gPhaseLose;
extern INPUT_LOSE_STRUCT		gInLose;
extern LOAD_LOSE_STRUCT		    gLoadLose;
extern FAN_CTRL_STRUCT			gFanCtrl;
extern Ulong					gBuffResCnt;	//缓冲电阻保护变量
extern CBC_PROTECT_STRUCT		gCBCProtect;
/************************************************************/
/*******************供外部引用函数声明***********************/
void InvDeviceControl(void);
void OutputLoseAdd(void);
extern struct FAULT_CODE_INFOR_STRUCT_DEF  gError;
#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================



