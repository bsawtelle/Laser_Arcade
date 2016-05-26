#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/fpu.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/interrupt.h"
#include "LaserBoardDrivers/LEDStrip.h"
#include "LaserBoardDrivers/midi.h"
#include "LaserBoardDrivers/Displays.h"
#include "LaserBoardDrivers/LinkedList.h"
#include "LaserBoardDrivers/Midi.h"

/*
 * main.c
 */
#include <stdarg.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "driverlib/i2c.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

unsigned char playerID = 0;

#define PULSES_PERFRAME 133333 //controls FPS of the game


#define LED_PERIPH SYSCTL_PERIPH_GPIOF
#define LED_BASE GPIO_PORTF_BASE
#define RED_LED GPIO_PIN_1

#define Button_PERIPH SYSCTL_PERIPH_GPIOF
#define ButtonBase GPIO_PORTF_BASE
#define Button GPIO_PIN_4
#define ButtonInt GPIO_INT_PIN_4

#define EXPANDER0_ADDRESS       0x20
#define EXPANDER0INT_PERIPH     SYSCTL_PERIPH_GPIOE
#define EXPANDER0INT_BASE       GPIO_PORTE_BASE
#define EXPANDER0INT_GPIO       GPIO_PIN_0
#define EXPANDER0INT_PIN        GPIO_INT_PIN_0

#define EXPANDER1_ADDRESS       0x21
#define EXPANDER1INT_PERIPH     SYSCTL_PERIPH_GPIOE
#define EXPANDER1INT_BASE       GPIO_PORTE_BASE
#define EXPANDER1INT_GPIO       GPIO_PIN_1
#define EXPANDER1INT_PIN        GPIO_INT_PIN_1

#define EXPANDER2_ADDRESS       0x22
#define EXPANDER2INT_PERIPH     SYSCTL_PERIPH_GPIOE
#define EXPANDER2INT_BASE       GPIO_PORTE_BASE
#define EXPANDER2INT_GPIO       GPIO_PIN_2
#define EXPANDER2INT_PIN        GPIO_INT_PIN_2

#define EXPANDER3_ADDRESS       0x23
#define EXPANDER3INT_PERIPH     SYSCTL_PERIPH_GPIOE
#define EXPANDER3INT_BASE       GPIO_PORTE_BASE
#define EXPANDER3INT_GPIO       GPIO_PIN_3
#define EXPANDER3INT_PIN        GPIO_INT_PIN_3

#define EXPANDER4_ADDRESS       0x24
#define EXPANDER4INT_PERIPH     SYSCTL_PERIPH_GPIOE
#define EXPANDER4INT_BASE       GPIO_PORTE_BASE
#define EXPANDER4INT_GPIO       GPIO_PIN_4
#define EXPANDER4INT_PIN        GPIO_INT_PIN_4

#define EXPANDER5_ADDRESS       0x25
#define EXPANDER5INT_PERIPH     SYSCTL_PERIPH_GPIOE
#define EXPANDER5INT_BASE       GPIO_PORTE_BASE
#define EXPANDER5INT_GPIO       GPIO_PIN_5
#define EXPANDER5INT_PIN        GPIO_INT_PIN_5

#define EXPANDER6_ADDRESS       0x26
#define EXPANDER6INT_PERIPH     SYSCTL_PERIPH_GPIOB
#define EXPANDER6INT_BASE       GPIO_PORTB_BASE
#define EXPANDER6INT_GPIO       GPIO_PIN_4
#define EXPANDER6INT_PIN        GPIO_INT_PIN_4

#define EXPANDER7_ADDRESS       0x27
#define EXPANDER7INT_PERIPH     SYSCTL_PERIPH_GPIOB
#define EXPANDER7INT_BASE       GPIO_PORTB_BASE
#define EXPANDER7INT_GPIO       GPIO_PIN_5
#define EXPANDER7INT_PIN        GPIO_INT_PIN_5


#define MIDI_RESET_PORT					GPIO_PORTB_BASE
#define MIDI_RESET_PIN          GPIO_PIN_7
#define MIDI_UART_PORT          GPIO_PORTB_BASE
#define MIDI_UART_PIN						GPIO_PIN_1

#define PLAYER_MUX_GPIO_PERIPH_0    SYSCTL_PERIPH_GPIOF
#define PLAYER_MUX_GPIO_BASE_0      GPIO_PORTF_BASE
#define PLAYER_MUX_PIN_0            GPIO_PIN_0
 
