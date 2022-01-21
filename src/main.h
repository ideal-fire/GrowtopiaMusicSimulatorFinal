#define VERSIONNUMBER 3
#define VERSIONSTRING "v1.5"

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define s8 int8_t
#define s16 int16_t
#define s32 int32_t

// Always add elements right before U_RESERVED, otherwise saved hotkey config breaks.
enum uiID{
	U_NOTUNIQUE,
	U_SELICON,
	U_PLAY,
	U_SAVE,
	U_LEFT,
	U_RIGHT,
	U_YPLAY,
	U_UPBUTTON,
	U_DOWNBUTTON,
	U_BPM,
	U_SIZE,
	U_SETTINGS,
	U_COUNT,
	U_CREDITS,
	U_LOAD,
	U_KEYCONF,
	U_METADATA,
	U_THEME,
	//
	U_BACK,
	U_INFO,
	U_VOL,
	//
	U_SCRIPTBUTTON,
	U_MOREUIBUTTON,
	U_ZOOMIN,
	U_ZOOMOUT,
	U_ERASER,
	//
	U_RSELECT,
	U_CUT,
	U_COPY,
	U_PASTE,

	U_RESERVED // Always last
};

typedef struct{
	u8 id;
	u8* extraData; // For audio gears, this could be their data. For repeat notes, it could be temp data about if they've been used or not.
		// For repeat notes, it's NULL if they have not been used or 1 if they have
		// For audio gears, it's a malloc'd array of AUDIOGEARSPACE*sizeof(u8)*2+1 size. The first ten bytes are a note id, its y position, the next note id, its y position, and so on. The last byte, the 11th one, is the audio gear's volume, with 100 being the max and default.
}noteSpot;
typedef struct{
	char letter;
	char accidental;
}noteInfo;

int gearBuffSize();
void freeNote(noteSpot* me);

extern u16 totalNotes;

extern uint8_t audioGearID;
#define AUDIOGEARSPACE 5
