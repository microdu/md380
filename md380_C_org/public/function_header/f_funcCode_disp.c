//======================================================================
//
// U组，停机状态显示等
//
// Time-stamp: <2012-08-1 9:01:32  Shisheng.Zhi, 0354>
//
//======================================================================

#define DISP_FRQ_RUN            0   //  0, 运行频率
#define DISP_FRQ_AIM            1   //  1, 设定频率
#define DISP_OUT_CURRENT        4   //  4, 输出电流
#define DISP_OUT_POWER          5   //  5, 输出功率
#define DISP_FRQ_RUN_FDB        19  //  19,反馈速度
#define DISP_FRQ_COMM           28  //  28,通讯设定值
#define DISP_P2P_COMM_SEND      63  //  63,点对点通讯数据发送
#define DISP_P2P_COMM_REV       64  //  64,点对点通讯数据接收
#define DISP_FRQ_FDB            29  //  29,反馈速度 
#define DISP_FRQ_X              30  //  30, 主频率X显示
#define DISP_FRQ_Y              31  //  31, 辅助频率Y显示
#define DISP_LOAD_SPEED         14  //  运行时负载速度显示的bit位置

#define DISP_DI_STATUS_SPECIAL1 41  // DI输入状态直观显示，DI1-DI19,VDI1
#define DISP_DO_STATUS_SPECIAL1 42  // DO输入状态直观显示，R1-R10,DO1,R11,R12,VDO1-VDO5
#define DISP_DI_FUNC_SPECIAL1   43  // DI功能状态直观显示1，功能01-功能40
#define DISP_DI_FUNC_SPECIAL2   44  // DI功能状态直观显示2，功能41-功能80

extern Uint16 ticker05msFuncEachDisp[4];
extern Uint16 ticker05msFuncInternalDisp[4];
extern Uint16 vfSeparateVol;
extern Uint16 vfSeprateVolAim;
extern Uint16 temperature;     // 检测到得电机温度值
extern Uint16 ZCount;          // Z信号计数器
extern Uint16 antiVibrateCoeff;       // VF振荡系数
#define UF_VIEW_ATTRIBUTE  0x3140

//=====================================================================
// 如下参数，作为显示使用，目前使用的地方有:
// 1. 运行时显示
// 2. 停机时显示
// 3. 功能码显示
// 4. 监控
// 前面32个，同运行时显示(MD320, F7-04功能码)顺序

