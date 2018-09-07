//======================================================================
//
// 功能码的一些处理
//
// Time-stamp: <2008-08-14 12:01:32  Shisheng.Zhi, 0354>
//
//======================================================================


#ifndef __F_FC_DEAL_H__
#define __F_FC_DEAL_H__


#if F_DEBUG_RAM

#define DEBUG_F_RESTORE_COMPANY_PARA_DEAL   0   // 恢复出厂参数
#define DEBUG_F_CLEAR_RECORD                0   // ClearRecordDeal
#define DEBUG_F_LIMIT_OTHER_CODE            0   // LimitOtherCodeDeal

#elif 1

#define DEBUG_F_RESTORE_COMPANY_PARA_DEAL   1
#define DEBUG_F_CLEAR_RECORD                1
#define DEBUG_F_LIMIT_OTHER_CODE            1

#endif


// 恢复出厂参数，清除记录
// 所需要的一些index宏定义


//====================================================
//
// 恢复出厂参数，某些参数不需要恢复
//
//====================================================

// 某些连续的功能码不需要恢复
#define INIT_EXCEPT_SERIES_S_0  GetCodeIndex(funcCode.group.ff[0])          // FF 厂家参数
#define INIT_EXCEPT_SERIES_E_0  GetCodeIndex(funcCode.group.ff[FFNUM-1])

#define INIT_EXCEPT_SERIES_S_1  GetCodeIndex(funcCode.group.fp[0])          // FP 功能码管理
#define INIT_EXCEPT_SERIES_E_1  GetCodeIndex(funcCode.group.fp[1])          //GetCodeIndex(funcCode.group.fp[FPNUM-1])

#define INIT_EXCEPT_SERIES_S_2  FC_MOTOR1_START_INDEX           // 第1电机参数
#define INIT_EXCEPT_SERIES_E_2  FC_MOTOR1_END_INDEX

#define INIT_EXCEPT_SERIES_S_3  FC_MOTOR2_START_INDEX           // 第2电机参数
#define INIT_EXCEPT_SERIES_E_3  FC_MOTOR2_END_INDEX

#define INIT_EXCEPT_SERIES_S_6  FC_MOTOR3_START_INDEX           // 第3电机参数
#define INIT_EXCEPT_SERIES_E_6  FC_MOTOR3_END_INDEX

#define INIT_EXCEPT_SERIES_S_7  FC_MOTOR4_START_INDEX           // 第4电机参数
#define INIT_EXCEPT_SERIES_E_7  FC_MOTOR4_END_INDEX

#define INIT_EXCEPT_SERIES_S_4  GetCodeIndex(funcCode.group.ae[0])          // AE AIAO出厂校正
#define INIT_EXCEPT_SERIES_E_4  GetCodeIndex(funcCode.group.ae[AENUM-1])

#define INIT_EXCEPT_SERIES_S_5  GetCodeIndex(funcCode.code.errorLatest1)    // 第一次故障类型
#define INIT_EXCEPT_SERIES_E_5  LAST_ERROR_RECORD_INDEX                     // 最后一个故障记录


// 某些单独的功能码不需要恢复
#define INIT_EXCEPT_SINGLE_0    GetCodeIndex(funcCode.code.runTimeAddup)        // 累计运行时间
#define INIT_EXCEPT_SINGLE_1    GetCodeIndex(funcCode.code.runTimeAddupSec)     // 累计运行时间
#define INIT_EXCEPT_SINGLE_2    GetCodeIndex(funcCode.code.softVersion)         // 软件版本号
#define INIT_EXCEPT_SINGLE_3    GetCodeIndex(funcCode.code.radiatorTemp)        // 逆变器模块散热器温度
#define INIT_EXCEPT_SINGLE_4    GetCodeIndex(funcCode.code.temp2)               // 整流桥散热器温度
#define INIT_EXCEPT_SINGLE_5    GetCodeIndex(funcCode.code.frqPoint)            // 频率指令小数点

#define INIT_EXCEPT_SINGLE_6    GetCodeIndex(funcCode.code.powerUpTimeAddup)    // 累计运行时间
#define INIT_EXCEPT_SINGLE_7    GetCodeIndex(funcCode.code.powerUpTimeAddupSec) // 累计运行时间
#define INIT_EXCEPT_SINGLE_8    GetCodeIndex(funcCode.code.powerAddup)          // 累计耗电量
#define INIT_EXCEPT_SINGLE_9    GetCodeIndex(funcCode.code.errorFrqUnit)        // 故障时频率小数点

#define INIT_EXCEPT_SINGLE_10   GetCodeIndex(funcCode.code.productVersion)      // 产品号
#define INIT_EXCEPT_SINGLE_11   GetCodeIndex(funcCode.code.softVersion)         // 软件版本号


//====================================================
//
// 清除记录
//
//====================================================

// 某些连续的功能码清除记录
#define CLEAR_RECORD_SERIES_S_0  GetCodeIndex(funcCode.code.errorLatest1)        // 第一次故障类型
#define CLEAR_RECORD_SERIES_E_0  LAST_ERROR_RECORD_INDEX                         // 最后一个故障记录

// 某些单独的功能码清除记录
#define CLEAR_RECORD_SINGLE_0    GetCodeIndex(funcCode.code.runTimeAddup)        // 累计运行时间
#define CLEAR_RECORD_SINGLE_1    GetCodeIndex(funcCode.code.runTimeAddupSec)     // 累计运行时间的s
#define CLEAR_RECORD_SINGLE_2    GetCodeIndex(funcCode.code.powerUpTimeAddup)    // 累计上电时间
#define CLEAR_RECORD_SINGLE_3    GetCodeIndex(funcCode.code.powerUpTimeAddupSec) // 累计上电时间的s
#define CLEAR_RECORD_SINGLE_4    GetCodeIndex(funcCode.code.powerAddup)          // 累计耗电量
#define CLEAR_RECORD_SINGLE_5    GetCodeIndex(funcCode.code.errorFrqUnit)        // 故障时频率小数点

#endif  // __F_FC_DEAL_H__










