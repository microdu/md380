//======================================================================
//
// S曲线
//
// Time-stamp: <2012-02-28 20:46:14  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_main.h"
#include "f_menu.h"
#include "f_debug.h"

#if F_DEBUG_RAM                         // 仅调试功能，在CCS的build option中定义的宏

#define DEBUG_F_S_CURVE_A         0       // S曲线A
#define DEBUG_F_S_CURVE_B         0       // S曲线B

#elif 1

#define DEBUG_F_S_CURVE_A         1

#if !DEBUG_F_POSITION_CTRL
#define DEBUG_F_S_CURVE_B         1
#elif 1
#define DEBUG_F_S_CURVE_B         1
#endif

#endif

#define ACC_DEC_DEBUG 1         // debug acc-dec 
enum S_CURVE_A_STATUS
{
    SA_STEP_START_S,
    SA_STEP_LINE,
    SA_STEP_END_S,
    SA_STEP_OK
};
enum S_CURVE_B_STATUS
{
    SB_STEP_START_S,
    SB_STEP_LINE,
    SB_STEP_END_S,
    SB_STEP_OK
};

                               
#define LINE_FRQ_CHANGE_STRTUCT_DEFALUTS   \
{                                          \
	(void (*)(void *))LineChangeCalc       \
}

LINE_CHANGE_STRUCT frqLine = LINE_FRQ_CHANGE_STRTUCT_DEFALUTS;

int32 frqCurAimOld;
int32 accTimeOld;
int32 decTimeOld;
// 目标频率发生改变标志，应该是S曲线的时间生效标志
// 启动时，也应该为1
Uint16 bFrqCurAimChg;

LOCALD int32 SCurveOridinaryCalc(int32 frq0, int32 accDecTime);
LOCALD int32 SCurveSpecialCalc(int32 frq0, int32 accDecTime);

//=====================================================================
//
// 加减速计算
// 参数：accTime -- 加速时间，单位: 同功能码
//       decTime -- 减速时间，单位: 同功能码
//       mode    -- 加减速方式，直线加减速/S曲线加减速
// 输入：
//       frqTmp -- 当前瞬时频率
//       frqCurAim -- 目标频率
// 输出：
//       frqTmp -- 当前瞬时频率
//
// 注意：frqCurAim一定要明确给定。
//
//=====================================================================
void AccDecFrqCalc(int32 accTime, int32 decTime, Uint16 mode)
{
    int32 accDecTime; 
    int32 frq0;
   
    if (((funcCode.code.ovGain) || (funcCode.code.ocGain))
        && (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_VF)   // VF运行
        && (motorFc.motorPara.elem.motorType != MOTOR_TYPE_PMSM)  // 非同步机
        && (!funcCode.code.droopCtrl)                             // 下垂控制无效
        && (runStatus != RUN_STATUS_TUNE)                         // 不处于调谐状态
        && (!((frqCurAim == 0) && (ABS_INT32(frqRun) <= 50)))       // 不为目标频率为0且运行频率小于0.5Hz情况
    ) // 过压失速，过流失速增益都为0
	{
		frq0 = frqRun;
    }
    else
    {
        frq0 = frqTmp;
		frqVFRunRemainder = 0;
    }

    
    if (frqCurAimOld != frqCurAim)  // 目标频率发生改变
    {
        frqCurAimOld = frqCurAim;
        bFrqCurAimChg = 1;
    }

    if (frqCurAim == frq0)                      // 恒速
    {
        runFlag.bit.accDecStatus = CONST_SPEED;
        frqTmpFrac = frqCurAimFrac;             // 瞬时频率的小数点   
    }
    else
    {
        // 加速
        if (((frq0 >= 0) && (frqCurAim > frq0))
            || ((frq0 <= 0) && (frqCurAim < frq0))) 
        {
            runFlag.bit.accDecStatus = ACC_SPEED;
            accDecTime = accTime;
         	benchFrq = ABS_INT32(frqCurAim);
            if (accTimeOld != accTime)
            {
                bFrqCurAimChg = 1;
            }
        }
         // 减速
        else                   
        {
            runFlag.bit.accDecStatus = DEC_SPEED;
            accDecTime = decTime;
            if (decTimeOld != decTime)
            {
                bFrqCurAimChg = 1;
            }
        }


        // 基准频率
        if (funcCode.code.accDecBenchmark == 0)
        {    
            benchFrq = maxFrq;    // 最大频率
        }
        else if (funcCode.code.accDecBenchmark == 2)
        {
            benchFrq = 100 * decNumber[funcCode.code.frqPoint]; // 100Hz
        }
        
#if DEBUG_F_S_CURVE_A || DEBUG_F_S_CURVE_A
        // 特殊S曲线，加减速时间不能太长。普通二次方S曲线，没有此限制
        if ((FUNCCODE_accDecSpdCurve_S_CURVE_B == mode) && 
            ((accDecTime > (int32)100 * TIME_UNIT_MS_PER_SEC / timeBench)    // 加减速时间大于_s
            || (ABS_INT32(frqCurAim) > 6 * motorFc.motorPara.elem.ratingFrq))) // 设定频率从大于_额定频率
        {
            mode = FUNCCODE_accDecSpdCurve_LINE;
        }
#endif
	    accDecTime = accDecTime * (timeBench / RUN_CTRL_PERIOD);// 转换成ticker
        //accDecTime = accDecTime * (timeBench * (Uint16)(1 / RUN_CTRL_PERIOD));// 转换成ticker
        
        if (FUNCCODE_accDecSpdCurve_LINE == mode) // 直线加减速
        {
            frqLine.aimValue = frqCurAim;
            frqLine.tickerAll = accDecTime;
            frqLine.maxValue = benchFrq;
            frqLine.curValue = frq0;
            frqLine.calc(&frqLine);
            frq0 = frqLine.curValue;
            //frqTmpFrac = ((int64)frqLine.remainder << 15) / frqLine.tickerAll;
        }
#if DEBUG_F_S_CURVE_A 
        else if (FUNCCODE_accDecSpdCurve_S_CURVE_A == mode)     // 普通二次方S曲线
        {
            frq0 = SCurveOridinaryCalc(frq0, accDecTime);
        }
#endif
#if DEBUG_F_S_CURVE_B        
        else    // 特殊的S曲线
        {
            frq0 = SCurveSpecialCalc(frq0, accDecTime);
        }
#endif

    }

    bFrqCurAimChg = 0;
    decTimeOld = decTime;
    accTimeOld = accTime;
    frqTmp = frq0;
}




