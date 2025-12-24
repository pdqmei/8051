/*Thiết kế hệ thống điều khiển 2 LED với 3 chế độ hoạt động, 
hiển thị thông số trên LED 7 đoạn và giao tiếp UART để cập nhật thông số từ máy tính.*/
#include <REGX52.H>
#include <stdio.h>
#include <string.h>

#define SEGMENT_PORT P0
#define SELECT_PORT  P2

#define SO_LED_7_HIEN_THI 4

//inputs
sbit BTN_MODE = P3^2;
sbit BTN_LED = P3^3;

//outputs
sbit LED1 = P1^0;
sbit LED2 = P1^1;

//choose led
unsigned char select = 0;
unsigned char mode = 0;

//Tracking T
unsigned int cnt50_ms = 0;
unsigned int cycle_t = 0; //cycle of led on/off
unsigned int T =0; //if you want to delay T ms, just change T in function

//string for Uart
volatile bit flag = 0;            
//volatile unsigned char recvData; 

//buffer uart, example: if enter 4 numbers of time 
volatile unsigned char uart_buffer[5];
volatile unsigned char uart_index = 0;

//flag for check some functions
bit flag_led = 0;
bit flag_update_display = 0;
bit uart_control = 0;

// 7_SEG
unsigned char digit_patterns[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 
    0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00
};

unsigned char gia_tri_hien_thi[SO_LED_7_HIEN_THI] = {
	10, 10, 10, 10,
};

unsigned int digit1, digit2, digit3, digit4; 
unsigned char vi_tri_hien_thi = 0;

// Private function
void hien_thi_7_doan(unsigned char vi_tri, unsigned char so);
void cap_nhat_gia_tri(void);
void cap_nhat_7_doan(void);
void ISR_Init(void);
void UART_EnableInterrupt(void);
void UART_Init(void);
void UART_SendChar(char c);
void UART_SendString(char *str);
void mode_handle(void);
void led_handle(void);
void uart_handle(void);
	
//main
void main()
{
	cap_nhat_gia_tri();
	LED1 = LED2 = 1;
	ISR_Init();
	UART_EnableInterrupt();
	UART_Init();
	while(1)
	{
		mode_handle();
		
		if(flag_led)
		{
			flag_led = 0 ;
			led_handle();
		}
		if(flag)
		{	
			flag = 0;
			uart_handle();
		}
		
		if(flag_update_display)
		{
			flag_update_display = 0;
			cap_nhat_gia_tri();
		}
	}
}

void ISR_Init(void)
{
	//button 0 init
	EX0 = 1;
	IT0 = 1;
	BTN_MODE = 1;
	
	//button 1 init
	EX1 = 1;
	IT1 = 1;
	BTN_LED = 1;
	
	//timer0 init
	TMOD = 0x01;
	
	//2ms
	TH0 = 0xF8;
	TL0 = 0x30;
	ET0 = 1;
	
	//enable all
	EA = 1;
	
	//start timer
	TR0 = 1;
}

//set LED1 and interrupt dont need Private function prototypes 
void ISR_EX0(void)interrupt 0
{
	select++;
	if (select > 2)select = 0;
	
	//when change led, return mode 0
	mode = 0;
	cycle_t = 0;
	flag_update_display = 1;
}

void ISR_EX1(void)interrupt 2
{
	if(select > 0){
		mode++;
		if (mode > 2) mode = 0;
		cycle_t = 0; //reset cycle
		flag_update_display = 1;
	}
}

//timer0 interrput
void ISR_Timer0(void) interrupt 1
{
	TH0 = 0xF8;
	TL0 = 0x30;

	cap_nhat_7_doan();
	
	cnt50_ms++;
	if(cnt50_ms > 25)
	{
		cnt50_ms = 0;
		cycle_t++;
		if(cycle_t >= T)
		{
			flag_led = 1;
			cycle_t =0;
		}
	}
}

