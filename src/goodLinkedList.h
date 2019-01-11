#ifndef NLISTHEADERINCLUDED
#define NLISTHEADERINCLUDED

// Optional way to change how the list is reallocated. Argument is the new minimum size of the list.
//#define REALLOCFORMULA(x) (x*2)
#define REALLOCFORMULA(x) (x)


#define ITERATENLIST(_passedStart,_passedCode)					\
	{															\
		if (_passedStart!=NULL){								\
			nList* _currentnList=_passedStart;					\
			nList* _cachedNext;									\
			do{													\
				_cachedNext = _currentnList->nextEntry;			\
				_passedCode;									\
			}while((_currentnList=_cachedNext));				\
		}														\
	}

typedef struct _goodLinkedList{
	struct _goodLinkedList* nextEntry;
	void* data;
}nList;

nList* addnList(nList** _passed);
void freenList(nList* _freeThis, char _freeMemory);
nList* newnList();
int nListLen(nList* _passed);
nList* getnList(nList* _passed, int _index);
#endif