//=====================================================================
//
// 普通二次方S曲线加减速计算
// 参数：accDecTime -- 加减速时间，单位: ticker
//
// 输入：
//       frq0 -- 上一拍瞬时频率
//       frqCurAim -- 目标频率，全局变量
// 输出：
//       frq0 -- 当前拍瞬时频率
//
// 注意：frqCurAim一定要明确给定。
//
//       S曲线加速时间 = 直线加减速时间
//
//=====================================================================
LOCALF int32 SCurveOridinaryCalc(int32 frq0, int32 accDecTime)
{
#if DEBUG_F_S_CURVE_A
    static enum S_CURVE_A_STATUS sCurveAStatus;
    static int32 ticker;
    static int32 t[3];      // S曲线起始段、直线段、S曲线结束段的时间, ticker
    static int32 a[3];      // S曲线起始段、直线段、S曲线结束段的系数
    static Uint16 q[3];     // a[]对应的Q格式
    static int64 remainder;
    int32 delta;
    int64 tmp;
    int32 tmp2;

    if (!accDecTime)    // 加减速时间为0
    {
        frq0 = frqCurAim;
        frqTmpFrac = 0;
        return frq0;
    }

    if (bFrqCurAimChg)
    {
        int32 tickerAll;  // 整个阶段需要的时间，调用次数为单位
        Uint16 startTime;
        Uint16 lineTime;   // 直线段时间的2倍
        Uint16 endTime;
        int32 frqAimDelta;
        int64 tmp1;
        int64 tmp3;
        
        sCurveAStatus = SA_STEP_START_S;
        ticker = 0;
        remainder = 0;

        startTime = funcCode.code.sCurveStartPhaseTime;
        endTime = funcCode.code.sCurveEndPhaseTime;
        lineTime = 2000 - startTime - endTime;
        frqAimDelta = frqCurAim - frq0;
#define Q   16
        tmp3 = ((int64)accDecTime * ABS_INT32(frqAimDelta) << Q) / (int32)benchFrq;
        tickerAll = tmp3 >> Q;
        t[0] = ((int64)tickerAll * startTime + 0) / 1000;  //186.
        t[2] = ((int64)tickerAll * endTime + 0) / 1000;    //407.
        t[1] = tickerAll - (t[0] + t[2]);

        q[0] = 20;  // 初始化a[0], a[1]对应的Q格式
        q[1] = 35;

        // 计算a[1], 直线段的一次项系数，同时更新对应的Q格式
        tmp = ((int64)benchFrq * 2000 << q[1]) / ((int64)accDecTime * lineTime); //310.
        while (tmp >= 1L << 18)
        {
            tmp >>= 1;
            q[1]--;
        }
        a[1] = tmp;
        
        // 计算a[0]，S曲线起始段的二次项系数
        tmp1 = ((int64)a[1] * 500 << (q[0] + Q)) / tmp3;
#undef Q
        q[0] += q[1];
        q[2] = q[0];
        tmp = tmp1 / startTime;
        while (tmp >= 1L << 18)
        {
            tmp >>= 1;
            q[0]--;
        }
        a[0] = tmp;
        
        // 计算a[2]，S曲线结束段的二次项系数
        tmp = tmp1 / endTime;
        while (tmp >= 1L << 18)
        {
            tmp >>= 1;
            q[2]--;
        }
        a[2] = tmp;
    }
    
    if (sCurveAStatus <= SA_STEP_END_S)
    {
        if (ticker >= t[sCurveAStatus])     // 该段已经完成
        {
            ticker = 0; // 初始化成0
            sCurveAStatus++;
            
            // 下一段时间为0
            while ((t[sCurveAStatus] == 0) && (SA_STEP_OK != sCurveAStatus))
            {
                sCurveAStatus++;
            }
            
            remainder = 0;
        }
        
        ticker++;
        
        if (SA_STEP_START_S == sCurveAStatus)
        {
            tmp2 = 2 * ticker - 1;
        }
        else if (SA_STEP_LINE == sCurveAStatus)
        {
            tmp2 = 1;
        }
        else //if (SA_STEP_END_S == sCurveAStatus)
        {
            tmp2 = 2 * t[2] - 2 * ticker + 1;     // 仔细推导，这里为+1, 而不是-1/0
        }
        
        tmp = (int64)tmp2 * a[sCurveAStatus] + remainder;
        delta = tmp >> q[sCurveAStatus];
        remainder = tmp - ((int64)delta << q[sCurveAStatus]);

        if (q[sCurveAStatus] >= 15)
        {
            frqTmpFrac = remainder >> (q[sCurveAStatus] - 15);
        }
        else
        {
            frqTmpFrac = remainder << (15 - q[sCurveAStatus]);
        }
        
        if (frq0 < frqCurAim)
        {
            frq0 += delta;
            if (frq0 > frqCurAim)   // 防止超过目标频率
            {
                frq0 = frqCurAim;
            }
        }
        else
        {
            frq0 -= delta;
            frqTmpFrac = -frqTmpFrac;
            
            if (frq0 < frqCurAim)   // 防止超过目标频率
            {
                frq0 = frqCurAim;
            }
        }
    }
    
    if (SA_STEP_OK == sCurveAStatus)
    {
        frq0 = frqCurAim;
    }
#endif
    return frq0;
}


