#include "MotorDefine.h"
#include "SubPrgInclude.h"
#include "MotorInclude.h"

#pragma DATA_SECTION(gDebugBuffer, "debug_buffer");

#define MOTOR_SYSTEM_DEBUG      //调试代码开关
#define CPU_TIME_DEBUG          // CPU时间片调试

#ifdef MOTOR_SYSTEM_DEBUG
#ifdef TMS320F2808
#define  C_DEBUG_BUFF_SIZE	    5000		//调试缓冲区大小
#else   // 28035
#define C_DEBUG_BUFF_SIZE       1000
#endif
#else   

#define C_DEBUG_BUFF_SIZE   1

#endif

Uint gDebugBuffer[C_DEBUG_BUFF_SIZE];
Uint gDebugPoint = 0;
int  gDebugFlag;

#ifdef MOTOR_SYSTEM_DEBUG
Ulong   saveIndexFunc;      // 列表保存数据选择，需要CCS中定义
Ulong   saveIndexP;         // pVD1到pVD4保存数据的个数，需要CCS中定义
Uint    saveCurveNum;       // 保存曲线的条数，可以读
Uint    saveRstMem;         // 缓存复位用的变量
Uint    SaveTmCnt;
int     startSave;

int *   pVD1;
int *   pVD2;
int *   pVD3;
int *   pVD4;

// 上溢调用 DebugSaveDeal(0)
// 下溢调用 DebugSaveDeal(1)
// 2ms 调用 DebugSaveDeal(2)
// 05ms调用 DebugSaveDeal(3)
void DebugSaveDeal(Uint savePrgPos)
{
    Uint    save;
    
// 确定保存数据时间
    SaveTmCnt ++;
    if(savePrgPos == 2 &&                     // 2ms 进入
        gTestDataReceive.TestData17 > 3)      // 选择长时间间隔
    {
        if(SaveTmCnt >= (gTestDataReceive.TestData17/2))
        {
            SaveTmCnt = 0;  // 时间到，需要保存数据
        }
        else
        {
            return;         // 保存时间还没有到
        }
    }
    else if(gTestDataReceive.TestData17 != savePrgPos)  // 不需要保存
    {
        return;             
    }
    //else  需要保存数据

// 确定本次是否保存
    switch (gTestDataReceive.TestData12)          // Cf-11    when to save data
    {
        case 0:
            save = 0;   // don't save any data
            break;
        case 1:
            save = 1;   // always save data
            break;
        case 2:         // save data when motor run
            save = (gMainCmd.Command.bit.Start) ? 1 : 0;
            break;
        default:
            save = 0;
            break;
    }

// 循环保存
    if(gDebugPoint >= C_DEBUG_BUFF_SIZE)
	{
	    // Cf-13 need to save data roll back and conver
        gDebugPoint = (gTestDataReceive.TestData14) ? 0 : C_DEBUG_BUFF_SIZE;
	}

// 本次保存数据
    if(save)            // begin to save
    {
        Uint i = 0;
        Uint totalN = 0;

        //saveIndex.all = gTestDataReceive.TestData11;        // Cf-10    select save content

        if(gTestDataReceive.TestData13)             // Cf-12 auto save, 保存pVD1 - pVD4中的数据
        {
            totalN = 0;
            if(saveIndexP == 1)
            {
                SaveDebugData16(*pVD1);
                totalN = 1;
            }
            else if(saveIndexP == 2)
            {
                SaveDebugData16(*pVD1);
                SaveDebugData16(*pVD2);
                totalN = 2;
            }
            else if(saveIndexP == 3)
            {
                SaveDebugData16(*pVD1);
                SaveDebugData16(*pVD2);
                SaveDebugData16(*pVD3);
                totalN = 3;
            }
            else if(saveIndexP == 4)
            {
                SaveDebugData16(*pVD1);
                SaveDebugData16(*pVD2);
                SaveDebugData16(*pVD3);
                SaveDebugData16(*pVD4);
                totalN = 4;
            }
            
        }
        else                                        // 通过功能码从地址列表中选择地址
        {
            for(i = 0; i < 32; i++)
            {
                
                if(saveIndexFunc & (1L<<i))
                {
                    
                    totalN ++;
                }
            }
        }
        saveCurveNum = totalN;
    }

// 是否复位缓存
    if(gTestDataReceive.TestData15 && saveRstMem == 0)  // Cf-14 buffer reset
    {
        ResetDebugBuffer();
        saveRstMem ++;
    }
    if(saveRstMem && gTestDataReceive.TestData15 == 0)  // need Cf-14 to reset itself
    {
        saveRstMem = 0;
    }
}

void SaveDebugData16(Uint data)
{
    if(gDebugPoint >= C_DEBUG_BUFF_SIZE)
	{
        //gDebugPoint = 0;
	    gDebugPoint = C_DEBUG_BUFF_SIZE-1;
	}
    //gDebugBuffer[0] = gDebugPoint;
	gDebugBuffer[gDebugPoint++] = data;
}

void SaveDebugData32(Ulong data)
{
	Ulong * p;
	gDebugFlag = 1;

	p = (Ulong *)(&gDebugBuffer);
	if(gDebugFlag)
	{
		if(gDebugPoint >= C_DEBUG_BUFF_SIZE/2)	gDebugPoint = C_DEBUG_BUFF_SIZE/2 - 2;
		p[gDebugPoint++] = data;
	}
}

void ResetDebugBuffer(void)
{
    int m_index;
    
    for(m_index=0;m_index<(C_DEBUG_BUFF_SIZE-1);m_index++)
    {
        gDebugBuffer[m_index] = 0;
    }
	gDebugPoint = 0;
}
#endif

