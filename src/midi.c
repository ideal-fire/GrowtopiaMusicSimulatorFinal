#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "midi.h"

typedef unsigned char U8;
typedef unsigned int U32;
typedef unsigned short U16;

// the information for each possible note tile
typedef struct{
	U8 baseId;
	U8 program;
	U8 channel;
}s_instrument;

// The total number of instruments
#define INSTR_COUNT 17
// The total number of note tile types ()
#define INSTR_MAX 35

enum{
	INSTR_NONE,
	INSTR_PIANO,
	INSTR_BASS,
	INSTR_DRUM,
	INSTR_BLANK,
	INSTR_SAX,
	INSTR_REPEAT0,
	INSTR_REPEAT1,
	INSTR_SPOOKY,
	INSTR_GEAR,
	INSTR_FLUTE,
	INSTR_FESTIVE,
	INSTR_GUITAR,
	INSTR_VIOLIN,
	INSTR_LYREM,
	INSTR_GUITAR2,
	INSTR_TRUMPET
};

// An array of the instruments and their fields
// {tileID, midiProgram, midiChannel}
static s_instrument INSTRUMENTS[INSTR_COUNT + 1] = {
	(s_instrument){0, 0,-1}, // null
	(s_instrument){1, 0, 1}, // piano
	(s_instrument){4, 32, 2}, // bass
	(s_instrument){7, 0, 9}, // drum
	(s_instrument){8, 0, -1}, // blank
	(s_instrument){9, 42, 4}, // sax
	(s_instrument){12, 0,-1}, // repeat0
	(s_instrument){13, 0,-1}, // repeat1
	(s_instrument){14, 0,-1}, // spooky
	(s_instrument){15, 0,-1}, // gear
	(s_instrument){16, 73, 6}, // flute
	(s_instrument){19, 0,-1}, // festive
	(s_instrument){20, 24, 8}, // guitar
	(s_instrument){23, 40, 10}, // violin
	(s_instrument){26, 46, 11}, // lyrem
	(s_instrument){29, 27, 12}, // guitar2
	(s_instrument){32, 56, 13}, // trumpet
	(s_instrument){INSTR_MAX, 0, -1} // CAP
};

// convert letter/position to midi note value
static int NOTE_MAP[7] = {0, 2, 4, 5, 7, 9, 11};
// convert tile type to their instrument
static s_instrument *INSTR_MAP[INSTR_MAX] = {NULL};
// find program by channel
static U8 CHANNEL_MAP[16] = {0};


static void midi_writeNoteOn(U8 *out, U8 channel, U8 note, U8 volume){
	out[0] = 128 | (MIDI_NOTE_ON << 4) | (channel);
	out[1] = note;
	out[2] = volume;
}

static void midi_writeNoteOff(U8 *out, U8 channel, U8 note){
	out[0] = 128 | (MIDI_NOTE_OFF << 4) | (channel);
	out[1] = note;
	out[2] = 127;
}

static void midi_writeSetTemp(U8 *out, U32 microPerBeat){
	out[0] = 255;
	out[1] = MIDI_SYS_SET_TEMPO;
	out[2] = 3;
	out[3] = (microPerBeat >> 16) & 255;
	out[4] = (microPerBeat >> 8) & 255;
	out[5] = (microPerBeat >> 0) & 255;
}

static void midi_writeTimeSignature(U8 *out, U8 top, U8 bot){
	out[0] = 255;
	out[1] = MIDI_SYS_TIME_SIGNATURE;
	out[2] = 4;
	out[3] = top;
	for(out[4] = 0; bot > 1; out[4]++, bot >>= 1);
	out[5] = 24;
	out[6] = 8;
}

static int midi_writeVarNum(FILE *file, U32 n){
	if(n >> 28) printf("midi_writeVarNum(): max reached (%d)\n", n);

	U8 v[4];
    U8 I = 3;
    do{
        v[I] = (n & 127);
        n = n >> 7;
        if(I != 3) v[I] |= 128;
    }while((I -= 1) >= 0 && n != 0);

    fwrite(v + I + 1, 1, 3 - I, file);
    return 3 - I;
}

static void midi_putInt(FILE *file, U32 num){
	U32 flip = num << 24 | (num << 8 & 0x00FF0000) | (num >> 8 & 0xFF00) | num >> 24;
	fwrite(&flip, 4, 1, file);
}

static void midi_putShort(FILE *file, U16 num){
	U16 flip = num << 8 | num >> 8;
	fwrite(&flip, 2, 1, file);
}


