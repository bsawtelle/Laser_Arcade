#include <stdbool.h>
#include <stdint.h>
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/rom.h"
#include "Midi.h"

uint32_t UARTAddress;

void MidiSetup(uint32_t UARTBase)
{
	UARTAddress = UARTBase;
}

void MidiWrite(uint32_t address, uint8_t data)
{
	while(!UARTCharsAvail(address))
	{
		
	}
	UARTCharPut(address, data);
}


void MidiSetInstrumentBank(uint8_t channel, uint8_t MidiBank)
{
	MidiWrite(UARTAddress, MIDI_PARAMETER_MESSAGE|channel);
	MidiWrite(UARTAddress, MIDI_BANK_SELECT);
	MidiWrite(UARTAddress, MidiBank);
}

void MidiSetInstrument(uint8_t channel, uint8_t MidiInstrument)
{
	MidiWrite(UARTAddress, MIDI_PROGRAM_MESSAGE|channel);
	MidiWrite(UARTAddress, MidiInstrument);
}

void MidiSetVolume(uint8_t volume)
{
	
}

void MidiSetChannelVolume(uint8_t channel, uint8_t volume)
{
	MidiWrite(UARTAddress, MIDI_PARAMETER_MESSAGE|channel);
	MidiWrite(UARTAddress, MIDI_CHANNEL_VOLUME);
	MidiWrite(UARTAddress, volume);
}

void MidiNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
	MidiWrite(UARTAddress, MIDI_NOTE_ON|channel);
	MidiWrite(UARTAddress, note);
	MidiWrite(UARTAddress, velocity);
}

void MidiNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
{
	MidiWrite(UARTAddress, MIDI_NOTE_OFF|channel);
	MidiWrite(UARTAddress, note);
	MidiWrite(UARTAddress, velocity);
}

void MidiAllNotesOff()
{
	
}
