/*
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include <GeneralGoodConfig.h>
#include <GeneralGood.h>
#include <GeneralGoodExtended.h>
#include <GeneralGoodGraphics.h>
#include <GeneralGoodSound.h>
#include <GeneralGoodImages.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifndef NO_FANCY_DIALOG
	#if SUBPLATFORM == SUB_WINDOWS
		#include <windows.h>
		#include <Commdlg.h>
	#else
		#include <nfd.h>
	#endif
#endif

#include "fonthelper.h"
#include "luaDofileEmbedded.h"
#include "nathanList.h"
#include "goodLinkedList.h"

//////////////////
#define SETTINGSVERSION 3
#define HOTKEYVERSION 1
#define SONGFORMATVERSION 1
///////////////////////////////////////

#define ISTESTINGMOBILE 0
#define DISABLESOUND 0

//#define fileByteOrder(x) htons(x)
//#define hostByteOrder(x) ntohs(x)

#define BGMODE_SINGLE 1
#define BGMODE_PART 2

#define LUAREGISTER(x,y) lua_pushcfunction(L,x);\
	lua_setglobal(L,y);

#define CONSTCHARW (singleBlockSize/2)

// length of MAXINTPLAE in base 10
#define MAXINTINPUT 7
// No values bigger than this
#define MAXINTPLAE 1000000

#define CLICKTOGOBACK "Click this text to go back."

#define ZOOMINCREMENT .1

#define BONUSENDCHARACTER 1 // char value to signal the true end of the message text in easyMessage

#define SAVEFORMATMAGIC "GMSF"
#define METADATAMARKER "META"
#define SAVEENDMARKER "FSMG"

#define STUPID_NOTICE_1 "Some images & sounds are"
#define STUPID_NOTICE_2 "assets by Ubisoft and"
#define STUPID_NOTICE_3 "Growtopia. Some were modified."

#define CANTOPENMESSAGE "Maybe you need to enable stroage permission or turn off alt song location."
//#define NOSTROAGEWARNING "Ensure the app has permission access to your files. To do this, go to the app's info, go to the permissions menu, and turn storage on."

#define THEME_FILENAME_FORMAT "assets/Free/Images/pcBackground%d.png"
/////
#define CURSORMODE_NORMAL 0
#define CURSORMODE_ERASER 1
#define CURSORMODE_SELECT 2
////////////////////////////////////////////////
#include "mainprivate.h"
#include "clipboard.h"
void getSelectionCorners(int* startx, int* starty, int* endx, int* endy);
////////////////////////////////////////////////
u8 optionPlayOnPlace=1;
u8 optionZeroBasedPosition=0;
u8 optionDoFancyPage=1;
u8 optionDoCenterPlay=0;
u8 optionExitConfirmation=1;
u8 optionDoubleXAllowsExit=1; // If clicking the X button twice lets you exit even with the confirmation prompt up
u8 optionUpdateCheck=0;
s8 masterVolume=25;
u8 currentThemeIndex=0;
u8 useAltDataDirectory=0;
////////////////////////////////////////////////
// From libGeneralGood
extern int _generalGoodRealScreenWidth;
extern int _generalGoodRealScreenHeight;
////////////////////////////////////////////////
int screenWidth;
int screenHeight;
int logicalScreenWidth;
int logicalScreenHeight;
int globalDrawXOffset;
int globalDrawYOffset;

lua_State* L = NULL;

u16 totalUI=0;
u16 totalVisibleUI=0;
u16 uiScrollOffset=0;
uiElement* myUIBar=NULL;
SDL_Keycode* uiHotkeys=NULL;

CrossTexture* playButtonImage;
CrossTexture* stopButtonImage;
CrossTexture* yellowPlayButtonImage;
CrossTexture* eraserTurnOnImage;
CrossTexture* eraserTurnOffImage;
CrossTexture* rectSelectTurnOnImage;
CrossTexture* rectSelectTurnOffImage;
CrossTexture* newButtonImage;

CrossTexture* uiScrollImage;

u16 songWidth=400;
u16 songHeight=14;

double generalScale=-1;

u16 singleBlockSize=32;

u8 isMobile;
u8 cursorMode=CURSORMODE_NORMAL;

s32 uiNoteIndex=0;
u16 totalNotes=0;

u8 currentlyPlaying=0;
s32 currentPlayPosition;
s32 nextPlayPosition;
u64 lastPlayAdvance=0;

s32 selectpos[4]={-1};

// Only the width of a page can change and it depends on the scale set by the user
u16 pageWidth=25;
#define pageHeight 14

// How much of the page's height you can see without scrolling
u16 visiblePageHeight;
// These are display offsets
s32 songYOffset=0;
s32 songXOffset=0;

// 0 for normal PC mode
// 1 for split mobile mode
u8 backgroundMode=BGMODE_SINGLE;
CrossTexture* bigBackground=NULL;
CrossTexture** bgPartsEmpty=NULL;
CrossTexture** bgPartsLabel=NULL;

CrossTexture** noteImages=NULL;
CROSSSFX*** noteSounds=NULL;
noteInfo* extraNoteInfo=NULL;
u8* noteUIOrder=NULL; // Order of note IDs in the UI
SDL_Keycode* noteHotkeys=NULL;

noteSpot** songArray;

u32 bpm=100;

u8 repeatStartID=255;
u8 repeatEndID=255;
u8 audioGearID=255;
// Max X position the song goes to
u32 maxX=0;

int uiPageSize;

// Position in the song as a string
char positionString[12];

uiElement backButtonUI;
uiElement infoButtonUI;
uiElement volumeButtonUI; // Can use this for both master volume and audio gear volume
uiElement upButtonUI;
uiElement downButtonUI;
uiElement moreUiButtonUI;

int uiUIScrollIndex=-1;

//extern SDL_Keycode lastSDLPressedKey;

const char noteNames[] = {'B','A','G','F','E','D','C','b','a','g','f','e','d','c'};

char* currentSongMetadata=NULL;
char* internalClipboard=NULL;

char unsavedChanges=0;

////////////////////////////////////////////////
char* fixSafeDataPath(char* _passedPath){
	char* _fixedPathBuffer = malloc(strlen(_passedPath)+strlen(getFixPathString(TYPE_DATA))+1);
	fixPath((char*)_passedPath,_fixedPathBuffer,TYPE_DATA);
	return _fixedPathBuffer;
}
char* getAltDataPath(){
	#if SUBPLATFORM == SUB_ANDROID
		return "/sdcard/GMSF/";
	#else
		return getFixPathString(TYPE_DATA);
	#endif
}
char* fixAltDataPath(char* _passedPath){
	#if SUBPLATFORM == SUB_ANDROID
		char* _retBuffer = malloc(strlen(getAltDataPath())+strlen(_passedPath)+1);
		strcpy(_retBuffer,getAltDataPath());
		strcat(_retBuffer,_passedPath);
		return _retBuffer;
	#else
		return fixSafeDataPath(_passedPath);
	#endif
}
char* getDynamicDataPath(){
	return useAltDataDirectory ? getAltDataPath() : getFixPathString(TYPE_DATA);
}
char* fixDynamicDataPath(char* _passedPath){
	return useAltDataDirectory ? fixAltDataPath(_passedPath) : fixSafeDataPath(_passedPath);
}
#if SUBPLATFORM == SUB_ANDROID
int mkdirpublic(const char *path){
	return mkdir(path,S_IRWXU | S_IRWXG | S_IRWXO);
}
#endif
int maybeMakeAltDataDir(){
	#if SUBPLATFORM == SUB_ANDROID
	if (!directoryExists(getAltDataPath())){
		return mkdirpublic(getAltDataPath());
	}
	#endif
	return 0;
}
char canWriteAltDataPath(){
	maybeMakeAltDataDir();
	char* _tempPath = fixAltDataPath("ifyouseethistestfilesomethinghasgoneveryrong");
	FILE* fp = fopen(_tempPath,"wb");
	if (fp!=NULL){
		fclose(fp);
		remove(_tempPath);
		return 1;
	}else{
		return 0;
	}
}

uiElement* insertUI(int _passedIndex){
	addUI();
	memmove(&myUIBar[_passedIndex+1],&myUIBar[_passedIndex],sizeof(uiElement)*((totalUI-1)-_passedIndex));
	return &myUIBar[_passedIndex];
}
void deleteUI(int _passedIndex){
	memmove(&(myUIBar[_passedIndex]),&(myUIBar[_passedIndex+1]),sizeof(uiElement)*(totalUI-(_passedIndex+1)));
	lowReallocUI(--totalUI);
}
int wrapNum(int _passed, int _min, int _max){
	if (_passed>_max){
		return _min+(_passed-_max-1);
	}else if (_passed<_min){
		return _max-(_min-_passed-1);
	}
	return _passed;
}
char* extraStrdup(char* _passed, int _extraSpace){
	char* _ret = malloc(strlen(_passed)+_extraSpace+1);
	strcpy(_ret,_passed);
	return _ret;
}
char* _currentTitlebar=NULL;
void goodSetTitlebar(char* _newTitle, char _isModified){
	if (_newTitle!=NULL){
		free(_currentTitlebar);
		_currentTitlebar = malloc(strlen(_newTitle)+2);
		strcpy(_currentTitlebar,_newTitle);
	}else{
		if (_currentTitlebar==NULL){
			goodSetTitlebar("GMSF",_isModified);
			return;
		}
	}
	if (_isModified){
		addChar(_currentTitlebar,'*');
		setWindowTitle(_currentTitlebar);
		_currentTitlebar[strlen(_currentTitlebar)-1]='\0'; // Prevent infinite stars
	}else{
		setWindowTitle(_currentTitlebar);
	}
}
void markModified(){
	if (!unsavedChanges){
		unsavedChanges=1;
		goodSetTitlebar(NULL,1);
	}
}

// -1 if failed, otherwise the index of the actually loaded theme
int loadTheme(u8 _preferredIndex){
	if (backgroundMode!=BGMODE_SINGLE){
		return -1;
	}
	freeTexture(bigBackground);
	bigBackground=NULL;

	CrossTexture* _newBackground;
	char _filenameBuffer[strlen(THEME_FILENAME_FORMAT)+4];
	sprintf(_filenameBuffer,THEME_FILENAME_FORMAT,_preferredIndex);
	_newBackground = loadEmbeddedPNG(_filenameBuffer);
	if (_newBackground==NULL){
		if (_preferredIndex!=0){
			return loadTheme(0);
		}else{
			easyMessage("Could not load theme 0, this should be included with the program. Somebody goofed.");
			easyMessage(_filenameBuffer);
			return -1;
		}
	}else{
		bigBackground = _newBackground;
		return _preferredIndex;
	}
}

char* wrapText(char* _passedMessage){
	char* currentTextboxMessage = malloc(strlen(_passedMessage)+2);
	strcpy(currentTextboxMessage,_passedMessage);
	// Step 1 - Put words into buffer with newlines
	//////////////////////////////////////////////////////////////////
	uint32_t _cachedMessageLength = strlen(currentTextboxMessage);
	currentTextboxMessage[_cachedMessageLength+1] = BONUSENDCHARACTER;
	uint32_t i;
	uint32_t j;
	// This will loop through the entire message, looking for where I need to add new lines. When it finds a spot that
	// needs a new line, that spot in the message will become 0.
	int _lastNewlinePosition=-1; // If this doesn't start at -1, the first character will be cut off
	for (i = 0; i < _cachedMessageLength; i++){
		if (currentTextboxMessage[i]==32){ // Only check when we meet a space. 32 is a space in ASCII
			currentTextboxMessage[i]='\0';
			if (bitmpTextWidth(&(currentTextboxMessage[_lastNewlinePosition+1]))>pageWidth*2*CONSTCHARW){
				uint8_t _didWork=0;
				for (j=i-1;j>_lastNewlinePosition+1;j--){
					//printf("J:%d\n",j);
					if (currentTextboxMessage[j]==32){
						currentTextboxMessage[j]='\0';
						_didWork=1;
						currentTextboxMessage[i]=32;
						_lastNewlinePosition=j;
						break;
					}
				}
				if (_didWork==0){
					currentTextboxMessage[i]='\0';
					_lastNewlinePosition=i+1;
				}
			}else{
				currentTextboxMessage[i]=32;
			}
		}
	}
	// This code will make a new line if there needs to be one because of the last word
	if (bitmpTextWidth(&(currentTextboxMessage[_lastNewlinePosition+1]))>pageWidth*2*CONSTCHARW){
		for (j=_cachedMessageLength-1;j>_lastNewlinePosition+1;j--){
			if (currentTextboxMessage[j]==32){
				currentTextboxMessage[j]='\0';
				break;
			}
		}
	}
	return currentTextboxMessage;
}

void drawWrappedText(char* _passedMessage, int _x, int _y){
	// Note - this code is copied for easyChoice
	int i;
	int _currentDrawPosition=0;
	for (i=0;;++i){
		drawString(&(_passedMessage[_currentDrawPosition]),_x,_y+i*singleBlockSize);
		_currentDrawPosition+=strlen(&(_passedMessage[_currentDrawPosition]));
		if (_passedMessage[_currentDrawPosition+1]==BONUSENDCHARACTER){
			break;
		}else{
			_currentDrawPosition++;
		}
	}
}

// Code stolen from Happy Land.
// Displays whatever message you want. Text will wrap.
void easyMessage(char* _newMessage){
	char* currentTextboxMessage = wrapText(_newMessage);
	//////////////////////////////////////////////////////////////////
	// Step 2 - Display words
	//////////////////////////////////////////////////////////////////
	controlLoop();
	while(1){
		controlsStart();
		if (wasJustPressed(SCE_TOUCH)){
			break;
		}
		controlsEnd();
		startDrawing();
		drawWrappedText(currentTextboxMessage,0,0);
		endDrawing();
	}
	free(currentTextboxMessage);
	controlsResetEmpty();
}

void saveHotkeys(){
	char* _settingsFilename = fixSafeDataPath("hotkeys.legSettings");
	FILE* fp = fopen(_settingsFilename,"wb");
	if (fp!=NULL){
		u8 _tempHoldVersion = HOTKEYVERSION;
		fwrite(&_tempHoldVersion,sizeof(u8),1,fp);

		int i;
		fwrite(&totalNotes,sizeof(u16),1,fp);
		for (i=0;i<totalNotes;++i){
			fwrite(&(noteHotkeys[i]),sizeof(SDL_Keycode),1,fp);
		}
		fwrite(&totalUI,sizeof(u16),1,fp);
		for (i=0;i<totalUI;++i){
			fwrite(&(myUIBar[i].uniqueId),sizeof(u8),1,fp);
			fwrite(&(uiHotkeys[i]),sizeof(SDL_Keycode),1,fp);
		}
		fclose(fp);
	}else{
		printf("Could not write hotkey settings file to\n");
		printf("%s\n",_settingsFilename);
	}
	free(_settingsFilename);
}

void saveSettings(){
	char* _settingsFilename = fixSafeDataPath("generalSettings.legSettings");
	FILE* fp = fopen(_settingsFilename,"wb");
	if (fp!=NULL){
		u8 _tempHoldVersion = SETTINGSVERSION;
		fwrite(&_tempHoldVersion,sizeof(u8),1,fp);

		fwrite(&optionPlayOnPlace,sizeof(u8),1,fp);
		fwrite(&optionZeroBasedPosition,sizeof(u8),1,fp);
		fwrite(&optionDoFancyPage,sizeof(u8),1,fp);
		fwrite(&optionDoCenterPlay,sizeof(u8),1,fp);
		fwrite(&optionExitConfirmation,sizeof(u8),1,fp);
		fwrite(&optionDoubleXAllowsExit,sizeof(u8),1,fp);
		fwrite(&optionUpdateCheck,sizeof(u8),1,fp);
		fwrite(&masterVolume,sizeof(s8),1,fp);
		fwrite(&currentThemeIndex,sizeof(u8),1,fp);
		fwrite(&useAltDataDirectory,sizeof(u8),1,fp);
		fwrite(&generalScale,sizeof(double),1,fp);

		fclose(fp);
	}else{
		printf("Could not write settings file to\n");
		printf("%s\n",_settingsFilename);
	}
	free(_settingsFilename);
}

char loadSettings(char* _passedPath){
	FILE* fp = fopen(_passedPath,"rb");
	if (fp!=NULL){
		u8 _tempHoldVersion = SETTINGSVERSION;
		fread(&_tempHoldVersion,sizeof(u8),1,fp);
		if (_tempHoldVersion>=1){
			fread(&optionPlayOnPlace,sizeof(u8),1,fp);
			fread(&optionZeroBasedPosition,sizeof(u8),1,fp);
			fread(&optionDoFancyPage,sizeof(u8),1,fp);
			fread(&optionDoCenterPlay,sizeof(u8),1,fp);
			fread(&optionExitConfirmation,sizeof(u8),1,fp);
			fread(&optionDoubleXAllowsExit,sizeof(u8),1,fp);
			fread(&optionUpdateCheck,sizeof(u8),1,fp);
			fread(&masterVolume,sizeof(s8),1,fp);
			fread(&currentThemeIndex,sizeof(u8),1,fp);
		}
		if (_tempHoldVersion>=2){
			fread(&useAltDataDirectory,sizeof(u8),1,fp);
		}
		if (_tempHoldVersion>=3){
			fread(&generalScale,sizeof(double),1,fp);
		}
		fclose(fp);
	}else{
		return 0;
	}
	return 1;
}

void loadHotkeys(){
	char* _settingsFilename = fixSafeDataPath("hotkeys.legSettings");
	FILE* fp = fopen(_settingsFilename,"rb");
	if (fp!=NULL){
		u8 _tempReadVersion = HOTKEYVERSION;
		fread(&_tempReadVersion,sizeof(u8),1,fp);
		if (_tempReadVersion>=1){
			int i;
			u16 _readTotalNotes;
			fread(&_readTotalNotes,sizeof(u16),1,fp);
			for (i=0;i<_readTotalNotes;++i){
				fread(&(noteHotkeys[i]),sizeof(SDL_Keycode),1,fp);
			}
			u16 _readTotalUI;
			fread(&_readTotalUI,sizeof(u16),1,fp);
			for (i=0;i<_readTotalUI;++i){
				u8 _lastLoadedID;
				fread(&(_lastLoadedID),sizeof(u8),1,fp);
				if (_lastLoadedID!=U_NOTUNIQUE){
					int j;
					for (j=0;j<_readTotalUI;++j){
						if (myUIBar[j].uniqueId==_lastLoadedID){
							fread(&(uiHotkeys[j]),sizeof(SDL_Keycode),1,fp);
							break;
						}
					}
				}
			}
			if (_readTotalUI!=totalUI){
				printf("Settings read total UI: %d. Actual total UI: %d\n",_readTotalUI,totalUI);
			}
		}
		fclose(fp);
	}
	free(_settingsFilename);
}

u8 getUINoteID(){
	return noteUIOrder[uiNoteIndex];
}
int gearBuffSize(){
	return AUDIOGEARSPACE*2+1;
}
u8* getGearVolume(u8* _passedExtraData){
	return &(_passedExtraData[gearBuffSize()-1]);
}
int touchXToBlock(int _passedTouchX){
	return floor((_passedTouchX-globalDrawXOffset)/singleBlockSize);
}
int touchYToBlock(int _passedTouchY){
	return floor((_passedTouchY-globalDrawYOffset)/singleBlockSize);
}

char isBigEndian(){
	volatile u32 i=0x01234567;
	return !((*((u8*)(&i))) == 0x67);
}

s16 fixShort(s16 _passedShort){
	if (isBigEndian()){ // I've never tested this
		printf("Big boy!\n");
		// Swap them because C# stores in files as little endian
		s16 _readASBigEndian;
		((u8*)(&_readASBigEndian))[0] = ((u8*)(&_passedShort))[1];
		((u8*)(&_readASBigEndian))[1] = ((u8*)(&_passedShort))[0];
		return _readASBigEndian;
	}else{
		return _passedShort;
	}
}

void setSongXOffset(int _newValue){
	songXOffset = _newValue;
	sprintf(positionString,"%d/%d",songXOffset+!optionZeroBasedPosition,songWidth);
}

void dupNote(noteSpot* old, noteSpot* new){
	new->id=old->id;
	if (old->id==audioGearID){
		new->extraData=malloc(gearBuffSize());
		memcpy(new->extraData,old->extraData,gearBuffSize());
	}else{
		new->extraData=NULL;
	}
}
void freeNote(noteSpot* me){
	if (me->id==audioGearID){
		free(me->extraData);
	}
}
void freeSongArr(noteSpot** inarr, int x, int y, int w, int h){
	for (int j=y;j<y+h;++j){
		for (int i=x;i<x+w;++i){
			freeNote(&(inarr[j][i]));
		}
	}
}
void clearSong(){
	freeSongArr(songArray,0,0,songWidth,songHeight);
	setSongXOffset(0);
	setSongWidth(songArray,songWidth,400);
	songWidth=400;
	int i;
	for (i=0;i<songHeight;++i){
		int j;
		for (j=0;j<songWidth;++j){
			songArray[i][j].id=0;
			songArray[i][j].extraData=NULL;
		}
	}
}

char* strndup(const char *str, size_t size){
	char* _newBuffer = malloc(size+1);
	int i;
	for (i=0;i<size;++i){
		_newBuffer[i] = str[i];
	}
	_newBuffer[size]='\0';
	return _newBuffer;
}

// Desired format for programmer:
// textFiles/txt,doc;ImageFiles/png,jpg;
// Output for nfd:
// txt,doc;png,jpg
// Output for windows:
// textFiles\0*.txt;*.doc\0ImageFiles\0*.png;*.jpg\0
	// always append \0All\0*.*\0
char* fixFiletypeFilter(const char* _passedFilters){
	if (strlen(_passedFilters)==0){
		return NULL;
	}
	
	// Step 1 - Change
		// textFiles/txt,doc;ImageFiles/png;jpg;
		// into list entries
		// textFiles/txt,doc
		// ImageFiles/png,jpg
	nathanList* _unparsedFilterChoices = calloc(1,sizeof(nathanList));
	const char* _lastStrChop = _passedFilters;
	const char* _currentCheck = _passedFilters;
	while (_currentCheck[0]!='\0'){
		if (_currentCheck[0]==';'){
			addNathanList(_unparsedFilterChoices)->memory = strndup(_lastStrChop,_currentCheck-_lastStrChop);
			_lastStrChop = _currentCheck+1; // Make sure we don't have the semicolon
		}
		_currentCheck++;
	}

	// Step 2 - Parse
		// textFiles/txt,doc
		// into list entries
		// textFiles
		// txt,doc
	nathanList* _filterNames = calloc(1,sizeof(nathanList));
	nathanList* _unparsedFilterLists = calloc(1,sizeof(nathanList));
	int _cachedListLength = getNathanListLength(_unparsedFilterChoices);
	int i;
	for (i=0;i<_cachedListLength;++i){
		nathanList* _iterationList = getNathanList(_unparsedFilterChoices,i);
		_currentCheck = _iterationList->memory;
		while (_currentCheck[0]!='\0'){
			if (_currentCheck[0]=='/'){
				addNathanList(_filterNames)->memory = strndup(_iterationList->memory,_currentCheck-(char*)_iterationList->memory);
				addNathanList(_unparsedFilterLists)->memory = strdup(_currentCheck+1);
				break;
			}
			_currentCheck++;
		}
	}

	// Step 2 - Parse
		// txt,doc
		// Into list entries
		// txt
		// doc
	_cachedListLength = getNathanListLength(_filterNames);
	nathanList** _fileTypes = malloc(sizeof(nathanList*)*_cachedListLength);
	for (i=0;i<_cachedListLength;++i){
		_fileTypes[i] = calloc(1,sizeof(nathanList));
		nathanList* _iterationList = getNathanList(_unparsedFilterLists,i);
		_currentCheck = _iterationList->memory;
		const char* _lastStrChop = _currentCheck;
		while (1){
			if (_currentCheck[0]==',' || _currentCheck[0]=='\0'){
				addNathanList(_fileTypes[i])->memory = strndup(_lastStrChop,_currentCheck-_lastStrChop);
				if (_currentCheck[0]=='\0'){
					break;
				}
				_currentCheck++;
				_lastStrChop = _currentCheck;
			}
			_currentCheck++;
		}
	}

	// Complete buffer we'll return
	char* _returnString = malloc(256);
	_returnString[0]='\0';

	// Finally, make our usable format string
	#if SUBPLATFORM == SUB_WINDOWS // win
		_cachedListLength = getNathanListLength(_filterNames);
		// We move this pointer around when making this string
		char* _currentAppendPosition = _returnString;
		// Do the bulk
		for (i=0;i<_cachedListLength;++i){
			strcpy(_currentAppendPosition,getNathanList(_filterNames,i)->memory);
			_currentAppendPosition+=strlen(getNathanList(_filterNames,i)->memory)+1;
			int j;
			for (j=0;j<getNathanListLength(_fileTypes[i]);++j){
				strcpy(_currentAppendPosition,"*.");
				_currentAppendPosition+=2;
				strcpy(_currentAppendPosition,getNathanList(_fileTypes[i],j)->memory);
				strcat(_currentAppendPosition,";");
				_currentAppendPosition+=strlen(getNathanList(_fileTypes[i],j)->memory)+1;
			}
			// Remove extra semicolon
			(_currentAppendPosition-1)[0]='\0';
		}
		// Append All\0*.*\0
		strcpy(_currentAppendPosition,"All");
		_currentAppendPosition+=4;
		strcpy(_currentAppendPosition,"*");
	#else // nfd
		_cachedListLength = getNathanListLength(_filterNames);
		for (i=0;i<_cachedListLength;++i){
			int j;
			for (j=0;j<getNathanListLength(_fileTypes[i]);++j){
				strcat(_returnString,getNathanList(_fileTypes[i],j)->memory);
				strcat(_returnString,",");
			}
			_returnString[strlen(_returnString)-1]='\0'; // Delete the extra comma we have
			strcat(_returnString,";");
		}
		_returnString[strlen(_returnString)-1]='\0'; // Delete the extra semicolon we have
	#endif

	// Free linked lists
	_cachedListLength = getNathanListLength(_filterNames);
	freeNathanList(_filterNames,1);
	freeNathanList(_unparsedFilterLists,1);
	freeNathanList(_unparsedFilterChoices,1);
	// Free list of file types
	for (i=0;i<_cachedListLength;++i){
		freeNathanList(_fileTypes[i],1);
	}
	free(_fileTypes);
	// Done
	return _returnString;
}

char* textInput(char* _initial, char* _restrictedCharacter, char* _prompt){
	controlLoop();
	int _maxChars;
	if (_initial!=NULL){
		_maxChars = strlen(_initial)>pageWidth*2 ? strlen(_initial) : pageWidth*2;
	}else{
		_maxChars = pageWidth*2;
	}
	char* _userInput = calloc(1,_maxChars+1);
	if (_initial!=NULL){
		strcpy(_userInput,_initial);
	}
	char _isDone=0;
	SDL_StartTextInput();
	while (!_isDone){
		SDL_Event event;
		if (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					XOutFunction();
					break;
				case SDL_TEXTINPUT:
					if (_userInput!=NULL){
						int i;
						int _cachedTotalRestircted = strlen(_restrictedCharacter);
						char _stopCharacter=0;
						for (i=0;i<_cachedTotalRestircted;++i){
							if (event.text.text[0]==_restrictedCharacter[i]){
								_stopCharacter=1;
								break;
							}
						}
						if (!_stopCharacter){
							// Add character the user typed as long as it's not going to overflow
							if (strlen(_userInput)<_maxChars){
								strcat(_userInput, event.text.text);
							}
						}
					}
					break;
				case SDL_KEYDOWN:
					if (_userInput!=NULL){
						if (event.key.keysym.sym == SDLK_BACKSPACE){
							if (strlen(_userInput)>0){
								_userInput[strlen(_userInput)-1]='\0';
							}
						}else if (event.key.keysym.sym == SDLK_KP_ENTER || event.key.keysym.sym==SDLK_RETURN){
							_isDone=1;
						}else if (event.key.keysym.sym == SDLK_ESCAPE){
							free(_userInput);
							_userInput=NULL;
							_isDone=1;
						}
					}
					break;
				case SDL_FINGERDOWN:
					;
					int _foundY = event.tfinger.y * _generalGoodRealScreenHeight;
					if(_foundY<singleBlockSize){
						_isDone=1;
					}else if  (_foundY<singleBlockSize*2){
						free(_userInput);
						_userInput=NULL;
						_isDone=1;
					}
					break;
			}
		}

		startDrawing();
		// Draw rectangles for buttons
		drawRectangle(0,0,logicalScreenWidth,singleBlockSize,0,255,0,255);
		drawRectangle(0,singleBlockSize,logicalScreenWidth,singleBlockSize,255,0,0,255);
		// Text depends on platform
		drawString(isMobile ? "Done" : "Done: Enter",0,CONSTCHARW/2);
		drawString(isMobile ? "Cancel" : "Cancel: Escape",0,CONSTCHARW/2+singleBlockSize);
		// Textbox
		drawRectangle(0,singleBlockSize*2,logicalScreenWidth,singleBlockSize,255,255,255,255);
			if (_userInput!=NULL){
				if (!isMobile && strlen(_userInput)==0){
					drawString("(Type to input text)",0,CONSTCHARW/2+singleBlockSize*2);
				}else{
					drawString(_userInput,0,CONSTCHARW/2+singleBlockSize*2);
				}
			}
		if (_prompt!=NULL){
			drawString(_prompt,0,CONSTCHARW/2+singleBlockSize*3);
		}
		endDrawing();
	}
	SDL_StopTextInput();
	controlsResetEmpty(); // Because we didn't catch the finger up or anything
	return _userInput;
}
#define MAXFILES 50
#define MAXFILELENGTH 500
char* sharedFilePicker(char _isSaveDialog, const char* _filterList, char _forceExtension, char* _forcedExtension){
	#ifdef NO_FANCY_DIALOG
		#ifdef MANUALPATHENTRY
			printf("Input path:\n");
			char* _readLine=NULL;
			size_t _readLineBufferSize = 0;
			getline(&_readLine,&_readLineBufferSize,stdin);
			removeNewline(_readLine);
			return _readLine;
		#else // Input for mobile
			if (useAltDataDirectory){
				maybeMakeAltDataDir();
			}
			nList* _fileList = NULL;
			CROSSDIR dir = openDirectory (getDynamicDataPath());
			if (dirOpenWorked(dir)==0){
				char* _tempString = extraStrdup("Failed to open directory. "CANTOPENMESSAGE"  ",strlen(getDynamicDataPath()));
				strcat(_tempString,getDynamicDataPath());
				easyMessage(_tempString);
				free(_tempString);
				return NULL;
			}
			CROSSDIRSTORAGE lastStorage;
			while(directoryRead(&dir,&lastStorage)!=0){
				addnList(&_fileList)->data = strdup(getDirectoryResultName(&lastStorage));
			}
			directoryClose (dir);
	
			int _selected=0;
			int _drawXPos = logicalScreenWidth - logicalScreenWidth/9;
			double _singleHeight = (logicalScreenHeight/(double)6);
			int _singleWidth = logicalScreenWidth/9;
			char* _ret=NULL;
			while(1){
				controlsStart();
				if (wasJustPressed(SCE_TOUCH)){
					int _fixedTouchX = (touchX-globalDrawXOffset);
					int _fixedTouchY = (touchY-globalDrawYOffset);
					if (_fixedTouchX>=_drawXPos){
						char _buttonPressed = _fixedTouchY/_singleHeight;
						if (_buttonPressed<=1){
							_selected = wrapNum(_selected-1,0,nListLen(_fileList)-1);
						}else if (_buttonPressed<=3){
							_selected = wrapNum(_selected+1,0,nListLen(_fileList)-1);
						}else if (_buttonPressed==4){
							if (_fileList!=NULL){
								_ret=strdup(getnList(_fileList,_selected)->data);
							}
							break;
						}else{
							break;
						}
					}else if (_isSaveDialog && _fixedTouchX<_drawXPos && _fixedTouchY>logicalScreenHeight-_singleHeight){
						_ret = textInput(NULL," /?%*:|\\<>",_isSaveDialog ? "Save filename" : "Load filename");
						if (_ret!=NULL){
							break;
						}
					}
				}
				controlsEnd();
				startDrawing();
				tempDrawImageSize(upButtonUI.image,_drawXPos,0,_singleWidth,_singleHeight*2);
				tempDrawImageSize(downButtonUI.image,_drawXPos,_singleHeight*2,_singleWidth,_singleHeight*2);
				tempDrawImageSize(playButtonImage,_drawXPos,_singleHeight*4,_singleWidth,_singleHeight);
				tempDrawImageSize(stopButtonImage,_drawXPos,_singleHeight*5,_singleWidth,_singleHeight);
				if (_isSaveDialog){
					tempDrawImageSize(newButtonImage,0,logicalScreenHeight-_singleHeight,_singleWidth,_singleHeight);
				}

				int _startDrawIndex;
				if (_selected>(logicalScreenHeight/singleBlockSize)/2){
					_startDrawIndex = _selected-(logicalScreenHeight/singleBlockSize)/2;
				}else{
					_startDrawIndex=0;
				}
				int i = _startDrawIndex;
				int _currentDrawY = 0;
				if (_fileList!=NULL){
					ITERATENLIST(getnList(_fileList,_startDrawIndex),{
						if (i==_selected){
							drawRectangle(0,_currentDrawY,strlen(_currentnList->data)*CONSTCHARW,singleBlockSize,0,255,0,255);
						}
						drawString(_currentnList->data,0,_currentDrawY);
						_currentDrawY+=singleBlockSize;
						++i;
					})
				}else{
					// Highlight select button in red
					drawRectangle(_drawXPos,(logicalScreenHeight/(double)6)*4,logicalScreenWidth/9,(logicalScreenHeight/(double)6),255,0,0,127);
					drawString("No files",0,0);
				}
				endDrawing();
			}
			controlsResetEmpty();
			freenList(_fileList,1);
			
			if (_ret!=NULL){
				char* _realRet = fixDynamicDataPath(_ret);
				free(_ret);
				return _realRet;
			}else{
				return NULL;
			}
			
		#endif
	#else
		char* _fixedFilterlist = fixFiletypeFilter(_filterList);
		char* _foundCompletePath=NULL;
		#if SUBPLATFORM == SUB_WINDOWS
			// This man is a hero.
			//https://www.daniweb.com/programming/software-development/code/217307/a-simple-getopenfilename-example

			OPENFILENAME ofn;
			// a another memory buffer to contain the file name
			char szFile[512];
			char _didWork=0;
			// open a file name
			// Meaning of these properties can be found here:
			//https://docs.microsoft.com/en-us/windows/desktop/api/commdlg/ns-commdlg-tagofna
			ZeroMemory( &ofn , sizeof( ofn));
			ofn.lStructSize = sizeof ( ofn );
			ofn.hwndOwner = NULL;
			ofn.lpstrFile = szFile;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof( szFile );
			ofn.lpstrFilter = _fixedFilterlist;
			ofn.nFilterIndex = 1; // Our currently selected filter by default. Starts at 1 because Microsoft.
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ;
			
			if (_isSaveDialog){
				if (GetSaveFileName( &ofn )!=0){
					_didWork=1;
				}
			}else{
				if (GetOpenFileName( &ofn )!=0){
					_didWork=1;
				}
			}
			if (_didWork){
				_foundCompletePath = strdup(szFile);
			}else{
				return NULL;
			}
			
			GetCurrentDirectory(512,szFile);
			printf("%s\n",szFile);
			
			// Now simpley display the file name 
			//MessageBox ( NULL , ofn.lpstrFile , "File Name" , MB_OK);
		#else
			nfdchar_t *outPath = NULL;
			nfdresult_t result;
			if (_isSaveDialog){
				result = NFD_SaveDialog( _fixedFilterlist, NULL, &outPath );
			}else{
				result = NFD_OpenDialog( _fixedFilterlist, NULL, &outPath );
			}
			if (result == NFD_OKAY){
				_foundCompletePath=outPath;
			}else if ( result == NFD_CANCEL ){
				return NULL;
			}else{
				printf("Error: %s\n", NFD_GetError() );
				return NULL;
			}
		#endif
		free(_fixedFilterlist);

		// If we're saving, add a file extension if the user didn't give one
		if (_isSaveDialog && _forceExtension && _foundCompletePath!=NULL){
			signed int i;
			char _foundDot=0;
			for (i=strlen(_foundCompletePath)-1;i>=0;--i){
				if (_foundCompletePath[i]=='\\' || _foundCompletePath[i]=='/'){
					_foundDot=0;
					break;
				}else if (_foundCompletePath[i]=='.'){
					_foundDot=1;
					break;
				}
			}
			if (!_foundDot){
				char* _newPath = malloc(strlen(_foundCompletePath)+strlen(_forcedExtension)+1);
				strcpy(_newPath,_foundCompletePath);
				strcat(_newPath,_forcedExtension);
				free(_foundCompletePath);
				_foundCompletePath = _newPath;
			}
		}
		controlsStart();
		controlsEnd();
		controlsResetEmpty(); // Because we didn't notice click up
		return _foundCompletePath;
	#endif
}

// This function returns malloc'd string or NULL
char* selectLoadFile(){
	return sharedFilePicker(0,"Growtopia Music Simulator/GMSF,gtmusic,AngryLegGuy,mylegguy;",0,NULL);
}

char* selectSaveFile(){
	return sharedFilePicker(1,"GMSF/GMSF;",1,".GMSF");
}

void findMaxX(){
	maxX=0;
	int i,j;
	for (i=0;i<songWidth;++i){
		for (j=0;j<songHeight;++j){
			if (songArray[j][i].id!=0){
				maxX=i;
				break;
			}
		}
	}
}

void controlLoop(){
	controlsStart();
	controlsEnd();
}

void int2str(char* _outBuffer, int _inNumber){
	sprintf(_outBuffer,"%d",_inNumber);
}

void _addNumberInput(long* _outNumber, char* _outBuffer, int _addNumber){
	if (*_outNumber>=MAXINTPLAE){
		drawRectangle(0,0,logicalScreenWidth,logicalScreenHeight,255,0,0,255);
	}else{
		*_outNumber*=10;
		*_outNumber+=_addNumber;
		int2str(_outBuffer,*_outNumber);
	}
}
void _delNumberInput(long* _outNumber, char* _outBuffer){
	if (*_outNumber==0){
		drawRectangle(0,0,logicalScreenWidth,logicalScreenHeight,255,0,0,255);
	}else{
		*_outNumber/=10;
		int2str(_outBuffer,*_outNumber);
	}
}
long getNumberInput(const char* _prompt, long _defaultNumber){
	controlLoop();
	long _userInput=_defaultNumber;
	char _userInputAsString[MAXINTINPUT+1]={'\0'};
	
	int _backspaceButtonX = 0;
	int _exitButtonX = CONSTCHARW*4;
	int _doneButtonX = CONSTCHARW*9;
	char* _backspaceString="Del ";
	char* _exitString="Exit ";
	char* _doneString="Done";
	int _backspaceWidth=strlen(_backspaceString)*CONSTCHARW;
	int _exitWidth=strlen(_exitString)*CONSTCHARW;
	int _doneWidth=strlen(_doneString)*CONSTCHARW;
	
	char _shouldExit=0;

	int2str(_userInputAsString,_userInput);
	//itoa(num, buffer, 10);
	while (!_shouldExit){
		startDrawing(); // Start drawing here because we draw a red background if we try to input too much text
		controlsStart();
		#if RENDERER == REND_SDL
			if (lastSDLPressedKey!=SDLK_UNKNOWN){
				int _pressedValue=-1;

				// Special cases first
				if (lastSDLPressedKey==SDLK_BACKSPACE){
					_delNumberInput(&_userInput,_userInputAsString);
				}else if (lastSDLPressedKey==SDLK_KP_ENTER || lastSDLPressedKey==SDLK_RETURN){
					_shouldExit=1;
				}else if (lastSDLPressedKey==SDLK_ESCAPE){
					_shouldExit=1;
					_userInput=_defaultNumber;
				}else{
					switch (lastSDLPressedKey){
						case SDLK_KP_0:
						case SDLK_0:
							_pressedValue=0;
							break;
						case SDLK_KP_1:
						case SDLK_1:
							_pressedValue=1;
							break;
						case SDLK_KP_2:
						case SDLK_2:
							_pressedValue=2;
							break;
						case SDLK_KP_3:
						case SDLK_3:
							_pressedValue=3;
							break;
						case SDLK_KP_4:
						case SDLK_4:
							_pressedValue=4;
							break;
						case SDLK_KP_5:
						case SDLK_5:
							_pressedValue=5;
							break;
						case SDLK_KP_6:
						case SDLK_6:
							_pressedValue=6;
							break;
						case SDLK_KP_7:
						case SDLK_7:
							_pressedValue=7;
							break;
						case SDLK_KP_8:
						case SDLK_8:
							_pressedValue=8;
							break;
						case SDLK_KP_9:
						case SDLK_9:
							_pressedValue=9;
							break;
					}
					// If they actually pressed a number
					if (_pressedValue!=-1){
						_addNumberInput(&_userInput,_userInputAsString,_pressedValue);
					}
				}
			}
		#endif
		if (wasJustPressed(SCE_TOUCH)){
			int _fixedTouchX = touchX-globalDrawXOffset;
			int _fixedTouchY = touchY-globalDrawYOffset;
			if (_fixedTouchY>logicalScreenHeight-singleBlockSize*2){
				if (_fixedTouchY<logicalScreenHeight-singleBlockSize){
					_addNumberInput(&_userInput,_userInputAsString,_fixedTouchX/singleBlockSize);
				}else{
					if (_fixedTouchX>_backspaceButtonX && _fixedTouchX<_backspaceButtonX+_backspaceWidth){
						_delNumberInput(&_userInput,_userInputAsString);
					}else if (_fixedTouchX>_exitButtonX && _fixedTouchX<_exitButtonX+_exitWidth){
						_shouldExit=1;	
						_userInput=_defaultNumber;
					}else if (_fixedTouchX>_doneButtonX && _fixedTouchX<_doneButtonX+_doneWidth){
						_shouldExit=1;
					}
				}
			}
		}
		controlsEnd();
		
		drawString(_prompt,0,0);
		drawString(_userInputAsString,0,singleBlockSize);
		char _currentDrawNumber=0;
		char j;
		for (j=0;j<=9;++j){
			char _tempString[2];
			_tempString[0]=_currentDrawNumber+48;
			_tempString[1]='\0';
			int _drawX = j*singleBlockSize;
			int _drawY = logicalScreenHeight-2*singleBlockSize;
			
			if (j%2==0){
				drawRectangle(_drawX,_drawY,singleBlockSize,singleBlockSize,204,255,204,255);
			}else{
				drawRectangle(_drawX,_drawY,singleBlockSize,singleBlockSize,255,204,255,255);
			}
			drawString(_tempString,_drawX+CONSTCHARW/2,_drawY+CONSTCHARW/2);
			_currentDrawNumber++;
		}
		drawRectangle(_backspaceButtonX,logicalScreenHeight-singleBlockSize,_backspaceWidth,singleBlockSize,204,204,255,255);
		drawRectangle(_exitButtonX,logicalScreenHeight-singleBlockSize,_exitWidth,singleBlockSize,255,204,204,255);
		drawRectangle(_doneButtonX,logicalScreenHeight-singleBlockSize,_doneWidth,singleBlockSize,204,204,255,255);

		drawString(_backspaceString,_backspaceButtonX,logicalScreenHeight-singleBlockSize+CONSTCHARW/2);
		drawString(_exitString,_exitButtonX,logicalScreenHeight-singleBlockSize+CONSTCHARW/2);
		drawString(_doneString,_doneButtonX,logicalScreenHeight-singleBlockSize+CONSTCHARW/2);

		endDrawing();
	}

	controlLoop();
	return _userInput;
}

// Inclusive bounds
void resetRepeatNotes(int _startResetX, int _endResetX){
	int i;
	for (i=_startResetX;i<=_endResetX;++i){
		int j;
		for (j=0;j<songHeight;++j){
			if (songArray[j][i].id==repeatEndID){
				songArray[j][i].extraData=NULL;
			}
		}
	}
}

// Does not change song data, just temporary note states and stuff
void resetPlayState(){
	resetRepeatNotes(0,songWidth-1);
}

u16 bitmpTextWidth(char* _passedString){
	return strlen(_passedString)*(CONSTCHARW);
}

// Flip every bit in the file called "possible"
u32 bpmFormula(u32 re){
	return 15000/re;
	//return 60000 / (4 * re);
}
// Given note wait time, find BPM
u32 reverseBPMformula(u32 re){
	//return 15000/re;
	return bpmFormula(re);
}

void centerAround(u32 _passedPosition){
	if (optionDoCenterPlay){
		u16 _halfScreenWidth = ceil(pageWidth/2);
		if (_passedPosition<_halfScreenWidth){
			setSongXOffset(0);
			return;
		}
		if (_passedPosition>=songWidth-_halfScreenWidth){
			setSongXOffset(songWidth-pageWidth);
			return;
		}
		setSongXOffset(_passedPosition-_halfScreenWidth);
	}else{
		int _possibleNewOffset = (_passedPosition/pageWidth)*pageWidth;
		if (_possibleNewOffset>songWidth-pageWidth){ // If we would go too far
			_possibleNewOffset = songWidth-pageWidth;
		}
		setSongXOffset(_possibleNewOffset);
	}
}

int getUIIndexByID(s16 _passedId){
	int i;
	for (i=0;i<totalUI;++i){
		if (myUIBar[i].uniqueId==_passedId){
			return i;
		}
	}
	return -1;
}
uiElement* getUIByID(s16 _passedId){
	int _possibleResult = getUIIndexByID(_passedId);
	return _possibleResult!=-1 ? &(myUIBar[_possibleResult]) : NULL;
}

void changeButtonIcon(s16 _passedId, CrossTexture* _newImage){
	uiElement* _elem = getUIByID(_passedId);
	if (_elem!=NULL){
		_elem->image=_newImage;
	}
}

void updateNoteIcon(){
	uiElement* _noteIconElement = getUIByID(U_SELICON);
	if (_noteIconElement!=NULL){
		_noteIconElement->image=noteImages[getUINoteID()];
	}
}

void pageTransition(int _destX){
	if (optionDoFancyPage){
		int _positiveDest=songWidth;
		int _negativeDest=-1;
		int _changeAmount = (_destX-songXOffset)/30;
		if (_changeAmount==0){
			if (_destX-songXOffset<0){
				_changeAmount=-1;
			}else{
				_changeAmount=1;
			}
		}
		if (_destX<songXOffset){
			_negativeDest = _destX;
		}else if (_destX>songXOffset){
			_positiveDest = _destX;
		}else{
			//printf("Bad, are equal, nothing to do.\n");
			return;
		}
		while(1){
			setSongXOffset(songXOffset+_changeAmount);
			if (songXOffset>=_positiveDest){
				setSongXOffset(_positiveDest);
				break;
			}
			if (songXOffset<=_negativeDest){
				setSongXOffset(_negativeDest);
				break;
			}
			startDrawing();
			doUsualDrawing();
			endDrawing();
		}
	}else{
		setSongXOffset(_destX);
	}
}

void togglePlayUI(){
	uiElement* _playButtonUI = getUIByID(U_PLAY);
	if (_playButtonUI->image==stopButtonImage){
		_playButtonUI->image = playButtonImage;
	}else{
		_playButtonUI->image = stopButtonImage;
	}
	_playButtonUI = getUIByID(U_YPLAY);
	if (_playButtonUI->image==stopButtonImage){
		_playButtonUI->image = yellowPlayButtonImage;
	}else{
		_playButtonUI->image = stopButtonImage;
	}
}

void playAtPosition(s32 _startPosition){
	resetPlayState();
	if (!currentlyPlaying){
		pageTransition(_startPosition);
		setSongXOffset(_startPosition);
		togglePlayUI();
		currentlyPlaying=1;
		//currentPlayPosition = songXOffset;
		lastPlayAdvance=0;
		nextPlayPosition=songXOffset;
	}else{
		togglePlayUI();
		currentlyPlaying=0;
		setSongXOffset((currentPlayPosition/pageWidth)*pageWidth);
	}
}

// Here we hard code the number of bytes to write because we want the same number of bytes on all systems so that save files are cross platform
char saveSong(char* _passedFilename){
	FILE* fp = fopen(_passedFilename,"wb");
	if (fp!=NULL){
		// Magic
		fwrite(SAVEFORMATMAGIC,strlen(SAVEFORMATMAGIC),1,fp);
		u8 _tempHoldVersion = SONGFORMATVERSION;
			fwrite(&_tempHoldVersion,1,1,fp);
		// Special IDs needed for loading
		fwrite(&audioGearID,1,1,fp);
		// Song info
		s16 _tempHoldBPM = bpm;
		s16 _tempHoldWidth = songWidth;
		s16 _tempHoldHeight = songHeight;
		_tempHoldBPM = fixShort(_tempHoldBPM);
		_tempHoldWidth = fixShort(_tempHoldWidth);
		_tempHoldHeight = fixShort(_tempHoldHeight);
		fwrite(&_tempHoldBPM,2,1,fp);
		fwrite(&_tempHoldWidth,2,1,fp);
		fwrite(&_tempHoldHeight,2,1,fp);
		
		int _y;
		for (_y=0;_y<songHeight;++_y){
			int _x;
			for (_x=0;_x<songWidth;++_x){
				fwrite(&(songArray[_y][_x].id),1,1,fp);
				if (songArray[_y][_x].id==audioGearID){
					fwrite(songArray[_y][_x].extraData,1,AUDIOGEARSPACE*2,fp);
					u8* _tempHoldVolume = getGearVolume(songArray[_y][_x].extraData);
					fwrite(_tempHoldVolume,1,1,fp);
				}
			}
		}

		fwrite(METADATAMARKER,strlen(METADATAMARKER),1,fp);
		if (currentSongMetadata==NULL){
			u8 _tempStoreLength=0;
			fwrite(&_tempStoreLength,1,1,fp);
		}else{
			u8 _tempStoreLength = strlen(currentSongMetadata);
			fwrite(&_tempStoreLength,1,1,fp);
			if (fwrite(currentSongMetadata,1,strlen(currentSongMetadata),fp)!=_tempStoreLength){
				printf("Error when writing metadata.\n");
			}
		}

		fwrite(SAVEENDMARKER,strlen(SAVEENDMARKER),1,fp);
		fclose(fp);
	}else{
		char* _tempString = extraStrdup("Failed to open file. "CANTOPENMESSAGE" ",strlen(_passedFilename));
		strcat(_tempString,_passedFilename);
		easyMessage(_tempString);
		free(_tempString);
		return 0;
	}
	return 1;
}

#include "songLoaders.h"

void uiSave(){
	char* _chosenFile = selectSaveFile();
	if (_chosenFile!=NULL){
		if (checkFileExist(_chosenFile)){
			if (optionExitConfirmation){
				char* _tempString = extraStrdup("Overwrite ",strlen(_chosenFile));
				strcat(_tempString,_chosenFile);
				char _choice = easyChoice(_tempString,"No","Yes");
				free(_tempString);
				if (_choice==0){
					return;
				}
			}
		}
		if (saveSong(_chosenFile)){
			goodSetTitlebar(NULL,0);
			unsavedChanges=0;
		}
	}
	free(_chosenFile);
}

void uiLoad(){
	if (currentlyPlaying){
		playAtPosition(0);
	}
	
	char* _chosenFile = selectLoadFile();
	if (_chosenFile!=NULL){
		if ( !optionExitConfirmation || !unsavedChanges || easyChoice("Really load?","No","Yes")){
			free(currentSongMetadata);
			currentSongMetadata=NULL;

			char _returnCode = loadSong(_chosenFile);
			if (_returnCode){
				if (_returnCode==1){
					easyMessage("Unknown file format");
				}else if (_returnCode==2){
					easyMessage("Could not open or find file.");
				}else{
					printf("Is %d\n",_returnCode);
					easyMessage("Unknown bad loading return code.");
				}
			}else{
				// Song loading code sets this flag, unset it.
				unsavedChanges=0;
				goodSetTitlebar(_chosenFile,0);
			}
		}
	}
	free(_chosenFile);
}

void uiKeyConf(){
	int _columnWidth=-1;
	int _iconHeight = singleBlockSize+CONSTCHARW;
	int _elementsPerColumn = (visiblePageHeight*singleBlockSize)/(_iconHeight);
	int _totalNoteColumns = ceil(totalNotes/(double)_elementsPerColumn);
	int _totalUIColumns = ceil(totalUI/(double)_elementsPerColumn);

	char _readyForInput=0;
	char _isUISelected;
	int _selectedIndex;
	while(1){
		controlsStart();
		noteUIControls();
		if (!_readyForInput){
			if (lastSDLPressedKey==SDLK_ESCAPE){
				break;
			}
			if (wasJustPressed(SCE_TOUCH)){
				int _fixedTouchX = touchX-globalDrawXOffset;
				int _fixedTouchY = touchY-globalDrawYOffset;
				int _selectedColumn = _fixedTouchX/_columnWidth;
				int _iconYSelect = _fixedTouchY/_iconHeight;
				if (_selectedColumn<_totalNoteColumns){
					_isUISelected=0;
					_selectedIndex = _iconYSelect+_selectedColumn*_elementsPerColumn;
					if (_selectedIndex<totalNotes){
						_readyForInput=1;
					}
				}else{
					_isUISelected=1;
					_selectedIndex = _iconYSelect+(_selectedColumn-_totalNoteColumns)*_elementsPerColumn;
					if (_selectedIndex<totalUI){
						_readyForInput=1;
					}
				}

				if (lastClickWasRight){
					if (_isUISelected){
						uiHotkeys[_selectedIndex]=SDLK_UNKNOWN;
					}else{
						noteHotkeys[_selectedIndex]=SDLK_UNKNOWN;
					}
					_readyForInput=0;
					_columnWidth=-1;
				}
			}
		}else{
			if (wasJustPressed(SCE_TOUCH)){
				// Do a frame perfect keypress and click at the same time to reset all hotkeys
				if (lastSDLPressedKey!=SDLK_UNKNOWN){
					printf("Reset all hotkeys.\n");
					int i;
					for (i=0;i<totalNotes;++i){
						noteHotkeys[i]=SDLK_UNKNOWN;
					}
					for (i=0;i<totalUI;++i){
						uiHotkeys[i] = SDLK_UNKNOWN;
					}
				}else{
					_readyForInput=0;
				}
			}else{
				if (lastSDLPressedKey!=SDLK_UNKNOWN){
					// Special cases first
					if (lastSDLPressedKey==SDLK_ESCAPE){
						_readyForInput=0;
					}else{
						// Overwrite duplicate hotkeys
						int i;
						for (i=0;i<totalNotes;++i){
							if (noteHotkeys[i]==lastSDLPressedKey){
								noteHotkeys[i] = SDLK_UNKNOWN;
							}
						}
						for (i=0;i<totalUI;++i){
							if (uiHotkeys[i]==lastSDLPressedKey){
								uiHotkeys[i] = SDLK_UNKNOWN;
							}
						}
						//
						if (_isUISelected){
							uiHotkeys[_selectedIndex]=lastSDLPressedKey;
						}else{
							noteHotkeys[_selectedIndex]=lastSDLPressedKey;
						}
						_readyForInput=0;
						_columnWidth=-1;
					}
				}
			}
		}
		controlsEnd();

		if (_columnWidth==-1){
			int _foundMaxStrlen=1;
			int i;
			for (i=0;i<totalNotes;++i){
				if (noteHotkeys[i]!=SDLK_UNKNOWN){
					const char* _lastStr = SDL_GetKeyName(noteHotkeys[i]);
					if (strlen(_lastStr)>_foundMaxStrlen){
						_foundMaxStrlen=strlen(_lastStr);
					}
				}
			}
			for (i=0;i<totalUI;++i){
				if (uiHotkeys[i]!=SDLK_UNKNOWN){
					const char* _lastStr = SDL_GetKeyName(uiHotkeys[i]);
					if (strlen(_lastStr)>_foundMaxStrlen){
						_foundMaxStrlen=strlen(_lastStr);
					}
				}
			}
			_columnWidth = singleBlockSize+CONSTCHARW*_foundMaxStrlen;
		}

		startDrawing();
		if (_readyForInput==0){
			int i;
			// Draw notes
			for (i=0;i<_totalNoteColumns;++i){
				int j;
				int _totalDraw = totalNotes-i*_elementsPerColumn<_elementsPerColumn ? totalNotes-i*_elementsPerColumn : _elementsPerColumn;
				for (j=0;j<_totalDraw;++j){
					int _drawX = i*(_columnWidth);
					int _drawY = j*(_iconHeight);
					drawImageScaleAlt(noteImages[i*_elementsPerColumn+j],_drawX,_drawY,generalScale,generalScale);
					if (noteHotkeys[i*_elementsPerColumn+j]!=SDLK_UNKNOWN){
						drawString(SDL_GetKeyName(noteHotkeys[i*_elementsPerColumn+j]),_drawX+singleBlockSize,_drawY+CONSTCHARW/2);
					}
				}
			}
			// Draw UI
			for (i=0;i<_totalUIColumns;++i){
				int j;
				int _totalDraw = totalUI-i*_elementsPerColumn<_elementsPerColumn ? totalUI-i*_elementsPerColumn : _elementsPerColumn;
				for (j=0;j<_totalDraw;++j){
					int _drawX = (i+_totalNoteColumns)*(_columnWidth);
					int _drawY = j*(_iconHeight);
					drawImageScaleAlt(myUIBar[i*_elementsPerColumn+j].image,_drawX,_drawY,generalScale,generalScale);
					if (uiHotkeys[i*_elementsPerColumn+j]!=SDLK_UNKNOWN){
						drawString(SDL_GetKeyName(uiHotkeys[i*_elementsPerColumn+j]),_drawX+singleBlockSize,_drawY+CONSTCHARW/2);
					}
				}
			}

			drawString("Click an icon to set a hotkey for it.",CONSTCHARW,visiblePageHeight*singleBlockSize);
			drawString("Press esc to go back.",CONSTCHARW,visiblePageHeight*singleBlockSize+CONSTCHARW);
		}else{
			drawImageScaleAlt(_isUISelected ? myUIBar[_selectedIndex].image : noteImages[_selectedIndex],CONSTCHARW,CONSTCHARW,generalScale*2,generalScale*2);
			drawString("Press a key to bind it to this.",CONSTCHARW,generalScale*2*singleBlockSize+CONSTCHARW*2);
			drawString("Click anywhere to cancel.",CONSTCHARW,generalScale*2*singleBlockSize+CONSTCHARW*3);
		}

		endDrawing();
	}
	saveHotkeys();
}

void uiUIScroll(){
	if (uiScrollOffset==0){
		uiScrollOffset=uiUIScrollIndex;
	}else{
		uiScrollOffset=0;
	}
}

void uiMetadata(){
	char* _newMetadata = textInput(currentSongMetadata!=NULL ? currentSongMetadata : "","","Song metadata");
	if (_newMetadata!=NULL){
		if (strlen(_newMetadata)>255){
			easyMessage("Too long.");
			free(_newMetadata);
		}else{
			free(currentSongMetadata);
			currentSongMetadata = _newMetadata;
		}
	}
}

void uiSetMasterVolume(){
	s8 _newVolume=-1;
	do{
		_newVolume = getNumberInput("Master volume (1-100)",masterVolume);
	}while(!(_newVolume>=0 && _newVolume<=100));
	masterVolume = _newVolume;
	saveSettings();
	if (masterVolume>75){
		easyMessage(">75 is ear rape territory");
	}
}

void uiTheme(){
	currentThemeIndex = loadTheme(currentThemeIndex+1);
	saveSettings();
}

void uiCredits(){
	while(1){
		controlsStart();
		if (wasJustPressed(SCE_TOUCH)){
			break;
		}
		controlsEnd();
		startDrawing();
		drawString("MyLegGuy - Programming",0,0);
		drawString("HonestyCow - Original Sound matching",0,CONSTCHARW);
		drawString("D.RS - Original Themes",0,CONSTCHARW*2);
		drawString("Bonk - BPM Forumla",0,CONSTCHARW*3);
		drawString(VERSIONSTRING,0,CONSTCHARW*5);
		drawString(__DATE__,0,CONSTCHARW*6);
		drawString(__TIME__,0,CONSTCHARW*7);
		if (isMobile){ // PC users can view license file directly
			drawString(STUPID_NOTICE_1,0,CONSTCHARW*9);
			drawString(STUPID_NOTICE_2,0,CONSTCHARW*10);
			drawString(STUPID_NOTICE_3,0,CONSTCHARW*11);
		}
		endDrawing();
	}
	controlsResetEmpty();
}

void uiSettings(){
	controlLoop(); // Because we're coming from the middle of a control loop.
	u8 _totalSettings=6;

	// No exit confirmations on mobile, so no need for those settings
	if (!isMobile){
		_totalSettings+=1;
	}else{
		_totalSettings+=1; // Even though this line is the same as the one above, it may not always be. Do not merge.
	}

	char* _settingsText[_totalSettings];
	u8* _settingsValues[_totalSettings];

	_settingsText[0]="Play on place";
		_settingsValues[0]=&optionPlayOnPlace;
	_settingsText[1]="Zero based position";
		_settingsValues[1]=&optionZeroBasedPosition;
	_settingsText[2]="Fancy page transition";
		_settingsValues[2]=&optionDoFancyPage;
	_settingsText[3]="Centered play bar";
		_settingsValues[3]=&optionDoCenterPlay;
	_settingsText[4]="Check for updates";
		_settingsValues[4]=&optionUpdateCheck;
	_settingsText[5]="Confirmation";
		_settingsValues[5]=&optionExitConfirmation;
	if (!isMobile){
		_settingsText[_totalSettings-1]="Double X allows confirmation override";
			_settingsValues[_totalSettings-1]=&optionDoubleXAllowsExit;
	}else{
		_settingsText[_totalSettings-1]="Alternate song location";
			_settingsValues[_totalSettings-1]=&useAltDataDirectory;
	}

	while (1){
		controlsStart();
		if (wasJustPressed(SCE_TOUCH)){
			int _fixedTouchY = touchY-globalDrawYOffset;
			// If we tapped the exit string
			if (_fixedTouchY>logicalScreenHeight-singleBlockSize){
				break;
			}
			int _touchedEntry = _fixedTouchY/singleBlockSize;
			if (_touchedEntry<_totalSettings){
				*_settingsValues[_touchedEntry] = !*_settingsValues[_touchedEntry];
				if (_settingsValues[_touchedEntry]==&useAltDataDirectory){ // Special case for when you toggle alt data directory
					controlsResetEmpty();
					if (easyChoice("Only change the song location if you know what you're doing. Continue?","No","Yes")){
						if (useAltDataDirectory){
							if (!canWriteAltDataPath()){
								easyMessage("To use the alternate data directory, go to the app's info, go to the permissions menu, and turn storage permission on.");
								useAltDataDirectory=0;
							}
						}
					}else{
						useAltDataDirectory=!useAltDataDirectory;
					}
				}else if (_settingsValues[_touchedEntry]==&optionUpdateCheck){
					easyMessage("Update check functionality has been removed but I'm too lazy to delete this option.");
					optionUpdateCheck=0;
				}
			}
		}
		controlsEnd();
		startDrawing();
		u8 i;
		for (i=0;i<_totalSettings;++i){
			if (*_settingsValues[i]==1){
				drawRectangle(0,i*singleBlockSize,singleBlockSize,singleBlockSize,0,255,0,255);
			}else{
				drawRectangle(0,i*singleBlockSize,singleBlockSize,singleBlockSize,255,0,0,255);
			}
			drawString(_settingsText[i],singleBlockSize,i+i*singleBlockSize+CONSTCHARW/2);
		}
		drawString(CLICKTOGOBACK,0,logicalScreenHeight-CONSTCHARW);
		endDrawing();
	}
	setSongXOffset(songXOffset);
	controlLoop();
	//
	saveSettings();
}

void uiResizeSong(){
	int _newSongWidth=0;
	do{
		_newSongWidth = getNumberInput("Enter song width",songWidth);
	}while(_newSongWidth<pageWidth);
	if (songWidth!=_newSongWidth){
		setSongWidth(songArray,songWidth,_newSongWidth);
		songWidth=_newSongWidth;
		setSongXOffset(songXOffset);
		findMaxX();
	}
}

void uiBPM(){
	do{
		bpm=getNumberInput("Input beats per minute.",bpm);
	}while(bpm==0);
	if (bpm<20 || bpm>200){
		if (bpm==18){ // Easter egg
			easyMessage("And remember.....decimals are friends, not numbers.");
			if (backgroundMode==BGMODE_SINGLE){
				freeTexture(bigBackground);
				bigBackground = loadEmbeddedPNG("assets/Free/Images/pcBackgroundClassic.png");
			}
		}else{
			easyMessage("Warning: Growtopia BPM goes from 20-200.");
		}
	}
}

void uiYellowPlay(){
	playAtPosition(songXOffset);
}

void uiCount(){
	controlsEnd();
	if (totalNotes==1){
		printf("Phew, prevented division by zero there.");
		return;
	}
	u16 _amountFound[totalNotes-1];
	int i, j, k;
	// Zero array
	for (i=1;i<totalNotes;++i){
		_amountFound[i-1]=0;
	}
	// Fill number array
	for (i=0;i<songHeight;++i){
		for (j=0;j<songWidth;++j){
			if (songArray[i][j].id!=0){
				++_amountFound[songArray[i][j].id-1];
			}
		}
	}
	// Convert number array to string
	char _amountsAsString[totalNotes-1][6]; // up to 99999 for each note
	u16 _maxFontWidth=0;
	for (k=1;k<totalNotes;++k){
		sprintf(_amountsAsString[k-1],"%d",_amountFound[k-1]);
		if (bitmpTextWidth(_amountsAsString[k-1])>_maxFontWidth){
			_maxFontWidth = bitmpTextWidth(_amountsAsString[k-1]);
		}
	}
	_maxFontWidth+=CONSTCHARW;

	// Space between the notes on y axis
	u16 _noteVSpace;
	if (isMobile){ // On mobile, keep everything as compressed as possible
		_noteVSpace = singleBlockSize;
	}else{
		_noteVSpace = singleBlockSize + CONSTCHARW;
	}
	u16 _columnTotalNotes = ((visiblePageHeight*singleBlockSize)/_noteVSpace);
	u16 _totalColumns = ceil((totalNotes-1)/(double)_columnTotalNotes);
	while (1){

		controlsStart();
		if (wasJustPressed(SCE_TOUCH)){
			if (floor((touchY-globalDrawYOffset)/singleBlockSize)==visiblePageHeight){
				// Because we break info control loop. This will make it so SCE_TOUCH is no longer wasJustPressed, but it will still be isDown, so we have to make sure the user tapped not on the song, but on the buttons, which require wasJustPressed
				controlLoop();
				break;
			}
		}
		controlsEnd();

		startDrawing();
		u16 _nextNoteDrawId=1;
		for (i=0;i<_totalColumns;++i){
			s16 _maxPerColumn=totalNotes-1-i*_columnTotalNotes;
			if (_maxPerColumn>_columnTotalNotes){
				_maxPerColumn = _columnTotalNotes;
			}
			for (j=0;j<_maxPerColumn;++j){
				drawImageScaleAlt(noteImages[_nextNoteDrawId],i*_maxFontWidth+singleBlockSize*i,j*_noteVSpace,generalScale,generalScale);
				drawString(_amountsAsString[_nextNoteDrawId-1],i*_maxFontWidth+singleBlockSize*i+singleBlockSize+CONSTCHARW/2,j*_noteVSpace+CONSTCHARW/2);
				++_nextNoteDrawId;
			}
		}
		drawString(CLICKTOGOBACK,0,logicalScreenHeight-CONSTCHARW);
		endDrawing();
	}
}

void uiPlay(){
	playAtPosition(0);
}

void uiUp(){
	if (songYOffset!=0){
		songYOffset--;
	}
}
void uiDown(){
	songYOffset++;
	if (songYOffset+visiblePageHeight>pageHeight){
		songYOffset = pageHeight-visiblePageHeight;
	}
}

void uiRight(){
	if (currentlyPlaying){
		return;
	}

	if (songXOffset+pageWidth>songWidth-pageWidth){ // If we would go too far
		if (songXOffset!=songWidth-pageWidth){
			pageTransition(songWidth-pageWidth);
		}else{
			pageTransition(0); // Wrap
		}
	}else{
		pageTransition(songXOffset+pageWidth);
	}
}
void uiLeft(){
	if (currentlyPlaying){
		return;
	}

	// Fix it if we're not on a mulitple of a page.
	if (songXOffset%pageWidth!=0){
		pageTransition((songXOffset/pageWidth)*pageWidth);
	}else{
		if (songXOffset<pageWidth){
			pageTransition(songWidth-pageWidth); // wrap
		}else{
			pageTransition(songXOffset-pageWidth);
		}
	}
}

void uiNoteIcon(){
	if (cursorMode!=CURSORMODE_NORMAL){
		revertCursorMode();
		return;
	}
	/*
	uiNoteIndex++;
	if (uiNoteIndex==totalNotes){
		uiNoteIndex=0;
	}
	*/
	while (1){
		controlsStart();
		if (wasJustPressed(SCE_TOUCH)){
			int _fixedTouchY = touchY-globalDrawYOffset;
			int _fixedTouchX = touchX-globalDrawXOffset;
			int _noteId = (_fixedTouchX/singleBlockSize)+((logicalScreenHeight-_fixedTouchY)/singleBlockSize)*pageWidth;
			if (_noteId<totalNotes){
				uiNoteIndex = _noteId;
			}
			break;
		}
		controlsEnd();
		startDrawing();
		int i;
		int _currentDrawY = logicalScreenHeight-singleBlockSize;
		int _currentDrawX = 0;
		for (i=0;i<totalNotes;++i){
			drawTextureScale(noteImages[noteUIOrder[i]],_currentDrawX,_currentDrawY,generalScale,generalScale);
			
			_currentDrawX+=singleBlockSize;
			if (_currentDrawX>=pageWidth*singleBlockSize){
				_currentDrawY-=singleBlockSize;
				_currentDrawX=0;
			}
		}
		endDrawing();
	}
	controlsResetEmpty();

	updateNoteIcon();
}

