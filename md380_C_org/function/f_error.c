//======================================================================
//
// 故障处理
//
// Time-stamp: <2012-03-14 14:08:23  Shisheng.Zhi, 0354>
//
//======================================================================


#include "f_error.h"
#include "f_main.h" 
#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_eeprom.h"
#include "f_io.h"
#include "f_menu.h"
#include "f_ui.h"
#include "f_comm.h"
#include "f_posCtrl.h"
#include "f_invPara.h"

#if F_DEBUG_RAM

#define DEBUG_F_ERROR                   0   // 故障处理开关
#define DEBUG_F_ERROR_FROM_MOTOR        0   // 使用性能的故障

#elif 1

#define DEBUG_F_ERROR                   1   // 故障处理开关
//=m
#if defined(FUNC_DEBUG)
#define DEBUG_F_ERROR_FROM_MOTOR        0   // 使用性能的故障
#else
#define DEBUG_F_ERROR_FROM_MOTOR        1   // 使用性能的故障
#endif

#define DEBUG_F_ERROR_LEVEL             1   // 故障分级

#endif



//------------------------------------------
Uint16 errorEeprom;     // EEPROM读写故障
Uint16 errorOther;      // 其他故障，包括外部故障、调谐故障、通讯故障、运行时间到达等
Uint16 errorOtherOld;
Uint16 errorCode;       // 新故障
// #define ERROR_NONE                0     // 0  -- 无
// #define ERROR_INVERTER_UNIT       1     // 1  -- 逆变单元保护
// #define ERROR_OC_ACC_SPEED        2     // 2  -- 加速过电流
// #define ERROR_OC_DEC_SPEED        3     // 3  -- 减速过电流
// #define ERROR_OC_CONST_SPEED      4     // 4  -- 恒速过电流
// #define ERROR_OV_ACC_SPEED        5     // 5  -- 加速过电压
// #define ERROR_OV_DEC_SPEED        6     // 6  -- 减速过电压
// #define ERROR_OV_CONST_SPEED      7     // 7  -- 恒速过电压
// #define ERROR_8                   8     // 8  -- 控制电源故障
// #define ERROR_UV                  9     // 9  -- 欠压故障
// #define ERROR_OL_INVERTER         10    // 10 -- 变频器过载
// #define ERROR_OL_MOTOR            11    // 11 -- 电机过载
// #define ERROR_LOSE_PHASE_INPUT    12    // 12 -- 输入缺相
// #define ERROR_LOSE_PHASE_OUTPUT   13    // 13 -- 输出缺相
// #define ERROR_OT_IGBT             14    // 14 -- 散热器过热
// #define ERROR_EXTERNAL            15    // 15 -- 外部故障
// #define ERROR_COMM                16    // 16 -- 通讯故障
// #define ERROR_CONTACTOR           17    // 17 -- 接触器故障
// #define ERROR_CURRENT_SAMPLE      18    // 18 -- 电流检测故障
// #define ERROR_TUNE                19    // 19 -- 电机调谐故障
// #define ERROR_ENCODER             20    // 20 -- 码盘故障
// #define ERROR_EEPROM              21    // 21 -- 数据溢出
// #define ERROR_22                  22    // 22 -- 变频器硬件故障
// #define ERROR_MOTOR_SHORT_TO_GND  23    // 23 -- 电机对地短路故障
// #define ERROR_24                  24    // 24 -- 保留
// #define ERROR_25                  25    // 25 -- 保留
// #define ERROR_RUN_TIME_OVER       26    // 26 -- 运行时间到达
// #define ERROR_SOFT_OC             27    // 31 -- 软件过流
// #define ERROR_CBC                 40    // 40 -- 快速限流超时故障
// #define ERROR_SWITCH_MOTOR        41    // 41 -- 运行时切换电机

//struct ERROR_DATA errorData;
union ERROR_ATTRIBUTE errorAttribute;   // 当前故障的属性
Uint16 errorInfo;

struct CUR_TIME curTime;   // 故障时时间记忆

LOCALF enum ERROR_DEAL_STATUS errorDealStatus;

enum UV_DEAL_STATUS
{
    UV_READY_FOR_WRITE_EEPROM,      // 欠压状态，准备掉电记忆数据
    UV_WAIT_WRITE_EEPROM,           // 欠压状态，等待掉电记忆
    UV_WRITE_EEPROM_OK              // 掉电记忆完成。等待恢复电压
};
LOCALF enum UV_DEAL_STATUS uvDealStatus;

Uint16 bUv;
LOCALF Uint32 errAutoRstClrWaitTicker;  // 故障自动复位次数清除的等待时间的ticker
LOCALF Uint16 errorSpaceTicker;         // 故障自动复位间隔时间

Uint16 errAutoRstNum;                   // 自动复位故障次数计数

