/*
//https://www.lua.org/manual/5.3/manual.html#lua_pushlightuserdata

todo - Add volume setting. Morons want it and don't know how to use volume mixer

add 4px terminal padding
Shoujo☆Kageki_Revue_Starlight
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <GeneralGood/GeneralGoodConfig.h>
#include <GeneralGood/GeneralGood.h>
#include <GeneralGood/GeneralGoodExtended.h>
#include <GeneralGood/GeneralGoodGraphics.h>
#include <GeneralGood/GeneralGoodSound.h>
#include <GeneralGood/GeneralGoodText.h>
#include <GeneralGood/GeneralGoodImages.h>

#include <Lua/lua.h>
#include <Lua/lualib.h>
#include <Lua/lauxlib.h>

#define ISTESTINGMOBILE 0
#define DISABLESOUND 0

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define s8 int8_t
#define s16 int16_t
#define s32 int32_t

#define BGMODE_SINGLE 1
#define BGMODE_PART 2

#define LUAREGISTER(x,y) lua_pushcfunction(L,x);\
	lua_setglobal(L,y);

////////////////////////////////////////////////
typedef void(*voidFunc)();
typedef struct{
	CrossTexture* image;
	voidFunc activateFunc;
}uiElement;
////////////////////////////////////////////////
u8 optionPlayOnPlace=1;
////////////////////////////////////////////////
int screenWidth;
int screenHeight;
int logicalScreenWidth;
int logicalScreenHeight;
int globalDrawXOffset;
int globalDrawYOffset;

lua_State* L = NULL;

u16 totalUI=0;
uiElement* myUIBar;

u16 songWidth=400;
u16 songHeight=14;

double generalScale;

u16 singleBlockSize=32;

u8 isMobile;

u16 gottenNoteIconIndex=0;

s32 selectedNote=0;
u16 totalNotes=0;

// Only the width of a page can change and it depends on the scale set by the user
u16 pageWidth=25;
#define pageHeight 14

// How much of the page's height you can see without scrolling
u16 visiblePageHeight;
// These are display offsets
u16 songYOffset=0;
u16 songXOffset=0;

// 0 for normal PC mode
// 1 for split mobile mode
u8 backgroundMode=BGMODE_SINGLE;
CrossTexture* bigBackground=NULL;
CrossTexture** bgPartsEmpty=NULL;
CrossTexture** bgPartsLabel=NULL;

CrossTexture** noteImages=NULL;
CROSSSFX*** noteSounds=NULL;

u8** songArray;

void updateNoteIcon(){
	myUIBar[gottenNoteIconIndex].image=noteImages[selectedNote];
}

void uiPlay(){

}

void uiNoteIcon(){
	selectedNote++;
	if (selectedNote==totalNotes){
		selectedNote=0;
	}
	updateNoteIcon();
}

int fixX(int _x){
	return _x+globalDrawXOffset;
}

int fixY(int _y){
	return _y+globalDrawYOffset;
}

uiElement* addUI(){
	++totalUI;
	myUIBar = realloc(myUIBar,sizeof(myUIBar)*totalUI);
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
void setSongWidth(u8** _passedArray, u16 _passedOldWidth, u16 _passedWidth){
	int i;
	for (i=0;i<songHeight;++i){
		_passedArray[i] = recalloc(_passedArray[i],_passedOldWidth,_passedWidth);
	}
}

void drawImageScaleAlt(CrossTexture* _passedTexture, int _x, int _y, double _passedXScale, double _passedYScale){
	if (_passedTexture!=NULL){
		drawTextureScale(_passedTexture,_x,_y,_passedXScale,_passedYScale);
	}
}

CrossTexture* loadEmbeddedPNG(const char* _passedFilename){
	char* _fixedPathBuffer = malloc(strlen(_passedFilename)+strlen(getFixPathString(TYPE_EMBEDDED)+1));
	fixPath((char*)_passedFilename,_fixedPathBuffer,TYPE_EMBEDDED);
	CrossTexture* _loadedTexture = loadPNG(_fixedPathBuffer);
	free(_fixedPathBuffer);
	return _loadedTexture;
}

// Return a path that may or may not be fixed to TYPE_EMBEDDED
char* possiblyFixPath(const char* _passedFilename, char _shouldFix){
	if (_shouldFix){
		char* _fixedPathBuffer = malloc(strlen(_passedFilename)+strlen(getFixPathString(TYPE_EMBEDDED)+1));
		fixPath((char*)_passedFilename,_fixedPathBuffer,TYPE_EMBEDDED);
		return _fixedPathBuffer;
	}else{
		return strdup(_passedFilename);
	}
}

void XOutFunction(){
	exit(0);
}

void setTotalNotes(u16 _newTotal){
	if (_newTotal<=totalNotes){
		return;
	}
	noteImages = realloc(noteImages,sizeof(CrossTexture*)*_newTotal);
	noteSounds = realloc(noteSounds,sizeof(CROSSSFX**)*_newTotal);

	int i;
	for (i=totalNotes;i<_newTotal;++i){
		noteSounds[i] = malloc(songHeight*sizeof(CROSSSFX*));
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
int L_addNote(lua_State* passedState){
	int _passedSlot = lua_tonumber(passedState,1);
	// Don't need to check if we're actually adding
	setTotalNotes(_passedSlot+1);

	noteImages[_passedSlot] = lua_touserdata(passedState,2);
	
	// Load array data
	int i;
	for (i=0;i<songHeight;++i){
		lua_rawgeti(passedState,3,i+1);
		noteSounds[_passedSlot][i] = lua_touserdata(passedState,-1);
		lua_pop(passedState,1);
	}

	return 0;
}

void pushLuaFunctions(){
	LUAREGISTER(L_addNote,"addNote");
	LUAREGISTER(L_setBigBg,"setBigBg");
	LUAREGISTER(L_setBgParts,"setBgParts");
	LUAREGISTER(L_getMobile,"isMobile");
	LUAREGISTER(L_loadSound,"loadSound");
	LUAREGISTER(L_loadImage,"loadImage");
}

void die(const char* message) {
  printf("%s\n", message);
  exit(EXIT_FAILURE);
}

void goodLuaDofile(lua_State* passedState, char* _passedFilename){
	if (luaL_dofile(passedState, _passedFilename) != LUA_OK) {
		die(lua_tostring(L,-1));
	}
}

void goodPlaySound(CROSSSFX* _passedSound){
	#if !DISABLESOUND
		playSound(_passedSound,1,0);
	#endif
}

void placeNote(int _x, int _y, u16 _noteId){
	if (songArray[_y][_x]!=_noteId && noteSounds[_noteId][_y]!=NULL && optionPlayOnPlace){
		goodPlaySound(noteSounds[_noteId][_y]);
	}
	songArray[_y][_x]=_noteId;
}

void init(){
	initGraphics(832,480,&screenWidth,&screenHeight);
	initAudio();
	setClearColor(192,192,192,255);
	if (screenWidth!=832 || screenHeight!=480){
		isMobile=1;
	}else{
		isMobile=ISTESTINGMOBILE;
	}
	if (isMobile){
		// An odd value for testing.
		generalScale=1.3;
	}else{
		generalScale=1;
	}
	if (generalScale!=0){
		singleBlockSize = 32*generalScale;
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

	L = luaL_newstate();
	luaL_openlibs(L);
	pushLuaFunctions();

	songArray = calloc(1,sizeof(u8*)*songHeight);
	setSongWidth(songArray,0,400);
	songWidth=400;

	// Init note array before we do the Lua
	setTotalNotes(1);
	setTotalNotes(5);
	noteImages[0] = loadEmbeddedPNG("assets/Free/Images/grid.png");


	// Set up UI
	uiElement* _newButton = addUI();
	_newButton->image = NULL;
	_newButton->activateFunc = uiNoteIcon;
	gottenNoteIconIndex = totalUI-1;

	_newButton = addUI();
	_newButton->image = loadEmbeddedPNG("assets/Free/Images/playButton.png");
	_newButton->activateFunc = uiPlay;

	// Very last, run the init script
	fixPath("assets/Free/Scripts/init.lua",tempPathFixBuffer,TYPE_EMBEDDED);
	goodLuaDofile(L,tempPathFixBuffer);

	// Hopefully we've added some note images in the init.lua.
	updateNoteIcon();
}



int main(int argc, char *argv[]){
	selectedNote=1;
	init();
	
	while(1){
		int i;
		controlsStart();
		if (isDown(SCE_TOUCH)){
			// We need to use floor so that negatives are not made positive
			int _placeX = floor((touchX-globalDrawXOffset)/singleBlockSize);
			int _placeY = floor((touchY-globalDrawYOffset)/singleBlockSize);
			if (_placeY==visiblePageHeight){ // If we click the UI section
				if (wasJustPressed(SCE_TOUCH)){
					for (i=0;i<totalUI;++i){
						if (_placeX==i){
							myUIBar[i].activateFunc();
						}
					}
				}
			}else{ // If we click the main section
				if (!(_placeX>=pageWidth || _placeX<0)){ // In bounds in the main section, I mean.
					placeNote(_placeX+songXOffset,_placeY+songYOffset,selectedNote);
				}
			}
		}
		if (wasJustPressed(SCE_MOUSE_SCROLL)){
			if (mouseScroll<0){
				selectedNote--;
				if (selectedNote<0){
					selectedNote=totalNotes-1;
				}
				updateNoteIcon();
			}else if (mouseScroll>0){
				selectedNote++;
				if (selectedNote==totalNotes){
					selectedNote=0;
				}
				updateNoteIcon();
			}// 0 scroll is ignored
		}
		controlsEnd();

		startDrawing();
		if (backgroundMode==BGMODE_SINGLE){
			drawImageScaleAlt(bigBackground,0,0,generalScale,generalScale);
		}
		for (i=0;i<visiblePageHeight;++i){
			int j;
			for (j=0;j<pageWidth;++j){
				if (songArray[i][j]==0){
					// If we're doing the special part display mode
					if (backgroundMode==BGMODE_PART){
						drawImageScaleAlt(bgPartsEmpty[i],j*singleBlockSize,i*singleBlockSize,generalScale,generalScale);
					}
				}else{
					drawImageScaleAlt(noteImages[songArray[i][j]],j*singleBlockSize,i*singleBlockSize,generalScale,generalScale);
				}
			}
			if (backgroundMode==BGMODE_PART){
				drawImageScaleAlt(bgPartsLabel[i],pageWidth*singleBlockSize,i*singleBlockSize,generalScale,generalScale);
			}
		}


		for (i=0;i<totalUI;++i){
			drawImageScaleAlt(myUIBar[i].image,i*singleBlockSize,visiblePageHeight*singleBlockSize,generalScale,generalScale);
		}
		endDrawing();
	}

	/* code */
	return 0;
}
