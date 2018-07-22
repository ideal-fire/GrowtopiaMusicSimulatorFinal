typedef void(*voidFunc)();
typedef struct{
	CrossTexture* image;
	voidFunc activateFunc;
	s16 uniqueId;
}uiElement;
typedef struct{
	u8 id;
	void* extraData; // For audio gears, this could be their data. For repeat notes, it could be temp data about if they've been used or not.
}noteSpot;
void _addNumberInput(long* _outNumber, char* _outBuffer, int _addNumber);
void _delNumberInput(long* _outNumber, char* _outBuffer);
void _placeNoteLow(int _x, int _y, u8 _noteId, u8 _shouldPlaySound);
uiElement* addUI();
u16 bitmpTextWidth(char* _passedString);
u32 bpmFormula(u32 re);
void centerAround(u32 _passedPosition);
void clearSong();
void controlLoop();
void die(const char* message);
void doUsualDrawing();
void drawImageScaleAlt(CrossTexture* _passedTexture, int _x, int _y, double _passedXScale, double _passedYScale);
void drawPlayBar(int _x);
void drawSong();
void drawString(char* _passedString, int _x, int _y);
void drawUI();
void findMaxX();
s16 fixShort(s16 _passedShort);
int fixX(int _x);
int fixY(int _y);
long getNumberInput(char* _prompt, long _defaultNumber);
uiElement* getUIByID(s16 _passedId);
void goodLuaDofile(lua_State* passedState, char* _passedFilename);
void goodPlaySound(CROSSSFX* _passedSound);
void init();
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