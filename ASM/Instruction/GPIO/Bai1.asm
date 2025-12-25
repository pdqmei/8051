/*Chương trình này điều khiển LED nối với
chân P1.1 của vi điều khiển 8051 nhấp nháy liên tục, 
với khoảng thời gian bật và tắt cách nhau khoảng 200 ms.*/
//chương trình ASM cho vi điều khiển 8051 viết bởi Bai1.asm
    
    LED1 EQU P1.1 
	ORG 00H
	SETB LED1
LOOP:
	CPL LED1
	CALL DELAY200ms
	JMP LOOP
DELAY200ms:
	MOV R2, #200
LAP2:
	MOV R1, #200
LAP1:
	NOP
	NOP
	NOP
	DJNZ R1, LAP1
	DJNZ R2, LAP2
	RET
	END