static void instr_setup(){
	if(INSTR_MAP[0] != NULL) return;

	int I, II;
	for(I = 0; I < INSTR_COUNT; I++){
		for(II = INSTRUMENTS[I].baseId; II < INSTRUMENTS[I+1].baseId; II++){
			INSTR_MAP[II] = &INSTRUMENTS[I];
		}

		if(INSTRUMENTS[I].channel != -1)
			CHANNEL_MAP[INSTRUMENTS[I].channel] = INSTRUMENTS[I].program;
	}
}

static int instr_toNote(int instr, int y){
	y = 13 - y; // y measures top down; this flips it

	if(instr == INSTRUMENTS[INSTR_DRUM].baseId){
		// -- drum special case

		switch(y % 7){
			case 0: return 49; // "Crash Cymbal 1"
			case 1: return 43; // "High Floor Tom"
			case 2: return 45; // "Low Tom"
			case 3: return 38; // "Acoustic Snare"
			// case 3: return 40; // "Electric Snare"
			case 4: return 44; // "Pedal Hi-Hat "
			case 5: return 39; // "Hand Clap"
			case 6: return 35; // "Acoustic Bass Drum"
			default: return 0;
		}

	}else{
		// -- normal

		// octave value (bass special case)
		int octave = 60;
		if(instr >= INSTRUMENTS[INSTR_BASS].baseId && instr < INSTRUMENTS[INSTR_BASS+1].baseId)
			octave = 24;

		// accidentals
		int baseInstr = INSTR_MAP[instr]->baseId;
		if(instr == baseInstr + 1) // sharp
			octave++;
		else if(instr == baseInstr + 2) // flat
			octave--;

		return octave + NOTE_MAP[y % 7] + (y / 7)*12;
	}

}


static void gmsf_resetLoops(s_gmsf *song, int x0, int x1){
	int x, y;
	for(y = 0; y < song->tall; y++){
		for(x = x0; x <= x1; x++){
			if(song->notes[y][x].id == INSTRUMENTS[INSTR_REPEAT1].baseId) // repeat1
				song->notes[y][x].extraData = NULL0;
		}
	}
}


