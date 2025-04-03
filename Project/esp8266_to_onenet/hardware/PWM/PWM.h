#ifndef __PWM_H
#define __PWM_H

void PWM_Init(void);
void PWM_SetCompare3(uint16_t Compare);
uint16_t PWM_TIM_GetCapture1(void);
#endif
