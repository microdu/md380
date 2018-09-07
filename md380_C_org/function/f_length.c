#include "f_length.h"


#define SERVO_ADJUST_MULTIPLY                   8
#define SERVO_ENABLE_AIM_FREQ                   4000    //如果目标频率超过该频率则加速过程不定位

Uint16 servoRunStatus;
SERVO_RUN_PARA_STRUCT servoRunControl;

LOCALF Uint16 runTickerServo;
LOCALF Uint32 qepPulseTemp;
Uint32 qepPulseCalALl;
Uint32 qepPulsePLC;
LOCALF Uint16 servoTicker;

unsigned int intervalTime;
unsigned int rangePer;
unsigned int bobofrq;
unsigned int boboreal;
unsigned int boboerr;
Uint16 servoRunStarFlag;
Uint16 servoControlEnable;
Uint32 pulseNote1;
Uint32 pulseNote2;
Uint32 pulseNote3;
Uint32 pulseTriangleLeft;
Uint16 timeLeft1;
Uint16 timeLeft2;
Uint16 frqSaveDis;
Uint32 motorPulseAll;
Uint16 servoRunResetFlag;
Uint16 servoCalFlag;
Uint16 servoRunDIFlag;
Uint32 servoPulseRunToPLC;
Uint16 servoOverFlag;
Uint16 servoEndTime;

LOCALF void servoParaCal(void);
LOCALF int32 ServoTrackPulseCal(int32 f0,int32 f1,int32 t);

