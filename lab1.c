#include "includes.h"

#define F_CPU   16000000UL   
#define CDS_VALUE   871

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#define UCHAR unsigned char 
#define USHORT unsigned short 
#define ATS75_CONFIG_REG 1
#define ATS75_TEMP_REG 0
#define ATS75_ADDR 0x98 

#define DO 17
#define RE 43
#define MI 66
#define FA 77
#define SOL 97
#define LA 114
#define TI 117
#define UDO 137

#define ON 1
#define OFF 0

#define  TASK_STK_SIZE  OS_TASK_DEF_STK_SIZE
#define  N_TASKS        7
#define       MSG_QUEUE_SIZE     1

OS_STK      TaskStk[N_TASKS][TASK_STK_SIZE];
OS_EVENT* Mbox1;
OS_EVENT* Mbox2;
OS_EVENT* Mbox3;
OS_EVENT* Mbox4;
volatile OS_FLAG_GRP* E_F1;
volatile OS_FLAG_GRP* E_F2;

int state = ON;
volatile int index = 0;
const int scale[8] = { DO, RE, MI, FA, SOL, LA, TI, UDO };

const unsigned int number[8] = { 1,2,4,8,16,32,64,128 };

UCHAR send[4];

volatile int isStart = 0;
int isFinish = 0;
volatile int pushVec5 = 0;
volatile int quizStart = 0;
volatile USHORT num = 0; //퀴즈 정답 제출로 쓰일 숫자

void init_adc();
void adcTask(void* data);
void showRandom(void* data);
void createRandom(void* data);
void quizControl(void* data);
void FndDisplayTask(void* data);

int main(void)
{
    INT8U err;
    OSInit();
    OS_ENTER_CRITICAL();

    DDRE = 0xcf; //스위치 4, 5 입력
    EICRB = 0x0a; //INT4, 5 Falling
    EIMSK = 0x30; //INT4, 5 Enable
    SREG |= 1 << 7; //sei()
    TCCR2 = 0x03; //32분주
    DDRA = 0xff; //LED활성화

    OS_EXIT_CRITICAL();


    // Create an event flag,
    E_F1 = OSFlagCreate(0x00, &err);
    E_F2 = OSFlagCreate(0x00, &err);
    Mbox1 = OSMboxCreate((void*)0);
    Mbox2 = OSMboxCreate((void*)0);
    Mbox3 = OSMboxCreate((void*)0);
    Mbox4 = OSMboxCreate((void*)0);


    OSTaskCreate(adcTask, (void*)0, (void*)&TaskStk[0][TASK_STK_SIZE - 1], 0);
    OSTaskCreate(createRandom, (void*)0, (void*)&TaskStk[1][TASK_STK_SIZE - 1], 1);
    OSTaskCreate(showRandom, (void*)0, (void*)&TaskStk[2][TASK_STK_SIZE - 1], 2);
    OSTaskCreate(quizControl, (void*)0, (void*)&TaskStk[3][TASK_STK_SIZE - 1], 3);
    OSTaskCreate(FndDisplayTask, (void*)0, (void*)&TaskStk[4][TASK_STK_SIZE - 1], 4);
    OSStart();
    return 0;
}

ISR(INT4_vect) {
    INT8U err;
    if (isStart == 0)// 1.조도센서를 작동시키며 시작을 알림.
    {
        isStart = 1;
        OSFlagPost(E_F1, 0x01, OS_FLAG_SET, &err);
    }
    else if (quizStart) // 2. 퀴즈 중에 스위치를 누르면 표기하는 음과 LED가 한단계 증가함.
    {
        num = (num + 1) % 8;
        index = num;
        PORTA = number[num];
    }
}

ISR(INT5_vect) {
    INT8U err;
    USHORT i = 0;
    if (quizStart == 1) {//1. 퀴즈 중에 스위치를 누르면 정답이 제출됨.
        OSMboxPost(Mbox1, (void*)&i);
    }
    else {//2. 퀴즈로 score를 3점 달성한 후 스위치를 눌러 FND와 LED를 끈다. 
        isFinish = 1;
        PORTA = 0x00;
    }
}

ISR(TIMER2_OVF_vect) { //타이머 인터럽트
    if (state == ON) {
        PORTB = 0x00;
        state = OFF;
    }
    else {
        PORTB = 0x10;
        state = ON;
    }
    TCNT2 = scale[index]; //음 조절
}

