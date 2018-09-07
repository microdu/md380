//======================================================================
//
// 功能码的一些处理
//
// Time-stamp: <2012-08-14 12:01:32  Shisheng.Zhi, 0354>
//
//======================================================================


#include "f_menu.h"
#include "f_main.h"
#include "f_eeprom.h"
#include "f_invPara.h"
#include "f_fcDeal.h"
#include "f_runSrc.h"
#include "f_frqSrc.h"

#if F_DEBUG_RAM
#define DEBUG_F_P_OFF_REM               0   // 掉电记忆
#elif 1
#define DEBUG_F_P_OFF_REM               1   // 掉电记忆
#endif


#if DEBUG_F_POSITION_CTRL
#pragma DATA_SECTION(limitedByOtherCodeIndex, "data_ram");
#endif
// 该功能码的上下限是其他功能码
#define LIMITED_BY_OTHER_CODE_NUM_MAX   150  
// 当前使用了21个，这里分配50个应该能保证很长一段时间修改功能码内的正常使用而不会溢出。2007.10.29
// 33个
Uint16 limitedByOtherCodeIndex[LIMITED_BY_OTHER_CODE_NUM_MAX];
Uint16 limitedByOtherCodeIndexNum;  // 上下限受其他功能码限制的功能码总数
#if DEBUG_F_LIMIT_OTHER_CODE
// 该功能码是其他功能码的上下限
#define LIMIT_OTHER_CODE_NUM  13    // 位置在funcCode的位置是从后到前，是因为使用limitOtherCodeIndex[]时是从后到前。
LOCALF Uint16 const limitOtherCodeIndex[LIMIT_OTHER_CODE_NUM] =
{
    PULSE_IN_MAX_INDEX,         // F2-22  0  PULSE最大输入
    PULSE_IN_MIN_INDEX,         // F2-20  1  PULSE最小输入
    CURVE2_MAX_INDEX,           // F2-16  2  AI2最大输入
    CURVE2_MIN_INDEX,           // F2-14  3  AI2最小输入
    CURVE1_MAX_INDEX,           // F2-10  4  AI1最大输入
    CURVE1_MIN_INDEX,           // F2-08  5  AI1最小输入
    VF_FRQ3_INDEX,              // F1-11  6  多点VF频率点3
    VF_FRQ2_INDEX,              // F1-09  7  多点VF频率点2
    VF_FRQ1_INDEX,              // F1-07  8  多点VF频率点1
    RATING_FRQ_INDEX,           // F1-03  9  电机额定频率
    LOWER_FRQ_INDEX,            // F0-07  10 下限频率
    UPPER_FRQ_INDEX,            // F0-06  11 上限频率
    MAX_FRQ_INDEX,              // F0-04  12 最大频率
};
#endif


