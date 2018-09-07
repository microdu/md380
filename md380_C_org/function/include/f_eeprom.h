#ifndef __F_EEPROM_H__
#define __F_EEPROM_H__


#include "f_funcCode.h"

// 保存1个功能码, 准备写入EEPROM
#define SaveOneFuncCode(index)  (funcCodeOneWriteIndex[funcCodeOneWriteNum++] = (index))

#define I2C_SLAVE_ADDR          0x50

#if DSP_2803X     // 2803x还是2808平台
#define EEPROM_PAGE_BYTE        4 // EEPROM(24LC08)的1页的byte
#elif 1
#define EEPROM_PAGE_BYTE        16 // EEPROM(24LC08)的1页的byte
#endif

#define FUNC_CODE_BYTE          2  // 一个功能码的byte
#define EEPROM_PAGE_NUM_FUNC_CODE (EEPROM_PAGE_BYTE/FUNC_CODE_BYTE) // EEPROM(24LC08)的1页可以写入的功能码个数

// 参数初始化
#define FUNCCODE_paraInitMode_NO_OPERATION              0   // 无操作
#define FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA      1   // 恢复(部分)出厂设定值
#define FUNCCODE_paraInitMode_CLEAR_RECORD              2   // 清除记录信息
#define FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL  3   // 恢复(部分)出厂设定值
#define FUNCCODE_paraInitMode_SAVE_USER_PARA            4   //+e 保存当前全部用户功能码
#define FUNCCODE_paraInitMode_RESTORE_USER_PARA         501   //+e 恢复保存的用户功能码
// 以下不要与FUNCCODE_paraInitMode_xx不能重复
#define FUNCCODE_RW_MODE_WRITE_ALL                  10  // 全部功能码写成出厂值
#define FUNCCODE_RW_MODE_WRITE_SERIES               11  // 连续写，使用这个时，必须EEPROM和FC地址要连续
#define FUNCCODE_RW_MODE_WRITE_ONE                  12  // 写一个功能码
#define FUNCCODE_RW_MODE_READ_ALL                   13  // 读取全部功能码
#define FUNCCODE_RW_MODE_READ_SERIES                14  // 连续读
#define FUNCCODE_RW_MODE_READ_ONE                   15  // 读一个功能码，请直接使用ReadFuncCode()函数
#define FUNCCODE_RW_MODE_NO_OPERATION   FUNCCODE_paraInitMode_NO_OPERATION   // 没有EEPROM操作


// I2C  Message Commands for I2CMSG struct
#if DSP_2803X     // 2803x还是2808平台
#define I2C_MSG_BUFFER_SIZE 4
#elif 1
#define I2C_MSG_BUFFER_SIZE 16
#endif
enum I2C_MSG_STATUS
{
    I2C_MSG_STATUS_IDLE,                   // 空闲
    I2C_MSG_STATUS_WRITE_BUSY,             // EEPROM内部正在擦写数据
    I2C_MSG_STATUS_SEND_NOSTOP_BUSY,       // DSP正在发送要读取的数据控制字
    I2C_MSG_STATUS_RESTART,                // DSP发送要读取控制字完毕
    I2C_MSG_STATUS_READ_BUSY,              // DSP准备接收数据
    I2C_MSG_STATUS_RW_OK                   // 读写完毕
};
// I2C Message Structure
struct I2C_MSG 
{
  enum I2C_MSG_STATUS status;           // Word status
  Uint16 bytes;                         // Num of valid bytes in (or to be put in buffer)
  Uint16 highAddr;                      // EEPROM address of data associated with msg (high byte)
  Uint16 lowAddr;
  Uint16 buffer[I2C_MSG_BUFFER_SIZE];	// Array holding msg data - max that
  									    // MAX_BUFFER_SIZE can be is 16 due to
  									    // the FIFO's
};

typedef struct
{
    Uint16 index;   // 要读写功能码的起始index, EEPROM的index
    Uint16 number;  // 要读写功能码的个数
    
    Uint16 data[EEPROM_PAGE_NUM_FUNC_CODE];
} FUNCCODE_RW;
extern FUNCCODE_RW funcCodeRead;

struct EEPROM_OPERATE_TIME
{
    // 调用 ReadFuncCode() 函数，则readFlag标志为1，读完成，readFlag标志为0
    Uint16 readFlag;  // 读标志
    Uint16 readTicker;

    // 调用 WriteFuncCode() 函数
    Uint16 writeFlag; // 写标志
    Uint16 writeTicker;

    // funcCodeRwMode不为0
    Uint16 rwFlag;
    Uint16 rwTicker;
};
extern struct EEPROM_OPERATE_TIME eepromOperateTime;

#if F_DEBUG_RAM
#define FUNCCODE_DEBUG              5555
#define FUNCCODE_RW_STATUS_DEBUG    5555
#endif

#define FUNCCODE_READ_RET_READING           0   // 正在读取
#define FUNCCODE_READ_RET_OK                1   // 读取完毕，读取的数据放在data开始的地址里面
//#define FUNCCODE_READ_RET_WAIT_FOR_REPEAT   2

#define FUNCCODE_WRITE_RET_WRITING          0   // 正在写入
#define FUNCCODE_WRITE_RET_OK               1   // 写入完毕
//#define FUNCCODE_WRITE_RET_READING          2
//#define FUNCCODE_WRITE_RET_WAIT_FOR_REPEAT  3

#define FUNCCODE_RW_RET_PARA_ERROR          4   // 参数错误，目前仅是readNumber错误
#define FUNCCODE_RW_RET_EEPROM_ERROR        5   // 读取EEPROM出错，或者读取之前就已经有EEPROM错误

extern Uint16 funcCodeOneWriteIndex[];
extern Uint16 funcCodeOneWriteNum;
extern Uint16 rwFuncCodeRepeatDelayTicker;
extern Uint16 funcCodeRwMode;
extern Uint16 funcCodeRwModeTmp;

extern Uint16 startIndexWriteSeries;
extern Uint16 endIndexWriteSeries;






//------------------------------------------------------
#define ERROR_EEPROM_NONE               0   // 
#define ERROR_EEPROM_WRITE              1   // 写功能码错误
#define ERROR_EEPROM_READ               2   // 读功能码错误
#define ERROR_EEPROM_RW_OVER_TIME       3   // 读写功能码错误
#define ERROR_EEPROM_WRITE_NUM_OVER     4   // 要保存的功能码个数越界, 数组funcCodeOneWriteIndex个数有限
extern Uint16 errorEeprom;
//------------------------------------------------------




Uint16 ReadFuncCode(FUNCCODE_RW *funcCodeRw);
Uint16 WriteFuncCode(FUNCCODE_RW *funcCodeRw);
void EepromDeal(void);
void InitSetI2ca(void);
void EepromOperateTimeDeal(void);

Uint16 GetEepromIndexFromFcIndex(Uint16 index);
Uint16 GetFcIndexFromEepromIndex(Uint16 a);

#endif // __F_EEPROM_H__


