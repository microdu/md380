//======================================================================
//
// Time-stamp: <2012-01-04 14:30:42  Shisheng.Zhi, >
//
//======================================================================

#include "f_funcCode.h"
#include "f_runSrc.h"
#include "f_main.h"
#include "f_ui.h"
#include "f_debug.h"




//=================================================================
// 通用测试函数
#define SCOPE_NUM_KK    1
//#define SCOPE_NUM_KK    1700
//#define SCOPE_NUM_KK    2000
//#define SCOPE_NUM_KK    3400
//#define SCOPE_NUM_KK    4500

#if 0//F_DEBUG_RAM
#define SCOPE_NUM0  1
#define SCOPE_NUM1  1
#define SCOPE_NUM2  1
#define SCOPE_NUM3  1
#define SCOPE_NUM4  1
#define SCOPE_NUM5  1
#elif 1
#define SCOPE_NUM0  SCOPE_NUM_KK
#define SCOPE_NUM1  SCOPE_NUM_KK
#define SCOPE_NUM2  SCOPE_NUM_KK
#define SCOPE_NUM3  SCOPE_NUM_KK
#define SCOPE_NUM4  1
#define SCOPE_NUM5  1
#elif 1
#define SCOPE_NUM0  SCOPE_NUM_KK
#define SCOPE_NUM1  SCOPE_NUM_KK
#define SCOPE_NUM2  SCOPE_NUM_KK
#define SCOPE_NUM3  SCOPE_NUM_KK
#define SCOPE_NUM4  SCOPE_NUM_KK
#define SCOPE_NUM5  SCOPE_NUM_KK
#endif

#define SCOPE_START_RUN_CLEAR       1   // 启动时数组所有值清零
#define SCOPE_START_RUN_ARRAY_0     1   // 启动时，从数组的起始开始保存
#define SCOPE_SAVE_ONLY_RUN         1   // 仅运行时保存
#define SCOPE_ARRAY_USER_OVER_LOOP  1   // 数组使用完毕，自动从数组的起始重新循环

//
// 调试时，有可能需要调整的变量
// SCOPE_K
// *matlabDebug[]
// scopeInterval[]
// 


#define SCOPE_NUMBER    6   // 模拟示波器通道数

Uint16 scope0[SCOPE_NUM0];              // 调试变量的示波器数组
Uint16 scope1[SCOPE_NUM1];
Uint16 scope2[SCOPE_NUM2];
Uint16 scope3[SCOPE_NUM3];
Uint16 scope4[SCOPE_NUM4];
Uint16 scope5[SCOPE_NUM5];
Uint16 scopeIndex[SCOPE_NUMBER];                // 当前存储的序号
Uint16 scopeInterval[SCOPE_NUMBER] =            // 每隔_拍保存一个数据
#if 0
{1, 1, 1, 1, 1, 1};
#elif 1
{2, 2, 2, 2, 2, 2};
#elif 0
{4, 4, 4, 4, 4, 4};
#elif 1
{8, 8, 8, 8, 8, 8};
#elif 1
{12, 12, 12, 12, 12, 12};
#elif 1
{16, 16, 16, 16, 16, 16};
#endif

//-------------------
extern int32 frq;
extern int32 frqRun;
extern int32 frqTmp;
extern int32 pcRef;
extern Uint16 pcErrorDisp;
Uint16 matlabDebugRsvd;

extern int32 mPosAct;
extern int32 mVelAct;
extern int32 sPosAct;
extern int32 sVelAct;

extern Uint16 pcRefDisp;
extern Uint16 pcFdbDisp;
extern Uint16 mPosActDisp;
extern Uint16 sPosActDisp;
Uint16 *matlabDebug[SCOPE_NUMBER] = 
{
#if 1
    (Uint16 *)&frq,
    (Uint16 *)&frqRun,
    (Uint16 *)&frqTmp,
    (Uint16 *)&matlabDebugRsvd,
    (Uint16 *)&matlabDebugRsvd,
    (Uint16 *)&matlabDebugRsvd,
#elif 0
    (Uint16 *)&gSendToMotor05MsDataBuff[4],
    (Uint16 *)&gSendToFunctionDataBuff[5],
    (Uint16 *)&frq2Core,
    (Uint16 *)&matlabDebugRsvd,
    (Uint16 *)&matlabDebugRsvd,
    (Uint16 *)&matlabDebugRsvd,
#elif 1
    (Uint16 *)&frq,
    (Uint16 *)&frqRun,
    (Uint16 *)&frqTmp,
    (Uint16 *)&pcErrorDisp,
    (Uint16 *)&mVelAct,
    (Uint16 *)&pcRefDisp,
    //(Uint16 *)&matlabDebugRsvd,
    //(Uint16 *)&pcFdbDisp,
#elif 1
    (Uint16 *)&frqTmp,
    (Uint16 *)&frqRun,
    (Uint16 *)&frq,
    (Uint16 *)&mVelAct,
    (Uint16 *)&pcRefDisp,
    (Uint16 *)&pcErrorDisp,
    //(Uint16 *)&pcFdbDisp,
#elif 1
    (Uint16 *)&mPosActDisp,
    (Uint16 *)&mVelAct,
    (Uint16 *)&sPosActDisp,
    (Uint16 *)&sVelAct,
    (Uint16 *)&pcErrorDisp,
    (Uint16 *)&frqRun,
#endif
};
//-------------------
//=================================================================




