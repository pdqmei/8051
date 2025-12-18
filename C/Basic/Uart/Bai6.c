/*Đề bài: Điều khiển 2 led đơn chớp tắt với tốc độ tùy chỉnh
- Cho led 1 chớp tắt với chu kỳ 2S (On 1s, Off 1s)
- Cho led 2 chớp tắt với chu kỳ 1S
- Thay đổi chu kỳ bằng 2 nút nhấn (1 nút tăng, 1 nút giảm). Mỗi lần tăng/giảm 0.5s. Chu kỳ thuộc đoạn [1: 5] (s).
- Thay đổi chu kỳ từng Led bằng cách gửi từ Terminal. Cấu trúc chuỗi (tùy bạn quy ước.) 
(VD: gửi “105” tức Led 1, chu kỳ 0.5S)
- Led7seg hiện giá trị chu kì của 2 led đơn.
(*)Lưu ý:
- Chạy được 100% ý nào thì chấm ý đó.
- Cho phép chọn chân tùy ý (phù hợp với đề bài)*/
#include <REGX52.H>
#include <stdio.h>
#include <string.h>

//Dinh ngia cac chan I/O
sbit LED0 = P1^0;
sbit LED1 = P1^1;

//cac bien toan cuc 
volatile bit flag = 0;             //Co bao co du lieu moi
volatile unsigned char recvData;   //Du lieu nhan duoc
volatile unsigned int tick_ms = 0; //Tao tick 1ms
volatile unsigned char value[2] = {20, 10}; // LED0 = 2s, LED1 = 1s
volatile unsigned char led = 0; // 0 = LED0, 1 = LED1

//Them buffer nhan UART
volatile unsigned char uart_buffer[4];
volatile unsigned char uart_index = 0;

#define SEGMENT_PORT P0 //Port dieu khien cac doan LED
#define SELECT_PORT  P2 //Port chon led

//Bang ma LED 7 doan (cathode chung)
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

//dinh nghia chu ki la so thap phan duoc hien thi bang 2 led 
// digit[led][vitri]
// led = 0 (LED0), 1 (LED1)
// vitri = 0 (trai), 1 (phai)
unsigned char digit[2][2];

void float_to_7seg(unsigned char led){
    digit[led][0] = value[led] / 10;  // phan nguyen
    digit[led][1] = value[led] % 10;  // phan thap phan
}

//Ham delay
void delay_ms(unsigned int ms){
    unsigned int start = tick_ms;
    while(tick_ms - start < ms);
}

//Quet led 7 doan 7,8 cho chu ki LED0 1,2 cho chu ki LED1
void scan_7seg(){
    static unsigned char current_led = 0;
    
    // T?t LED tru?c khi chuy?n
    SEGMENT_PORT = 0x00;
    
    switch(current_led){
        case 0:
            SELECT_PORT = (7 << 2);
            SEGMENT_PORT = digit_patterns[digit[0][0]] | 0x80;
            break;
        case 1:
            SELECT_PORT = (6 << 2);
            SEGMENT_PORT = digit_patterns[digit[0][1]];
            break;
        case 2:
            SELECT_PORT = (1 << 2);
            SEGMENT_PORT = digit_patterns[digit[1][0]] | 0x80;
            break;
        case 3:
            SELECT_PORT = (0 << 2);
            SEGMENT_PORT = digit_patterns[digit[1][1]];
            break;
    }
    
    current_led++;
    if(current_led >= 4) current_led = 0;
}

//Khoi tao UART 
void UART_Init(void){
	SCON = 0x50;   //Mode 1, 8bit, REN = 1 (cho phep nhan)
	TMOD &= 0x0F;  //Xoa cac bit che do timer 1
	TMOD |= 0x20;  //Thiet lap timer 1 o che do 2 
	TH1 = 0xFD;    //Toc do baud 9600. Smod = 0
	TR1 = 1;       //Kich hoat Timer 1
	TI = 1;        //San sang de truyen byte dau tien
}

//Ham gui mot ky tu 
void UART_SendChar(char c){
	while(!TI);    //Cho cho den khi co TI = 1 (truyen xong)
	TI = 0;        //Xoa co TI
	SBUF = c;      //Gui ky tu
}

