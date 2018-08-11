#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nathanList.h"

int getNathanListLength(nathanList* _startingList){
	int i;
	for (i=1;;++i){
		if (_startingList->nextEntry!=NULL){
			_startingList = _startingList->nextEntry;
		}else{
			break;
		}
	}
	return i;
}
// 0 based
nathanList* getNathanList(nathanList* _listOn, int _index){
	int i;
	for (i=0;i<_index;++i){
		_listOn=_listOn->nextEntry;
	}
	return _listOn;
}
// Adds to the end. Will return what you pass to it if what you passed points to NULL for memory
nathanList* addNathanList(nathanList* _startingList){
	if (_startingList->memory==NULL){
		return _startingList;
	}else{
		nathanList* tempList = calloc(1,sizeof(nathanList));
		nathanList* listOn = (getNathanList(_startingList,getNathanListLength(_startingList)-1));
		// Check if there's already an entry where we're about to overwrite
		if (listOn->nextEntry){
			printf("Problem add to list, maybe we'll loose some entries.");
		}
		listOn->nextEntry=tempList;
		return tempList;
	}
}

void freeSingleEntry(nathanList* _deleteEntry, char _freeAdditionalMemory){
	if (_freeAdditionalMemory){
		free(_deleteEntry->memory);
	}
	free(_deleteEntry);
}

void removeNathanList(nathanList** _startingList, int _removeIndex, char _freeAdditionalMemory){
	// Special if it's the first entry
	if (_removeIndex==0){
		nathanList* _tempHold = (*_startingList)->nextEntry;
		freeSingleEntry(*_startingList,_freeAdditionalMemory);
		if (_tempHold!=NULL){
			*_startingList=_tempHold;
		}else{
			*_startingList = calloc(1,sizeof(nathanList));
			printf("Can't remove only entry, only reset.\n");
		}
		return;
	}
	// Regular
	int i;
	nathanList* _iterationList = *_startingList;
	for (i=0;i<_removeIndex;++i){
		if (i==_removeIndex-1){
			nathanList* _entryToDelete = _iterationList->nextEntry;
			nathanList* _nextEntryOfDeleted = _entryToDelete->nextEntry;
			freeSingleEntry(_entryToDelete,_freeAdditionalMemory);
			_iterationList->nextEntry = _nextEntryOfDeleted;
			break;
		}else{
			_iterationList = _iterationList->nextEntry;
		}
	}
}

void freeNathanList(nathanList* _startingList, char _freeAdditionalMemory){
	do{
		nathanList* _tempHold = _startingList->nextEntry;
		freeSingleEntry(_startingList,_freeAdditionalMemory);
		_startingList = _tempHold;
	}while(_startingList!=NULL);
}

#ifdef ADD_USELESS_DEBUG_CODE
	int testNathanList(){
		// Init lists yourself
		nathanList* _startList = calloc(1,sizeof(nathanList));
		// Test adding entries with string data
		addNathanList(_startList)->memory = strdup("whan.");
		addNathanList(_startList)->memory = strdup("too");
		addNathanList(_startList)->memory = strdup("free");
		addNathanList(_startList)->memory = strdup("fore");
		// Test removing from the middle
		removeNathanList(&_startList,1,1);
		// Print contents
		int i;
		for (i=0;i<getNathanListLength(_startList);++i){
			printf("%d:%s\n",i,getNathanList(_startList,i)->memory);
		}
		// Test freeing list
		freeNathanList(_startList,1);
	}
#endif