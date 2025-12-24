/*HT điều khiển TB điện 2
1. Hiện trạng thái 3 tảiv(hiển thị băng 3 led đơn): Đèn, quạt, máy lạnh lên Terminal (Mặc định: OFF)
(Cập nhật mới khi có thay đổi)
2. Bật/tắt 3 tải: bằng 3 nút nhấn đơn. (Nhấn giữ nút không quá ~ 200ms)
3. 1 led báo nguồn, chớp tắt chu kỳ 2s (sáng 1s/ tắt 1s)
Thời gian chớp tắt hiển thị Led 7seg (đơn vị ms)
Đổi chu kỳ: bằng cách gửi số ms từ Terminal.*/

#include <REGX52.H>
#include <stdio.h>
#include <string.h>

//Dinh nghia chan I/O
sbit LED1 = P1^0;
sbit LED2 = P1^1; //Den
sbit LED3 = P1^2; //Quat
sbit LED4 = P1^3; //May

//Nut nhan
sbit BTN_A = P3^3;
sbit BTN_B = P3^4;
sbit BTN_C = P3^5;

#define SEGMENT_PORT P0
#define SELECT_PORT  P2

unsigned char digit_patterns[] = {
	0x3F, //0
	0x06, //1
	0x5B, //2
	0x4F, //3
	0x66, //4
	0x6D, //5
	0x7D, //6
	0x07, //7
	0x7F, //8
	0x6F  //9
};
unsigned char seg7_show[8] = {0,0,0,0,0,0,0,0};
unsigned int Speed = 2000;
unsigned int last_blink_time = 0;
bit led_blink_state = 0;
unsigned char state_den = 0;      // 0=OFF, 1=ON
unsigned char state_quat = 0;
unsigned char state_maylanh = 0;

//Cac bien toan cuc
volatile bit flag = 0;           //Co bao co du lieu moi 
volatile unsigned char recvData; //Du lieu nhan duoc

//Them buffer nhan UART
volatile unsigned char uart_buffer[5];
volatile unsigned char uart_index = 0;

volatile unsigned int tick_ms = 0;
//Ham delay
void delay_ms(unsigned int ms){
    unsigned int start = tick_ms;
    while(tick_ms - start < ms);
}
//Cap nhat len man hinh led 7 doan
void Screen_Update(void){
	// hien thi 4 so tuong ung voi ms
	seg7_show[0] = digit_patterns[Speed % 10];
	seg7_show[1] = digit_patterns[(Speed / 10) % 10];
	seg7_show[2] = digit_patterns[(Speed / 100) % 10];
	seg7_show[3] = digit_patterns[Speed / 1000];	
	seg7_show[4] = 0;
  seg7_show[5] = 0;
	seg7_show[6] = 0;
	seg7_show[7] = 0;

}
void scan_7seg(void)
{
    static unsigned char index = 0;

    SEGMENT_PORT = 0x00;        // T?t do?n tru?c khi chuy?n LED

    switch(index)
    {
        case 0: SELECT_PORT = (0 << 2); SEGMENT_PORT = seg7_show[0]; break;
        case 1: SELECT_PORT = (1 << 2); SEGMENT_PORT = seg7_show[1]; break;
        case 2: SELECT_PORT = (2 << 2); SEGMENT_PORT = seg7_show[2]; break;
        case 3: SELECT_PORT = (3 << 2); SEGMENT_PORT = seg7_show[3]; break;
        case 4: SELECT_PORT = (4 << 2); SEGMENT_PORT = seg7_show[4]; break;
        case 5: SELECT_PORT = (5 << 2); SEGMENT_PORT = seg7_show[5]; break;
        case 6: SELECT_PORT = (6 << 2); SEGMENT_PORT = seg7_show[6]; break;
        case 7: SELECT_PORT = (7 << 2); SEGMENT_PORT = seg7_show[7]; break;
    }

    index++;
    if(index >= 8) index = 0;
}

//Khoi tao Timer
void Timer0_Init(void)
{
	TMOD |= 0x01;

	TH0 = 0xFC;
	TL0 = 0x18;
	
	ET0 = 1;
	EA = 1;
	
  TR0 = 1;
}

void Timer0_ISR(void) interrupt 1{
	TH0 = 0xFC;
	TL0 = 0x18;

	tick_ms++;       // tang moi 1ms
	scan_7seg();
}

//Khoi tao Uart voi toc do baud 9600
void UART_Init(void){
	SCON = 0X50;      //Che do 1, 8 bit, REN=1 (cho phep nhan)
	TMOD &= 0x0F;     //Xoa cac bit che do Timer 1 
	TMOD |= 0X20;     //Thiet lap Timer 1 p che do 2
	TH1 = 0xFD;       //9600, SMOD=0
	TR1 = 1;          //Kich hoat Timer 1 
	TI = 1;          //San sang truyen byte dau tien
}

