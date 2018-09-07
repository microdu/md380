/***********************************Inovance***********************************
功能描述（Function Description）:
最后修改日期（Date）：
修改日志（History）:（以下记录为第一次转测试后，开始记录）
	作者 		时间 		更改说明
1 	xx 		xxxxx 		xxxxxxx
2 	yy 		yyyyy 		yyyyyyy
************************************Inovance
***********************************/
#ifndef ZERO_POS_CTL_INCLUDE_H
#define ZERO_POS_CTL_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes 
------------------------------------------------------------------*/
#include "DataTypeDef.h"

/* Private typedef 
-----------------------------------------------------------*/
typedef struct ZERO_POS_STRUCT_DEF {
    u16     Flag;               /*Flag = 1 表示零伺服启动*/

	s16  	PosInit;            /*零伺服零点位置*/
	s16  	PosLast;            /*零伺服时候移动的位置,上一拍*/
	s16  	Pos;                /*零伺服时候移动的位置*/

    s16     KPPos;              /*零伺服的位置KP*/
    s16     KPSpeed;            /*零伺服的速度KP*/
	s16  	KPDecCoff;          /*零伺服KP调节器的减小比例*/

	s16  	ItSet;              /*零伺服控制时候的输出力矩*/
	s16  	ItKpPos;            /*位置调节部分提供的力矩*/
	s16  	ItKpSpeed;          /*速度控制部分提供的力矩*/

    s16     Speed;              /*计算的速度*/
    u32     TimeLast;           /*上一拍的时间*/
    u32     DetaTime;           /*每脉冲的时间*/
    
}ZERO_POS_STRUCT;	//零伺服控制结构


/* Private define 
------------------------------------------------------------*/
//取绝对值的函数(适用所有数据类型)
#define abs(x)  	((x>0)?(x):-(x))
//取大值函数(适用所有数据类型)
#define Max(x,y)	((x>y)?(x):(y))
//取小值函数(适用所有数据类型)
#define Min(x,y)	((x<y)?(x):(y))

/* Private macro 
-------------------------------------------------------------*/


/* Private function prototypes 
-----------------------------------------------*/
extern ZERO_POS_STRUCT          gZeroPos;

extern s16 Filter(s16 LastOne, s16 Input, s16 Coff);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

/******************************* END OF FILE
***********************************/


