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

char send0[3][128] = {"I2C总线实验-AT24C02\r\n", 
	"吴熠玮 董梓玥 组合\r\n",
	"18 05 10 9 47 30\r\n"};

char send1[128] = {"请输入目标地址和数据\r\n"};

char writeOrder[128]; // 写入数据的命令, "0x04 0x01 0x10 0x80 0x40"
char readOrder[128]; // 读数据的命令, "读0x04地址开始的5 BYTE", 地址要写两位16进制字符
u8 buffer[32]; // 写入或读出的数据
u8 address; // 写入或读出的地址

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
	u8 tenMs = 0; // 经过了多少个 10 ms
	u8 recLen; // 收到的命令字符串的长度
	u8 size; // 读多少 byte 的数据
	 
	init();
	
	// LED0 始终保持亮200ms、灭500ms的运行模式，等待按键命令，就发送三行字符	 
	while(1)
	{	
		if(KEY0 == 0 || KEY1 == 0 || KEY2 == 0 || KEY3 == 1){ // 有按键按下，进行下一步，第三步
			sendSend0();
			delay_ms(200); // 这里延迟一下等按键松开，否则一次按键持续几十毫秒，后面的都运行了
			break;
		}
		
		// 如果是要读的话
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
		if(!LED0 && tenMs == 20){ // 亮 200 ms
			LED0 = 1;
			tenMs= 0;
		}else if(LED0 && tenMs == 50){ // 灭 500 ms
			LED0 = 0;
			tenMs= 0;
		}
	}
	
	// LED0 亮200ms、灭200ms，等待按键命令，就发送提示语 
	while(1){
		if(KEY0 == 0 || KEY1 == 0 || KEY2 == 0 || KEY3 == 1){ // 有按键按下，进行下一步，第 5 步
			sendSend1();
			delay_ms(200); // 这里延迟一下等按键松开
			break;
		}
		tenMs ++;
		delay_ms(10);
		if(tenMs == 20){ // 亮 200 ms，灭 200 ms
			LED0 = !LED0;
			tenMs= 0;
		}
	}
	
	// 等待计算机串口向开发板发送 ADR  DATA1  DATA2  DATA3，写入数据
	while(1){
        if(USART_RX_STA & 0x8000){ 
            recLen = USART_RX_STA & 0x3fff; // length in byte
            memcpy(writeOrder, USART_RX_BUF, recLen); // 发送"0x04 0x01 0x10 0x80 0x40"
            writeOrder[recLen] = '\0';
			write();
            USART_RX_STA = 0; // set received data length to 0
			break;
        }
		tenMs ++;
		delay_ms(10);
		if(tenMs == 20){ // 亮 200 ms，灭 200 ms
			LED0 = !LED0;
			tenMs= 0;
		}
	}
	
	// 写入完成以后，蜂鸣器响500ms后停止
	BEEP = 1;
	delay_ms(500);
	BEEP = 0;
	
	// LED0改为亮600ms、灭600ms模式
	while(1){
		tenMs ++;
		delay_ms(10);
		if(tenMs == 60){ // 亮 600 ms，灭 600 ms
			LED0 = !LED0;
			tenMs= 0;
		}
	}
}
 
void init(void){
	delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 	 //串口初始化为9600
 	LED_Init();			     //LED端口初始化
	BEEP_Init();	 
	KEY_Init();
 	AT24CXX_Init();			 //IIC初始化 
}
u8 read(void){
	// 根据收到命令 readOrder 的要求读数据至 buffer[] 中
	// 例子："读0(#2)x04地址开始的5(#16) BYTE"
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
	// 根据char writeOrder[128]中所写的命令，写入数据
	u8 orderlen = strlen(writeOrder);
	u8 i;
	u8 iBuffer = 0;
	u8 start = 0;
	for(i = 0; i < 3; i ++){ // 做一下 trim
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
	u8 sendLen = 0; // 待发送字符串的长度
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
	u8 sendLen = strlen(send1); // 待发送字符串的长度
	for(j = 0; j < sendLen; j ++){
		USART_SendData(USART1, send1[j]);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET){ 
		// haven't finished sending, do nothing
		}
	}
}

void sendByteData(u8 size){
	// 将buffer 中的数据表示成16进制字符，发送给计算机
	char temp[4];
	u8 i, j;
	for(i = 0; i < size; i ++){
		myItoaHex(temp, buffer[i]);
		for(j = 0; j < 4; j ++){
			USART_SendData(USART1, temp[j]);
			while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET){ 
			}
		}
		USART_SendData(USART1, ' '); //再发个空格
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET){ 
		}
	}
	USART_SendData(USART1, '\n');
}

u8 myAtoiHex(char* a, u8 start, u8 end){
	// 根据char a[start: end) 解析16进制字符
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
	// 根据char a[start: end) 解析10进制字符
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
	// 把 num 中的数转成两位 16 进制字符，0x??，写在 p 开头的位置
	char digits[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	*(p ++) = '0';
	*(p ++) = 'x';
	*(p ++) = digits[num / 16];
	*(p ++) = digits[num % 16];
}

