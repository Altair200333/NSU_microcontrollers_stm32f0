#pragma once
#include <stdbool.h>
#include <stdlib.h>

#include "renderAPI.h"


typedef struct _transfer
{
	unsigned char dataT;
	unsigned char dataR;
	bool isTransmit;
}Transfer;
Transfer transfer;

void initUsartTransferTransmit()
{
	/* (1) Oversampling by 16, 9600 baud */
	/* (2) 8 data bit, 1 start bit, 1 stop bit, no parity */
	USART3->CR1 &= ~USART_CR1_UE;
	USART3->ICR = 0;
	USART3->CR1 &= ~USART_CR1_RE;
	USART3->BRR = 480000 / 96; /* (1) */
	USART3->CR1 = USART_CR1_TE | USART_CR1_UE;/* (2) */
	//while(!(USART3->ISR & USART_ISR_TC)); // polling idle frame Transmission
	//USART3->ICR |= USART_ICR_TCCF; // clear TC flag
}

void initUsartTransferReceive()
{
	/* (1) oversampling by 16, 9600 baud */
	/* (2) 8 data bit, 1 start bit, 1 stop bit, no parity, reception mode */
	USART3->CR1 &= ~USART_CR1_UE;
	USART3->ICR = 0;
	USART3->CR1 &= ~USART_CR1_TE;
	USART3->BRR = 480000 / 96; /* (1) */
	USART3->CR1 = USART_CR1_RE | USART_CR1_UE; /* (2) */
}

void initUsart(Transfer* transfer)
{
	if (transfer->isTransmit)
	{
		initUsartTransferTransmit();
		//USART3->TDR = 0;
	}
	else
	{
		initUsartTransferReceive();
	}
}

void ConstrTransfer(Transfer* transfer, bool isTransmit)
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
	

	transfer->dataR = 0;
	transfer->dataT = 0;
	transfer->isTransmit = isTransmit;
	
	initUsart(transfer);
}



bool transmitMessage(Transfer* transfer)
{
		if ((USART3->ISR & USART_ISR_TC) == USART_ISR_TC)
		{
				/* clear transfer complete flag and fill TDR with a new char */
				USART3->TDR = transfer->dataT;
				USART3->ICR |= USART_ICR_TCCF;
				return true;
		}
		return false;
}

bool receiveMessage(Transfer* transfer)
{
		if ((USART3->ISR & USART_ISR_RXNE) == USART_ISR_RXNE)
		{
			transfer->dataR = (uint8_t)(USART3->RDR); /* Receive data, clear flag */
			return true;
		}
		return false;
}