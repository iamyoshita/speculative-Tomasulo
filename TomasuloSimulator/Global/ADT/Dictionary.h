/*
 * Dictionary.h
 *
 *  Created on: Oct 2, 2015
 *      Author: DebashisGanguly
 */

#ifndef GLOBAL_ADT_DICTIONARY_H_
#define GLOBAL_ADT_DICTIONARY_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	Dictionary *table = (Dictionary *)malloc (sizeof(Dictionary));
	table -> head = NULL;
	table -> getHashCode = getHashCode;
	table -> compareValues = compareValues;
	return table;
}

static inline DictionaryValue *createDictionaryValue (void *value)  {
	DictionaryValue *newValue = (DictionaryValue *)malloc (sizeof(DictionaryValue));
	newValue -> value = value;
	newValue -> next = NULL;
	return newValue;
}

static inline DictionaryEntry *createDictionaryEntry (Dictionary *table, void *key, void *value) {
	DictionaryEntry *newEntry = (DictionaryEntry *)malloc (sizeof(DictionaryEntry));
	newEntry -> hashKey = table -> getHashCode (key);
	newEntry -> key = key;
	newEntry -> value = createDictionaryValue (value);
	newEntry -> next = NULL;
	return newEntry;
}

static inline DictionaryEntry *getValueChainByDictionaryKey (Dictionary *table, void *key) {
	DictionaryEntry *match = NULL;
	DictionaryEntry *current;

	int hashKey = table -> getHashCode (key);

	if (table != NULL) {
		for (current = table -> head; current != NULL; current = current -> next)
			if (current -> hashKey == hashKey) {
				match = current;
				break;
			}
	}

	return match;
}

static inline void addValueToDictionaryChain (DictionaryEntry *existing, void *value, _compareValues compareValues) {
	DictionaryValue *previous;
	DictionaryValue *current;
	DictionaryValue *match = NULL;

	for (current = existing -> value; current != NULL; previous = current, current = current -> next)
		if (compareValues (value, current -> value) == 0) {
			match = current;
		}

	if (match == NULL)
		previous -> next = createDictionaryValue (value);
}

static inline void addDictionaryEntry (Dictionary *table, void *key, void *value) {
	DictionaryEntry *existing;

	if (table != NULL) {
		if (table -> head == NULL) {
			table -> head = createDictionaryEntry(table, key, value);
		} else {
			if ((existing = getValueChainByDictionaryKey(table, key)) != NULL) {
				addValueToDictionaryChain (existing, value, table -> compareValues);
			} else {
				DictionaryEntry *previous;
				DictionaryEntry *current;

				for (current = table -> head; current != NULL; previous = current, current = current -> next);

				previous -> next = createDictionaryEntry (table, key, value);
			}
		}
	}
}

static inline void removeDictionaryEntriesByKey (Dictionary *table, void *key) {
	DictionaryEntry *previous;
	DictionaryEntry *current;
	DictionaryEntry *match = NULL;

	int hashKey = table -> getHashCode (key);

	if (table != NULL) {
		for (current = table -> head; current != NULL; previous = current, current = current -> next) {
			if (current -> hashKey == hashKey) {
				match = current;
				break;
			}
		}

		if (match != NULL) {
			if (match == table -> head) {
				if (match -> next == NULL) {
					table -> head = NULL;
					free (match);
				} else {
					table -> head = table -> head -> next;
					free (match);
				}
			} else {
				previous -> next = match -> next;
				free (match);
			}
		}
	}
}

static inline void removeDictionaryEntryByKeyValue (Dictionary *table, void *key, void *value) {
	DictionaryEntry *matchedEntry = getValueChainByDictionaryKey(table, key);

	DictionaryValue *previous;
	DictionaryValue *current;
	DictionaryValue *match = NULL;

	if (matchedEntry != NULL) {
		for (current = matchedEntry -> value; current != NULL; previous = current, current = current -> next)
			if (table -> compareValues (value, current -> value) == 0) {
				match = current;
				break;
			}

		if (match != NULL) {
			if (match == matchedEntry -> value) {
				if (match -> next == NULL)
					removeDictionaryEntriesByKey (table, key);
				else {
					matchedEntry -> value = matchedEntry -> value -> next;
					free (match);
				}
			} else {
				previous -> next = current -> next;
				free (matchedEntry -> value);
			}
		}
	}
}

static inline DictionaryEntry* popDictionaryEntry (Dictionary *table){
    DictionaryEntry *popEntry = NULL;
    if (table -> head != NULL){
        popEntry = table -> head;
        if (popEntry -> next == NULL) {
            table -> head = NULL;
        } else {
            table -> head = table -> head -> next;
            popEntry -> next = NULL;
        }
     }
    return popEntry;
}

static inline void appendDictionaryEntry (Dictionary *table, DictionaryEntry *newEntry){

	if (table != NULL) {
		if (table -> head == NULL) {
			table -> head = newEntry;
		} else {

            DictionaryEntry *previous;
            DictionaryEntry *current;

            for (current = table -> head; current != NULL; previous = current, current = current -> next){
            }

            previous -> next = newEntry;
		}
    }
    return;
}

static inline int countDictionaryLen (Dictionary *table){
    if (table == NULL){
        return 0;
    }else{

        int count = 0;
        DictionaryEntry *previous;
        DictionaryEntry *current;

        for (current = table -> head; current != NULL; previous = current, current = current -> next){
            count ++;
        }
        return count;
    }
}

/*
static inline int getDictionaryLen (Dictionary *table){
    DictionaryEntry *current = table -> head;
    int len = 0;
    while(current!=NULL){
        len += 1;
        current = current -> next;
    }
    return len;
}
*/

#endif /* GLOBAL_ADT_DICTIONARY_H_ */
