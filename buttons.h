#pragma once
#include <stdbool.h>
#include "stdlib.h"
#include <stm32f0xx.h>

#define btn_delay 15


#define Key_Up 2
#define Key_Down 3
#define Key_Right 1
#define Key_Left 0
#define Usr_Btn 4

typedef struct 
{
	bool prevState;
	bool state;
	bool clicked;
	uint32_t lastChangeTime;
} keyState;

static volatile keyState keyStates[5] = {0}; 

static void resetBtns()
{
	for(int i=0;i<5;++i)
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


bool buttonDown()
{
	return GPIOA->IDR & GPIO_IDR_0;
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
	
	trySet(&keyStates[4], buttonDown(), timestamp);

}
void initBtns()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOAEN;	//PC6 - LED
	
	GPIOC->MODER &= ~GPIO_MODER_MODER0;
	GPIOC->MODER |= GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0;//enable ports
	
	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT_8 | ~GPIO_OTYPER_OT_9);
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPDR8 | GPIO_PUPDR_PUPDR9);
	//--------

	GPIOA->MODER |= GPIO_MODER_MODER15_0;
	GPIOA->MODER &= ~GPIO_MODER_MODER4;
	GPIOA->MODER &= ~GPIO_MODER_MODER5;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR4_1 | GPIO_PUPDR_PUPDR5_1;
	GPIOC->MODER |= GPIO_MODER_MODER12_0;
}