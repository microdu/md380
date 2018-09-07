//======================================================================
//
// EEPROM处理。24LC32, I2C总线
//
// 
// Time-stamp: <2012-01-03 14:21:33  Shisheng.Zhi, >
//
//======================================================================
 

#include "f_eeprom.h"
#include "f_main.h"
#include "f_menu.h"
 

//--------------------------------------
#if F_DEBUG_RAM
#define DEBUG_F_TABLE_FC2EEPROM_CONST   0   // eeprom2Fc使用const，程序空间
#elif 1
#define DEBUG_F_TABLE_FC2EEPROM_CONST   1
#endif

#if DEBUG_F_TABLE_FC2EEPROM_CONST
// 对应关系表
// y = eeprom2Fc[i]
// i, 数组下标，----该功能码在EEPROM的位置
// y, 数组的值，----功能码的序号
//
// 自动生成
#include "f_table_eeprom2Fc.c"

#endif
//--------------------------------------


//======================================================================
// EEPROM芯片选择
#define EEPROM_24LC08   0
#define EEPROM_24LC16   1
#define EEPROM_24LC32   2

//#define EEPROM_TYPE     EEPROM_24LC08
//#define EEPROM_TYPE     EEPROM_24LC16
#define EEPROM_TYPE     EEPROM_24LC32
//======================================================================



// 参数初始化，包括保存全部当前功能码到EEPROM剩余的空间
// 目前EEPROM空间不够，暂时去掉。
#define PARA_INIT_MODE_SAVE_ALL_CODE    1       // 编译宏

#define DEBUG_F_EEPROM                  1       // 调试使用，把全部功能码写成__数据

#define FUNCCODE_READ_VERIFY_RIGHT_TIME_MAX     3       // 3次
#define FUNCCODE_READ_VERIFY_WRONG_TIME_MAX     4
#define FUNCCODE_READ_AFTER_WRITE_VERIFY_RIGHT_TIME_MAX     2   // 2次
#define FUNCCODE_READ_AFTER_WRITE_VERIFY_WRONG_TIME_MAX     3
#define FUNCCODE_READ_TIME_MAX                  5

enum FUNCCODE_WRITE_STATUS
{
    FUNCCODE_WRITE_STATUS_WRITE,                 
    FUNCCODE_WRITE_STATUS_WRITE_REMAINDER,       
    FUNCCODE_WRITE_STATUS_READ_TO_VERIFY_WRITE  
};
//#define FUNCCODE_WRITE_TIME_MAX                     5

// 必须为8的倍数，否则会超出24LC08的一个页面
// I2cMsg.highAddr
// 需要操作eeprom的地址highAddr(24lc08不需要MemoryLowAddr)，在给highAddr赋值时，
// 加上eeprom的起始地址即可。
#define FUNCCODE_EEPROM_START_INDEX  16   // eeprom的前面(_*2)个byte被误写的可能性比较大，所以不使用

// 必须为8的倍数，否则会超出24LC08的一个页面
#define USER_PARA_START_INDEX       (2048 + FUNCCODE_EEPROM_START_INDEX)

#if DEBUG_F_POSITION_CTRL
#pragma DATA_SECTION(funcCodeOneWriteIndex, "data_ram");
#endif
// 一个一个的往EEPROM写数据。
// 目前使用funcCodeOneWriteIndex写功能码的情况有：
// 1. 一般的，menu3OnEnter保存一个功能码(键盘、通讯)
// 2. 当修改最大频率(等等)，以最大频率为限值的功能码也要修改。。。，目前最多为14个。
// 3. 掉电记忆
// 4. 停机记忆
// 5. 清除记录参数
#define FUNCCODE_ONE_WRITE_NUMBER_MAX       200     //+= 100个应该足够，但是仍存在越界的可能。剩余的防止越界
Uint16 funcCodeOneWriteNum;
Uint16 funcCodeOneWriteIndex[FUNCCODE_ONE_WRITE_NUMBER_MAX];

// 成片的读写EEPROM数据。
// 目前使用paraInitMode写功能码的情况有：
// 1. 该EEPROM第一次使用，全部功能码和掉电保存变量出厂值写入到EEPROM中。
// 2. 该EEPROM曾经使用过，读取全部功能码和掉电保存变量到RAM中。
// 3. FP-01, 参数初始化，恢复出厂参数
Uint16 funcCodeRwMode;
Uint16 funcCodeRwModeTmp;
LOCALF struct I2C_MSG I2cMsg;

// 连续写的范围[startIndexWriteSeries, endIndexWriteSeries]
Uint16 startIndexWriteSeries;   // 连续写的起始index，连续写的功能码个数为endIndexWriteSeries-startIndexWriteSeries+1
Uint16 endIndexWriteSeries;     // 连续写的结束index
Uint16 endIndexRwFuncCode;


enum FUNC_CODE_RW_STATUS
{
    FUNCCODE_RW_STATUS_PREPARE_DATA_FOR_EAD,        // 准备读取功能码
    FUNCCODE_RW_STATUS_READ_CURRENT_DATA,           // 读取功能码在EEPROM的当前数据
    FUNCCODE_RW_STATUS_PREPARE_DATA_FOR_WRITE,      // 准备需要写入的数据
    FUNCCODE_RW_STATUS_WRITE_DATA                   // 写入数据
};
LOCALF enum FUNC_CODE_RW_STATUS funcCodeRwStatus;
#define FUNC_CODE_RW_STATUS_INIT    FUNCCODE_RW_STATUS_PREPARE_DATA_FOR_EAD

FUNCCODE_RW funcCodeRead;           // 连续(4/8个)的往EEPROM写入数据
LOCALF FUNCCODE_RW funcCodeWrite;

#define FUNCCODE_RW_DELAY_TIME_MAX  100     // * 2ms
Uint16 rwFuncCodeRepeatDelayTicker; // 读写功能码时，验证不一致，延时一段时间之后再读取。延时计时器

#define EEPROM_READ_SINGLE_MAX      250     // _*2ms，单次读EEPROM的超时时间
#define EEPROM_WRITE_SINGLE_MAX     500     // _*2ms，单次写EEPROM的超时时间
#define EEPROM_RW_FUNCCODE_MAX      5000    // _*2ms，连续读写功能码的超时时间

LOCALF Uint16 index4EepromDeal;     // eeprom地址
LOCALF Uint16 number4EepromDeal;

struct EEPROM_OPERATE_TIME eepromOperateTime;

#define RW_I2C_BUS_WRITE    0   // 写
#define RW_I2C_BUS_READ     1   // 读
#define RW_I2C_BUS_ACK      2   // 查询
LOCALD Uint16 RwI2cBus(Uint16 mode);

