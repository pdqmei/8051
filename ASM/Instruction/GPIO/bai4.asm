;Gán số tự nhiên vào bộ nhớ RAM có địa chỉ 30H. Tính tổng các số nguyên từ 0 đến
;giá trị trong ô nhớ có địa chỉ 30H. Lưu giá trị vào ô nhớ 40H và xuất ra Port 1

	ORG 0000H

	;---Gan gia tri n vao o nho 30H---
	MOV 30H, #05H    ; Gia su cho n = 5

	;---Khoi tao---
	MOV R1, 30H      ; R1 = n
	MOV R2, #00H     ; R2 = 0 (bien tam de tinh tong)
	CLR A            ; A = 0 (dung A chua tong)

LOOP:
	MOV A, R1        ; R1 chua gia tri cong tiep theo 
	ADD A, R2        ; R2 chua tong moi lan cong don 
	MOV R2, A        ; luu tong moi tinh vao R2
	DJNZ R1, LOOP    ; giam R1 va lap neu R1 khác 0
	
	;---Sau khi cong xong---
	MOV 40H, R2      ; luu tong vao o nho 40H
	MOV P1, R2       ; xuat ket qua ra Port 1
	
	JMP $            ; vong lap vo han
	END