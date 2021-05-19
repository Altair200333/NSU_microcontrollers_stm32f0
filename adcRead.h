#pragma once
#include <stm32f0xx.h>
#include <stdbool.h>

typedef struct _DMA
{
	uint16_t ADC_array[16];
	bool DMA_half;
	bool DMA_full;
}DMA;
DMA dma;

void initADC()
{
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	ADC1->CHSELR |= ADC_CHSELR_CHSEL1;					
	ADC1->CFGR1 |= 0x01 << ADC_CFGR1_RES_Pos; //resolution (1024 points)
	//ADC1->CFGR1 |= ADC_CFGR1_CONT; //continuous conversion mode
	ADC1->CR |= ADC_CR_ADCAL;
	while (ADC1->CR & ADC_CR_ADCAL) {}
	ADC1->CR |= ADC_CR_ADEN;

	while (!(ADC1->ISR & ADC_ISR_ADRDY)) {}
	ADC1->CR |= ADC_CR_ADSTART;
}

uint16_t blockingRead()
{
	ADC1->CR |= ADC_CR_ADSTART; /* Start the ADC conversion */
	while (!(ADC1->ISR & ADC_ISR_EOC)) {} /* Wait end of Analot to Digital Conversion */
	return ADC1->DR; /* Store the ADC conversion result */
}

void initDMA(DMA* dma)
{
	for(int i=0;i<16;++i)
		dma->ADC_array[i] = 0;
	dma->DMA_full = false;
	dma->DMA_half = false;
	
	ADC1->CFGR1 |= ADC_CFGR1_CONT | ADC_CFGR1_OVRMOD; 
	ADC1->CFGR1 |= ADC_CFGR1_DMAEN | ADC_CFGR1_DMACFG;
	ADC1->CR |= ADC_CR_ADSTART;
	/* The following example is given for the ADC. It can be easily ported on
	any peripheral supporting DMA transfer taking of the associated channel
	to the peripheral, this must check in the datasheet. */
	/* (1) Enable the peripheral clock on DMA */
	/* (2) Enable DMA transfer on ADC */
	/* (3) Configure the peripheral data register address */
	/* (4) Configure the memory address */
	/* (5) Configure the number of DMA tranfer to be performs on channel 1 */
	/* (6) Configure increment, size and interrupts */
	/* (7) Enable DMA Channel 1 */
	RCC->AHBENR |= RCC_AHBENR_DMA1EN; /* (1) */
	//ADC1->CFGR1 |= ADC_CFGR1_DMAEN; /* (2) */
	DMA1_Channel1->CPAR = (uint32_t) (&(ADC1->DR)); /* (3) */
	DMA1_Channel1->CMAR = (uint32_t)(dma->ADC_array); /* (4) */
	DMA1_Channel1->CNDTR = 16; /* (5) */
	DMA1_Channel1->CCR |= DMA_CCR_MINC | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0; /* (6) */
	//DMA1_Channel1->CCR |= DMA_CCR_TEIE | DMA_CCR_TCIE ;
	DMA1_Channel1->CCR |= DMA_CCR_PL | DMA_CCR_CIRC;
	DMA1_Channel1->CCR |= DMA_CCR_TCIE;//DMA_CCR_HTIE|DMA_CCR_TCIE;
	DMA1_Channel1->CCR |= DMA_CCR_EN; /* (7) */
	/* Configure NVIC for DMA */
	/* (1) Enable Interrupt on DMA Channel 1 */
	/* (2) Set priority for DMA Channel 1 */
	NVIC_EnableIRQ(DMA1_Channel1_IRQn); /* (1) */
	//NVIC_SetPriority(DMA1_Channel1_IRQn,0);
}