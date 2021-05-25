#pragma once
#include <stdbool.h>
#include <stdlib.h>

#include "renderAPI.h"


typedef struct _transfer
{
	unsigned char data;
	bool isTransmit;
}Transfer;

static volatile Transfer transfer;


void initUsartTransferTransmit()
{
	/* (1) Oversampling by 16, 9600 baud */
	/* (2) 8 data bit, 1 start bit, 1 stop bit, no parity */
	USART3->BRR = 480000 / 96; /* (1) */
	USART3->CR1 &= ~USART_CR1_RE;
	USART3->CR1 = USART_CR1_TE | USART_CR1_UE;/* (2) */
	//while(!(USART3->ISR & USART_ISR_TC)); // polling idle frame Transmission
	//USART3->ICR |= USART_ICR_TCCF; // clear TC flag
	//USART3->CR1 |= USART_CR1_RXNEIE;
}

void initUsartTransferReceive()
{
	/* (1) oversampling by 16, 9600 baud */
	/* (2) 8 data bit, 1 start bit, 1 stop bit, no parity, reception mode */
	USART3->BRR = 480000 / 96; /* (1) */
	USART3->CR1 &= ~USART_CR1_TE;//unset trasnmit enabled
	USART3->CR1 = USART_CR1_RE | USART_CR1_UE; /* (2) */
}

void setTransferMode(bool isTransmit)
{
	//https://github.com/jxwleong/stm32-usart#usart
	USART3->ICR = 0;//clear all registers
	volatile uint32_t tmp = USART3->RDR;//clear interrupt flags
	//clear overrun
	tmp = USART3->ISR;
	tmp = USART3->RDR;
	tmp = USART3->TDR;
	//
	transfer.data = 0;
	transfer.isTransmit = isTransmit;
	if (isTransmit)
	{
		initUsartTransferTransmit();
		USART3->TDR = 0;
	}
	else
	{
		initUsartTransferReceive();
	}
}
void initTransfer()
{
	RCC->APB1ENR|= RCC_APB1ENR_USART3EN;
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN;
	
	GPIOC->MODER |= GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1;
	GPIOD->MODER |= GPIO_MODER_MODER2_1;
	
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR10_1 | GPIO_PUPDR_PUPDR11_1;
	GPIOD->PUPDR |= GPIO_PUPDR_PUPDR2_1;
	
	USART3->CR3 |= USART_CR3_DEM;
	
		// Set Alternate function 1 to PC10
	GPIOC->AFR[1] |= 1 << (10 - 8) * 4;
	// Set alternate function 1 to PC11
	GPIOC->AFR[1] |= 1 << (11 - 8) * 4;
	// Set Alternate function 1 to PD2
	GPIOD->AFR[0] |= 1 << (2  - 0) * 4;
	// Clocking
	
}

void transmitMessage()
{
	//if (transfer->isTransmit)
	{
		if ((USART3->ISR & USART_ISR_TC) == USART_ISR_TC)
		{
				/* clear transfer complete flag and fill TDR with a new char */
				USART3->TDR = transfer.data;
				USART3->ICR |= USART_ICR_TCCF;
				
		}
	}
	//else
	{
		// you are idiotte
	}
}

void receiveMessage()
{
	//if (!transfer->isTransmit)
	{
		int g = 0;
		if ((USART3->ISR & USART_ISR_RXNE) == USART_ISR_RXNE)
		{
			transfer.data = (uint8_t)(USART3->RDR); /* Receive data, clear flag */
		}
	}
	//else
	{
		// you are idiotte
	}
}