void setCursorModeButtons(CrossTexture* img){
	static int ids[] = {U_SELICON,U_ERASER,U_RSELECT};
	for (int i=0;i<sizeof(ids)/sizeof(int);++i){
		changeButtonIcon(ids[i], img);
	}
}

void uiToggleEraser(){
	if (cursorMode!=CURSORMODE_NORMAL){
		revertCursorMode();
	}else{
		cursorMode=CURSORMODE_ERASER;
		setCursorModeButtons(eraserTurnOffImage);
	}
}
void revertCursorMode(){
	if (cursorMode==CURSORMODE_NORMAL){
		return;
	}
	cursorMode=CURSORMODE_NORMAL;
	// fix icons
	updateNoteIcon();
	changeButtonIcon(U_ERASER, eraserTurnOnImage);
	changeButtonIcon(U_RSELECT, rectSelectTurnOnImage);
	// if more work was needed for a specific mode, it would go here.
	selectpos[0]=-1;
}

void uiRectSelect(){
	if (cursorMode!=CURSORMODE_NORMAL){
		revertCursorMode();
	}else{
		cursorMode=CURSORMODE_SELECT;
		setCursorModeButtons(rectSelectTurnOffImage);
	}
}
int uiReqSelection(){
	if (cursorMode!=CURSORMODE_SELECT){
		easyMessage("This button is used in selection mode.");
		return 1;
	}
	return 0;
}
int uiReqBothSelection(){
	if (uiReqSelection()){
		return 1;
	}else if (selectpos[0]==-1 || selectpos[2]==-1){
		easyMessage("This button requires both points of the selection to be set.");
		return 1;
	}
	return 0;
}
int uiReqSingleSelection(){
	if (totalNotes>(0x21 - 0x7E)+1){
	}
	if (uiReqSelection()){
		return 1;
	}else if (selectpos[0]==-1 || selectpos[2]!=-1){
		easyMessage("This button requires exactly one point of the selection to be set.");
		return 1;
	}
	return 0;
}
static int uiRequireCopyLimit(){
	if (totalNotes>MAXCLIPNUM){
		easyMessage("Too many note types defined for copy/paste to work. What year is it?");
		return 1;
	}
	return 0;
}
void uiCopy(){
	if (uiReqBothSelection() || uiRequireCopyLimit()){
		return;
	}
	free(internalClipboard);
	internalClipboard=NULL;
	int startx,starty,endx,endy;
	getSelectionCorners(&startx, &starty, &endx, &endy);
	char* strbuff=makeclipbuff(songArray, startx, starty, endx, endy);
	if (!strbuff){
		easyMessage("failed to allocate clipboard buff");
	}else{
		if (SDL_SetClipboardText(strbuff) || !SDL_HasClipboardText()){
			internalClipboard=strbuff;
		}else{
			free(strbuff);
		}
	}
	revertCursorMode();
}
void uiCut(){
	if (uiReqBothSelection() || uiRequireCopyLimit()){
		return;
	}
	int startx,starty,endx,endy;
	getSelectionCorners(&startx, &starty, &endx, &endy);
	uiCopy();
	freeSongArr(songArray,startx,starty,endx-startx+1,endy-starty+1);
	for (int j=starty;j<=endy;++j){
		for (int i=startx;i<=endx;++i){
			memset(&(songArray[j][i]),0,sizeof(noteSpot));
		}
	}
	findMaxX();
	markModified();
}
void uiPaste(){
	if (uiReqSingleSelection() || uiRequireCopyLimit()){
		return;
	}
	char* clip=internalClipboard;
	if (!clip){
		clip=SDL_GetClipboardText();
		if (clip[0]=='\0'){
			fprintf(stderr,"clipboard get failed: %s\n",SDL_GetError());
			SDL_free(clip);
		}
	}
	char* err=insertclipbuff(clip,songArray,selectpos[0],selectpos[1],songWidth,songHeight);
	if (err){
		fprintf(stderr,"paste failed\n");
		easyMessage(err);
	}
	if (clip!=internalClipboard){
		SDL_free(clip);
	}
	findMaxX();
	markModified();
	revertCursorMode();
}

