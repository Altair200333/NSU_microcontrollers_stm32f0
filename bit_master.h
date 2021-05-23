#pragma once
#include <stdlib.h>
#include <stm32f0xx.h>

uint8_t getBit(uint8_t pos, uint8_t src)
{
	return (src & (1 << pos)) >> pos;
}
void setBit(uint8_t pos, uint8_t* dst)
{
	*dst |= 1UL << pos;
}