Uint16 errorInfoFromMotor;
Uint16 errorCodeOld;   // 老故障
Uint16 errorInfoOld;
union ERROR_ATTRIBUTE errorAttributeOld;

const Uint16 errorCodeFromMotorList[] =
{
    ERROR_OC_ACC_SPEED,       // 2  -- 加速过电流
    ERROR_OV_ACC_SPEED,       // 5  -- 加速过电压
    ERROR_BUFFER_RES,         // 8  -- 缓冲电阻过载故障
    ERROR_UV,                 // 9  -- 欠压故障
    
    ERROR_OL_INVERTER,        // 10 -- 变频器过载
    ERROR_OL_MOTOR,           // 11 -- 电机过载
    ERROR_LOSE_PHASE_INPUT,   // 12 -- 输入缺相
    ERROR_LOSE_PHASE_OUTPUT,  // 13 -- 输出缺相
    
    ERROR_OT_IGBT,            // 14 -- 散热器过热
    ERROR_CONTACTOR,          // 17 -- 接触器故障
    ERROR_CURRENT_SAMPLE,     // 18 -- 电流检测故障
    ERROR_MOTOR_SHORT_TO_GND, // 23 -- 电机对地短路故障
    
    ERROR_LOSE_LOAD,          // 30 -- 掉载
    ERROR_CBC,                // 40 -- 逐波限流故障
    ERROR_INIT_POSITION,      // 51 -- 初始位置错误
    ERROR_RSVD,               // 52 -- 保留
    
    ERROR_RSVD,               // 99 -- 程序执行逻辑错误
    ERROR_TUNE,               // 19 -- 调谐故障
    ERROR_UVW_FDB             // 53 -- uvw信号反馈错误
};

//------------------------------------------
LOCALD void underVoltageDeal(void);
void PowerOffRemDeal(void);
void ErrorSceneSave(void);
void UpdateError(void);
Uint16 AutoResetErrorTrueDeal(void);
Uint16 AutoResetErrorFcDeal(void);
Uint16 AutoResetErrorDeal(void);
void GetNewErrorAttribute(void);
void GetNewError(void);
void GetErrorResetDeal(void);



#if DEBUG_F_ERROR
//=====================================================================
//
// 函数: 故障处理
// 描述: 更新errorCode，包括
//          1. 欠压处理
//          2. 掉电保存
//          3. 更新故障
//          4. 错误现场保存
// 
//=====================================================================
void ErrorDeal(void)
{
    // 欠压处理
    underVoltageDeal();
    // 更新故障
    UpdateError();
    // 故障现场保存
    ErrorSceneSave();
}


// 欠压处理
LOCALF void underVoltageDeal(void)
{
#if DEBUG_F_ERROR_FROM_MOTOR
    // 上电完成后性能报母线电压电压 或 功能等待时间超时
    if (((!dspStatus.bit.uv) && (POWER_ON_WAIT != powerOnStatus))    // 当上电完成之后，才判断是否欠压
        || (POWER_ON_FUNC_WAIT_OT == powerOnStatus))                // 功能等待时间超时，认为欠压
    {
        bUv = 1;  // 置欠压标志
    }
#endif

    if (bUv)          // 欠压
    {
        if (runFlag.bit.run)        // 不要等待掉电保存完毕才显示故障。立即显示故障
                                    // 欠压时有运行命令，runFlag.bit.run也为1。但不发送PWM，立即报警
        {
            errorOther = ERROR_UV;  // 运行时不会有(其他)故障
        }
        // 在errorReset()中立即关断PWM和清除运行命令
        
        switch (uvDealStatus)       // 欠压状态
        {
            // 准备掉电记忆数据
            case UV_READY_FOR_WRITE_EEPROM:
                // 没有其它EEPROM操作命令
                if (!funcCodeRwModeTmp)
                {
                    PowerOffRemDeal();          // 掉电记忆处理
                    // 置欠压状态为等待掉电记忆
                    uvDealStatus = UV_WAIT_WRITE_EEPROM;
                }
                break;
            // 等待掉电记忆
            case UV_WAIT_WRITE_EEPROM:          // 等待掉电保存完毕
                // 没有EEPROM操作命令表示掉电EEPROM操作已完成
                if (!funcCodeRwMode)       
                {
                    // 置欠压状态为等待恢复电压
                    uvDealStatus = UV_WRITE_EEPROM_OK;
                }
                break;
            // 等待恢复电压
            case UV_WRITE_EEPROM_OK:
                // 判断不再欠压的条件是，性能没有欠压标志，且功能的上电逻辑已经完成
                if ((dspStatus.bit.uv)      // 出现了欠压标志，当掉电保存完毕之后，才判断是否已经不欠压
//                    && (POWER_ON_CORE_OK == powerOnStatus)
                    )
                {
                    // 清欠压状态和欠压标志
                    uvDealStatus = UV_READY_FOR_WRITE_EEPROM;
                    bUv = 0;
                }
                break;

            default:
                break;
        }
    } // if (bUv)
}


