#include "stm32f10x.h"
#include "led.h"
#include "delay.h"
#include "key.h"
#include "beep.h"
#include "timer.h"
#include "sys.h"
#include "usart.h"
#include "usmart.h"	 
#include "string.h"
#include "STM32_WaveOutput.h"
#include "dac.h"
char send0[][128] = {
	" I2C����ʵ��-AT24C02\r\n", 
	"������ �����h ���\r\n",
	"18 05 17 16 30 30\r\n"};
char send1[128] = {"������Ŀ���ַ������\r\n"};
void sendSend0(void);
void sendSend1(void);
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


void initial(void){
    delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(9600);	 	 //���ڳ�ʼ��Ϊ9600
 	LED_Init();			     //LED�˿ڳ�ʼ��
	BEEP_Init();	 
	KEY_Init();
}	
void TriangleWave_Init(void)
{
Dac1_Init();
TIM2_Int_Init();
}
int main(void)
 {	 
	//u16 vol=1;
	int hms=0;// ���ڿ��Ʋ�ͬ��������Ĳ���������ʱ100ms
	initial();
	while(1)
		{	//�ϵ��LED0 ������200ms����200ms������ģʽ
			hms ++;
			delay_ms(100);
			if(!LED0 && hms == 2)
				{ // �� 200 ms
					LED0 = 1;
					hms= 0;
				}
			else if(LED0 && hms == 2)
				{ // �� 500 ms
					LED0 = 0;
					hms= 0;
				}
			if( KEY0==0 )
				{ // ��KEY1
					sendSend0();//���������������ַ�
					delay_ms(200); 
				}
			
			if( KEY1 == 0) //�������ǲ�
			{
			DMA_Cmd(DMA2_Channel3, DISABLE);//sin���ε�ͨ��3��4��ʹ��
			DMA_Cmd(DMA2_Channel4, DISABLE);
			TriangleWave_Init();
			
			}
			if( KEY2 ==0)//�������Ҳ�
			{
				SineWave_Init( SinWave,200,ENABLE ,SawToothWave,200 ,ENABLE);//PA4���200Hz�����ǲ���PA5���Ϊ10Hz�����Ҳ�����ֵ��Ϊ0~3.6V�����ʱΪ3.6��һ���ȶ���3.4V���ң�
			}
			if( KEY3==1)//�������Ҳ�
			{
				SineWave_Init( SinWave,100,ENABLE ,SawToothWave,100 ,ENABLE);//PA4���200Hz�����ǲ���PA5���Ϊ10Hz�����Ҳ�����ֵ��Ϊ0~3.6V�����ʱΪ3.6��һ���ȶ���3.4V���ң�
			}
	}
 }