#define RW_EEPROM_WRITE     0   // 写
#define RW_EEPROM_READ      1   // 读
LOCALD Uint16 RwEeprom(Uint16 mode);
LOCALD void I2cIntDeal(void);
LOCALD void UpdateIndexForEepromDeal(void);

void I2cDealBeforeInit(void);
void I2CNoACK(void);
void I2CStop(void);
void I2CRcvByte(void);


//=====================================================================
//
// 读写I2C数据
//
//=====================================================================
LOCALF Uint16 RwI2cBus(Uint16 mode)
{
    int16 i;

   // Wait until the STP bit is cleared from any previous master communication.
   // Clearing of this bit by the module is delayed until after the SCD bit is
   // set. If this bit is not checked prior to initiating a new message, the
   // I2C could get confused.
    if (I2caRegs.I2CMDR.bit.STP == 1)
    {
        return I2C_STP_NOT_READY_ERROR;
    }
    
// Check if bus busy
    if ((I2caRegs.I2CSTR.bit.BB == 1) && (I2cMsg.status != I2C_MSG_STATUS_RESTART))
    {
        return I2C_BUS_BUSY_ERROR;
    }

// Setup slave address
#if (EEPROM_TYPE == EEPROM_24LC32)
    I2caRegs.I2CSAR = I2C_SLAVE_ADDR;
#elif 1
    I2caRegs.I2CSAR = (I2C_SLAVE_ADDR | (I2cMsg.highAddr >> 8)) & 0xff;
#endif    

    if (mode == RW_I2C_BUS_WRITE)
    {
        // Setup number of bytes to send
        // buffer + Address
#if (EEPROM_TYPE == EEPROM_24LC32)
        I2caRegs.I2CCNT = I2cMsg.bytes + 2;
#elif 1
        I2caRegs.I2CCNT = I2cMsg.bytes + 1;
#endif

        I2caRegs.I2CDXR = I2cMsg.highAddr; // Setup data to send
#if (EEPROM_TYPE == EEPROM_24LC32)
        I2caRegs.I2CDXR = I2cMsg.lowAddr;
#endif
        for (i = 0; i < I2cMsg.bytes; i++)
        {
            I2caRegs.I2CDXR = I2cMsg.buffer[i];
        }

        // Send start as master transmitter
        I2caRegs.I2CMDR.all = 0x6E20;   // S.A.D.P
    }
    else if ((mode == RW_I2C_BUS_READ) && (I2C_MSG_STATUS_RESTART == I2cMsg.status))
    {
        I2caRegs.I2CCNT = I2cMsg.bytes; // Setup how many bytes to expect
        
        I2caRegs.I2CMDR.all = 0x6C20;   // Send restart as master receiver
                                        // S.A.D.P
    }
    else                                // ACK, or start read
    {
#if (EEPROM_TYPE == EEPROM_24LC32)
        I2caRegs.I2CCNT = 2;
        I2caRegs.I2CDXR = I2cMsg.highAddr;
        I2caRegs.I2CDXR = I2cMsg.lowAddr;
#elif 1
        I2caRegs.I2CCNT = 1;
        I2caRegs.I2CDXR = I2cMsg.highAddr;
#endif
        
        I2caRegs.I2CMDR.all = 0x6620;   // Send data to setup EEPROM address
                                        // S.A.D
    }

    return I2C_SUCCESS;
}


//=====================================================================
//
// I2C总线状态处理
//
//=====================================================================
LOCALF void I2cIntDeal(void)
{
    Uint16 IntSource;

// Read interrupt source
    IntSource = I2caRegs.I2CISRC.all;

    if (IntSource == I2C_NO_ISRC)   // 没有中断标志，返回
        return;

    if (IntSource == I2C_SCD_ISRC)   // Interrupt source = stop condition detected
    {
        if (I2cMsg.status == I2C_MSG_STATUS_WRITE_BUSY)
        {
            Uint16 tmp = 0;
            
            while (RwI2cBus(RW_I2C_BUS_ACK) != I2C_SUCCESS) // 如果还没有完成，继续查询I2C总线状态
            {
                // The EEPROM will send back a NACK while it is performing
                // a write operation. Even though the write communique is
                // complete at this point, the EEPROM could still be busy
                // programming the data. Therefore, multiple attempts are
                // necessary.
                if (++tmp >= 100)
                    break;
            }
        }
        // If a message receives a NACK during the address setup portion of the
        // EEPROM read, the code further below included in the register access ready
        // interrupt source code will generate a stop condition. After the stop
        // condition is received (here), set the message status to try again.
        // User may want to limit the number of retries before generating an error.
        else if (I2cMsg.status == I2C_MSG_STATUS_SEND_NOSTOP_BUSY)
        {
            I2cMsg.status = I2C_MSG_STATUS_IDLE;
        }
        // If completed message was reading EEPROM data, reset msg to inactive state
        // and read data from FIFO.
        else if (I2cMsg.status == I2C_MSG_STATUS_READ_BUSY)
        {
            int16 i;
            
            for (i = 0; i < I2cMsg.bytes; i++)
            {
                I2cMsg.buffer[i] = I2caRegs.I2CDRR;
            }

            I2cMsg.status = I2C_MSG_STATUS_RW_OK;
        }
    }  // end of stop condition detected
    // Interrupt source = Register Access Ready
    // This interrupt is used to determine when the EEPROM address setup portion of the
    // read data communication is complete. Since no stop bit is commanded, this flag
    // tells us when the message has been sent instead of the SCD flag. If a NACK is
    // received, clear the NACK bit and command a stop. Otherwise, move on to the read
    // data portion of the communication.
    else if (IntSource == I2C_ARDY_ISRC)    // ARDY中断的发生条件参考I2CSTR说明！
    {
        if (I2caRegs.I2CSTR.bit.NACK == 1)
        {
            I2caRegs.I2CMDR.bit.STP = 1;    // 之后SCD=1，进入SCDINT
            I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;
        }
        else if (I2cMsg.status == I2C_MSG_STATUS_SEND_NOSTOP_BUSY)
        {
            I2cMsg.status = I2C_MSG_STATUS_RESTART;
        }
        else if (I2cMsg.status == I2C_MSG_STATUS_WRITE_BUSY)
        {
            I2cMsg.status = I2C_MSG_STATUS_RW_OK;

            InitSetI2ca();
            //I2caRegs.I2CMDR.all = 0x0000;    // reset I2C
            //I2caRegs.I2CMDR.all = 0x0020;
        }
    }
}


