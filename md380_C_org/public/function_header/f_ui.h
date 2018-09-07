#ifndef __F_DISPLAY_KEY_H__
#define __F_DISPLAY_KEY_H__

#include "f_funcCode.h"

#define DISPLAY_8LED_NUM    5   // 数码管个数
#define DISPLAY_LED_NUM     7   // LED个数


// ====================================================================
// 8段式数码管的编码
#define DISPLAY_8LED_CODE_NUM       38  // 8段式数码管显示的字符个数
// 显示字符段码在数组中对应的下标
#define DISPLAY_0       0   // 0
#define DISPLAY_1       1   // 1
#define DISPLAY_2       2   // 2
#define DISPLAY_3       3   // 3
#define DISPLAY_4       4   // 4

#define DISPLAY_5       5   // 5
#define DISPLAY_6       6   // 6
#define DISPLAY_7       7   // 7
#define DISPLAY_8       8   // 8
#define DISPLAY_9       9   // 9

#define DISPLAY_A       10  // A
#define DISPLAY_B       11  // b
#define DISPLAY_C       12  // C
#define DISPLAY_D       13  // d
#define DISPLAY_E       14  // E

#define DISPLAY_F       15  // F
#define DISPLAY_P       16  // P
#define DISPLAY_H       17  // H
#define DISPLAY_J       18  // J
#define DISPLAY_L       19  // L

#define DISPLAY_n       20  // n
#define DISPLAY_r       21  // r
#define DISPLAY_U       22  // U
#define DISPLAY_y       23  // y
#define DISPLAY_G       24  // G

#define DISPLAY_h       25  // h
#define DISPLAY_I       26  // I
#define DISPLAY_N       27  // N
#define DISPLAY_o       28  // o
#define DISPLAY_q       29  // q

#define DISPLAY_S       5   // S
#define DISPLAY_d       13  // D
#define DISPLAY_b       11  // b

#define DISPLAY_T       30  // T
#define DISPLAY_t       31  // t
#define DISPLAY_u       32  // u/v，小写u/V
#define DISPLAY_NULL    33  // 全灭
#define DISPLAY_LINE    34  // -

#define DISPLAY_DOT     35  // 显示小数点，其他编码&即可
#define DISPLAY_ALL     36  // 全亮	 

#define DISPLAY_c      (DISPLAY_8LED_CODE_NUM-1)  // 小写c
// ====================================================================

// ====================================================================
// LED灯的编码
#define DISPLAY_LED_CODE_NUM    11   // LED灯的显示方式个数
// LED在数组中对应的下标
#define LED_RUN      0   // Run
#define LED_LOCAL    1   // Local/Remote
#define LED_DIR      2   // FWD/REV
#define LED_TUNE     3   // TUNE/TC
#define LED_V        4   // V
#define LED_A        5   // A
#define LED_HZ       6   // Hz
#define LED_RPM      7   // RPM
#define LED_PERCENT  8   // %
#define LED_ALL      9   // 全亮
#define LED_NULL     (DISPLAY_LED_CODE_NUM-1)  // 全灭
// ====================================================================


// ====================================================================
// keyboard
/*
#define KEY_NULL    0xFF
#define KEY_PRG     0xFE    // 1111, 1110,  bit0
#define KEY_STOP    0xFD    // 1111, 1101,  bit1
#define KEY_MFK     0xFB    // 1111, 1011,  bit2
#define KEY_RUN     0xF7    // 1111, 0111,  bit3
#define KEY_DOWN    0xEF    // 1110, 1111,  bit4
#define KEY_SHIFT   0xDF    // 1101, 1111,  bit5
#define KEY_ENTER   0xBF    // 1011, 1111,  bit6
#define KEY_UP      0x7F    // 0111, 1111,  bit7
*/
// 外接键盘和面板键盘的编码是一样的
#define KEY_NUM     9       // 按键个数
#define KEY_PRG     (0+1)   // 1111, 1110,  bit0
#define KEY_QUICK   (1+1)   // 1111, 1101,  bit1
#define KEY_RUN     (2+1)   // 1111, 1011,  bit2
#define KEY_MFK     (3+1)   // 1111, 0111,  bit3
#define KEY_STOP    (4+1)   // 1110, 1111,  bit4
#define KEY_DOWN    (5+1)   // 1101, 1111,  bit5
#define KEY_SHIFT   (6+1)   // 1011, 1111,  bit6
#define KEY_ENTER   (7+1)   // 0111, 1111,  bit7
#define KEY_UP      (8+1)   // 

