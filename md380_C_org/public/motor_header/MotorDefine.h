/************************************************************
------------------该文件是主程序的头文件---------------------
定义性能部分使用的宏和常量(2908 和28035 是共用的)
************************************************************/
#ifndef MAIN_INCLUDE_H
#define MAIN_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorDefine.h"

/*----------------------------------------------------------------------
---------------------------Data typr show-------------------------------
Type 				Size 		Representation 		Minimum 		Maximum

char, signed char 	16 bits 	ASCII 				-32768 			32767
unsigned char 		16 bits		ASCII 				0 				65535
short 				16 bits 	2s complement 		-32768 			32767
unsigned short 		16 bits 	Binary 				0 				65535
int, signed int 	16 bits 	2s complement 		-32768 			32767
unsigned int 		16 bits 	Binary 				0 				65535
long, signed long 	32 bits 	2s complement 		-2147483648 	214783647
unsigned long 		32 bits 	Binary 				0 				4294967295
long long, 
signed long long	64 bits 	2s complement 		-9223372036854775808 
																	9223372036854775807
unsigned long long 	64 bits 	Binary 				0 				18446744073709551615
enum 				16 bits 	2s complement 		-32768 			32767
pointers 			16 bits 	Binary 				0 				0xFFFF
far pointers 		22 bits 	Binary 				0 				0x3FFFFF
-----------------------------------------------------------------------*/
/************************************************************
	工程使用的新定义变量类型
************************************************************/
typedef	long long 				llong;
typedef	unsigned int			Uint;
typedef	unsigned long			Ulong;
typedef	unsigned long long 		Ullong;

typedef struct BIT32_REG_DEF {
   Uint16  LSW;
   Uint16  MSW;
}BIT32_REG;

typedef union BIT32_GROUP_DEF {
   Uint32     all;
   BIT32_REG  half;
}BIT32_GROUP;

/************************************************************
	以下为调试代码临时定义的变量
************************************************************/
//#define  C_CMD_OK			   0
//#define  C_CMD_CHECKFALSE	   1
//#define  C_CMD_TIMEOUT	   2
#define  C_CMD_WRITE	0		//和PC机的通讯命令
#define  C_CMD_READ		1
#define  C_CMD_DEBUG	2
#define  C_COM_OVER_TIME	   100      //和PC机通讯超时时间 2ms单位


/************************************************************
	基本函数定义和引用
************************************************************/
#define  GetTime() 	(CpuTimer1.RegsAddr->TIM.all)

/************************************************************
	常数定义
************************************************************/
//电机类型
#define MOTOR_TYPE_IM			0		//感应电机
#define MOTOR_TYPE_VARFREQ      1       //变频异步电机
#define MOTOR_TYPE_PM			2		//永磁同步电机
#define MOTOR_NONE              100
//控制方式
#define IDC_SVC_CTL				0		//SVC
#define IDC_FVC_CTL				1		//FVC
#define IDC_VF_CTL				2		//VF
//变频器电压等级
#define INV_VOLTAGE_220V        0   
#define INV_VOLTAGE_380V        1
#define INV_VOLTAGE_480V        2
#define INV_VOLTAGE_690V        3
#define INV_VOLTAGE_1140V       5
//变频器主状态
#define STATUS_LOW_POWER		1		//欠压状态
#define STATUS_GET_PAR			2		//静态参数辨识阶段
#define STATUS_STOP				3		//停机状态
#define STATUS_SPEED_CHECK		4		//转速跟踪阶段
#define STATUS_RUN				5		//运行状态, (包括直流制动阶段)
#define STATUS_SHORT_GND		6		//对地短路检测阶段
#define STATUS_IPM_INIT_POS		7		//同步机识别磁极初始位置角阶段

//加减速标志
#define C_SPEED_FLAG_CON 		0		//恒速标志
#define C_SPEED_FLAG_ACC 		1		//加速标志
#define C_SPEED_FLAG_DEC 		2		//减速标志
// 辅助宏
#define speed_ACC (gMainCmd.Command.bit.SpeedFlag == C_SPEED_FLAG_ACC)
#define speed_DEC (gMainCmd.Command.bit.SpeedFlag == C_SPEED_FLAG_DEC)
#define speed_CON (gMainCmd.Command.bit.SpeedFlag == C_SPEED_FLAG_CON)

//连续/离散调制
#define  MODLE_CPWM				0		//连续调制
#define  MODLE_DPWM				1		//离散调制
//同步调制/异步调制
#define  MODLE_SYN				0		//同步调制
#define  MODLE_ASYN				1		//异步调制

#define  CANCEL_DB_COMP_NO      0       // 不补偿
#define  DEADBAND_COMP_280      1       //当前280采用的死区补偿方式, AD中断中计算
#define  DEADBAND_COMP_380      2      //优化的380死区补偿方式


#define  IDENTIFY_PROGRESS_LENGTH 5     //当前参数辨识最多4步
//和载波频率相关时间定义    
#ifdef	DSP_CLOCK100
	#define C_INIT_PRD			10000	//初始(5KHz)的PWM周期（PWM定时器时间为10ns）
	#define C_MAX_DB			320		//初始死区大小3.2us
	#define SHORT_GND_PERIOD 	12500	//上电对地短路检测时候的载波周期
    #define TUNE_Rs_PRD         25000  //380V电压登等级定子电阻参数辨识载频2K
    #define SHORT_GND_CMPR_INC	100     // 1us对应计数器值
	#define SHORT_GND_PERIOD_1140 	50000
    #define SHORT_GND_PERIOD_690    33000
#else
	#define C_INIT_PRD			6000	//初始(5KHz)的PWM周期（PWM定时器时间为16.7ns）
	#define C_MAX_DB			192		//初始死区大小3.2us
	#define SHORT_GND_PERIOD 	7500
    #define TUNE_Rs_PRD         15000  //380V电压登等级定子电阻参数辨识载频2K
	#define SHORT_GND_CMPR_INC	60
    #define SHORT_GND_PERIOD_1140 	30000
    #define SHORT_GND_PERIOD_690    20000
#endif
#define C_MAX_LONG_VALUE		(0x7FFFFFFF)	//长整型数据的最大值
#define C_MAX_PER               (1073741824L)   //Q30

#define  COM_PHASE_DEADTIME		600	   //电流极性判断的超前角度

/************************************************************
	 定义DSP的ADC输入口、GPIO口输出口、GPIO输入口
************************************************************/
#define TurnOnFan()    		gMainStatus.StatusWord.bit.FanControl = 1						  
#define TurnOffFan()     	    gMainStatus.StatusWord.bit.FanControl = 0	//风扇控制

#define TurnOnBrake()    		GpioDataRegs.GPACLEAR.bit.GPIO8 = 1					  
#define TurnOffBrake()     	GpioDataRegs.GPASET.bit.GPIO8 = 1	        //制动电阻控制


#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================