//=====================================================================
//
// Read/Write data from EEPROM section
// 返回值：0--还没有完成
//         1--完成
//
//=====================================================================
LOCALF Uint16 RwEeprom(Uint16 mode)
{
    Uint16 tmp = 0;
    
    if (I2C_MSG_STATUS_IDLE == I2cMsg.status)
    {
        if (RW_EEPROM_WRITE == mode)
        {
            if (RwI2cBus(RW_I2C_BUS_WRITE) == I2C_SUCCESS)
            {
                I2cMsg.status = I2C_MSG_STATUS_WRITE_BUSY;
            }
        }
        else //if (RW_EEPROM_READ == mode)
        {
            while (RwI2cBus(RW_I2C_BUS_READ) != I2C_SUCCESS)
            {
                if (++tmp >= 100)
                {
                    return 0;
                }
            }

            I2cMsg.status = I2C_MSG_STATUS_SEND_NOSTOP_BUSY;
        }
        return 0;
    }
    else if (I2C_MSG_STATUS_RESTART == I2cMsg.status)
    {
        while (RwI2cBus(RW_I2C_BUS_READ) != I2C_SUCCESS)
        {
            if (++tmp >= 100)
            {
                return 0;
            }
        }

        I2cMsg.status = I2C_MSG_STATUS_READ_BUSY;
        return 0;
    }
    else if (I2C_MSG_STATUS_RW_OK == I2cMsg.status) // 读写数据完成
    {
        I2cMsg.status = I2C_MSG_STATUS_IDLE;
        return 1;
    }
    else
        return 0;
}


//======================================================================================
//
// 从EEPROM(24LC08)中读取n个功能码
//
// 输入:
//      funcCodeRw->index  -- 读取功能码的起始index
//      funcCodeRw->number -- 读取功能码的个数, (0, 8]
// 输出：
//      funcCodeRw->data[] -- 保存读取功能码
// 返回：
//      FUNCCODE_READ_RET_READING    -- 正在读取
//      FUNCCODE_READ_RET_OK         -- 读取完毕，读取的数据放在data开始的地址里面
//      FUNCCODE_RW_RET_PARA_ERROR   -- 参数错误，目前仅是readNumber错误
//      FUNCCODE_RW_RET_EEPROM_ERROR -- 读取EEPROM出错，或者读取之前就已经有EEPROM错误
//
// 处理过程：
// 1. 开始读取，若连续读取(FUNCCODE_READ_VERIFY_RIGHT_TIME_MAX)次每次数据一致，读取完毕；
// 2. 若在读取中发现有不一致，延时(FUNCCODE_RW_DELAY_TIME_MAX * 1ms)，
//    重新再连续读取(FUNCCODE_READ_VERIFY_WRONG_TIME_MAX)次，每次数据一致，读取完毕；
// 3. 否则，重复第2步。
// 3. 若第2步重复次数达到(FUNCCODE_READ_TIME_MAX)次，报错。
// 4. 当前有读写EEPROM错误，不读取而直接退出。
//
//=======================================================================================
Uint16 ReadFuncCode(FUNCCODE_RW *funcCodeRw)
{
    static Uint16 readTime;      // 读取次数，仅用来判断是否是第一次读取
    static Uint16 readRightTime; // 连续读取且数据一致的次数
    static Uint16 bVerifyWrong;  // 验证不一致标志，延时使用
    Uint16 dataRead[EEPROM_PAGE_NUM_FUNC_CODE];
    int16 readRightTimeMax;
    int16 i;
    Uint16 tmp;

    eepromOperateTime.readFlag = 1;

// 检查输入参数
    if ((0 == funcCodeRw->number) || (funcCodeRw->number > EEPROM_PAGE_NUM_FUNC_CODE))
        return FUNCCODE_RW_RET_PARA_ERROR;

// 验证不一致，延时一段时间之后再读取
    if (bVerifyWrong && (rwFuncCodeRepeatDelayTicker < FUNCCODE_RW_DELAY_TIME_MAX))
        return FUNCCODE_READ_RET_READING; //FUNCCODE_READ_RET_WAIT_FOR_REPEAT;

    rwFuncCodeRepeatDelayTicker = 0;
    bVerifyWrong = 0;

// 读取EEPROM数据
// allows the entire memory contents to be serially read during one operation.
#if (EEPROM_TYPE == EEPROM_24LC32)
    tmp = (funcCodeRw->index + FUNCCODE_EEPROM_START_INDEX) << 1;
    I2cMsg.highAddr = tmp >> 8;
    I2cMsg.lowAddr = tmp & 0x00FF;
#elif 1
    I2cMsg.highAddr = (funcCodeRw->index + FUNCCODE_EEPROM_START_INDEX) << 1;
#endif
    
    I2cMsg.bytes = funcCodeRw->number << 1;
    if (1 == RwEeprom(RW_EEPROM_READ)) // 如果当前I2C没有准备好，或者I2C总线忙，会自动重读
    {
        readTime++;
        
        for (i = funcCodeRw->number - 1; i >= 0 ; i--)
        {
            dataRead[i] = (I2cMsg.buffer[i<<1] << 8) + I2cMsg.buffer[(i<<1)+1];
            if (1 == readTime)
            {
                funcCodeRw->data[i] = dataRead[i];
            }
            else
            {
                if (funcCodeRw->data[i] != dataRead[i])
                    break;
            }
        }

        if (1 == readTime)
        {
            readRightTime = 1;  // 验证不一致，则再连续读取_次，全部一致，认为OK. 
            return FUNCCODE_READ_RET_READING;
        }

        if (i < 0)           // 与上次(第一次)读取的数据一致
        {
#if 0
            if (!repeatTime)
                readRightTimeMax = FUNCCODE_READ_VERIFY_RIGHT_TIME_MAX;
            else
                readRightTimeMax = FUNCCODE_READ_VERIFY_WRONG_TIME_MAX;

            if (FUNCCODE_RW_STATUS_WRITE_DATA == funcCodeRwStatus) // 写之后读取验证
                readRightTimeMax--;
#elif 1
            readRightTimeMax = FUNCCODE_READ_VERIFY_RIGHT_TIME_MAX;
            if (FUNCCODE_RW_STATUS_WRITE_DATA == funcCodeRwStatus) // 写之后读取验证
                readRightTimeMax = FUNCCODE_READ_AFTER_WRITE_VERIFY_RIGHT_TIME_MAX;
#endif

            // 从开始读取验证，连续读取_次，全部一致，OK
            // 验证不一致，则再连续读取_次，全部一致，OK
            if (++readRightTime >= readRightTimeMax)
            {
                readTime = 0;
                eepromOperateTime.readFlag = 0;     // 读完成
                eepromOperateTime.readTicker = 0;   // ticker清零

                return FUNCCODE_READ_RET_OK;
            }
        }
        else                   // 与上次读取的数据不一致
        {
            readTime = 0;      // 第一次的读取就有可能不正确，要重新读取
            bVerifyWrong = 1;  // 验证不一致标志，延时使用
            
            return FUNCCODE_READ_RET_READING; // FUNCCODE_READ_RET_WAIT_FOR_REPEAT;
        }
    }

    return FUNCCODE_READ_RET_READING;   // 正在读取
}