void uiScriptButton(){
	easyMessage("This is a button for running trustworthy custom code.");
	char* _chosenFile = sharedFilePicker(0,"Lua Script/lua,gmsflua;",0,NULL);
	if (_chosenFile!=NULL){
		if ( !optionExitConfirmation || (!unsavedChanges || easyChoice("You have unsaved changes. If this script crashes the application, you'll lose your progress. Continue?","No","Yes"))){
			goodLuaDofile(L,_chosenFile,0);
		}
	}
	free(_chosenFile);
}
void uiZoomIn(){
	if (updateGeneralScale(generalScale+ZOOMINCREMENT,0)==0){
		generalScale+=ZOOMINCREMENT;
		saveSettings();
	}else{
		updateGeneralScale(generalScale,1);
	}
	controlsResetEmpty();
}
void uiZoomOut(){
	if ((updateGeneralScale(generalScale-ZOOMINCREMENT,0)==0) && singleBlockSize>20){
		generalScale-=ZOOMINCREMENT;
		saveSettings();
	}else{
		updateGeneralScale(generalScale,0);
	}
}
int fixX(int _x){
	return _x+globalDrawXOffset;
}

int fixY(int _y){
	return _y+globalDrawYOffset;
}

void lowReallocUI(int _newNum){
	myUIBar = realloc(myUIBar,sizeof(uiElement)*_newNum);
	uiHotkeys = recalloc(uiHotkeys,sizeof(SDL_Keycode)*(_newNum-1),sizeof(SDL_Keycode)*_newNum);
}
// Add uiElement to main myUIBar and return it
uiElement* addUI(){
	lowReallocUI(++totalUI);
	myUIBar[totalUI-1].uniqueId=U_NOTUNIQUE;
	return &(myUIBar[totalUI-1]);
}

