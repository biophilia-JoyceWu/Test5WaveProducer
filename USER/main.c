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
	" I2C总线实验-AT24C02\r\n", 
	"吴熠玮 董梓玥 组合\r\n",
	"18 05 17 16 30 30\r\n"};
char send1[128] = {"请输入目标地址和数据\r\n"};
void sendSend0(void);
void sendSend1(void);
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


void initial(void){
    delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 	 //串口初始化为9600
 	LED_Init();			     //LED端口初始化
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
	int hms=0;// 用于控制不同周期亮灭的参数，可延时100ms
	initial();
	while(1)
		{	//上电后，LED0 保持亮200ms、灭200ms的运行模式
			hms ++;
			delay_ms(100);
			if(!LED0 && hms == 2)
				{ // 亮 200 ms
					LED0 = 1;
					hms= 0;
				}
			else if(LED0 && hms == 2)
				{ // 灭 500 ms
					LED0 = 0;
					hms= 0;
				}
			if( KEY0==0 )
				{ // 按KEY1
					sendSend0();//向计算机发送三行字符
					delay_ms(200); 
				}
			
			if( KEY1 == 0) //生成三角波
			{
			DMA_Cmd(DMA2_Channel3, DISABLE);//sin波形的通道3、4不使能
			DMA_Cmd(DMA2_Channel4, DISABLE);
			TriangleWave_Init();
			
			}
			if( KEY2 ==0)//生成正弦波
			{
				SineWave_Init( SinWave,200,ENABLE ,SawToothWave,200 ,ENABLE);//PA4输出200Hz的三角波；PA5输出为10Hz的正弦波；幅值均为0~3.6V（最大时为3.6，一般稳定在3.4V左右）
			}
			if( KEY3==1)//生成正弦波
			{
				SineWave_Init( SinWave,100,ENABLE ,SawToothWave,100 ,ENABLE);//PA4输出200Hz的三角波；PA5输出为10Hz的正弦波；幅值均为0~3.6V（最大时为3.6，一般稳定在3.4V左右）
			}
	}
 }
