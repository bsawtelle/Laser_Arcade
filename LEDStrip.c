#include <stdbool.h>
#include <stdint.h>
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/rom.h"
#include "inc/hw_memmap.h"
#include "LEDStrip.h"


bool SetupRun = false; 
//bool UseUART;
volatile uint16_t NumberLEDs;
uint8_t  LEDStrings;
uint32_t SSIBAddress;
uint32_t volatile LEDMap[LEDS][LEDSTRIPS];
uint8_t pins = sizeof(LEDAddressPins)/sizeof(LEDAddressPins[0]);

uint32_t MultiplexRegisters[] = {SYSCTL_PERIPH_GPIOB, SYSCTL_PERIPH_GPIOF, SYSCTL_PERIPH_GPIOE}; //store the multiplex registers
uint32_t LEDAddressPins[6][2] = {GPIO_PORTB_BASE, GPIO_PIN_6,			//store the data of the physical pins that control multiplexing
																 GPIO_PORTB_BASE, GPIO_PIN_7,
																 GPIO_PORTF_BASE, GPIO_PIN_4,
																 GPIO_PORTE_BASE, GPIO_PIN_3,
																 GPIO_PORTE_BASE, GPIO_PIN_2};
											
void LEDManipSetup(uint32_t SSIBase, uint16_t LEDsPerString, uint8_t NumberofStrings)
{
	SSIBAddress = SSIBase;                  //Which SSI register 
	NumberLEDs  = LEDsPerString;     //How many LEDs are on each string
	LEDStrings = NumberofStrings;
	SetupRun = true;      	//Currently doesn't do anything, might eventually be used to check if this was run
	//UseUART = TurnOnUART;  	//TurnOnUART was an argument planned for debugging... it wasn't used
	
	
	for(int y = 0; y < LEDStrings; y++)
	{
		for(int x = 0; x < NumberLEDs; x++)
		{
			LEDMap[x][y] = LEDOFF;
		}
	}	
	
}


void UpdateLEDState()
{
	uint32_t datamask = 0xFF;
	uint8_t shiftLength = 8;
	uint8_t dataLength = 4;
	volatile uint8_t dataArray[4];
	
	//UARTprintf("\n\n ---------------------------------------------------------\n\n");
	for(uint8_t StartFrameByte = 0; StartFrameByte < 4; StartFrameByte++)
	{
			SSIDataPut(SSIBAddress, 0);
	}
		
	for(int y = 0; y < LEDStrings; y++)
	{
		for(uint16_t x = 0; x < NumberLEDs; x++)
		{
			
			//SSIDataPutNonBlocking(SSIBAddress, LEDMap[x][y]);
				for(int byte = (dataLength-1); byte >= 0; byte--)
				{
					dataArray[byte] = ((LEDMap[x][y]>>(shiftLength*byte))&datamask);
					SSIDataPut(SSIBAddress, dataArray[byte]);
				/*	
					if(x < 15)
					{
						UARTprintf("%X", dataArray[byte]);
					}
					*/
				}
				//UARTprintf(" ");
			//SysCtlDelay(10);
				
				
		}
		//UARTprintf("\n");
		//SysCtlDelay(10);
	}
	for(int time = 0; time < 30; time++) //allows the signal to propagate through the final few LEDs (the last few won't light up otherwise)
	{
		for(uint8_t StartFrameByte = 0; StartFrameByte < 4; StartFrameByte++)
		{
			SSIDataPut(SSIBAddress, 0);
		}
	}
}

void MultiplexStrip (uint8_t strip)
{
	volatile int LEDOn;
	volatile int shift;
	for(volatile int x = 0; x < pins; x++)
	{
		shift = 1<<x;
		LEDOn = strip&shift;
		if(LEDOn)
		{
			GPIOPinWrite(LEDAddressPins[x][0], LEDAddressPins[x][1], LEDAddressPins[x][1]);
		}
		else
		{
			GPIOPinWrite(LEDAddressPins[x][0], LEDAddressPins[x][1], 0);
		}
	}
}