// noteLength : 	The duration of the notes played, in milliseconds
// loops : 			The number of times to loop the song. 0 -> plays once
int gmsf_makeMIDI(FILE* file, s_gmsf *song, unsigned short noteDuration, int loops){
	instr_setup();

	U8 inst[8];
	U8 lenDiv = MIDI_SCALE * 4 * song->bpm * noteDuration / (60 * 1000);

	// main chunk thing
	fwrite(MIDIMAGIC, 1, 4, file);
	midi_putInt(file, 6);
	midi_putShort(file, 0);
	midi_putShort(file, 1);
	midi_putShort(file, MIDI_SCALE * 4);

	// track thing
	fwrite("MTrk", 1, 4, file);
	U32 holdPos = ftell(file);
	U32 trackLen = 0;
	midi_putInt(file, 0);

	// write tempo
	trackLen += midi_writeVarNum(file, 0);
	midi_writeSetTemp(inst, 1000000 * 60 / song->bpm);
	fwrite(inst, 1, 6, file);
	trackLen += 6;

	// write time signature
	trackLen += midi_writeVarNum(file, 0);
	midi_writeTimeSignature(inst, 4, 4);
	fwrite(inst, 1, 7, file);
	trackLen += 7;

	// write instruments
	for(int I = 0; I < 16; I++){
		trackLen += midi_writeVarNum(file, 0) + 2;
		inst[0] = 128 | (MIDI_PROGRAM_CHANGE << 4) | I;
		inst[1] = CHANNEL_MAP[I];
		fwrite(inst, 1, 2, file);
	}

	// do note things
	U16 noteOff[16][128];
	U32 curTime = 0, lastTime = 0;
	U32 nextX, x, y, I, II;

	memset(noteOff, 0, sizeof(noteOff));

	// find the end of the song
	U16 songLen = 0;
	for(I = 0; I < song->wide; I++){
		for(II = 0; II < song->tall; II++){
			if(song->notes[II][I].id > 0){
				songLen = I + 1;
				break;
			}
		}
	}

	// write the actual song
	 LOOP:
	gmsf_resetLoops(song, 0, songLen - 1);
	for(x = 0, nextX = 1; x < songLen; x = nextX, nextX++, curTime += MIDI_SCALE){

		// block note offs
		for(y = 0; y < song->tall; y++){
			if(INSTR_MAP[song->notes[y][x].id]->channel != 255){
				int channel = INSTR_MAP[song->notes[y][x].id]->channel;
				int note = instr_toNote(song->notes[y][x].id, y);
				if(noteOff[channel][note] > curTime)
					noteOff[channel][note] = curTime;

			}else if(song->notes[y][x].id == INSTRUMENTS[INSTR_GEAR].baseId){

				// do gear things
				U8 *arr = song->notes[y][x].extraData;
				for(I = 0; I < AUDIOGEARSPACE; I++){
					if(arr[I*2] != 0){
						int channel = INSTR_MAP[arr[I*2]]->channel;
						int note = instr_toNote(arr[I*2], arr[I*2+1]);
						if(noteOff[channel][note] > curTime)
							noteOff[channel][note] = curTime;
					}
				}


			}
		}

		// write initial note offs. Don't turn all the notes off at t = 0
		if(curTime > 0){
			for(I = 0; I < 16; I++){
				for(II = 0; II < 128; II++){
					if(noteOff[I][II] == curTime){
						int dtime = curTime - lastTime;

						midi_writeNoteOff(inst, I, II);
						trackLen += midi_writeVarNum(file, dtime) + 3;
						fwrite(inst, 1, 3, file);
						lastTime += dtime;
					}
				}
			}
		}

		// write the note ons
		for(y = 0; y < song->tall; y++){
			if(song->notes[y][x].id < INSTR_MAX){

				if(INSTR_MAP[song->notes[y][x].id]->channel != 255){
					// --- normal notes

					int channel = INSTR_MAP[song->notes[y][x].id]->channel;
					int note = instr_toNote(song->notes[y][x].id, y);
					int dtime = curTime - lastTime;

					midi_writeNoteOn(inst, channel, note, 127);
					trackLen += midi_writeVarNum(file, dtime) + 3;
					fwrite(inst, 1, 3, file);
					lastTime += dtime;

					noteOff[channel][note] = curTime + lenDiv;

				}else if(song->notes[y][x].id == INSTRUMENTS[INSTR_GEAR].baseId){
					// --- gears

					U8 *arr = song->notes[y][x].extraData;
					U8 vol = arr[AUDIOGEARSPACE*2] / 100.0 * 127;

					for(I = 0; I < AUDIOGEARSPACE; I++){
						if(arr[I*2] != 0){
							int channel = INSTR_MAP[arr[I*2]]->channel;
							int note = instr_toNote(arr[I*2], arr[I*2+1]);
							int dtime = curTime - lastTime;

							midi_writeNoteOn(inst, channel, note, vol);
							trackLen += midi_writeVarNum(file, dtime) + 3;
							fwrite(inst, 1, 3, file);
							lastTime += dtime;

							noteOff[channel][note] = curTime + lenDiv;
						}
					}

				}else if(song->notes[y][x].id == INSTRUMENTS[INSTR_REPEAT1].baseId && nextX > x && song->notes[y][x].extraData == NULL0){ // repeat1
					// --- repeats

					// find the start of the loop
					for(I = x - 1; I > 0; I--)
						if(song->notes[y][I].id == INSTRUMENTS[INSTR_REPEAT0].baseId)
							break;
					if(I >= 0) nextX = I;

					// reset the contained loops
					gmsf_resetLoops(song, nextX, x - 1);

					// mark this loop as used
					song->notes[y][x].extraData = NULL1;
				}
			}
		}

		// write middle note offs (no need to sort because will all happen at the same time)
		for(I = 0; I < 16; I++){
			for(II = 0; II < 128; II++){
				if(noteOff[I][II] > curTime && noteOff[I][II] < curTime + MIDI_SCALE){
					int dtime = curTime - lastTime;

					midi_writeNoteOff(inst, I, II);
					trackLen += midi_writeVarNum(file, dtime) + 3;
					fwrite(inst, 1, 3, file);
					lastTime += dtime;
				}
			}
		}
	}

	if(loops > 0){
		loops--;
		goto LOOP;
	}

	// write final note offs
	for(x = 0; x < noteDuration + MIDI_SCALE; x += MIDI_SCALE, curTime += MIDI_SCALE){
		for(I = 0; I < 16; I++){
			for(II = 0; II < 128; II++){
				if(noteOff[I][II] >= curTime && noteOff[I][II] < curTime + MIDI_SCALE){
					int dtime = curTime - lastTime;

					midi_writeNoteOff(inst, I, II);
					trackLen += midi_writeVarNum(file, dtime) + 3;
					fwrite(inst, 1, 3, file);
					lastTime += dtime;
				}
			}
		}
	}

	// write the thing that ends the track
	trackLen += midi_writeVarNum(file, 0) + 3; // delay
	fputc(255, file); // MIDI_SYSTEM_EXCLUSIVE
	fputc(MIDI_SYS_END_OF_TRACK, file);
	fputc(0, file);

	// update track length
	unsigned int curPos = ftell(file);
	fseek(file, holdPos, SEEK_SET);
	midi_putInt(file, trackLen);
	fseek(file, curPos, SEEK_SET);


	fclose(file);
	return 0;
}