//============================================================
//
// 伺服运行控制
//
//============================================================
void ServoRunCtrl(void)
{   
        Uint32 mAdjustTmp;
        Uint32 i;
        
        if (!runCmd.bit.common)                         // 运行中有停机命令
        {
                swingStatus = SWING_NONE;
                runStatus = RUN_STATUS_STOP;
                stopRunStatus = STOP_RUN_STATUS_INIT;

                frqTmp = frqRun;                        // 反馈频率
                runFlag.bit.servo = 0;
                return;
        }

        runFlag.bit.run = 1;                            // 置运行标志，之前有使用该标志
        runFlag.bit.common = 1;
        runFlag.bit.servo = 1;
        dspMainCmd.bit.run = 1;
        
        // 零点位置输入确认
        if(0)             
        {
            servoRunResetFlag =1;                   //此时的位置要重新计算，根据给定的脉冲数和当前的运行频率计算减速时间
            servoControlEnable = 1;                 //表示定位运行有效

            qepPulsePLC = EQep1Regs.QPOSCNT;        //传给PLC的数据清零 
            servoPulseRunToPLC = 0;
            servoOverFlag = 0;
            servoEndTime = 0;

            servoRunControl.motorRealRunPulse = 0;  //清掉运行的脉冲频率
            qepPulseTemp = EQep1Regs.QPOSCNT;       //获取当前计数值

            servoRunControl.servoLeftPulse = funcCode.code.servoPulseCheck ;          //上位机给定脉冲个数,校正脉冲个数处理

            pulseTriangleLeft = ServoTrackPulseCal(0,frqTmp,servoRunControl.decTime);   //计算当前频率下到停机的走过脉冲数
            //------------------------------------------------------
            // 如果当前频率按减速时间减速超过则需要重新计算减速时间
            //------------------------------------------------------
            if( servoRunControl.servoLeftPulse <= pulseTriangleLeft)
            {
                servoRunControl.pulseSet = servoRunControl.servoLeftPulse; //务必加上

                timeLeft1 = (Uint64)servoRunControl.servoLeftPulse * servoRunControl.motorPolePairs * 2 *100000
                           /((Uint32)frqTmp * servoRunControl.encoderPulse);
                           
                timeLeft2 = (Uint32) maxFrq * timeLeft1 / frqTmp;
                servoRunControl.decTime = timeLeft2 / 10;       //加减速时间2位小数，还要除以10
                servoRunControl.constTime =0;
                servoRunStatus = DEC_RUN_STATUS;
            }
            else
            {
                    servoParaCal();                                 //计算目标频率和恒速运行时间
                    servoRunStatus = ACC_RUN_STATUS;
            }
        }

        //--------------------------------------------------------
        // 调节器调节范围，如果大于40Hz，不定位处理
        // 运行过程中重新定位 servoRunResetFlag =1不在比较40hz计算
        //--------------------------------------------------------        
        if((servoRunControl.aimFreq > SERVO_ENABLE_AIM_FREQ) && (!servoRunResetFlag))
        {
                if((ABS_INT16(frqTmp) <= SERVO_ENABLE_AIM_FREQ) && (DEC_RUN_STATUS == servoRunStatus))      //小于40Hz的时候定位
                {
                        servoControlEnable = 1;
                        
                        if(!servoCalFlag)
                        {
                                frqSaveDis = frqTmp;
                                servoRunStarFlag = 1;
                                servoCalFlag = 1;
                                servoOverFlag = 0;
                                servoEndTime = 0;
                                
                                servoRunControl.motorRealRunPulse += (int32)(EQep1Regs.QPOSCNT - qepPulseTemp); 
                                pulseNote2 = (servoRunControl.motorRealRunPulse >> 2);       
                                qepPulseTemp = EQep1Regs.QPOSCNT;                       
                                servoRunControl.servoLeftPulse = servoRunControl.pulseSet - (servoRunControl.motorRealRunPulse >> 2); 
                                servoRunControl.motorRealRunPulse = 0;
                                
                                pulseTriangleLeft = ServoTrackPulseCal(0,frqTmp,servoRunControl.decTime);
                                
                                pulseNote1 = servoRunControl.calRunPulse; 
                                
                                //----------------------------------------------------
                                //如果剩下的比直接减速的脉冲数大，则要计算恒速运行时间
                                //否则要重新计算减速时间
                                //----------------------------------------------------
                                if( servoRunControl.servoLeftPulse >= pulseTriangleLeft)
                                {                               
                                        servoRunControl.constTime = (int64)servoRunControl.motorPolePairs* (servoRunControl.servoLeftPulse - pulseTriangleLeft)
                                                                  * FRQ_UINT * (TIME_UNIT_MS_PER_SEC/RUN_CTRL_PERIOD)
                                                                  / ((int64)frqTmp  * servoRunControl.encoderPulse); 
                                        servoRunStatus = CONST_RUN_STATUS;      //转为恒速运行
                                        servoRunControl.constTicker = 1;        //时间清掉
                                        servoRunControl.accCoursePulse =0;      //加速过程的脉冲数清掉                        
                                }
                                //--------------------------
                                // 小于需要重新计算减速时间
                                //--------------------------
                                else
                                {
                                        timeLeft1 = (Uint64)servoRunControl.servoLeftPulse * servoRunControl.motorPolePairs * 2 *100000
                                                   /((Uint32)frqTmp * servoRunControl.encoderPulse);
                                                   
                                        timeLeft2 = (Uint32) maxFrq * timeLeft1 / frqTmp;
                                        servoRunControl.decTime = timeLeft2 / 10;
                                        servoRunControl.constTime =0;
                                }            
                        }
                }
                else
                {
                        mAdjustTmp = 0; 
                        //servoRunStarFlag = 0;  
                }                            
        }   

        //-----------------------------------------------------
        // 定位控制运行过程程序
        //-----------------------------------------------------
        switch (servoRunStatus)
        {
                case ZERO_PREPARE_PARA_CAL:             //伺服运行参数计算准备计算
                {
                        servoParaCal();
                        servoControlEnable = 0; 
                        if(servoRunControl.aimFreq <= SERVO_ENABLE_AIM_FREQ)
                        {
                            servoControlEnable = 1;     // 允许定位运行标志
                        }
                        servoRunStatus = ACC_RUN_STATUS; 
                        servoRunStarFlag = 0;
                        motorPulseAll = 0; 
                        servoPulseRunToPLC = 0;
                        qepPulsePLC = EQep1Regs.QPOSCNT;
                        servoOverFlag = 0;
                        servoEndTime = 0;
                        break;
                } 
                case ACC_RUN_STATUS:                    //伺服加速运行过程
                {
                        frqCurAim = servoRunControl.aimFreq;
                        AccDecFrqCalc(servoRunControl.accTime, servoRunControl.decTime, 0);
                        //-----------------------------------------
                        // 因为目标频率超过40HZ所以处于不定位阶段
                        //-----------------------------------------
                        if(servoControlEnable)
                        {
                            servoRunControl.accCoursePulse = ServoTrackPulseCal(servoRunControl.runStarFreq,frqTmp,servoRunControl.accTime);
                            servoRunControl.calRunPulse = servoRunControl.accCoursePulse;
                        } 
                        
                        if (frqCurAim == frqTmp)        //加速到目标频率了     
                        {
                                runTickerServo = 0;    
                                servoRunStatus = CONST_RUN_STATUS;
                                if(servoRunControl.constTime)
                                {
                                        servoRunControl.constTicker++;
                                }
                                else
                                {
                                    servoRunStatus = DEC_RUN_STATUS;// 无恒速运行时间转为减速过程
                                }
                        }
                        break;
                }
                case CONST_RUN_STATUS:                  //伺服恒速运行过程
                {     
                        //-------------------------------------------------
                        // 如果是限制在40HZ运行，则要以当前的运行频率作计算
                        //-------------------------------------------------
                        if(servoCalFlag)
                        {
                            servoRunControl.aimFreq = frqTmp;
                        }
                        if(servoControlEnable)          //定位使能运行时才处理
                        {
                            servoRunControl.constCoursePulse = ((int64)servoRunControl.aimFreq * servoRunControl.encoderPulse * servoRunControl.constTicker * 2)
                                                                / ((int32)servoRunControl.motorPolePairs * TIME_UNIT_MS_PER_SEC * FRQ_UINT);
                            servoRunControl.calRunPulse = servoRunControl.accCoursePulse + servoRunControl.constCoursePulse;
                        }
                        if (servoRunControl.constTicker++ >= (servoRunControl.constTime-1) )
                        {
                                servoRunStatus = DEC_RUN_STATUS;
                        }
                        break;
                }
                case DEC_RUN_STATUS:                    //伺服减速运行过程
                {
                        frqCurAim = 0;
                        AccDecFrqCalc(servoRunControl.accTime, servoRunControl.decTime, 0); 
                        
                        if(!servoControlEnable)         //定位不使能处理
                        {
                            break;
                        }
                        servoRunControl.decCoursePulse = ServoTrackPulseCal(0,frqTmp,servoRunControl.decTime);  
                        //-----------------------------
                        // 如果需要调整过程的减速计算
                        // 如果目标频率超过40HZ，当减速小于40HZ时处理
                        //-----------------------------
                        if(servoRunStarFlag)
                        {
                                servoRunControl.calRunPulse = servoRunControl.servoLeftPulse - servoRunControl.decCoursePulse;

                        }
                        else
                        {                             
                                servoRunControl.calRunPulse = servoRunControl.pulseSet - servoRunControl.decCoursePulse;
                        }                          
                        break;
                } 
                default:
                {
                        break;
                }    
        }

        // 查看传给PLC的脉冲数
        servoPulseRunToPLC += (int32)(EQep1Regs.QPOSCNT - qepPulsePLC);
        qepPulsePLC = EQep1Regs.QPOSCNT;
        
        // 如果定位运行无效，则直接推出
        if(!servoControlEnable)
        {
            servoRunControl.servoDspFreq = frqTmp;
            return;
        }
        
        // 调节器调节范围计算，不定位闭环控制时清为零
        mAdjustTmp = (Uint32)funcCode.code.servoAdjustRange * maxFrq / 1000;                 // 调节器调节范围 
        
        // 查看总共走的脉冲数
        motorPulseAll += (int32)(EQep1Regs.QPOSCNT - qepPulseCalALl);         
        qepPulseCalALl = EQep1Regs.QPOSCNT; 
   
        // 以下计算电机走过的脉冲数   
        servoRunControl.motorRealRunPulse += (int32)(EQep1Regs.QPOSCNT - qepPulseTemp);        
        qepPulseTemp = EQep1Regs.QPOSCNT;
        
        // 以下计算脉冲偏差      
        servoRunControl.errorPulse =(int32)(servoRunControl.calRunPulse - (servoRunControl.motorRealRunPulse >> 2));
        boboerr = servoRunControl.errorPulse;
        
        // 以下当伺服脉冲偏差比较小时，要增强速度环增益     
        if (ABS_INT16(servoRunControl.errorPulse) < funcCode.code.servoSwitchPulseErr)
        {
                Uint16 upper;
            
                vcSpdLoopKp1 = funcCode.code.servoVcSpdLoopKp1 << 1;
                upper = funcCodeAttribute[GetCodeIndex(funcCode.code.servoVcSpdLoopKp1)].upper;
                if (vcSpdLoopKp1 > upper)
                {
                        vcSpdLoopKp1 = upper;
                }
                vcSpdLoopKp2 = funcCode.code.servoVcSpdLoopKp2 << 1;
                upper = funcCodeAttribute[GetCodeIndex(funcCode.code.servoVcSpdLoopKp2)].upper;
                if (vcSpdLoopKp2 > upper)
                {
                        vcSpdLoopKp2 = upper;
                }
        }
        servoRunControl.servoKp  = funcCode.code.servoLoopKp1;
        
        // 以下当伺服脉冲偏差比较大时，切换到强的伺服增益                                  
        if(ABS_INT16(servoRunControl.errorPulse) >= funcCode.code.servoSwitchPulseErr)
        {
                servoRunControl.servoKp = funcCode.code.servoLoopKp2;                
        } 
        
        servoRunControl.adjustFreq = (((int32)servoRunControl.errorPulse *  servoRunControl.servoKp) >> SERVO_ADJUST_MULTIPLY);

        // 伺服微调频率限幅
        if(ABS_INT16(servoRunControl.adjustFreq) > mAdjustTmp)
        {
                if(servoRunControl.adjustFreq < 0) 
                {
                        servoRunControl.adjustFreq = - mAdjustTmp;
                }
                else
                {                     
                        servoRunControl.adjustFreq =  mAdjustTmp;
                }        
        }
        
        // 位置定位当前计算传给DSP的频率
        servoRunControl.servoDspFreq = frqTmp + servoRunControl.adjustFreq; 
}     