// 更新故障
void UpdateError(void)
{
    // 保存老故障, 老的故障属性
    {
        errorCodeOld = errorCode;                   
        errorAttributeOld.all = errorAttribute.all;
        errorInfoOld = errorInfo;
    }

    // 获得新的故障
    GetNewError();

    // 获得新故障的属性
    GetNewErrorAttribute();

    // 确定是否更新为新的故障
    if (errorCodeOld != errorCode)  // 故障有更新
    {
        if (ERROR_LEVEL_NO_ERROR != errorAttribute.bit.level)  // 有新的故障
        {
            if (ERROR_LEVEL_NO_ERROR == errorAttributeOld.bit.level)  // 之前无故障
            {
                ;
            }
#if DEBUG_F_ERROR_LEVEL             
            // 新的故障，优先级更高
            else if (errorAttribute.bit.level < errorAttributeOld.bit.level)
            {
                ;
            }
			// 老的故障优先级为继续运行，响应新故障
            else if (errorAttributeOld.bit.level == ERROR_LEVEL_RUN)
			{
				;
			}
#endif            
            // 新的故障，优先级不比老的高,恢复为老的故障
            else
            {
                errorCode = errorCodeOld;
                errorAttribute.all = errorAttributeOld.all;
            }
        }
        else    // 无故障. 恢复为老的故障
        {
            errorCode = errorCodeOld;
            errorAttribute.all = errorAttributeOld.all;
        }
    }
}

//=====================================================================
//
// 函数: 获得性能故障信息
// 描述: 
// 
//=====================================================================
#if 0
Uint16 getMotorErrorInfo(Uint16 index)
{
    Uint16 i, j, errorInfo;
#if DEBUG_F_ERROR_FROM_MOTOR  
    
#if 1
    errorInfo = (motorErrInfor[index/4] << ((index%4)*4)) >> 12;
#else
    // 结构体中位置
    //size = sizeof(errorCodeFromMotorList);
    i = index%4;  // 所在故障信息结构体中序号
    j = index/4;  // 所在故障信息数组的序号
    
    switch(i)
    {
        case 0:   
            errorInfo = motorErrInfor[j].bits.errorInfo1;
            break;
            
        case 1:   
            errorInfo = motorErrInfor[j].bits.errorInfo2;
            break;
            
        case 2:   
            errorInfo = motorErrInfor[j].bits.errorInfo3;
            break;
            
        case 3:   
            errorInfo = motorErrInfor[j].bits.errorInfo4;
            break;
            
        default:
		   errorInfo = 0;
            break;    
    }
#endif    

#endif
    return errorInfo;

    
}
#endif

//=====================================================================
//
// 函数: 获得性能故障
// 描述: 返回故障处理优先级最高的故障
// 
//=====================================================================
void getMotorError(void)
{
#if DEBUG_F_ERROR_FROM_MOTOR
    Uint16 errorStatus;
    Uint16 errorCodeTmp, errorCode;
    Uint16 errorAttributeTmp, errorAttribute; 
    Uint16 errorInfo;
    Uint16 i;
    
    // 初始为无故障
    errorCode = ERROR_NONE;
    errorAttribute = GetErrorAttribute(errorCode);
    
    // 存在性能传递的故障
    if (errorsCodeFromMotor[0] || errorsCodeFromMotor[1])
    {
        for (i = 0; i < sizeof(errorCodeFromMotorList); i++)
        {
            // 0~15个故障存放于数组1
            if ( i < 16)
            {
                // 故障状态(0-有效  1-无效)
                errorStatus = errorsCodeFromMotor[0] >> i;
            }
            // 16~31个故障存放于数组2
            else
            {
                // 故障状态(0-有效  1-无效)
                errorStatus = errorsCodeFromMotor[1] >> (i - 16);
            }

            // 故障有效
            if (errorStatus & 0x01)
            {
                // 获得有效故障的故障编码
                errorCodeTmp = errorCodeFromMotorList[i];
                // 获得故障属性(继续运行、减速停机、自由停机)
                errorAttributeTmp = GetErrorAttribute(errorCodeTmp);
                // 优先级是否高于前一个故障
                if (errorAttributeTmp < errorAttribute)
                {
                    errorAttribute = errorAttributeTmp;
                    errorCode = errorCodeTmp;

                    // 获取故障信息    
                    errorInfo = (motorErrInfor[i/4] >> ((i%4)*4)) & 0xF;

                    // 为自由停车故障则不再继续查找
                    if (ERROR_LEVEL_FREE_STOP == errorAttribute)
                    {
                        break;
                    }
                }
            }
            
        }
    }
    else
    {
        errorInfo = 0;
    }
#if 0    
    else
    {
        errorCode = 0;
    }
#endif    

    // 获得性能故障
    errorCodeFromMotor = errorCode;
    // 获得性能故障信息
    errorInfoFromMotor = errorInfo;
#endif    
}

