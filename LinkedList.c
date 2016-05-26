#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "Displays.h"
#include "LinkedList.h"


//struct LEDEvent;

struct LEDEvent *root;
struct LEDEvent *PreviousEvent;
volatile uint8_t EventArray[5];

void InitialiseEvents()
{
	root = (struct LEDEvent *) malloc(sizeof(struct LEDEvent));
	root -> next_event = NULL;
}

void CreateNewEvent(uint8_t x_coordinate, uint8_t y_coordinate, uint8_t event_type)
{
	
	struct LEDEvent *current_event = root;
	
	while(true) //runs until it creates a new link in the list (exits with a return statement)
	{
		if(current_event->next_event == NULL)
		{
			current_event->next_event = malloc( sizeof(struct LEDEvent));
			
			current_event = current_event->next_event;
			
			current_event->x = x_coordinate;
			
			current_event->y = y_coordinate;
			
			current_event->animation_type = event_type;
			
			current_event->next_event = NULL;
			
			current_event->frames_till_change = FRAME_LENGTH;
	
			return;
		}
		else 
		{
			current_event = current_event->next_event;
		}
		
	} 
	
}

void readList()
{
	struct LEDEvent *current_event = root;
	uint8_t count = 0;
	
	if(current_event->next_event == NULL) //leaves the root link
	{
		return;
	}
	
	
	current_event = current_event->next_event;
	
	while(current_event != NULL)
	{
		EventArray[count] = current_event->animation_type;
		count++;
		current_event = current_event->next_event;
	}
}


void RunAnimations()
{
	struct LEDEvent *current_event = root;
	
	if(current_event->next_event == NULL) //stops if there are no links in the list
	{
		return;
	}
	else
	{
		current_event = current_event->next_event;
	}
	
	while(current_event != NULL)
	{
		if(current_event->frames_till_change==0)
		{
			
			if(current_event->animation_type == RECTANGLE_TYPE)
			{
				uint8_t x0 = (current_event->x - current_event->animation_stage);
				uint8_t x1 = (current_event->x + current_event->animation_stage);
				
				uint8_t y0 = (current_event->y - current_event->animation_stage);
				uint8_t y1 = (current_event->y + current_event->animation_stage);
				Rectangle(x0, y0, x1, y1, 0xFF, current_event->player_color);
				
				//delete event if it reaches the end
			}
			
			else if(current_event->animation_type == CIRCLE_TYPE)
			{
				uint8_t radius = current_event->animation_stage;
				uint8_t x0 = current_event->x;
				uint8_t y0 = current_event->y;
				Circle(x0, y0, radius, 0xFF, current_event->player_color);
				current_event->frames_till_change = FRAME_LENGTH;
			}
			else
			{
				
			}
			current_event->animation_stage++;
			
		}
		current_event->frames_till_change--;
		return;
	}
	
}

void DeleteEvent(struct LEDEvent *Event)
{
	struct LEDEvent *NextLink = Event->next_event;
	PreviousEvent->next_event = NextLink; 
	free(Event);
}
