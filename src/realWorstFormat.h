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
					fread(&(songArray[y][x].id),1,1,fp);
					if (songArray[y][x].id==rebornAudioGearId){
						printf("TODO\n");
							fseek(fp,10,SEEK_CUR); // TEMP
						//LoadAudioGearInFile(ref file, ref workMap, x, y, tmf);
						//past=255;
						//present=254;
						continue;
					}
					past = present;
					present = songArray[y][x].id;
					//Debug.Print ("Wrote: " + workMap [trueX, trueY].ToString () + " and present and past is: " + present.ToString () + " ; " + past.ToString () + ".");
					continue;
				}
				songArray[y][x].id = rollValue;
				rollAmount--;
				continue;
			}else{
				fread(&(songArray[y][x].id),1,1,fp);
				if (songArray[y][x].id==rebornAudioGearId){
					printf("TODO\n");
							fseek(fp,10,SEEK_CUR); // TEMP
					//LoadAudioGearInFile(ref file, ref workMap, x, y, tmf);
					//past=255;
					//present=254;
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
				fread(&(songArray[y][x].id),1,1,fp);
				if (songArray[y][x].id==rebornAudioGearId){
					printf("TODO\n");
							fseek(fp,10,SEEK_CUR); // TEMP
					//LoadAudioGearInFile(ref file, ref workMap, x, y, tmf);
					//past=255;
					//present=254;
					continue;
				}
				past = present;
				present = songArray[y][x].id;
				//Debug.Print ("Wrote: " + workMap [trueX, trueY].ToString () + " and present and past is: " + present.ToString () + " ; " + past.ToString () + ".");
				continue;
			}else{
				songArray[y][x].id = rollValue;
				rollAmount--;
			}

		}
	}
}