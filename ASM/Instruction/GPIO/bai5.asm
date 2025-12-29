;Lưu 2 giá trị vào bộ nhớ RAM nội, sử dụng cách truy xuất bằng địa chỉ gián tiếp
;Thực hiện phép chia 2 số nay và xuất kết quả ra Port P1, P2. Viết chương trình
;con thực hiện phép chia và xuất ra 2 port 

ORG 0000H

	MOV R0, #30H
	MOV A, #20
	MOV @R0, A
	
	MOV R1, #31H
	MOV A, #6
	MOV @R1, A
	
	CALL DIVIDE
	
	JMP $
	
DIVIDE:
	MOV R0, #30H
	MOV A, @R0
	
	MOV R1, #31H
	MOV B, @R1
	
	DIV AB
	
	MOV P1, A
	MOV P2, B
	
	RET
	
	END
