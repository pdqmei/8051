/* Có làm thì gửi với á nha 
1. Dùng 4 nút nhấn → Bật/tắt 3 led đơn
a. 2 nút để đảo trạng thái 2 led.
b. 2 nút (1 nút bật, 1 nút tắt) → 1 led còn lại.
2. Gửi mã "Onl, Offl,..." (6 loại mã) từ Terminal → Bật/tắt 3 led đơn ở trên.
3. Hiện 4 số 0000 → 3111 lên led7seg[7:4]
a. Số đầu tiên = số led đang sáng.
b. 3 số sau = tương ứng trạng thái 3 led đơn.
4. Gửi mã từ Terminal
a. TO: Tắt hết các led. Và không cho điều khiển 3 led đơn.
b. T1: Cho điều khiến 3 led đơn.
c. Tx: led nào được bật, thì sáng trong xms rồi sẽ tự tắt*/

#include <REGX51.H>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ========== LED BEGIN ========== */

#define LED_PORT P1

sbit led_1 = P1^0;
sbit led_2 = P1^1;
sbit led_3 = P1^2;

/* ========== LED END ============ */

/* ========== BUTTON BEGIN ========== */

sbit btn_a = P3^4;
sbit btn_b = P3^5;
sbit btn_c = P3^6;
sbit btn_d = P3^7;

bit flag_btn_a = 0;
bit flag_btn_b = 0;
bit flag_btn_c = 0;
bit flag_btn_d = 0;

void btn_polling_init(void);
void btn_polling_handle(void);

void btn_a_handler(void);
void btn_b_handler(void);
void btn_c_handler(void);
void btn_d_handler(void);

/* ========== BUTTON END ========== */

/* ========== LED 7 SEG BEGIN ========== */

#define SEGMENT_PORT P0
#define SELECT_PORT P2

#define SO_LED_7_HIEN_THI 8

unsigned char digit_patterns[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 
    0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00
};

unsigned char gia_tri_hien_thi[SO_LED_7_HIEN_THI] = {
	10, 10, 10, 10,
	10, 10, 10, 10
};

unsigned int digit0;
unsigned char vi_tri_hien_thi = 0;

void hien_thi_7_doan(unsigned char vi_tri, unsigned char so);
void cap_nhat_gia_tri(void);
void cap_nhat_7_doan(void);

/* ========== LED 7 SEG END ========== */

/* ========== UART BEGIN ========== */

#define MAX_BUFFER 16

volatile unsigned char recvData;

volatile unsigned char buffer_index = 0;

volatile char string_buffer[MAX_BUFFER];

void UART_Init(void);

void UART_EnableInterrupt(void);

void UART_SendChar(char c);

void UART_SendString(char *str);

char UART_GetChar(void);

void UART_GetString(char *buffer, unsigned int max_length, char end_char);

volatile bit flag_rx;

void uart_handler_rx(void);

void uart_handler_tx(void);

/* ========== UART END ========== */

/* ======== CONTROL BEGIN ======= */

bit flag_ctrl = 0;

bit flag_retain = 0;

bit flag_off_led = 0;

unsigned int time_ms;

unsigned int count_ms;

/* ======== CONTROL END ========= */

void isr_init(void);

unsigned char dem_bit_0(unsigned char byte_value);

/* ========== MAIN =================== */

void main(void)
{
	LED_PORT = 0xFF;
	
	btn_polling_init();
	
	isr_init();
	
	UART_Init();
	UART_EnableInterrupt();
	
	
	while (1)
	{
		
		if (flag_ctrl)
		{
			LED_PORT = 0xFF;
		}
		else
		{
			btn_polling_handle();
		
			if(flag_btn_a)
			{
				flag_btn_a = 0;
				btn_a_handler();
				uart_handler_tx();
			}
		
			if(flag_btn_b)
			{
				flag_btn_b = 0;
				btn_b_handler();
				uart_handler_tx();
			}
		
			if(flag_btn_c)
			{
				flag_btn_c = 0;
				btn_c_handler();
				uart_handler_tx();
			}
		
			if(flag_btn_d)
			{
				flag_btn_d = 0;
				btn_d_handler();
				uart_handler_tx();
			}
			
			if(flag_off_led)
			{
				flag_off_led = 0;
				LED_PORT = 0xFF;
				
				// Send Menu
				uart_handler_tx();
			}
		}
		
		// UART Rx Handler
		if (flag_rx)
		{
			flag_rx = 0;
			uart_handler_rx();
			if (flag_ctrl == 0)
			{
				if(flag_off_led)
				{
					// ...
				}
				else
				{
					uart_handler_tx();
				}
			}
			else
			{
				UART_SendString("Blocking\r\n");
			}
		}
	}
}

/* ============= INIT ================ */

void isr_init(void)
{
	// Init timer0_isr
	TMOD = 0x01;
	
	// 2ms
	TH0 = 0xF8;
	TL0 = 0x30;
	ET0 = 1;
	
	// Enable All 
	EA = 1;
	
	// Start timer
	TR0 = 1;
}

void timer0_isr(void) interrupt 1
{
	// 2ms
	TH0 = 0xF8;
	TL0 = 0x30;
	
	cap_nhat_gia_tri();
	cap_nhat_7_doan();
	
	if (flag_retain)
	{
		count_ms++;
	
		if (count_ms >= (time_ms/2))
		{
			count_ms = 0;
			
			flag_retain = 0;
			flag_off_led = 1;
		}
	}

}

/* ========== BUTTON FUNCTIONS ========== */