void SetLEDValue(uint16_t x_pos, uint8_t y_pos, uint8_t LED_Brightness, uint32_t Color)
{
	if((x_pos >= LEDS) || (y_pos >= LEDSTRIPS))
	{
		return;
	}
	uint32_t data;
	LED_Brightness = (LED_Brightness|0xE0);
	if(y_pos%2 == 0)
	{
		x_pos = (NumberLEDs -1) - x_pos;
	}
	data = Color;
	data = ((LED_Brightness|0xE0)<<24)|data;
	
	LEDMap[x_pos][y_pos] = data;
}


void BlendLEDValue(uint16_t x_pos, uint8_t y_pos, uint8_t LED_Brightness, uint32_t Color)
{
	if((x_pos >= LEDS) || (y_pos >= LEDSTRIPS))
	{
		return;
	}
	uint32_t data;
	LED_Brightness = (LED_Brightness|0xE0);
	if(y_pos%2 == 0)
	{
		x_pos = (NumberLEDs -1) - x_pos;
	}
	data = LEDMap[x_pos][y_pos]|Color;
	data = ((LED_Brightness|0xE0)<<24)|data;
	
	LEDMap[x_pos][y_pos] = data;
}


void SetLEDOff(uint8_t x_pos, uint8_t y_pos)
{
	if((x_pos >= LEDS) || (y_pos >= LEDSTRIPS))
	{
		return;
	}

	if(y_pos%2 == 0)
	{
		x_pos = (NumberLEDs -1) - x_pos;
	}

	LEDMap[x_pos][y_pos] = LEDOFF;
	
}


void ClearLEDMap()
{
	for(int y = 0; y < LEDStrings; y++)
	{
		for(int x = 0; x < NumberLEDs; x++)
		{
			LEDMap[x][y] = LEDOFF;
		}
	}	
}




//This seems like it will be horribly slow and is completely non-ideal
bool PowerCalc(bool DivideCurrent)
{
	uint32_t LEDStatus; //the immediate status of the LED being checked
	uint32_t LEDCurrent;  //the current consumption of the LED being checked
	uint8_t  LEDBrightness;
	uint32_t ColorMasks[3]	= {0x000000FF, 0x0000FF00, 0x00FF0000}; //these check which colors are on (R, G, B)
	uint32_t ColorCurrent[3]  =	{RED_LED_CURRENT, GREEN_LED_CURRENT, BLUE_LED_CURRENT}; //stores the current each color uses
	uint32_t BrightnessMask = 0xFF000000;
	uint32_t TotalCurrent = STANDBY_CURRENT; //holds the overall required current
	float CurrentRatio; //Ratio of required current to available current
	
	
	for(int y = 0; y < LEDStrings; y++)
	{
		for(int x = 0; x < NumberLEDs; x++)
		{
			LEDStatus = LEDMap[x][y];
			LEDCurrent = 0; //set the initial current to zero
			for(uint8_t color = 0; color < 3; color++) //loop through the color arrays
			{
				LEDBrightness = ((LEDStatus&BrightnessMask) - 0xE0); // 0xE0 is a signal for communication between strips and does not contribute to brightness
				if(LEDStatus & ColorMasks[color]) //bitwise and to see if the color is on
				{
					LEDCurrent = LEDCurrent + (ColorCurrent[color]*LEDBrightness); //add all of the colors that are on into LEDCurrent
				}
			}
			TotalCurrent = TotalCurrent + LEDCurrent; //add the current of the LED to the total power, then check the next one
		}
	}
	if(TotalCurrent < MAX_TOTAL_CURRENT)
	{
		
		return true; //return true if the current is within limits
	}
	else
	{
		if(DivideCurrent)
		{
			CurrentRatio = (float) MAX_TOTAL_CURRENT/TotalCurrent; //can you really typecast a #define?
			for(int y = 0; y < LEDStrings; y++)
			{
				for(int x = 0; x < NumberLEDs; x++)
				{
					LEDBrightness = LEDMap[x][y]&BrightnessMask; //get brightness info
					LEDBrightness = (float) (LEDBrightness*CurrentRatio)*(3/4);  //multiply by the currentRatio sketchy syntax goin on here
					LEDMap[x][y] = LEDMap[x][y] & (!BrightnessMask); //clear brightness bits
					LEDMap[x][y] = LEDMap[x][y] | (LEDBrightness<<24); //replace brightness bits with new brigtness value
				}
			}
		}
		return false;
	}
}
	
	








