#include "STM32_WaveOutput.h"

/********�������Ҳ��������***********/
void SineWave_Data( u16 cycle ,u16 *D)
{
	u16 i;
	for( i=0;i<cycle;i++)
	{
		D[i]=(u16)((Um*sin(( 1.0*i/(cycle-1))*2*PI)+Um)*4095/3.3);
	}
}
/********���ɾ�ݲ��������***********/
void SawTooth_Data( u16 cycle ,u16 *D)
{
	u16 i;
	for( i=0;i<cycle;i++)
	{
		D[i]= (u16)(1.0*i/255*4095);
	}
}

/******************���Ҳ��α�***********************/
#ifdef  Sine_WaveOutput_Enable 	
     u16 SineWave_Value[256];		//���ú�������
#endif
/******************��ݲ��α�***********************/
#ifdef  SawTooth_WaveOutput_Enable
     u16 SawToothWave_Value[256];  //���ú�������
#endif	
	
/******DAC�Ĵ�����ַ����*******/	
#define DAC_DHR12R1    (u32)&(DAC->DHR12R1)   //DACͨ��1����Ĵ�����ַ
#define DAC_DHR12R2    (u32)&(DAC->DHR12R2)   //DACͨ��2����Ĵ�����ַ


/****************���ų�ʼ��******************/
void SineWave_GPIO_Config( u8 NewState1 ,u8 NewState2)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //��ʱ��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       //�������ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//�������	
  if( NewState1!=DISABLE)
	{
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4 ; //ѡ������
		GPIO_SetBits(GPIOA,GPIO_Pin_4)	;	//�������
	}
	if( NewState2!=DISABLE)
	{
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5 ; //ѡ������
		GPIO_SetBits(GPIOA,GPIO_Pin_5)	;	//�������
	}
	GPIO_Init(GPIOA, &GPIO_InitStructure);		//��ʼ��
}

/******************DAC��ʼ��*************************/
void SineWave_DAC_Config( u8 NewState1 ,u8 NewState2)
{
	DAC_InitTypeDef            DAC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);//��DACʱ��
	
  /**************DAC�ṹ��ʼ��������Ҫ�������޲���*******************/
	DAC_StructInit(&DAC_InitStructure);		
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;//����������
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable; //��ʹ���������
	if( NewState1!=DISABLE)
	{
		DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;//ѡ��DAC����ԴΪTIM2
	  DAC_Init(DAC_Channel_1, &DAC_InitStructure);//��ʼ��
	  DAC_Cmd(DAC_Channel_1, ENABLE);	   //ʹ��DACͨ��1
	  DAC_DMACmd(DAC_Channel_1, ENABLE); //ʹ��DACͨ��1��DMA
	}
	if( NewState2!=DISABLE)
	{
		DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;//ѡ��DAC����ԴΪTIM6
		DAC_Init(DAC_Channel_2, &DAC_InitStructure);//��ʼ��
		DAC_Cmd(DAC_Channel_2, ENABLE);	   //ʹ��DACͨ��2
		DAC_DMACmd(DAC_Channel_2, ENABLE); //ʹ��DACͨ��2��DMA
	}	
}
		 
/*********��ʱ������************/
void SineWave_TIM_Config( u32 Wave1_Fre ,u8 NewState1 ,u32 Wave2_Fre ,u8 NewState2)
{
	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	if( NewState1!=DISABLE)	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);//��ʱ��
	if( NewState2!=DISABLE)	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);//��ʱ��
	
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
 	TIM_TimeBaseStructure.TIM_Prescaler = 0x0;     //��Ԥ��Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0; //����Ƶ	
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//���ϼ���ģʽ
  if( NewState1!=DISABLE)
	{
			TIM_TimeBaseStructure.TIM_Period = Wave1_Fre;    
	    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
	    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);//����TIM2�������Ϊ����ģʽ
	}
	if( NewState2!=DISABLE)
	{
			TIM_TimeBaseStructure.TIM_Period = Wave2_Fre;     //�������Ƶ��       
	    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);  //��ʼ��
	    TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
	}
}

/*********DMA����***********/
void SineWave_DMA_Config( u16 *Wave1_Mem ,u8 NewState1 ,u16 *Wave2_Mem ,u8 NewState2)
{					
	DMA_InitTypeDef            DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);//��DMA2��ʱ��
	
	DMA_StructInit( &DMA_InitStructure);		//DMA�ṹ��ʼ��
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//�Ӵ洢��������
	DMA_InitStructure.DMA_BufferSize = 256;//�����С��һ��Ϊ256��
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//�����ַ������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	//�ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//���Ϊ����
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//���Ϊ����
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;//���ȼ����ǳ���
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//�ر��ڴ浽�ڴ�ģʽ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//ѭ������ģʽ 
	if( NewState1!=DISABLE)
	{
			DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R1;//�����ַΪDACͨ��1���ݼĴ���
			DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Wave1_Mem;//�ڴ��ַΪ���������������
			DMA_Init(DMA2_Channel3, &DMA_InitStructure);//��ʼ��
			DMA_Cmd(DMA2_Channel3, ENABLE);  //ʹ��DMAͨ��3       
	}
	if( NewState2!=DISABLE)
	{
			DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R2;//���������ַΪDACͨ��1���ݼĴ���
			DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Wave2_Mem;//�ڴ��ַΪ���������������
			DMA_Init(DMA2_Channel4, &DMA_InitStructure);//��ʼ��
			DMA_Cmd(DMA2_Channel4, ENABLE);	 //ʹ��DMAͨ��4
	}      
}

/***********���Ҳ���ʼ��***************/
void SineWave_Init(u8 Wave1,u16 Wave1_Fre,u8 NewState1,u8 Wave2,u16 Wave2_Fre,u8 NewState2)
{
  u16 *add1,*add2;
	u16 f1=(u16)(72000000/sizeof(SineWave_Value)*2/Wave1_Fre);
	u16 f2=(u16)(72000000/sizeof(SineWave_Value)*2/Wave2_Fre);
  SineWave_Data( N ,SineWave_Value);		//���ɲ��α�1
	SawTooth_Data( N ,SawToothWave_Value);//���ɲ��α�2
	if( NewState1!=DISABLE)
	{
			if( Wave1==0x00)   add1=SineWave_Value;
	    else							 add1=SawToothWave_Value;
	}
  if( NewState2!=DISABLE)
	{
		  if( Wave2==0x00)   add2=SineWave_Value;
	    else							 add2=SawToothWave_Value;
	}
	SineWave_GPIO_Config( ENABLE ,ENABLE);			  //��ʼ������ 
	SineWave_TIM_Config( f1 , NewState1 ,f2 ,NewState2);			  //��ʼ����ʱ�� 
	SineWave_DAC_Config(NewState1 ,NewState2);			  //��ʼ��DAC
	SineWave_DMA_Config( add1 ,NewState1 ,add2 ,NewState2);			  //��ʼ��DMA
  if( NewState1!=DISABLE)		TIM_Cmd(TIM2, ENABLE);			 //ʹ��TIM2,��ʼ��������  
  if( NewState2!=DISABLE)   TIM_Cmd(TIM6, ENABLE);			 //ʹ��TIM6,��ʼ��������  
}

void Set_WaveFre( u8 Wave_Channel ,u16 fre)
{
	TIM_TypeDef* TIMX;
	u16 reload;
	
	if( Wave_Channel==0x00)		TIMX = TIM2;
	else if(Wave_Channel==0x01)		TIMX = TIM6;
		
	reload=(u16)(72000000/512/fre);
	TIM_SetAutoreload( TIMX ,reload);
}

