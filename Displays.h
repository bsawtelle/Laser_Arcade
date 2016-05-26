#ifndef _DISPLAYS_H_
#define _DISPLAYS_H_

#define PI 3.1415

//first point, second point, color value
extern void GraphLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t brightness, uint32_t color);


extern void Triangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t brightness, uint32_t color);

//top left corner coordinates, top right corner coordinates, color value
extern void Rectangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t brightness, uint32_t color);

extern void Circle(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t brightness, uint32_t color);

#endif //Displays.h
