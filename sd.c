
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
#include "sd.h"




uint32_t SSIBaseSD = SDSSIBASE;

uint32_t SPICommandSD(uint8_t command_index, uint32_t argument, bool CRC)
{
	uint8_t argument_byte;
	uint8_t CRCPoly = 0x89;
	
	GPIOPinWrite(SDCS_PORT, SDCS_PIN, 0); //select the SD card by setting the chip select low
	
	SSIDataPut(SSIBaseSD, 0x40&command_index);
	for(int byte = 0; byte < (sizeof(argument)/sizeof(argument_byte)); byte++)
	{
		argument_byte = (uint8_t) (argument>>(byte*sizeof(argument_byte)));
		SSIDataPut(SSIBaseSD, argument_byte);
	}
	
	if(CRC)
	{
		SSIDataPut(SSIBaseSD, 0x00);
	}
	else
	{
		SSIDataPut(SSIBaseSD, 0x00);
	}
	GPIOPinWrite(SDCS_PORT, SDCS_PIN, SDCS_PIN);
	
	CRCPoly = 1 + CRCPoly; //gets rid of an error, this doesn't actually do anything and should be removed
	
	return 0; //this should actually return the response data
}



bool InitialiseSD()
{
	uint32_t DataRX[RESPONSE_BYTES]; //place to hold the 5 possible bytes of data
	
	SSIConfigSetExpClk(SSIBaseSD, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 300000, 8);  //needs to be between 100 and 400 kHz
	
	GPIOPinWrite(SDCS_PORT, SDCS_PIN, SDCS_PIN); //set the SD Chip Select high
	
	
	 while(SSIDataGetNonBlocking(SSI0_BASE, &DataRX[0])) //clear SSI Read FIFO
    {

    }
	
	for(int byte = 0; byte <= 10; byte++) //pulse 74 clock cycles (actually 80) with MOSI high
	{
		SSIDataPut(SSIBaseSD, 0xFF); 
	}
	
	GPIOPinWrite(SDCS_PORT, SDCS_PIN, 0); //send command 0
	SSIDataPut(SSIBaseSD, 0x40);
	SSIDataPut(SSIBaseSD, 0x00);
	SSIDataPut(SSIBaseSD, 0x00);
	SSIDataPut(SSIBaseSD, 0x00);
	SSIDataPut(SSIBaseSD, 0x00);
	SSIDataPut(SSIBaseSD, 0x95);
	GPIOPinWrite(SDCS_PORT, SDCS_PIN, SDCS_PIN);
	

	GPIOPinWrite(SDCS_PORT, SDCS_PIN, 0); //get response 
	SSIDataGetNonBlocking(SSIBaseSD, &DataRX[0]);
	GPIOPinWrite(SDCS_PORT, SDCS_PIN, SDCS_PIN);
	
	 while(SSIDataGetNonBlocking(SSI0_BASE, &DataRX[1])) //clear any garbage from FIFO
    {

    }
	
	
	
	if(DataRX[0] & SDFLAG_IDLE_STATE) //if response says the sd card is in idle state.....
	{
		
		GPIOPinWrite(SDCS_PORT, SDCS_PIN, 0); //send command 8
		SSIDataPut(SSIBaseSD, 0x48);
		SSIDataPut(SSIBaseSD, 0x00);
		SSIDataPut(SSIBaseSD, 0x00);
		SSIDataPut(SSIBaseSD, 0x01);
		SSIDataPut(SSIBaseSD, 0xAA);
		SSIDataPut(SSIBaseSD, 0x0F);
		GPIOPinWrite(SDCS_PORT, SDCS_PIN, SDCS_PIN);
		
		GPIOPinWrite(SDCS_PORT, SDCS_PIN, 0); //get response 
		SSIDataGetNonBlocking(SSIBaseSD, &DataRX[0]);
		GPIOPinWrite(SDCS_PORT, SDCS_PIN, SDCS_PIN);
	
		if(DataRX[0]&SDFLAG_ILLEGAL_COMMAND)
		{
			
			while(SSIDataGetNonBlocking(SSI0_BASE, &DataRX[1])) //clear any garbage from FIFO
			{

			}
			
			//send command 58
		}
		
		
		for(int rxbyte = 0; rxbyte < RESPONSE_BYTES; rxbyte++)
		{
			GPIOPinWrite(SDCS_PORT, SDCS_PIN, 0);
			SSIDataGetNonBlocking(SSIBaseSD, &DataRX[rxbyte]);
			GPIOPinWrite(SDCS_PORT, SDCS_PIN, SDCS_PIN);
		}
		
		
		
	}

	//set CS Low and send this command
	//0x40 0x00 0x00 0x00 0x00 0x95
	//read response, if 0x01 then it works fine
	
	return true;
}