//=======================================================
//
// 一些伺服参数计算
//
//=======================================================
LOCALF void servoParaCal(void)
{
        Uint32 mFrqCal; 
        Uint32 pulseTriangle;
        Uint32 pulseRectangle;     

        // 参数计算前的参数初始化准备      
        qepPulseTemp = EQep1Regs.QPOSCNT;
        qepPulseCalALl = EQep1Regs.QPOSCNT; 
        servoRunControl.motorRealRunPulse =0;
        servoRunControl.accCoursePulse = 0;
        servoRunControl.constCoursePulse = 0;
        servoRunControl.decCoursePulse = 0;
        servoTicker = 0;
        servoRunControl.constTicker = 0;
        servoRunControl.calRunPulse = 0;  
        servoCalFlag =0; 
        
        servoRunControl.runStarFreq = frqTmp;                           //定位运行时刻的启动频率
        servoRunControl.pulseSet = (Uint32)funcCode.code.servoPulseHigh *65535 + funcCode.code.servoPulseLower;//上位机给定脉冲个数
        servoRunControl.accTime = funcCode.code.servoCtrlAccTime;
        servoRunControl.decTime = funcCode.code.servoCtrlDecTime;
        servoRunControl.encoderPulse = funcCode.code.servoEncoderPulse;

        // 电机极对数计算
        servoRunControl.motorPolePairs = 60UL * funcCode.code.servoRatingFrq / ((int32)FRQ_UINT * funcCode.code.servoRatingSpeed); 
        

        // 以下计算要走的脉冲个数
        servoRunControl.pulseSet = (Uint64)servoRunControl.pulseSet * funcCode.code.servoElectroGearB / funcCode.code.servoElectroGearA;


        // 判断是否需要重新计算
        // servoRunControl.servoLeftPulse：重新定位的脉冲数
        if(servoRunResetFlag)
        {
                servoRunControl.pulseSet = servoRunControl.servoLeftPulse; 
        }

        // 计算定位目标频率
        mFrqCal = ((Uint64)servoRunControl.motorPolePairs * 2 * maxFrq * servoRunControl.pulseSet * FRQ_UINT
                * (TIME_UNIT_MS_PER_SEC / SERVO_ADD_DEC_TIME_UINT) + (Uint64)servoRunControl.encoderPulse 
                * servoRunControl.accTime * frqTmp * frqTmp)
                / ((Uint32)servoRunControl.encoderPulse  * (servoRunControl.accTime + servoRunControl.decTime));
                
        servoRunControl.aimFreq = qsqrt(mFrqCal);      //开根号        

        // 计算目标频率是否超过最大频率      
        if (servoRunControl.aimFreq > maxFrq)
        {
                servoRunControl.aimFreq = maxFrq;

                pulseTriangle = ServoTrackPulseCal(frqTmp,servoRunControl.aimFreq, servoRunControl.accTime)
                              + ServoTrackPulseCal( 0,servoRunControl.aimFreq, servoRunControl.decTime);
                              
                servoRunControl.constTime = (int64)servoRunControl.motorPolePairs* (servoRunControl.pulseSet - pulseTriangle)
                                          * FRQ_UINT *  (TIME_UNIT_MS_PER_SEC/RUN_CTRL_PERIOD)//TIME_UNIT_MS_PER_SEC
                                          / ((int64)servoRunControl.aimFreq  * servoRunControl.encoderPulse);
        }
        else
        {
                servoRunControl.constTime = 0;
        } 
}


//============================================================
//
// 计算从频率F0加减速到F1走过的脉冲数,t为加减速时间
//
//============================================================
LOCALF int32 ServoTrackPulseCal(int32 f0,int32 f1,int32 t)
{
        int32 mPulse;
        
        mPulse = ((int64)servoRunControl.encoderPulse * ((int64) f1 * f1 - f0 * f0) * t)
                 /((int32)servoRunControl.motorPolePairs * 2* maxFrq * FRQ_UINT * (TIME_UNIT_MS_PER_SEC / SERVO_ADD_DEC_TIME_UINT));          
        
        return mPulse;
}