//Ham gui mot chuoi
void UART_SendString(char *str){
	while(*str){   //Lap cho den khi gap ki tu null
		UART_SendChar(*str); //Gui ki tu hien tai
		str++;               //Gui ki tu tiep theo
	}
}

//Kich hoat ngat UART
void UART_EnableInterrupt(void){
	EA = 1;        //Cho phep ngat toan cuc
	ES = 1;        //Cho phep ngat UART
}	

//Ham xu ly ngat UART 
void UART_ISR(void) interrupt 4{
	if(RI){
		RI = 0;
		uart_buffer[uart_index++] = SBUF;
							 
		if(uart_index == 3){  // �u 3 k� tu
			uart_index = 0;
			flag = 1;
		}
	}	
}

unsigned int period0 = 2000;
unsigned int period1 = 1000;
bit led0_state = 0, led1_state = 0;
unsigned int t0 = 0, t1 = 0;
//Cap nhat period tu value
void Update_Period(void){
    period0 = value[0] * 100;   // value=10 ? 1000ms
    period1 = value[1] * 100;  
	
		t0 = tick_ms;
    t1 = tick_ms;
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
	scan_7seg();     // qu�t LED
}

void External0_ISR(void) interrupt 0{
    // Tang Ca 2 LED
    if(value[0] < 50){
        value[0] += 5;
        float_to_7seg(0);
    }
    if(value[1] < 50){
        value[1] += 5;
        float_to_7seg(1);
    }
    Update_Period();
}

void External1_ISR(void) interrupt 2{
    // Giam Ca 2 LED
    if(value[0] > 10){
        value[0] -= 5;
        float_to_7seg(0);
    }
    if(value[1] > 10){
        value[1] -= 5;
        float_to_7seg(1);
    }
    Update_Period();
}

void External_Init(void){
	IT0 = 1;   // Kich hoat ngat canh xuong
	IT1 = 1;
  EX0 = 1;   // Cho phep ngat ngoai 0
	EX1 = 1;
  EA  = 1;   // Cho phep ngat toan cuc
}

//Blink khong bi block de tang giam
void Blink_Task(void){
  if(tick_ms - t0 >= period0){
		t0 = tick_ms;
		led0_state = !led0_state;
    LED0 = led0_state;
    }

  if(tick_ms - t1 >= period1){
    t1 = tick_ms;
    led1_state = !led1_state;
    LED1 = led1_state;
	}
}
//Ham xu li UART
void Handle_UART(void){
    unsigned char led_num;
    unsigned char tens, ones;
    unsigned char new_value;
    
    // Kiem tra dinh dang
    if(uart_buffer[0] < '1' || uart_buffer[0] > '2' ||
       uart_buffer[1] < '0' || uart_buffer[1] > '9' ||
       uart_buffer[2] < '0' || uart_buffer[2] > '9'){
        UART_SendString("ERROR: Format XYZ (X=1-2, YZ=10-50)\r\n");
        return;
    }
    
    led_num = uart_buffer[0] - '1';
    tens = uart_buffer[1] - '0';
    ones = uart_buffer[2] - '0';
    new_value = tens * 10 + ones;
    
    // Kiem tra gioi han 1.0s - 5.0s
    if(new_value >= 10 && new_value <= 50){
        value[led_num] = new_value;
        float_to_7seg(led_num);
        Update_Period();
        
        // Gui phan hoi
        UART_SendString("OK: LED");
        UART_SendChar(led_num + '1');
        UART_SendString(" = ");
        UART_SendChar(tens + '0');
        UART_SendChar('.');
        UART_SendChar(ones + '0');
        UART_SendString("s\r\n");
    }
    else{
        UART_SendString("ERROR: Period must be 1.0s-5.0s\r\n");
    }
}

void main(void){
	External_Init();
    UART_Init();
	UART_EnableInterrupt();
	Timer0_Init();
	
	float_to_7seg(0);  // Khoi tao digit cho LED0
	float_to_7seg(1);  // Khoi tao digit cho LED1
	while(1){
		Blink_Task();
		if(flag == 1){
			flag = 0;
			Handle_UART();
		}
	}
}