// realloc, but new memory is zeroed out
void* recalloc(void* _oldBuffer, int _oldSize, int _newSize){
	void* _newBuffer = realloc(_oldBuffer,_newSize);
	if (_newSize > _oldSize){
		void* _startOfNewData = ((char*)_newBuffer)+_oldSize;
		memset(_startOfNewData,0,_newSize-_oldSize);
	}
	return _newBuffer;
}

// Can't change song height
// I thought about it. This function is so small, it wouldn't be worth it to just make a helper function for all arrays I want to resize.
// TODO - Memory leak can happen if user puts audio gears at the end of the song and then shrinks the song
void setSongWidth(noteSpot** _passedArray, u16 _passedOldWidth, u16 _passedWidth){
	if (_passedOldWidth==_passedWidth){
		return;
	}
	int i;
	for (i=0;i<songHeight;++i){
		_passedArray[i] = recalloc(_passedArray[i],_passedOldWidth*sizeof(noteSpot),_passedWidth*sizeof(noteSpot));
	}
}

void tempDrawImageSize(CrossTexture* _passedTexture, int _x, int _y, int _newWidth, int _newHeight){
	drawTextureScale(_passedTexture,_x,_y,_newWidth/(double)getTextureWidth(_passedTexture),_newHeight/(double)getTextureHeight(_passedTexture));
}
void drawImageScaleAlt(CrossTexture* _passedTexture, int _x, int _y, double _passedXScale, double _passedYScale){
	if (_passedTexture!=NULL){
		drawTextureScale(_passedTexture,_x,_y,_passedXScale,_passedYScale);
	}
}