#define KEY_FWD_JOG (0x93)  // MFK-JOG
#define KEY_REV_JOG (0x94)  // MFK-JOG
#define KEY_REV     (0x91)  // MFK-REV
#define KEY_SWITCH  (0x92)  // MFK-switch
#define KEY_NO_KEY  0       // 没有按键
// ====================================================================


// 2808 DSP大板，使用GPIO19作为键盘的选择口
// 主轴伺服
// MD380
#if DSP_2803X        // 2803x还是2808平台
#define RCK (GpioDataRegs.GPADAT.bit.GPIO22) // 与74HC594/165的配合
#define KEYSEL (GpioDataRegs.AIODAT.bit.AIO4)
#else
#define RCK (GpioDataRegs.GPADAT.bit.GPIO19) // 与74HC594/165的配合
#endif


extern Uint16 keyBordTestFlag;     // 键盘测试标志  0-不启动按键测试  1-启动按键测试
extern Uint16 keyBordValue;        // 记录已按键信息

extern const Uint16 DISPLAY_CODE[DISPLAY_8LED_CODE_NUM];
extern const Uint16 LED_CODE[DISPLAY_LED_CODE_NUM];
extern Uint16 scanStep;
extern Uint16 displayBuffer[];

//=======================================================================
#define SPI_BYTE    3
//=======================================================================

extern Uint16 upDownDelta;

extern Uint16 factoryPwd;   // 厂家随机密码明文

extern Uint16 keyFunc;
extern Uint16 keyPreCtrl;

void InitSetSpiForFunc(void);
void UpdateDisplayBuffer(void);
void WinkDeal(void);
void DisplayScanPrepare(void);
void DisplayScan(void);
void KeyScan(void);
void KeyProcess(void);
extern Uint16 keySelStatus;

#if DSP_2803X         // 2803x还是2808平台
#define SPI_LENGTH  1       // SPI发送和接收的长度
#else
#define SPI_LENGTH  2       // SPI发送和接收的长度
#endif

struct SPI_OUT_A_BITS
{
#if DSP_2803X        // 2803x还是2808平台
    Uint16 dispCode:8;          // 7:0  显示位码
    Uint16 dispSect:8;          // 15:8 显示段码
#else
    Uint16 doData:8;        // 7:0  do
    Uint16 dispCode:8;      // 15:8 显示位码
#endif
};
union SPI_OUT_A
{
    Uint16              all;
    struct SPI_OUT_A_BITS bit;
};

struct SPI_OUT_B_BITS
{
    Uint16 dispSect:8;      // 7:0  显示段码
    Uint16 rsvd:8;          // 15:8 保留
};
union SPI_OUT_B
{
    Uint16              all;
    struct SPI_OUT_B_BITS bit;
};

struct SPI_OUT
{
    union SPI_OUT_A a;
    union SPI_OUT_B b;
};


union SPI_IN
{
    Uint16 all[SPI_LENGTH];

#if DSP_2803X        // 2803x还是2808平台
    Uint16 key;
#else
    struct SPI_IN_STRUCT
    {
        Uint16 di;
        Uint16 key;
    } c;
#endif
};


struct SPI_IN_OUT
{
    union SPI_IN in;        // SPI接收的数据
    struct SPI_OUT out;     // SPI发送的数据
};

extern struct SPI_IN_OUT spiData;



#endif  // __F_DISPLAY_KEY_H__












