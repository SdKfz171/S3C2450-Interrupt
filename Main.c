/***************************************************************
*
*   1. System Init Test 
* 
*   Created by MDS Tech. NT Div.(2Gr) (2015.10.30)
*
****************************************************************
*/
#include <stdio.h>
#include "option.h"
#include "2450addr.h"

typedef struct {
    unsigned char GPIO_PIN_0    : 1;
    unsigned char GPIO_PIN_1    : 1;
    unsigned char GPIO_PIN_2    : 1;
    unsigned char GPIO_PIN_3    : 1;
    unsigned char LED           : 4;
//     unsigned char GPIO_PIN_4    : 1;
//     unsigned char GPIO_PIN_5    : 1;
//     unsigned char GPIO_PIN_6    : 1;
//     unsigned char GPIO_PIN_7    : 1;
    unsigned char GPIO_PIN_8    : 1;
    unsigned char GPIO_PIN_9    : 1;
    unsigned char GPIO_PIN_10   : 1;
    unsigned char GPIO_PIN_11   : 1;
    unsigned char GPIO_PIN_12   : 1;
    unsigned char GPIO_PIN_13   : 1;
    unsigned char GPIO_PIN_14   : 1;
    unsigned char GPIO_PIN_15   : 1;
} GPIOG; 

typedef struct {
    unsigned char GPIO_PIN_0    : 1;
    unsigned char GPIO_PIN_1    : 1;
    unsigned char GPIO_PIN_2    : 1;
    unsigned char GPIO_PIN_3    : 1;
    unsigned char GPIO_PIN_4    : 1;
    unsigned char GPIO_PIN_5    : 1;
    unsigned char GPIO_PIN_6    : 1;
    unsigned char GPIO_PIN_7    : 1;
    unsigned char res           : 8;
} GPIOF;

enum IO_MODE {
    INPUT = (0x0),
    OUTPUT = (0x1),
    EINT = (0x2),
    RESERVED = (0x3)
};

enum EXTI_MODE {
    LOW_LEVEL = (0x0),
    HIGH_LEVEL = (0x1),
    FALLING_EDGE = (0x2),
    RISING_EDGE = (0x4),
    BOTH_EDGE = (0x6)
};

typedef struct {
    unsigned char GPIO_PIN_0   : 2;
    unsigned char GPIO_PIN_1   : 2;
    unsigned char GPIO_PIN_2   : 2;
    unsigned char GPIO_PIN_3   : 2;
    unsigned char GPIO_PIN_4   : 2;
    unsigned char GPIO_PIN_5   : 2;
    unsigned char GPIO_PIN_6   : 2;
    unsigned char GPIO_PIN_7   : 2;
    unsigned char GPIO_PIN_8   : 2;
    unsigned char GPIO_PIN_9   : 2;
    unsigned char GPIO_PIN_10  : 2;
    unsigned char GPIO_PIN_11  : 2;
    unsigned char GPIO_PIN_12  : 2;
    unsigned char GPIO_PIN_13  : 2;
    unsigned char GPIO_PIN_14  : 2;
    unsigned char GPIO_PIN_15  : 2;
} GPCON;

#define GPGCON    (*(volatile GPCON *)0x56000060)
#define GPGDAT    (*(volatile GPIOG *)0x56000064)

#define GPFCON (*(volatile GPCON *)0x56000050)
#define GPFDAT (*(volatile GPIOF *)0x56000054)

#define GPHCON (*(volatile unsigned *)0x56000070)

#define APBCLOCK (*(volatile unsigned *)0x4C000034)

#define ULCON1 (*(volatile unsigned *)0x50004000)
#define UCON1 (*(volatile unsigned *)0x50004004)
#define UFCON1 (*(volatile unsigned *)0x50004008)
#define UMCON1 (*(volatile unsigned *)0x5000400C)
#define UTRSTAT1 (*(volatile unsigned *)0x50004010)
#define UFSTAT1 (*(volatile unsigned *)0x50004018)
#define TX_BUFFER (*(volatile unsigned *)0x50004020)
#define RX_BUFFER (*(volatile unsigned *)0x50004024)
#define UBRDIV1 (*(volatile unsigned *)0x50004028)
#define UDIVSLOT1 (*(volatile unsigned *)0x5000402C)

void __attribute__((interrupt("IRQ"))) isr_eint_0(void);
void __attribute__((interrupt("IRQ"))) isr_eint_1(void);

int RX_Count = 0;
int mutex = 1;
int time_count = 0;

int pow(int base, int exp){
    int res = 1;
    while(exp){
        if(exp & 1)
            res *= base;
        exp >>= 1;
        base *= base;
    }
    return res;
}

int sqrt(int num, int val)
{   
    int count = 0;

    while((num / val) > 0){
        num /= val; 
        count++;
    }
    
    return count;
}

int swap(int led){
    int i = 0;
    int rem;
    int out = 0;

    for(i = 0; i < 4; i++){
        rem = led % 2;
        out += rem * pow(2, (3 - i));
        led /= 2;
    }
    return out;
}

char * itoa(int n, char * num) { 
    
    if(n < 10){
        num[0] = (n + 0x30);
        num[1] = '\0';

        return num;
    }
    else{
        num[0] = ((n / 10) + 0x30);
        num[1] = ((n % 10) + 0x30);
        num[2] = '\0';

        return num;
    } 
}