//=====================================================================
//
// 特殊的S曲线，参考三菱S曲线B
// 
//=====================================================================
LOCALF int32 SCurveSpecialCalc(int32 frq0, int32 accDecTime)
{
#if DEBUG_F_S_CURVE_B
    static enum S_CURVE_B_STATUS sCurveBStatus;
    const int16 m1 = 13848;        // Q15, m = 1/(16*0.7)+1/3, 也可能是5/12
    const int16 fm = 11483;        // Q15, (9*m-1)/8
    //int32 A = 688;
    //int32 B = 530;
    //int16 Sden = 1000;
    const int16 A = 21;
    const int16 B = 16;
    const int16 Sden = 30;
    //  A = 750;
    //  B = 500;
    static int32 ticker;
    static int32 tickerAll1;  // 拐点前的S曲线的tickerAll
    static int32 tickerAll2;
    static int32 tickerAll3;
    static int32 inflexionFrq;            // 拐点
    static int32 pointFrq;
    static int64 rem1;
    static int64 remainder;
    static int32 delta;
    static int32 deltaFrqAim; // 频率的变化值
    Uint16 tmp0;
    int16 flag = 0;
    int16 s;
    int32 tmp;
    int32 frqTmpOld = frq0;
    int32 a;
    int32 b;

    if (!accDecTime)    // 加减速时间为0
    {
        frq0 = frqCurAim;
        return frq0;
    }
    
    if (bFrqCurAimChg)
    {
        if (frq0 > 0)
        {
            inflexionFrq = motorFc.motorPara.elem.ratingFrq;
        }
        else if (frq0 < 0)
        {
            inflexionFrq = -(int32)motorFc.motorPara.elem.ratingFrq;
        }
        else if (frqCurAim > 0)
        {
            inflexionFrq = motorFc.motorPara.elem.ratingFrq;
        }
        else if (frqCurAim < 0)
        {
            inflexionFrq = -(int32)motorFc.motorPara.elem.ratingFrq;
        }
        
        pointFrq = ((inflexionFrq * fm) >> 15); // 二次曲线与直线的交点

        tickerAll2 = (int32)accDecTime * ABS_INT32(inflexionFrq) / (int32)benchFrq;
        tickerAll1 = (tickerAll2 * m1) >> 15; // m = 1/(16*0.7)+1/3

        tmp = 0;
        if (runFlag.bit.accDecStatus == ACC_SPEED) // 加速
        {
            if (ABS_INT32(frqCurAim) > ABS_INT32(inflexionFrq))
            {
                tmp = frqCurAim;
            }
        }
        else
        {
            if (ABS_INT32(frq0) > ABS_INT32(inflexionFrq))
            {
                tmp = frq0;
            }
        }

        if (tmp)
        {
            // (4/9*(f/fb)^2 + 5/9) * T
#define QQ1 13                  // 频率达到 8*inflexionFrq, 会溢出
            tmp = ((tmp << QQ1) / inflexionFrq);
            tickerAll3 = ((((tmp * tmp) >> (QQ1-2)) + (5UL << QQ1)) * tickerAll2 / 9 + (1UL << (QQ1-1))) >> QQ1;
#undef QQ1
        }
        else
        {
            tickerAll3 = tickerAll2;
        }

        if (ABS_INT32(frq0) <= ABS_INT32(pointFrq)) // 位于二次曲线
        {
            Uint32 tmp;
#define QQ1 15
            // 求解时间，一元二次方程求根
            // ((B^2 + 4*A*f)^0.5 - B)/(2*A)
            tmp = (((((Uint64)B*B) << (2*QQ1)) / (Sden*Sden))
                   + (4*((((Uint64)A) << (2*QQ1)) * frq0) / (Sden * inflexionFrq)));
            tmp0 = qsqrt(tmp);
            ticker = (((((int32)tmp0 + 3)*Sden - ((int32)B<<QQ1)) * tickerAll2 / A + (1UL << QQ1)) >> (QQ1+1));
#undef QQ1

            remainder = 0;
            deltaFrqAim = inflexionFrq;
            sCurveBStatus = SB_STEP_START_S;
        }
        else if (ABS_INT32(frq0) <= ABS_INT32(inflexionFrq)) // 位于直线
        {
            //(f/fb - fm) * 8/9 * t2 + t1 = t2* (f/fb - fm) * 8/9 * (1+m1)
            ticker = ((((((frq0 << 15) / inflexionFrq - fm) * 8 + (int32)m1 * 9)) * tickerAll2 / 9
                      + (1UL<<(15-1))) >> (15));
            
            deltaFrqAim = inflexionFrq - pointFrq;

            sCurveBStatus = SB_STEP_LINE;
        }
        else                    // 大于基准频率
        {
            ticker = tickerAll3;
            sCurveBStatus = SB_STEP_END_S;
        }
    }

    switch (sCurveBStatus)
    {
        case SB_STEP_START_S:
            if (runFlag.bit.accDecStatus == ACC_SPEED)
            {
                ticker++;
                s = 1;
                if (ticker < tickerAll1)
                {
                    flag = 1;
                }
            }
            else
            {
                ticker--;
                s = -1;
                if (ticker > 0)
                {
                    flag = 1;
                }
            }
            
            if (flag)           // 二次曲线
            {
                a = 2 * ticker - 1;
                b = 1;
                
                delta = (((int64)A * a + tickerAll2 * B * b) * deltaFrqAim + remainder)
                    / ((int64)tickerAll2 * tickerAll2 * Sden);
                remainder = (((int64)A * a + tickerAll2 * B * b) * deltaFrqAim + remainder)
                    % ((int64)tickerAll2 * tickerAll2 * Sden);
                frq0 += s*delta;
            }
            else
            {
                if (runFlag.bit.accDecStatus == ACC_SPEED)
                {
                    frq0 += s*delta;
                    deltaFrqAim = inflexionFrq - frq0;
                    rem1 = 0;
                    sCurveBStatus = SB_STEP_LINE;
                }
                else
                {
                    sCurveBStatus = SB_STEP_OK;
                }
            }
            
            break;
            
        case SB_STEP_LINE:
            if (runFlag.bit.accDecStatus == ACC_SPEED)
            {
                ticker++;
                s = 1;
                if (ticker < tickerAll2)
                    flag = 1;
            }
            else
            {
                ticker--;
                s = -1;
                if (ticker > tickerAll1 + 1)
                    flag = 1;
            }
            
            if (flag)           // 直线
            {
                delta = (deltaFrqAim + rem1) / (tickerAll2 - tickerAll1);
                rem1 = (deltaFrqAim + rem1) % (tickerAll2 - tickerAll1);

                frq0 += s*delta;
            }
            else
            {
                if (runFlag.bit.accDecStatus == ACC_SPEED)
                {
                    frq0 += s*delta;
                    sCurveBStatus = SB_STEP_END_S;
                }
                else
                {
                    frq0 += s*delta;
                    remainder = 0;
                    deltaFrqAim = inflexionFrq;
                    sCurveBStatus = SB_STEP_START_S;

                    ticker = tickerAll1 + 1;
                }
            }

            break;

        case SB_STEP_END_S:
            if (runFlag.bit.accDecStatus == ACC_SPEED)
            {
                ticker++;
                if (ticker < tickerAll3)
                    flag = 1;
            }
            else
            {
                ticker--;
                if (ticker > tickerAll2 + 0)
                    flag = 1;
            }
            
            if (flag)
            {
                Uint32 tmp;
#define QQ2 28
                // 3/2*inflexionFrq * ((t/T)-5/9)^0.5
                tmp = ((((int64)9 * ticker - 5 * tickerAll2) << QQ2) / (9 * tickerAll2));
                tmp0 = qsqrt(tmp);
                frq0 = (((int32)tmp0 + 3) * inflexionFrq * 3 ) >> (QQ2/2 + 1);
#undef QQ2
            }
            else
            {
                if (runFlag.bit.accDecStatus == ACC_SPEED)
                {
                    sCurveBStatus = SB_STEP_OK;
                }
                else
                {
                    frq0 = inflexionFrq;
                    deltaFrqAim = inflexionFrq - pointFrq;
                    rem1 = 0;
                    sCurveBStatus = SB_STEP_LINE;

                    ticker = tickerAll2 + 1;
                }
            }
            break;
            
        default:
            break;
    }

// 防止超过频率
    if (((frqCurAim > frqTmpOld) && (frq0 > frqCurAim))
        || ((frqCurAim <= frqTmpOld) && (frq0 < frqCurAim)))
    {
        frq0 = frqCurAim;
    }

    if (sCurveBStatus == SB_STEP_OK)
    {
        frq0 = frqCurAim;
    }
#endif
    return frq0;
}


//=====================================================================
//
// 加减速直线计算
//
//=====================================================================
void LineChangeCalc(LINE_CHANGE_STRUCT *p)
{
    int32 delta;

    if (p->curValue != p->aimValue)
    {
        if (!p->tickerAll)
        {
            p->curValue = p->aimValue;
        }
        else
        {
            delta = ((int32)p->maxValue + p->remainder) / p->tickerAll;
            p->remainder = ((int32)p->maxValue + p->remainder) % p->tickerAll;

            if (p->aimValue > p->curValue)
            {
                p->curValue += delta;
                if (p->curValue > p->aimValue)
                    p->curValue = p->aimValue;
            }
            else
            {
                p->curValue -= delta;
                if (p->curValue < p->aimValue)
                    p->curValue = p->aimValue;
            }
        }
    }
}