void LightLEDSimple(uint16_t LED_Number)
{
	uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF};
	uint8_t unlit[] = {0xE0, 0x00, 0x00, 0x00};
	
	uint8_t startFrame[] = {0x00, 0x00, 0x00, 0x00};
	uint8_t endFrame[] = {0xFF, 0xFF, 0xFF, 0xFF};
	
	for(int byte = 0; byte <= (NumberLEDs + 1); byte++)
	{
		for(int x = 0; x < 4; x++)
		{
			if(byte == 0)
			{
				SSIDataPut(SSIBAddress, startFrame[x]);
			}
			else if(byte == (NumberLEDs + 1))
			{
				SSIDataPut(SSIBAddress, endFrame[x]);
			}
			else if(byte == LED_Number)
			{
				SSIDataPut(SSIBAddress, data[x]);
			}
			else
			{
				SSIDataPut(SSIBAddress, unlit[x]);
			}
		}
	}
}



void LightLED(uint16_t LED_Number, uint8_t Brightness, uint32_t Color)
{
	uint8_t startFrame[] = {0x00, 0x00, 0x00, 0x00};
	uint8_t endFrame[] = {0xFF, 0xFF, 0xFF, 0xFF};
	
	volatile uint32_t blue = ((Color&BLUE) >> 16);
	//UARTprintf("Blue: %x\n", blue);
	volatile uint16_t green = ((Color&GREEN) >> 8);
	//UARTprintf("Green: %x\n", green);
	volatile uint8_t red = (Color&RED);
	//UARTprintf("RED: %x\n", green);
	

	uint8_t data[] = {0xE0|Brightness, blue, green, red};
	uint8_t unlit[] = {0xE0, 0x00, 0x00, 0x00};
	
	for(int byte = 0; byte <= (NumberLEDs +1); byte++)
	{
		for(int x = 0; x < 4; x++)
		{
			if(byte == 0)
			{
				SSIDataPut(SSIBAddress, startFrame[x]);
			}
			else if(byte == (NumberLEDs + 1))
			{
				SSIDataPut(SSIBAddress, endFrame[x]);
			}
			else if(byte == LED_Number)
			{
				SSIDataPut(SSIBAddress, data[x]);
				//UARTprintf("Data %d: ", x);
				//UARTprintf("%x\n", data[x]);
			}
			else
			{
				SSIDataPut(SSIBAddress, unlit[x]);
			}
		}
	}
}


void LightAllLEDsSimple(void)
{
	uint8_t startFrame[] = {0x00, 0x00, 0x00, 0x00};
	uint8_t endFrame[] = {0xFF, 0xFF, 0xFF, 0xFF};
	uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF};
	
	for(int byte = 0; byte <= (NumberLEDs+1); byte++)
	{
		for(int x = 0; x < 4; x++)
		{
			if(byte ==0)
			{
				SSIDataPut(SSIBAddress, startFrame[x]);
			}
			else if(byte == (NumberLEDs +1))
			{
				SSIDataPut(SSIBAddress, endFrame[x]);
			}
			else
			{
				SSIDataPut(SSIBAddress, data[x]);
			}
		}
	}
}