//=====================================================================
// 
// 
// 与机型相关的功能码，非电机参数
// 不可更改参数，已经考虑
// 
//=====================================================================
Uint16 GetFuncCodeInit(Uint16 index, Uint16 type)
{
//#if DEBUG_F_RESTORE_COMPANY_PARA_DEAL || DEBUG_F_CHECK_MENU_MODE
#if DEBUG_F_RESTORE_COMPANY_PARA_DEAL
    Uint16 init;

// 恢复出厂参数，某些参数不需要恢复
// 某些连续的功能码不需要恢复
    if (((INIT_EXCEPT_SERIES_S_0 <= index) 
        && (index <= INIT_EXCEPT_SERIES_E_0))
      || ((INIT_EXCEPT_SERIES_S_1 <= index) 
        && (index <= INIT_EXCEPT_SERIES_E_1))         
      || ((INIT_EXCEPT_SERIES_S_4 <= index) 
        && (index <= INIT_EXCEPT_SERIES_E_4))
      || ((INIT_EXCEPT_SERIES_S_5 <= index) 
        && (index <= INIT_EXCEPT_SERIES_E_5))      
      || (( ((INIT_EXCEPT_SERIES_S_2 <= index)      // 第一电机参数
            && (index <= INIT_EXCEPT_SERIES_E_2))           
         || ((INIT_EXCEPT_SERIES_S_3 <= index)      // 第二电机参数
            && (index <= INIT_EXCEPT_SERIES_E_3))               
         || ((INIT_EXCEPT_SERIES_S_6 <= index)      // 第三电机参数
            && (index <= INIT_EXCEPT_SERIES_E_6))               
         || ((INIT_EXCEPT_SERIES_S_7 <= index)      // 第四电机参数
            && (index <= INIT_EXCEPT_SERIES_E_7))
        ) 
        && (!type)) // 恢复电机参数
        )
    {
        init = funcCode.all[index];
    }
// 某些单独的功能码不需要恢复
    else if ((INIT_EXCEPT_SINGLE_0 == index) 
            || (INIT_EXCEPT_SINGLE_1 == index)  
            || (INIT_EXCEPT_SINGLE_2 == index)
            || (INIT_EXCEPT_SINGLE_3 == index)
            || (INIT_EXCEPT_SINGLE_4 == index)
            || ((INIT_EXCEPT_SINGLE_5 == index) 
            	&& (!type)   // 恢复电机参数时恢复频率小数点
			   )
            || (INIT_EXCEPT_SINGLE_6 == index)
            || (INIT_EXCEPT_SINGLE_7 == index)
            || (INIT_EXCEPT_SINGLE_8 == index)
            || (INIT_EXCEPT_SINGLE_9 == index)
            || (INIT_EXCEPT_SINGLE_10 == index)
            || (INIT_EXCEPT_SINGLE_11 == index)
        )
    {
        init = funcCode.all[index];
    }
// A7 AIAO校正，从AE组恢复
    else if (((GetCodeIndex(funcCode.code.aiCalibrateCurve[0].before1) <= index)) 
            && (index <= GetCodeIndex(funcCode.code.aoCalibrateCurve[AO_NUMBER-1].after2))
            )
    {
        // AI3不恢复
        if (((GetCodeIndex(funcCode.code.aiCalibrateCurve[2].before1) <= index)) 
            && (index <= GetCodeIndex(funcCode.code.aiCalibrateCurve[2].after2))
            )
        {
            init = funcCode.all[index];
        }
        // AO2不恢复
        else if (((GetCodeIndex(funcCode.code.aoCalibrateCurve[1].before1) <= index)) 
            && (index <= GetCodeIndex(funcCode.code.aoCalibrateCurve[1].after2))
            )
        {
            init = funcCode.all[index];
        }
        else
        {
            init = funcCode.group.ae[index - GetCodeIndex(funcCode.code.aiCalibrateCurve[0].before1)];
        }
    }
// 加减速时间
    else if ((ACC_TIME1_INDEX == index) ||   
        (DEC_TIME1_INDEX == index) ||
        (ACC_TIME2_INDEX == index) ||
        (DEC_TIME2_INDEX == index) ||
        (ACC_TIME3_INDEX == index) ||
        (DEC_TIME3_INDEX == index) ||
        (ACC_TIME4_INDEX == index) ||
        (DEC_TIME4_INDEX == index)
        )
    {
        if (invPara.type < invPara.bitAccDecStart)  // 机型 < 21
        {
            init = ACC_DEC_T_INIT1;
        }
        else                    // 机型大于等于21，加减速时间出厂值为50s
        {
            init = ACC_DEC_T_INIT2;
        }
    }
#if (DEBUG_F_INV_TYPE_RELATE || DEBUG_F_MOTOR_POWER_RELATE)
// 载波频率
    else if (CARRIER_FRQ_INDEX == index)    
    {
        init = GetInvParaPointer(invPara.type)->elem.carrierFrq;
    }
// 振荡抑制增益
    else if ((ANTI_VIBRATE_GAIN_INDEX == index)          
            || (ANTI_VIBRATE_GAIN_MOTOR2_INDEX == index)
            || (ANTI_VIBRATE_GAIN_MOTOR3_INDEX == index)
            || (ANTI_VIBRATE_GAIN_MOTOR4_INDEX == index)
            )
    {
        init = GetInvParaPointer(invPara.type)->elem.antiVibrateGain;
    }
// 转矩提升
    else if (TORQUE_BOOST_INDEX == index)   
    {
        init = TorqueBoostDeal(funcCode.code.motorParaM1.elem.ratingPower);
    }
    else if (TORQUE_BOOST_MOTOR2_INDEX == index)   // 第2电机转矩提升
    {
        init = TorqueBoostDeal(funcCode.code.motorFcM2.motorPara.elem.ratingPower);
    }
    else if (TORQUE_BOOST_MOTOR3_INDEX == index)   // 第3电机转矩提升
    {
        init = TorqueBoostDeal(funcCode.code.motorFcM3.motorPara.elem.ratingPower);
    }
    else if (TORQUE_BOOST_MOTOR4_INDEX == index)   // 第4电机转矩提升
    {
        init = TorqueBoostDeal(funcCode.code.motorFcM4.motorPara.elem.ratingPower);
    }
    else if (SVC_MODE_INDX == index)    // SVC优化方式
    {
        // 1140V机器SVC默认不优化
        if (invPara.ratingVoltage == 1140)
        {
            init = 0;
        }
        else
        {
            init = 1;
        }
    }
    // 过压点设置
    else if (OV_POINT_SET_INDEX == index)
    {
        init = ovVoltageInitValue[invPara.volLevel];
    }
#endif
    else
    {
        init = GetFuncCodeInitOriginal(index);
    }
    
    return init;
#endif
}


