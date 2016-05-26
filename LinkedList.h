#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

#define RECTANGLE_TYPE    0x00
#define CIRCLE_TYPE       0x01

#define FRAME_LENGTH      0x0F

extern volatile uint8_t EventArray[5];


struct LEDEvent
{
	uint8_t player_color;
	uint8_t animation_type;
	uint8_t frames_till_change;
	uint8_t animation_stage; //sort of acts like a radius
	uint8_t x;
	uint8_t y;
	struct LEDEvent *next_event;
};

extern struct LEDEvent *root;

extern void InitialiseEvents(void);

extern void CreateNewEvent(uint8_t x_coordinate, uint8_t y_coordinate, uint8_t event_type);

extern void RunAnimations(void);

extern void DeleteEvent(struct LEDEvent *Event);
	
#endif //LinkedList.h