CrossTexture* loadEmbeddedPNG(const char* _passedFilename){
	char* _fixedPathBuffer = malloc(strlen(_passedFilename)+strlen(getFixPathString(TYPE_EMBEDDED))+1);
	fixPath((char*)_passedFilename,_fixedPathBuffer,TYPE_EMBEDDED);
	CrossTexture* _loadedTexture;
	//if (checkFileExist(_fixedPathBuffer)){
		_loadedTexture = loadPNG(_fixedPathBuffer);
	//}else{
	//	_loadedTexture = NULL;
	//}
	free(_fixedPathBuffer);
	return _loadedTexture;
}

// Return a path that may or may not be fixed to TYPE_EMBEDDED
char* possiblyFixPath(const char* _passedFilename, char _shouldFix){
	if (_shouldFix){
		char* _fixedPathBuffer = malloc(strlen(_passedFilename)+strlen(getFixPathString(TYPE_EMBEDDED))+1);
		fixPath((char*)_passedFilename,_fixedPathBuffer,TYPE_EMBEDDED);
		return _fixedPathBuffer;
	}else{
		return strdup(_passedFilename);
	}
}

void drawString(const char* _passedString, int _x, int _y){
	// See fonthelper.h
	_drawString(_passedString,_x,_y,generalScale,CONSTCHARW);
}

// Returns....
// 0 for red choice
// 1 for pressing esc
// 2 for green choice
// 3 for pressing enter
char easyChoice(char* _title, char* _redChoice, char* _greenChoice){
	char _isDone=0;
	char _returnValue=0;
	int _optionsDrawY = (logicalScreenHeight - CONSTCHARW*2)/2-(CONSTCHARW/2);
	char* _wrappedMessage = wrapText(_title);
	while(!_isDone){
		controlsStart();
		if (lastSDLPressedKey!=SDLK_UNKNOWN){
			if (lastSDLPressedKey==SDLK_ESCAPE){
				_isDone=1;
				_returnValue=1;
			}else if (lastSDLPressedKey==SDLK_RETURN || lastSDLPressedKey==SDLK_KP_ENTER){
				_isDone=1;
				_returnValue=3;
			}
		}
		if (wasJustPressed(SCE_TOUCH)){
			int _fixedTouchY = touchY-globalDrawYOffset;
			int _fixedTouchX = touchX-globalDrawXOffset;
			if (_fixedTouchY>singleBlockSize*2){
				_isDone=1;
				if (_fixedTouchX>logicalScreenWidth/2){ // If we pressed the green choice
					_returnValue=2;
				}else{ // If we pressed the red choice
					_returnValue=0;
				}
			}
		}
		controlsEnd();
		startDrawing();
		int i;
		int _currentDrawPosition=0;
		for (i=0;;++i){
			drawString(&(_wrappedMessage[_currentDrawPosition]),logicalScreenWidth/2-bitmpTextWidth(&(_wrappedMessage[_currentDrawPosition]))/2,CONSTCHARW/2+i*singleBlockSize);
			_currentDrawPosition+=strlen(&(_wrappedMessage[_currentDrawPosition]));
			if (_wrappedMessage[_currentDrawPosition+1]==BONUSENDCHARACTER){
				break;
			}else{
				_currentDrawPosition++;
			}
		}

		//drawString(_title,logicalScreenWidth/2-CONSTCHARW*(strlen(_title)/2),CONSTCHARW/2);

		drawRectangle(logicalScreenWidth/4-CONSTCHARW*(strlen(_redChoice)/2+1),_optionsDrawY-CONSTCHARW,CONSTCHARW*(strlen(_redChoice)+2),CONSTCHARW*3,255,53,53,255);
		drawString(_redChoice,logicalScreenWidth/4-CONSTCHARW*(strlen(_redChoice)/2),_optionsDrawY);
		
		drawRectangle(logicalScreenWidth/2+logicalScreenWidth/4-CONSTCHARW*(strlen(_greenChoice)/2+1),_optionsDrawY-CONSTCHARW,CONSTCHARW*(strlen(_greenChoice)+2),CONSTCHARW*3,100,255,100,255);
		drawString(_greenChoice,logicalScreenWidth/2+logicalScreenWidth/4-CONSTCHARW*(strlen(_greenChoice)/2),_optionsDrawY);
		endDrawing();
	}
	free(_wrappedMessage);
	controlsResetEmpty();
	return _returnValue;
}