//=====================================================================
//
// 清除记录信息
// 
//=====================================================================
void ClearRecordDeal(void)
{
#if DEBUG_F_CLEAR_RECORD
    int16 i;

    funcCodeRwModeTmp = FUNCCODE_RW_MODE_WRITE_SERIES;
    startIndexWriteSeries = CLEAR_RECORD_SERIES_S_0;    // 第一次故障类型
    endIndexWriteSeries = CLEAR_RECORD_SERIES_E_0;      // 最后一个故障记录
    for (i = startIndexWriteSeries; i <= endIndexWriteSeries; i++)
    {
        funcCode.all[i] = 0;
    }

    // 累计运行时间
    funcCode.all[CLEAR_RECORD_SINGLE_0] = 0;
    SaveOneFuncCode(CLEAR_RECORD_SINGLE_0);
    // 累计运行时间的秒
    funcCode.all[CLEAR_RECORD_SINGLE_1] = 0;
    SaveOneFuncCode(CLEAR_RECORD_SINGLE_1);
    // 累计上电时间
    funcCode.all[CLEAR_RECORD_SINGLE_2] = 0;
    SaveOneFuncCode(CLEAR_RECORD_SINGLE_2);
    // 累计上电时间的秒
    funcCode.all[CLEAR_RECORD_SINGLE_3] = 0;
    SaveOneFuncCode(CLEAR_RECORD_SINGLE_3);
    // 累计耗电量
    funcCode.all[CLEAR_RECORD_SINGLE_4] = 0;
    SaveOneFuncCode(CLEAR_RECORD_SINGLE_4);
    // 故障频率小数点
    funcCode.all[CLEAR_RECORD_SINGLE_5] = 0x222;
    SaveOneFuncCode(CLEAR_RECORD_SINGLE_5);
#endif
}


// 恢复出厂参数之后的处理
void RestoreCompanyParaOtherDeal(void)
{
#if DEBUG_F_RESTORE_COMPANY_PARA_DEAL
    
    funcCode.code.motorDebugFc = 0;       // 恢复性能调试
	funcCode.code.aiaoCalibrateDisp = 0;  // 恢复AIAO校正功能码显示
    SaveOneFuncCode(GetCodeIndex(funcCode.code.motorDebugFc));
    SaveOneFuncCode(GetCodeIndex(funcCode.code.aiaoCalibrateDisp));
    MenuModeDeal();

    // 某些功能码是其他功能码上下限的处理
    LimitOtherCodeDeal(RATING_FRQ_INDEX);   // 电机额定频率

    upDownFrq = 0;          // up/down修改之后，再恢复出厂参数，显示为预置频率。
    frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_OFF;
    runCmd.all = 0;         // 清除所有运行命令
    runDirPanelOld = 0;
#endif
}

