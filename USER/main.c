#include "stm32f10x.h"
#include "led.h"
#include "delay.h"
#include "key.h"
#include "beep.h"
#include "sys.h"
#include "usart.h"
#include "usmart.h"	 
#include "24cxx.h"	 
#include "string.h"

char send0[3][128] = {"I2C����ʵ��-AT24C02\r\n", 
	"������ �����h ���\r\n",
	"18 05 10 9 47 30\r\n"};

char send1[128] = {"������Ŀ���ַ������\r\n"};

char writeOrder[128]; // д�����ݵ�����, "0x04 0x01 0x10 0x80 0x40"
char readOrder[128]; // �����ݵ�����, "��0x04��ַ��ʼ��5 BYTE", ��ַҪд��λ16�����ַ�
u8 buffer[32]; // д������������
u8 address; // д�������ĵ�ַ

void init(void);
u8 read(void);
void write(void);
void sendSend0(void);
void sendSend1(void);
void sendByteData(u8);
u8 myAtoiHex(char*, u8, u8);
u8 myAtoiDec(char*, u8, u8);
void myItoaHex(char*, u8);

 int main(void)
 {	
	u8 tenMs = 0; // �����˶��ٸ� 10 ms
	u8 recLen; // �յ��������ַ����ĳ���
	u8 size; // ������ byte ������
	 
	init();
	
	// LED0 ʼ�ձ�����200ms����500ms������ģʽ���ȴ���������ͷ��������ַ�	 
	while(1)
	{	
		if(KEY0 == 0 || KEY1 == 0 || KEY2 == 0 || KEY3 == 1){ // �а������£�������һ����������
			sendSend0();
			delay_ms(200); // �����ӳ�һ�µȰ����ɿ�������һ�ΰ���������ʮ���룬����Ķ�������
			break;
		}
		
		// �����Ҫ���Ļ�
		if(USART_RX_STA & 0x8000){ 
            // save received data as a string
            recLen = USART_RX_STA & 0x3fff; // length in byte
            memcpy(readOrder, USART_RX_BUF, recLen); 
            readOrder[recLen] = '\0';
            size = read();
            // set received data length to 0
            USART_RX_STA = 0;
			sendByteData(size);
			break;
        }
		
		tenMs ++;
		delay_ms(10);
		if(!LED0 && tenMs == 20){ // �� 200 ms
			LED0 = 1;
			tenMs= 0;
		}else if(LED0 && tenMs == 50){ // �� 500 ms
			LED0 = 0;
			tenMs= 0;
		}
	}
	
	// LED0 ��200ms����200ms���ȴ���������ͷ�����ʾ�� 
	while(1){
		if(KEY0 == 0 || KEY1 == 0 || KEY2 == 0 || KEY3 == 1){ // �а������£�������һ������ 5 ��
			sendSend1();
			delay_ms(200); // �����ӳ�һ�µȰ����ɿ�
			break;
		}
		tenMs ++;
		delay_ms(10);
		if(tenMs == 20){ // �� 200 ms���� 200 ms
			LED0 = !LED0;
			tenMs= 0;
		}
	}
	
	// �ȴ�����������򿪷��巢�� ADR  DATA1  DATA2  DATA3��д������
	while(1){
        if(USART_RX_STA & 0x8000){ 
            recLen = USART_RX_STA & 0x3fff; // length in byte
            memcpy(writeOrder, USART_RX_BUF, recLen); // ����"0x04 0x01 0x10 0x80 0x40"
            writeOrder[recLen] = '\0';
			write();
            USART_RX_STA = 0; // set received data length to 0
			break;
        }
		tenMs ++;
		delay_ms(10);
		if(tenMs == 20){ // �� 200 ms���� 200 ms
			LED0 = !LED0;
			tenMs= 0;
		}
	}
	
	// д������Ժ󣬷�������500ms��ֹͣ
	BEEP = 1;
	delay_ms(500);
	BEEP = 0;
	
	// LED0��Ϊ��600ms����600msģʽ
	while(1){
		tenMs ++;
		delay_ms(10);
		if(tenMs == 60){ // �� 600 ms���� 600 ms
			LED0 = !LED0;
			tenMs= 0;
		}
	}
}
 