void mode_handle(void)
{
	if (select == 0) 
	{
		LED1 = LED2 = 1; 
		T = 0;
		return;
	}
	//Neu dang dùng UART control thì không ghi dè T
  if(uart_control)
  {
    return;
  }
	
	switch(mode)
	{
		case 0:
			T = 0;
			if (select == 1) {
				LED1 = 0;
				LED2 = 1;
			}
			else if (select == 2) {
				LED1 = 1;
				LED2 = 0;
			}
			break;
		case 1:
			if (select == 1) 
			{
				T =10;
				flag_update_display = 1;
				LED2 = 1;
			}	
			else if (select == 2)
			{
				T = 5;
				flag_update_display = 1;
				LED1 = 1;
			}	
			break;
		case 2:
			if (select == 1) 
			{
				T = 5;
				flag_update_display = 1;
				LED2 = 1;
			}	
			else if (select == 2)
			{
				T = 10;
				flag_update_display = 1;
				LED1 = 1;
			}	
			break;
	}	
}

//led handle when select mode 
void led_handle(void)
{
	if (mode == 0) return;
	
	if (select == 1)
		LED1 = !LED1;
	else if (select == 2)
		LED2 = !LED2;	
}

void hien_thi_7_doan(unsigned char vi_tri, unsigned char so)
{
	SELECT_PORT = (SELECT_PORT & 0XE3) | ( vi_tri << 2);
	
	SEGMENT_PORT = digit_patterns[so];
}

void cap_nhat_gia_tri(void)
{ 
	unsigned int tmp = T * 100;
	
	digit1 = (tmp / 1000) % 10;
	digit2 = (tmp / 100) % 10;
	digit3 = (tmp / 10) % 10;
	digit4 = (tmp / 1) % 10;
	
	gia_tri_hien_thi[3] = digit1;
	gia_tri_hien_thi[2] = digit2;
	gia_tri_hien_thi[1] = digit3;
	gia_tri_hien_thi[0] = digit4;
}

//quet led
void cap_nhat_7_doan(void)
{
	hien_thi_7_doan(vi_tri_hien_thi, gia_tri_hien_thi[vi_tri_hien_thi]);
	vi_tri_hien_thi++;
	
	if (vi_tri_hien_thi >= SO_LED_7_HIEN_THI)
	{
		vi_tri_hien_thi = 0;
	}
}
void UART_EnableInterrupt(void)
{
	EA = 1;
	ES = 1;
}
void UART_Init(void)
{
	SCON = 0x50;
	TMOD &= 0x0F;
	TMOD |= 0x20; 
	TH1 = 0xFD;
	TR1 = 1;
	TI = 1; 
}

void UART_SendChar(char c)
{
	SBUF = c;
	while(!TI);
	TI = 0;
}

void UART_SendString(char *str)
{
	while (*str)
	{
		UART_SendChar(*str);
		str++;
	}
}

void UART_ISR(void) interrupt 4
{
	if(RI)
	{	
		RI = 0;
		uart_buffer[uart_index] = SBUF;
		uart_index++;
		if(uart_index >= 4)
		{
			uart_buffer[4] = '\0';
			uart_index = 0;
			flag = 1;
		}
	}
	if (TI)
	{
		TI = 0;   // NOTE: clear TI 
	}
}

void uart_handle(void)
{
	unsigned char thou, hund, tens, ones;
	unsigned int new_value;
			
	// Kiem tra dinh dang
	if(uart_buffer[0] < '0' || uart_buffer[0] > '9' ||
		uart_buffer[1] < '0' || uart_buffer[1] > '9' ||
		uart_buffer[2] < '0' || uart_buffer[2] > '9' ||
		uart_buffer[3] < '0' || uart_buffer[3] > '9')
	{
		UART_SendString("ERROR: Format XYZM (X,Y,Z,M=0-9,)\r\n");
		return;
	}
	thou = uart_buffer[0] - '0';
	hund = uart_buffer[1] - '0';
	tens = uart_buffer[2] - '0';
	ones = uart_buffer[3] - '0';

	new_value = thou * 1000 + hund * 100 + tens * 10 + ones; //ghep thanh so nguyen
			
	//gia su kiem tra pham vi
	if(new_value >= 0 && new_value <= 9999)
	{					
		// Gui phan hoi
    char msg[30];
    sprintf(msg, "OK: Value = %u\r\n", new_value);
    UART_SendString(msg);
  }
  else{
    UART_SendString("ERROR: Out of range\r\n");
  }
	if(new_value <= 9999)
		T = new_value / 100;  
	uart_control = 1;
	flag_update_display = 1;
    else
    {
        UART_SendString("ERROR: Out of range\r\n");
    }
}
