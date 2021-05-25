#pragma once
#include "pingPong.h"
#include "usart_base.h"
#include "tim_timer.h"
#include "leds.h"

static volatile int numberOfSends = 0;

int get_delay(int lower, int upper)
{
	return (rand() % (upper - lower + 1)) + lower;
}

void autoSyncLan()
{
	if (transfer.isTransmit)
	{
		timer.counter = 0;
		//transfer.dataT = cursorY;
		setTransmitData();
		if (transmitMessage(&transfer))
		{
			numberOfSends++;
			if (numberOfSends > 3)
			{
				transfer.isTransmit = false;
				initUsart(&transfer);
				timer.counter = 0;
				numberOfSends = 0;
			}
		}
	}
	else
	{
		if (receiveMessage(&transfer))
		{
			timer.counter = get_delay(10, 20);
		}
		if (timer.counter > 30)
		{
			transfer.isTransmit = true;
			initUsart(&transfer);
		}
		receiveData();
	}
	
	
	if (transfer.isTransmit)
	{
		setBitV(&LED, Oled);
	}
	else
	{
		resetBitV(&LED, Oled);
	}
	
}