//=====================================================================
//
// 往EEPROM(24LC08)写入n个功能码
//
// 输入：
//      funcCodeRw->index  -- 写入功能码的起始index
//      funcCodeRw->data   -- 写入功能码的起始地址
//      funcCodeRw->number -- 写入功能码的个数, 范围：[0, 8]，且不能超过一页
// 输出：
//      无。
// 返回：
//      FUNCCODE_WRITE_RET_WRITING   -- 正在写入
//      FUNCCODE_WRITE_RET_OK        -- 写入完毕
//      FUNCCODE_RW_RET_PARA_ERROR   -- 参数错误，目前仅是number错误
//      FUNCCODE_RW_RET_EEPROM_ERROR -- 写入EEPROM出错，或者写入之前就已经有EEPROM错误
//
// 处理过程：
// 1. 写入。
// 2. 读取验证。一致，写入完毕。
// 3. 若验证不一致，延时(FUNCCODE_RW_DELAY_TIME_MAX * 1ms)之后，
//    返回第1步(重新写入，再读取验证)。
// 4. 若重复写入次数达到(FUNCCODE_WRITE_TIME_MAX)，报错。
// 5. 当前有读写EEPROM错误，不读取而直接退出。
//
//=====================================================================
Uint16 WriteFuncCode(FUNCCODE_RW *funcCodeRw)
{
    static enum FUNCCODE_WRITE_STATUS writeStatus;
    static Uint16 bVerifyWrong;  // 验证不一致标志，延时使用
    static Uint16 number;
    int16 i;
    Uint16 tmp;

    eepromOperateTime.writeFlag = 1;

// 检查输入参数
#if DSP_2803X    // 2803x还是2808平台
    if ((funcCodeRw->index + funcCodeRw->number > (funcCodeRw->index & 0xFFFE) + 0x0002) // 不能超过一页,注意是 > , 不是 >= !!
#elif 1
    if ((funcCodeRw->index + funcCodeRw->number > (funcCodeRw->index & 0xFFF8) + 0x0008) // 不能超过一页,注意是 > , 不是 >= !!
#endif
        || (0 == funcCodeRw->number) || (funcCodeRw->number > EEPROM_PAGE_NUM_FUNC_CODE))
    {
        return FUNCCODE_RW_RET_PARA_ERROR;
    }

// 验证不一致，延时一段时间之后重新写入
    if (bVerifyWrong && (rwFuncCodeRepeatDelayTicker < FUNCCODE_RW_DELAY_TIME_MAX))
        return FUNCCODE_WRITE_RET_WRITING; //FUNCCODE_WRITE_RET_WAIT_FOR_REPEAT;
        
    rwFuncCodeRepeatDelayTicker = 0;
    bVerifyWrong = 0;

// 写入EEPROM数据
    if (FUNCCODE_WRITE_STATUS_WRITE == writeStatus)
    {
        if (EEPROM_PAGE_NUM_FUNC_CODE == funcCodeRw->number) // 如果是8个，先写7个；然后写最后1个
            number = EEPROM_PAGE_NUM_FUNC_CODE - 1;
        else
            number = funcCodeRw->number;

        for (i = number - 1; i >= 0; i--)
        {
            I2cMsg.buffer[i << 1] = funcCodeRw->data[i] >> 8;
            I2cMsg.buffer[(i << 1) + 1] = funcCodeRw->data[i] & 0x00ff;
        }

#if (EEPROM_TYPE == EEPROM_24LC32)
        tmp = (funcCodeRw->index + FUNCCODE_EEPROM_START_INDEX) << 1;
        I2cMsg.highAddr = tmp >> 8;
        I2cMsg.lowAddr = tmp & 0x00FF;
#elif 1
        I2cMsg.highAddr = (funcCodeRw->index + FUNCCODE_EEPROM_START_INDEX) << 1;
#endif
        I2cMsg.bytes = number << 1;

        if (1 == RwEeprom(RW_EEPROM_WRITE))     // 如果当前I2C没有准备好，或者I2C总线忙，会自动重写
        {
            if (number == funcCodeRw->number)
                writeStatus = FUNCCODE_WRITE_STATUS_READ_TO_VERIFY_WRITE;
            else
                writeStatus = FUNCCODE_WRITE_STATUS_WRITE_REMAINDER;
        }
        else
            return FUNCCODE_WRITE_RET_WRITING;
    }

// 写剩余的1个功能码
    if (FUNCCODE_WRITE_STATUS_WRITE_REMAINDER == writeStatus)
    {
        I2cMsg.buffer[0] = funcCodeRw->data[EEPROM_PAGE_NUM_FUNC_CODE - 1] >> 8;
        I2cMsg.buffer[1] = funcCodeRw->data[EEPROM_PAGE_NUM_FUNC_CODE - 1] & 0x00ff;
        
#if (EEPROM_TYPE == EEPROM_24LC32)
        tmp = (funcCodeRw->index + FUNCCODE_EEPROM_START_INDEX + EEPROM_PAGE_NUM_FUNC_CODE - 1) << 1;
        I2cMsg.highAddr = tmp >> 8;
        I2cMsg.lowAddr = tmp & 0x00FF;
#elif 1
        I2cMsg.highAddr = (funcCodeRw->index + FUNCCODE_EEPROM_START_INDEX + EEPROM_PAGE_NUM_FUNC_CODE - 1) << 1;
#endif
        I2cMsg.bytes = 1 << 1;

        if (1 == RwEeprom(RW_EEPROM_WRITE))     // 如果当前I2C没有准备好，或者I2C总线忙，会自动重写
            writeStatus = FUNCCODE_WRITE_STATUS_READ_TO_VERIFY_WRITE;
        else
            return FUNCCODE_WRITE_RET_WRITING;
    }

// 写入完成之后，读取进行验证
    if (FUNCCODE_WRITE_STATUS_READ_TO_VERIFY_WRITE == writeStatus)
    {
        funcCodeRead.index = funcCodeRw->index;
        funcCodeRead.number = funcCodeRw->number;

        if (FUNCCODE_READ_RET_OK == ReadFuncCode(&funcCodeRead))
        {
            for (i = funcCodeRw->number - 1; i >= 0; i--)
            {
                if (funcCodeRead.data[i] != funcCodeRw->data[i]) // 写入完成之后读取进行验证
                    break;
            }

            if (i < 0)          // 读取的数据与写入的数据一致
            {
                eepromOperateTime.writeFlag = 0;            // 写完成
                eepromOperateTime.writeTicker = 0;          // ticker清零
                writeStatus = FUNCCODE_WRITE_STATUS_WRITE;  // 写入完毕，状态复位
                
                return FUNCCODE_WRITE_RET_OK;
            }
            else                    // 读取的数据与写入的数据不一致
            {
                bVerifyWrong = 1;   // 验证不一致标志，延时使用
                writeStatus = FUNCCODE_WRITE_STATUS_WRITE; // 验证不一致，延时一段时间之后重新写入
                
                return FUNCCODE_WRITE_RET_WRITING; //FUNCCODE_WRITE_RET_WAIT_FOR_REPEAT;
            }
        }
        else
            return FUNCCODE_WRITE_RET_WRITING; //FUNCCODE_WRITE_RET_READING;
    }

    return FUNCCODE_WRITE_RET_WRITING; // 返回值有FUNCCODE_WRITE_RET_READING时，其实不需要
}


//=====================================================================
//
// EEPROM处理，即功能码的读取、保存
// 处理内容：
//     FUNCCODE_RW_MODE_WRITE_ONE                  ---- 保存一个功能码
//     FUNCCODE_RW_MODE_WRITE_ALL                  ---- 全部功能码恢复出厂参数
//     FUNCCODE_RW_MODE_WRITE_SERIES               ---- 连续写
//     FUNCCODE_RW_MODE_READ_ALL                   ---- 读取全部功能码
//     FUNCCODE_paraInitMode_CLEAR_RECORD          ---- 清除记录信息
//     FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA  ---- 恢复出厂设定值(不包含电机参数)
//     FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL  ---- 恢复出厂设定值(包含电机参数)
//     FUNCCODE_paraInitMode_SAVE_USER_PARA        ---- 保存当前用户功能码到EEPROM的剩余空间
//     FUNCCODE_paraInitMode_RESTORE_USER_PARA     ---- 恢复保存的用户功能码
//
// 处理过程：
// 1. 读取功能码在EEPROM的当前数据。
// 2. 读取需要写入的数据。
// 3. 确定是否数据需要写入：全部与当前数据相等，则跳过；否则，写入。
// 4. 写入数据
//
//=====================================================================
void EepromDeal(void)
{
    int16 i;
    Uint16 fcIndex;
    
#if F_DEBUG_RAM     // 仅调试功能，在CCS的build option中定义的宏
// 防止功能码超过EEPROM的容量
    if (((FNUM_EEPROM + 0 + FUNCCODE_EEPROM_START_INDEX) << 1) > 4096)     // 4K bytes
    {    
        //errorEeprom = ERROR_EEPROM_WRITE_NUM_OVER;
        errorEeprom = FNUM_EEPROM + 1 + FUNCCODE_EEPROM_START_INDEX;
        errorCode = 01; // 01未使用
    }
#endif

    if (funcCodeOneWriteNum >= (FUNCCODE_ONE_WRITE_NUMBER_MAX >> 1))    // 防止funcCodeOneWriteIndex越界
    {
        funcCodeOneWriteNum = FUNCCODE_ONE_WRITE_NUMBER_MAX >> 1;       // 防止溢出

        if (POWER_ON_WAIT != powerOnStatus)  // 防止上电发现功能码超过限值，报错。而这可能是正常的。
        {
            errorEeprom = ERROR_EEPROM_WRITE_NUM_OVER;
        }
    }

#if 0   // EEPROM错误时，也可以继续执行
    if (errorEeprom)    // EEPROM错误，不执行本函数
        return;
#endif

//---------------------------------------------------------
// I2C总线状态处理
    I2cIntDeal();
//---------------------------------------------------------


//---------------------------------------------------------
// 判断是否正在操作EEPROM。当前没有操作EEPROM，可以响应EEPROM操作；否则，等待当前EEPROM操作完成。
    if (FUNCCODE_RW_MODE_NO_OPERATION == funcCodeRwMode)
    {
        // 有(成片的/连续的)功能码需要操作
        if (funcCodeRwModeTmp)
        {
            funcCodeRwMode = funcCodeRwModeTmp;
            if (FUNCCODE_RW_MODE_READ_ALL == funcCodeRwMode)
            {
                index4EepromDeal = 0;
            }
            else
            {
                index4EepromDeal = EEPROM_INDEX_USE_INDEX;      // 功能参数使用地址起始
            }
            endIndexRwFuncCode = EEPROM_INDEX_USE_LENGTH;   // EEPROM的使用长度
            if (FUNCCODE_RW_MODE_WRITE_SERIES == funcCodeRwMode) // 连续写
            {
                // 连续写的起始地址
                index4EepromDeal = GetEepromIndexFromFcIndex(startIndexWriteSeries);
                // 要加1，与endIndexRwFuncCode类似
                endIndexRwFuncCode = GetEepromIndexFromFcIndex(endIndexWriteSeries) + 1;
            }
        }
        else if (funcCodeOneWriteNum)   // 还有(一个一个的)功能码需要保存
        {
            funcCodeRwMode = FUNCCODE_RW_MODE_WRITE_ONE;

            funcCodeOneWriteNum--;      // 需要保存的功能码的数量减少1
            
            index4EepromDeal = GetEepromIndexFromFcIndex(funcCodeOneWriteIndex[funcCodeOneWriteNum]);
        }
        else                            // EEPROM仍然没有读写命令(除 上电时读取EEPROM_CHECK 之外)
        {
            // 上电时读取EEPROM_CHECK，也有可能发生EEPROM读写延时错误
            // 但是，目前读写一个/页，也有时间限制
            return;
        }
    }

// 准备读取功能码。准备index(要写入功能码的起始index)和number(要写入的功能码个数)
    if (FUNCCODE_RW_STATUS_PREPARE_DATA_FOR_EAD == funcCodeRwStatus)
    {
        if (FUNCCODE_RW_MODE_WRITE_ONE == funcCodeRwMode)
        {
            number4EepromDeal = 1;
        }
        else
        {
            Uint16 tmp;     // 下一个循环的起始index
#if DSP_2803X     // 2803x还是2808平台            
            tmp = (index4EepromDeal & 0xFFFE) + EEPROM_PAGE_NUM_FUNC_CODE;
#elif 1
            tmp = (index4EepromDeal & 0xFFF8) + EEPROM_PAGE_NUM_FUNC_CODE;
#endif
            if (tmp > endIndexRwFuncCode)
            {
                tmp = endIndexRwFuncCode;
            }
            number4EepromDeal = tmp - index4EepromDeal;
        }
        
        funcCodeRead.index = index4EepromDeal;
        funcCodeRead.number = number4EepromDeal;
        
        if (FUNCCODE_RW_MODE_WRITE_ALL == funcCodeRwMode) // 恢复全部出厂参数，没有必要先读取
        {
            funcCodeRwStatus = FUNCCODE_RW_STATUS_PREPARE_DATA_FOR_WRITE;
        }
        else
        {
#if PARA_INIT_MODE_SAVE_ALL_CODE
            if (FUNCCODE_paraInitMode_SAVE_USER_PARA == funcCodeRwMode) // 保存当前的全部(用户)功能码
                funcCodeRead.index = index4EepromDeal + USER_PARA_START_INDEX;    // USER_PARA_START_INDEX必须为8的倍数!
#endif

            for (i = 0; i < number4EepromDeal; i++)
            {
                fcIndex = GetFcIndexFromEepromIndex(index4EepromDeal + i);  // 获取功能码index
                if (FUNCCODE_RSVD4ALL_INDEX != fcIndex)     // 该EEPROM地址不是保留的
                    break;
            }
            
            if (i == number4EepromDeal)     // 本段EEPROM地址全部是保留的
            {
                UpdateIndexForEepromDeal();
            }
            else    // 否则，需要操作
            {
                funcCodeRwStatus = FUNCCODE_RW_STATUS_READ_CURRENT_DATA;
            }
        }
    }

// 读取功能码在EEPROM的当前数据。
    if (FUNCCODE_RW_STATUS_READ_CURRENT_DATA == funcCodeRwStatus)
    {
        if (FUNCCODE_READ_RET_OK == ReadFuncCode(&funcCodeRead))
        {
            funcCodeRwStatus = FUNCCODE_RW_STATUS_PREPARE_DATA_FOR_WRITE;
        }
    }

// 准备要写入的数据
    if (FUNCCODE_RW_STATUS_PREPARE_DATA_FOR_WRITE == funcCodeRwStatus)
    {
#if PARA_INIT_MODE_SAVE_ALL_CODE
        if ((FUNCCODE_paraInitMode_SAVE_USER_PARA == funcCodeRwMode) // 恢复用户保存的功能码
            || (FUNCCODE_paraInitMode_RESTORE_USER_PARA == funcCodeRwMode)) // 保存当前的全部(用户)功能码
        {
            if (FUNCCODE_paraInitMode_RESTORE_USER_PARA == funcCodeRwMode) // 恢复
                funcCodeWrite.index = index4EepromDeal + USER_PARA_START_INDEX;
            else
                funcCodeWrite.index = index4EepromDeal;

            funcCodeWrite.number = number4EepromDeal;

            if (FUNCCODE_READ_RET_OK != ReadFuncCode(&funcCodeWrite))
            {
                return;
            }
        }
#endif
        
        for (i = 0; i < number4EepromDeal; i++)
        {
            fcIndex = GetFcIndexFromEepromIndex(index4EepromDeal + i);  // 获取功能码index
            
            if (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA == funcCodeRwMode)   // 恢复(部分)出厂参数
            {
                funcCode.all[fcIndex] = GetFuncCodeInit(fcIndex, 0);
            }
            else if(FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL == funcCodeRwMode)
            {
                funcCode.all[fcIndex] = GetFuncCodeInit(fcIndex, 1); // 恢复全部出厂值
            }
            else if (FUNCCODE_RW_MODE_READ_ALL == funcCodeRwMode) // 读取全部功能码
            {
                funcCode.all[fcIndex] = funcCodeRead.data[i];
            }
            else if (FUNCCODE_RW_MODE_WRITE_ALL == funcCodeRwMode) // 所有功能码恢复出厂参数
            {
                // aiaoChckReadOK为1时，AI1\AI2\AO1已经校正过不需要读取
                if ((((fcIndex >= AI1_CALB_START) && (fcIndex <= AI2_CALB_STOP))
                    || ((fcIndex >= AO1_CALB_START) && (fcIndex <= AO1_CALB_STOP)))
                    && (aiaoChckReadOK == AIAO_CHK_READ_OK)
					)
                {
                    ; 
                }
                else if ((fcIndex == SAVE_USER_PARA_PARA1) 
                    || (fcIndex == SAVE_USER_PARA_PARA2))
                {
                    ;
                }
                else
                {
                    funcCode.all[fcIndex] = GetFuncCodeInitOriginal(fcIndex); 
                }
                // 这里没有考虑与机型相关的功能码
                // 初次上电，发现EEPROM是新的，请手动更改机型
                
                //-=funcCode.all[fcIndex] = 99;// 调试

                // 所有功能码恢复出厂参数，一定需要写入
                funcCodeRwStatus = FUNCCODE_RW_STATUS_WRITE_DATA;
            }
            else if (FUNCCODE_RW_MODE_WRITE_ONE == funcCodeRwMode)  // 写一个功能码
            {
                ;   // 什么都不需要做，数据已经准备好
            }
            else if (FUNCCODE_RW_MODE_WRITE_SERIES == funcCodeRwMode) // 连续写功能码
            {
                ;   // 什么都不需要做，数据已经准备好
            }
#if PARA_INIT_MODE_SAVE_ALL_CODE
            else if (FUNCCODE_paraInitMode_RESTORE_USER_PARA == funcCodeRwMode) // 恢复
            {
                funcCode.all[fcIndex] = funcCodeWrite.data[i];
            }
#endif

            funcCodeWrite.data[i] = funcCode.all[fcIndex];  // 准备要写入的数据

            // 确定是否数据需要写入：全部与当前EEPROM数据相等，则跳过；否则，需要写入。
            // 读取全部功能码，不会有写操作
            if (funcCodeWrite.data[i] != funcCodeRead.data[i])
            {
                funcCodeRwStatus = FUNCCODE_RW_STATUS_WRITE_DATA;
            }
        }

        // EEPROM当前值与需要写入值完全一致，不需要真正的写EEPROM
        // 读取全部功能码，不会写入
        if (FUNCCODE_RW_STATUS_WRITE_DATA != funcCodeRwStatus)
        {
            UpdateIndexForEepromDeal();
        }
        else    // 需要写
        {
            funcCodeWrite.index = index4EepromDeal;
            funcCodeWrite.number = number4EepromDeal;

#if PARA_INIT_MODE_SAVE_ALL_CODE
            if (FUNCCODE_paraInitMode_SAVE_USER_PARA == funcCodeRwMode) // 保存当前的全部(用户)功能码
                funcCodeWrite.index = index4EepromDeal + USER_PARA_START_INDEX;
#endif
        }
    }

// 写入数据
    if (FUNCCODE_RW_STATUS_WRITE_DATA == funcCodeRwStatus)
    {
        if (FUNCCODE_WRITE_RET_OK == WriteFuncCode(&funcCodeWrite))
        {
            UpdateIndexForEepromDeal();
        }
    }

#if (F_DEBUG_RAM && (DEBUG_F_EEPROM))
// 调试使用，把全部功能码写成__数据
    if (FUNCCODE_RW_STATUS_DEBUG == funcCodeRwStatus)
    {
        Uint16 all = FNUM_EEPROM; // 1024
        
        funcCodeRwMode = FUNCCODE_DEBUG;
        funcCodeWrite.index = index4EepromDeal;
        funcCodeWrite.number = all - funcCodeWrite.index;
        if (funcCodeWrite.number > EEPROM_PAGE_NUM_FUNC_CODE)
            funcCodeWrite.number = EEPROM_PAGE_NUM_FUNC_CODE;

        for (i = funcCodeWrite.number - 1; i >= 0; i--)
            funcCodeWrite.data[i] = ((index4EepromDeal+i) << 1) + 0x0aec;//0x345d; //0xffff;   // 把全部功能码写成__数据
        if (FUNCCODE_WRITE_RET_OK == WriteFuncCode(&funcCodeWrite))
        {
            index4EepromDeal += funcCodeWrite.number;        // 下一次准备写入功能码的indexForEepromDeal
            if (index4EepromDeal >= all)
                funcCodeRwMode = FUNCCODE_RW_MODE_NO_OPERATION;
        }
    }
#endif
}


//=====================================================================
//
// 更新indexForEepromDeal
//
//=====================================================================
LOCALF void UpdateIndexForEepromDeal(void)
{
    funcCodeRwStatus = FUNC_CODE_RW_STATUS_INIT;    
    // 不能放在if (FUNCCODE_RW_MODE_NO_OPERATION == funcCodeRwMode){}中
    // 因为，连续操作时，要不断循环

    if (FUNCCODE_RW_MODE_WRITE_ONE == funcCodeRwMode) // 保存几个功能码
    {
        funcCodeRwMode = FUNCCODE_RW_MODE_NO_OPERATION;
    }
    else
    {
        index4EepromDeal += number4EepromDeal;  // 下一次准备写入功能码的indexForEepromDeal
        if (index4EepromDeal >= endIndexRwFuncCode)  // 全部写入到EEPROM中
        {
            if ((FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA == funcCodeRwMode) ||     // 恢复(部分)出厂参数
                (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL == funcCodeRwMode))   // 恢复(全部)出厂参数
            {
                RestoreCompanyParaOtherDeal();  // 全部恢复之后还需要处理
            }

            // 存储已进行用户存储EEPROM操作
            if (FUNCCODE_paraInitMode_SAVE_USER_PARA == funcCodeRwMode)
            {
                funcCode.code.saveUserParaFlag1 = USER_PARA_SAVE_FLAG1;
                funcCode.code.saveUserParaFlag2 = USER_PARA_SAVE_FLAG2;
            }
            
            funcCodeRwMode = FUNCCODE_RW_MODE_NO_OPERATION; // 也要清零。考虑FUNCCODE_RW_MODE_WRITE_ONE的处理方式。
            funcCodeRwModeTmp = FUNCCODE_RW_MODE_NO_OPERATION;
        }
    }
}



//=====================================================================
//
// EEPROM时间处理，判断是否超时. 2ms调用1次
//
//=====================================================================
void EepromOperateTimeDeal(void)
{
    rwFuncCodeRepeatDelayTicker++;

    if (eepromOperateTime.readFlag)   // 正在进行EEPROM的读操作
    {
        if (++eepromOperateTime.readTicker > EEPROM_READ_SINGLE_MAX)   // 单次读超时，报错
        {
            eepromOperateTime.readTicker = 0;
            errorEeprom = ERROR_EEPROM_READ;
        }
    }

    if (eepromOperateTime.writeFlag)   // 正在进行EEPROM的写操作
    {
        if (++eepromOperateTime.writeTicker > EEPROM_WRITE_SINGLE_MAX)   // 单次写超时，报错
        {
            eepromOperateTime.writeTicker = 0;
            errorEeprom = ERROR_EEPROM_WRITE;
        }
    }

    if (FUNCCODE_RW_MODE_NO_OPERATION != funcCodeRwMode)    // EEPROM正在进行读写
    {// 判断是否有EEPROM读写超时错误。若一段时间内功能码读写没有完成，报错
        if (++eepromOperateTime.rwTicker >= EEPROM_RW_FUNCCODE_MAX)
        {
            eepromOperateTime.rwTicker = 0;
            errorEeprom = ERROR_EEPROM_RW_OVER_TIME;
        }
    }
    else
    {
        eepromOperateTime.rwTicker = 0;
    }
}




void InitI2CGpio(void)
{
   EALLOW;
/* Enable internal pull-up for the selected pins */
// Pull-ups can be enabled or disabled disabled by the user.
// This will enable the pullups for the specified pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;    // Enable pull-up for GPIO32 (SDAA)
    GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;    // Enable pull-up for GPIO33 (SCLA)

/* Set qualification for selected pins to asynch only */
// This will select asynch (no qualification) for the selected pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPBQSEL1.bit.GPIO32 = 3;  // Asynch input GPIO32 (SDAA)
    GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 3;  // Asynch input GPIO33 (SCLA)

/* Configure SCI pins using GPIO regs*/
// This specifies which of the possible GPIO pins will be I2C functional pins.
// Comment out other unwanted lines.

    GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 1;   // Configure GPIO32 for SDAA operation
    GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 1;   // Configure GPIO33 for SCLA operation

    EDIS;
}


void InitSetI2ca(void)
{
    // Initialize I2C
    I2caRegs.I2CMDR.all = 0x4000;    // reset I2C

#if (DSP_CLOCK == 100)      // DSP运行频率100MHz
    I2caRegs.I2CPSC.all = 9;        // Prescaler - need 7-12 Mhz on module clk, I2C module clock = 10MHz
#elif (DSP_CLOCK == 60)      // DSP运行频率60MHz
    I2caRegs.I2CPSC.all = 5;        // Prescaler - need 7-12 Mhz on module clk, I2C module clock = 10MHz
#endif

#if 0
    I2caRegs.I2CCLKL = 55;          // NOTE: must be non zero, clk低电平设置为 (55+5)*0.1us = 6us
    I2caRegs.I2CCLKH = 55;          // NOTE: must be non zero, clk高电平设置为 (55+5)*0.1us = 6us
#elif 1
    I2caRegs.I2CCLKL = 25;          // clk低电平设置为 (25+5)*0.1us = 3us
    I2caRegs.I2CCLKH = 25;
#elif 1     // 200KHz
    I2caRegs.I2CCLKL = 20;          // clk低电平设置为 (20+5)*0.1us = 2.5us
    I2caRegs.I2CCLKH = 20;
#elif 1
    I2caRegs.I2CCLKL = 10;           // 试验ok
    I2caRegs.I2CCLKH = 10;
#elif 1
    I2caRegs.I2CCLKL = 4;           // 试验ok
    I2caRegs.I2CCLKH = 9;
#endif

    I2caRegs.I2CIER.all = 0x0024;   // SCD & ARDY interrupts

    I2caRegs.I2CMDR.all = 0x4020;   // Take I2C out of reset
                                    // Stop I2C when suspended

    I2caRegs.I2CFFTX.all = 0x6040;  // Enable FIFO mode and TXFIFO
    I2caRegs.I2CFFRX.all = 0x2040;  // Enable RXFIFO, clear RXFFINT,
}


//--------------------------------------------------------------
#define SDA (GpioDataRegs.GPBDAT.bit.GPIO32)
#define SCL (GpioDataRegs.GPBDAT.bit.GPIO33)
#define SdaIoAsInput()  {                               \
        EALLOW;                                         \
        GpioCtrlRegs.GPBDIR.bit.GPIO32 = 0;             \
        EDIS;                                           \
    }
