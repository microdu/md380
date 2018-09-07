/****************************************************************
文件功能：主程序中调用的大的模块函数
文件版本：
最新更新：
	
****************************************************************/
 
#include "MotorDefine.h"
#include "SubPrgInclude.h"

#define  SETOVM asm(" SETC OVM")
#define  CLROVM asm(" CLRC OVM")

/****************************************************************
函数说明：反正切函数，该函数输入x，y，求得的反正切角度以及4象限的角度
****************************************************************/
int atan(int x, int y)
{
	int  result;
	long m_Input;

	if(x == 0)
	{
		if(y < 0)			
		{
			return(-16384);
		}
		else
		{
			return(16384);
		}
	}
	m_Input = (((long)y)<<16)/x;
	result = qatan(m_Input);
	if(x < 0)
	{
		result += 32768;
	}
	return result;
}

/****************************************************************
函数说明：PID函数(暂时不考虑D增益的作用)
输入偏差为int型变量
输出结果为long型变量。右移16位得到需要的结果
比例增益在pid内部倍左移 4位
****************************************************************/
void PID(PID_STRUCT * pid)
{
	long m_Max,m_Min,m_Out,m_OutKp,m_OutKi;
    long vMax = 0x7FFFFFFF;
    
    SETOVM;
 	m_Max = ((long)pid->Max)<<16;						// 最大值
	m_Min = ((long)pid->Min)<<16;						// 最小值
	
	m_OutKp = (long)pid->KP * (long)pid->Deta;			// 比例
	m_OutKp = __IQsat(m_OutKp, (vMax>> (4+ pid->QP)), -(vMax>> (4+ pid->QP))); //保证使用算术右移
    m_OutKp = m_OutKp << (4+ pid->QP);
    
 	m_OutKi = (long)pid->KI * (long)pid->Deta;// 积分
    m_OutKi = __IQsat(m_OutKi, (vMax >> pid->QI), -(vMax >> pid->QI));
    m_OutKi = m_OutKi << pid->QI;
    
    // 饱和情况下的去饱和处理
    if((m_OutKp > m_Max) && (pid->Total > 0))
    {
        pid->Total -= (pid->Total>>8) + 1;  //加1去掉滤波静差
        m_OutKi = 0;
    }
    else if((m_OutKp < m_Min) && (pid->Total < 0))
    {
        pid->Total -= (pid->Total>>8) - 1; 
        m_OutKi = 0;
    }
	pid->Total += m_OutKi;	 
	m_Out       = pid->Total + m_OutKp;
 	pid->Out    = __IQsat(m_Out,m_Max,m_Min);
    pid->Total  = __IQsat(pid->Total,m_Max,m_Min);

    CLROVM;
}
/****************************************************************
函数说明：
PID函数(暂时不考虑D增益的作用)
输入偏差为long型变量;上下限需要与偏差的Q值相同

输出为llong型变量，右移16位得到需要的结果
*考虑增加饱和情况下的去饱和处理*

比例增益在pid内部倍左移 4位
****************************************************************/
void PIDLongRegulate(PID_STRUCT_LONG * pid)
{
	llong  m_Max,m_Min,m_Out,m_OutKp,m_OutKi;
 	m_Max = ((llong)pid->Max)<<16;						//最大值
	m_Min = ((llong)pid->Min)<<16;						//最小值
    m_OutKp = (llong)pid->KP * (llong)pid->Deta << 4;		//比例
	
	m_OutKi = (llong)pid->KI * (llong)pid->Deta;

// 饱和情况下的去饱和处理
    if(((m_OutKp > m_Max) && (pid->Total > 0)) ||
       ((m_OutKp < m_Min) && (pid->Total < 0))) 
    {
        pid->Total -= (pid->Total>>8);
        m_OutKi = 0;
    }

	pid->Total += m_OutKi;		
	m_Out      = pid->Total + m_OutKp;
    if(m_Out < m_Min)               pid->Out = m_Min;
    else if(pid->Out > m_Max)       pid->Out = m_Max;
    else                            pid->Out = m_Out;

    if(pid->Total < m_Min)          pid->Total = m_Min;
    else if(pid->Total > m_Max)     pid->Total = m_Max;      
}


void PID32(PID32_STRUCT * pid)
{
	long  m_OutKp,m_OutKi;
	long  mTotalMax,mTotalMin;

	//if(pid->Deta == 0)		return;

	//计算比例作用，并调整积分作用
	m_OutKp = ((llong)pid->KP * (llong)pid->Deta) >> (16-4);	
	//调整积分的上(下)限
	if(pid->Deta > 0)		
	{
		mTotalMax = pid->Max - m_OutKp;
		if(mTotalMax < 0)	mTotalMax  = 0;

		mTotalMin = pid->Min;
	}
	else
	{
		mTotalMin = pid->Min - m_OutKp;
		if(mTotalMin > 0)	mTotalMin  = 0;

		mTotalMax = pid->Max;
	}

    //计算积分作用
	m_OutKi = ((llong)pid->KI * (llong)pid->Deta)>>16;
	pid->Total = pid->Total + m_OutKi;
	pid->Total = __IQsat(pid->Total, mTotalMax, mTotalMin);

	//计算PID的输出
	pid->Out = pid->Total + m_OutKp;
	pid->Out = __IQsat(pid->Out, pid->Max, pid->Min);
}

/****************************************************************
	剔除毛刺滤波处理程序
*****************************************************************/
void BurrFilter(BURR_FILTER_STRUCT * filter)
{
	int 	m_Deta;

	m_Deta = abs((filter->Input) - (filter->Output));
	if(m_Deta > filter->Err)
	{
		filter->Err = filter->Err << 1;
	}
	else
	{
		filter->Output = filter->Input;
	  	if(m_Deta < (filter->Err >> 1))
		{
			filter->Err = filter->Err >> 1;
		}
	}
	filter->Err = (filter->Err > filter->Max)?filter->Max:filter->Err;
	filter->Err = (filter->Err < filter->Min)?filter->Min:filter->Err;
}

/****************************************************************
	滑动滤波程序(根据滤波时间常数设定加全平均的数据个数)
*****************************************************************/
void SlipFilter(CUR_LINE_STRUCT_DEF * pCur)
{
	int  m_Index,m_IndexStart,m_Coff,m_TotalCoff;
	long m_Total;
	CUR_LINE_STRUCT_DEF * m_pCur = pCur;

	for(m_Index = 0;m_Index<(16-1);m_Index++)
	{
		m_pCur->Data[m_Index] = m_pCur->Data[m_Index+1];
	}
	m_pCur->Data[15] = m_pCur->Input;

	m_Total = 0;
	m_TotalCoff = 0;
	m_IndexStart = 16 - m_pCur->FilterTime;
	for(m_Coff = 0;m_Coff<(m_pCur->FilterTime);m_Coff++)
	{
		m_Total += (long)m_pCur->Data[m_IndexStart + m_Coff] * (m_Coff +1);
		m_TotalCoff += (m_Coff+1);
	}
	m_pCur->Output = m_Total/m_TotalCoff;
}


