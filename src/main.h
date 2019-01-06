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
	U_RESERVED // Always last
};

typedef void(*voidFunc)();
typedef struct{
	CrossTexture* image;
	voidFunc activateFunc;
	u8 uniqueId;
}uiElement;
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
typedef struct{
	u8 id;
	SDL_Keycode boundKey;
}hotkeyConf;

void* soundPlayerThread(void* data);
void addChar(char* _sourceString, char _addChar);
void easyMessage(char* _newMessage);
char easyChoice(char* _title, char* _redChoice, char* _greenChoice);
void _addNumberInput(long* _outNumber, char* _outBuffer, int _addNumber);
void _delNumberInput(long* _outNumber, char* _outBuffer);
void _placeNoteLow(int _x, int _y, u8 _noteId, u8 _shouldPlaySound, noteSpot** _passedSong);
uiElement* addUI();
void audioGearGUI(u8* _gearData);
u16 bitmpTextWidth(char* _passedString);
u32 bpmFormula(u32 re);
void centerAround(u32 _passedPosition);
void clearSong();
void controlLoop();
void die(const char* message);
void doUsualDrawing();
void drawImageScaleAlt(CrossTexture* _passedTexture, int _x, int _y, double _passedXScale, double _passedYScale);
void drawPlayBar(int _x);
void drawSong(noteSpot** _songToDraw, int _drawWidth, int _drawHeight, int _xOffset, int _yOffset);
void drawString(const char* _passedString, int _x, int _y);
void drawUI(uiElement* _passedUIBar);
void findMaxX();
s16 fixShort(s16 _passedShort);
int fixX(int _x);
int fixY(int _y);
long getNumberInput(const char* _prompt, long _defaultNumber);
uiElement* getUIByID(s16 _passedId);
void goodLuaDofile(lua_State* passedState, char* _passedFilename, char _doCrash);
void goodPlaySound(CROSSSFX* _passedSound, int _volume);
char init();
void int2str(char* _outBuffer, int _inNumber);
char isBigEndian();
int L_addNote(lua_State* passedState);
int L_addUI(lua_State* passedState);
int L_deleteUI(lua_State* passedState);
int L_getMobile(lua_State* passedState);
int L_loadImage(lua_State* passedState);
int L_loadSound(lua_State* passedState);
int L_setBgParts(lua_State* passedState);
int L_setBigBg(lua_State* passedState);
int L_setSpecialID(lua_State* passedState);
int L_swapUI(lua_State* passedState);
CrossTexture* loadEmbeddedPNG(const char* _passedFilename);
int main(int argc, char *argv[]);
void noteUIControls();
void pageTransition(int _destX);
void placeNote(int _x, int _y, u16 _noteId);
void playAtPosition(s32 _startPosition);
void playColumn(s32 _columnNumber);
char* possiblyFixPath(const char* _passedFilename, char _shouldFix);
void pushLuaFunctions();
void* recalloc(void* _oldBuffer, int _oldSize, int _newSize);
void resetPlayState();
void resetRepeatNotes(int _startResetX, int _endResetX);
u32 reverseBPMformula(u32 re);
char* selectLoadFile();
void setSongWidth(noteSpot** _passedArray, u16 _passedOldWidth, u16 _passedWidth);
void setSongXOffset(int _newValue);
void setTotalNotes(u16 _newTotal);
void togglePlayUI();
void uiBPM();
void uiCount();
void uiCredits();
void uiDown();
void uiLeft();
void uiLoad();
void uiNoteIcon();
void uiPlay();
void uiResizeSong();
void uiRight();
void uiSave();
void uiSettings();
void uiUIScroll();
void uiUp();
void uiYellowPlay();
void updateNoteIcon();
void XOutFunction();