char _inExitConfirmation=0;
void XOutFunction(){
	char _shouldExit=1;
	if (!isMobile && optionExitConfirmation && unsavedChanges){
		if (_inExitConfirmation){
			if (!optionDoubleXAllowsExit){
				_shouldExit=0;
			}
		}else{
			_inExitConfirmation=1;

			char _choiceResult = easyChoice("Really exit?","Exit","Don't");
			// Click red choice or press enter
			if (_choiceResult==1 || _choiceResult==2){
				_shouldExit=0;
			}

			_inExitConfirmation=0;
		}
	}
	if (_shouldExit){
		printf("Exit\n");
		exit(0);
	}else{
		// Wait for tap to end so we don't place stuff on song by mistake
		while (1){
			controlsStart();
			if (!isDown(SCE_TOUCH)){
				break;
			}
			controlsEnd();
		}
	}
}

void setTotalNotes(u16 _newTotal){
	if (_newTotal<=totalNotes){
		return;
	}

	noteImages = realloc(noteImages,sizeof(CrossTexture*)*_newTotal);
	noteSounds = realloc(noteSounds,sizeof(CROSSSFX**)*_newTotal);
	extraNoteInfo = recalloc(extraNoteInfo,sizeof(noteInfo)*totalNotes,sizeof(noteInfo)*_newTotal);
	noteUIOrder = recalloc(noteUIOrder,sizeof(u8)*totalNotes,sizeof(u8)*_newTotal);
	noteHotkeys = recalloc(noteHotkeys,sizeof(SDL_Keycode)*totalNotes,sizeof(SDL_Keycode)*_newTotal);

	int i;
	for (i=totalNotes;i<_newTotal;++i){
		noteSounds[i] = calloc(1,songHeight*sizeof(CROSSSFX*));
	}

	totalNotes = _newTotal;
}

// string
int L_loadSound(lua_State* passedState){
	char* _fixedPath = possiblyFixPath(lua_tostring(passedState,1), (lua_gettop(passedState)==2 && lua_toboolean(passedState,2)==0) ? 0 : 1);
	lua_pushlightuserdata(passedState,loadSound(_fixedPath));
	free(_fixedPath);
	return 1;
}
int L_loadImage(lua_State* passedState){
	char* _fixedPath = possiblyFixPath(lua_tostring(passedState,1), (lua_gettop(passedState)==2 && lua_toboolean(passedState,2)==0) ? 0 : 1);
	lua_pushlightuserdata(passedState,loadPNG(_fixedPath));
	free(_fixedPath);
	return 1;
}
int L_getMobile(lua_State* passedState){
	lua_pushboolean(passedState,isMobile);
	return 1;
}
// table of 14 loaded images
int L_setBgParts(lua_State* passedState){
	backgroundMode=BGMODE_PART;
	bgPartsEmpty = malloc(sizeof(CrossTexture*)*songHeight);
	bgPartsLabel = malloc(sizeof(CrossTexture*)*songHeight);
	int i;
	for (i=0;i<songHeight;++i){
		// Get empty image
		lua_rawgeti(passedState,1,i+1);
		bgPartsEmpty[i] = lua_touserdata(passedState,-1);
		lua_pop(passedState,1);
		// Get label image
		lua_rawgeti(passedState,2,i+1);
		bgPartsLabel[i] = lua_touserdata(passedState,-1);
		lua_pop(passedState,1);
	}
	return 0;
}
int L_setBigBg(lua_State* passedState){
	backgroundMode=BGMODE_SINGLE;
	char* _fixedPath = possiblyFixPath(lua_tostring(passedState,1), (lua_gettop(passedState)==2 && lua_toboolean(passedState,2)==0) ? 0 : 1);
	bigBackground = loadPNG(_fixedPath);
	free(_fixedPath);
	return 0;
}
// <int slot> <loaded image> <table of loaded sounds 14 elements long>
// Third argument is optional to disable sound effect
int L_addNote(lua_State* passedState){
	int _passedSlot = lua_tonumber(passedState,1);
	// Don't need to check if we're actually adding
	setTotalNotes(_passedSlot+1);

	noteUIOrder[_passedSlot]=_passedSlot;
	noteImages[_passedSlot] = lua_touserdata(passedState,2);
	
	if (lua_gettop(passedState)==3){
		// Load array data
		int i;
		for (i=0;i<songHeight;++i){
			lua_rawgeti(passedState,3,i+1);
			noteSounds[_passedSlot][i] = lua_touserdata(passedState,-1);
			lua_pop(passedState,1);
		}
	}else{
		int i;
		for (i=0;i<songHeight;++i){
			noteSounds[_passedSlot][i] = NULL;
		}
	}
	return 0;
}
int L_setNoteGearData(lua_State* passedState){
	int i = lua_tonumber(passedState,1);
	extraNoteInfo[i].letter = lua_tostring(passedState,2)[0];
	extraNoteInfo[i].accidental = lua_tostring(passedState,3)[0];
	return 0;
}
int L_swapUI(lua_State* passedState){
	int _slot1 = lua_tonumber(passedState,1);
	int _slot2 = lua_tonumber(passedState,2);
	uiElement _tempSwapHold = myUIBar[_slot1];
	myUIBar[_slot1] = myUIBar[_slot2];
	myUIBar[_slot2]=_tempSwapHold;
	return 0;
}
int L_deleteUI(lua_State* passedState){
	int _slotIHate = lua_tonumber(passedState,1);
	myUIBar[_slotIHate].image=NULL;
	myUIBar[_slotIHate].activateFunc=NULL;
	myUIBar[_slotIHate].uniqueId=-1;
	if (_slotIHate==totalUI-1){
		totalUI--;
	}
	return 0;
}
// Add a UI slot and return its number
int L_addUI(lua_State* passedState){
	addUI();
	lua_pushnumber(passedState,totalUI-1);
	return 1;
}
int L_getTotalUI(lua_State* passedState){
	lua_pushnumber(passedState,totalUI);
	return 1;
}
int L_setSpecialID(lua_State* passedState){
	const char* _passedIdentifier = lua_tostring(passedState,1);
	if (strcmp(_passedIdentifier,"repeatStart")==0){
		repeatStartID = lua_tonumber(passedState,2);
	}else if (strcmp(_passedIdentifier,"repeatEnd")==0){
		repeatEndID = lua_tonumber(passedState,2);
	}else if (strcmp(_passedIdentifier,"audioGear")==0){
		audioGearID = lua_tonumber(passedState,2);
	}else{
		printf("Invalid identifier %s\n",_passedIdentifier);
	}
	return 0;
}
int L_swapNoteUIOrder(lua_State* passedState){
	int _sourceIndex = lua_tonumber(passedState,1);
	int _destIndex = lua_tonumber(passedState,2);
	u8 _oldDestValue = noteUIOrder[_destIndex];
	noteUIOrder[_destIndex]=noteUIOrder[_sourceIndex];
	noteUIOrder[_sourceIndex]=_oldDestValue;
	return 0;
}
int L_getSongWidth(lua_State* passedState){
	lua_pushnumber(passedState,songWidth);
	return 1;
}
int L_getSongHeight(lua_State* passedState){
	lua_pushnumber(passedState,songHeight);
	return 1;
}
int L_setSongWidth(lua_State* passedState){
	setSongWidth(songArray,songWidth,lua_tonumber(passedState,1));
	songWidth = lua_tonumber(passedState,1);
	return 0;
}

int L_getNoteSpot(lua_State* passedState){
	int _x = lua_tonumber(passedState,1);
	int _y = lua_tonumber(passedState,2);
	lua_pushnumber(passedState,songArray[_y][_x].id);
	return 1;
}
int L_setNoteSpot(lua_State* passedState){
	_placeNoteLow(lua_tonumber(passedState,1),lua_tonumber(passedState,2),lua_tonumber(passedState,3),0,songArray);
	return 0;
}
int L_getAudioGearSize(lua_State* passedState){
	lua_pushnumber(passedState,AUDIOGEARSPACE);
	return 1;
}
// x y
int L_getAudioGear(lua_State* passedState){
	int _x = lua_tonumber(passedState,1);
	int _y = lua_tonumber(passedState,2);
	if (songArray[_y][_x].id!=audioGearID){
		return 0;
	}
	u8* _gearData = songArray[_y][_x].extraData;
	// New table is at the top of the stack
	lua_newtable(passedState);
	int i;
	for (i=0;i<AUDIOGEARSPACE;++i){
		// Add the note ID to the table first
		// First push index of the place we want to set in the table
		lua_pushnumber(passedState,(i*2)+1);
		// Push our value for that index
		lua_pushnumber(passedState,_gearData[i*2]);
		// This pops the top two from the stack, the table is now back at the top
		lua_settable(passedState,-3);

		// Same as before, but we push add the Y position this time. It is possible to push uninitialized values here, but only if the pushed note ID is 0, so the programmer should ignore it anyway
		lua_pushnumber(passedState,(i*2)+2);
		lua_pushnumber(passedState,_gearData[i*2+1]);
		lua_settable(passedState,-3);
	}
	// Table is still at the top of the stack
	// Push the volume
	lua_pushnumber(passedState,*getGearVolume(_gearData));
	return 2;
}
// x y table volume
int L_setAudioGear(lua_State* passedState){
	int _x = lua_tonumber(passedState,1);
	int _y = lua_tonumber(passedState,2);
	lua_len(passedState,3);
		int _tableLength = lua_tonumber(passedState,-1);
		lua_pop(passedState,1);
	if (songArray[_y][_x].id!=audioGearID){
		_placeNoteLow(_x,_y,audioGearID,0,songArray);
	}
	u8* _gearData = songArray[_y][_x].extraData;
	_tableLength/=2; // Half of the entries
	int i;
	for (i=0;i<_tableLength;++i){
		lua_rawgeti(passedState,3,(i*2)+1); // Get ID from table
		_gearData[i*2]=lua_tonumber(passedState,-1);
		lua_pop(passedState,1);

		lua_rawgeti(passedState,3,(i*2)+2); // Get Y position from table
		_gearData[i*2+1]=lua_tonumber(passedState,-1);
		lua_pop(passedState,1);
	}
	*getGearVolume(_gearData) = lua_tonumber(passedState,4);
	return 0;
}
// one argument, the allowed file types
int L_selectFile(lua_State* passedState){
	char* _gottenString = sharedFilePicker(0,lua_tostring(passedState,1),0,NULL);
	if (_gottenString!=NULL){
		lua_pushstring(passedState,_gottenString);
		free(_gottenString);
		return 1;
	}else{
		return 0;
	}
}
int L_saveFile(lua_State* passedState){
	char* _gottenString = sharedFilePicker(1,lua_tostring(passedState,1),0,NULL);
	if (_gottenString!=NULL){
		lua_pushstring(passedState,_gottenString);
		free(_gottenString);
		return 1;
	}else{
		return 0;
	}
}
int L_findMaxX(lua_State* passedState){
	findMaxX();
	return 0;
}
int L_setMaxX(lua_State* passedState){
	maxX = lua_tonumber(passedState,1);
	return 0;
}
// prompt default
int L_getNumberInput(lua_State* passedState){
	long _returnedValue = getNumberInput(lua_tostring(passedState,1),lua_tonumber(passedState,2));
	lua_pushnumber(passedState,_returnedValue);
	return 1;
}
int L_getBPM(lua_State* passedState){
	lua_pushnumber(passedState,bpm);
	return 1;
}
int L_setBPM(lua_State* passedState){
	bpm = lua_tonumber(passedState,1);
	return 0;
}
int L_getThemeIndex(lua_State* passedState){
	lua_pushnumber(passedState,currentThemeIndex);
	return 1;
}
int L_loadTheme(lua_State* passedState){
	backgroundMode=BGMODE_SINGLE;
	currentThemeIndex = lua_tonumber(passedState,1);
	loadTheme(currentThemeIndex);
	return 0;
}
int L_easyMessage(lua_State* passedState){
	easyMessage((char*)lua_tostring(passedState,1));
	return 0;
}
void pushLuaFunctions(){
	LUAREGISTER(L_addNote,"addNote");
	LUAREGISTER(L_setBigBg,"setBigBg");
	LUAREGISTER(L_setBgParts,"setBgParts");
	LUAREGISTER(L_getMobile,"isMobile");
	LUAREGISTER(L_loadSound,"loadSound");
	LUAREGISTER(L_loadImage,"loadImage");
	LUAREGISTER(L_swapUI,"swapUI");
	LUAREGISTER(L_deleteUI,"deleteUI");
	LUAREGISTER(L_getTotalUI,"getTotalUI");
	LUAREGISTER(L_addUI,"addUI");
	LUAREGISTER(L_setSpecialID,"setSpecialID");
	LUAREGISTER(L_setNoteGearData,"setGearInfo");
	LUAREGISTER(L_swapNoteUIOrder,"swapNoteUIOrder");
	LUAREGISTER(L_getThemeIndex,"getThemeIndex");
	LUAREGISTER(L_loadTheme,"loadTheme");
	//
	LUAREGISTER(L_getSongWidth,"getSongWidth");
	LUAREGISTER(L_getSongHeight,"getSongHeight");
	LUAREGISTER(L_setSongWidth,"setSongWidth");
	LUAREGISTER(L_getNoteSpot,"getNoteSpot");
	LUAREGISTER(L_setNoteSpot,"setNoteSpot");
	LUAREGISTER(L_getAudioGearSize,"getAudioGearSize");
	LUAREGISTER(L_getAudioGear,"getAudioGear");
	LUAREGISTER(L_setAudioGear,"setAudioGear");
	LUAREGISTER(L_selectFile,"selectFile");
	LUAREGISTER(L_saveFile,"saveFile");
	LUAREGISTER(L_findMaxX,"findMaxX");
	LUAREGISTER(L_setMaxX,"setMaxX");
	LUAREGISTER(L_getNumberInput,"getNumberInput");
	LUAREGISTER(L_getBPM,"getBPM");
	LUAREGISTER(L_setBPM,"setBPM");
	LUAREGISTER(L_easyMessage,"easyMessage");
}

void die(const char* message){
  printf("die:\n%s\n", message);
  exit(EXIT_FAILURE);
}

void goodLuaDofile(lua_State* passedState, char* _passedFilename, char _doCrash){
	if (luaL_dofile(passedState, _passedFilename) != LUA_OK) {
		if (_doCrash){
			die(lua_tostring(L,-1));
		}else{
			printf("%s\n",lua_tostring(L,-1));
		}
	}
}

// The 0-100 scale
double volumeToPercent(s8 _passedVolume){
	return _passedVolume/(double)100;
}

void goodPlaySound(CROSSSFX* _passedSound, int _volume){
	#if !DISABLESOUND
		if (_passedSound!=NULL){
			int _noteChannel = Mix_PlayChannel( -1, _passedSound, 0 );
			Mix_Volume(_noteChannel,volumeToPercent(_volume)*volumeToPercent(masterVolume)*MIX_MAX_VOLUME);
		}
	#endif
}

