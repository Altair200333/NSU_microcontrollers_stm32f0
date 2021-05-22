#pragma once
#include "stm32f0xx.h"
#include "renderAPI.h"
#include <math.h>
//idle value = 145(5-7)
//min  value = 1455
static volatile uint32_t rawValue = 0;
int group = 1;
#define b1111 15
#define b11 3
#define b111 7
#define b101 5
#define b110 6
#define b11111111 255

typedef struct{
uint32_t   s[3];
uint8_t      i;
uint8_t      ready;
}TSC_RESULT;

TSC_RESULT Result;

void ResetSensors(TSC_RESULT *pResult){
uint8_t   i;

for(i=0;i<3; i++) pResult->s[i] = 0xFFFF;

pResult->i = 0;
pResult->ready=0;
}

void ReadSensors(TSC_RESULT *pResult){

ResetSensors(pResult); // ????????? ???? ?????????
TSC->IOGCSR &= ~b11111111; // ????????? ???? ?????
TSC->IOGCSR |= TSC_IOGCSR_G2E; // ????????? ?????? G1
TSC->IOCCR &= ~b1111; // ????????? ???? ??????? ??????
TSC->IOCCR |= TSC_IOCCR_G2_IO1; // ????????? ??????? ?????? ?????? = PA1
__enable_irq ();    // ???????? ?????????? ??????????
	
	NVIC_EnableIRQ(TSC_IRQn);//bind interruput
	TSC->CR |= TSC_CR_START;
}

void TSC_IRQHandler(void)
{
	if (TSC -> ISR & TSC_ISR_MCEF) { // ????? ????? ??????
    TSC -> ICR |= (TSC_ICR_EOAIC | TSC_ICR_MCEIC);
    // ????? ???? ??????, ??? ????????? ??????
    Result.s[0] = Result.s[1] = Result.s[2] = 0;
    Result.i = 0;
    Result.ready = 1;
    // ??????? ??? ???-?? ...
    // ...

		drawSpiPos(6,6);
    return;
  }
	// 2. ??? ???????? ??????????
  TSC -> ICR |= (TSC_ICR_EOAIC); // ????? ?????
  switch (Result.i)
	{
  case 0: // ????????? ?????? PA4
    Result.s[0] = TSC -> IOGXCR[1]; // ?????????? ????????
    TSC -> IOCCR &= ~b1111; // ?????????? ???? ??????? ??????
    TSC -> IOCCR |= TSC_IOCCR_G2_IO2; // ????????? ?????????? (PA5)
    break;
  case 1: // ????????? ?????? PA2
    Result.s[1] = TSC -> IOGXCR[1]; // ?????????? ????????
    TSC -> IOCCR &= ~b1111; // ?????????? ???? ??????? ??????
    TSC -> IOCCR |= TSC_IOCCR_G2_IO3; // ????????? ?????????? (PA6)
    break;
  case 2: // ????????? ?????? PA3
    Result.s[2] = TSC -> IOGXCR[1]; // ?????????? ????????
    TSC -> IOCCR &= ~b1111; // ?????????? ???? ??????? ??????
    break;
  }
  Result.i++;
  if (Result.i < 3) {
    // ?????? ?????????? ??????
    TSC -> CR |= TSC_CR_START;
    Result.ready = 0;
  } else {
    Result.ready = 1;
		drawSpiPos(0, Result.s[0]%8);
	  drawSpiPos(1, Result.s[1]%8);
	  drawSpiPos(2, Result.s[2]%8);
  }
	drawSpiPos(3, Result.i%8);
	
}

