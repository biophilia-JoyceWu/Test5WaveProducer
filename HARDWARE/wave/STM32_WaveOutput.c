#include "STM32_WaveOutput.h"

/********生成正弦波形输出表***********/
void SineWave_Data( u16 cycle ,u16 *D)
{
	u16 i;
	for( i=0;i<cycle;i++)
	{
		D[i]=(u16)((Um*sin(( 1.0*i/(cycle-1))*2*PI)+Um)*4095/3.3);
	}
}
/********生成锯齿波形输出表***********/
void SawTooth_Data( u16 cycle ,u16 *D)
{
	u16 i;
	for( i=0;i<cycle;i++)
	{
		D[i]= (u16)(1.0*i/255*4095);
	}
}

/******************正弦波形表***********************/
#ifdef  Sine_WaveOutput_Enable 	
     u16 SineWave_Value[256];		//已用函数代替
#endif
/******************锯齿波形表***********************/
#ifdef  SawTooth_WaveOutput_Enable
     u16 SawToothWave_Value[256];  //已用函数代替
#endif	
	
/******DAC寄存器地址声明*******/	
#define DAC_DHR12R1    (u32)&(DAC->DHR12R1)   //DAC通道1输出寄存器地址
#define DAC_DHR12R2    (u32)&(DAC->DHR12R2)   //DAC通道2输出寄存器地址


/****************引脚初始化******************/
void SineWave_GPIO_Config( u8 NewState1 ,u8 NewState2)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //开时钟
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       //推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//输出速率	
  if( NewState1!=DISABLE)
	{
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4 ; //选择引脚
		GPIO_SetBits(GPIOA,GPIO_Pin_4)	;	//拉高输出
	}
	if( NewState2!=DISABLE)
	{
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5 ; //选择引脚
		GPIO_SetBits(GPIOA,GPIO_Pin_5)	;	//拉高输出
	}
	GPIO_Init(GPIOA, &GPIO_InitStructure);		//初始化
}

/******************DAC初始化*************************/
void SineWave_DAC_Config( u8 NewState1 ,u8 NewState2)
{
	DAC_InitTypeDef            DAC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);//开DAC时钟
	
  /**************DAC结构初始化，很重要，否则无波形*******************/
	DAC_StructInit(&DAC_InitStructure);		
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;//不产生波形
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable; //不使能输出缓存
	if( NewState1!=DISABLE)
	{
		DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;//选择DAC触发源为TIM2
	  DAC_Init(DAC_Channel_1, &DAC_InitStructure);//初始化
	  DAC_Cmd(DAC_Channel_1, ENABLE);	   //使能DAC通道1
	  DAC_DMACmd(DAC_Channel_1, ENABLE); //使能DAC通道1的DMA
	}
	if( NewState2!=DISABLE)
	{
		DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;//选择DAC触发源为TIM6
		DAC_Init(DAC_Channel_2, &DAC_InitStructure);//初始化
		DAC_Cmd(DAC_Channel_2, ENABLE);	   //使能DAC通道2
		DAC_DMACmd(DAC_Channel_2, ENABLE); //使能DAC通道2的DMA
	}	
}
		 
/*********定时器配置************/
void SineWave_TIM_Config( u32 Wave1_Fre ,u8 NewState1 ,u32 Wave2_Fre ,u8 NewState2)
{
	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	if( NewState1!=DISABLE)	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);//开时钟
	if( NewState2!=DISABLE)	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);//开时钟
	
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
 	TIM_TimeBaseStructure.TIM_Prescaler = 0x0;     //不预分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0; //不分频	
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
  if( NewState1!=DISABLE)
	{
			TIM_TimeBaseStructure.TIM_Period = Wave1_Fre;    
	    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
	    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);//设置TIM2输出触发为更新模式
	}
	if( NewState2!=DISABLE)
	{
			TIM_TimeBaseStructure.TIM_Period = Wave2_Fre;     //设置输出频率       
	    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);  //初始化
	    TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
	}
}

/*********DMA配置***********/
void SineWave_DMA_Config( u16 *Wave1_Mem ,u8 NewState1 ,u16 *Wave2_Mem ,u8 NewState2)
{					
	DMA_InitTypeDef            DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);//开DMA2的时钟
	
	DMA_StructInit( &DMA_InitStructure);		//DMA结构初始化
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//从存储器读数据
	DMA_InitStructure.DMA_BufferSize = 256;//缓存大小，一般为256点
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	//内存地址递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//宽度为半字
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//宽度为半字
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;//优先级：非常高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//关闭内存到内存模式
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//循环发送模式 
	if( NewState1!=DISABLE)
	{
			DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R1;//外设地址为DAC通道1数据寄存器
			DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Wave1_Mem;//内存地址为输出波形数据数组
			DMA_Init(DMA2_Channel3, &DMA_InitStructure);//初始化
			DMA_Cmd(DMA2_Channel3, ENABLE);  //使能DMA通道3       
	}
	if( NewState2!=DISABLE)
	{
			DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R2;//设置外设地址为DAC通道1数据寄存器
			DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Wave2_Mem;//内存地址为输出波形数据数组
			DMA_Init(DMA2_Channel4, &DMA_InitStructure);//初始化
			DMA_Cmd(DMA2_Channel4, ENABLE);	 //使能DMA通道4
	}      
}

/***********正弦波初始化***************/
void SineWave_Init(u8 Wave1,u16 Wave1_Fre,u8 NewState1,u8 Wave2,u16 Wave2_Fre,u8 NewState2)
{
  u16 *add1,*add2;
	u16 f1=(u16)(72000000/sizeof(SineWave_Value)*2/Wave1_Fre);
	u16 f2=(u16)(72000000/sizeof(SineWave_Value)*2/Wave2_Fre);
  SineWave_Data( N ,SineWave_Value);		//生成波形表1
	SawTooth_Data( N ,SawToothWave_Value);//生成波形表2
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
	SineWave_GPIO_Config( ENABLE ,ENABLE);			  //初始化引脚 
	SineWave_TIM_Config( f1 , NewState1 ,f2 ,NewState2);			  //初始化定时器 
	SineWave_DAC_Config(NewState1 ,NewState2);			  //初始化DAC
	SineWave_DMA_Config( add1 ,NewState1 ,add2 ,NewState2);			  //初始化DMA
  if( NewState1!=DISABLE)		TIM_Cmd(TIM2, ENABLE);			 //使能TIM2,开始产生波形  
  if( NewState2!=DISABLE)   TIM_Cmd(TIM6, ENABLE);			 //使能TIM6,开始产生波形  
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