// 获得新的故障
// 欠压状态但还没有报欠压故障时，也要响应其他故障
#define ERROR_EXTERNAL_INFO_OPEN    1
#define ERROR_EXTERNAL_INFO_CLOSE   2
#define DEV_CHK_TIME_MIN            5   //  最小10ms检测时间
void GetNewError(void)
{
    static Uint16 devTcnt = 0;
    static Uint16  osTcnt = 0;
    static Uint16 motorSnOld;
	Uint16 errorLevel;
    
#if DEBUG_F_ERROR_FROM_MOTOR
    // 获得性能传递的故障及信息
    getMotorError();
    errorInfo = errorInfoFromMotor;

    // 功能过压故障
    if ((generatrixVoltage >= funcCode.code.ovPointSet)
        && (powerOnStatus == POWER_ON_CORE_OK)
        )
    {
        errorOther = ERROR_OV_ACC_SPEED;   // 报故障
        errorInfo = 0;
    }
    
    if (ERROR_OC_ACC_SPEED == errorCodeFromMotor)// 过流
    {
        if (runFlag.bit.accDecStatus == ACC_SPEED)
        {
            errorCode = ERROR_OC_ACC_SPEED;     // 加速过电流
        }
        else if (runFlag.bit.accDecStatus == DEC_SPEED)
        {
            errorCode = ERROR_OC_DEC_SPEED;     // 减速过电流
        }
        else
        {
            errorCode = ERROR_OC_CONST_SPEED;   // 恒速过电流
        }
    }
    else if ((ERROR_OV_ACC_SPEED == errorCodeFromMotor) // 过压
            || (errorOther == ERROR_OV_ACC_SPEED)       // 功能传递的过压
            )
    {
        if (runFlag.bit.accDecStatus == ACC_SPEED)
        {
            errorCode = ERROR_OV_ACC_SPEED;     // 加速过电压
        }
        else if (runFlag.bit.accDecStatus == DEC_SPEED)
        {
            errorCode = ERROR_OV_DEC_SPEED;     // 减速过电压
        }
        else
        {
            errorCode = ERROR_OV_CONST_SPEED;   // 恒速过电压
        }
        
    }
    else if (ERROR_UVW_FDB == errorCodeFromMotor)
    {
        errorCode = ERROR_ENCODER;
        errorCodeFromMotor = ERROR_ENCODER;
    }
    else if (errorCodeFromMotor && (ERROR_UV != errorCodeFromMotor)) // 其它由性能传递的故障, 欠压已经在前面处理
    {                       // 上电时不能直接显示ERR09
        errorCode = errorCodeFromMotor;     
        
    }
    else 
    {
         errorInfo = 0;
    }
#endif

	errorLevel = GetErrorAttribute(errorOther);

    // 运行时更改电机报错
    if ((motorSnOld != motorSn)
        && (runFlag.bit.run)
        )
    {
        errorOther = ERROR_SWITCH_MOTOR_WHEN_RUN;   // 报故障
        errorLevel = ERROR_LEVEL_FREE_STOP;
    }
    motorSnOld = motorSn;

    

    // 外部故障输入;常开/常闭   
	if ((diFunc.f1.bit.externalErrOpenIn) ||   // 外部故障输入长开
        ( (!diFunc.f2.bit.externalErrCloseIn) &&
          (diSelectFunc.f2.bit.externalErrCloseIn)
         )                                      // 外部故障常闭  
        )
    {
	    if(GetErrorAttribute(ERROR_EXTERNAL) < errorLevel)
	    {
            errorOther = ERROR_EXTERNAL;
			errorLevel = GetErrorAttribute(errorOther);
            
            if (diFunc.f1.bit.externalErrOpenIn)
            {
                errorInfo = ERROR_EXTERNAL_INFO_OPEN;   // 常开
            }
            else
            {
                errorInfo = ERROR_EXTERNAL_INFO_CLOSE;  // 常闭
            }
        }
    }
    // 用户自定义故障1
	if(diFunc.f2.bit.userError1)
    {
	    if(GetErrorAttribute(ERROR_USER_1) < errorLevel)
	    {
            errorOther = ERROR_USER_1;
			errorLevel = GetErrorAttribute(errorOther);
        }
    }

    // 用户自定义故障2
	if(diFunc.f2.bit.userError2)
    {
	    if(GetErrorAttribute(ERROR_USER_2) < errorLevel) 
	    {
            errorOther = ERROR_USER_2; 
			errorLevel = GetErrorAttribute(errorOther);
        }
    }
    
    // EEPROM故障 
	if (errorEeprom)
    {
	    if(GetErrorAttribute(ERROR_EEPROM) < errorLevel)
	    {
            errorOther = ERROR_EEPROM;   // 故障标示
			errorLevel = GetErrorAttribute(errorOther);
        }
    }

    // 速度偏差过大
    if( (ABS_INT32(frqRun - frq) > ((Uint32)maxFrq*funcCode.code.devChkValue/1000)) // 速度偏差过大
	    && (RUN_MODE_TORQUE_CTRL != runMode)                      // 非转矩控制
	    && runFlag.bit.run                                        // 运行中
	    && funcCode.code.devChkTime                               // 检测时间不为0
	    && (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_FVC)  // 闭环控制
	    && (!runFlag.bit.tune)                                    // 不处于调谐过程
       )
    {
        if(devTcnt <= (funcCode.code.devChkTime*(100/ERROR_DEAL_PERIOD) + DEV_CHK_TIME_MIN))
        {
            devTcnt++;
        }         
    }
    else
    {
        devTcnt = 0;
    }

    if (devTcnt > (funcCode.code.devChkTime*(100/ERROR_DEAL_PERIOD)))
    {
	    if (GetErrorAttribute(ERROR_DEV) < errorLevel)
	    {
            errorOther = ERROR_DEV;
			errorLevel = GetErrorAttribute(errorOther);
        }
    }

     // 电机过速度
     // VF运行无效、转矩控制有效
    if ((ABS_INT32(frqFdbTmp) > ((Uint32)maxFrq * (1000 + funcCode.code.osChkValue)/1000)) 
        && (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_FVC)  // 闭环控制
        && funcCode.code.osChkTime                                // 检测时间不为0
        && (!runFlag.bit.tune)                                    // 不处于调谐过程
        && (runFlag.bit.run)                                      // 运行中
       )
    {
        if (osTcnt <= (funcCode.code.osChkTime*(100/ERROR_DEAL_PERIOD)))
        {
            osTcnt++;
        }  
    }
    else
    {
        osTcnt = 0;
    }   
    
	if (osTcnt > (funcCode.code.osChkTime*(100/ERROR_DEAL_PERIOD)))
    {
    	if (GetErrorAttribute(ERROR_OS) < errorLevel)
    	{
           errorOther = ERROR_OS;
		   errorLevel = GetErrorAttribute(errorOther);
        }
    }

#if DEBUG_F_PLC_CTRL
    // PLC可编程卡传递的故障
    if (funcCode.code.plcEnable && funcCode.code.plcErrorCode)
    {
		if((funcCode.code.plcErrorCode >= PLC_DEFINE_ERROR_START)
		 && (funcCode.code.plcErrorCode <= PLC_DEFINE_ERROR_END)
		 )
        {
            errorOther = funcCode.code.plcErrorCode;
            errorLevel = ERROR_LEVEL_FREE_STOP;
        }
    }
    // 清PLC卡故障
    funcCode.code.plcErrorCode = 0;
#endif    
    
    // 功能的故障响应级别高于性能时才响应性能故障
    if (errorLevel < GetErrorAttribute(errorCode))
    {
        errorCode = errorOther; // other error, 如外部错误
    }
    
    errorOtherOld = errorOther;
    errorOther = ERROR_NONE;
}