int atoi(char * num) { 
    int sum;
    sum = (num[0] - 0x30) * 10;
    sum += (num[1] - 0x30); 

    return sum;
}

void putstr(char * str){
    char ch;
    unsigned char flag_statue;
    while((ch = *str) != '\0'){
        // send
        TX_BUFFER = ch;     
        UFCON1 = (0x51);
        flag_statue = UTRSTAT1 & (1 << 1);
        while(flag_statue == 1);
        str++;
    }
}

char getchar_(){
    char ch;
    UFCON1 = (0x55);
    ch = RX_BUFFER;
    RX_Count = 1;
    return ch;
}

void getstr(char * buffer){
    int i = 0;
    char ch;
    unsigned char flag_statue;
    int size;

    size = UFSTAT1 & 0x003F;

    for(i = 0; i < size; i++){
        UFCON1 = (0x55);
        ch = RX_BUFFER;
        flag_statue = UTRSTAT1 & 1;
        while(flag_statue == 1);
        buffer[i] = ch;
    }
    buffer[i] = '\0';
    RX_Count = size;
}

void Uart_Init(int baud)
{
    int pclk;
    pclk = APBCLOCK;
    
    // PORT GPIO initial
    GPHCON &= ~(0xf<<4);    // USART1 Bit Clear
    GPHCON |= (0xa<<4);     // USART1 TX, RX Bit Set
    
    UFCON1 = 0x0;   
    UMCON1 = 0x0;               
    
    /* TODO : Line Control(Normal mode, No parity, One stop bit, 8bit Word length */
    ULCON1 = 0x03;

    /* TODO : Transmit & Receive Mode is polling mode  */
    UCON1 = 0x05;

    /* TODO : Baud rate ¼³Á¤  */        
    UBRDIV1 = 0x22;
    UDIVSLOT1 = 0xDFDD;
}

void gpio_init(){
    // LED INIT
    GPGCON.GPIO_PIN_4 = OUTPUT;
    GPGCON.GPIO_PIN_5 = OUTPUT;
    GPGCON.GPIO_PIN_6 = OUTPUT;
    GPGCON.GPIO_PIN_7 = OUTPUT;

    GPGDAT.LED = (0xF);    

    // KEY INIT
    GPFCON.GPIO_PIN_0 = EINT;
    GPFCON.GPIO_PIN_1 = EINT;
}

void exti_init(){
    // Set Interrupt Mod to IRQ
    rINTMOD = (0x0);
        
    // Reset Interrupt Mask
    rINTMSK1 = BIT_ALLMSK;              // (0xffffffff)

    // Clear Source Pending Bit 
    rSRCPND1 = BIT_EINT1;               // (0x1<<1)
    rSRCPND1 |= BIT_EINT0;              // (0x1)

    // Clear Interrupt Pending Bit 
    rINTPND1 = BIT_EINT1;       
    rINTPND1 |= BIT_EINT0;      

    // Set Interrupt Mask
    rINTMSK1 = ~(BIT_EINT0 | BIT_EINT1);

    // Set External Interrupt Edge Trigger
    rEXTINT0 = (rEXTINT0 & ~(0x7 << 1)) | (FALLING_EDGE << 1);
    rEXTINT0 |= (rEXTINT0 & ~(0x7 << 0)) | (FALLING_EDGE << 0);

    // ISR    
    pISR_EINT0 = (unsigned)isr_eint_0; 
    pISR_EINT1 = (unsigned)isr_eint_1;

}

void timer0_init(){
    rTCFG0 = (rTCFG0 & ~0xFF) | (33 - 1);   // 66,000,000 / 33
    rTCFG1 = (rTCFG1 & ~0xF);               // 66,000,000 / 33 / 2
    rTCNTB0 =  (0xFFFF);                    // Max size of buffer 
    rTCON |= (0x02);                        // update TCNTB0
    rTCON = (rTCON & ~(0xf) | (1 << 3) | (1 << 0)); // auto reload & start timer0
}

void delay_us(int us){
    volatile unsigned long now, last, diff;
    now = rTCNTO0;
    while(us > 0){
        last = now;
        now = rTCNTO0;
        if(now > last){ // timer reloaded
            diff = last + (0xFFFF - now) + 1;
        } else {
            diff = last - now;
        }
        us -= diff;
    }
}

void delay_ms(int ms){
    delay_us(ms * 1000);
}

void Main()
{   
    gpio_init();

    // BUS INIT
    APBCLOCK = (0xFFFFFFFF);
    
    Uart_Init(115200);

    exti_init();

    timer0_init();

    putstr("Program Started!!\r\n");

    while(1){
        GPGDAT.LED = ~(0xF);
        delay_ms(1000);
        GPGDAT.LED = (0xF);
        delay_ms(1000);
    }
}

// __irq =>  __attribute__((interrupt("IRQ")))

void  __attribute__((interrupt("IRQ"))) isr_eint_0(void)
{
    ClearPending1(BIT_EINT0);
    //putstr("e0\r\n");
    GPGDAT.LED = ~swap(0x8);
}

void __attribute__((interrupt("IRQ"))) isr_eint_1(void)
{
    ClearPending1(BIT_EINT1);
    //putstr("e1\r\n");
    GPGDAT.LED = ~swap(0x1);
}