// 更新U0组显示数据
void UpdateU0Data(void)
{
    funcCode.group.u0[0]  = frqDisp;                     //  0; 运行频率
    funcCode.group.u0[1]  = frqAimDisp;                  //  1; 设定频率
    funcCode.group.u0[2]  = generatrixVoltage;           //  2; 母线电压
    funcCode.group.u0[3]  = outVoltageDisp;              //  3; 输出电压
    funcCode.group.u0[4]  = outCurrentDisp;              //  4; 输出电流
    funcCode.group.u0[5]  = outPower;                    //  5; 输出功率
    funcCode.group.u0[6]  = itDisp;                      //  6; 输出转矩
    funcCode.group.u0[7]  = diStatus.a.all&0x7FFF;       //  7; DI输入状态
    funcCode.group.u0[8]  = doStatus.a.all;              //  8; DO输出状态
    funcCode.group.u0[9]  = aiDeal[0].voltage;           //  9; AI1电压
    funcCode.group.u0[10] = aiDeal[1].voltage;           // 10; AI2电压
    funcCode.group.u0[11] = aiDeal[2].voltage;           // 11; AI3电压
    funcCode.group.u0[12] = funcCode.code.counterTicker; // 12; 计数值
    funcCode.group.u0[13] = funcCode.code.lengthCurrent; // 13; 长度值
    funcCode.group.u0[14] = loadSpeedDisp;               // 14; 负载速度显示(停机时显示设定频率*系数  运行时显示运行频率*系数)
    funcCode.group.u0[15] = pidFuncRefDisp;              // 15; PID设定
    funcCode.group.u0[16] = pidFuncFdbDisp;              // 16; PID反馈
    funcCode.group.u0[17] = plcStep;                     // 17; PLC阶段
    funcCode.group.u0[18] = pulseInFrqDisp;              // 18; PULSE输入脉冲频率，单位0.01KHz
    funcCode.group.u0[19] = frqRunDisp;                  // 19，反馈速度，单位0.1Hz    // 电机反馈频率
    funcCode.group.u0[20] = setRunLostTime;              // 20; 剩余运行时间  
    funcCode.group.u0[21] = aiDeal[0].voltageOrigin;     // 21; AI1校正前电压
    funcCode.group.u0[22] = aiDeal[1].voltageOrigin;     // 22; AI2校正前电压
    funcCode.group.u0[23] = aiDeal[2].voltageOrigin;     // 23; AI3校正前电压                         
    funcCode.group.u0[24] = lineSpeed;                   // 24; 线速度
    funcCode.group.u0[25] = curTime.powerOnTime;         // 25; 当前上电时间
    funcCode.group.u0[26] = curTime.runTime;             // 26; 当前运行时间
    funcCode.group.u0[27] = pulseInFrq;                  // 27; PULSE输入脉冲频率，单位1Hz
    funcCode.group.u0[28] = funcCode.code.frqComm;       // 28; 通讯设定值
    funcCode.group.u0[29] = frqFdb;                      // 29; 实际反馈速度
    funcCode.group.u0[30] = frqXDisp;                    // 30; 主频率X显示
    funcCode.group.u0[31] = frqYDisp;                    // 31; 辅频率Y显示
    funcCode.group.u0[32] = memoryValue;                 // 32; 查看任意内存地址值
    funcCode.group.u0[33] = pmsmRotorPos;                // 33; 同步机转子位置(性能实时更新)
    funcCode.group.u0[34] = temperature;                 // 34; 电机温度值
    funcCode.group.u0[35] = torqueAim;                   // 35; 目标转矩
    funcCode.group.u0[36] = enCoderPosition;             // 36; 旋变位置
    funcCode.group.u0[37] = gPhiRtDisp;                  // 37; 功率因素角度
    funcCode.group.u0[38] = ABZPos;                      // 38; ABZ位置
    funcCode.group.u0[39] = vfSeprateVolAim;             // 39; VF分离目标电压
    funcCode.group.u0[40] = vfSeparateVol;               // 40; VF分离输出电压
    //funcCode.group.u0[41] = rsvdData;                    // 41; DI输入直观显示
    //funcCode.group.u0[42] = rsvdData;                    // 42; DO输入直观显示
    //funcCode.group.u0[43] = rsvdData;                    // 43; DI功能状态直观显示
    //funcCode.group.u0[44] = rsvdData;                    // 44; DO功能状态直观显示
    funcCode.group.u0[45] = errorInfo;                   // 45; 故障信息
    //funcCode.group.u0[46] = rsvdData;                    // 46; 保留
    //funcCode.group.u0[47] = rsvdData;                    // 47; 保留
    //funcCode.group.u0[48] = rsvdData;                    // 48; 保留
    //funcCode.group.u0[49] = rsvdData;                    // 49; 保留
    funcCode.group.u0[50] = ticker05msFuncInternalDisp[0]; // 50; 保留
    funcCode.group.u0[51] = ticker05msFuncInternalDisp[1]; // 51; 保留
    funcCode.group.u0[52] = ticker05msFuncInternalDisp[2]; // 52; 保留
    funcCode.group.u0[53] = ticker05msFuncInternalDisp[3]; // 53; 保留
    funcCode.group.u0[54] = ticker05msFuncEachDisp[0];     // 54; 保留
    funcCode.group.u0[55] = ticker05msFuncEachDisp[1];     // 55; 保留
    funcCode.group.u0[56] = ticker05msFuncEachDisp[2];     // 56; 保留
    funcCode.group.u0[57] = ticker05msFuncEachDisp[3];     // 57; 保留
    funcCode.group.u0[58] = ZCount;                        // 58; Z信号计数器
    funcCode.group.u0[59] = frqAimPLCDisp;                 // 59; 设定频率 -100.00%~100.00%
    funcCode.group.u0[60] = frqPLCDisp;                    // 60; 运行频率 -100.00%~100.00%
    funcCode.group.u0[61] = invtStatus.all;                // 61; 变频器运行状态
    //funcCode.group.u0[62] = antiVibrateCoeff;              // 62; VF振荡系数
    funcCode.group.u0[63] = p2pData.P2PSendData;           // 63; 点对点通讯发送值
    funcCode.group.u0[64] = p2pData.P2PRevData;            // 64; 点对点通讯接收值
 #if 0      
    funcCode.group.u0[62] = 0;
    funcCode.group.u0[63] = 0;
    funcCode.group.u0[64] = 0;
    funcCode.group.u0[65] = 0;
    funcCode.group.u0[66] = 0;
    funcCode.group.u0[67] = 0;
    funcCode.group.u0[68] = 0;
    funcCode.group.u0[69] = 0;
    funcCode.group.u0[70] = 0;
    funcCode.group.u0[71] = 0;
    funcCode.group.u0[72] = 0;
    funcCode.group.u0[73] = 0;
    funcCode.group.u0[74] = 0;
    funcCode.group.u0[75] = 0;
    funcCode.group.u0[76] = 0;
    funcCode.group.u0[77] = 0;
    funcCode.group.u0[78] = 0;
    funcCode.group.u0[79] = 0;
    funcCode.group.u0[80] = 0;
    funcCode.group.u0[81] = 0;
    funcCode.group.u0[82] = 0;
    funcCode.group.u0[83] = 0;
    funcCode.group.u0[84] = 0;
    funcCode.group.u0[85] = 0;
    funcCode.group.u0[86] = 0;
    funcCode.group.u0[87] = 0;
    funcCode.group.u0[88] = 0;
    funcCode.group.u0[89] = 0;
    funcCode.group.u0[90] = 0;
    funcCode.group.u0[91] = 0;
    funcCode.group.u0[92] = 0;
    funcCode.group.u0[93] = 0;
    funcCode.group.u0[94] = 0;
    funcCode.group.u0[95] = 0;
    funcCode.group.u0[96] = 0;
    funcCode.group.u0[97] = 0;
    funcCode.group.u0[98] = 0;
    funcCode.group.u0[99] = 0;
#endif
}