// 获取新故障的故障属性
void GetNewErrorAttribute(void)
{
    if (errorCode == ERROR_NONE)
    {
       errorAttribute.bit.level = ERROR_LEVEL_NO_ERROR;
    }
    else
    {
        errorAttribute.bit.level = GetErrorAttribute(errorCode);
    }
}



// 故障现场保存，RAM更新
void ErrorSceneSave(void)
{
    if (errorCode 
        && (errorCode != errorCodeOld) // 发生新的故障，才进行故障记录
        && ((errorCode < PLC_DEFINE_ERROR_START) || (errorCode > PLC_DEFINE_ERROR_END))
        ) 
    {
        int16 i;

        funcCode.code.errorLatest1 = funcCode.code.errorLatest2; // 最近3次故障类型
        funcCode.code.errorLatest2 = funcCode.code.errorLatest3;
        funcCode.code.errorLatest3 = errorCode;

#define ERROR_SCENE_NUMBER  8  // 故障现场记录个数

        // 第一次故障记录
        for (i = ERROR_SCENE_NUMBER - 1; i >= 0; i--)
        {
            funcCode.code.errorScene1.all[i] = funcCode.code.errorScene2.all[i];
        }

        // 第二次故障记录
        for (i = ERROR_SCENE_NUMBER - 1; i >= 0; i--)
        {
            funcCode.code.errorScene2.all[i] = funcCode.code.errorScene3.all[i];
        }

        funcCode.code.errorScene3.elem.errorFrq = ABS_INT32(frqRun);         // 故障时频率
        funcCode.code.errorScene3.elem.errorCurrent = outCurrentDispOld;     // 故障时电流，应该为上一拍的输出电流
        funcCode.code.errorScene3.elem.errorGeneratrixVoltage = generatrixVoltage;   // 故障时母线电压
        funcCode.code.errorScene3.elem.errorDiStatus = diStatus.a.all;     // 故障时DI状态
        funcCode.code.errorScene3.elem.errorDoStatus = doStatus.a.all;     // 故障时DO状态
        funcCode.code.errorScene3.elem.errorInverterStatus = runFlag.all;  // 故障时变频器状态
        funcCode.code.errorScene3.elem.errorTimeFromPowerUp = curTime.powerOnTime;     // 故障时时间（从本次上电开始计时）
        funcCode.code.errorScene3.elem.errorTimeFromRun = curTime.runTime;         // 故障时时间（从运行时开始计时）

        // 故障频率的小数点
        funcCode.code.errorFrqUnit = ((funcCode.code.errorFrqUnit & 0x0FFF) << 4) + funcCode.code.frqPoint;

        if (ERROR_OC_ACC_SPEED == errorCodeFromMotor)   // 过流。// 性能传递故障
        {
            // 故障记录
            if (currentOcDisp > outCurrentDispOld)      // 传递的故障电流大于前一拍电流
            {
                funcCode.code.errorScene3.elem.errorCurrent = currentOcDisp;
            }
        }
    }
}



