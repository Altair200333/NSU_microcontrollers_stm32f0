#pragma once
#include "stm32f0xx.h"
#include "renderAPI.h"
#include <math.h>
//idle value = 145(5-7)
static volatile uint32_t rawValue = 0;
void TSC_IRQHandler(void)
{
	if ((TSC->ISR & TSC_ISR_EOAF) == TSC_ISR_EOAF)
	{	
		TSC->ICR = TSC_ICR_EOAIC; /* Clear flag */
		
	  uint32_t AcquisitionValue = TSC->IOGXCR[1]; /* Get G2 counter value */
		rawValue = AcquisitionValue;
		int n = AcquisitionValue;
		int i = 0;
		while (n != 0) {
			int tmp = n%10;
			drawSpiPos(i,tmp);

			n /= 10;
			++i;
		}

		TSC->CR |= TSC_CR_START;
	}
}
void lateTSCinit()
{
	TSC->IER |= TSC_IER_EOAIE; //enable end of acquisition interrupt
	
	NVIC_EnableIRQ(TSC_IRQn);//bind interruput
	
	TSC->CR |= TSC_CR_PGPSC_2 | TSC_CR_PGPSC_0 | TSC_CR_CTPH_0 | TSC_CR_CTPL_0 | TSC_CR_MCV_2 | TSC_CR_MCV_1 | TSC_CR_TSCE;//TSC_CR_PGPSC_2 | TSC_CR_PGPSC_0 | TSC_CR_CTPH_0 | TSC_CR_CTPL_0 | TSC_CR_MCV_2 | TSC_CR_MCV_1 | TSC_CR_TSCE;//pure magic lol
	TSC->CR |= TSC_CR_START;//(0x01 + 0x00C0 + 0x02); //enable TSC and start acquisition
}

void initPins(int pin1, int pin2)
{
	uint32_t gio1 = 0, gio1_h = 0;
	uint32_t gio2 = 0, gio2_h = 0;
	
	GPIOA->MODER |= (0x2 << (2*pin1 - (pin1>=8?8:0))) + (0x2 << (2*pin2 - (pin2>=8?8:0))); //enable AF for pin1 and pin2
	
	GPIOA->AFR[pin1>=8?1:0] |= (0x3 << (4*pin1 - (pin1>=8?8:0))) + (0x3 << (4*pin2 - (pin2>=8?8:0))); //enable AF3 for PA7 and PA7
	GPIOA->OTYPER |= (1<<3);//nah whatewer i dont care
	
	RCC->AHBENR |= RCC_AHBENR_TSEN;//enable TS clock

	TSC->IOSCR |= gio1; //enable G1_IO4 as sampling capacitor
	TSC->IOCCR |= gio2; //enable G1_IO3 as channel
	
	TSC->IOGCSR |= TSC_IOGCSR_G2E; //enable G2 analog group
	
	TSC->IOHCR &= (uint32_t)(~(TSC_IOHCR_G2_IO4 | TSC_IOHCR_G2_IO3)); //disable hysteresis on PA6 and PA7
}
//GPIOA->AFR[0] |= 3 << (0) * 4;
//alt.func. number ^;pin.^;   ^ - always the same
void touch_init(void) 
{
	//PA7  PA6
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;
	
	GPIOA->MODER |= (0x2 << (2*6)) + (0x2 << (2*7)); //enable AF for PA6 and PA7
	
	
	GPIOA->AFR[0] |= (0x3 << (4*6)) + (0x3 << (4*7)); //enable AF3 for PA7 and PA7
	GPIOA->OTYPER |= (1<<3);//nah whatewer i dont care

	RCC->AHBENR |= RCC_AHBENR_TSEN;//enable TS clock
	
	TSC->IOSCR |= TSC_IOSCR_G2_IO4; //enable G1_IO4 as sampling capacitor
	TSC->IOCCR |= TSC_IOCCR_G2_IO3; //enable G1_IO3 as channel
	
	TSC->IOGCSR |= TSC_IOGCSR_G2E; //enable G2 analog group
	
	TSC->IOHCR &= (uint32_t)(~(TSC_IOHCR_G2_IO4 | TSC_IOHCR_G2_IO3)); //disable hysteresis on PA6 and PA7
		
	
	
	lateTSCinit();
}

