local neutralAccidental='-';
local sharpAccidental='#';
local flatAccidental='b';


-- For a sequential table, get the number of things that are not -1.
function getNotNegativeOneLength(_passedTable)
	local _totalValid=0;
	for i=1,#_passedTable do
		if (_passedTable[i]~=-1) then
			_totalValid=_totalValid+1;
		end
	end
	return _totalValid;
end

function sortLoadedSounds(passedLoadedSounds)
	noteSounds = {};
	noteSharpSounds = {};
	noteFlatSounds = {};

	local _foundTableLength = getNotNegativeOneLength(passedLoadedSounds);

	-- If we have at least one full note of sounds
	if (_foundTableLength>=14) then
		-- For stuff like spooky and festive, we only load the first 14 notes
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
		
		-- For regular notes like piano and bass, we also load the flat and sharp
		if (_foundTableLength==26) then
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
		end
	elseif (_foundTableLength==7) then -- If we have half of a full note of sounds, like drums
		for i=1,7 do
			noteSounds[i] = passedLoadedSounds[i];
			noteSounds[i+7] = noteSounds[i];
		end
	else
		print("Bad length " .. #passedLoadedSounds);
	end

	return noteSounds, noteSharpSounds, noteFlatSounds;
end

function _loadFormattedSound(_filenameformatString, _formatInput)
	return loadSound(string.format(_filenameformatString,_formatInput))
end

-- Given a format string that has the numbers 0 through 25 passed to it, load all the sound effects into an array. 
function loadGeneralSounds(_filenameformatString, _totalNotes)
	if (_totalNotes==nil) then
		_totalNotes=25;
	end
	local _returnLoadedTable = {};
	for i=0,_totalNotes do
		_returnLoadedTable[i+1] = _loadFormattedSound(_filenameformatString,i);
	end
	return _returnLoadedTable;
end
function loadHalfSounds(_filenameformatString)
	return loadGeneralSounds(_filenameformatString,6);
end
function loadSingleNoteSounds(_filenameformatString)
	local _returnLoadedTable = {};

	-- Write dummy data to all the slots so that the table will be sequential so # count operator will work
	for i=1,25 do
		_returnLoadedTable[i]=-1;
	end

	_returnLoadedTable[24+1]= _loadFormattedSound(_filenameformatString,24);
	_returnLoadedTable[22+1]= _loadFormattedSound(_filenameformatString,22);
	_returnLoadedTable[20+1]= _loadFormattedSound(_filenameformatString,20);
	_returnLoadedTable[18+1]= _loadFormattedSound(_filenameformatString,18);
	_returnLoadedTable[17+1]= _loadFormattedSound(_filenameformatString,17);
	_returnLoadedTable[15+1]= _loadFormattedSound(_filenameformatString,15);
	_returnLoadedTable[13+1]= _loadFormattedSound(_filenameformatString,13);
	_returnLoadedTable[12+1]= _loadFormattedSound(_filenameformatString,12);
	_returnLoadedTable[10+1]= _loadFormattedSound(_filenameformatString,10);
	_returnLoadedTable[6+1]=_loadFormattedSound(_filenameformatString,6);
	_returnLoadedTable[5+1]=_loadFormattedSound(_filenameformatString,5);
	_returnLoadedTable[8+1]=_loadFormattedSound(_filenameformatString,8);
	_returnLoadedTable[3+1]=_loadFormattedSound(_filenameformatString,3);
	_returnLoadedTable[1+1]=_loadFormattedSound(_filenameformatString,1);

	return _returnLoadedTable;
end

function addTripleNotes(_soundFormat, _startId, _filenameNormal, _filenameSharp, _filenameFlat, _audioGearSymbol)
	myNoteArray = {};
	myNoteFlatArray = {};
	myNoteFlatArray = {};
	myLoadedSoundsArray = {};

	myLoadedSoundsArray = loadGeneralSounds(_soundFormat);
	myNoteArray, myNoteSharpArray, myNoteFlatArray = sortLoadedSounds(myLoadedSoundsArray);
	addNote(_startId,loadImage(_filenameNormal),myNoteArray);
	addNote(_startId+1,loadImage(_filenameSharp),myNoteSharpArray);
	addNote(_startId+2,loadImage(_filenameFlat),myNoteFlatArray);

	if (_audioGearSymbol~=nil) then
		setGearInfo(_startId,_audioGearSymbol,neutralAccidental);
		setGearInfo(_startId+1,_audioGearSymbol,sharpAccidental);
		setGearInfo(_startId+2,_audioGearSymbol,flatAccidental);
	end

	-- Change the order to flat, normal, sharp
	-- We had to add them in the wrong order to keep legacy Growtopia Music Simulator note IDs

										  -- normal sharp flat
	swapNoteUIOrder(_startId,_startId+1); -- sharp normal flat
	swapNoteUIOrder(_startId,_startId+2); -- flat normal sharp
end

function addSingleNote(_soundFormat, _startId, _filenameNormal)
	myNoteArray = {};

	myNoteArray = sortLoadedSounds(loadSingleNoteSounds(_soundFormat));
	addNote(_startId,loadImage(_filenameNormal),myNoteArray);
end

--/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

-- Step 1 - Set up song board UI
if (isMobile()) then
	myBGPartTableEmpty = {};
	myBGPartyTableLabel = {};

	-- Load the images for empty slots and filled slots. There are 7 unique images.
	for i=0,6 do
		myBGPartTableEmpty[i+1] = loadImage(string.format("assets/Free/Images/PartBG/empty%d.png",i));
		-- Octave loop
		myBGPartTableEmpty[i+8] = myBGPartTableEmpty[i+1];
	end

	-- Do note labels next.
	for i=0,13 do
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
	--setBigBg("assets/Free/Images/pcBackground0.png");
	loadTheme(getThemeIndex())
end

-- Add notes
-- Reuse these three arrays. Make all three then add all three at once and repeat.
myNoteArray = {};
myNoteFlatArray = {};
myNoteFlatArray = {};
myLoadedSoundsArray = {};

-- Add piano notes
addTripleNotes("assets/Proprietary/Sound/piano_%d.wav",1,"assets/Proprietary/Images/piano.png","assets/Proprietary/Images/pianoSharp.png","assets/Proprietary/Images/pianoFlat.png",'P');

-- Add bass notes
addTripleNotes("assets/Proprietary/Sound/bass_%d.wav",4,"assets/Proprietary/Images/bass.png","assets/Proprietary/Images/bassSharp.png","assets/Proprietary/Images/bassFlat.png",'B');

-- Add drums, this is unique because only half of the note sounds are unique. Use the special function loadHalfSounds
myNoteArray = sortLoadedSounds(loadHalfSounds("assets/Proprietary/Sound/drum_%d.wav"));
addNote(7,loadImage("assets/Proprietary/Images/drum.png"),myNoteArray);
setGearInfo(7,'D',neutralAccidental); -- One of the only strange notes that can be used in audio gear

-- Add the blank note, we don't have sounds for it.
addNote(8,loadImage("assets/Proprietary/Images/blank.png"));

-- Add sax notes
addTripleNotes("assets/Proprietary/Sound/sax_%d.wav",9,"assets/Proprietary/Images/sax.png","assets/Proprietary/Images/saxSharp.png","assets/Proprietary/Images/saxFlat.png",'S');

-- Add the repeat notes, no sound effects for them
addNote(12,loadImage("assets/Proprietary/Images/repeatStart.png"));
addNote(13,loadImage("assets/Proprietary/Images/repeatEnd.png"));
-- Special IDs
setSpecialID("repeatStart",12);
setSpecialID("repeatEnd",13);

-- Add the spooky note. It doesn't have a flat or sharp friend, so we use a different function to load the sounds
addSingleNote("assets/Proprietary/Sound/spooky_%d.wav",14,"assets/Proprietary/Images/spooky.png");

-- Add the audio gear
addNote(15,loadImage("assets/Proprietary/Images/audioGear.png"));
setSpecialID("audioGear",15);

-- Add fltue notes
addTripleNotes("assets/Proprietary/Sound/flute_%d.wav",16,"assets/Proprietary/Images/flute.png","assets/Proprietary/Images/fluteSharp.png","assets/Proprietary/Images/fluteFlat.png",'F');

-- Add festive note, like spooky is it special
addSingleNote("assets/Proprietary/Sound/festive_%d.wav",19,"assets/Proprietary/Images/festive.png");

-- Add guitar notes
addTripleNotes("assets/Proprietary/Sound/spanish_guitar_%d.wav",20,"assets/Proprietary/Images/guitar.png","assets/Proprietary/Images/guitarSharp.png","assets/Proprietary/Images/guitarFlat.png",'G');

-- Add violin notes
addTripleNotes("assets/Proprietary/Sound/violin_%d.wav",23,"assets/Proprietary/Images/violin.png","assets/Proprietary/Images/violinSharp.png","assets/Proprietary/Images/violinFlat.png",'V');

-- Add lyre notes
addTripleNotes("assets/Proprietary/Sound/lyre_%d.wav",26,"assets/Proprietary/Images/lyre.png","assets/Proprietary/Images/lyreSharp.png","assets/Proprietary/Images/lyreFlat.png",'L');

-- Add electric guitar note
addTripleNotes("assets/Proprietary/Sound/electric_guitar_%d.wav",29,"assets/Proprietary/Images/electricg.png","assets/Proprietary/Images/electricgsharp.png","assets/Proprietary/Images/electricgflat.png",'E');

-- Add mexican trumpet note
addTripleNotes("assets/Proprietary/Sound/mexican_trumpet_%d.wav",32,"assets/Proprietary/Images/mtrumpet.png","assets/Proprietary/Images/mtrumpetsharp.png","assets/Proprietary/Images/mtrumpetflat.png",'T');