#define NUM_L_DEBUG   1
LOCALF Uint32 vDL[NUM_L_DEBUG];     // 调试变量


LOCALD void InitDebug(void);

// keyboard
#define SPI     1   // SPI
#define IOSIM   0   // IO simulate
#define keyValueDebugIndexMax 1
//extern LOCALF Uint16 keyValue;
LOCALF int16 keyValueDebug[keyValueDebugIndexMax];
LOCALF int16 keyValueDebugIndex;
LOCALD void InitKeyDebug(void);
LOCALD void DealKeyDebug(void);


// I2C
#define I2CDebugNum 1
#define I2C_IO      0
#define I2C_DSP     1
//LOCALF Uint16 passTimer;
//LOCALF Uint16 failTimer;
//LOCALF Uint16 passTimerAll;
LOCALF Uint16 I2CWrongTimer;
LOCALF Uint16 sendDataI2C[I2CDebugNum];
LOCALF Uint16 rcvDataI2C[I2CDebugNum];
LOCALD void InitI2cDebug(void);
LOCALD void TestI2cDebug(void);
#if 0
LOCALD void pass(void);
LOCALD void fail(void);
#endif


//===========
//Uint16 daValue[4];
//=========

#if 0
void BitAssign(Uint32 source, Uint32 *dest, int16 bit)
{
    if ((source) & (0x1UL << (bit)))
        (*dest) |= (0x1UL << (bit));
    else
        (*dest) &= (~(0x1UL << (bit)));
}
#endif


#if 1

#if 0
#define ATTRIBUTE_READ_AND_WRITE1        0   // (任何时候)可写
#define ATTRIBUTE_READ_ONLY_WHEN_RUN1    1   // 运行时只读
#define ATTRIBUTE_READ_ONLY_ANYTIME1     2   // 只读
struct  FUNC_ATTRIBUTE_BITS1
{                        // bits   description
   Uint16 point:3;       // 2:0    radix point,小数点。
                         //        (0.0000-100,00.000-011,000.00-010,0000.0-001,00000-000)
   Uint16 unit:3;        // 5:3    unit,单位
                         //        1-hz, 2-A, 3-RPM, 4-V, 6-%; 001-Hz, 010-A, 100-V
   Uint16 displayBits:3; // 8:6    5个数码管要显示的位数。0-显示1位，1-显示2位，...，4-显示5位
   Uint16 upperLimit:1;  // 9      1-参数由上限相关功能码限制
   Uint16 lowerLimit:1;  // 10     1-参数由下限相关功能码限制，0-直接由下限限制
   Uint16 writable:2;    // 12:11  参数读写特性，00-可以读写, 01-运行中只读，10-参数只读
   Uint16 signal:1;      // 13     符号，unsignal-0; signal-1
   Uint16 rsvd:2;        // 15:14  保留
};
union FUNC_ATTRIBUTE1
{
   Uint16                       all;
   struct FUNC_ATTRIBUTE_BITS1  bit;
};
#elif 1
#define ATTRIBUTE_READ_AND_WRITE        0   // (任何时候)可写
#define ATTRIBUTE_READ_ONLY_WHEN_RUN    1   // 运行时只读
#define ATTRIBUTE_READ_ONLY_ANYTIME     2   // 只读
struct  FUNC_ATTRIBUTE_BITS1
{                        // bits   description
   Uint16 point:3;       // 2:0    radix point,小数点。
                         //        0-无小数点，1-1位小数，...，4-4位小数
                         //        (0.0000-100,00.000-011,000.00-010,0000.0-001,00000-000)
   Uint16 unit:3;        // 5:3    unit,单位
                         //        1-hz, 2-A, 3-RPM, 4-V, 6-%; 001-Hz, 010-A, 100-V
   Uint16 displayBits:3; // 8:6    5个数码管要显示的位数。0-显示0位，1-显示1位，...，5-显示5位
   Uint16 upperLimit:1;  // 9      1-参数由上限相关功能码限制
   Uint16 lowerLimit:1;  // 10     1-参数由下限相关功能码限制，0-直接由下限限制
   Uint16 writable:2;    // 12:11  参数读写特性，00-可以读写, 01-运行中只读，10-参数只读
   Uint16 signal:1;      // 13     符号，unsignal-0; signal-1
   Uint16 rsvd:2;        // 15:14  保留
};

