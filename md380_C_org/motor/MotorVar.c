/****************************************************************
文件功能：和电机控制相关的公共变量定义
文件版本： 
最新更新： 

****************************************************************/

#include "MotorInclude.h"

// 以下为基本变量定义
INV_STRUCT 				    gInvInfo;		//变频器信息
MOTOR_STRUCT 			    gMotorInfo;	    //电机信息
MOTOR_EXTERN_STRUCT		    gMotorExtInfo;	//电机扩展信息（实际值表示）
MOTOR_EXTERN_STRUCT		    gMotorExtPer;	//电机扩展信息（标么值表示）
RUN_STATUS_STRUCT 		    gMainStatus;	//主运行状态
BASE_COMMAND_STRUCT		    gMainCmd;		//主命令
MAIN_COMMAND_EXTEND_UNION   gExtendCmd;     //主命令字扩展
SUB_COMMAND_UNION           gSubCommand;	//辅命令字结构

CONTROL_MOTOR_TYPE_ENUM     gCtrMotorType;  //电机类型和控制模式的组合
MOTOR_POWER_TORQUE          gPowerTrq;      // 变频器输出功率和电机输出转矩


// 以下为和电机控制相关设定参数定义
BASE_PAR_STRUCT			    gBasePar;	    //基本运行参数
COM_PAR_INFO_STRUCT		    gComPar;	    //公共参数

// 其它参数定义
CPU_TIME_STRUCT			    gCpuTime;

// 驱动部分调试用变量
MOTOR_DEBUG_DATA_RECEIVE_STRUCT     gTestDataReceive;//预留的用于驱动部分调试的数据
MOTOR_DEBUG_DATA_DISPLAY_STRUCT     gTestDataDisplay;//预留的用于显示驱动部分调试数据


