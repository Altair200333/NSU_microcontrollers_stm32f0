#include <stm32f0xx.h>
#include <stdlib.h>

#include "buttons.h"
#include "renderAPI.h"

typedef struct 
{
	uint32_t counter;
} Context;

void init(void);
void loop(Context*);
void resetAll(void);

//set and reset bit for volatile uint32_t
void setBitV(volatile uint32_t* bit, uint32_t value);
void resetBitV(volatile uint32_t* bit, uint32_t value);
void wait(uint32_t);
bool buttonDown(void);
void SysTick_Handler (void);
void traffic(void);
void SysTick_Init(void);
void SysTick_Wait(uint32_t);
void renderLoop(void);

#define LED GPIOC->ODR
#define F_CPU 		72000000UL	
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

static volatile uint32_t timestamp = 0;
  
void SysTick_Handler (void)
{
   timestamp++;
	 fetchButtons(timestamp);	
	 //renderLoop(); 
}
static int ypos = 0;
static int xpos = 0;
void SPI2_IRQHandler(void)
{
	setBitV(&LED, Rled);
	
	volatile uint16_t data = SPI2->DR;
	renderInt();
	resetBitV(&LED, Rled);
}
void wait(uint32_t delay)
{
	uint32_t delaystamp = timestamp + delay;
	while(1)
	{
		 if (timestamp > delaystamp)
		 {
				break;
		 }
	}
}
void resetAll()
{
	for(unsigned long i =0;i<15;++i)
	{
		GPIOC->MODER |= GPIO_MODER_MODER0+i;
	}
}

void init(void)
{
	resetAll();
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
	//SystemInit(); // Device initialization
	SystemInit();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/1000);
	SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
	
	resetBtns();
	initSPI();
}

bool buttonDown()
{
	return GPIOA->IDR & GPIO_IDR_0;
}
static int clamp(int val, int min,int max)
{
	if(val>max)
		return max;
	if(val<min)
		return min;
	return val;
}
static volatile bool states[4] = {false,false,false,false};

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))


void loop(Context* context)
{
	uint32_t leds[4] = {Rled, Bled, Gled, Oled};

	if(buttonDown())
	{
		states[0] = true;
	}
	
	for (int i = 0; i < 4; ++i)
	{
		if(keyStates[i].state)
		{
			states[i] = !states[i];
			//keyStates[i].clicked = false;
			
			if(i==2)
				ypos++;
			if(i==3)
				ypos--;
			
			if(i==1)
				xpos++;
			if(i==0)
				xpos--;
			
			xpos = clamp(xpos, 0, 7);
			ypos = clamp(ypos, 0, 7);

		}
		
	}
	for (int i = 0; i < 4; ++i)
	{
		//if(states[i])
		//	setBitV(&LED, leds[i]);
		//else
		//	resetBitV(&LED, leds[i]);
	}
	
	
	for(int i =-1;i<=1;++i)
		if(xpos+i>=0 && xpos+i<8)
			drawSpiPos(xpos+i, ypos);
	
	for(int i =-1;i<=1;++i)
		if(ypos+i>=0 && ypos+i<8)
			drawSpiPos(xpos, ypos+i);
	
	//for(int i=0; i<8; i++)
	//{
	//	renderBegin();
	//	drawRow(i, &renderBuffer);
	//	while(renderBusy()){}	
	//	renderFlush();
	//}
	clientFlush();
	clearImage();
	wait(40);
}	

int main(void)
{
	
	Context* context = (Context*)malloc(sizeof(Context));
	context->counter = 0;
	
	init();
	
	while(1)
	{
		loop(context);
	}
}
