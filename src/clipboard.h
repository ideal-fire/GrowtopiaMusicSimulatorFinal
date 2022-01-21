#define MINCLIPASCII 0x21
#define MAXCLIPASCII 0x7E
#define MAXCLIPNUM ((MAXCLIPASCII-MINCLIPASCII+1))
char* makeclipbuff(noteSpot** sarr, int startx, int starty, int endx, int endy);
char* insertclipbuff(char* clipbuff, noteSpot** sarr, int startx, int starty, int arrw, int arrh);
