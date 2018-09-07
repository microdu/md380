;//###########################################################################
* 该文件是DSP程序的入口文件，C语言编程的时候，该文件能够执行MAIN函数前的操作
;//###########################################################################
    .ref 	_c_int00
    .global code_start
	.global _MovePrgFrFlashToRam

***********************************************************************
* Description: Branch to code starting point
***********************************************************************

    .sect "codestart"

code_start:
    LB main_init       ;Branch User Code:One instruction only

***********************************************************************
* 在这里把所有的RAM区域初始化为0
***********************************************************************
	.text
main_init:
    SETC 	OBJMODE        		;Set OBJMODE for 28x object code
    EALLOW              		;Enable EALLOW protected register access
    MOVZ 	DP, #7029h>>6  		;Set data page for WDCR register
    MOV 	@7029h, #0068h  	;Set WDDIS bit in WDCR to disable WD
    EDIS                		;Disable EALLOW protected register access

	MOV		ACC,	#00H	
	MOVL 	XAR5,	#0000H		;Clear M0
	MOVL 	XAR4,	#(400H-1)
	RPT		@AR4
	|| MOV	*XAR5++,	ACC

	MOVL 	XAR5,	#0400H		;Clear M1
	MOVL 	XAR4,	#(400H-1)
	RPT		@AR4
	|| MOV	*XAR5++,	ACC

	MOVL 	XAR5,	#8000H		;Clear L0, L2, L3
	MOVL 	XAR4,	#(1000H-1)
	RPT		@AR4
	|| MOV	*XAR5++,	ACC
	
	MOVL 	XAR5,	#9000H		;Clear L1
	MOVL 	XAR4,	#(1000H-1)
	RPT		@AR4
	|| MOV	*XAR5++,	ACC

	LB _c_int00					;此处跳转到C语言的main函数处

***********************************************************************
* MovePrgFrFlashToRam是把程序从FLASH搬移到RAM的函数，C语言调用方式：
* MovePrgFrFlashToRam(src_addr, dst_addr, size)
***********************************************************************
_MovePrgFrFlashToRam:
	MOVL XAR7, ACC
	MOVL XAR6, *-SP[4]
	RPT	 @AR4
	|| PREAD *XAR6++, *XAR7
	LRETR

	.end
	
;//===========================================================================
;// End of file.
;//===========================================================================