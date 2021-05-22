#pragma once
#include <stm32f0xx.h>
#include <system_stm32f0xx.h>
#include <stdlib.h>
#include "buttons.h"
#include "renderAPI.h"
#include <math.h>
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
	
	if(ball.x>7 || ball.x<0)
	{
			ball.xVelocity *= -1.0f;
			//ball.x = 3;
			//ball.y = 3;
	}
	if(ball.y>7 || ball.y<0)
		ball.yVelocity *= -1.0f;
	
	if ((ball.x<1 && ball.y<platformLeft.y+platformLeft.width && ball.y>platformLeft.y-platformLeft.width) ||
			(ball.x>6 && ball.y<platformRight.y+platformRight.width && ball.y>platformRight.y-platformRight.width))
	{
		//ball.yVelocity *= -1.0f;
	}
	
}

void onUpdatePong(volatile uint32_t timestamp)
{
	drawPlatform(&platformLeft);
	drawPlatform(&platformRight);
	
	if(keyStates[Key_Up].state && controlled->y+controlled->width+1<8)
	{
		controlled->y += 1;
	}
	if(keyStates[Key_Down].state && controlled->y-controlled->width-1>=0)
	{
		controlled->y -= 1;
	}
	
	if(keyStates[Key_Right].state && platformRight.y+platformRight.width+1<8)
	{
		platformRight.y += 1;
	}
	if(keyStates[Key_Left].state && platformRight.y-platformRight.width-1>=0)
	{
		platformRight.y -= 1;
	}
	
	drawSpiPos((int)round(ball.x), (int)round(ball.y));
	
	updateBall();
}