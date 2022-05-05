
#ifndef cqheader
#define cqheader

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Data structure to represent a queue
struct Cqueue
{
    void** items;     // array to store Cqueue elements
    int maxsize;    // maximum capacity of the Cqueue
    int front;      // front points to the front element in the Cqueue (if any)
    int rear;       // rear points to the last element in the Cqueue
    int size;       // current capacity of the Cqueue
};

// Utility function to initialize a queue
static inline struct Cqueue* newQueue(int size)
{
    struct Cqueue* pt=NULL;
    pt=(struct Cqueue*)malloc(sizeof(struct Cqueue));

    pt->items=(void**)malloc(size * sizeof(void*));
    pt->maxsize=size;
    pt->front=0;
    pt->rear=0;
    pt->size=0;

    return pt;
}

// Utility function to return the size of the queue
static inline int getSize(struct Cqueue* pt) {
    if (pt == NULL)
        return 0;
    return pt->size;
}

static inline int getMaxSize(struct Cqueue* pt) {
    if (pt == NULL)
        return 0;
    return pt->maxsize;
}

// Utility function to check if the queue is empty or not
static inline int isEmpty(struct Cqueue* pt) {
    return !getSize(pt);
}

// Utility function to return the front element of the queue
static inline void* front(struct Cqueue* pt)
{
    void* ans=NULL;
    if (getSize(pt)==0)
    {
        return ans;
    }

    ans=pt->items[pt->front];
    return ans;
}

static inline int isValid(struct Cqueue* pt, int index) {
    if (pt->front < pt->rear) {
        return (index >= pt->front && index <= pt->rear);
    }
    else {
        return (index >= pt->front && index < pt->maxsize) || (index <= pt->rear);
    }
}

// Utility function to add an element `x` to the Cqueue
static inline void enqueue(struct Cqueue* pt, void* x)
{
    if (pt->size == pt->maxsize)
    {
        return;
    }
    pt->items[pt->rear]=x;
    pt->rear=(pt->rear + 1) % pt->maxsize;    // circular Cqueue
    pt->size++;

}

// Utility function to deCqueue the front element
static inline void* dequeue(struct Cqueue* pt)
{
    void* ans=NULL;
    if (getSize(pt)==0)    // front == rear
    {
        return ans;
    }
    ans=pt->items[pt->front];
    pt->front=(pt->front + 1) % pt->maxsize;  // circular Cqueue
    pt->size--;
    return ans;

}

static inline int isFull(struct Cqueue* pt) {
    return getSize(pt) == pt->maxsize;
}

typedef int (*_getHashCode)(void *key);
typedef int (*_compareValues)(void *value1, void *value2);

typedef struct _dictionaryValue {
	void *value;
	struct _dictionaryValue *next;
} DictionaryValue;

typedef struct _dictionaryEntry {
	int hashKey;
	void *key;
	DictionaryValue *value;
	struct _dictionaryEntry *next;
} DictionaryEntry;

typedef struct _dictionary {
	DictionaryEntry *head;
	_getHashCode getHashCode;
	_compareValues compareValues;
} Dictionary;

static inline Dictionary *createDictionary (_getHashCode getHashCode, _compareValues compareValues) {
	Dictionary *table=(Dictionary *)malloc (sizeof(Dictionary));
	table -> head=NULL;
	table -> getHashCode=getHashCode;
	table -> compareValues=compareValues;
	return table;
}

static inline DictionaryValue *createDictionaryValue (void *value)  {
	DictionaryValue *newValue=(DictionaryValue *)malloc (sizeof(DictionaryValue));
	newValue -> value=value;
	newValue -> next=NULL;
	return newValue;
}

static inline DictionaryEntry *createDictionaryEntry (Dictionary *table, void *key, void *value) {
	DictionaryEntry *newEntry=(DictionaryEntry *)malloc (sizeof(DictionaryEntry));
	newEntry -> hashKey=table -> getHashCode (key);
	newEntry -> key=key;
	newEntry -> value=createDictionaryValue (value);
	newEntry -> next=NULL;
	return newEntry;
}

static inline DictionaryEntry *getValueChainByDictionaryKey (Dictionary *table, void *key) {
	DictionaryEntry *match=NULL;
	DictionaryEntry *current;

	int hashKey=table -> getHashCode (key);
	if (table != NULL) {
		for (current=table -> head; current != NULL; current=current -> next)
			if (current -> hashKey == hashKey) {
				match=current;
				break;
			}
	}
	return match;
}

static inline void addValueToDictionaryChain (DictionaryEntry *existing, void *value, _compareValues compareValues) {
	DictionaryValue *previous;
	DictionaryValue *current;
	DictionaryValue *match=NULL;

	for (current=existing -> value; current != NULL; previous=current, current=current -> next)
		if (compareValues (value, current -> value) == 0) {
			match=current;
		}

	if (match == NULL)
		previous -> next=createDictionaryValue (value);
}

static inline void addDictionaryEntry (Dictionary *table, void *key, void *value) {
	DictionaryEntry *existing;

	if (table != NULL) {
		if (table -> head == NULL) {
			table -> head=createDictionaryEntry(table, key, value);
		} else {
			if ((existing=getValueChainByDictionaryKey(table, key)) != NULL) {
				addValueToDictionaryChain (existing, value, table -> compareValues);
			} else {
				DictionaryEntry *previous;
				DictionaryEntry *current;

				for (current=table -> head; current != NULL; previous=current, current=current -> next);

				previous -> next=createDictionaryEntry (table, key, value);
			}
		}
	}
}

static inline void removeDictionaryEntriesByKey (Dictionary *table, void *key) {
	DictionaryEntry *previous;
	DictionaryEntry *current;
	DictionaryEntry *match=NULL;

	int hashKey=table -> getHashCode (key);

	if (table != NULL) {
		for (current=table -> head; current != NULL; previous=current, current=current -> next) {
			if (current -> hashKey == hashKey) {
				match=current;
				break;
			}
		}

		if (match != NULL) {
			if (match == table -> head) {
				if (match -> next == NULL) {
					table -> head=NULL;
					free (match);
				} else {
					table -> head=table -> head -> next;
					free (match);
				}
			} else {
				previous -> next=match -> next;
				free (match);
			}
		}
	}
}


static inline DictionaryEntry* popDictionaryEntry (Dictionary *table){
    DictionaryEntry *popEntry=NULL;
    if (table -> head != NULL){
        popEntry=table -> head;
        if (popEntry -> next == NULL) {
            table -> head=NULL;
        } else {
            table -> head=table -> head -> next;
            popEntry -> next=NULL;
        }
     }
    return popEntry;
}

static inline void appendDictionaryEntry (Dictionary *table, DictionaryEntry *newEntry){

	if (table != NULL) {
		if (table -> head == NULL) {
			table -> head=newEntry;
		} else {

            DictionaryEntry *previous;
            DictionaryEntry *current;

            for (current=table -> head; current != NULL; previous=current, current=current -> next){
            }

            previous -> next=newEntry;
		}
    }
    return;
}

static inline int countDictionaryLen (Dictionary *table){
    if (table == NULL){
        return 0;
    }else{

        int count=0;
        DictionaryEntry *previous;
        DictionaryEntry *current;

        for (current=table -> head; current != NULL; previous=current, current=current -> next){
            count ++;
        }
        return count;
    }
}


#endif 

