function clearRow(_rowNum)
	for i=0,getSongWidth()-1 do
		setNoteSpot(i,_rowNum,0);
	end
end

local _selectedRow = getNumberInput("Which row to delete? (Zero based) Enter 99 to clear song",0);

if (_selectedRow==99) then
	for i=0,getSongHeight()-1 do
		clearRow(i)
	end
elseif (_selectedRow>=getSongHeight()) then
	easyMessage("out of range.")
else
	clearRow(_selectedRow)
end

-- Cleanup
_selectedRow=nil;
clearRow=nil;