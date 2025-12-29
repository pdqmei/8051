;Thực hiện các tập lệnh cơ bản trong 8051: cộng, trừ, nhân, chia 2 số


ORG 0000H			; Bat dau tu dia chi 0
MOV R0, #0AH		; Gan so 10 vao R0
MOV R1, #5 			; Gan so 05 vao R14

; -------------Tinh Tong----------------
MOV A, R0
ADD A, R1			; A = R0 + R1
MOV P0, A			; Xuat ket qua tong ra Port 0 
; -------------Tinh Hieu----------------
MOV A, R0
CLR C				; Xoa co Carry de thuc hien phep tru
SUBB A, R1			; A = R0 - R1
MOV P1, A			; Xuat ket qua hieu ra Port 1
; -------------Tinh Tích----------------
MOV A, R0
MOV B, R1
MUL AB				; A = LSB, B = MSB
MOV P2, A 			; Xuat LSB cua tich ra Port 2 
MOV P3, B			; Xuat LSB cua tich ra Port 3