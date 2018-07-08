function sortLoadedSounds(passedLoadedSounds)
	noteSounds = {};
	noteSharpSounds = {};
	noteFlatSounds = {};

	noteSounds[1]=passedLoadedSounds[24+1];
	noteSounds[2]=passedLoadedSounds[22+1];
	noteSounds[3]=passedLoadedSounds[20+1];
	noteSounds[4]=passedLoadedSounds[18+1];
	noteSounds[5]=passedLoadedSounds[17+1];
	noteSounds[6]=passedLoadedSounds[15+1];
	noteSounds[7]=passedLoadedSounds[13+1];
	noteSounds[8]=passedLoadedSounds[12+1];
	noteSounds[9]=passedLoadedSounds[10+1];
	noteSounds[10]=passedLoadedSounds[8+1];
	noteSounds[11]=passedLoadedSounds[6+1];
	noteSounds[12]=passedLoadedSounds[5+1];
	noteSounds[13]=passedLoadedSounds[3+1];
	noteSounds[14]=passedLoadedSounds[1+1];
	
	noteFlatSounds[1]=passedLoadedSounds[23+1];
	noteFlatSounds[2]=passedLoadedSounds[21+1];
	noteFlatSounds[3]=passedLoadedSounds[19+1];
	noteFlatSounds[4]=passedLoadedSounds[17+1];
	noteFlatSounds[5]=passedLoadedSounds[16+1];
	noteFlatSounds[6]=passedLoadedSounds[14+1];
	noteFlatSounds[7]=passedLoadedSounds[12+1];
	noteFlatSounds[8]=passedLoadedSounds[11+1];
	noteFlatSounds[9]=passedLoadedSounds[9+1];
	noteFlatSounds[10]=passedLoadedSounds[7+1];
	noteFlatSounds[11]=passedLoadedSounds[5+1];
	noteFlatSounds[12]=passedLoadedSounds[4+1];
	noteFlatSounds[13]=passedLoadedSounds[2+1];
	noteFlatSounds[14]=passedLoadedSounds[0+1];
	
	noteSharpSounds[1]=passedLoadedSounds[25+1];
	noteSharpSounds[2]=noteFlatSounds[1];
	noteSharpSounds[3]=noteFlatSounds[2];
	noteSharpSounds[4]=noteFlatSounds[3];
	noteSharpSounds[5]=noteSounds[4];
	noteSharpSounds[6]=noteFlatSounds[5];
	noteSharpSounds[7]=noteFlatSounds[6];
	noteSharpSounds[8]=noteSounds[7];
	noteSharpSounds[9]=noteFlatSounds[8];
	noteSharpSounds[10]=noteFlatSounds[9];
	noteSharpSounds[11]=noteFlatSounds[10];
	noteSharpSounds[12]=noteSounds[11];
	noteSharpSounds[13]=noteFlatSounds[12];
	noteSharpSounds[14]=noteFlatSounds[13];

	return noteSounds, noteSharpSounds, noteFlatSounds;
end

-- Given a format string that has the numbers 0 through 25 passed to it, load all the sound effects into an array. 
function loadGeneralSounds(_filenameformatString)
	local _returnLoadedTable = {};
	for i=0,25 do
		_returnLoadedTable[i+1] = loadSound(string.format(_filenameformatString,i));
	end
	return _returnLoadedTable;
end


-- Step 1 - Set up song board UI
if (isMobile()) then
	myBGPartTableEmpty = {};
	myBGPartyTableLabel = {};

	-- Load the images for empty slots and filled slots. There are 7 unique images.
	for i=0,6 do
		-- Do empty spot images first
		myBGPartTableEmpty[i+1] = loadImage(string.format("assets/Free/Images/PartBG/empty%d.png",i));
		-- Octave loop
		myBGPartTableEmpty[i+8] = myBGPartTableEmpty[i+1];

		-- Do note labels next. Fun fact, this note label feature was coincidentally used in somebody else's Growtopia Music Simulator....except he put the labels on the left side. What? I'm not implying anything.
		myBGPartyTableLabel[i+1] = loadImage(string.format("assets/Free/Images/PartBG/label%d.png",i));
		myBGPartyTableLabel[i+8] = myBGPartyTableLabel[i+1];
	end

	-- Actually send the data to the program. Calling this change the background mode.
	setBgParts(myBGPartTableEmpty,myBGPartyTableLabel);
	
	-- Free memory
	myBGPartTableEmpty = nil;
	myBGPartyTableLabel = nil;
else
	-- Calling this changes the background mode.
	setBigBg("assets/Free/Images/pcBackground.png");
end

-- Add notes
-- Reuse these three arrays. Make all three then add all three at once and repeat.
myNoteArray = {};
myNoteFlatArray = {};
myNoteFlatArray = {};
myLoadedSoundsArray = {};

-- Add piano notes
myLoadedSoundsArray = loadGeneralSounds("assets/Proprietary/Sound/piano_%d.wav");
myNoteArray, myNoteSharpArray, myNoteFlatArray = sortLoadedSounds(myLoadedSoundsArray);
addNote(1,loadImage("assets/Proprietary/Images/piano.png"),myNoteArray);
addNote(2,loadImage("assets/Proprietary/Images/pianoSharp.png"),myNoteSharpArray);
addNote(3,loadImage("assets/Proprietary/Images/pianoFlat.png"),myNoteFlatArray);

-- Add bass notes
myLoadedSoundsArray = loadGeneralSounds("assets/Proprietary/Sound/bass_%d.wav");
myNoteArray, myNoteSharpArray, myNoteFlatArray = sortLoadedSounds(myLoadedSoundsArray);
addNote(4,loadImage("assets/Proprietary/Images/bass.png"),myNoteArray);
addNote(5,loadImage("assets/Proprietary/Images/bassSharp.png"),myNoteSharpArray);
addNote(6,loadImage("assets/Proprietary/Images/bassFlat.png"),myNoteFlatArray);
