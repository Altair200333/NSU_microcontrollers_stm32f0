#pragma once
#include <stm32f0xx.h>
#include <stdbool.h>
#include "renderAPI.h"

typedef struct _timer
{
	int counter; //20 ms
}Timer;
Timer timer;


void initializeTimer()
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
	TIM17->PSC = 3000 - 1;
	TIM17->ARR = 1;
	
	TIM17->CR1 |= TIM_CR1_ARPE | TIM_CR1_DIR | TIM_CR1_CEN; 
	TIM17->DIER |= TIM_DIER_UIE;
	
	
	NVIC_EnableIRQ(TIM17_IRQn);
	
	timer.counter = 0;
}

void TIM17_IRQHandler(void)
{
	timer.counter++;
  TIM17->SR &=(~TIM_SR_UIF);
}	