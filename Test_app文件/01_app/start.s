
                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset
                AREA    RESET, DATA, READONLY
				EXPORT  __Vectors
					
__Vectors       DCD     0x20000000+0x10000                  
                DCD     Reset_Handler              ; Reset Handler

				AREA    |.text|, CODE, READONLY

; Reset handler
Reset_Handler   PROC
				EXPORT  Reset_Handler             [WEAK]
                IMPORT  mymain
                ;IMPORT  copy_myself
                ;IMPORT |Image$$ER_IROM1$$Length|;代码段长度

                ;adr r0, Reset_Handler   ;r0 = 0x08009000
                ;bic r0,r0,#0xff

                ;ldr r1, =__Vectors;r1 = 0x20000000
                ;ldr r2,=|Image$$ER_IROM1$$Length|

                ;BL copy_myself
				;LDR SP, =(0x20000000+0x10000)
			    ;BL mymain
                ldr pc,=mymain

                ENDP
                
                 END