#define SdaIoAsOutput() {                               \
        EALLOW;                                         \
        GpioCtrlRegs.GPBDIR.bit.GPIO32 = 1;             \
        EDIS;                                           \
    }

#define I2cClkHigh()    (GpioDataRegs.GPBSET.bit.GPIO33 = 1)
#define I2cClkLow()     (GpioDataRegs.GPBCLEAR.bit.GPIO33 = 1)

#define I2cDataHigh()   (GpioDataRegs.GPBSET.bit.GPIO32 = 1)
#define I2cDataLow()    (GpioDataRegs.GPBCLEAR.bit.GPIO32 = 1)
// 在EEPROM初始化之前，EEPROM有可能占用I2C总线，直接初始化不会成功。
// 执行本函数之后，I2C总线由DSP控制。
void I2cDealBeforeInit(void)
{
    Uint16 a = 0;

// 初始化为IO口。
    EALLOW;
	GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;     // Enable pull-up for GPIO32 (SDAA)
	GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;	    // Enable pull-up for GPIO33 (SCLA)

	GpioCtrlRegs.GPBQSEL1.bit.GPIO32 = 3;   // Asynch input GPIO32 (SDAA)
    GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 3;   // Asynch input GPIO33 (SCLA)

	GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 0;    // Configure GPIO32 for SDAA operation
	GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 0;    // Configure GPIO33 for SCLA operation

    GpioCtrlRegs.GPBDIR.bit.GPIO32 = 1;     // output
    GpioCtrlRegs.GPBDIR.bit.GPIO33 = 1;     // output

    GpioDataRegs.GPBDAT.bit.GPIO32 = 0;	    // low
    GpioDataRegs.GPBDAT.bit.GPIO33 = 0;	    // low
    EDIS;

// 复位时EEPROM可能正在进行擦写操作，延时等待。
    DELAY_US(20000);    // 延长20ms，等待EEPROM写完毕
    I2CStop();

    SdaIoAsOutput();
    I2cDataHigh();
    SdaIoAsInput();
    if (!SDA)
    {
        a = 1;
    }
    else
    {
        SdaIoAsOutput();
        I2cDataLow();
        SdaIoAsInput();
        if (SDA)
        {
            a = 1;
        }
    }
    
    if (a == 1)     // 
    {
        I2CRcvByte();
        I2CNoACK();
        I2CStop();
    }
}