//=====================================================================
//
// 故障处理函数
//
// 1. 故障现场保存, EEPROM
// 2. 故障复位处理
// 
//=====================================================================
void ErrorReset(void)
{
    GetErrorResetDeal();

    //if (runFlag.bit.run)
    {
        // 达到故障自动复位次数清除时间(1h)，故障自动复位次数清零
        if (++errAutoRstClrWaitTicker > 10*TIME_UNIT_ERR_AUTO_RST_CLR / ERROR_DEAL_PERIOD)
        {
            errAutoRstClrWaitTicker = 0;
            errAutoRstNum = 0;
        }
    }

    if (errorCode)
    {
        tuneCmd = 0;
        if (ERROR_LEVEL_FREE_STOP == errorAttribute.bit.level)
        {
            runFlag.bit.run = 0;        // 欠压时有运行命令不要run灯亮一下
            
            // 当前拍立即关断PWM
            dspMainCmd.bit.run = 0;
            dspMainCmd1.bit.speedTrack = 0;
            dspMainCmd.bit.stopBrake = 0;
            dspMainCmd.bit.startBrake = 0;
            dspMainCmd.bit.startFlux = 0;
            dspMainCmd.bit.accDecStatus = 0;
        }

        errAutoRstClrWaitTicker = 0;    // 发生故障，故障自动复位次数清除时间清零

        switch (errorDealStatus)
        {
            case ERROR_DEAL_PREPARE_FOR_WRITE_EEPROM:   // 保存故障记录
            
                if (!funcCodeRwModeTmp)
                {
                    funcCodeRwModeTmp = FUNCCODE_RW_MODE_WRITE_SERIES;
                    startIndexWriteSeries = GetCodeIndex(funcCode.code.errorLatest1);     // 第一个故障记录
                    endIndexWriteSeries = GetCodeIndex(funcCode.code.errorScene1.all[sizeof(struct ERROR_SCENE_STRUCT) - 1]);     // 最后一个故障记录

                    //SaveOneFuncCode(GetCodeIndex(funcCode.code.errorFrqUnit));  // 保存 故障频率的小数点

                    errorDealStatus = ERROR_DEAL_WAIT_FOR_WRITE_EEPROM;
                }
                break;

            case ERROR_DEAL_WAIT_FOR_WRITE_EEPROM:
                if (!funcCodeRwModeTmp)   // 保存故障记录完成
                {
                    errorDealStatus = ERROR_DEAL_WRITE_EEPROM_OK;
                    
                    errorEeprom = ERROR_EEPROM_NONE;    // 程序能够执行到这里，表明EEPROM能成功读写。
                }
                break;

            case ERROR_DEAL_WRITE_EEPROM_OK:
                if (AutoResetErrorDeal())       // 复位故障
                {
                    errorSpaceTicker = 0;       // 非自动复位(包括手动)时，自动复位间隔时间清零

                    if (errorCodeFromMotor)     // 性能传递的故障
                    {
                        ERROR_DEALING = 1;      // 给性能信息: 功能已经处理该故障
                    }

                    errorDealStatus = ERROR_DEAL_OK;
                }

                break;

            case ERROR_DEAL_OK:
                // 当errorDealing为1时，性能在_拍内，会把errorCodeFromMotor清零。
                // 功能要保证_拍内errorDealing没有重新变回0。
                // 可以延时...。
                ERROR_DEALING = 0;              // 完成 给性能信息: 功能已经处理该故障

                if (errorCode == errorOther)    // 复位之前没有新的故障
                {
                    errorOther = ERROR_NONE;
                }
                if (errorCode == errorCodeFromMotor)
                {
                    errorCodeFromMotor = ERROR_NONE; // 也要清除. 考虑以后通讯交互
                }

                errorCode = ERROR_NONE;

                errorAttribute.bit.level = ERROR_LEVEL_NO_ERROR;

                errorDealStatus = ERROR_DEAL_PREPARE_FOR_WRITE_EEPROM;
                break;

            default:
                break;
        }
    }    
}