// 可以与pDispValueU0形成结构体，但占用空间变大
union FUNC_ATTRIBUTE const dispAttributeU0[U0NUM] =
{
    0x094A & ~(1U<<11) | (1U<<12),     // 0, 运行频率
    0x094A & ~(1U<<11) | (1U<<12),     // 1, 设定频率
    0x1161 & ~(1U<<11) | (1U<<12),     // 2, 母线电压
    0x08E0 & ~(1U<<11) | (1U<<12),     // 3, 输出电压
    0x0952 & ~(1U<<11) | (1U<<12),     // 4, 输出电流

    0x2941 & ~(1U<<11) | (1U<<12),     // 5, 输出功率
    0x2131 & ~(1U<<11) | (1U<<12),     // 6, 输出转矩
    0x9D00 & ~(1U<<11) | (1U<<12),     // 7, DI输入状态
    0x9D00 & ~(1U<<11) | (1U<<12),     // 8, DO输出状态
    0x2122 & ~(1U<<11) | (1U<<12),     // 9, AI1电压

    0x2122 & ~(1U<<11) | (1U<<12),     // 10, AI2电压
    0x2122 & ~(1U<<11) | (1U<<12),     // 11, AI3电压
    0x0140 & ~(1U<<11) | (1U<<12),     // 12, 计数值
    0x0140 & ~(1U<<11) | (1U<<12),     // 13, 长度值
    0x0140 & ~(1U<<11) | (1U<<12),     // 14, 负载速度显示

    0x0170 & ~(1U<<11) | (1U<<12),     // 15, PID设定
    0x0170 & ~(1U<<11) | (1U<<12),     // 16, PID反馈
    0x0140 & ~(1U<<11) | (1U<<12),     // 17, PLC阶段
    0x1142 & ~(1U<<11) | (1U<<12),     // 18, PULSE输入脉冲频率，单位0.01kHz
    0x294A & ~(1U<<11) | (1U<<12),     // 19, 反馈频率，0.1Hz
    0x0141 & ~(1U<<11) | (1U<<12),     // 20, 剩余运行时间
    
    0x2163 & ~(1U<<11) | (1U<<12),      // 21, AI1校正前电压
    0x2163 & ~(1U<<11) | (1U<<12),      // AI2校正前电压
    0x2163 & ~(1U<<11) | (1U<<12),      // AI3校正前电压
    
    0x1D00 & ~(1U<<11) | (1U<<12),

    0x0140 & ~(1U<<11) | (1U<<12),     // 25,
    0x0141 & ~(1U<<11) | (1U<<12),     // 26,
    0x0148 & ~(1U<<11) | (1U<<12),     // 27,
    0x2972 & ~(1U<<11) | (1U<<12),     // 28,
    0x294A & ~(1U<<11) | (1U<<12),     // 29,

