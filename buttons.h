#pragma once
#include <stdbool.h>
#include "stdlib.h"

#define btn_delay 15

typedef struct 
{
	bool prevState;
	bool state;
	bool clicked;
	uint32_t lastChangeTime;
} keyState;

static volatile keyState keyStates[4] = {0}; 

static void resetBtns()
{
	for(int i=0;i<4;++i)
	{
		keyStates[i].lastChangeTime = 0;
		keyStates[i].state = false;
		keyStates[i].prevState = false;
		keyStates[i].clicked = false;
	}
}

static void trySet(volatile keyState* key, bool btnState, volatile uint32_t timestamp)
{
	if(timestamp > key->lastChangeTime + btn_delay)
	{
		key->prevState = key->state;
		key->state = btnState;
		key->clicked = key->state != key->prevState;
		key->lastChangeTime = timestamp;
	}
}


static void fetchButtons(volatile uint32_t timestamp)
{
	GPIOA->ODR &= ~GPIO_ODR_15;
	GPIOC->ODR &= ~GPIO_ODR_12;
	GPIOA->IDR &= ~GPIO_IDR_4;
	GPIOA->IDR &= ~GPIO_IDR_5;
	
	GPIOA->ODR |= GPIO_ODR_15;
	GPIOC->ODR &= ~GPIO_ODR_12;

  //wait
	//keys[0] = GPIOA->IDR & GPIO_IDR_4;
	//keys[1] = GPIOA->IDR & GPIO_IDR_5;
	
	trySet(&keyStates[0], GPIOA->IDR & GPIO_IDR_4, timestamp);
	trySet(&keyStates[1], GPIOA->IDR & GPIO_IDR_5, timestamp);
	
	GPIOA->ODR &= ~GPIO_ODR_15;
	GPIOC->ODR &= ~GPIO_ODR_12;
	GPIOA->IDR &= ~GPIO_IDR_4;
	GPIOA->IDR &= ~GPIO_IDR_5;
	
	GPIOA->ODR &= ~GPIO_ODR_15;
	GPIOC->ODR |= GPIO_ODR_12;

	//wait
	//keys[2] = GPIOA->IDR & GPIO_IDR_4;
	//keys[3] = GPIOA->IDR & GPIO_IDR_5;

	trySet(&keyStates[2], GPIOA->IDR & GPIO_IDR_4, timestamp);
	trySet(&keyStates[3], GPIOA->IDR & GPIO_IDR_5, timestamp);
	
  GPIOA->ODR &= ~GPIO_ODR_15;
	GPIOC->ODR &= ~GPIO_ODR_12;
	GPIOA->IDR &= ~GPIO_IDR_4;
	GPIOA->IDR &= ~GPIO_IDR_5;
}