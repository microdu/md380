

#ifndef __F_ERROR_H__
#define __F_ERROR_H__

#include "f_funcCode.h"

// 故障信息
#define ERROR_NONE                      0       // 0  -- 无
#define ERROR_INVERTER_UNIT             1       // 1  -- 逆变单元保护
#define ERROR_OC_ACC_SPEED              2       // 2  -- 加速过电流
#define ERROR_OC_DEC_SPEED              3       // 3  -- 减速过电路
#define ERROR_OC_CONST_SPEED            4       // 4  -- 恒速过电流
#define ERROR_OV_ACC_SPEED              5       // 5  -- 加速过电压
#define ERROR_OV_DEC_SPEED              6       // 6  -- 减速过电压
#define ERROR_OV_CONST_SPEED            7       // 7  -- 恒速过电压
#define ERROR_BUFFER_RES                8       // 8  -- 缓冲电阻过载故障
#define ERROR_UV                        9       // 9  -- 欠压故障
#define ERROR_OL_INVERTER               10      // 10 -- 变频器过载
#define ERROR_OL_MOTOR                  11      // 11 -- 电机过载
#define ERROR_LOSE_PHASE_INPUT          12      // 12 -- 输入缺相
#define ERROR_LOSE_PHASE_OUTPUT         13      // 13 -- 输出缺相
#define ERROR_OT_IGBT                   14      // 14 -- 散热器过热
#define ERROR_EXTERNAL                  15      // 15 -- 外部故障
#define ERROR_COMM                      16      // 16 -- 通讯(超时)故障
#define ERROR_CONTACTOR                 17      // 17 -- 接触器故障
#define ERROR_CURRENT_SAMPLE            18      // 18 -- 电流检测故障
#define ERROR_TUNE                      19      // 19 -- 电机调谐故障
#define ERROR_ENCODER                   20      // 20 -- 码盘故障
#define ERROR_EEPROM                    21      // 21 -- EEPORM读写故障
#define ERROR_22                        22      // 22 -- 变频器硬件故障
#define ERROR_MOTOR_SHORT_TO_GND        23      // 23 -- 电机对地短路故障
#define ERROR_24                        24      // 24 -- 保留
#define ERROR_25                        25      // 25 -- 电机过热
#define ERROR_RUN_TIME_OVER             26      // 26 -- 运行时间到达

#define ERROR_USER_1                    27      // 27 -- 用户自定义故障1
#define ERROR_USER_2                    28      // 28 -- 用户自定义故障2
#define ERROR_POWER_UP_TIME_OVER        29      // 29 -- 上电时间到达
#define ERROR_LOSE_LOAD                 30      // 30 -- 掉载
#define ERROR_FDB_LOSE                  31      // 31 -- 运行时PID反馈丢失
#define ERROR_CBC                       40      // 40 -- 逐波限流故障
#define ERROR_SWITCH_MOTOR_WHEN_RUN     41      // 41 -- 运行时切换电机
#define ERROR_DEV                       42      // 42 -- 速度偏差过大
#define ERROR_OS                        43      // 43 -- 电机超速度
#define ERROR_MOTOR_OT                  45      // 45 -- 电机过温故障
#define ERROR_INIT_POSITION             51      // 51 -- 磁极位置检测失败
//#define ERROR_SPEED_DETECT              52      // 52 -- 零点位置辨识失败
#define ERROR_UVW_FDB                   53      // 53 -- uvw信号反馈错误
//#define ERROR_PG_PARA_ERROR             90      // 90 -- 编码器线数设定错误 
//#define ERROR_PG_LOST                   91      // 91 -- 未接编码器
//#define ERROR_PROGRAM_LOGIC             99      // 99 -- 程序执行逻辑错误

#define ERROR_RSVD                      9999    // 保留

#define PLC_DEFINE_ERROR_START          80
#define PLC_DEFINE_ERROR_END            89

//--------------------------------------------------
#define ERROR_LEVEL_NO_ERROR    0   // 无故障
#define ERROR_LEVEL_FREE_STOP   1   // 故障，自由停车
#define ERROR_LEVEL_STOP        2   // 故障，减速停车(按功能码停车)
#define ERROR_LEVEL_RUN         3   // 故障，继续运行

#define COMM_ERROR_MODBUS     1
#define COMM_ERROR_CANLINK    2
#define COMM_ERROR_CANOPEN    3
#define COMM_ERROR_PROFIBUS   4
#define COMM_ERROR_P2P        5
#define COMM_ERROR_PLC        6

struct ERROR_ATTRIBUTE_BITS
{
    Uint16 reset:1;     // 复位方式，0-必须手动复位，1-自动复位
    Uint16 level:3;     // 000-无故障，001-自由停车，010-按停机方式停机，011-继续运行，
};
union ERROR_ATTRIBUTE
{
    Uint16 all;
    struct ERROR_ATTRIBUTE_BITS bit;
};

/****************************************************
 * 故障时时间记忆
 *    上电时间、运行时间
****************************************************/
struct CUR_TIME
{
    Uint16 runTimeM;        // 运行时间-分
    Uint16 runTimeSec;      // 运行时间-秒

    Uint16 powerOnTimeM;    // 上电时间-分
    Uint16 powerOnTimeSec;  // 上电时间-秒

    Uint16 runTime;
    Uint16 powerOnTime;
    
};

// 故障处理属性  1: 自由停车  2:减速停机  3:继续运行
struct ERROR_ATTRIBUTE_LEVEL_BITS
{
    Uint16 Err1:3;
    Uint16 Err2:3;
    Uint16 Err3:3;
    Uint16 Err4:3;
    Uint16 Err5:3;
};
union ERROR_ATTRIBUTE_LEVEL
{
    Uint16 all;
    struct ERROR_ATTRIBUTE_LEVEL_BITS bit;
};

extern union ERROR_ATTRIBUTE errorAttribute;

extern Uint16 motorErrInfor[5];

extern struct CUR_TIME curTime; 

extern Uint16 errorCode;
extern Uint16 errorCodeOld;
extern Uint16 errAutoRstNum;
extern Uint16 bUv;
extern Uint16 errorOther;
extern Uint16 errorsCodeFromMotor[2];
extern Uint16 errorInfo;

enum ERROR_DEAL_STATUS
{
    ERROR_DEAL_PREPARE_FOR_WRITE_EEPROM,    // 准备故障记录保存
    ERROR_DEAL_WAIT_FOR_WRITE_EEPROM,       // 故障处理，等待保存故障记录完成
    ERROR_DEAL_WRITE_EEPROM_OK,             // 完成了故障记录保存。等待清除故障
    ERROR_DEAL_OK                           // 
};
extern enum ERROR_DEAL_STATUS errorDealStatus;

#define ERROR_DEALING (dspSubCmd.bit.errorDealing)

void ErrorDeal(void);
void ErrorReset(void);
Uint16 GetErrorAttribute(Uint16 errCode);

#endif  // __F_ERROR_H__