void playColumn(s32 _columnNumber){
	if (_columnNumber>maxX){
		resetPlayState();
		_columnNumber=0;
		currentPlayPosition=0;
		nextPlayPosition=1;
		setSongXOffset(0);
	}
	int i;
	char _repeatAlreadyFound=0;
	char _foundRepeatY=0;
	for (i=0;i<pageHeight;++i){
		if (songArray[i][_columnNumber].id!=0){
			if (songArray[i][_columnNumber].id==repeatEndID && !_repeatAlreadyFound && songArray[i][_columnNumber].extraData!=(void*)1){
				_repeatAlreadyFound=1;
				_foundRepeatY=i;
				songArray[i][_columnNumber].extraData=(void*)1;
			}else if (songArray[i][_columnNumber].id==audioGearID){
				u8 j;
				for (j=0;j<AUDIOGEARSPACE;++j){
					if (songArray[i][_columnNumber].extraData[j*2]!=0){
						goodPlaySound(noteSounds[songArray[i][_columnNumber].extraData[j*2]][songArray[i][_columnNumber].extraData[j*2+1]],*getGearVolume(songArray[i][_columnNumber].extraData));
					}
				}
			}else{
				goodPlaySound(noteSounds[songArray[i][_columnNumber].id][i],100);
			}
		}
	}
	// Process the repeat note we found
	if (_repeatAlreadyFound){
		int _searchX;
		for (_searchX=_columnNumber-1;_searchX>=0;--_searchX){
			if (songArray[_foundRepeatY][_searchX].id==repeatStartID){
				resetRepeatNotes(_searchX+1,_columnNumber-1);
				nextPlayPosition = _searchX;
				_repeatAlreadyFound=0;
				break;
			}
		}
		// If we didn't find a repeat start to go with our repeat end
		if (_repeatAlreadyFound){
			// Jump to song start
			nextPlayPosition=0;
		}
	}
}
void _placeNoteLow(int _x, int _y, u8 _noteId, u8 _shouldPlaySound, noteSpot** _passedSong){
	if (_passedSong[_y][_x].id==audioGearID){
		free(_passedSong[_y][_x].extraData);
	}
	if (_noteId==audioGearID){
		_passedSong[_y][_x].extraData = calloc(1,gearBuffSize());
		*getGearVolume(_passedSong[_y][_x].extraData)=100;
	}else{
		_passedSong[_y][_x].extraData=NULL;
	}
	if (_shouldPlaySound){
		if (noteSounds[_noteId][_y]!=NULL){
			goodPlaySound(noteSounds[_noteId][_y],100);
		}
	}
	_passedSong[_y][_x].id=_noteId;
}
void placeNote(int _x, int _y, u16 _noteId){
	if (_noteId==audioGearID && songArray[_y][_x].id==audioGearID){
		controlLoop();
		audioGearGUI(songArray[_y][_x].extraData);
		controlLoop();
	}/*else if (songArray[_y][_x].id==_noteId){
		return;
	}*/else{
		if (_x>maxX){
			maxX=_x;
		}
		_placeNoteLow(_x,_y,_noteId,optionPlayOnPlace,songArray);
		if (_x==maxX && _noteId==0){
			findMaxX();
		}
	}
	markModified();
}

void drawPlayBar(int _x){
	drawRectangle((_x-songXOffset)*singleBlockSize,0,singleBlockSize,visiblePageHeight*singleBlockSize,128,128,128,100);
}

void drawSong(noteSpot** _songToDraw, int _drawWidth, int _drawHeight, int _xOffset, int _yOffset){
	int i;
	if (backgroundMode==BGMODE_SINGLE){
		if (_drawWidth!=25){
			drawImageScaleAlt(bigBackground,(-1*getTextureWidth(bigBackground))+singleBlockSize+singleBlockSize*_drawWidth,0,generalScale,generalScale);
		}else{
			drawImageScaleAlt(bigBackground,0,0,generalScale,generalScale);
		}
	}
	for (i=0;i<_drawHeight;++i){
		int j;
		for (j=0;j<_drawWidth;++j){
			if (_songToDraw[i+_yOffset][j+_xOffset].id==0){
				// If we're doing the special part display mode
				if (backgroundMode==BGMODE_PART){
					drawImageScaleAlt(bgPartsEmpty[i+_yOffset],j*singleBlockSize,i*singleBlockSize,generalScale,generalScale);
				}
			}else{
				drawImageScaleAlt(noteImages[_songToDraw[i+_yOffset][j+_xOffset].id],j*singleBlockSize,i*singleBlockSize,generalScale,generalScale);
			}
		}
		if (backgroundMode==BGMODE_PART){
			drawImageScaleAlt(bgPartsLabel[i+_yOffset],_drawWidth*singleBlockSize,i*singleBlockSize,generalScale,generalScale);
		}
	}
}

void noteUIControls(){
	if (wasJustPressed(SCE_MOUSE_SCROLL)){
		revertCursorMode();
		if (mouseScroll<0){
			uiNoteIndex--;
			if (uiNoteIndex<0){
				uiNoteIndex=totalNotes-1;
			}
			updateNoteIcon();
		}else if (mouseScroll>0){
			uiNoteIndex++;
			if (uiNoteIndex==totalNotes){
				uiNoteIndex=0;
			}
			updateNoteIcon();
		}// 0 scroll is ignored
	}
}

void drawSingleUI(uiElement* _passedElement, int _drawSlot){
	drawImageScaleAlt(_passedElement->image,_drawSlot*singleBlockSize,visiblePageHeight*singleBlockSize,generalScale,generalScale);
}

void drawUIPointers(uiElement** _passedUIBar, int _totalLength){
	int i;
	for (i=0;i<_totalLength;++i){
		drawSingleUI(_passedUIBar[i],i);
	}
}

void drawUIBar(uiElement* _passedUIBar){
	int i;
	for (i=uiScrollOffset;i<uiScrollOffset+uiPageSize;++i){
		if (i==totalUI){
			break;
		}
		drawSingleUI(&(_passedUIBar[i]),(i-uiScrollOffset));
	}
}
//
void maybeDrawSelectionCorner(int absolutex, int absolutey){
	if (absolutex!=-1){
		drawRectangle(absolutex,absolutey,singleBlockSize,singleBlockSize,232,147,36,200);
	}
}
int gmsfmin(int a, int b){
	return a<b ? a : b;
}
int gmsfmax(int a, int b){
	return a>b ? a : b;
}
void getSongOverlayPos(int tilexin, int tileyin, int songoffx, int songoffy, int* rx, int* ry){
	*rx=tilexin-songoffx;
	*ry=tileyin-songoffy;
	if (*rx<pageWidth && *ry<visiblePageHeight){
		(*rx)*=singleBlockSize;
		(*ry)*=singleBlockSize;
	}else{
		*rx=-1;
	}
}
void getSelectionCorners(int* startx, int* starty, int* endx, int* endy){
	*startx=gmsfmin(selectpos[0],selectpos[2]);
	*starty=gmsfmin(selectpos[1],selectpos[3]);
	*endx=gmsfmax(selectpos[0],selectpos[2]);
	*endy=gmsfmax(selectpos[1],selectpos[3]);
}
void drawSelectionMisc(){
	if (cursorMode==CURSORMODE_SELECT){
		drawRectangle(0,0,pageWidth*singleBlockSize,visiblePageHeight*singleBlockSize,0,0,255,50);
		if (selectpos[0]!=-1){
			int xdraw,ydraw;
			getSongOverlayPos(selectpos[0],selectpos[1],songXOffset,songYOffset,&xdraw,&ydraw);
			maybeDrawSelectionCorner(xdraw,ydraw);
			if (selectpos[2]!=-1){
				getSongOverlayPos(selectpos[2],selectpos[3],songXOffset,songYOffset,&xdraw,&ydraw);
				maybeDrawSelectionCorner(xdraw,ydraw);
				// draw rectangle
				int startx,starty,endx,endy;
				getSelectionCorners(&startx,&starty,&endx,&endy);
				startx-=songXOffset;
				starty-=songYOffset;
				endx-=songXOffset;
				endy-=songYOffset;
				drawRectangle(startx*singleBlockSize,starty*singleBlockSize,(endx-startx+1)*singleBlockSize,(endy-starty+1)*singleBlockSize,255,68,0,100);
			}
		}
	}
}
//
void doUsualDrawing(){
	drawSong(songArray,pageWidth,visiblePageHeight,songXOffset,songYOffset);
	drawSelectionMisc();
	if (currentlyPlaying){
		drawPlayBar(currentPlayPosition);
	}
	drawUIBar(myUIBar);
	drawString(positionString,uiPageSize*singleBlockSize+CONSTCHARW/2,visiblePageHeight*singleBlockSize+CONSTCHARW/2);
}

void addChar(char* _sourceString, char _addChar){
	int _gottenLength = strlen(_sourceString);
	_sourceString[_gottenLength]=_addChar;
	_sourceString[_gottenLength+1]='\0';
}

char uiHotkeyCheck(){
	int i;
	for (i=0;i<totalUI;++i){
		if (uiHotkeys[i]==lastSDLPressedKey){
			controlsEnd();
			myUIBar[i].activateFunc();
			controlLoop();
			return 1;
		}
	}
	return 0;
}

char noteHotkeyCheck(){
	int i;
	for (i=0;i<totalNotes;++i){
		if (noteHotkeys[i]==lastSDLPressedKey){
			// noteUIOrder
			int j;
			for (j=0;j<totalNotes;++j){
				if (noteUIOrder[j]==i){
					revertCursorMode();
					uiNoteIndex=j;
					updateNoteIcon();
					return 1;
				}
			}
		}
	}
	return 0;
}

char* makeAudioGearString(noteSpot** _passedSong){
	char* _completeString = malloc(AUDIOGEARSPACE*3+1+(AUDIOGEARSPACE-1));
	_completeString[0]='\0';
	int i;
	for (i=0;i<AUDIOGEARSPACE;++i){
		int j;
		for (j=0;j<songHeight;++j){
			if (_passedSong[j][i].id!=0){
				if (i!=0){
					addChar(_completeString,' ');
				}
				addChar(_completeString,extraNoteInfo[_passedSong[j][i].id].letter);
				addChar(_completeString,noteNames[j]);
				addChar(_completeString,extraNoteInfo[_passedSong[j][i].id].accidental);
				break;
			}
		}
	}
	if (strlen(_completeString)==0){
		strcpy(_completeString,"NoData"); // Shrinking AUDIOGEARSPACE too much could cause this to overflow
	}
	return _completeString;
}

void audioGearGUI(u8* _gearData){
	// UI variables
	uiElement* _foundUpUI = NULL;
	uiElement* _gearUIPointers[7];
	u8* _gearVolume = getGearVolume(_gearData);
	u8 _totalGearUI=0;
	// Add some UI elements
	_gearUIPointers[_totalGearUI++] = getUIByID(U_SELICON);
	// If we have up and down buttons, add them to this too.
	_foundUpUI = getUIByID(U_UPBUTTON);
	if (_foundUpUI!=NULL){
		_gearUIPointers[_totalGearUI++]=_foundUpUI;
		_gearUIPointers[_totalGearUI++]=getUIByID(U_DOWNBUTTON);
	}
	// Always have these
	_gearUIPointers[_totalGearUI++]=&volumeButtonUI;
	_gearUIPointers[_totalGearUI++]=&infoButtonUI;
	_gearUIPointers[_totalGearUI++]=&backButtonUI;

	noteSpot** _fakedMapArray=malloc(sizeof(noteSpot*)*songHeight);
	int i;
	for (i=0;i<songHeight;++i){
		_fakedMapArray[i] = calloc(1,sizeof(noteSpot)*AUDIOGEARSPACE);
	}
	for (i=0;i<AUDIOGEARSPACE;++i){
		if (_gearData[i*2]!=0){
			_fakedMapArray[_gearData[i*2+1]][i].id=_gearData[i*2];
		}
	}
	// Init info for user to see
	char _volumeString[strlen("Vol: ")+3+1];
		sprintf(_volumeString,"Vol: %d",*_gearVolume);
	char* _completeGearString=NULL;
	if (!isMobile){
		_completeGearString = makeAudioGearString(_fakedMapArray);
	}
	while(1){
		controlsStart();
		if (wasJustPressed(SCE_TOUCH)){
			int _placeX = touchXToBlock(touchX);
			int _placeY = touchYToBlock(touchY);
			if (_placeY==visiblePageHeight){
				if (_placeX<_totalGearUI){
					if (_gearUIPointers[_placeX]->uniqueId==U_BACK){
						break;
					}else if (_gearUIPointers[_placeX]->uniqueId==U_INFO){
						if (isMobile){
							free(_completeGearString);
							_completeGearString = makeAudioGearString(_fakedMapArray);
						}
						controlLoop();
						while(1){
							controlsStart();
							if (wasJustPressed(SCE_TOUCH)){
								break;
							}
							controlsEnd();
							startDrawing();
							drawString(_completeGearString,0,0);
							drawString(_volumeString,0,CONSTCHARW);
							endDrawing();
						}
						controlLoop();
					}else if (_gearUIPointers[_placeX]->uniqueId==U_VOL){
						u8 _newVolume;
						do{
							_newVolume = getNumberInput("Audio Gear volume (1-100)",*_gearVolume);
						}while(_newVolume<=0 || _newVolume>100);
						*_gearVolume=_newVolume;
						sprintf(_volumeString,"Vol: %d",*_gearVolume);
					}else{
						controlLoop();
						_gearUIPointers[_placeX]->activateFunc();
						controlLoop();
					}
				}
			}else{
				if (getUINoteID()==0 || extraNoteInfo[getUINoteID()].letter!=0){ // Restirct notes we can't use in audio gear to notes with audio gear letters or symbols
					if (_placeX<AUDIOGEARSPACE && _placeY<visiblePageHeight){
						// Clear any other notes we have in the same column.
						// Because 0 is a valid note id to place, clicking anywhere in a column with note id 0 will erase everything in that column
						for (i=0;i<songHeight;++i){
							_fakedMapArray[i][_placeX].id=0;
						}

						// Place our new note
						_placeNoteLow(_placeX,_placeY+songYOffset,lastClickWasRight ? 0 : getUINoteID(),optionPlayOnPlace,_fakedMapArray);
						
						if (!isMobile){
							free(_completeGearString);
							_completeGearString = makeAudioGearString(_fakedMapArray);
						}
					}
				}
			}
		}
		if (wasJustPressed(SCE_ANDROID_BACK)){
			break;
		}
		noteUIControls();
		if (lastSDLPressedKey!=SDLK_UNKNOWN){
			noteHotkeyCheck();
		}
		controlsEnd();
		startDrawing();
		drawSong(_fakedMapArray,AUDIOGEARSPACE,visiblePageHeight,0,songYOffset);
		drawUIPointers(_gearUIPointers,_totalGearUI);
		if (!isMobile){
			drawString(_completeGearString,(AUDIOGEARSPACE+2)*singleBlockSize,singleBlockSize);
			drawString(_volumeString,(AUDIOGEARSPACE+2)*singleBlockSize,singleBlockSize*2);
		}
		endDrawing();
	}
	free(_completeGearString);
	//
	// Transfer data from song map to audio gear	
	for (i=0;i<AUDIOGEARSPACE;++i){
		int j;
		_gearData[i*2]=0; // If we don't find anything in the loop, this column will be empty.
		for (j=0;j<songHeight;++j){
			if (_fakedMapArray[j][i].id!=0){
				_gearData[i*2]=_fakedMapArray[j][i].id;
				_gearData[i*2+1]=j;
				break;
			}
		}
	}
	// Free faked map
	for (i=0;i<songHeight;++i){
		free(_fakedMapArray[i]);
	}
	free(_fakedMapArray);
}

char updateGeneralScale(double _passed, char _isForced){
	if (_passed!=0){
		singleBlockSize = 32*_passed;
		logicalScreenWidth = floor(screenWidth/singleBlockSize)*singleBlockSize;
		logicalScreenHeight = floor(screenHeight/singleBlockSize)*singleBlockSize;
		globalDrawXOffset = (screenWidth-logicalScreenWidth)/2;
		globalDrawYOffset = (screenHeight-logicalScreenHeight)/2;
	}else{
		globalDrawXOffset=0;
		globalDrawYOffset=0;
		logicalScreenWidth = screenWidth;
		logicalScreenHeight = screenHeight;
		singleBlockSize=32;
	}
	visiblePageHeight = logicalScreenHeight/singleBlockSize;
	if (visiblePageHeight>15){
		visiblePageHeight=15;
	}
	visiblePageHeight--; // Leave a space for the toolbar.
	pageWidth = (logicalScreenWidth/singleBlockSize)-1;

	int _rightButtonIndex = getUIIndexByID(U_RIGHT);
	if (visiblePageHeight!=pageHeight){
		if (myUIBar[_rightButtonIndex+1].uniqueId!=U_UPBUTTON){
			*(insertUI(_rightButtonIndex+1)) = upButtonUI;
			*(insertUI(_rightButtonIndex+2)) = downButtonUI;
		}
	}

	uiPageSize=totalUI;
	// If we need more space for the UI because screen in small, add moreUI button
	// We'll be moving the more UI button or removing it, so get rid of it either way
	if (getUIIndexByID(U_MOREUIBUTTON)!=-1){
		deleteUI(getUIIndexByID(U_MOREUIBUTTON));
	}
	if (pageWidth<totalUI+3){ // 4 spaces for page number display. We only subtract 3 because we can use the labels on the far right as a UI spot
		uiPageSize=pageWidth-3;
		
		uiElement* _newButton = insertUI(uiPageSize-1);
		_newButton->image = uiScrollImage;
		_newButton->activateFunc = uiUIScroll;
		_newButton->uniqueId=U_MOREUIBUTTON;
		
		uiUIScrollIndex = uiPageSize-1;
		if (uiScrollOffset!=0){
			uiScrollOffset=uiUIScrollIndex;
		}
	}else{
		uiScrollOffset=0;
	}

	if (!_isForced){
		if (ceil((totalUI+(getUIIndexByID(U_MOREUIBUTTON)!=-1)+5)/2)>=pageWidth){
			return 1;
		}else if (singleBlockSize<20){
			return 2;
		}
	}

	return 0;
}

