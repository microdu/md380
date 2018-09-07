#ifndef __F_COMM_H__
#define __F_COMM_H__

#include "f_funcCode.h"
#include "f_main.h"
#include "f_menu.h"
#include "f_runSrc.h"
#include "f_eeprom.h"
#include "f_frqSrc.h"


#define SCI_CMD_READ                0x03
#define SCI_CMD_WRITE               0x06
#define SCI_CMD_WRITE_RAM           0x07

#define SCI_WRITE_NO_EEPROM         0
#define SCI_WRITE_WITH_EEPROM       1

#define MODBUS                        0
#define PROFIBUS                      1
#define CAN_OPEN                      2
#define CANLINK                       3

#define RTU_READ_DATA_NUM_MAX     12    // 最多读取数据个数

#define COMM_ERR_NONE               0   //
#define COMM_ERR_PWD                1   // 密码错误
#define COMM_ERR_CMD                2   // 读写命令错误
#define COMM_ERR_CRC                3   // CRC校验错误
#define COMM_ERR_ADDR               4   // 无效地址
#define COMM_ERR_PARA               5   // 无效参数
#define COMM_ERR_READ_ONLY          6   // 参数更改无效
#define COMM_ERR_SYSTEM_LOCKED      7   // 系统锁定
#define COMM_ERR_SAVE_FUNCCODE_BUSY 8   // 正在存储参数

// 通讯波特率对应的的寄存器值
typedef struct
{
    Uint16 baudRate;    // _*100bps
    
    Uint16 high;
    Uint16 low;
#if DSP_2803X
    Uint16 M;
#endif
} DSP_BAUD_REGISTER_DATA;


// 通讯标志字
struct SCI_FLAG_BITS
{                                   // bits  description
    Uint16 read:1;                  // 0    读取功能标志位
    Uint16 write:1;                 // 1    写功能码标志位
    Uint16 crcChkErr:1;             // 2    CRC校验故障 Err3
    Uint16 rcvDataEnd:1;            // 3

    Uint16 send:1;                  // 4    有数据发送标志位，区别广播模式和非广播模式，目前未使用
    Uint16 sendDataStart:1;         // 5
    Uint16 paraOver:1;              // 6    无效参数 Err5
    Uint16 addrOver:1;              // 7    无效地址 Err4

    Uint16 cmdErr:1;                // 8    读写命令错误 Err2
    Uint16 paraReadOnly:1;          // 9    参数更改无效。参数只读，不能修改错误 Err6
    Uint16 pwdErr:1;                // 10   密码输入错误 Err1
    Uint16 pwdPass:1;               // 11   密码校验通过

    Uint16 systemLocked:1;          // 12   系统锁定 Err7
    Uint16 saveFunccodeBusy:1;      // 13   正在储存功能码 Err8
    Uint16 rsvd:2;                  // 15:14 保留
};

union SCI_FLAG
{
    Uint16               all;
    struct SCI_FLAG_BITS bit;
};

// DO数字输出端子控制
struct DO_SCI_BITS
{
    Uint16 do1:1;       // bit0,  DO1输出控制
    Uint16 do2:1;       // bit1,  DO2
    Uint16 relay1:1;    // bit2,  ralay1
    Uint16 relay2:1;    // bit3,  relay2，280F没有
    Uint16 fmr:1;       // bit4,  fmr(do3)

    Uint16 vdo1:1;      // VDO1
    Uint16 vdo2:1;      // VDO2
    Uint16 vdo3:1;      // VDO3
    Uint16 vdo4:1;      // VDO4
    Uint16 vdo5:1;      // VDO5
    
    Uint16 rsvd:6;     // 15:5,  保留
};

union DO_SCI
{
    Uint16 all;
    struct DO_SCI_BITS bit;
};



#define SCI_RUN_CMD_NONE        0   // 0， 无命令
#define SCI_RUN_CMD_FWD_RUN     1   // 01，正转运行
#define SCI_RUN_CMD_REV_RUN     2   // 02，反转运行
#define SCI_RUN_CMD_FWD_JOG     3   // 03, 正转点动
#define SCI_RUN_CMD_REV_JOG     4   // 04，反转点动
#define SCI_RUN_CMD_FREE_STOP   5   // 05，自由停机
#define SCI_RUN_CMD_STOP        6   // 06，减速停机。目前为根据停机方式功能码停机
#define SCI_RUN_CMD_RESET_ERROR 7   // 07，故障复位

extern Uint16 commRunCmd;

enum COMM_STATUS
{
    SCI_RECEIVE_DATA,            // SCI接收数据
    SCI_RECEIVE_OK,              // SCI接收数据完毕
    SCI_SEND_DATA_PREPARE,       // 准备发送数据
    SCI_SEND_DATA,               // SCI发送数据
    SCI_SEND_OK                  // SCI发送数据完毕
};

extern enum COMM_STATUS commStatus;
extern Uint32 commTicker;
extern union DO_SCI doComm;
extern Uint16 aoComm[];
extern Uint16 sendFrame[];        // 从机响应帧
extern Uint16 rcvFrame[];         // 接收数据数组(主机命令帧)
extern Uint16 commReadData[RTU_READ_DATA_NUM_MAX];     // 读取的数据
extern union SCI_FLAG sciFlag;      // SCI使用的标志
extern Uint16 commProtocol;         // 通讯数据传送格式
void InitSetScia(void);
void UpdateSciFormat(void);
void SciDeal(void);
Uint16 CommRead(Uint16 addr, Uint16 data);
Uint16 CommWrite(Uint16, Uint16);
    
// 通讯波特率对应的的寄存器值
typedef struct
{
	Uint16 slaveAddr;
    Uint16 commAddr;    // _*100bps
    Uint16 commData;
    Uint16 rcvNum;
    Uint16 rcvFlag;
    Uint16 crcRcv;
    Uint16 commCmd;
    Uint16 crcSize;
    Uint16 rcvNumMax;
    Uint16 frameSpaceTime;
    Uint16 delay;
    Uint16 rcvCrcErrCounter;
    Uint16 rcvDataJuageFlag;
    Uint16 commCmdSaveEeprom;
} COMM_RCV_DATA;
extern COMM_RCV_DATA commRcvData;

typedef struct
{
    Uint16 sendNumMax;
    Uint16 sendNum;
    Uint16 commData;
    Uint16 crcRcv;
    Uint16 commCmd;
    Uint16 crcSize;
} COMM_SEND_DATA;
extern COMM_SEND_DATA commSendData;


void ProfibusRcvDataDeal(void);
void ModbusRcvDataDeal(void);
Uint16 ModbusStartDeal(Uint16 tmp);
Uint16 ProfibusStartDeal(Uint16 tmp);
void UpdateModbusCommFormat(Uint16 baudRate);
void UpdateProfibusCommFormat(Uint16 baudRate);
void ModbusSendDataDeal(Uint16 err);
void ProfibusSendDataDeal(Uint16 err);
Uint16 ModbusCommErrCheck(void);
Uint16 ProfibusCommErrCheck(void);

typedef struct CommProtocolDeal
{
    void (*RcvDataDeal)();         // 接收数据处理 
    Uint16 (*StartDeal)();         // 帧头判断
    void (*UpdateCommFormat)();    // 更新通讯配置
    void (*SendDataDeal)();        // 发送数据处理
    Uint16 (*CommErrCheck)();      // 通讯出错
}protocolDeal;
#endif // __F_COMM_H__



