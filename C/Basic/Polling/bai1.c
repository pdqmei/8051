#include <REGX51.H>

#define SEGMENT_PORT P0
#define SELECT_PORT P2

#define SO_LED_7_HIEN_THI 8

#define LED_PORT P1

// Input
sbit BTN_INC = P3^2;
sbit BTN_DEC = P3^3;

sbit BTN_DEN = P3^4;
sbit BTN_QUAT = P3^5;
sbit BTN_AC = P3^6;

// Output
sbit LED_DEN = P1^0;
sbit LED_QUAT = P1^1;
sbit LED_AC = P1^2;

// LED state (1: OFF,0:ON)
bit DEN_STATE = 1;
bit QUAT_STATE = 1;
bit AC_STATE = 1;

// Tracking T
sbit LED_STATE = P1^7;

bit CHECK_STATE = 0;

unsigned int cycle_t = 0; 

bit flag_t = 0;

unsigned int cnt50_ms = 0; 

unsigned int T = 10;

// 7_SEG
unsigned char digit_patterns[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 
    0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00
};

unsigned char gia_tri_hien_thi[SO_LED_7_HIEN_THI] = {
	10, 10, 10, 10,
	10, 10, 10 ,10
};

unsigned int digit1, digit2, digit3, digit4;

unsigned char vi_tri_hien_thi = 0;

void hien_thi_7_doan(unsigned char vi_tri, unsigned char so);

void cap_nhat_gia_tri(void);

// Private function

void btn_polling_init(void);

void btn_polling_handle(void);

void isr_init(void);

void led_handle(void);

void cap_nhat_7_doan(void);

void main(void)
{
	LED_PORT = 0xFF;
	
	btn_polling_init();
	
	isr_init();
	
	while(1)
	{
		btn_polling_handle();
		if (CHECK_STATE)
		{
			CHECK_STATE = 0;
			LED_DEN = DEN_STATE;
			LED_QUAT = QUAT_STATE;
			LED_AC = AC_STATE;
		}
		if (flag_t)
		{
			flag_t = 0;
			led_handle();
		}
	}
}

void led_handle(void)
{
	LED_STATE = !LED_STATE;
	
	// ...
}

void btn_polling_init(void)
{
	BTN_DEN = 1;
	BTN_QUAT = 1;
	BTN_AC = 1;
}

void btn_polling_handle(void)
{
	if (BTN_DEN == 0)
	{
		CHECK_STATE = 1;
		DEN_STATE = !DEN_STATE;
		while(BTN_DEN == 0);
	}
	
	if (BTN_QUAT == 0)
	{
		CHECK_STATE = 1;
		QUAT_STATE = !QUAT_STATE;
		while(BTN_QUAT == 0);
	}
	
	if (BTN_AC == 0)
	{
		CHECK_STATE = 1;
		AC_STATE = !AC_STATE;
		while(BTN_AC == 0);
	}
}

void isr_init(void)
{
	// Init button0_isr
	EX0 = 1;
	IT0 = 1;
	BTN_INC = 1;
	
	// Init button1_isr
	EX1 = 1;
	IT1 = 1;
	BTN_DEC = 1;
	
	// Init timer_isr
	TMOD = 0x01;
	
	//2ms
	TH0 = 0xF8;
	TL0 = 0x30;
	ET0 = 1;
	

	// Enable All 
	EA = 1;
	
	// Start timer
	TR0 = 1;
}

void hien_thi_7_doan(unsigned char vi_tri, unsigned char so)
{
    SELECT_PORT = (SELECT_PORT & 0xE3) | (vi_tri << 2);
    
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

void cap_nhat_7_doan(void)
{
	hien_thi_7_doan(vi_tri_hien_thi, gia_tri_hien_thi[vi_tri_hien_thi]);
	
	vi_tri_hien_thi++;
	
	if (vi_tri_hien_thi >= SO_LED_7_HIEN_THI)
	{
		vi_tri_hien_thi = 0;
	}
}

void timer0_isr(void) interrupt 1
{
	TH0 = 0xF8;
	TL0 = 0x30;
	
	cap_nhat_gia_tri();
	
	cap_nhat_7_doan();
	
	cnt50_ms++;
	
	if (cnt50_ms > 25) // 50ms
	{
		cnt50_ms = 0;
		cycle_t ++;
		
		if (cycle_t > T)
		{
			cycle_t = 0;
			flag_t = 1;
		}
	}
}

void button0_isr(void) interrupt 0
{
	T += 5;
	if (T >= 30)
	{
		T = 30;
	}
}

void button1_isr(void) interrupt 2
{
	T -= 5;
	if (T <= 5)
	{
		T = 5;
	}
}