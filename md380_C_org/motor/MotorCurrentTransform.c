/****************************************************************
文件功能：电流、电压的坐标变换
文件版本：
最新更新：
	
****************************************************************/
#include "MotorCurrentTransform.h"
#include "MotorInclude.h"
#include "MotorEncoder.h"

//extern MT_STRUCT_Q24           gIMTQ24_obs;
//extern MT_STRUCT_Q24           gIMTQ12_obs;
/*******************************************************************
    由于采用了标么值系统，要求所有的坐标变换都必须保证幅值相等的变换，
理由：标么值系统下，基值确保恒磁场变换。
********************************************************************/
/*******************************************************************
Date Type Q24(保证幅值不变的坐标变换)(三相有效值为1的弦波，坐标变换后值为1)
	Alph= U * (1/2)^0.5 
	Beta= (3^0.5/2) * (U + 2*V)
	UVW电流是以峰值表示的，ALPH BETA和M T轴电流都是有效值概念。
********************************************************************/
void inline UVWToAlphBetaAxes(UVW_STRUCT_Q24 * uvw, ALPHABETA_STRUCT * AlphBeta)
{
	AlphBeta->Alph = ((llong)uvw->U * 23170L)>>15;	

	AlphBeta->Beta = ((llong)((long)uvw->V - (long)uvw->W) * 13377L)>>15;
}

/*******************************************************************
Date Type Q12 （q轴超d轴90度）
	d= cos(theta)*alph + sin(theta)*beta;
	q= -sin(theta)*alph + cos(theta)*beta;
********************************************************************/
void AlphBetaToDQ(ALPHABETA_STRUCT * AlphBeta, int angle, MT_STRUCT_Q24 * MT)
{
	int m_sin,m_cos;

	m_sin  = qsin(angle);
	m_cos  = qsin(16384 - angle);
	MT->M = ( ((llong)m_cos * (llong)(AlphBeta->Alph)) + 
	          ((llong)m_sin * (llong)(AlphBeta->Beta)) )>>15;
	MT->T = (-((llong)m_sin * (llong)(AlphBeta->Alph)) + 
	          ((llong)m_cos * (llong)(AlphBeta->Beta)) )>>15;
}

/*******************************************************************
Date Type Q12
	A= (d*d + q*q)^0.5
	q= atan(q/d)
********************************************************************/
void DQToAmpTheta(MT_STRUCT * MT,AMPTHETA_STRUCT * AmpTheta)
{
	long m_Input;

	m_Input = (((long)MT->M * (long)MT->M) + ((long)MT->T * (long)MT->T));
	AmpTheta->Amp = (Uint)qsqrt(m_Input);

	AmpTheta->Theta = atan(MT->M,MT->T);
}

/*************************************************************
	电流变换程序
*************************************************************/
void ChangeCurrent(void)
{
    //Ulong   m_Long;    
    int temp;


    // 获取三相电流瞬时值, 三相电流转换为定子两相坐标轴下的电流
	UVWToAlphBetaAxes((UVW_STRUCT_Q24*)&gIUVWQ24,(ALPHABETA_STRUCT*)&gIAlphBeta);
    gIAlphBetaQ12.Alph = gIAlphBeta.Alph>>12;
    gIAlphBetaQ12.Beta = gIAlphBeta.Beta>>12;
	// 定子两相坐标轴下电流转换为DQ下的电流
	AlphBetaToDQ((ALPHABETA_STRUCT*)&gIAlphBeta,(gPhase.IMPhase>>16), &gIMTQ24);
    
        //AlphBetaToDQ((ALPHABETA_STRUCT*)&gIAlphBeta,(gPhase.IMPhase>>16), &gIMTQ24_obs);
	gIMTQ12.M = Filter2((gIMTQ24.M>>12), gIMTQ12.M);
	gIMTQ12.T = Filter2((gIMTQ24.T>>12), gIMTQ12.T);

   // gIAmpTheta.Theta = atan(gIMTSetQ12.M, gIMTSetQ12.T);	//计算MT轴夹角

    gIAmpTheta.Theta = atan(gIMTQ12.M, gIMTQ12.T);	//计算MT轴夹角

    temp = gOutVolt.VoltPhaseApply - gIAmpTheta.Theta;
    gIAmpTheta.PowerAngle = Filter8(temp, gIAmpTheta.PowerAngle);
}