#define PLAYER_MUX_GPIO_PERIPH_1  		SYSCTL_PERIPH_GPIOB
#define PLAYER_MUX_GPIO_BASE_1    		GPIO_PORTB_BASE
#define PLAYER_MUX_PIN_1         		  GPIO_PIN_6

#define PLAYER1            		  0x00
#define PLAYER2            		  0x01
#define PLAYER3									0x02
#define PLAYER4                 0x03


 uint8_t const ExpanderAddresses[8] = {EXPANDER0_ADDRESS, EXPANDER1_ADDRESS, EXPANDER2_ADDRESS, EXPANDER3_ADDRESS, EXPANDER4_ADDRESS,
											EXPANDER5_ADDRESS, EXPANDER6_ADDRESS, EXPANDER7_ADDRESS};
 
uint8_t const ExpanderIntPins[8] = {EXPANDER0INT_PIN, EXPANDER1INT_PIN, EXPANDER2INT_PIN, EXPANDER3INT_PIN, EXPANDER4INT_PIN,
											EXPANDER5INT_PIN, EXPANDER6INT_PIN, EXPANDER7INT_PIN};

volatile uint8_t expander_state[8];
volatile uint8_t past_state[8];
uint8_t expander_statemask = 0x3f; //lowest 6 bits
volatile uint8_t note;
volatile bool new_hit = false;
volatile bool advance_game_loop = false;
volatile uint32_t color_select;
uint32_t colors[] = {RED, YELLOW, BLUE, GREEN, WHITE, YELLOW, CYAN};
volatile uint8_t count = 0;
											
void Timer0IntHandler(void){

	//UARTprintf("Time 0\n");
	// Clear the timer interrupt.
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	//allow loop to advance
	advance_game_loop = true;
}



void
InitConsole(void)
{
    //
    // Enable GPIO port A which is used for UART0 pins.
    // Enable UART0 so that we can configure the clock.
    //    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	
    //
    // Configure the pin muxing for UART0 functions on port A0 and A1.
    // This step is not necessary if your part does not support pin muxing.
    // Select the alternate (UART) function for these pins.
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);


    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}




// function to initialie I2C using pins B2 and B3
void InitI2C0(void)
{
    //enable I2C module 0
		
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

    //reset module
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    //enable GPIO peripheral that contains I2C 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);
		GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    // Enable and initialize the I2C0 master module.  Use the system clock for
    // the I2C0 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);

    //clear I2C FIFOs
    HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
}


void SelectPlayer(uint8_t player)
{
	uint32_t pinState0;
	uint32_t pinState1;
	
	if (player == PLAYER1)
	{
		pinState0 = 0;
		pinState1 = 0;
	}		
	else if (player == PLAYER2)
	{
		pinState0 = 1;
		pinState1 = 0;
	}
	
	GPIOPinWrite(PLAYER_MUX_GPIO_BASE_0, PLAYER_MUX_PIN_0, pinState0);
	GPIOPinWrite(PLAYER_MUX_GPIO_BASE_1, PLAYER_MUX_PIN_1, pinState1);
}

/* --------simple version------------*/
void getLED(uint8_t player)
{
	ClearLEDMap();
	bool exit = false;
	uint32_t color; //player color
	int scale[6] = {0, 2, 5, 7, 9, 12}; 
	//change LED color to whatever player 
	if( player == PLAYER1)
	{
		color = RED;
	}
	else if (player == PLAYER2)
	{
		color = BLUE;
	}
	
	for(int expander = 0; expander < 8; expander++)
	{
		if((expander_state[expander]&expander_statemask) == (past_state[expander]&expander_statemask))
		{
			continue;
		}
		new_hit = true;
		uint8_t state = ~expander_state[expander];
		for(uint8_t bit = 0; bit < 6; bit++)
		{
			for(uint8_t y = (expander*3); y < (expander+1)*3; y++)
			{
			
				for(uint8_t x = (bit)*5; x < (bit+1)*5; x++)
				{
					if(state & (1<<bit))
					{
						note = ((11-expander)*12) + scale[bit];
						SetLEDValue(x, y, 0xFF, color_select);
						exit = true;
					}
				}
			}
			if(exit)
			{
				return;
			}
		}
		expander_state[expander] = 0xFF; //reset the address after it's done
	}
}

