#include "stm32f10x.h"                  // Device header
#include "Delay.h"
/*
int main(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	int flag =0;
	while (1)
	{
		
		//GPIO_ResetBits(GPIOA, GPIO_Pin_0);
	//	Delay_ms(500);
	//	GPIO_SetBits(GPIOA, GPIO_Pin_0);
		//Delay_ms(500);
		
		
		GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
		
		GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET);
		/*
		
		if(flag)
		{
			GPIO_WriteBit(GPIOA, GPIO_Pin_6, (BitAction)flag);
			flag=0;
		}
		else{
			GPIO_WriteBit(GPIOA, GPIO_Pin_6, (BitAction)flag);
			flag =1;
		}
		Delay_ms(1000);
		//Delay_ms(500);
		
		//GPIO_WriteBit(GPIOA, GPIO_Pin_6, (BitAction)1);
		//Delay_ms(500);
		//GPIO_WriteBit(GPIOA, GPIO_Pin_0, (BitAction)1);
	//	Delay_ms(500);
	
		}
}

*/

#include "stm32f10x.h"


// ????????
#define MOTOR_IN1_PIN    GPIO_Pin_0   // PA0
#define MOTOR_IN2_PIN    GPIO_Pin_1   // PA1
#define MOTOR_PORT       GPIOA

// PWM????
#define PWM_TIMER        TIM2
#define PWM_CHANNEL      TIM_Channel_1  // PA0??PWM??
#define PWM_PERIOD       999            // PWM?? = (999+1)/72MHz = 13.9us (72kHz)
#define PWM_CLOCK        72             // ????72MHz

// ????
volatile uint8_t pwm_auto_mode = 1;     // PWM????????
volatile uint8_t current_speed = 0;     // ????(0-100)
volatile uint8_t target_speed = 0;      // ????
volatile uint8_t speed_direction = 1;   // ??????: 1=??, 0=??
volatile uint32_t speed_change_time = 0; // ????????

// ????
void GPIO_Config(void);
void PWM_Config(void);
void TIM_Config(void);
void Motor_GPIO_Control(uint8_t speed);
void Motor_PWM_Control(uint8_t speed);
void Motor_Stop(void);
void Delay_ms(uint32_t time);
void PWM_Auto_Speed_Control(void);

int main(void)
{

    SystemInit();
    

    GPIO_Config();
    

    PWM_Config();
    
  
    TIM_Config();
    
   
    Motor_Stop();
    
    while(1)
    {
     
        if(pwm_auto_mode)
        {
            PWM_Auto_Speed_Control();
        }
        

        Delay_ms(10);
    }
}


void GPIO_Config(void)
{
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = MOTOR_IN1_PIN | MOTOR_IN2_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(MOTOR_PORT, &GPIO_InitStructure);
    
   
    GPIO_ResetBits(MOTOR_PORT, MOTOR_IN1_PIN | MOTOR_IN2_PIN);
}


void PWM_Config(void)
{
  
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
  
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;     
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;           // PA0
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // 50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    

    TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD;                    
    TIM_TimeBaseStructure.TIM_Prescaler = (PWM_CLOCK - 1);            
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;           
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;    
    TIM_TimeBaseInit(PWM_TIMER, &TIM_TimeBaseStructure);
    
  
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;                 
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;     
    TIM_OCInitStructure.TIM_Pulse = 0;                                
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;         
    
    
    TIM_OC1Init(PWM_TIMER, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(PWM_TIMER, TIM_OCPreload_Enable);            
    

    TIM_ARRPreloadConfig(PWM_TIMER, ENABLE);
    
    
    TIM_Cmd(PWM_TIMER, ENABLE);
    
   
    TIM_CtrlPWMOutputs(PWM_TIMER, ENABLE);
}


void TIM_Config(void)
{
 
    // SysTick = 72MHz / 8 = 9MHz

    if(SysTick_Config(SystemCoreClock / 1000))
    {
        while(1);  
    }
}


void Motor_GPIO_Control(uint8_t speed)
{
    if(speed > 0)
    {

        GPIO_SetBits(MOTOR_PORT, MOTOR_IN1_PIN);
        GPIO_ResetBits(MOTOR_PORT, MOTOR_IN2_PIN);
    }
    else
    {
 
        GPIO_ResetBits(MOTOR_PORT, MOTOR_IN1_PIN);
        GPIO_ResetBits(MOTOR_PORT, MOTOR_IN2_PIN);
    }
}


void Motor_PWM_Control(uint8_t speed)
{
    if(speed > 100) speed = 100;
    current_speed = speed;
    
    if(speed > 0)
    {

        uint32_t compare = (speed * PWM_PERIOD) / 100;
        TIM_SetCompare1(PWM_TIMER, compare);
        
   
        GPIO_SetBits(MOTOR_PORT, MOTOR_IN1_PIN);
        GPIO_ResetBits(MOTOR_PORT, MOTOR_IN2_PIN);
    }
    else
    {

        TIM_SetCompare1(PWM_TIMER, 0);
        GPIO_ResetBits(MOTOR_PORT, MOTOR_IN1_PIN);
        GPIO_ResetBits(MOTOR_PORT, MOTOR_IN2_PIN);
    }
}


void Motor_Stop(void)
{
 
    TIM_SetCompare1(PWM_TIMER, 0);
    
 
    GPIO_ResetBits(MOTOR_PORT, MOTOR_IN1_PIN);
    GPIO_ResetBits(MOTOR_PORT, MOTOR_IN2_PIN);
    
    current_speed = 0;
    target_speed = 0;
}


void PWM_Auto_Speed_Control(void)
{

        
        // 20%  50%  80% 50% 20%
        static const uint8_t speed_steps[] = {20, 50, 80, 50, 20};
        static uint8_t step_index = 0;
        
        target_speed = speed_steps[step_index];
        step_index++;
        
   
        if(step_index >= sizeof(speed_steps))
        {
            step_index = 0;
        }
        
  
        if(target_speed > current_speed)
        {
          
            for(uint8_t s = current_speed + 1; s <= target_speed; s++)
            {
                Motor_PWM_Control(s);
                Delay_ms(50);  
            }
        }
        else if(target_speed < current_speed)
        {
        
            for(uint8_t s = current_speed - 1; s >= target_speed; s--)
            {
                Motor_PWM_Control(s);
                Delay_ms(50);
            }
        }
        else
        {
            Motor_PWM_Control(target_speed);
        }
        
    
}