// 获取故障复位命令
extern Uint16 parallelMasterSendErrorResetTicker;
void GetErrorResetDeal(void)
{
    static Uint16 diFuncErrRstOld;
    
    // 判断是否error reset，脉冲有效
    if ((!diFuncErrRstOld) && (diFunc.f1.bit.errorReset))   // DI的故障复位状态变为有效
    {
        runCmd.bit.errorReset = 1;          // 在命令源的开始已经清零
    }
    diFuncErrRstOld = diFunc.f1.bit.errorReset;
    
    if ((KEY_STOP == keyFunc) // 任何控制方式下，STOP键的故障复位功能均有效
        && ((ERROR_LEVEL_RUN != errorAttribute.bit.level)    // 故障后继续运行的故障，stop仅为减速，不为复位
            || (!runFlag.bit.run)
            )
        )
    {
        runCmd.bit.errorReset = 1;
    }
}



// 自动复位故障处理
Uint16 AutoResetErrorDeal(void)
{
    Uint16 bResetError = 0;
    
    if ((AutoResetErrorTrueDeal()) 
        || AutoResetErrorFcDeal())
        bResetError = 1;

    return bResetError;
}


// 不需要功能码设置，就自动复位故障的处理
// 如，欠压处理
Uint16 AutoResetErrorTrueDeal(void)
{
    Uint16 bResetError = 0;
    
    // 欠压之后电压恢复正常，由性能把握。只要性能没有欠压标志，就认为电压已经恢复正常，自动清除欠压故障。
    if (ERROR_UV == errorCode)
    {
        if (!bUv)                       // 性能传递的欠压故障已经取消
        {
            bResetError = 1;            // 自动复位故障
        }
    }
    else if (ERROR_EEPROM == errorCode) // EEPROM故障
    {
        if (ERROR_EEPROM_NONE == errorEeprom)   // EEPROM无故障
        {
            bResetError = 1;                    // 自动复位故障
        }
    }
#if DEBUG_F_ERROR_LEVEL    
    // 继续运行的故障，自动复位
    else if (ERROR_LEVEL_RUN == errorAttribute.bit.level)
    {
        // 实际故障不为当前提示故障
        if((errorCode != errorOtherOld) && 
           (errorCode != errorCodeFromMotor))
        {
            bResetError = 1;
        }
    }
#endif
    return bResetError;
}



// 功能码的故障自动复位，或者手动故障复位
Uint16 AutoResetErrorFcDeal(void)
{
    Uint16 bResetError = 0;
    Uint16 errAutoRstSpaceTime;
    
    if (ERROR_MOTOR_SHORT_TO_GND != errorCode)  // 上电对地短路，不能复位
    {
        if (runCmd.bit.errorReset)  // 手动复位故障(欠压也可以手动复位)
        {
            //runCmd.bit.errorReset = 0;//可以不用清除，每1拍的开始都清零了
            errAutoRstNum = 0;      // 手动故障复位，故障自动复位次数清零
            // 只有自由停车故障才手动复位或停机后故障以及编码器参数设定、未接编码器故障可手动复位
            if (((ERROR_LEVEL_FREE_STOP == errorAttribute.bit.level) || (!runFlag.bit.run))
               )
            {
                bResetError = 1;
            }
        }
        else //if ((ERROR_EXTERNAL != errorCode)             // 不对外部故障复位
             //   && (ERROR_RUN_TIME_OVER != errorCode)      // 不对运行时间到达自动复位
             //   && (ERROR_POWER_UP_TIME_OVER != errorCode) // 不对上电时间到达自动复位
             //   && (ERROR_USER_1 != errorCode)             // 不对用户自定义故障1自动复位 
             //   && (ERROR_USER_2 != errorCode)             // 不对用户自定义故障2自动复位
             //   )            
        {   
            // 自动复位次数功能码为0时，故障自动复位间隔时间无效, 所以，这里为<，而不是<=
            if (errAutoRstNum < funcCode.code.errAutoRstNumMax)
            {
                // 为过流故障时故障自动复位时间处理为最小0.5s
                if ((errorCode == ERROR_OC_ACC_SPEED) 
                    || (errorCode == ERROR_OC_DEC_SPEED)
                    || (errorCode == ERROR_OC_CONST_SPEED))
                {
                    errAutoRstSpaceTime = ((funcCode.code.errAutoRstSpaceTime > 5) ? funcCode.code.errAutoRstSpaceTime : 5);
                }
                else
                {
                    errAutoRstSpaceTime = funcCode.code.errAutoRstSpaceTime;
                }
                
                if (++errorSpaceTicker >= (Uint32)errAutoRstSpaceTime 
                    * (Uint16)(TIME_UNIT_ERR_AUTO_RST_DELAY / RUN_CTRL_PERIOD))
                {
                    errAutoRstNum++;                // 自动复位故障次数计数
                    runCmd.bit.startProtect = 0;    // 启动保护

                    bResetError = 1;
                }
            }
        }
    }

    return bResetError;
}


