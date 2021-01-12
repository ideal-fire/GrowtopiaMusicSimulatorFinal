#include <stdio.h>
#include <stdlib.h>

#include "goodLinkedList.h"

nList* newnList(){
	nList* _ret = malloc(sizeof(nList));
	_ret->nextEntry = NULL;
	_ret->data = NULL;
	return _ret;
}


nList* getnList(nList* _passed, int _index){
	int i=0;
	ITERATENLIST(_passed,{
		if (i==_index){
			return _currentnList;
		}
		++i;
	})
	return NULL;
}
int nListLen(nList* _passed){
	if (_passed==NULL){
		return 0;
	}
	int _ret;
	for (_ret=1;(_passed=_passed->nextEntry)!=NULL;++_ret);
	return _ret;
}

nList* addnList(nList** _passed){
	if (*_passed==NULL){
		return (*_passed=newnList());
	}
	nList* _temp = *_passed;
	for (;_temp->nextEntry!=NULL;_temp=_temp->nextEntry);
	return (_temp->nextEntry = newnList());
}

void freenList(nList* _freeThis, char _freeMemory){
	ITERATENLIST(_freeThis,{
		if (_freeMemory){
			free(_currentnList->data);
		}
		free(_currentnList);
	})
}