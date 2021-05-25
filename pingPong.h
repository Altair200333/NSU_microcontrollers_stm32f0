#pragma once
#include <stm32f0xx.h>
#include <system_stm32f0xx.h>
#include <stdlib.h>
#include "buttons.h"
#include "renderAPI.h"
#include <math.h>
#include "leds.h"
#include "usart_base.h"
#include "bit_master.h"
#include "tscHandler.h"

#define GAME 1
#define MENU 0

typedef struct 
{
	short x;//0 or 7
	short y;
	short width;
} Platform;

typedef struct 
{
	float x;
	float y;
	float xVelocity;
	float yVelocity;

} Ball;

typedef struct
{
	int mode;
	short side;//0 - left; 1 - right
	bool single;
	bool host;
	uint32_t phase;
} GameState;

volatile static GameState gameState;
static volatile Platform platformLeft;
static volatile Platform platformRight;
static volatile Ball ball;


static volatile Platform* controlled;
	
void initPong()
{
	platformLeft.x = 0;
	platformRight.x = 7;
	
	platformLeft.y = platformRight.y = 1;
	platformLeft.width = platformRight.width = 1;
	
	controlled = &platformLeft;
	
	ball.x = 3;
	ball.y = 3;
	ball.xVelocity = 0.2f;
	ball.yVelocity = 0.1f;
	
	gameState.mode = MENU;
	gameState.side = 0;
	gameState.single = true;
}

void drawPlatform(volatile Platform* player)
{
	for(int i=-player->width;i<=player->width; ++i)
	{
		drawSpiPos(player->x, player->y+i);
	}
}

void updateBall()
{
	//mb better to also use deltaTime here
	ball.x += ball.xVelocity;
	ball.y += ball.yVelocity;
	resetBitV(&LED, Bled);
	if(ball.x>7 || ball.x<0)
	{
		if ((round(ball.y)<=platformLeft.y+platformLeft.width && round(ball.y)>=platformLeft.y-platformLeft.width && ball.x<0) ||
			(round(ball.y)<=platformRight.y+platformRight.width && round(ball.y)>=platformRight.y-platformRight.width && ball.x>7))
		{
				ball.xVelocity *= -1.0f;
		}
		else
		{
			//setBitV(&LED, Bled);
			ball.x = 3;
			ball.y = 3;
		}
	}
	if(ball.y>7 || ball.y<0)
		ball.yVelocity *= -1.0f;
}
static volatile bool useTSC = false;
void processInput()
{
	if(!useTSC)
	{
		if(keyStates[Key_Up].state && controlled->y+controlled->width+1<8)
		{
			controlled->y += 1;
		}
		if(keyStates[Key_Down].state && controlled->y-controlled->width-1>=0)
		{
			controlled->y -= 1;
		}
	}
	else
	{
		int val = raw_result%8;
		if(val - controlled->width>=0 && val+controlled->width<8)
		{
			controlled->y = val;
		}
	}
	if(gameState.single)
	{	
		if(keyStates[Key_Right].state && platformRight.y+platformRight.width+1<8)
		{
			platformRight.y += 1;
		}
		if(keyStates[Key_Left].state && platformRight.y-platformRight.width-1>=0)
		{
			platformRight.y -= 1;
		}
	}
}
int cursorY = 1;
void drawCursor()
{
	for(int i=0;i<4;++i)
	{
		drawSpiPos(i, cursorY);
	}
	drawSpiPos(1, cursorY-1);
	drawSpiPos(1, cursorY+1);

}
void drawMenu()
{
	drawSpiPos(5,1);
	drawSpiPos(6,1);
	
	drawSpiPos(5,6);
	drawSpiPos(6,6);
	drawSpiPos(5,5);
	drawSpiPos(6,5);
	if(cursorY == 5 || cursorY == 6)
	{
		drawSpiPos(7,6);
		drawSpiPos(7,5);
	}
	if(cursorY == 1)
	{
		drawSpiPos(7,1);
	}
}
static volatile uint32_t lastUpdate = 0;
static volatile int rtState = 0;