//=====================================================================
//
// 掉电记忆处理函数，包括需要单独记忆的功能码
//
// 长度计数器的ticker，计数器输入的ticker，累计运行时间的s，
// 通讯修改频率值，运行/停机时LED显示参数的bit位置。
// 程序中直接使用，fr[]已经包含。
//
//=====================================================================
void PowerOffRemDeal(void)
{
#if DEBUG_F_P_OFF_REM

#if DEBUG_F_MOTOR_FUNCCODE
    int16 i;
#endif

    // 实际长度
    SaveOneFuncCode(GetCodeIndex(funcCode.code.lengthCurrent));
    
    // 累计运行时间
    SaveOneFuncCode(RUN_TIME_ADDUP_INDEX);
    // 累计上电时间
    SaveOneFuncCode(POWER_TIME_ADDUP_INDEX);
    // 累计耗电量
    SaveOneFuncCode(POWER_ADDUP_INDEX);
    // DP卡通讯需要存储EEPROM数据
    if (saveEepromIndex)
    {
        SaveOneFuncCode(saveEepromIndex);
    }

    // 以下为FR组
    // 数字设定频率UP/DOWN掉电记忆
    funcCode.code.upDownFrqRem = upDownFrq;     // 保存至掉电记忆区
    
    // PLC掉电记忆. 都记忆，但是仅功能码设定为掉电记忆时才起作用。
    funcCode.code.plcStepRem = plcStep;
    if (plcStep >= PLC_STEP_MAX) // 若已经是MAX，下次上电从step0开始运行。防止下次上电后不响应运行命令。
    {
        funcCode.code.plcStepRem = 0;
    }
    funcCode.code.plcTimeHighRem = (plcTime & 0xFFFF0000UL) >> 16;
    funcCode.code.plcTimeLowRem = plcTime & 0x0000FFFFUL;

    funcCode.code.pmsmRotorPos = pmsmRotorPos;   // 同步机转子位置
    //funcCode.code.extendType = ;

#if DEBUG_F_MOTOR_FUNCCODE
#if !F_DEBUG_RAM
    for (i = REM_P_OFF_MOTOR - 1; i >= 0; i--)  // 性能需要的掉电记忆参数
    {
        funcCode.code.remPOffMotorCtrl[i] = gSendToFunctionDataBuff[i + MOTOR_TO_Func_2MS_DATA_NUM];
    }
#endif
#endif

    funcCodeRwModeTmp = FUNCCODE_RW_MODE_WRITE_SERIES;
    startIndexWriteSeries = GetCodeIndex(funcCode.group.remember[0]);
    endIndexWriteSeries = GetCodeIndex(funcCode.group.remember[REM_NUM-1]);

#endif
}