union FUNC_ATTRIBUTE1
{
   Uint16                      all;
   struct FUNC_ATTRIBUTE_BITS1  bit;
};
#endif

//union FUNC_ATTRIBUTE1 attributeDebug[FNUM_PARA];
int16 i;

LOCALD void ChgAttr(void);  // 改变属性命令字的bit位置
#endif


//=================================================================
//=================================================================

#define SCOPE_START_RUN_FLAG        (runFlag.bit.run)   // 当时是否运行标志，1-运行，0-不运行

//=====================================================================
//
// 通用测试函数
// *matlabDebug[sn]:    要保存数据的值
// sn:      要保存到哪个通道
//          0-scope0[]
//          1-scope1[]
//          2-scope2[]
//          3-scope3[]
//          4-scope4[]
//          5-scope5[]
//
//=====================================================================
void UniversalDebug(Uint16 sn)
{
    static Uint16 runOld[SCOPE_NUMBER];
    static Uint16 cnt[SCOPE_NUMBER];       // 每隔_次才保存数据，进行计数
    int16 j;

    Uint16 *pScope = scope0;        // 默认调试示波器的第1个数据，使用第1个数组
    Uint16 scopeNum = SCOPE_NUM0;

// SCOPE_NUMBER个通道
    if (sn >= SCOPE_NUMBER)
    {
        return;
    }

    switch (sn)
    {
        case 1:
            pScope = scope1;
            scopeNum = SCOPE_NUM1;
            break;
            
        case 2:
            pScope = scope2;
            scopeNum = SCOPE_NUM2;
            break;

        case 3:
            pScope = scope3;
            scopeNum = SCOPE_NUM3;
            break;

        case 4:
            pScope = scope4;
            scopeNum = SCOPE_NUM4;
            break;

        case 5:
            pScope = scope5;
            scopeNum = SCOPE_NUM5;
            break;

        default:
            break;
    }
    

    // 开始运行时保存数据，且全部清零
    if ((!runOld[sn]) && (SCOPE_START_RUN_FLAG))
    {
        cnt[sn] = 0;
        
#if SCOPE_START_RUN_ARRAY_0     // 启动时，从数组的起始开始保存
        scopeIndex[sn] = 0;
#endif
        
#if SCOPE_START_RUN_CLEAR       // 启动时数组所有值清零
        for (j = scopeNum - 1; j >= 0; j--)    // 清零
        {
            *(pScope + j) = 0;
        }
#endif
    }
    runOld[sn] = SCOPE_START_RUN_FLAG;

#if SCOPE_SAVE_ONLY_RUN         // 仅运行时保存
    if (!SCOPE_START_RUN_FLAG)
    {
        return;
    }
#endif

    if (++cnt[sn] >= scopeInterval[sn])    // 每隔_次保存数据
    {
        cnt[sn] = 0;

        if (scopeIndex[sn] < scopeNum)
        {
            *(pScope + scopeIndex[sn]) = *matlabDebug[sn]; // 保存数据至数组
            
            if (++scopeIndex[sn] >= scopeNum)
            {
#if SCOPE_ARRAY_USER_OVER_LOOP  // 数组使用完毕，自动从数组的起始重新循环
                scopeIndex[sn] = 0;
#elif 1                         // 停止保存
                scopeIndex[sn] = scopeNum-1;
#endif
            }
        }
    }
}
//=================================================================
//=================================================================




//=====================================================================
//
// 通用测试函数
//
//=====================================================================
void UniversalDebug32(Uint32 a)
{
#if 0
    static Uint16 i;

#if 1
    if (!SCOPE_START_RUN_FLAG)
        return;
#endif

    vDL[i] = a;
    if (++i >= NUM_L_DEBUG)
        i = 0;
#endif
}



