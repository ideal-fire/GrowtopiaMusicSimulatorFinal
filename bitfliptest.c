// Test code to flip every bit in a file. I wonder what it's for?
// This code was designed to be written fast, not run fast.
#include <stdio.h>
#include <stdlib.h>
char* readStringFile(char* _filename, int* _resultLength){
	char* _loadedStringBuffer;
	FILE* fp = fopen(_filename, "rb");
	// Get file size
	fseek(fp, 0, SEEK_END);
	long _foundFilesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	// Read file into memory
	_loadedStringBuffer = malloc(_foundFilesize);
	fread(_loadedStringBuffer, _foundFilesize, 1, fp);
	fclose(fp);
	*_resultLength = _foundFilesize;
	return _loadedStringBuffer;
}
int main(){
	int _totalFileLength;
	char* _readFile = readStringFile("a.txt",&_totalFileLength);

	FILE* fp = fopen("a.txt","wb");
	int i;
	for (i=0;i<_totalFileLength;++i){
		char _lastFlippedByte = _readFile[i];
		_lastFlippedByte = _lastFlippedByte^0x11111111;
		fwrite(&_lastFlippedByte,1,1,fp);
	}
	fclose(fp);
}