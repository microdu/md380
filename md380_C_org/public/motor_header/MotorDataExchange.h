/***************************************************************
文件功能:
文件版本：
最新更新：
************************************************************/
#ifndef MOTOR_INCLUDE_H
#define MOTOR_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif


/***********************外部函数的声明********************************/
/*********************************************************************/
extern void ParSend05Ms(void);
extern void ParGet05Ms(void);
extern void ParSend2Ms(void);
extern void ParGet2Ms(void);
extern void ParSendTune(void);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================