int main(void)
{
 uint32_t pui32DataRx[5];
 uint8_t player = PLAYER1;
 //Set the clock to 16Mhz
 SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	IntMasterDisable();
  /*
    No need to enable the button peripheral since it's the same as the LED
    in this case
  */
  SysCtlPeripheralEnable(LED_PERIPH);
  SysCtlDelay(3);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(GPIO_PA2_SSI0CLK);
  GPIOPinConfigure(GPIO_PA3_SSI0FSS);
  GPIOPinConfigure(GPIO_PA4_SSI0RX);
  GPIOPinConfigure(GPIO_PA5_SSI0TX);
  GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 |
                   GPIO_PIN_2);
	
	SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                      SSI_MODE_MASTER, 1000000, 8);
											 
											 
	SSIEnable(SSI0_BASE);
	
	
	//timer interrupt setup
	FPULazyStackingEnable();
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER0_BASE, TIMER_A, PULSES_PERFRAME);
	TimerIntRegister(TIMER0_BASE, TIMER_CFG_PERIODIC, Timer0IntHandler);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	
	
	//player setup, both have the same pin block (GPIO BASEA)
	SysCtlPeripheralEnable(PLAYER_MUX_GPIO_PERIPH_0);
	SysCtlPeripheralEnable(PLAYER_MUX_GPIO_PERIPH_1);
	GPIOPinTypeGPIOOutput(PLAYER_MUX_GPIO_BASE_0, PLAYER_MUX_PIN_0);
	GPIOPinTypeGPIOOutput(PLAYER_MUX_GPIO_BASE_1, PLAYER_MUX_PIN_1);
	
  
	//MIDI declarations
	SysCtlPeripheralEnable(MIDI_RESET_PORT);
	SysCtlPeripheralEnable(MIDI_UART_PORT);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
	GPIOPinConfigure(GPIO_PB1_U1TX);
	GPIOPinTypeGPIOOutput(MIDI_RESET_PORT, MIDI_RESET_PIN);
	GPIOPinTypeUART(MIDI_UART_PORT, MIDI_UART_PIN);
	
	GPIOPinWrite(MIDI_RESET_PORT, MIDI_RESET_PIN, 0);
	SysCtlDelay(10000);
	GPIOPinWrite(MIDI_RESET_PORT, MIDI_RESET_PIN, MIDI_RESET_PIN);
	SysCtlDelay(10000);
	
	MidiSetup(UART1_BASE);
	
	UARTClockSourceSet(UART1_BASE, UART_CLOCK_PIOSC);
	UARTConfigSetExpClk(UART1_BASE, 16000000, 31250,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
	
	LEDManipSetup(SSI0_BASE, 30, 24);
	UpdateLEDState();
	
	volatile uint32_t data = 1;
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_PLL | SYSCTL_OSC_INT | SYSCTL_XTAL_16MHZ);
	InitI2C0();
	
	
	InitConsole();
	//InitialiseEvents(); //create the root linked list	
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A); //start timer count
	
	IntMasterEnable();
		
		
		
		MidiSetInstrumentBank(0, MIDI_BANK_MELODIC);
		MidiSetInstrument(0, 1);
		
		
		
	while(true)
	{
		
		volatile uint8_t expanderReadArray[8];
		SelectPlayer(player);
		
		for(int expander = 0; expander < 8; expander++)
		{
			
			volatile uint8_t expander_read = 0xFF;
					I2CMasterSlaveAddrSet(I2C0_BASE, ExpanderAddresses[expander], false); //send data to the expander
					I2CMasterDataPut(I2C0_BASE, 0xFF); //send FF to turn all signals to inputs
					I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND); //send that data 
					
					while(I2CMasterBusy(I2C0_BASE))
					{				
					
						I2CMasterSlaveAddrSet(I2C0_BASE, ExpanderAddresses[expander], true); //read address
						expander_read = I2CMasterDataGet(I2C0_BASE); //get the state of the expander at the address
						
						
						if(expander_read != 0xff)
						{
							expanderReadArray[expander] = expander_read;
						}
						
							UARTprintf("Expander %d: ", expander);
							UARTprintf("%d\n", expander_state[expander]); 
						
						
					}
		
					for(int expander = 0; expander < 8; expander++)
					{
						if(expander != 0)
						{
							expander_state[expander - 1] = expanderReadArray[expander];
						}
						
						else if(expander == 0)
						{
							expander_state[7] = expanderReadArray[expander];
						}
					}
					
		}
		
	
		
		getLED(player);

			UpdateLEDState();
			
			advance_game_loop = false;
		
		if(new_hit)
		{
			MidiNoteOn(0, note, 127);
		}
		
	
	for(int expander = 0; expander < 8; expander++)
		{
			past_state[expander] = expander_state[expander];
			count++;
		}
	new_hit = false;
		
		color_select = colors[count%7];
	SysCtlDelay(100000);

	}
}
