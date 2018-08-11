#ifndef NATHANLISTHEADERHAVEBEENINCLUDED
#define NATHANLISTHEADERHAVEBEENINCLUDED

typedef struct nathanList_t{
	struct nathanList_t* nextEntry;
	void* memory;
}nathanList;

int getNathanListLength(nathanList* _startingList);
nathanList* getNathanList(nathanList* _listOn, int _index);
nathanList* addNathanList(nathanList* _startingList);
//void freeSingleEntry(nathanList* _deleteEntry, char _freeAdditionalMemory)
void removeNathanList(nathanList** _startingList, int _removeIndex, char _freeAdditionalMemory);
void freeNathanList(nathanList* _startingList, char _freeAdditionalMemory);

#endif