uint8_t loadInt(uint8_t src, int offset)
{
	uint8_t result=0;
	if(getBit(2+offset, src))
		setBit(0, &result);
	if(getBit(3+offset, src))
		setBit(1, &result);
	if(getBit(4+offset, src))
		setBit(2, &result);
	return result;
}
uint8_t writeInt(uint8_t src, int offset)
{
	uint8_t result=0;
	if(getBit(0, src))
		setBit(2+offset, &result);
	if(getBit(1, src))
		setBit(3+offset, &result);
	if(getBit(2, src))
		setBit(4+offset, &result);
	return result;
}
void setTransmitData()
{
	if(gameState.mode == GAME)
	{
		if(gameState.host)
		{
			if(rtState == 0)
			{
				uint8_t out = writeInt(platformLeft.y, 0);
				setBit(1, &out);//01
				transfer.dataT = out;
			}
			if(rtState == 1)
			{
				uint8_t out = writeInt((int)round(ball.x), 0) | writeInt((int)round(ball.y), 3);
				setBit(0, &out);//10
				transfer.dataT = out;
			}
		}
		else
		{
			uint8_t out = writeInt(platformRight.y, 0);
			setBit(1, &out);//01
			transfer.dataT = out;
		}
		rtState = (rtState+1)%2;
	}
}
void receiveData()
{
	if(gameState.mode == GAME)
	{
		if(!gameState.host)
		{
			if(getBit(0, transfer.dataR) == 0 && getBit(1, transfer.dataR) == 1)
			{
				platformLeft.y = loadInt(transfer.dataR, 0);
			}
			if(getBit(0, transfer.dataR) == 1 && getBit(1, transfer.dataR) == 0)
			{
				ball.x = (float)loadInt(transfer.dataR, 0);
				ball.y = (float)loadInt(transfer.dataR, 3);
			}
		}
		else
		{
			if(getBit(0, transfer.dataR) == 0 && getBit(1, transfer.dataR) == 1)
			{
				platformRight.y = loadInt(transfer.dataR, 0);
			}
		}
	}
}
void onUpdatePong(volatile uint32_t timestamp)
{
	if(gameState.mode == GAME)
	{
		if(keyStates[Usr_Btn].state && keyStates[Usr_Btn].clicked)
		{
			keyStates[Usr_Btn].clicked = !keyStates[Usr_Btn].clicked;
			useTSC = !useTSC;
		}
		if(timestamp-lastUpdate>40)
		{
			if(gameState.host)
				updateBall();
			processInput();
			lastUpdate = timestamp;
		}
		
		drawPlatform(&platformLeft);
		drawPlatform(&platformRight);
		drawSpiPos((int)round(ball.x), (int)round(ball.y));
	}
	else if(gameState.mode == MENU)
	{
		if(timestamp-lastUpdate>30)
		{
			if(keyStates[Key_Up].state &&cursorY+1<7)
			{
				cursorY+=1;
			}
			if(keyStates[Key_Down].state &&cursorY-1>=1)
			{
				cursorY-=1;
			}
			lastUpdate = timestamp;
		}
		drawCursor();
		drawMenu();

		if(keyStates[Key_Right].state && cursorY == 1)//connect to host machine
		{
			gameState.mode = GAME;
			gameState.host = false;
			gameState.phase = 0;
			controlled = &platformRight;
			
			transfer.isTransmit = false;
			initUsart(&transfer);
		}
		if(keyStates[Key_Right].state && (cursorY == 5 || cursorY == 6))//wanna host?
		{
			gameState.mode = GAME;
			gameState.host = true;
			gameState.phase = 0;
			setBitV(&LED, Gled);
			controlled = &platformLeft;
			
			transfer.isTransmit = true;
			initUsart(&transfer);
		}
	}
}