# S3C2450-Interrupt

S3C2450 Board Interrupt Configuration

# s3c2450_startup.S

S3C2450 스타트업 코드에서의 인터럽트 부분 분석

    .equ _ISR_STARTADDRESS,		0x33ffff00    // ISR 시작 주소 

![](Image/Untitled-8ef6fd58-3648-4c75-b6e3-e6dabc848a6c.png)

    /* ResetHandler가 처음부터 나오는 것이 아니라 
    	 * vector 주소 영역에는 reset vector가 존재해야 한다
    	 * exception이 발생하면 ARM은 하드웨어적으로 다음 주소로 분기된다
    	 */
    	b	ResetHandler
    	b	HandlerUndef			/* Undefined mode 핸들러 */
    	b	HandlerSWI				/* SWI 인터럽트 핸들러 */
    	b	HandlerPabort			/* PAbort 핸들러 */
    	b	HandlerDabort			/* DAbort 핸들러 */
    	b	.						      /* 예약된 공간 */
    	b	HandlerIRQ				/* IRQ 핸들러 */
    	b	HandlerFIQ				/* FIQ 핸들러 */

S3C2450는 예외 벡터 테이블(Exception Vector Table)을 가지고 있는데, 만약 IRQ 신호를 받게되면 이 중 0x18 번지로 점프하게 된다.

    /* macro 정의 */
    	.macro HANDLER, HandlerLabel, HandleLabel
    \HandlerLabel:
    	sub		sp,sp,#4			/* decrement sp(to store jump address) */
    	stmfd	sp!,{r0}			/* PUSH the work register to stack(lr doesn`t push because */ 
    						   		/* it return to original address) */
    	ldr		r0,=\HandleLabel	/* load the address of HandleXXX to r0 */
    	ldr		r0,[r0]         	/* load the contents(service routine start address) of HandleXXX */
    	str		r0,[sp,#4]      	/* store the contents(ISR) of HandleXXX to stack */
    	ldmfd	sp!,{r0,pc}     	/* POP the work register and pc(jump to ISR) */
    	.endm

HANDLER라는 매크로 함수를 정의한다.

HANDLER 매크로 함수

1. 스택포인터를 4만큼 감소 시켜서 빈공간을 확보한다.
2. r0를 사용하기 위해 사용중이던 r0를 스택포인터에 저장한다.
3. r0에 HandleIRQ의 주소를 로드한다.
4. r0에 r0의 값(HandleIRQ의 값)을 로드한다.
5. r0를 스택포인터에 4를 증가시킨 곳에 저장한다.
6. 저장 해 둔 r0의 값을 읽어오고 프로그램 카운터의 핸들러의 값을 넣어, 핸들러로 점프한다.
<pre><code>    HANDLER HandlerFIQ, HandleFIQ
    HANDLER HandlerIRQ, HandleIRQ
    HANDLER HandlerUndef, HandleUndef
    HANDLER HandlerSWI, HandleSWI
    HANDLER HandlerDabort, HandleDabort
    HANDLER HandlerPabort, HandlePabort
</code></pre>
HandlerIRQ와 HandleIRQ를 인자로 HANDLER 매크로 실행 ⇒ HandleIRQ를 핸들러로 사용하게 된다.

    /* 여기서 IRQ가 발생할때 위에서 만든 
    	 * IsrIRQ 루틴으로 분기하기 위한 ISR_VECTOR 설정을 한다 
    	 */
    
    	/* Setup IRQ handler */
    	ldr	r0,=HandleIRQ  
    	ldr	r1,=IsrIRQ			
    	str	r1,[r0]

HandleIRQ의 주소를 로드 해와서 그 값에 IsrIRQ의 주소를 저장한다.

이렇게 되면 HandleIRQ에서 IsrIRQ 루틴으로 분기하게 된다.

    /* IRQ Handler 생성
    	 * IRQ는 발생 소스가 다양하기 때문에 해당 C루틴의 주소를 획득하여야 한다
    	 */1
    	.globl IsrIRQ
    IsrIRQ:
    	sub	sp,sp,#4			/* reserved for PC */
    	stmfd	sp!,{r8-r9}
    	
    	ldr	r9,=INTOFFSET1
    	ldr	r9,[r9]
    	ldr	r8,=HandleEINT0
    	add	r8,r8,r9,lsl #2
    	ldr	r8,[r8]
    	str	r8,[sp,#8]
    	ldmfd	sp!,{r8-r9,pc}

IRQ 핸들러를 생성하고 전역으로 선언한다

1. 스택 포인터를 4 만큼 감소시켜서 빈공간을 확보한다.
2. r8과 r9의 값을 백업하는데 스택의 포인터를 감소시키며 저장한다.
3. r9에 INTOFFSET1을 로드한다, 두 개의 INTOFFSET들 중에 첫번째 INTOFFSET1
4. r9에 r9의 값을 로드한다.
5. r8에 HandleEINT0를 로드한다, HandleEINT0는 첫번째 인터럽트 핸들러 
6. r9를 왼쪽으로 2비트 시프트 하고 r8에 더한 것을 r8에 로드한다.
7. r8에 r8의 값을 로드한다,
8. 스택포인터에서 8만큼(스택 2칸) 증가 시킨 곳(처음에 만든 빈공간)에 r8을 저장한다.
9. 데이터를 로드하면서 스택 포인터를 증가  저장해둔 r8, r9의 값을 읽어오고 8에서 저장해둔 ISR의 주소를 프로그램 카운터에 넣어줌으로써 ISR로 점프한다.
<pre><code>    //; .=0x33ffff00
    //HandleReset 	#   4
    //HandleUndef 	#   4
    //HandleSWI		#   4
    //HandlePabort    #   4
    //; .=0x33ffff10
    //HandleDabort    #   4
    //HandleReserved  #   4
    //HandleIRQ		#   4
    //HandleFIQ		#   4
    //
    //; .=0x33ffff20
    //;IntVectorTable
    //HandleEINT0		#   4
    //HandleEINT1		#   4
    //HandleEINT2		#   4
    //HandleEINT3		#   4
    //; .=0x33ffff30
    //HandleEINT4_7	#   4
    //HandleEINT8_23	#   4
    //HandleCAM		#   4
    //HandleBATFLT	#   4
    //; .=0x33ffff40
    //HandleTICK		#   4
    //HandleWDT		#   4
    //HandleTIMER0 	#   4
    //HandleTIMER1 	#   4
    //; .=0x33ffff50
    //HandleTIMER2 	#   4
    //HandleTIMER3 	#   4
    //HandleTIMER4 	#   4
    //HandleUART2  	#   4
    //; .=0x33ffff60
    //HandleLCD 		#   4
    //HandleDMA0		#   4
    //HandleDMA1		#   4
    //HandleDMA2		#   4
    //; .=0x33ffff70
    //HandleDMA3		#   4
    //HandleMMC		#   4
    //HandleSPI0		#   4
    //HandleUART1	#   4
    //; .=0x33ffff80
    //HandleNFCON	#   4
    //HandleUSBD		#   4
    //HandleUSBH		#   4
    //HandleIIC		#   4
    //; .=0x33ffff90
    //HandleUART0 	#   4
    //HandleSPI1 		#   4
    //HandleRTC 		#   4
    //HandleADC 		#   4
    //; .=0x33ffffa0
    //
    //	END 
    */
</code></pre>
0x33FFFF00 ~ 0x33FFFF1C : 예외 벡터 테이블, 0x33FFFF20 ~ 0x33FFFFA0 : 인터럽트 벡터 테이블

스타트업 코드에서의 전체적인 순서

1. 인터럽트 신호 발생
2. 예외 벡터 테이블 0x18 번지로 점프
3. HandlerIRQ 루틴 실행
4. HandlerIRQ에서 IsrIRQ 루틴으로 점프
5. 인터럽트 벡터 테이블로 점프
6. 해당 주소에 할당된 핸들러 함수 실행

---

# Main.c

![](Image/S3C2450_UM_REV11-beeeee99-d771-4e48-b365-754ea41e3456.30_20090305.pdf-AdobeAcrobatReaderDC1_7_20194_36_58PM(2).png)

S3C2450의 인터럽트 프로세싱 다이어그램

## 인터럽트 관련 레지스터

만약 처리하고자 하는 인터럽트가 서브 인터럽트가 존재한다면, 먼저 서브 인터럽트를 펜딩시켜주고 마스킹 해 주어야 한다.

SUBSRCPND 레지스터

![](Image/SUBSRCPND-c7c9f9ce-055c-4f71-9263-97efbd7e575e.PNG)

![](Image/SUBSRCPND_-8e59f364-0e97-44e4-adb2-48a06da0c0aa.PNG)

서브 소스 펜딩 레지스터에는 UART 인터럽트의 TX 서브 인터럽트, RX 서브 인터럽트 등과 같이 세부 인터럽트의 펜딩 값이 있다.

INTSUBMSK 레지스터

![](Image/INTSUBMSK-4ae41794-0b11-4e07-aba3-83cab63b54f3.PNG)

![](Image/INTSUBMSK_-433a7a46-5101-44f4-8c0b-58986ea9f013.PNG)

같은 느낌으로 INTSUBMASK에는 세부 인터럽트의 마스크 값이 있다.

위의 두 레지스터값은 서브 소스가 존재 할 때에만 사용하면 되고 없으면 사용 할 필요가 없다.

SRCPND 레지스터

![](Image/SRCPND-e4ceb96a-dfb7-400f-8e21-bda6b47cd7f6.PNG)

![](Image/SRCPND_-02dd7784-cc89-4ba2-bd2d-e2ff26cb7c0b.PNG)

인터럽트 소스로 부터 요청이 있으면 해당 비트를 1로 세팅한다.

- ISR에서 수동으로 클리어 해 주어야 함, 클리어 하지 않으면 ISR 반복 실행한다.
- 해당비트에 1을 써 넣으면 클리어 된다.

INTMOD 레지스터

![](Image/INTMOD-f6c52b1f-3ae7-458e-b2f7-6a7b2458434e.PNG)

![](Image/INTMOD_-521850a2-0599-40df-bb9e-2e139a1ca2e8.PNG)

값이 0 이면 인터럽트의 모드를 IRQ로 세팅하고 1이면 FIQ로 세팅한다, 기본 값은 0

- FIQ는 매우 높은 우선순위를 가진 인터럽트를 의미하며 하나 만 사용 가능 하다.
- IRQ는 기본적인 인터럽트이고, 보통 수준의 우선순위를 가진다.

INTMSK 레지스터

![](Image/INTMSK-2d7a7c2c-a8d9-47d2-a78c-ed3099c0018c.PNG)

![](Image/INTMSK_-48f5a5dd-509c-4662-bcaa-99c79eff14c0.PNG)

원하는 비트를 0으로 마스킹 해 주면 해당 비트의 인터럽트를 사용 가능, 기본값은 1 

INTPND 레지스터

![](Image/INTPND-81b8a740-c9ec-43be-a9bd-63537ee24c96.PNG)

![](Image/INTPND_-11cfcaeb-5ebb-4641-8e02-023f6659f716.PNG)

인터럽트 서비스를 요청하면 1로 세팅 된다.

우선순위 로직 이후의 상황이므로 32개의 비트중 단, 하나만 설정된다.

- 해당 비트에 1을 써넣으면 클리어 된다.

EXTINT 레지스터

![](Image/EXTINT-9131a8bc-5a62-4fc2-8184-91ee0284b7c8.PNG)

![](Image/Extinct_-c37c003a-ef65-4d99-a0db-8ce8a9306fe1.PNG)

![](Image/Extinct_--4558e286-9a2a-49c0-8731-85538b0d8416.PNG)

![](Image/Extinct_---5cef8a6c-b309-451f-afbd-3e70a3c8db72.PNG)

![](Image/Extinct_----4090a325-47b1-468a-a5fe-345f8e0154a4.PNG)

외부 인터럽트의 트리거를 설정하는 레지스터, 0x0은 Low Level, 0x1은 High Level, 0x2는 Falling Edge, 0x4는 Rising Edge, 0x6은 Both Edge

![](Image/Untitled-cacaacb8-7cc6-438e-bf79-6ca47b3160a6.png)

폴링 엣지는 클럭 펄스가 HIGH에서 LOW로 떨어 질 때 인터럽트가 발생하고

라이징 엣지는 반대로 LOW에서 HIGH로 올라 갈 때 인터럽트가 발생한다.

EINTMASK 레지스터

![](Image/EINTMASK-28c9371f-6ee6-4f6c-9e9d-6f771a426910.PNG)

원하는 비트를 0으로 마스킹 해 주면 해당 비트의 외부 인터럽트를 사용 가능, 기본값은 1 

EINT0 ~ EINT3의 경우는 별도의 외부 인터럽트 마스킹이 불필요하다.

EINTPEND 레지스터

![](Image/EINTPEND-ea5daac2-27bb-4c9e-a215-ad658376a848.PNG)

INTPND와 동일하나 외부 인터럽트 한정이다.

## 인터럽트 초기화 코드

GPIO 초기화

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
    
    enum IO_MODE {
        INPUT = (0x0),
        OUTPUT = (0x1),
        EINT = (0x2),
        RESERVED = (0x3)
    };
    
    #define GPGCON    (*(volatile GPCON *)0x56000060)
    #define GPGDAT    (*(volatile GPIOG *)0x56000064)
    
    #define GPFCON (*(volatile GPCON *)0x56000050)
    #define GPFDAT (*(volatile GPIOF *)0x56000054)
    
    void gpio_init(){
        // LED INIT
        GPGCON.GPIO_PIN_4 = OUTPUT;     // Set LED1 to OUTPUT
        GPGCON.GPIO_PIN_5 = OUTPUT;     // Set LED2 to OUTPUT
        GPGCON.GPIO_PIN_6 = OUTPUT;     // Set LED3 to OUTPUT
        GPGCON.GPIO_PIN_7 = OUTPUT;     // Set LED4 to OUTPUT
    
        GPGDAT.LED = (0xF);             //  Turn off LEDs
    
        // KEY INIT
        GPFCON.GPIO_PIN_0 = EINT;       // Set Key1 to EINT
        GPFCON.GPIO_PIN_1 = EINT;       // Set Key2 to EINT
    }

LED1 ~ LED4를 GPG4 ~ GPG7에 할당하고 출력핀으로 설정한다.

LED에 VCC를 걸어놓고 GND부분을 출력 핀에 연결하여 LED를 제어함 ⇒ 0 == LED ON, 1 == LED OFF 

Key1, Key2를 GPF0, GPF1에 할당하고 외부 인터럽트로 설정한다.

GPGCON, GPFCON, GPGDAT, GPFDAT 레지스터를 비트필드 구조체로 선언하여서 비트별로 제어 할 수 있게한다.

인터럽트 초기화

    // 2450addr.h
    #define BIT_EINT0		(0x1)
    #define BIT_EINT1		(0x1<<1)
    #define BIT_ALLMSK		(0xffffffff)
    
    #define	rSRCPND1	(*(volatile unsigned *)	0x4a000000)    /*Interrupt request status*/		
    #define	rINTMOD1	(*(volatile unsigned *)	0x4a000004)    /*Interrupt mode control*/		
    #define	rINTMSK1 	(*(volatile unsigned *)	0x4a000008 )   /*Interrupt mask control*/  		
    #define	rINTPND1	(*(volatile unsigned *)	0x4a000010)		  /*Interrupt request status*/
    #define	rINTOFFSET1	(*(volatile unsigned *)	0x4a000014 )   /*Interruot request source offset*/		
    #define	rSUBSRCPND	(*(volatile unsigned *)	0x4a000018)    /*Sub source pending*/		
    #define	rINTSUBMSK	(*(volatile unsigned *)	0x4a00001c)    /*Interrupt sub mask*/
    
    #define rEXTINT0   (*(volatile unsigned *)0x56000088)	//External interrupt control register 0
    #define rEXTINT1   (*(volatile unsigned *)0x5600008c)	//External interrupt control register 1
    
    #define pISR_EINT0		(*(unsigned *)(_ISR_STARTADDRESS+0x20))
    #define pISR_EINT1		(*(unsigned *)(_ISR_STARTADDRESS+0x24))
    
    // Main.c
    enum EXTI_MODE {
        LOW_LEVEL = (0x0),
        HIGH_LEVEL = (0x1),
        FALLING_EDGE = (0x2),
        RISING_EDGE = (0x4),
        BOTH_EDGE = (0x6)
    };
    
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

1. 먼저 인터럽트 모드를 IRQ모드로 세팅한다.
2. 인터럽트 마스크를 모두 1로 만들어서 리셋한다.
3. 원하는 소스 펜딩 비트에 1을 써넣어 클리어한다.
4. 원하는 인터럽트 펜딩 비트에 1을 써넣어 클리어한다.
5. 원하는 인터럽트 마스킹 비트를 마스킹해서 인터럽트를 사용 가능하게 한다.
6. 원하는 외부 인터럽트의 트리거를 세팅, 여기서는 EINT1과 EINT0을 사용해서 0b000을 각각 1번 0번 쉬프트해서 rEXTINT0에 &연산을 해주어 기존의 값을 지우고 원하는 트리거를 똑같이 쉬프트를 하여 넣어준다. 
7. 마지막으로 어셈블러에서 처리되는 ISR의 주소에 만든 인터럽트 핸들러 함수 포인터를 대입한다.

인터럽트 핸들러 구현

    // 2450addr.h
    #define	ClearPending1(bit) {\
    			rSRCPND1 = bit;\
    			rINTPND1 = bit;\
    			rINTPND1;\
    		}	
    
    // Main.c
    
    // LED Display Function
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
    
    // Prototype
    void __attribute__((interrupt("IRQ"))) isr_eint_0(void);
    void __attribute__((interrupt("IRQ"))) isr_eint_1(void);
    
    // Handler
    void  __attribute__((interrupt("IRQ"))) isr_eint_0(void)
    {
        ClearPending1(BIT_EINT0);
        // putstr("e0\r\n");
        GPGDAT.LED = ~swap(0x8);
    }
    
    void __attribute__((interrupt("IRQ"))) isr_eint_1(void)
    {
        ClearPending1(BIT_EINT1);
        // putstr("e1\r\n");
        GPGDAT.LED = ~swap(0x1);
    }

레지스터 부분에서 설명 했듯이 펜딩비트를 수동으로 클리어 해주지 않으면 ISR이 반복 실행 되기 때문에 처음에 펜딩비트를 클리어 해 주어야한다.

보통 다른 예제나 책에서 __irq를 사용하여 인터럽트 핸들러를 구현했으나,

같은 방법으로 했을 때 에러가 발생하여 __attribute__((interrupt("IRQ")))를 사용했다.

__irq와 __attribute__((interrupt("IRQ")))는 같은 의미이며 

__irq가 사용 가능하다면 이미 선언이 되어있는 것이고 불가능 하다면 선언이 되어있지 않은 것이다.

보통 핸들러 함수 내부에서는 정말 간단한 작업만을 하는 것이 좋다.

시리얼포트로 값을 전송하는 것도 가능하지만 자신이 원하는 대로 전송되지 않을 수 있다.

Main 함수 구현

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

간단한 딜레이 함수를 만들어 Main함수 내부에서 전체 LED를 1초간격으로 Blink할때,

인터럽트 주면 실행되다 인터럽트 핸들러에서의 명령이 실행된다.
