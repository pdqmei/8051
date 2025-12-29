;Viết chương trình dùng ngắt ngoài INT0 để tăng biến đếm và hiển thị trên Port P2

ORG 0000H
JMP MAIN

ORG 0003H
JMP BTN_INC

ORG 0030H
MAIN:
SETB IT0
SETB EX0
SETB EA

MOV P2, #0
MOV R7, #0

LOOP:
MOV P2, R7
JMP LOOP

BTN_INC:
INC R7
RETI

END
