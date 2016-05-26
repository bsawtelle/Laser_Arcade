/********************LED Manipulation Library*******************
*This library depends on the fact that SSI has been set up. UART can optionally be used to return error messages, but it has to be set up first.
*This happens outside of this library. 
*/


#ifndef _LEDSTRIP_H_
#define _LEDSTRIP_H_

	
#define LEDS			30
#define LEDSTRIPS 24
	
#define LEDOFF    0xE0000000	

//*************LED Colors*************
#define RED      0x0000FF
#define GREEN    0x00FF00
#define BLUE     0xFF0000
#define YELLOW   0x00FFFF
#define PURPLE   0xFF00FF
#define CYAN     0xFFFF00
#define WHITE    0xFFFFFF
#define PINK     0xB469FF
#define NAVYBLUE 0xFF0156



//*************Power Stuff*************
//All cvurrents in microamps
#define RED_LED_CURRENT       				633
#define GREEN_LED_CURRENT							566
#define BLUE_LED_CURRENT							566
#define STANDBY_CURRENT								35000
#define MAX_CURRENT_PER_POWER_LINE		2500000
#define POWER_LINES										8
#define MAX_TOTAL_CURRENT             POWER_LINES*MAX_CURRENT_PER_POWER_LINE


extern uint32_t SSIBAddress; //All functions need to know which SSIBase to work with
extern volatile uint16_t NumberLEDs; //also how many LEDs there are.
extern bool SetupRun; //Keeps track of whether the setup has been run or not.
extern bool UseUART; //Tells the program to send error messages back

extern volatile uint32_t LEDMap[LEDS][LEDSTRIPS];


extern uint32_t MultiplexRegisters[];
extern uint32_t LEDAddressPins[6][2]; 
extern uint8_t pins;

//This needs to be run to tell the library the variables above
extern void LEDManipSetup(uint32_t SSIBase, uint16_t LEDsPerString, uint8_t NumberofStrings);

extern void MultiplexStrip(uint8_t strip);

//************************************************
//****************LED Manipulation****************
//************************************************


//***********Grid based LED manipulation**********

//input an x and a y position, brightness and color and it will be stored in the LEDMap,
// a 2-dimensional array that holds the LED Data 
//x is currently LED number on a strip and y is currently the strip number
extern void SetLEDValue(uint16_t x_pos, uint8_t y_pos, uint8_t LED_Brightness, uint32_t Color);

//adds the color to the current LED rather than overwriting it
extern void BlendLEDValue(uint16_t x_pos, uint8_t y_pos, uint8_t LED_Brightness, uint32_t Color);

//turns off the LED at the coordinates
extern void SetLEDOff(uint8_t x_pos, uint8_t y_pos);

//sets all LED values to 0xE0000000 (off)
extern void ClearLEDMap(void);


//Display what's stored in LEDMap 
extern void UpdateLEDState(void);


extern bool PowerCalc(bool DivideCurrent);

//*******Individual 30 LED strip maniplation******

//Light selected LED to max brightness and full color (White). This only lights one LED
//at any given time, all others are turned off.
extern void LightLEDSimple(uint16_t LED_Number);


//Light selected LED to specified color and brightness. This only lights one LED at any
//given time, all others are turned off.
extern void LightLED(uint16_t LED_Number, uint8_t Brightness, uint32_t Color);


//Turn on all LEDs full color and brightness (White)
extern void LightAllLEDsSimple(void);
	
//Turn on all LEDs to specified color and brightness
extern void LightAllLEDs(uint8_t Brightness, uint32_t Color);


//Light an individual LED to max brightness and full color (White) while not changing
//the state of any other LEDs on the string (any LEDs that are on will stay on and any that
//are off will stay off).
extern void AddLEDSimple(uint16_t LED_Number);


//Light an individual LED to a specified color and brightness while not changing
//the state of any other LEDs on the string (any LEDs that are on will stay on and any that
//are off will stay off).
extern void AddLED(uint16_t LED_Number, uint8_t Brightness, uint32_t Color);



//Turn off an individual LED in the strip
extern void ClearLED(uint16_t LED_Number);


//Turn off ALL LEDs in the strip
extern void ClearAllLEDs(void);


//Light a selected number of LEDs to full brightness and full color (White)
extern void LightLEDArraySimple(uint16_t* LED_Numbers);


//Light a selected number of LEDs to specified color and brightness
extern void LightLEDArray (uint16_t* LED_Numbers, uint8_t* LED_Brightness, uint32_t* Color);


//Test Patterns, mostly just me messing around for a bit
extern void LEDBounce(void);
extern void LightLEDPattern(void);
extern void LEDBrightnessPattern(void);


#endif