void init_adc(void) //조도 센서 시작
{
    ADMUX = 0x00;
    ADCSRA = 0x87;
}

unsigned short read_adc(void) //조도센서값을 읽는 함수
{
    unsigned char adc_low, adc_high;
    unsigned short value;
    ADCSRA |= 0x40;
    while ((ADCSRA & 0x10) != 0x10);
    adc_low = ADCL;
    adc_high = ADCH;
    value = (adc_high << 8) | adc_low;

    return value;
}

void adcTask(void* data)
{
    INT8U err;
    data = data;
    USHORT value = 0;
    init_adc();
    OSFlagPend(E_F1, 0x01, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME, 0, &err);//스위치4가 깨워준다.
    PORTA = 0xff;
    while (1)
    {
        value = read_adc();
        if (value > CDS_VALUE) { //10LUX보다 밝아지면 알람시작!
            OSFlagPost(E_F2, 0x01, OS_FLAG_SET, &err);
            break;
        }
    }
    OSTimeDlyHMSM(0, 0, 1, 0);
}

void createRandom(void* data) { //랜덤숫자를 생성해준다. 
    data = data;
    INT8U err;
    USHORT r;
    while (1) {
        OSFlagPend(E_F2, 0x01, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME, 0, &err);
        r = (13 + rand()) % 8;
        OSMboxPost(Mbox4, (void*)&r);
    }
}

void showRandom(void* data) { //랜덤으로 생성한 숫자로 버저와 LED를 켠다.
    data = data;
    INT8U err;
    USHORT Rvalue;
    while (1) {
        Rvalue = *(USHORT*)OSMboxPend(Mbox4, 0, &err);
        TIMSK = 0x40; //overFlow
        DDRB = 0x10; //버저활성화
        index = Rvalue;
        PORTA = number[Rvalue];

        _delay_ms(3000); //일정시간 랜덤음과, LED를 보여주고 끈다.

        PORTA = 0x00;
        TIMSK = 0x00; //overFlow비활성화
        PORTB = 0x00;
        _delay_ms(3000);

        OSMboxPost(Mbox2, (void*)&Rvalue);
    }
}

void quizControl(void* data) { //퀴즈가 시작된다.
    data = data;
    INT8U err;
    USHORT correctValue;
    USHORT score = 0; //점수가 3점이 되면 알람이 꺼진다!
    USHORT tmp;
    while (1) {
        correctValue = *(USHORT*)OSMboxPend(Mbox2, 0, &err);
        num = 0; // 0번 음, LED에서 시작한다.
        quizStart = 1;
        TIMSK = 0x40; //overFlow 
        PORTA = number[num];
        OSMboxPost(Mbox3, (void*)&score); //FND에 현재 점수를 표기한다.
        *(USHORT*)OSMboxPend(Mbox1, 0, &err);//스위치 5로 정답이 제출된다.
        quizStart = 0;
        if (correctValue == num) { //정답(랜덤생성값)과 현재 num을 비교한다.
            score++; //같다면 점수를 올려준다.
            PORTA = 0xff;//맞췄을 떄는 LED전체를 점등한다.
        }
        else {//틀렸을 때는 LED를 띄엄띄엄 점등한다.
            PORTA = 0xaa;
        }
        TIMSK = 0x00; //overFlow비활성화
        PORTB = 0x00;
        PORTC = 0x00;

        _delay_ms(2000);
        if (score < 3) //3점이 되지 않았다면 다시 랜덤 숫자를 뽑아 퀴즈를 진행한다.
            OSFlagPost(E_F2, 0x01, OS_FLAG_SET, &err);
        else {
        }
    }

}

void FndDisplayTask(void* data)
{
    data = data;
    INT8U err;
    UCHAR FND_DATA[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x40, 0x00 };
    UCHAR fnd_sel[4] = { 0x01, 0x02, 0x04, 0x08 };
    USHORT* display;
    int i;

    display = (USHORT*)OSMboxPend(Mbox3, 0, &err);
    DDRC = 0xff;
    DDRG = 0x0f;
    while (1) {
        if (isFinish) //끝났다면 FND를 끈다.
        {
            *display = 11;
        }
        for (i = 0; i < 4; i++)
        {
            PORTC = FND_DATA[*display];
            PORTG = fnd_sel[i];
            OSTimeDlyHMSM(0, 0, 0, 1);
        }
    }
}