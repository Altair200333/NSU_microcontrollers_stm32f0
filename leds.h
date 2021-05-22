#pragma once
#include <stdlib.h>

#define LED GPIOC->ODR
#define Rled GPIO_ODR_6
#define Oled GPIO_ODR_8
#define Bled GPIO_ODR_7
#define Gled GPIO_ODR_9


void setBitV(volatile uint32_t* bit, uint32_t value)
{
	*bit |= value;
}
void resetBitV(volatile uint32_t* bit, uint32_t value)
{
	*bit &= ~value;
}