LOCALF void InitDebug(void)
{
    InitKeyDebug();
    InitI2cDebug();

    ChgAttr();  //
}







//////////////////////////////////////////////////////////////
// keyboard
LOCALF void InitKeyDebug(void)
{
#if 0
    for (keyValueDebugIndex = 0;
         keyValueDebugIndex < keyValueDebugIndexMax;
         keyValueDebugIndex++)
        keyValueDebug[keyValueDebugIndex] = 0xffaa;

    keyValueDebugIndex = 0;
#endif
}

LOCALF void DealKeyDebug(void)
{
#if 0
    keyValueDebug[keyValueDebugIndex++] = keyValue;
    if (keyValueDebugIndex >= keyValueDebugIndexMax)
        keyValueDebugIndex = 0;
#endif
}
//////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////
// I2C 测试
LOCALF void InitI2cDebug(void)
{
#if 0
    int16 i;

#if 0
    int16 k[I2CDebugNum];
//    i = RAND_MAX;
//    srand(5);

    for (i = 0; i < I2CDebugNum; i++)
        k[i] = rand();
#endif
    for (i = 0; i < I2CDebugNum; i++)
    {
//        sendDataI2C[i] = (((i + 78) >> 1) + i + (i<<1) + 212) & 0xff;
        sendDataI2C[i] = (sendDataI2C[i] + 7) & 0xff;
        rcvDataI2C[i] = 0;
    }
#endif
}


LOCALF void TestI2cDebug(void)
{
#if 0
    int16 i;
    Uint32 j;

//    I2CWrongTimer = 0;
    for (i = 0; i < I2CDebugNum; i++)
    {
        if (rcvDataI2C[i] != sendDataI2C[i])
        {
            I2CWrongTimer++;

            for (j = 0; j < 400; j++)
            {
                DELAY_US(25000);
            }
            asm(" nop");
        }
    }
#endif
}

#if 0
LOCALF void pass()
{
//     asm("   ESTOP0");
//     for(;;);
    passTimer++;
}

LOCALF void fail()
{
//     asm("   ESTOP0");
//     for(;;);
    failTimer++;
}
#endif
//////////////////////////////////////////////////////////////



//=====================================================================
//
// 把上电时的第一个变量放在功能码里面显示出来
// 目前可以使用FF[0], FF[6], FF[7]显示
//
//=====================================================================
void PowerOnDebug(Uint16 value)
{
#if 0
    static Uint16 a,b;

    if (!a)
    {
        a = 1;
        b = value;
    }

    funcCode.group.ff[0]= b;    // 目前可以使用FF[0], FF[6], FF[7]调试
#endif
}


#if 0
#define DA_SYNC (GpioDataRegs.GPADAT.bit.GPIO23)
#if 0
#define DA_SYNC_DEAL()  \
{                       \
    DA_SYNC = 1;        \
    asm(" nop ");       \
    asm(" nop ");       \
    asm(" nop ");       \
    asm(" nop ");       \
    DA_SYNC = 0;        \
    asm(" nop ");       \
}
#elif 0
#define DA_SYNC_DEAL()  \
{                       \
    DA_SYNC = 1;        \
    DA_SYNC = 0;        \
}
#elif 1
#define DA_SYNC_DEAL()  \
{                       \
    DA_SYNC = 0;        \
    asm(" nop ");       \
    asm(" nop ");       \
    DA_SYNC = 1;        \
    asm(" nop ");       \
    asm(" nop ");       \
    asm(" nop ");       \
    asm(" nop ");       \
    DA_SYNC = 0;        \
    asm(" nop ");       \
    asm(" nop ");       \
}
#endif
//=====================================================================
// 
// DA初始化
// 使用SPIB的SPISIMOB，SPICLKB，以及GPIO34
// 
//=====================================================================
void InitDa(void)
{
// init IO
    EALLOW;
    
    GpioCtrlRegs.GPAPUD.bit.GPIO24 = 0;     // Enable pull-up on GPIO24 (SPISIMOB)
    GpioCtrlRegs.GPAPUD.bit.GPIO26 = 0;     // Enable pull-up on GPIO26 (SPICLKB)
    GpioCtrlRegs.GPAPUD.bit.GPIO23 = 0;                                                               

    GpioCtrlRegs.GPAQSEL2.bit.GPIO24 = 3;   // Asynch input GPIO24 (SPISIMOB)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO26 = 3;   // Asynch input GPIO26 (SPICLKB)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO23 = 3;   //

    GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 3;    // Configure GPIO24 as SPISIMOB
    GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 3;    // Configure GPIO26 as SPICLKB
    GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 0;    // IO

    GpioCtrlRegs.GPADIR.bit.GPIO23 = 1;     // output

    EDIS;

    DA_SYNC = 0;

#if 0
// Initialize SPI FIFO registers
    SpibRegs.SPIFFTX.all=0xE040;
    SpibRegs.SPIFFRX.all=0x405f;    // Receive FIFO reset
    SpibRegs.SPIFFRX.all=0x205f;    // Re-enable transmit FIFO operation
    SpibRegs.SPIFFCT.all=0x0000;
#endif

// Initialize  SPI
    SpibRegs.SPICCR.all =0x000F;    // Reset on, rising edge, 16-bit char bits
    SpibRegs.SPICTL.all =0x0006;                 

    SpibRegs.SPIBRR = 0x0000;       // 60/4 * 10^6 / (_+1)

    SpibRegs.SPICCR.all |= 0x0080;  // Relinquish SPI from Reset
    SpibRegs.SPIPRI.bit.FREE = 1;   // Set so breakpoints don't disturb xmission
}

