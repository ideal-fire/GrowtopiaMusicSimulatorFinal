// File with code for loading all the different file formats for Growtopia Music Simulator songs
// The file format constants are in order of the programs' release dates.
// And yes, although the forum thread came later, Growtopia Music Simulator Online came before Growtopia Music Player.

// File format not found
#define FILE_FORMAT_UNKNOWN 0

// File format of Growtopia Music Simulator Classic/Java/Original
// File extension is .mylegguy
// https://github.com/MyLegGuy/GrowtopiaMusicSimulatorLegacy/blob/master/GrowtopiaMusicSimulatorClassic/src/nathan/MainClass.java#L1490
#define FILE_FORMAT_GMSCLASSIC 1
void loadGMSClassicSong(FILE* fp){
	fseek(fp,3,SEEK_SET); // maxX
	int y;
	for (y = 0; y < 14; ++y) {
		int x;
		for (x = 0; x < 400; ++x) {
			int _readId = fgetc(fp);
			_readId-=48; // Convert from ASCII
			_placeNoteLow(x,y,_readId,0,songArray);
		}
	}
}

// File format of original Growtopia Music Simulator Android
// This is just here for remembrance, nobody will be able to get this file type without rooting their device, so there's no point.
#define FILE_FORMAT_GMSANDROID 2
void loadGMSASong(FILE* fp){
	printf("Maybe if I'm board enough. Maybe I could add a feature to Growtopia Music Simulator Android where people can upload their songs to pastebin!");
	printf("Actually, another idea is to make it so Growtopia Music Simulator Final has the same App ID, then I can just open the user's old songs.");
}

// File format of Growtopia Music Simulator Reborn
// Will usually have the .AngryLegGuy extension
#define FILE_FORMAT_GMSR 3
//https://pastebin.com/raw/wPDeq5JM
void loadGMSrSong(FILE* fp){
	// Don't look at this code, please, I beg you. I wrote it once, it's terrible, but I don't want to write it again.
	const u8 rebornAudioGearId=15;
	
	char mapversion;
	fread(&mapversion,1,1,fp);
	
	if (mapversion >= 4) { // Seek past magic
		fseek(fp,4,SEEK_CUR);
	}else if (mapversion == 3) { // Seek past dummy bytes
		fseek(fp,3,SEEK_CUR);
	}
	
	s16 _readBPM;
	fread(&_readBPM,2,1,fp);
	_readBPM = fixShort(_readBPM);
	bpm = _readBPM;
	
	s16 _readMapWidth=400;
	s16 _readMapHeight=14;
	if (mapversion <= 4) {
		fseek(fp,2,SEEK_CUR); // Seek past useless dummy u8 values for map width and height
	} else {
		// Version 5 is when map resizing was introduced, actually load the map width.
		fread(&_readMapWidth,2,1,fp);
		_readMapWidth = fixShort(_readMapWidth);
	}
	fseek(fp,1,SEEK_CUR); // Seek past useles byte for number of layers
	
	u8 past = 255;
	u8 present = 254;
	u8 rollValue = 55;
	u8 rolling = 0;
	u8 rollAmount = 0;
	
	setSongWidth(songArray,songWidth,_readMapWidth);
	songWidth=_readMapWidth;
	printf("read width is %d\n",_readMapWidth);
	//Debug.Print(mapversion.ToString()+";"+mapWidth.ToString()+";"+mapHeight.ToString()+".");
	int x, y;
	for (y = 0; y < _readMapHeight; y++) {
		for (x = 0; x < _readMapWidth; x++) {
			if (!rolling) {
				if (past == present) {
					// Checked here for good reasons.
					rolling = 1;
					rollValue = present;
					fread(&rollAmount,1,1,fp);
					//Debug.Print("Starting roll with value: "+rollValue.ToString()+" and amount: "+rollAmount.ToString()+".");
					if (rollAmount <= 0) {
						//Debug.Print ("Ending roll...");
						past = 255;
						present = 244;
						rolling = 0;
						u8 _readByte = fgetc(fp);
						_placeNoteLow(x,y,_readByte,0,songArray);
						if (songArray[y][x].id==rebornAudioGearId){
							fread(songArray[y][x].extraData,2,5,fp); // Read 10 bytes directly into extraData
							past=255;
							present=254;
							continue;
						}
						past = present;
						present = songArray[y][x].id;
						//Debug.Print ("Wrote: " + workMap [trueX, trueY].ToString () + " and present and past is: " + present.ToString () + " ; " + past.ToString () + ".");
						continue;
					}
					_placeNoteLow(x,y,rollValue,0,songArray);
					rollAmount--;
					continue;
				}else{
					u8 _readByte = fgetc(fp);
					_placeNoteLow(x,y,_readByte,0,songArray);
					if (songArray[y][x].id==rebornAudioGearId){
						fread(songArray[y][x].extraData,2,5,fp); // Read 10 bytes directly into extraData
						past=255;
						present=254;
						continue;
					}
					past = present;
					present = songArray[y][x].id;
				}
				//Debug.Print ("Wrote: " + workMap [trueX, trueY].ToString () + " and present and past is: " + present.ToString () + " ; " + past.ToString () + ".");
			} else {
				if (rollAmount <= 0) {
					//Debug.Print ("Ending roll...");
					past = 255;
					present = 244;
					rolling = 0;
					u8 _readByte = fgetc(fp);
					_placeNoteLow(x,y,_readByte,0,songArray);
					if (songArray[y][x].id==rebornAudioGearId){
						fread(songArray[y][x].extraData,2,5,fp); // Read 10 bytes directly into extraData
						past=255;
						present=254;
						continue;
					}
					past = present;
					present = songArray[y][x].id;
					//Debug.Print ("Wrote: " + workMap [trueX, trueY].ToString () + " and present and past is: " + present.ToString () + " ; " + past.ToString () + ".");
					continue;
				}else{
					_placeNoteLow(x,y,rollValue,0,songArray);
					rollAmount--;
				}
			}
		}
	}
}

