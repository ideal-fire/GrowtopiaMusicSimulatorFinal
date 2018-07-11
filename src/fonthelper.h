#include "font.h"
extern SDL_Renderer* mainWindowRenderer;
SDL_Texture** loadedFont=NULL; 

#define FIRSTFONTCHARACTER 33
#define LASTFONTCHARACTER 126

void set_pixel(SDL_Surface *surface, int x, int y, u32 pixel){
	u32 *target_pixel = (u32*)((u8*) surface->pixels + y * surface->pitch + x * sizeof *target_pixel);
	*target_pixel = pixel;
}

SDL_Texture* getFontCharacter(int _asciiValue){
	SDL_Surface* _tempLetterSurface;
	_tempLetterSurface = SDL_CreateRGBSurface(0, 8, 8, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

	int i, j;
	for (j=0;j<8;++j){
		for (i=0;i<8;++i){
			if (font[(8*8*_asciiValue)+i+8*j]!=0){
				set_pixel(_tempLetterSurface,i,j,0x000000ff);
			}else{
				set_pixel(_tempLetterSurface,i,j,0x00000000);
			}
		}
	}

	SDL_Texture* _returnTexture;
	_returnTexture = SDL_CreateTextureFromSurface( mainWindowRenderer, _tempLetterSurface );
	// Free memori
	SDL_FreeSurface(_tempLetterSurface);
	return _returnTexture;
}
// See wrapper in main.c
void _drawString(char* _myString, int x, int y, double _scale, int _characterWidth){
	int i;
	int _cachedStrlen = strlen(_myString);
	for (i=0;i<_cachedStrlen;++i){
		int _foundFontIndex = _myString[i]-FIRSTFONTCHARACTER;
		if (_foundFontIndex>0){
			drawTextureScale(loadedFont[_foundFontIndex],x+_characterWidth*i,y,_scale*2,_scale*2);
		}
	}
}

void initEmbeddedFont(){
	loadedFont = malloc(sizeof(SDL_Texture*)*(LASTFONTCHARACTER-FIRSTFONTCHARACTER));
	int i;
	for (i=0;i<(LASTFONTCHARACTER-FIRSTFONTCHARACTER);++i){
		loadedFont[i]=getFontCharacter(FIRSTFONTCHARACTER+i);
	}
}