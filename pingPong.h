#pragma once
#include <stm32f0xx.h>
#include <system_stm32f0xx.h>
#include <stdlib.h>
#include "buttons.h"
#include "renderAPI.h"
#include <math.h>
#include "leds.h"

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
			setBitV(&LED, Bled);
			ball.x = 3;
			ball.y = 3;
		}
	}
	if(ball.y>7 || ball.y<0)
		ball.yVelocity *= -1.0f;
}
void processInput()
{
	
	if(keyStates[Key_Up].state && controlled->y+controlled->width+1<8)
	{
		controlled->y += 1;
	}
	if(keyStates[Key_Down].state && controlled->y-controlled->width-1>=0)
	{
		controlled->y -= 1;
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
void onUpdatePong(volatile uint32_t timestamp)
{
	if(gameState.mode == GAME)
	{
		updateBall();

		drawPlatform(&platformLeft);
		drawPlatform(&platformRight);
		drawSpiPos((int)round(ball.x), (int)round(ball.y));

		processInput();
	}
	else if(gameState.mode == MENU)
	{
		if(keyStates[Key_Up].state &&cursorY+1<7)
		{
			cursorY+=1;
		}
		if(keyStates[Key_Down].state &&cursorY-1>=1)
		{
			cursorY-=1;
		}
		drawCursor();
		drawMenu();
		if(keyStates[Key_Right].state && cursorY == 1)
		{
			gameState.mode = GAME;
		}
	}
}