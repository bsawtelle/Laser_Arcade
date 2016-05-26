#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/rom.h"
#include "inc/hw_memmap.h"
#include "Displays.h"
#include "math.h"
#include "LEDStrip.h"


//Draws a line, ripped straight from wikipedia "Bresenham's_line_algorithm"
void GraphLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t brightness, uint32_t color)
{
	
	float deltax   = x0 - x1;
	float deltay   = y0 - y1;
	float slope = abs(deltay/deltax); //this initialization doesn't really do anything
	
	volatile uint8_t x = x0;
	volatile uint8_t y = y0;
	
	while((deltax != 0) & (deltay != 0))
	{
		
		
		SetLEDValue(x, y, brightness, color);
		
	
		slope = abs(deltay/deltax);
		
		if((deltax == 0) & (deltay != 0))
		{
			if(deltay > 0)
			{
				y++;
			}
			else
			{
				y--;
			}
		}
		
		if (slope < 1)
		{
			if(deltax > 0)
			{
				x++;
			}
			else if (deltax < 0)
			{
				x--;
			}
		}
		else if (slope > 1)
		{
			if(deltay > 0)
			{
				y++;
			}
			else if (deltay < 0)
			{
				y--;
			}
		}
		else
		{
			if((deltax < 0) & (deltay < 0))
			{
				x--;
				y--;
			}
			else if((deltax < 0) & (deltay > 0))
			{
				x--;
				y++;
			}
			else if((deltax > 0) & (deltay < 0))
			{
				x++;
				y--;
			}
			else if ((deltax > 0) & (deltay > 0))
			{
				x++;
				y++;
			}
			else
			{
				UARTprintf("All cases should be exhausted, this shouldn't happen.");
			}
		}
	}
	
}

void Triangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t brightness, uint32_t color)
{
	GraphLine(x0, y0, x1, y1, brightness, color);
	GraphLine(x0, y0, x2, y2, brightness, color);
	GraphLine(x1, y1, x2, y2, brightness, color);
}

void Rectangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t brightness, uint32_t color)
{
	
	

	for(uint8_t x = x0; x <= x1; x++) //graph horizontal lines
	{
		int y = y0;
	
		SetLEDValue(x, y, brightness, color);
	
		
		y = y1;
		
		
		SetLEDValue(x, y, brightness, color);
		
	}
	
	for(uint8_t y = y0; y <= y1; y++)
	{
		int x = x0;
	
		SetLEDValue(x, y, brightness, color);
		
		
		x = x1;
		
		
		SetLEDValue(x, y, brightness, color);
		
	}
	
}

void Circle(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t brightness, uint32_t color)
{
	volatile uint8_t resolution_multiplier = 3;
	volatile uint8_t resolution = radius*resolution_multiplier;
	
	/*
	for(uint8_t x = x0 - radius; x <= (x0+radius); x++)
	{
		int y = y0 + sqrt((radius*radius) - ((x - x0)*(x - x0)));
		if((x >= LEDS)|| (y >= LEDSTRIPS))
		{
			break;
		}
		SetLEDValue(x, y, brightness, color);
		
		y = y0 - sqrt((radius*radius) - ((x - x0)*(x - x0)));
		if((x >= LEDS)||(y >= LEDSTRIPS))
		{
			break;
		}
		SetLEDValue(x, y, brightness, color);
	}
	*/
	float angle = 0;
	while(angle < (PI/2))
	{
		uint8_t x = radius*cos(angle);
		uint8_t y = radius*sin(angle);

		UARTprintf("%d", angle);
		SetLEDValue(x0-x, y0-y, brightness, color);
		SetLEDValue(x0-x, y+y0, brightness, color);
		SetLEDValue(x+x0, y0-y, brightness, color);
		SetLEDValue(x+x0, y+y0, brightness, color);
		angle = angle + ((PI/2)/resolution);
	}
	
}