void LightAllLEDs(uint8_t Brightness, uint32_t Color)
{
	uint8_t startFrame[] = {0x00, 0x00, 0x00, 0x00};
	uint8_t endFrame[] = {0xFF, 0xFF, 0xFF, 0xFF};
	
	volatile uint32_t blue = ((Color&BLUE) >> 16);
	//UARTprintf("Blue: %x\n", blue);
	volatile uint16_t green = ((Color&GREEN) >> 8);
	//UARTprintf("Green: %x\n", green);
	volatile uint8_t red = (Color&RED);
	//UARTprintf("RED: %x\n", green);
	

	uint8_t data[] = {0xE0|Brightness, blue, green, red};
	
	for(int byte = 0; byte <= (NumberLEDs +1); byte++)
	{
		for(int x = 0; x < 4; x++)
		{
			if(byte == 0)
			{
				SSIDataPut(SSIBAddress, startFrame[x]);
			}
			else if(byte == (NumberLEDs + 1))
			{
				SSIDataPut(SSIBAddress, endFrame[x]);
			}
			else
			{
				SSIDataPut(SSIBAddress, data[x]);
				//UARTprintf("Data %d: ", x);
				//UARTprintf("%x\n", data[x]);
			}
		}
	}
}


void ClearAllLEDs(void)
{
	uint8_t startFrame[] = {0x00, 0x00, 0x00, 0x00};
	uint8_t endFrame[] = {0xFF, 0xFF, 0xFF, 0xFF};
	uint8_t unlit[] = {0xE0, 0x00, 0x00, 0x00};
	
	for(int byte = 0; byte <= (NumberLEDs+1); byte++)
	{
		for(int x = 0; x < 4; x++)
		{
			if(byte ==0)
			{
				SSIDataPut(SSIBAddress, startFrame[x]);
			}
			else if(byte == (NumberLEDs +1))
			{
				SSIDataPut(SSIBAddress, endFrame[x]);
			}
			else
			{
				SSIDataPut(SSIBAddress, unlit[x]);
			}
		}
	}
}



void LEDBounce()
		{
			uint32_t data[] = {0xFF, 0xCB, 0xC0, 0xFF};
			
			uint32_t unlit[] = {0xE0, 0x00, 0x00, 0x00};
			
			uint32_t endFrame[] = {0xFF, 0xFF, 0xFF, 0xFF};
			
			uint32_t startFrame[] = {0x00, 0x00, 0x00, 0x00};
			
			uint32_t LED = 0;
			uint32_t direction = 1;
			
			while(true)
			{
				
				for(int byte = 0; byte < 32; byte++)
				{
					for(int x = 0; x < 4; x++)
					{
						if(byte == 0)
						{
							SSIDataPut(SSIBAddress, startFrame[x]);
						}
						else if(byte == 32)
						{
							SSIDataPut(SSIBAddress, endFrame[x]);
						}
						else if(byte == LED)
						{
							SSIDataPut(SSIBAddress, data[x]);
						}
						else
						{
							SSIDataPut(SSIBAddress, unlit[x]);
						}
					}
				}
				
				SysCtlDelay(100000);
				if(direction)
				{
					LED++;
				}
				else if(direction == 0)
				{
					LED--;
				}
				
				
				if(LED >= 32)
				{
					direction = 0;
				}
				else if(LED <= 0)
				{
					direction = 1;
				}
	}
	
}