//======================================================= 
//获取故障的故障属性
//=======================================================
Uint16 GetErrorAttribute(Uint16 errCode)
{
    Uint16 digit[5];
    Uint16 level;

    level = ERROR_LEVEL_FREE_STOP;
#if DEBUG_F_ERROR_LEVEL
    if (errCode == ERROR_NONE)
	{
        level = 4;
    }   
    // 每5个故障的故障保护动作选择，放在一个功能码里的5位
    else if ((errCode == ERROR_OL_MOTOR)             // 11
    	||  (errCode == ERROR_LOSE_PHASE_INPUT)  // 12
    	||  (errCode == ERROR_LOSE_PHASE_OUTPUT) // 13
    	||  (errCode == ERROR_EXTERNAL)          // 15
   		||  (errCode == ERROR_COMM)              // 16
		)
	{
        GetNumberDigit1(digit, funcCode.code.errorAction[0]);
        if(errCode < ERROR_OT_IGBT)
        {
            level = digit[errCode - ERROR_OL_MOTOR] + 1;
        }
        else
        {
            level = digit[errCode - ERROR_LOSE_PHASE_INPUT] + 1;
        }
	}
    else if( (errCode == ERROR_ENCODER)           // 20
        ||  (errCode == ERROR_EEPROM)            // 21
        ||  (errCode == ERROR_24)                // 24
        ||  (errCode == ERROR_25)                // 25
        ||  (errCode == ERROR_RUN_TIME_OVER)     // 26
		)
	{
        GetNumberDigit1(digit, funcCode.code.errorAction[1]); 
        if(errCode < ERROR_22)
        {
            level = digit[errCode - ERROR_ENCODER] + 1;
        }
        else
        {
            level = digit[errCode - ERROR_24 + 2] + 1;
        }
    }
    else if( (errCode == ERROR_USER_1)              // 27 -- 用户自定义故障1
    	||  (errCode == ERROR_USER_2)              // 28 -- 用户自定义故障2
    	||  (errCode == ERROR_POWER_UP_TIME_OVER)  // 29 -- 上电时间到达
    	||  (errCode == ERROR_LOSE_LOAD)           // 30 -- 掉载
    	||  (errCode == ERROR_FDB_LOSE)            // 31 -- 运行时PID反馈丢失
		)
	{
        GetNumberDigit1(digit, funcCode.code.errorAction[2]); 
        level = digit[errCode - ERROR_USER_1] + 1;
    }     
    else if ((errCode == ERROR_DEV)                       // 42 速度偏差过大
    	||  (errCode == ERROR_OS)                       // 43 电机超速度  
		)
	{
        GetNumberDigit1(digit, funcCode.code.errorAction[3]); 
        level = digit[errCode - ERROR_DEV] + 1;
    }
    else if (errCode == ERROR_INIT_POSITION)            // 51 初始位置错误
	{
        GetNumberDigit1(digit, funcCode.code.errorAction[3]); 
        level = digit[2] + 1;
    } 
#if 0     
    else if (errCode == ERROR_SPEED_DETECT)              // 52 速度反馈错误
	{
        GetNumberDigit1(digit, funcCode.code.errorAction[3]); 
        level = digit[3] + 1;
    }
    else if (errCode == ERROR_PROGRAM_LOGIC)             // 99 程序执行逻辑错误
	{
        GetNumberDigit1(digit, funcCode.code.errorAction[3]); 
        level = digit[4] + 1;
    }
#endif
 #endif     
    return level;
}


#elif 1

void ErrorDeal(void){}

#endif