// File format of Growtopia Music Simulator Online
// File extension is .AngryLegGuy by default, but it usually won't have a file extension.
#define FILE_FORMAT_GMSO 4
void loadGMSOSong(FILE* fp){
	//https://github.com/MyLegGuy/GrowtopiaMusicSimulatorOnline/blob/master/happy.js#L419

	fseek(fp,4,SEEK_SET); // Magic string "GMSO"
	fseek(fp,1,SEEK_CUR); // Growtopia Music Simulator Online was never updated, it stayed as ASCII version "1".
	// 3 digit number that is the BPM
	char _loadedBPMString[4];
	_loadedBPMString[3]='\0';
	fread(_loadedBPMString,3,1,fp);
	bpm = atoi(_loadedBPMString);

	// Now we're at the actual song data
	int i;
	for (i=0;i<400;++i){
		int j;
		for (j=0;j<14;++j){
			u8 _readId = fgetc(fp);
			_readId-=48; // Convert from ASCII
			_placeNoteLow(i,j,_readId,0,songArray);
		}
	}
}

// File format of cernodile's Growtopia Music Player.
// Will have the .gtmusic extension.
// Files are made from this tool: https://tools.cernodile.com/musicSim.php
#define FILE_FORMAT_GTMUSIC 5
void loadDumbGtmusicFormat(FILE* fp){
	int _maxReadLine = 4*14+1; // 14 notes, 3 chars per note, 1 comma per note
	char _placeholderChar;
	char _readLine[_maxReadLine];
	if (fgets(_readLine,_maxReadLine,fp)==NULL){
		fprintf(stderr,"failed to read first line form file?\n");
		return;
	}
	removeNewline(_readLine);
	if (strcmp(_readLine,"%gsmusicsim;")==0){
		_placeholderChar=';';
	}else{
		_placeholderChar=',';
		if (strcmp(_readLine,"%cernmusicsim;")!=0){
			printf("assuming cernmusicsim even though idk what it is: \"%s\"\n",_readLine);
		}
	}
	fgets(_readLine,_maxReadLine,fp); // Second line is bpm=x
	removeNewline(_readLine);
	bpm = atoi(&(_readLine[strlen("bpm=")]));
	// The file has space for up to 400 columns, but last I checked you only can use 100 columns.
	int i;
	for (i=0;i<400;++i){
		fgets(_readLine,_maxReadLine,fp);
		removeNewline(_readLine);
		int _stringReadPosition=0;
		int _len=strlen(_readLine);
		// Notes are listed from bottom to top
		// All lines start with a comma
		// Last note (high B) does not end with a comma
		while (1){
			// All lines start with a comma
			_stringReadPosition++;
			if (_stringReadPosition>=_len-3){
				break;
			}
			if (_readLine[_stringReadPosition]!=_placeholderChar){ // If there's a note in this slot
				char _foundLetter;
				char _foundNoteName;
				char _foundAccidental;

				_foundLetter = _readLine[_stringReadPosition];
				_foundNoteName = _readLine[_stringReadPosition+1];
				_foundAccidental = _readLine[_stringReadPosition+2];

				// Find note ID using info we have
				char _foundActualNoteID=0;
				if (_foundLetter=='L'){ // Blank
					printf("Cannot load blank notes from stupid format\n");
				}else if (_foundLetter=='r'){ // Repeat start
					_foundActualNoteID = repeatStartID;
				}else if (_foundLetter=='R'){ // Repeat end
					_foundActualNoteID = repeatEndID;
				}else if (_foundLetter=='H'){ // Spooky
					printf("Cannot load spooky notes from stupid format\n");
				}else{
					int k;
					// Find the note's ID based on the letter and accidental
					for (k=0;k<totalNotes;++k){
						if (extraNoteInfo[k].letter==_foundLetter && extraNoteInfo[k].accidental==_foundAccidental){
							_foundActualNoteID=k;
							break;
						}
					}
				}

				// If we found the note, use more info to place it
				if (_foundActualNoteID!=0){
					// Find the position that goes with that note name
					int l;
					for (l=0;l<sizeof(noteNames);++l){
						if (_foundNoteName==noteNames[l]){
							_placeNoteLow(i,l,_foundActualNoteID,0,songArray);
							break;
						}
					}
				}else{
					printf("Note not found, %c%c%c\n%s\n",_foundLetter,_foundNoteName,_foundAccidental,_readLine);
				}
				// We only read two more bytes than we normally would because the first letter is the comma we always read
				_stringReadPosition+=2;
			}
		}
	}
	findMaxX();
	// More authentic
	if (maxX<=100){
		setSongWidth(songArray,songWidth,100);
		songWidth=100;
	}
}

