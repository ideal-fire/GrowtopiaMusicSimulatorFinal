if (isMobile()) then
	myBGPartTableEmpty = {};
	myBGPartyTableLabel = {};

	-- Load the images for empty slots and filled slots. There are 7 unique images.
	for i=0,6 do
		-- Do empty spot images first
		myBGPartTableEmpty[i+1] = loadImage(string.format("assets/Free/Images/PartBG/empty%d.png",i));
		-- Octave loop
		myBGPartTableEmpty[i+7] = myBGPartTableEmpty[i+1];

		-- Do note labels next. Fun fact, this note label feature was coincidentally used in somebody else's Growtopia Music Simulator....except he put the labels on the left side. What? I'm not implying anything.
		myBGPartyTableLabel[i+1] = loadImage(string.format("assets/Free/Images/PartBG/label%d.png",i));
		myBGPartyTableLabel[i+7] = myBGPartyTableLabel[i+1];
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