void I2CNoACK(void)
{
    DELAY_US(5);
    
    I2cDataHigh();
    DELAY_US(5);
    
    I2cClkHigh();
    DELAY_US(5);
    
    I2cClkLow();
    DELAY_US(5);
}


void I2CStop(void)
{
    I2cDataLow();
    DELAY_US(5);
    
    I2cClkHigh();
    DELAY_US(5);
    
    I2cDataHigh();
    DELAY_US(5);
}


void I2CRcvByte(void)
{
    int i;
    
// SDA为输入    
    SdaIoAsInput();
    for (i = 0; i < 800; i++)
    {
        I2cClkLow();
        DELAY_US(5);
        I2cClkHigh();
        DELAY_US(5);
    }

    I2cClkLow();
    DELAY_US(5);
    
// SDA恢复为输出
    SdaIoAsOutput();
    DELAY_US(5);
}
//--------------------------------------------------------------



// 从 EepromIndex(物理地址) 获得对应功能码的 CodeIndex(逻辑地址)
Uint16 GetFcIndexFromEepromIndex(Uint16 a)
{
    int16 i;

#if DEBUG_F_TABLE_FC2EEPROM_CONST
    i = eeprom2Fc[a];
#elif 1
    for (i = 0; i < EEPROM_INDEX_USE_LENGTH; i++)
    {
        if (GetEepromIndexFromFcIndex(i) == a)
            break;
    }
    if (i == EEPROM_INDEX_USE_LENGTH)   // 功能码中没有，预留
    {
        i = FUNCCODE_RSVD4ALL_INDEX;
    }
#endif

    return i;
}



extern const Uint16 fcNoAttri2Eeprom[];
// 从 CodeIndex(逻辑地址) 获得对应 EEPROM index(物理地址)
Uint16 GetEepromIndexFromFcIndex(Uint16 index)
{
    if (index >= GetCodeIndex(funcCode.group.fChk[0]))
    {
        return fcNoAttri2Eeprom[index - GetCodeIndex(funcCode.group.fChk[0])];
    }
    else
    {
        return funcCodeAttribute[index].eepromIndex;
    }
}