// File format of Growtopia Music Simulator Final
#define FILE_FORMAT_GMSF 6
void loadGMSFSong(FILE* fp){
	fseek(fp,strlen(SAVEFORMATMAGIC),SEEK_SET); // Seek past magic string
	
	u8 _readVersion;
	u8 _readAudioGearID;
	s16 _readWidth;
	s16 _readHeight;
	s16 _readBPM;

	// General info
	fread(&_readVersion,1,1,fp);
	fread(&_readAudioGearID,1,1,fp);

	fread(&_readBPM,2,1,fp);
	fread(&_readWidth,2,1,fp);
	fread(&_readHeight,2,1,fp);
	
	// Process general info
	_readBPM = fixShort(_readBPM);
	_readWidth = fixShort(_readWidth);
	_readHeight = fixShort(_readHeight);
	bpm = _readBPM;
	setSongWidth(songArray,songWidth,_readWidth);
	songWidth = _readWidth;
	if (audioGearID!=_readAudioGearID){
		printf("Audio gears will not work. %d;%d\n",audioGearID,_readAudioGearID);
	}

	// Bulk of the data
	int _y;
	for (_y=0;_y<_readHeight;++_y){
		int _x;
		for (_x=0;_x<_readWidth;++_x){
			u8 _lastReadId;
			fread(&(_lastReadId),1,1,fp);
			_placeNoteLow(_x,_y,_lastReadId,0,songArray);
			if (songArray[_y][_x].id==_readAudioGearID){
				fread(songArray[_y][_x].extraData,AUDIOGEARSPACE*2+1,1,fp); // usually 10 bytes + the volume byte for 11 in total
			}
		}
	}
	// Load metadata
	// But check for metadata marker first
	char _possibleEndMagic[5];
	_possibleEndMagic[4]='\0';
	fread(_possibleEndMagic,4,1,fp);
	if (strcmp(_possibleEndMagic,METADATAMARKER)!=0){
		printf("Bad metadata marker.");
		char i;
		for (i=0;i<4;++i){
			printf("%d\n",_possibleEndMagic[i]);
		}
		return;
	}
	// Now load it
	u8 _readLength;
	fread(&_readLength,1,1,fp);
	if (_readLength!=0){
		free(currentSongMetadata);
		currentSongMetadata=malloc(_readLength+1);
		fread(currentSongMetadata,_readLength,1,fp);
		currentSongMetadata[_readLength]='\0';
	}

	// Check if we got to the end of the file okay
	_possibleEndMagic[4]='\0';
	fread(_possibleEndMagic,4,1,fp);
	if (strcmp(_possibleEndMagic,SAVEENDMARKER)!=0){
		printf("Bad end of file magic.\n");
		char i;
		for (i=0;i<4;++i){
			printf("%d\n",_possibleEndMagic[i]);
		}
	}
}

