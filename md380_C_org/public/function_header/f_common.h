#ifndef __F_COMMON_H__
#define __F_COMMON_H__



#include "f_funcCode.h"



Uint16 CrcValueByteCalc(const Uint16 *data, Uint16 length);
Uint16 CrcValueWordCalc(const Uint16 *data, Uint16 length);


Uint16 Reverse4Bit(Uint16 value);
Uint16 Reverse8Bit(Uint16 value);




#endif // __F_COMMON_H__







