#ifndef _MIDI_H_
#define _MIDI_H_


//********************************
//**************Notes*************
//********************************



//********************************
//********Instrument Banks********
//********************************
#define MIDI_BANK_DEFAULT 0x00
#define MIDI_BANK_DRUMS1  0x78
#define MIDI_BANK_DRUMS2  0x7f
#define MIDI_BANK_MELODIC 0x79


//********************************
//**********Midi Messages*********
//********************************
#define MIDI_MASTER_VOLUME     0x01
#define MIDI_NOTE_ON           0x90
#define MIDI_NOTE_OFF          0x80
#define MIDI_PROGRAM_MESSAGE   0xC0


#define MIDI_PARAMETER_MESSAGE 0xB0
//these must be sent after MIDI_PARAMETER_MESSAGE
#define MIDI_BANK_SELECT       0x00
#define MIDI_CHANNEL_VOLUME    0x07
#define MIDI_ALL_SOUND_OFF     0x78
#define MIDI_ALL_NOTES_OFF     0x7B


extern uint32_t UARTAddress;

extern void MidiWrite(uint32_t address, uint8_t data);





extern void MidiSetup(uint32_t UARTBase); //sets the UART pins

extern void MidiSetInstrumentBank(uint8_t channel, uint8_t MidiBank);

extern void MidiSetInstrument(uint8_t channel, uint8_t MidiInstrument);

extern void MidiSetVolume(uint8_t volume); //empty function

extern void MidiSetChannelVolume(uint8_t channel, uint8_t volume);

extern void MidiNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);

extern void MidiNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);

extern void MidiAllNotesOff(void); //empty function

#endif // ifndef _MIDI_H_
