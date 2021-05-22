#pragma once
#include "stm32f0xx.h"
#include "renderAPI.h"
#include <math.h>

void TSC_IRQHandler(void)
{
	if ((TSC->ISR & TSC_ISR_EOAF) == TSC_ISR_EOAF)
	{	
		TSC->ICR = TSC_ICR_EOAIC; /* Clear flag */
		//for(int i=0;i<8;++i)
		//{
		//	int AcquisitionValue = TSC->IOGXCR[i]; /* Get G2 counter value */
		//	drawSpiPos(i,(AcquisitionValue%8));
		//}
	  uint32_t AcquisitionValue = TSC->IOGXCR[1]; /* Get G2 counter value */
		drawSpiPos(6,(AcquisitionValue%8));

		TSC->CR |= TSC_CR_START;
	}
}
//GPIOA->AFR[0] |= 3 << (0) * 4;
//alt.func. number ^;pin.^;   ^ - always the same
void touch_init(void) 
{
	
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;
	
	GPIOA->MODER |= (0x2 << (2*1)) + (0x2 << (2*2)); //enable AF for PA1 and PA2
	
	
	GPIOA->AFR[0] |= (0x3 << (4*1)) + (0x3 << (4*2)); //enable AF3 for PA1 and PA2
	GPIOA->OTYPER |= (1<<3);
		
	RCC->AHBENR |= RCC_AHBENR_TSEN;
	
	TSC->IOSCR |= TSC_IOSCR_G2_IO4; //enable G1_IO4 as sampling capacitor
	TSC->IOCCR |= TSC_IOCCR_G2_IO3; //enable G1_IO3 as channel
	
	TSC->IOGCSR |= TSC_IOGCSR_G2E; //enable G1 analog group
	
	TSC->IOHCR &= (uint32_t)(~(TSC_IOHCR_G2_IO4 | TSC_IOHCR_G2_IO3)); //disable hysteresis on PA1 and PA2 
		
	TSC->IER |= TSC_IER_EOAIE; //enable end of acquisition interrupt
	NVIC_EnableIRQ(TSC_IRQn);
	
	TSC->CR |= TSC_CR_PGPSC_2 | TSC_CR_PGPSC_0 | TSC_CR_CTPH_0 | TSC_CR_CTPL_0 | TSC_CR_MCV_2 | TSC_CR_MCV_1 | TSC_CR_TSCE;
	TSC->CR |= TSC_CR_START;//(0x01 + 0x00C0 + 0x02); //enable TSC and start acquisition //TSC_CR_TSCE | TSC_CR_START
}