//Ham gui mot ky tu 
void UART_SendChar(char c){
	while (!TI);      //Cho cho den khi co TI=1 (truyen xong)
	TI = 0;           //Xoa co TI
	SBUF = c;         //Gui ky tu 
}
void UART_SendString(char *str){
	while (*str){     //lap cho den khi gap ky tu NULL
		UART_SendChar(*str); //Gui ky tu hien tai 
		str++;          // Chuyen den ky tu tiep theo
	}
}

//Kich hoat ngat UART 
void UART_EnableInterrupt(void){
	EA = 1;           //Cho phep ngat toan cuc
	ES = 1;           //Cho phep ngat UART
} 

void UART_ISR(void) interrupt 4{
	if(RI)            //Neu nhan duoc du lieu
	{
		RI = 0;         
		uart_buffer[uart_index] = SBUF;
		uart_index++;
		if(uart_index == 5){
			uart_index = 0;
			flag = 1;
		}      
	}		
}

void Show(void){
	//Gui loi chao va menu huong dan
	UART_SendString("------Dieu khien------\r\n");
	UART_SendString("P3.3 DEN :     OFF\r\n");
	UART_SendString("P3.4 QUAT:     OFF\r\n");
	UART_SendString("P3.5 MAY LANH: OFF\r\n");
}

//Xu ly nut nhan 
void Handle_Button(void){
	if(BTN_A == 0){
		while(BTN_A == 0); 
		state_den = !state_den;
		if(state_den){
			LED2 = 0;
			UART_SendString("P3.3 DEN: ON\r\n");
		}
		else{
			UART_SendString("P3.3 DEN: OFF\r\n");
			LED2 = 1;
		}
	}
	if(BTN_B == 0){
		while(BTN_B == 0); 
		state_quat = !state_quat;
		if(state_quat){
			LED3 = 0;
			UART_SendString("P3.4 QUAT: ON\r\n");
		}
		else{
			LED3 = 1;
			UART_SendString("P3.4 QUAT: OFF\r\n");
		}
	}
	if(BTN_C == 0){
		while(BTN_C == 0); 
		state_maylanh = !state_maylanh;
		if(state_maylanh){
			LED4 = 0;
			UART_SendString("P3.4 MAY LANH: ON\r\n");
		}
		else{
			LED4 = 1;
			UART_SendString("P3.4 MAY LANH: OFF\r\n");
		}
	}
}

void Blink_Task(void)
{
    if(tick_ms - last_blink_time >= (Speed / 2))
    {
        last_blink_time = tick_ms;
        led_blink_state = !led_blink_state;
        LED1 = !led_blink_state;   // LED active LOW
    }
}

void Handle_Uart(void){
    unsigned char thou, hund, tens, ones;
    unsigned int new_value;
    
    // Kiem tra dinh dang
    if(uart_buffer[0] < '0' || uart_buffer[0] > '9' ||
       uart_buffer[1] < '0' || uart_buffer[1] > '9' ||
       uart_buffer[2] < '0' || uart_buffer[2] > '9' ||
		   uart_buffer[3] < '0' || uart_buffer[3] > '9'){
        UART_SendString("ERROR: Format XYZM (X,Y,Z,M=0-9,)\r\n");
        return;
    }
    thou = uart_buffer[0] - '0';
    hund = uart_buffer[1] - '0';
    tens = uart_buffer[2] - '0';
    ones = uart_buffer[3] - '0';
		
		new_value = thou * 1000 + hund * 100 + tens * 10 + ones; //ghep thanh so nguyen
		
		//gia su kiem tra pham vi
    if(new_value >= 0 && new_value <= 9999){
        
        // Gui phan hoi
        char msg[30];
        sprintf(msg, "OK: Value = %u\r\n", new_value);
        UART_SendString(msg);
    }
    else{
        UART_SendString("ERROR: Out of range\r\n");
    }
		if(new_value <= 9999){
				Speed = new_value;   // ? QUAN TR?NG
		}
}


void main(void){
	LED1 = LED2 = LED3 = LED4 = 1; //Tat LED
	Timer0_Init();
	UART_Init();                   //Khoi tao UART
	UART_EnableInterrupt();        //Kich hoat Interrupt
	
	Show();	
	while(1)
	{
		Screen_Update();
		Blink_Task();
		Handle_Button();
		if(flag == 1)
		{
			flag = 0;
			Handle_Uart();
		}
	}
}