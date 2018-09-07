;***********************************************************************; 
;文件说明：用汇编语言实现的公共子函数
;文件版本：
;最新更新：
;***********************************************************************;

	   .def     _swap                     
	   .def     _abs                     
	   .def     _absl                     
       .def 	_qsin
       .def 	_qatan
       .def 	_qsqrt

	   .global  _swap                     
	   .global  _abs                     
	   .global  _absl                     
       .global  _qsin
       .global  _qatan
       .global  _qsqrt

       .text
_abs:	MOV     ACC,AL<<16      ; AH='x', AL=0
        ABS   	ACC             ; TC= sign(x), AH=abs(x)
        MOV     AL,AH
		LRETR

_absl:	ABS   	ACC
		LRETR

_swap:	FLIP	AL
		LRETR
;---------------------------------------------------------------
;SIN 函数，调用方式：qsin(angle);
;---------------------------------------------------------------

K5      .set 0x6480             ; Scaled to Q13
K4      .set 0x52FF             ; Scaled to Q20
K3      .set 0xAACC             ; Scaled to Q12
K2      .set 0x45B8             ; Scaled to Q15
K1      .set 0x7338             ; Scaled to Q14

_qsin:	PUSH	ST0
		PUSH	ST1

        SETC    SXM,OVM         ; ACC=x
        MOV     ACC,AL<<16      ; AH='x', AL=0
        CLRC    TC  
        ABSTC   ACC             ; TC= sign(x), AH=abs(x)

        LSL     ACC,#1          ; Convert to first quadrant (0 to pi/2)                     
        ABS     ACC         
        SFR     ACC,#1  
      
        MOVL    XT,ACC          ; XT=x in Q31 and in first quardrant            

        MPY     ACC,T,#K1   

        ADD     ACC,#K2<<14     ; ACC=K1*x+K2 in Q29
        
        QMPYL   ACC,XT,ACC      ; ACC=(K1*x+K2)*x in Q28
        ADD     AH,#K3          ; ACC=(K1*x+K2)*x+K3 in Q28

        QMPYL   ACC,XT,ACC      ; ACC=((K1*x+K2)*x+K3)*x in Q27
        ADD     ACC,#K4<<7      ; ACC=((K1*x+K2)*x+K3)*x+K4 in Q27
        
        QMPYL   ACC,XT,ACC      ; ACC=(((K1*x+K2)*x+K3)*x+K4)*x in Q26
        ADD     ACC,#K5<<13     ; ACC=(((K1*x+K2)*x+K3)*x+K4)*x+K5 in Q26
        QMPYL   ACC,XT,ACC      ; ACC=((((K1*x+K2)*x+K3)*x+K4)*x+K5)*x in Q25
        
        LSL     ACC,#6          ; in Q31            
        ABS     ACC             ; Saturate to 0x7fff
        NEGTC   ACC             ; ACC=-sin(x), if TC=1
        MOV     AL,AH

		POP		ST1
		POP		ST0
        LRETR
       
;---------------------------------------------------------------
;atan 函数，调用方式：qatan(x);  x - Q16.16
;---------------------------------------------------------------
A1      .set    0x5179      ; in Q16 for  0.318253 
A2      .set    0x6C97      ; in Q23 for  0.003314 
A3      .set    0xBCFA      ; in Q17 for -0.130908
A4      .set    0x462F      ; in Q18 for  0.068542 
A5      .set    -2401       ; in Q18 for -0.009159

_qatan:     PUSH	ST0
			PUSH	ST1
			
			SETC    SXM,OVM
            MOV     AR4,AH          ; AR4=HI(X)
            ABS     ACC             ; x=abs('X') 
            LSR64   ACC:P,#1
            MOV     AR5,AL          ; AR5=x
            
            CSB     ACC             ; Count sign bits, T=E+1
            LSL64   ACC:P,T         ; Remove upto 15 extra sign bits
                                    ; T=nrof(sign bits)
            MOV     PH,AH
            TCLR    T,#4            ; T=1, if x<1, clear bit 4
            
