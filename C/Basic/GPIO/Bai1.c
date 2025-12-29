//Điều chỉnh độ sáng của led ở pin P2.2 sử dụng PWM 
#include <REGX51.H>
#include <intrins.h>

sbit LED = P2 ^ 2;

void delay_us(unsigned int us){
    while (us--){
        _nop_();
    }
}

void main(){
    unsigned char i, duty_cycle;
    while (1)
    {
        for (duty_cycle = 0; duty_cycle <= 100; duty_cycle++)
        {
            for (i = 0; i < 100; i++)
            {
                if (i < duty_cycle)
                    LED = 0; // Turn LED on
                else
                    LED = 1;  // Turn LED off
                delay_us(10); // Total period of 1ms
            }
        }
        for (duty_cycle = 100; duty_cycle > 0; duty_cycle--)
        {
            for (i = 0; i < 100; i++)
            {
                if (i < duty_cycle)
                    LED = 0; // Turn LED on
                else
                    LED = 1;  // Turn LED off
                delay_us(10); // Total period of 1ms
            }
        }
    }
}