    0x094A & ~(1U<<11) | (1U<<12),     // 30,
    0x094A & ~(1U<<11) | (1U<<12),     // 31,
    0x0140 & ~(1U<<11) | (1U<<12),     // 32,
    0x0101 & ~(1U<<11) | (1U<<12),     // 33,
    0x0140 & ~(1U<<11) | (1U<<12),     // 34,

    0x2131 & ~(1U<<11) | (1U<<12),     // 35,
    0x0140 & ~(1U<<11) | (1U<<12),     // 36,
    0x2101 & ~(1U<<11) | (1U<<12),     // 37,
    0x0140 & ~(1U<<11) | (1U<<12),     // 38,
    0x0140 & ~(1U<<11) | (1U<<12),     // 39,

    0x0140 & ~(1U<<11) | (1U<<12),     // 40,
    0x0140 & ~(1U<<11) | (1U<<12),     // 41,
    0x0140 & ~(1U<<11) | (1U<<12),     // 42,
    0x0140 & ~(1U<<11) | (1U<<12),     // 33,
    0x0140 & ~(1U<<11) | (1U<<12),     // 44,

    0x0140 & ~(1U<<11) | (1U<<12),     // 45,
    0x0140 & ~(1U<<11) | (1U<<12),     // 46,
    0x0140 & ~(1U<<11) | (1U<<12),     // 47,
    0x0140 & ~(1U<<11) | (1U<<12),     // 48,
    0x0140 & ~(1U<<11) | (1U<<12),     // 49,

    0x1001,                             // 50
    0x1001,                             // 51
    0x1001,                             // 52
    0x1001,                             // 53
    0x1001,                             // 54
    0x1001,                             // 55
    0x1001,                             // 56
    0x1001,                             // 57
    0x0140 & ~(1U<<11) | (1U<<12),      // 58

    0x3172 & ~(1U<<11) | (1U<<12),      // 59
    0x3172 & ~(1U<<11) | (1U<<12),      // 60
    0x0140 & ~(1U<<11) | (1U<<12),      // 61
    0x0140 & ~(1U<<11) | (1U<<12),      // 62
    0x2972 & ~(1U<<11) | (1U<<12),      // 63
    0x2972 & ~(1U<<11) | (1U<<12),      // 64
    0x0140 & ~(1U<<11) | (1U<<12),      // 65
	0x0140 & ~(1U<<11) | (1U<<12),      // 65
};
//=====================================================================
// 停机监视参数
// 1.对应于运行参数中的序号
//=====================================================================
const Uint16 stopDispIndex[STOP_DISPLAY_NUM] =
{
    1,      // 0  设定频率
    2,      // 1  母线电压
    7,      // 2  DI输入状态
    8,      // 3  DO输出状态
    9,      // 4  AI1电压

    10,     // 5  AI2电压
    11,     // 6  AI3电压
    12,     // 7  计数值
    13,     // 8  长度值
    17,     // 9  plcStep

    14,     // 10 负载速度显示
    15,     // 11 PID设定
    18,     // 12 PULSE输入脉冲频率
};

Uint16 const commDispIndex[COMM_PARA_NUM] =  // 通讯读取停机或运行显示参数
{
    28,         // 通讯设定值
    0,          // 运行频率
    2,          // 母线电压
    3,          // 输出电压
    4,          // 输出电流
    5,          // 输出功率
    6,          // 输出转矩
    19,         // 运行速度
    7,          // DI输入状态
    8,          // DO输出状态
    9,          // AI1电压
    10,         // AI2电压
    11,         // AI3电压
    12,         // 计数值输入
    13,         // 长度值输入
    14,         // 负载速度
    15,         // PID设定
    16,         // PID反馈
    17,         // PLC阶段
    
    18,         // PULSE输入脉冲频率，单位0.01KHz
    19,         // 反馈速度，单位0.1Hz    // 电机反馈频率
    20,         //  剩余运行时间  
    21,         //  AI1校正前电压
    22,         //  AI2校正前电压
    23,         //  AI3校正前电压                         
    24,         //  线速度
    25,         //  当前上电时间
    26,         //  当前运行时间
    27,         //  PULSE输入脉冲频率，单位1Hz
    28,         //  通讯设定值
    29,         //  实际反馈速度
    30,         //  主频率X显示
    31,         //  辅频率Y显示
};