; 1/|X| is obtained by 16 bit division, though the X input was in 16.16 format,         
; The strategy adopted here uses the 16 significant bits obtained through removing 
; upto 15 sign bits for division. 

            MOV     ACC,#1<<14      ; ACC=#1 in Q14
            LSLL    ACC,T           ; ACC=#1 in 17-E.15+E format

            RPT     #15
            || SUBCU ACC,PH         ; (17-E.15+E)/(16-E.E)= 1.15 format
            
            MOV     ACC,AL<<16 
            ABS     ACC             ; AH=1/x
            MOV     AR5,AH,NTC      ; AR5=x or 1/x in Q15 format
            MOV     TL,#0
            MOV     T,AR5           ; T=x in Q15 format
            
            MPY     ACC,@T,#A5      ; ACC=x*a5 in Q33 

            ADD     ACC,#A4<<15     ; ACC=a4+x*a5 in Q33
 
            QMPYL   ACC,XT,@ACC     ; ACC=x*(a4+x*a5) in Q32
            ADD     ACC, #A3<<15    ; ACC=a3+x*(a4+x*a5) in Q32
            
            QMPYL   ACC,XT,@ACC     ; ACC=x*(a3+(a4+x*a5)) in Q31
            ADD     ACC, #A2<<8     ; ACC=a2+x*(a3+(a4+x*a5)) in Q31
            
            QMPYL   ACC,XT,@ACC     ; ACC=x*(a2+x*(a3+(a4+x*a5))) in Q30
            ADD     ACC, #A1<<14    ; ACC=a1+x*(a2+x*(a3+(a4+x*a5))) in Q30 
            
            QMPYL   ACC,XT,@ACC     ; ACC=x*(a1+x*(a2+x*(a3+(a4+x*a5)))) in Q29
            
            SFR     ACC,#14         ; ACC=atan(x) in Q15 format
            MOVL    P,ACC           ; P=arctan(x)
            NEG     ACC             ; ACC=atan(x)
            ADD     ACC,#4000h      ; ACC=0.5-atan(1/x) in Q15 format           
            MOVL    P,ACC,NTC       
            MOVL    ACC,P           ; ACC=0.5-atan(x), if X>1
                                    ; ACC=atan(x), if X<1
            TBIT    AR4,#15         ; TC=sign(X)
            NEGTC   ACC
		
			POP		ST1
			POP		ST0
            LRETR           

;---------------------------------------------------------------
;atan 函数，调用方式：qatan(x);  x - Q16.16
;---------------------------------------------------------------
B0      .set    06a48h          ; 0.1037903  scaled by 2^18
B1      .set    05d1dh          ; 0.7274475 scaled by 2^15 
B2      .set    0a9edh          ; -0.672455 scaled by 2^15
B3      .set    046d6h          ; 0.553406 scaled by 2^15
B4      .set    0bb54h          ; -0.2682495 scaled by 2^16 
B5      .set    00e5ah          ; 0.0560605 scaled by 2^16 
SQRT2   .set    05a82h           ;(1/sqrt(2)) in Q15 format
                                ; Also sqrt(2) in Q14 format

_qsqrt:		PUSH	ST0
			PUSH	ST1
                                    ; ACC=X in Q16 format
            SETC    SXM             ; Set the sign ext. mode
            MPY     P,T,#0          ; P=0 
            LSR64   ACC:P,#1        ; X=X/2

            CSB     ACC             ; Count sign bits, T=E
            LSL64   ACC:P,T         ; ACC=x=X/(2^(E-15) in  Q31 format

            TBIT    @T,#0           ; TC=odd/even shift indicator
            MOV     AR4,T           ; AH=n             
            MOVL    XT,ACC          ; T=x in Q31
            MOV     AH,AR4          ; AH=n
            LSR     AH,#1           ; AH=n/2
            MOV     AR4,AH          ; AR4=n/2
   
            MPY     ACC,T,#B5       ; ACC=x*a5 in Q31 
            ADD     ACC,#B4<<15     ; ACC=a4+x*a5 in Q31
 
            QMPYL   ACC,XT,@ACC     ; ACC=x*(a4+x*a5) in Q30
            ADD     ACC, #B3<<15    ; ACC=a3+x*(a4+x*a5) in Q30
            
            QMPYL   ACC,XT,@ACC     ; ACC=x*(a3+(a4+x*a5)) in Q29
            ADD     ACC, #B2<<14    ; ACC=a2+x*(a3+(a4+x*a5)) in Q29
            
            QMPYL   ACC,XT,@ACC     ; ACC=x*(a2+x*(a3+(a4+x*a5))) in Q28
            ADD     ACC, #B1<<13    ; ACC=a1+x*(a2+x*(a3+(a4+x*a5))) in Q28 
            
            QMPYL   ACC,XT,@ACC     ; ACC=x*(a1+x*(a2+x*(a3+(a4+x*a5)))) in Q27
            ADD     ACC, #B0<<9     ; ACC=a0+x*(a1+x*(a2+x*(a3+(a4+x*a5)))) in Q27
                                    ; ACC=0.5sqrt(s*x) in Q27
                                    ; ACC=sqrt(s*x) in Q26

;*********** De-normalise the result ****************************   
        
            MOVH    T,ACC<<5        ; ACC=sqrt(s*x) in Q15
            MPY     P,T,#SQRT2      ; P=sqrt(s*x)*(1/sqrt(2)) in Q30 format
                                    
            LSL     ACC,#4          ; ACC=sqrt(s*x) in Q30  
            MOVL    P,ACC,NTC       ; P=sqrt(s*x) in Q30, if n is odd
            MOVL    ACC,P

            MOV     T,AR4
            LSRL    ACC,T           ; ACC=sqrt(x) in Q30
            MOVH    AL,ACC<<2       ; AL=sqrt(x)    

			POP		ST1
			POP		ST0
            LRETR
