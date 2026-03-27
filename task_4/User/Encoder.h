#ifndef __ENCODER_H
#define __ENCODER_H
#define ENCODER_TIM_PERIOD       (uint16_t)(65535)
void Encoder_Init(void);
int16_t Encoder_Get(void);

#endif
