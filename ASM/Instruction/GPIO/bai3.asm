;Lưu 2 hằng số vào vùng nhớ ROM, đọc và tính hiệu và xuất ra các Port 1

ORG 0000H
	
;---Chuong trinh chinh---
MOV DPTR, #TABLE       ;DPTR tro toi du lieu ROM

;Doc hang so 1 
MOV A, #00H
MOVC A, @A+DPTR        ;A = HANG SO THU 1
MOV R0, A              ;LUU VAO R0

;Doc hang so 2
MOV A, #01H
MOVC A, @A+DPTR        ;A = HANG SO THU 2
MOV R1, A              ; LUU VAO R1

;Tinh hieu: R0-R1
MOV A, R0
CLR C                  ;XOA CO CARRY
SUBB A, R1             ;A = R0 - R1

;Xuat ket qua ra Port 1
MOV P1, A

JMP $                  ;VONG LAP VO HAN
	
;---Bang hang so trong ROM---
	ORG 0100H
TABLE:
	DB 0AH, 03H        ;HAI GIA TRI LUU TAI DIA CHI 0100H
		
	END