// 0 - ok
// 1 - unknown format
// 2 - file not found
char loadSong(char* _passedFilename){
	FILE* fp = fopen(_passedFilename,"rb");
	if (fp!=NULL){
		u8 _detectedFormat = FILE_FORMAT_UNKNOWN;

		// The first byte is very important for checking the file format
		u8 _firstByte = fgetc(fp);

		// TODO - Add detection for GMSF file format here.

		// GMSO is the first 4 bytes of a Growtopia Music Simulator Online file
		// GMSF is the first 4 bytes of a Growtopia Music Simulator Final file
		if (_firstByte=='G'){
			char _fourMagic[5];
			_fourMagic[4]='\0';
			_fourMagic[0]=_firstByte;
			fread(&(_fourMagic[1]),1,3,fp);
			if (strcmp(_fourMagic,"GMSO")==0){
				_detectedFormat=FILE_FORMAT_GMSO;
			}else if (strcmp(_fourMagic,"GMSF")==0){
				_detectedFormat=FILE_FORMAT_GMSF;
			}else{
				fseek(fp,1,SEEK_SET); // Undo the extra bytes we read otherwise
			}
		}

		// If it's not a GMSO or GMSF file...
		if (_detectedFormat==FILE_FORMAT_UNKNOWN){
			// Check for Growtopia Music Simulator Reborn file format.
			// The first thing in Growtopia Music Simulator Reborn format is the version number. It started at 3 and ended at 6.
			if (_firstByte>=3 && _firstByte<=6){
				// Versions 4 and up have the magic numbers after.
				if (_firstByte>=4){
					char _fourMagic[5];
					_fourMagic[4]='\0';
					fread(_fourMagic,1,4,fp);
	
					if (strcmp(_fourMagic,"GMSr")==0){
						_detectedFormat = FILE_FORMAT_GMSR;
					}else{
						fseek(fp,1,SEEK_SET); // Undo the extra bytes we read otherwise
					}
				}else if (_firstByte == 3) {// Looking at the Growtopia Music Simulator Reborn source code, it seems the first version number was 3. I don't know why, but it could be because of my map library I made before Growtopia Music Simulator Reborn. It could also be that Growtopia Music Simulator Reborn is the third Growtopia Music Simulator I made.
					// The two bytes after the version number were the BPM, we don't worry about that for now.
					fseek(fp,2,SEEK_CUR);

					// Not in ASCII, the values 1 2 3 are written starting after the one byte version number.
					// The reason behind this was that my map library I made that I was using for this saved the width, height, and number of layers in one byte each. These were not needed for Growtopia Music Simulator because they (were) always the same. The format was obviously changed later to allow song resizing.
					u8 _tripleMagic[3];

					fread(_tripleMagic,1,3,fp);
					if (_tripleMagic[0]==1 && _tripleMagic[1]==2 && _tripleMagic[2]==3){
						_detectedFormat = FILE_FORMAT_GMSR;
					}else{
						fseek(fp,1,SEEK_SET); // Undo the extra bytes we read otherwise
					}
				}
			}

			// If it's not a GMSR file....
			if (_detectedFormat==FILE_FORMAT_UNKNOWN){
				// Detect .gtmusic file extension
				if (strlen(_passedFilename)>=8 && strcmp(&(_passedFilename[strlen(_passedFilename)-8]),".gtmusic")==0){
					_detectedFormat = FILE_FORMAT_GTMUSIC;
				}
				// If it's not a GTMUSIC file...
				if (_detectedFormat==FILE_FORMAT_UNKNOWN){
					// Detect .mylegguy file extension
					if (strlen(_passedFilename)>=8 && strcmp(&(_passedFilename[strlen(_passedFilename)-9]),".mylegguy")==0){
						_detectedFormat = FILE_FORMAT_GMSCLASSIC;
					}
				}
			}
		}

		// Reset any seeking, give our load functions a clean state.
		fseek(fp,0,SEEK_SET);
		// Done detecting, load the file now.
		if (_detectedFormat==FILE_FORMAT_UNKNOWN){
			fclose(fp);
			return 1;
		}else{
			// Because we're loading we won't need the old song data
			clearSong();
			// Load depending on format
			switch(_detectedFormat){
				case FILE_FORMAT_GMSCLASSIC:
					loadGMSClassicSong(fp);
					break;
				case FILE_FORMAT_GMSANDROID:
					loadGMSASong(fp);
					break;
				case FILE_FORMAT_GMSR:
					loadGMSrSong(fp);
					break;
				case FILE_FORMAT_GMSO:
					loadGMSOSong(fp);
					break;
				case FILE_FORMAT_GTMUSIC:
					loadDumbGtmusicFormat(fp);
					break;
				case FILE_FORMAT_GMSF:
					loadGMSFSong(fp);
					break;
			}
			// New maxX because new song data
			findMaxX();
			setSongXOffset(0); // Reloads display for song width too.
		}
		fclose(fp);
		return 0;
	}else{
		return 2;
	}
}
