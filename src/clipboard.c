#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "clipboard.h"

#define CLIPBOARDPREFIX "GMSF-CLIPBOARD"

static unsigned char numToClip(int in, int baseval){
	return (in-baseval)+MINCLIPASCII;
}
static unsigned char clipToNum(unsigned char in, int baseval){
	return (in-MINCLIPASCII)+baseval;
}

char* makeclipbuff(noteSpot** sarr, int startx, int starty, int endx, int endy){
	char* strbuff=malloc(strlen(CLIPBOARDPREFIX)+1+(endx-startx+1)*(endy-starty+1)*gearBuffSize());
	if (!strbuff){
		return NULL;
	}
	// make clipboard contents
	char* next=strbuff;
	strcpy(next,CLIPBOARDPREFIX);
	next+=strlen(CLIPBOARDPREFIX);
	*(next++)=numToClip(VERSIONNUMBER,0);
	//
	*(next++)=numToClip(endx-startx+1,0);
	// loop
	for (int j=starty;j<=endy;++j){
		for (int i=startx;i<=endx;++i){
			*(next++)=numToClip(sarr[j][i].id,0);
			if (sarr[j][i].id==audioGearID){
				unsigned char* buff=sarr[j][i].extraData;
				for (int i=0;i<AUDIOGEARSPACE;++i){
					*(next++)=numToClip(*(buff++),0);
					*(next++)=numToClip(*(buff++),0);
				}
				// huh.
				int vol=*buff;
				*(next++)=numToClip(vol/10,0);
				*(next++)=numToClip(vol%10,0);
			}
		}
	}
	*(next)='\0';
	return strbuff;
}
char* insertclipbuff(char* clipbuff, noteSpot** sarr, int startx, int starty, int arrw, int arrh){
	if (strncmp(clipbuff,CLIPBOARDPREFIX,strlen(CLIPBOARDPREFIX))!=0){
		return "Not growtopia music simulator final clipboard data";
	}
	char* ret=NULL;
	char* next=clipbuff;
	next+=strlen(CLIPBOARDPREFIX);
	next++;
	int roww=clipToNum(*(next++),0);
	int endx=startx+roww;
	int lenleft=strlen(next);
	int gearreqlen=AUDIOGEARSPACE*2+2;
	for (int y=starty;;++y){
		if (y>=arrh){
			break;
		}
		for (int x=startx;x<endx;++x){
			if(lenleft==0){
				goto gotoret;
			}
			int id=clipToNum(*(next++),0);
			lenleft--;
			if (x>=arrw){
				if (id==audioGearID){
					lenleft-=gearreqlen;
					next+=gearreqlen;
				}
			}else{
				freeNote(&(sarr[y][x]));
				sarr[y][x].id=id;
				if (sarr[y][x].id==audioGearID){
					if (lenleft<gearreqlen){
						ret="Early EOF with audio gear data";
						goto gotoret;
					}
					lenleft-=gearreqlen;
					sarr[y][x].extraData=calloc(1,gearBuffSize());
					for (int i=0;i<AUDIOGEARSPACE;++i){
						unsigned char noteid=clipToNum(*(next++),0);
						unsigned char ypos=clipToNum(*(next++),0);
						if (noteid>=totalNotes || ypos>=arrh){
							ret="Corrupted audio gear note data";
							goto gotoret;
						}
						sarr[y][x].extraData[i*2]=noteid;
						sarr[y][x].extraData[i*2+1]=ypos;
					}
					unsigned char volhigh=clipToNum(*(next++),0);
					unsigned char vollow=clipToNum(*(next++),0);
					int vol=volhigh*10+vollow;
					printf("%d;%d;%d\n",volhigh,vollow,vol);
					if (vol>100){
						vol=100;
						ret="invalid audio gear volume level";
					}
					sarr[y][x].extraData[gearBuffSize()-1]=vol;
					if (ret){
						goto gotoret;
					}
				}else{
					sarr[y][x].extraData=NULL;
				}
			}
		}
	}
gotoret:
	return ret;
}
