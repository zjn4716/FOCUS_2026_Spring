#include "stm32f10x.h"                  // Device header
#include "Delay.h"

#include "Serial.h"

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

uint8_t ParseSpeedValue(char *cmd)
{
    // ??"SPEED "??,????????
    char *num_start = cmd + 6;
    while(*num_start == ' ') num_start++;  // ????
    
    if(*num_start < '0' || *num_start > '9')
        return 0xFF;  // ????
    
    // ????
    uint8_t speed = 0;
    while(*num_start >= '0' && *num_start <= '9')
    {
        speed = speed * 10 + (*num_start - '0');
        num_start++;
    }
    
    // ??????????(????????)
    while(*num_start == ' ') num_start++;
    if(*num_start != '\0')
        return 0xFF;  // ?????
    
    return speed;
}
void  Motor_PWM(int target_speed ){
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
}



void ProcessCommand(void)
{
    char *cmd = Serial_GetRxBuf();
    
    // ??????
    while(*cmd == ' ') cmd++;
    
    // ?????ON??
    if(strncmp(cmd, "ON", 2) == 0 || strncmp(cmd, "on", 2) == 0)
    {
        // ????,???????
        if(current_speed == 0)
        {
            Motor_PWM_Control(30);  // ??20%
					Serial_SendString("OK 30\r\n");
        }
        else
        {
            Motor_PWM_Control(current_speed);
					        Serial_SendString("OK PWM\r\n");
        }

    }
    // ?????OFF??
    else if(strncmp(cmd, "OFF", 3) == 0 || strncmp(cmd, "off", 3) == 0)
    {
        Motor_Stop();
        Serial_SendString("OK\r\n");
    }
    // ?????SPEED??
    else if(strncmp(cmd, "SPEED", 5) == 0 || strncmp(cmd, "speed", 5) == 0)
    {
        uint8_t speed = ParseSpeedValue(cmd);
        
        if(speed == 0xFF || speed > 100)
        {
            Serial_SendString("ERROR\r\n");
        }
        else
        {
            Motor_PWM_Control(speed);
            Serial_SendString("OK\r\n");
        }
    }
    // ????
    else
    {
        Serial_SendString("ERROR\r\n");
    }
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
int main(void)
{
	//OLED_Init();
	//OLED_ShowString(1, 1, "RxData:");
	 GPIO_Config();
    

    PWM_Config();
    
  
    TIM_Config();
    
   
    Motor_Stop();
	Serial_Init();
	
	while (1)
	{
		/*if (Serial_GetRxFlag() == 1)
		{
			RxData = Serial_GetRxData();
			Serial_SendByte(RxData);
		}*/
		
		
		if(Serial_GetComplete())
        {
            ProcessCommand();
					Serial_ClearRxBuf();
        }
				Delay_ms(20);
	}
}


void GPIO_Config(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // ??IN1?IN2?????
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = MOTOR_IN1_PIN | MOTOR_IN2_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(MOTOR_PORT, &GPIO_InitStructure);
    
    // ???????
    GPIO_ResetBits(MOTOR_PORT, MOTOR_IN1_PIN | MOTOR_IN2_PIN);
}

void PWM_Config(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;     
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;           
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
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
    // SysTick??,????
    if(SysTick_Config(SystemCoreClock / 1000))
    {
        while(1);  
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
        
        // ??????(????)
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
}
