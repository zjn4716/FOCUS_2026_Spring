#include "stm32f10x.h"                  // Device header
#include "Encoder.h"
void Encoder_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //'??????4?????

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			  //'???????
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;			  //???????
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //????????
	GPIO_Init(GPIOA, &GPIO_InitStructure);		  //????????????'??GPIO

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			  //'???????
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;			  //???????
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //????????
	GPIO_Init(GPIOA, &GPIO_InitStructure);		  //????????????'??GPIO

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0x0;					// ??????
	TIM_TimeBaseStructure.TIM_Period = ENCODER_TIM_PERIOD;		//???????????????
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//???????????????
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM???????
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising); //'?ñ?????g?3
	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_ICFilter = 10;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);                   //???TIM????±???
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	//Reset counter
	TIM_SetCounter(TIM3, 0);
	TIM_Cmd(TIM3, ENABLE);
}



int16_t Encoder_Get(void)
{
	int16_t Encoder_TIM = 0;
	Encoder_TIM = (short)TIM3->CNT;
	TIM3->CNT=0;
	return Encoder_TIM;
}