//=====================================================================
//
// 某些功能码是其他功能码上下限的处理
//
// 
// 说明：
// 1、当修改了某个功能码的值时，应该调用本函数。
// 
// 2、修改了这些限制其他功能码上下限的功能码时，要同时修改被限制上下限功能码的值。
// 
// 3、目前最极限的情况是: 修改最大频率maxFrq。
// 电机额定频率， 范围是 0.01Hz        - 最大频率
// 多点VF频率点1，范围是 0.00Hz        - 多点VF频率点1
// 多点VF频率点2，范围是 多点VF频率点1 - 多点VF频率点3
// 多点VF频率点3，范围是 多点VF频率点2 - 电机额定频率
//
//=====================================================================
LOCALF void LimitOtherCodeDeal(Uint16 index)
{
#if DEBUG_F_LIMIT_OTHER_CODE
    int16 i, j, k;

    for (j = LIMIT_OTHER_CODE_NUM - 1; j >= 0; j--)
    {
        if (limitOtherCodeIndex[j] == index)
        {
            for (k = 2 - 1; k >= 0; k--)    // 目前最多往前推_级，就能保证目前的情况都正确。
            {   // limitedByOtherCodeIndex[]的数据，数组下标从小到大，数组的值实际上是从大到小。
                i = (int16)limitedByOtherCodeIndexNum - 1;
                for (; i >= 0; i--)    // 考虑limitedByOtherCodeIndexNum为0的情况
                {
                    const FUNCCODE_ATTRIBUTE *pAttribute = &funcCodeAttribute[limitedByOtherCodeIndex[i]];
                    Uint16 *pCode = &funcCode.all[limitedByOtherCodeIndex[i]];
                    Uint16 tmp = 0;
                    Uint16 upper;
                    Uint16 lower;
                    
                    if (pAttribute->attribute.bit.upperLimit)
                    {
                        int32 fc1, upper1;
                        
                        upper = funcCode.all[pAttribute->upper];

                        if (pAttribute->attribute.bit.signal)   // 有符号
                        {
                            fc1 = (int32)(int16)*pCode;
                            upper1 = (int32)(int16)upper;
                        }
                        else
                        {
                            fc1 = *pCode;
                            upper1 = upper;
                        }
                        
                        //if (*pCode > upper) // 这里没有考虑该功能码的符号，有错误。支世胜，2009-07-22
                        if (fc1 > upper1)
                        {
                            *pCode = upper;       // 存入RAM
                            SaveOneFuncCode(limitedByOtherCodeIndex[i]);    // 存入EEPROM
                            
                            tmp = 1;
                        }
                    }
                    
                    if (pAttribute->attribute.bit.lowerLimit)
                    {
                        int32 fc1, lower1;
                        
                        lower = funcCode.all[pAttribute->lower];

                        if (pAttribute->attribute.bit.signal)   // 有符号
                        {
                            fc1 = (int32)(int16)*pCode;
                            lower1 = (int32)(int16)lower;
                        }
                        else
                        {
                            fc1 = *pCode;
                            lower1 = lower;
                        }
                        
                        //if ((int16)*pCode < (int16)lower)   // 这里没有考虑该功能码的符号，有错误。支世胜，2009-07-26
                        if (fc1 < lower1)
                        {
                            if (!tmp)
                            {
                                *pCode = lower; // ...
                                SaveOneFuncCode(limitedByOtherCodeIndex[i]);// 存入EEPROM
                            }
                            else
                            {
                                funcCode.all[pAttribute->lower] = *pCode;   // 存入RAM
                                SaveOneFuncCode(pAttribute->lower);         // 应该写入EEPROM的是lower，而不是limitedByOtherCodeIndex[i]
                            }
                        }
                    }
                }
            }
            
            break;
        }
    }
#endif
}



// 直接根据数组得到的出厂值
Uint16 GetFuncCodeInitOriginal(Uint16 index)
{
    Uint16 init;

    // F0-FF,FP,A0-AF,B0-BF,C0-CF
    // 不包括 fChk，remember
    if (index < FNUM_PARA)                  // EEPROM CHK 之前
    {
        init = (funcCodeAttribute[index].init);
    }
#if 0     
    else if (EEPROM_CHECK_INDEX1 == index)  // EEPROM CHK1
    {
        init = EEPROM_CHECK1;
    }   
    else if (EEPROM_CHECK_INDEX2 == index)  // EEPROM CHK2
    {
        init = EEPROM_CHECK2;
    }
    else if (AI_AO_CHK_FLAG == index)  // AIAO CHK2
    {
        init = AIAO_CHK_WORD;
    }
#endif    
    else if (SAVE_USER_PARA_PARA1 == index)   // USER PARA SAVE CHK1
    {
        init = funcCode.code.saveUserParaFlag1;  // 恢复出厂值不恢复该值，新EEPROM恢复成0
    }
    else if (SAVE_USER_PARA_PARA2 == index)   // USER PARA SAVE CHK2
    {
        init = funcCode.code.saveUserParaFlag2;
    }
    else                                    // REM(掉电记忆)
    {
        init = 0;
    }

    return init;
}