void LightLEDPattern(void)
		{
			
			uint32_t unlit[] = {0xE0, 0x00, 0x00, 0x00};
			
			uint32_t startFrame[] = {0x00, 0x00, 0x00, 0x00};
			
			uint32_t endFrame[] = {0xFF, 0xFF, 0xFF, 0xFF};
			
			uint32_t RedLED = 0;
			uint32_t GreenLED = (NumberLEDs/2);
			uint32_t BlueLED = NumberLEDs;
			uint32_t Reddirection = 1;
			uint32_t Greendirection = 0;
			uint32_t Bluedirection = 1;
			uint32_t RedSpeed = 1;
			uint32_t GreenSpeed = 1;
			uint32_t BlueSpeed = 1;
			
			while(true)
			{
				
				for(int byte = 0; byte < (NumberLEDs+2); byte++)
				{
					for(int x = 0; x < 4; x++)
					{
						if(byte == 0)
						{
							SSIDataPut(SSIBAddress, startFrame[x]);
						}
						else if(byte == 32)
						{
							SSIDataPut(SSIBAddress, endFrame[x]);
						}
						
						else if(byte == BlueLED & x == 1)
						{
							SSIDataPut(SSIBAddress, 0xFF);
						}
						
						else if(byte == GreenLED & x == 2)
						{
							SSIDataPut(SSIBAddress, 0xFF);
						}
						
						else if(byte == RedLED & x == 3)
						{
							SSIDataPut(SSIBAddress, 0xFF);
						}
						else
						{
							SSIDataPut(SSIBAddress, unlit[x]);
						}
					}
				}
				
				SysCtlDelay(100000);
				if(Reddirection)
				{
					RedLED += RedSpeed  ;
				}
				else if(Reddirection == 0)
				{
					RedLED -= RedSpeed;
				}
				
				
				if(RedLED >= 32)
				{
					Reddirection = 0;
					RedLED = 32;
				}
				else if(RedLED <= 0)
				{
					Reddirection = 1;
					RedLED = 0;
				}
				
				if(Greendirection)
				{
					GreenLED +=GreenSpeed;;
				}
				else if(Greendirection == 0)
				{
					GreenLED -= GreenSpeed;
				}
				
				
				if(GreenLED >= 32)
				{
					Greendirection = 0;
					GreenLED = 32;
				}
				else if(GreenLED <= 0)
				{
					Greendirection = 1;
					GreenLED = 0;
				}
				
				if(Bluedirection)
				{
					BlueLED += BlueSpeed;
				}
				else if(Bluedirection == 0)
				{
					BlueLED -= BlueSpeed;
				}
				
				
				if(BlueLED >= 32)
				{
					Bluedirection = 0;
					BlueLED = 32;
				}
				else if(BlueLED <= 0)
				{
					Bluedirection = 1;
					BlueLED = 0;
				}
	}
	

	
}



void LEDBrightnessPattern(void)
	{
		uint32_t data[] = {0xFF, 0x00, 0x00, 0xFF};
		
	//	uint32_t unlit[] = {0x10, 0xFF, 0xFF, 0xFF};
		
		uint32_t endFrame[] = {0xFF, 0xFF, 0xFF, 0xFF};
		
		uint32_t startFrame[] = {0x00, 0x00, 0x00, 0x00};
		uint32_t brightnessMask[] = {0xE0, 0x00, 0x00, 0x00};
		uint32_t brightnessValue = 0x00;
		uint32_t brightness = 1;
		
		
		while(true)
		{
			SysCtlDelay(500000);
			for(int byte = 0; byte < 32; byte++)
			{
				for(int x = 0; x < 4; x++)
				{
					if(byte == 0)
					{
						SSIDataPut(SSIBAddress, startFrame[x]);
					}
					else if(byte == 32)
					{
						SSIDataPut(SSIBAddress, endFrame[x]);
					}
					else
					{
						if(x == 0)
						{
							SSIDataPut(SSIBAddress, (data[x] + brightnessValue) | brightnessMask[x]  );
						}
						else
						{
							SSIDataPut(SSIBAddress, data[x]);
						}
					}
				}
			}
			if(brightness == 0)
				{
					brightnessValue--;
				}
			else if(brightness == 1)
				{
					brightnessValue++;
				}
				
				
				if(brightnessValue >= 32)
				{
					brightness = 0;
					brightnessValue = 32;
				}
				else if((brightnessValue <= 0))
				{
					brightness = 1;
					brightnessValue = 0;
				}
			}
		}