#define DA_MAX  4094
//=====================================================================
// 
// DA数据更新，仅更新缓存，不更新输出
// 
// 范围 (0 -- 4095) ==> (0V -- 5V)
// port  00-->A PORT,01-->B PORT, 10-->C PORT,11-->D PORT
// 
//=====================================================================
void UpdateDaBuffer(Uint16 port)
{
    int16 data = daValue[port];
    if (data > DA_MAX)
        data = DA_MAX;
    data = ((port << 14) | 0x3000) + (data);

    DA_SYNC_DEAL();

    SpibRegs.SPITXBUF = data;
}


//=====================================================================
// 
// DA数据更新，几个DA通道同时输出
// 
// 范围 (0 -- 4095) ==> (0V -- 5V)
// port  00-->A PORT,01-->B PORT, 10-->C PORT,11-->D PORT
// 
//=====================================================================
void DaOut(Uint16 port)
{
    int16 data = daValue[port];
//    DA_SYNC_DEAL();
    if (data > DA_MAX)
        data = DA_MAX;
    data = ((port << 14) | 0x2000) + (data);

//    (void)SpibRegs.SPIRXBUF;
    DA_SYNC_DEAL();
    SpibRegs.SPITXBUF = data;
}
#endif


LOCALF void ChgAttr(void)
{
#if 0
    i = sizeof(attributeDebug) - 1; 
    for (; i >= 0; i--)
    {
        attributeDebug[i].bit.point = funcCodeAttribute[i].attribute.bit.point;
        attributeDebug[i].bit.unit = funcCodeAttribute[i].attribute.bit.unit;

#if 0
        if (funcCodeAttribute[i].attribute.bit.displayBits == 0)
            attributeDebug[i].bit.displayBits = 0;
        else
            attributeDebug[i].bit.displayBits = funcCodeAttribute[i].attribute.bit.displayBits - 1;
#elif 0
        attributeDebug[i].bit.displayBits = funcCodeAttribute[i].attribute.bit.displayBits;
#elif 1
        attributeDebug[i].bit.displayBits = funcCodeAttribute[i].attribute.bit.displayBits+1;
#endif

        attributeDebug[i].bit.upperLimit = funcCodeAttribute[i].attribute.bit.upperLimit;
        attributeDebug[i].bit.lowerLimit = funcCodeAttribute[i].attribute.bit.lowerLimit;

#if 1
        attributeDebug[i].bit.writable = funcCodeAttribute[i].attribute.bit.writable;
#elif 1
        if (funcCodeAttribute[i].attribute.bit.writable == ATTRIBUTE_READ_AND_WRITE)
            attributeDebug[i].bit.writable = ATTRIBUTE_READ_AND_WRITE1;
        else if (funcCodeAttribute[i].attribute.bit.writable == ATTRIBUTE_READ_ONLY_WHEN_RUN)
            attributeDebug[i].bit.writable = ATTRIBUTE_READ_ONLY_WHEN_RUN1;
        else
            attributeDebug[i].bit.writable = ATTRIBUTE_READ_ONLY_ANYTIME1;
#endif

        attributeDebug[i].bit.signal = funcCodeAttribute[i].attribute.bit.signal;
        //attributeDebug[i].bit.rsvd = funcCodeAttribute[i].attribute.bit.rsvd;
        attributeDebug[i].bit.rsvd = 0;
    }
#endif
}









