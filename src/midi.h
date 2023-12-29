#ifndef MIDI_H
#define MIDI_H

#include "main.h"

#define MIDIMAGIC "MThd"

#define MIDI_NOTE_OFF           0
#define MIDI_NOTE_ON            1
#define MIDI_AFTER_TOUCH        2
#define MIDI_CONTROL_CHANGE     3
#define MIDI_PROGRAM_CHANGE     4
#define MIDI_CHANNEL_PRESSURE   5
#define MIDI_PITCH_BEND         6
#define MIDI_SYSTEM_EXCLUSIVE   7

#define MIDI_SYS_SEQUENCE           0
#define MIDI_SYS_TEXT               1
#define MIDI_SYS_COPYRIGHT          2
#define MIDI_SYS_TRACK_NAME         3
#define MIDI_SYS_INSTRUMENT_NAME    4
#define MIDI_SYS_LYRICS             5
#define MIDI_SYS_MARKER             6
#define MIDI_SYS_CUE_POINT          7
#define MIDI_SYS_CHANNEL_PREFIX     32
#define MIDI_SYS_END_OF_TRACK       47
#define MIDI_SYS_SET_TEMPO          81
#define MIDI_SYS_SMPTE_OFFSET       84
#define MIDI_SYS_TIME_SIGNATURE     88
#define MIDI_SYS_KEY_SIGNATURE      89
#define MIDI_SYS_SEQUENCER_SPECIFIC 127

#define MIDI_SCALE 10

#define NULL0 ((void*)0)
#define NULL1 ((void*)1)

typedef struct{
	unsigned char id;
	unsigned char* extraData;
}s_gmsf_note;

typedef struct{
	short bpm;
	short wide;
	short tall;
	noteSpot** notes;
}s_gmsf;


// noteLength : 	The duration of the notes played, in milliseconds
// loops : 			The number of times to loop the song. 0 -> plays once
int gmsf_makeMIDI(FILE* file, s_gmsf *song, unsigned short noteDuration, int loops);

#endif // MIDI_H