void init(void){
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(9600);	 	 //���ڳ�ʼ��Ϊ9600
 	LED_Init();			     //LED�˿ڳ�ʼ��
	BEEP_Init();	 
	KEY_Init();
 	AT24CXX_Init();			 //IIC��ʼ�� 
}
u8 read(void){
	// �����յ����� readOrder ��Ҫ��������� buffer[] ��
	// ���ӣ�"��0(#2)x04��ַ��ʼ��5(#16) BYTE"
	u8 i;
	u8 size;
	address = myAtoiHex(readOrder, 2, 6);
	for(i = 16; i < strlen(readOrder); i ++){
		if(readOrder[i] > '9' || readOrder[i] < '0'){
			break;
		}
	}
	size = myAtoiDec(readOrder, 16, i);
	AT24CXX_Read(address, buffer, size);
	return size;
}

void write(void){
	// ����char writeOrder[128]����д�����д������
	u8 orderlen = strlen(writeOrder);
	u8 i;
	u8 iBuffer = 0;
	u8 start = 0;
	for(i = 0; i < 3; i ++){ // ��һ�� trim
		if(writeOrder[orderlen - 1] == '\n' || writeOrder[orderlen - 1] == '\r' || writeOrder[orderlen - 1] == ' '){
			orderlen --;
		}
	}
	for(i = 0; i < orderlen; i ++){
		if(writeOrder[i] == ' ' && start == 0){
			address = myAtoiHex(writeOrder, start, i);
			start = i + 1;
		}else if(writeOrder[i] == ' '){
			buffer[iBuffer ++] = myAtoiHex(writeOrder, start, i);
			start = i + 1;
		}
	}
	buffer[iBuffer ++] = myAtoiHex(writeOrder, start, i);
	AT24CXX_Write(address, (u8*)buffer, iBuffer);
}


void sendSend0(void){
	u8 i, j;
	u8 sendLen = 0; // �������ַ����ĳ���
	for(i = 0; i < 3; i ++){
		sendLen = strlen(send0[i]);
		for(j = 0; j < sendLen; j ++){
			USART_SendData(USART1, send0[i][j]);
			while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET){ 
			// haven't finished sending, do nothing
			}
		}
	}
}

void sendSend1(void){
	u8 j;
	u8 sendLen = strlen(send1); // �������ַ����ĳ���
	for(j = 0; j < sendLen; j ++){
		USART_SendData(USART1, send1[j]);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET){ 
		// haven't finished sending, do nothing
		}
	}
}

void sendByteData(u8 size){
	// ��buffer �е����ݱ�ʾ��16�����ַ������͸������
	char temp[4];
	u8 i, j;
	for(i = 0; i < size; i ++){
		myItoaHex(temp, buffer[i]);
		for(j = 0; j < 4; j ++){
			USART_SendData(USART1, temp[j]);
			while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET){ 
			}
		}
		USART_SendData(USART1, ' '); //�ٷ����ո�
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET){ 
		}
	}
	USART_SendData(USART1, '\n');
}

u8 myAtoiHex(char* a, u8 start, u8 end){
	// ����char a[start: end) ����16�����ַ�
	u8 ans = 0;
	u8 i;
	for(i = start + 2; i < end; i ++){
		if(a[i] >= '0' && a[i] <= '9'){
			ans = ans * 16 + a[i] - '0';
		}else if (a[i] >= 'a' && a[i] <= 'z'){
			ans = ans * 16 + a[i] - 'a';
		}else if (a[i] >= 'A' && a[i] <= 'Z'){
			ans = ans * 16 + a[i] - 'A';
		}
	}
	return ans;
}


u8 myAtoiDec(char *a, u8 start, u8 end){
	// ����char a[start: end) ����10�����ַ�
	u8 ans = 0;
	u8 i;
	for(i = start; i < end; i ++){
		if(a[i] >= '0' && a[i] <= '9'){
			ans = ans * 10 + a[i] - '0';
		}
	}
	return ans;
}

void myItoaHex(char* p, u8 num){
	// �� num �е���ת����λ 16 �����ַ���0x??��д�� p ��ͷ��λ��
	char digits[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	*(p ++) = '0';
	*(p ++) = 'x';
	*(p ++) = digits[num / 16];
	*(p ++) = digits[num % 16];
}