double tryUpdateGeneralScale(double _passed){
	double _currentAttempt = _passed;
	char _lastUpdateResult;
	for (;(_lastUpdateResult=updateGeneralScale(_currentAttempt,0))>0;){
		if (_lastUpdateResult==1){ // too big
			_currentAttempt-=ZOOMINCREMENT;
		}else if (_lastUpdateResult==2){ // too small
			_currentAttempt+=ZOOMINCREMENT;
		}
		if (_currentAttempt==_passed || _currentAttempt<0){ // We've looped
			return -1;
		}
	}
	return _currentAttempt;
}

char init(){
	#ifdef NEXUS_RES // Can test with resolution of Nexus 7 2012.
		initGraphics(1280,800,&screenWidth,&screenHeight);
	#else
		initGraphics(832,480,&screenWidth,&screenHeight);
	#endif
	goodSetTitlebar("Growtopia Music Simulator Final",0);
	setClearColor(192,192,192,255);
	if (screenWidth!=832 || screenHeight!=480){
		isMobile=1;
	}else{
		isMobile=ISTESTINGMOBILE;
	}

	//initAudio();
	// Manually do SDL2_mixer audio init
	SDL_Init( SDL_INIT_AUDIO );
	//Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 );
	Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 100 ); // Keep sound buffer small, like 100! Otherwise it's bad.
	//Mix_Init(MIX_INIT_OGG);
	Mix_AllocateChannels(14*4); // We need a lot of channels for all these music notes

	L = luaL_newstate();
	luaL_openlibs(L);
	pushLuaFunctions();

	songArray = calloc(1,sizeof(noteSpot*)*songHeight);
	setSongWidth(songArray,0,400);
	songWidth=400;
	songXOffset=0; // Raw to init values for scripts. Real set is after loading settings

	initEmbeddedFont();
	startDrawing();
	generalScale=1;
	drawString("Loading...",0,0);
	generalScale=-1;
	endDrawing();

	// Init note array before we do the Lua
	setTotalNotes(1);
	noteImages[0] = loadEmbeddedPNG("assets/Free/Images/grid.png");

	// Set up UI
	uiElement* _newButton;

	// Add play button
	_newButton = addUI();
	playButtonImage = loadEmbeddedPNG("assets/Free/Images/playButton.png");
	stopButtonImage = loadEmbeddedPNG("assets/Free/Images/stopButton.png");
	_newButton->image = playButtonImage;
	_newButton->activateFunc = uiPlay;
	_newButton->uniqueId = U_PLAY;

	// Add selected note icon
	_newButton = addUI();
	_newButton->image = NULL;
	_newButton->activateFunc = uiNoteIcon;
	_newButton->uniqueId = U_SELICON;

	// Add save Button
	_newButton = addUI();
	_newButton->image = loadEmbeddedPNG("assets/Free/Images/saveButton.png");
	_newButton->activateFunc = uiSave;
	_newButton->uniqueId = U_SAVE;

	// Add page buttons
	_newButton = addUI();
	_newButton->image = loadEmbeddedPNG("assets/Free/Images/leftButton.png");
	_newButton->activateFunc = uiLeft;
	_newButton->uniqueId = U_LEFT;
	_newButton = addUI();
	_newButton->image = loadEmbeddedPNG("assets/Free/Images/rightButton.png");
	_newButton->activateFunc = uiRight;
	_newButton->uniqueId = U_RIGHT;

	// eraser
	if (isMobile){
		eraserTurnOnImage=loadEmbeddedPNG("assets/Free/Images/eraser.png");
		eraserTurnOffImage=loadEmbeddedPNG("assets/Free/Images/eraserTurnOff.png");;

		_newButton = addUI();
		_newButton->image = eraserTurnOnImage;
		_newButton->activateFunc = uiToggleEraser;
		_newButton->uniqueId = U_ERASER;
	}
	
	// Add yellow play button
	_newButton = addUI();
	yellowPlayButtonImage = loadEmbeddedPNG("assets/Free/Images/yellowPlayButton.png");
	_newButton->image = yellowPlayButtonImage;
	_newButton->activateFunc = uiYellowPlay;
	_newButton->uniqueId = U_YPLAY;

	// Put the less used buttons here
	_newButton = addUI();
	_newButton->image = loadEmbeddedPNG("assets/Free/Images/bpmButton.png");
	_newButton->activateFunc = uiBPM;
	_newButton->uniqueId = U_BPM;
	// 
	_newButton = addUI();
	_newButton->image = loadEmbeddedPNG("assets/Free/Images/resizeButton.png");
	_newButton->activateFunc = uiResizeSong;
	_newButton->uniqueId = U_SIZE;
	//
	_newButton = addUI();
	_newButton->image = loadEmbeddedPNG("assets/Free/Images/settingsButton.png");
	_newButton->activateFunc = uiSettings;
	_newButton->uniqueId = U_SETTINGS;
	//
	_newButton = addUI();
	_newButton->image = loadEmbeddedPNG("assets/Free/Images/countButton.png");
	_newButton->activateFunc = uiCount;
	_newButton->uniqueId = U_COUNT;
	//
	_newButton = addUI();
	_newButton->image = loadEmbeddedPNG("assets/Free/Images/creditsButton.png");
	_newButton->activateFunc = uiCredits;
	_newButton->uniqueId = U_CREDITS;
	//
	_newButton = addUI();
	_newButton->image = loadEmbeddedPNG("assets/Free/Images/loadButton.png");
	_newButton->activateFunc = uiLoad;
	_newButton->uniqueId = U_LOAD;
	//
	_newButton = addUI();
	_newButton->image = loadEmbeddedPNG("assets/Free/Images/metadataButton.png");
	_newButton->activateFunc = uiMetadata;
	_newButton->uniqueId = U_METADATA;
	//
	rectSelectTurnOnImage=loadEmbeddedPNG("assets/Free/Images/rectSelectButton.png");
	rectSelectTurnOffImage=loadEmbeddedPNG("assets/Free/Images/rectSelectButtonOff.png");
	_newButton = addUI();
	_newButton->image = rectSelectTurnOnImage;
	_newButton->activateFunc = uiRectSelect;
	_newButton->uniqueId = U_RSELECT;
	//
	_newButton = addUI();
	_newButton->image = loadEmbeddedPNG("assets/Free/Images/cutButton.png");
	_newButton->activateFunc = uiCut;
	_newButton->uniqueId = U_CUT;
	_newButton = addUI();
	_newButton->image = loadEmbeddedPNG("assets/Free/Images/copyButton.png");
	_newButton->activateFunc = uiCopy;
	_newButton->uniqueId = U_COPY;
	_newButton = addUI();
	_newButton->image = loadEmbeddedPNG("assets/Free/Images/pasteButton.png");
	_newButton->activateFunc = uiPaste;
	_newButton->uniqueId = U_PASTE;
	//

	// Three general use UI buttons
	backButtonUI.image = loadEmbeddedPNG("assets/Free/Images/backButton.png");
	backButtonUI.activateFunc = NULL;
	backButtonUI.uniqueId=U_BACK;

	infoButtonUI.image = loadEmbeddedPNG("assets/Free/Images/infoButton.png");
	infoButtonUI.activateFunc = NULL;
	infoButtonUI.uniqueId=U_INFO;

	volumeButtonUI.image = loadEmbeddedPNG("assets/Free/Images/volumeButton.png");
	volumeButtonUI.activateFunc=NULL;
	volumeButtonUI.uniqueId=U_VOL;

	newButtonImage = loadEmbeddedPNG("assets/Free/Images/newButton.png");
	uiScrollImage = loadEmbeddedPNG("assets/Free/Images/moreUI.png");

	upButtonUI.activateFunc = uiUp;
	upButtonUI.uniqueId = U_UPBUTTON;
	upButtonUI.image = loadEmbeddedPNG("assets/Free/Images/upButton.png");
	downButtonUI.activateFunc = uiDown;
	downButtonUI.uniqueId = U_DOWNBUTTON;
	downButtonUI.image = loadEmbeddedPNG("assets/Free/Images/downButton.png");

	if (!isMobile){
		_newButton = addUI();
		_newButton->image = loadEmbeddedPNG("assets/Free/Images/optionsButton.png");
		_newButton->activateFunc = uiKeyConf;
		_newButton->uniqueId = U_KEYCONF;

		_newButton = addUI();
		_newButton->image = loadEmbeddedPNG("assets/Free/Images/scriptButton.png");
		_newButton->activateFunc = uiScriptButton;
		_newButton->uniqueId = U_SCRIPTBUTTON;

		// Add the shared volume button to the main UI bar when not on mobile
		uiElement* _slotForSharedVolume = addUI();
		memcpy(_slotForSharedVolume,&volumeButtonUI,sizeof(uiElement));
		_slotForSharedVolume->activateFunc=uiSetMasterVolume;
	}else{
		_newButton = addUI();
		_newButton->image = loadEmbeddedPNG("assets/Free/Images/zoomIn.png");
		_newButton->activateFunc = uiZoomIn;
		_newButton->uniqueId = U_ZOOMIN;

		_newButton = addUI();
		_newButton->image = loadEmbeddedPNG("assets/Free/Images/zoomOut.png");
		_newButton->activateFunc = uiZoomOut;
		_newButton->uniqueId = U_ZOOMOUT;
	}

	//////////////////////////////

	// Run before so theme index is loaded
	// First, try and load from safe data path
	char* _settingsFilename = fixSafeDataPath("generalSettings.legSettings");
	char _didLoadedSettings = loadSettings(_settingsFilename);
	free(_settingsFilename);
	if (!_didLoadedSettings){
		// Failed to find settings in safe location, try alt
		if (directoryExists(getAltDataPath())){
			//easyMessage("Welcome to v1.3!");
			useAltDataDirectory=1;
			// Optionally transfer their settings file to the new location
			_settingsFilename = fixAltDataPath("generalSettings.legSettings");
			if (loadSettings(_settingsFilename)){
				remove(_settingsFilename);
			}
			free(_settingsFilename);
		}else{
			// This is their first run
			//easyMessage("Welcome to Growtopia Music Simulator Final!");
		}
		// Save settings here so the useAltDataDirectory is saved or so the welcome message isn't shown every time
		saveSettings();
	}

	if (backgroundMode==BGMODE_SINGLE){
		_newButton = addUI();
		_newButton->image = loadEmbeddedPNG("assets/Free/Images/themeButton.png");
		_newButton->activateFunc = uiTheme;
		_newButton->uniqueId = U_THEME;
	}

	generalScale = tryUpdateGeneralScale(generalScale!=-1 ? generalScale : (isMobile ? 1.7 : 1));
	if (generalScale<0){
		updateGeneralScale(1,1);
		generalScale=1;
	}

	// Very last, run the init script
	fixPath("assets/Free/Scripts/init.lua",tempPathFixBuffer,TYPE_EMBEDDED);
	goodLuaDofile(L,tempPathFixBuffer,1);

	// Load hotkey config here because all UI and notes should be added by now.
	loadHotkeys();

	// Hopefully we've added some note images in the init.lua.
	updateNoteIcon();

	// After we've loaded settings
	setSongXOffset(0);

	// Fourth argument is data
	pthread_t _soundPthread;
	if(pthread_create(&_soundPthread, NULL, soundPlayerThread, NULL)) {
		printf("Could not make sound thread.");
		return 1;
	}
	optionUpdateCheck=0;
	return 0;
}
void* soundPlayerThread(void* data){
	
	while(1){
		// Process playing
		if (currentlyPlaying){
			u64 _new = getTicks();
			// If our body is ready for the next column of notes
			if (_new>=lastPlayAdvance+bpmFormula(bpm)){
				//printf("%ld\n",_new-lastPlayAdvance+bpmFormula(bpm));
				currentPlayPosition = nextPlayPosition;
				nextPlayPosition = currentPlayPosition+1; // Advance one.
				centerAround(currentPlayPosition); // Move camera
				playColumn(currentPlayPosition);
				lastPlayAdvance = getTicks(); // How long needs to pass before the next column
			}
		}
		wait(1); // Low latency
	}
	
	pthread_exit(NULL);
	return NULL;
}
int main(int argc, char *argv[]){
	printf("Loading...\n");
	uiNoteIndex=1;
	if (init()){
		printf("Init error.");
		return 1;
	}
	printf("Done loading.\n");

	s16 _lastPlaceX=-1;
	s16 _lastPlaceY=-1;

	#ifdef ENABLEFPSCOUNTER
		int _countedFrames=0;
		int _lastFrameReport=getTicks();
	#endif
	// If not -1, draw a red rectangle at this UI slot
	s32 _uiSelectedHighlight=-1;
	while(1){
		controlsStart();
		if (isDown(SCE_TOUCH)){
			if (wasJustPressed(SCE_TOUCH)){ // If we're not dragging the mouse, you're allowed to place anywhere.
				_lastPlaceY=-1;
				_lastPlaceX=-1;
			}
			// We need to use floor so that negatives are not made positive
			int _placeX = touchXToBlock(touchX);
			int _placeY = touchYToBlock(touchY);
			if (_placeY==visiblePageHeight){ // If we click the UI section
				if (wasJustPressed(SCE_TOUCH)){
					_placeX+=uiScrollOffset;
					if (!(_placeX>=totalUI)){ // If we didn't click out of bounds
						if (myUIBar[_placeX].activateFunc==NULL){
							printf("NULL activateFunc for %d\n",_placeX);
						}else{
							controlsEnd();
							myUIBar[_placeX].activateFunc();
							controlLoop();
						}
						_uiSelectedHighlight=_placeX-uiScrollOffset;
					}
				}
			}else{ // If we click the main section
				if (!(_placeX>=pageWidth || _placeX<0) && !(_placeY>=visiblePageHeight || _placeY<0)){ // In bounds in the main section, I mean.
					if (!(_placeX==_lastPlaceX && _placeY==_lastPlaceY)){ // Don't place where we've just placed. Otherwise we'd be placing the same note on top of itself 60 times per second
						_lastPlaceX = _placeX;
						_lastPlaceY = _placeY;
						if (cursorMode==CURSORMODE_SELECT){
							s32* d;
							if (selectpos[0]==-1 || (selectpos[2]!=-1)){
								selectpos[2]=-1;
								d=&selectpos[0];
							}else if (selectpos[2]==-1){
								d=&selectpos[2];
							}
							d[0]=_placeX+songXOffset;
							d[1]=_placeY+songYOffset;
						}else{
							int placeval=-1;
							if (cursorMode==CURSORMODE_NORMAL){
								placeval=lastClickWasRight ? 0 : getUINoteID();
							}else if (cursorMode==CURSORMODE_ERASER){
								placeval=0;
							}
							if (placeval!=-1){
								placeNote(_placeX+songXOffset,_placeY+songYOffset,placeval);
							}
						}
					}
				}
			}
		}
		noteUIControls();
		if (lastSDLPressedKey!=SDLK_UNKNOWN){
			if (!noteHotkeyCheck()){
				uiHotkeyCheck();
			}
		}
		controlsEnd();

		// Start drawing
		startDrawing();
		doUsualDrawing();
		// If we need to draw UI highlight
		if (_uiSelectedHighlight!=-1){
			drawRectangle(_uiSelectedHighlight*singleBlockSize,visiblePageHeight*singleBlockSize,singleBlockSize,singleBlockSize,0,0,0,100);
			_uiSelectedHighlight=-1;
		}
		endDrawing();

		#ifdef ENABLEFPSCOUNTER
			_countedFrames++;
			if (getTicks()>=_lastFrameReport+1000){
				printf("Fps:%d\n",_countedFrames);
				_lastFrameReport=getTicks();
				_countedFrames=0;
			}
		#endif
	}

	/* code */
	return 0;
}