void btn_polling_init(void)
{
	btn_a = 1;
	btn_b = 1;
	btn_c = 1;
	btn_d = 1;
}

void btn_polling_handle(void)
{
	if (btn_a == 0)
	{
		flag_btn_a = 1;
		while(btn_a == 0);
	}

	if (btn_b == 0)
	{
		flag_btn_b = 1;
		while(btn_b == 0);
	}

	if (btn_c == 0)
	{
		flag_btn_c = 1;
		while(btn_c == 0);
	}

	if (btn_d == 0)
	{
		flag_btn_d = 1;
		while(btn_d == 0);
	}
}

void btn_a_handler(void)
{
	led_1 = !led_1;
}

void btn_b_handler(void)
{
	led_2 = !led_2;
}

void btn_c_handler(void)
{
	led_3 = 0;
}

void btn_d_handler(void)
{
	led_3 = 1;
}

/* ========== LED 7 SEGMENT FUNCTIONS ========== */

void hien_thi_7_doan(unsigned char vi_tri, unsigned char so)
{
	SELECT_PORT = (SELECT_PORT & 0xE3) | (vi_tri << 2);
	SEGMENT_PORT = digit_patterns[so];
}

void cap_nhat_gia_tri(void)
{
	digit0 = dem_bit_0(LED_PORT); 
	
	gia_tri_hien_thi[7] = digit0;
	gia_tri_hien_thi[6] = !led_1;
	gia_tri_hien_thi[5] = !led_2;
	gia_tri_hien_thi[4] = !led_3;
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

/* ========== UART FUNCTIONS ========== */

void uart_handler_tx(void)
{
	char tmp_buffer[20];
	UART_SendString("==== Menu ====\r\n");
	
	// Led 0
	sprintf(tmp_buffer, "Led 1: %s\r\n", led_1 == 0 ? "on" : "off");
	UART_SendString(tmp_buffer);
	
	// Led 1
	sprintf(tmp_buffer, "Led 2: %s\r\n", led_2 == 0 ? "on" : "off");
	UART_SendString(tmp_buffer);
	
	// Led 2
	sprintf(tmp_buffer, "Led 3: %s\r\n", led_3 == 0 ? "on" : "off");
	UART_SendString(tmp_buffer);
}

void uart_handler_rx(void)
{
	// Next buffer
	if (strlen(string_buffer) == 0)
	{
		return;
	}
	
	if (string_buffer[0] == 'T' && strlen(string_buffer) > 2 && isdigit(string_buffer[1]))
	{
		time_ms = atoi(string_buffer + 1);
		flag_retain = 1;
	}
	else if(strcmp(string_buffer, "T1") == 0)
	{
		flag_ctrl = 0;
	}
	
	if (strcmp(string_buffer, "T0") == 0)
	{
		flag_ctrl = 1;
	}
	
	// Control Led
	if (flag_ctrl == 0)
	{
			if (strcmp(string_buffer, "On1") == 0)
		{
			led_1 = 0;
		}
		if (strcmp(string_buffer, "Off1") == 0)
		{
			led_1 = 1;
		}
	
		if (strcmp(string_buffer, "On2") == 0)
		{
			led_2 = 0;
		}
		if (strcmp(string_buffer, "Off2") == 0)
		{
			led_2 = 1;
		}
	
		if (strcmp(string_buffer, "On3") == 0)
		{
			led_3 = 0;
		}
		if (strcmp(string_buffer, "Off3") == 0)
		{
			led_3 = 1;
		}
	}
}

void UART_ISR(void) interrupt 4
{
	if (RI)
	{
		RI = 0;
		recvData = SBUF;
        
		if (recvData == '\n' || recvData == '\r')
		{
			string_buffer[buffer_index] = '\0';
			flag_rx = 1;
			buffer_index = 0;
		}
		else if (buffer_index < MAX_BUFFER - 1)
		{
			string_buffer[buffer_index++] = recvData;
		}
		else
		{
			buffer_index = 0;
			string_buffer[0] = '\0';
		}
	}
}

void UART_Init(void)
{
	SCON = 0x50;
    
	TMOD &= 0x0F;
	TMOD |= 0x20;
    
	TH1 = 0xFD;
	TL1 = 0xFD;
	TR1 = 1;
    
	TI = 1;
    
	flag_rx = 0;
}

void UART_EnableInterrupt(void)
{
	ES = 1;
	EA = 1;
}

void UART_SendChar(char c)
{
	while (!TI);
	TI = 0;
	SBUF = c;
}

void UART_SendString(char *str)
{
	while (*str)
	{
		UART_SendChar(*str);
		str++;
	}
}

char UART_GetChar(void)
{
	while (!RI);
	RI = 0;
	return SBUF;
}

void UART_GetString(char *buffer, unsigned int max_length, char end_char)
{
	unsigned int i = 0;
	char c;
    
	do
	{
		c = UART_GetChar();
		if (c == end_char) 
		{
			buffer[i] = '\0';
			break;
		} 
		else 
		{
			buffer[i++] = c;
		}
	} while (i < max_length - 1);
    
	if (i == max_length - 1) 
	{
		buffer[i] = '\0';
	}
}

/* ========== HELPER FUNCTIONS ========== */

unsigned char dem_bit_0(unsigned char byte_value)
{
	unsigned char count = 0;
    
	//byte_value &= 0xE3;  // 0xE3 = 1110 0011
	
	while (byte_value)
	{
		byte_value &= (byte_value - 1);  
		count++;
	}
    
	return (8 - count);
}