void lateTSCinit()
{
	TSC->IER |= TSC_IER_EOAIE | TSC_IER_MCEIE; //enable end of acquisition interrupt
	
	NVIC_EnableIRQ(TSC_IRQn);//bind interruput
	
	//TSC->CR |= TSC_CR_PGPSC_2 | TSC_CR_PGPSC_0 | TSC_CR_CTPH_0 | TSC_CR_CTPL_0 | TSC_CR_MCV_2 | TSC_CR_MCV_1 | TSC_CR_TSCE;//TSC_CR_PGPSC_2 | TSC_CR_PGPSC_0 | TSC_CR_CTPH_0 | TSC_CR_CTPL_0 | TSC_CR_MCV_2 | TSC_CR_MCV_1 | TSC_CR_TSCE;//pure magic lol
	TSC->CR = (b111 << TSC_CR_CTPH_Pos)   // Charge transfer pulse high [3:0] = t_PGCLK * "?32"
      | (0 << TSC_CR_CTPL_Pos)         // Charge transfer pulse low [3:0] = t_PGCLK * "x1"
      | (0 << TSC_CR_SSD_Pos)            // Spread spectrum deviation [6:0] = "x1" * t_SSCLK
      | (0 << TSC_CR_SSE_Pos)            // Spread spectrum enable = 0: DISABLE / 1: ENABLE
      | (1 << TSC_CR_SSPSC_Pos)         // Spread spectrum prescaler = 0: f_HCLK / 1: 0.5 * f_HCLK
      | (b101 << TSC_CR_PGPSC_Pos)   // Pulse generator prescaler [2:0] (f_HCLK/1... f_HCLK/128), t_PGCLK = b101 = /32
      | (b110 << TSC_CR_MCV_Pos)      // Max count value [2:0] = datasheet RM0091 -> p.311/1004, b110 = 16383
      | (0 << TSC_CR_IODEF_Pos)         // I/O Default mode: 0: output push-pull LOW / 1: input floating
      | (0 << TSC_CR_SYNCPOL_Pos)      // Synchronization pin polarity
      | (0 << TSC_CR_AM_Pos)            // Acquisition mode: 0: Normal acquisition mode / 1: Synchronized acquisition mode
      | (0 << TSC_CR_START_Pos)         // Start a new acquisition: 0 (hard): Acquisition not started / 1 (soft): Start a new acquisition
      | (1 << TSC_CR_TSCE_Pos);         // Touch sensing controller enable: 0: disabled / 1: enabled
	//TSC->CR |= TSC_CR_START;//(0x01 + 0x00C0 + 0x02); //enable TSC and start acquisition
	//TSC->CR |= TSC_CR_AM;

}
//GPIOA->AFR[0] |= 3 << (0) * 4;
//alt.func. number ^;pin.^;   ^ - always the same
void touch_init(void) 
{
	//PA7  PA6
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	
	GPIOA->MODER &= ~(GPIO_MODER_MODER4_Msk | GPIO_MODER_MODER5_Msk | GPIO_MODER_MODER6_Msk | GPIO_MODER_MODER7_Msk);
	GPIOA->MODER |=  GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;
	
	
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL4 | GPIO_AFRL_AFSEL5 | GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7);
	GPIOA->AFR[0] |= (b11 << GPIO_AFRL_AFSEL4_Pos) | (b11 << GPIO_AFRL_AFSEL5_Pos) | (b11 << GPIO_AFRL_AFSEL6_Pos) | (b11 << GPIO_AFRL_AFSEL7_Pos);

	RCC->AHBENR |= RCC_AHBENR_TSEN;//enable TS clock
	
	//TSC->IOCCR |= TSC_IOCCR_G2_IO1; //enable G1_IO3 as channel
	
	TSC->IOGCSR |= TSC_IOGCSR_G2E; //enable G2 analog group
	
	TSC->IOHCR &= (uint32_t)(~(TSC_IOHCR_G2_IO4 | TSC_IOHCR_G2_IO3 | TSC_IOHCR_G2_IO2 | TSC_IOHCR_G2_IO1)); //disable hysteresis on PA6 and PA7
	
	GPIOA->OTYPER |= (GPIO_OTYPER_OT_7); // OD
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_5 | GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_4); // PP
	
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR4 | GPIO_PUPDR_PUPDR5 | GPIO_PUPDR_PUPDR6 | GPIO_PUPDR_PUPDR7);

	//GPIOA->OSPEEDR |= (b11 << GPIO_OSPEEDR_OSPEEDR0_Pos) | (b11 << GPIO_OSPEEDR_OSPEEDR1_Pos) | (b11 << GPIO_OSPEEDR_OSPEEDR2_Pos) | (b11 << GPIO_OSPEEDR_OSPEEDR3_Pos);

	lateTSCinit();
	TSC->IOSCR |= TSC_IOSCR_G2_IO4; //enable G1_IO4 as sampling capacitor

}