/*
 * EmulateCache.c
 *
 *  Created on: Oct 2, 2015
 *      Author: DebashisGanguly
 */

#include "../Global/TomasuloSimulator.h"

int getHashCodeFromCacheAddress (void *address);
int compareInstructions (void *instruction1, void *instruction2);
int compareMemoryValues (void *value1, void *value2);

int getHashCodeFromCodeLabel (void *codeLabel);
int compareCodeLabelAddress (void *codeLabelAddress1, void *codeLabelAddress2);

/**
 * This function emulates loader. Fills instruction and data cache upon reading .DAT file.
 * @param fileName: .DAT file provided as input to simulator
 */
void fillInstructionAndDataCache (char *fileName) {
	char *line = (char *) malloc (sizeof(char) * MAX_LINE);
	char *tempLine = (char *) malloc (sizeof(char) * MAX_LINE);
	char label [MAX_LINE];
	char *memoryAddress;
	char *memoryValue;

	size_t len = 0;
	ssize_t read;

	int dataSegmentFound = 0;
	int codeLabelNotFound = 1;
	int currentAddress;

	FILE *fp;

	if ((fp = fopen(fileName, "r")) == NULL) {
		perror ("Error to open the configuration file...");
		exit (EXIT_FAILURE);
	}

	//Instantiate caches as dictionary data structure keyed by HEX address
	instructionCache = createDictionary (getHashCodeFromCacheAddress, compareInstructions);
	dataCache = createDictionary (getHashCodeFromCacheAddress, compareMemoryValues);
	codeLabels = createDictionary (getHashCodeFromCodeLabel, compareCodeLabelAddress);

	numberOfInstruction = 0;

	while ((read = getline(&line, &len, fp)) != -1) { //loop to read file line by line and tokenize
		strcpy (tempLine, line);

		if ((tempLine = strtok(tempLine, WHITE_SPACE)) == NULL || *tempLine == 0)
			continue;

		if (strcmp (DATA_LABEL, tempLine) == 0)
			dataSegmentFound = 1;

		if (!dataSegmentFound) { //Check whether the section in file comes before DATA tag
			currentAddress = instructionCacheBaseAddress + (cacheLineSize * numberOfInstruction++);

			void *addrPtr = malloc(sizeof(int));
			*((int*)addrPtr) = currentAddress;

			if (strchr(line, LABEL_TERMINATOR_CHAR) != NULL) {
				if ((line = strtok(line, LABEL_TERMINATOR)) == NULL || *line == 0)
					continue;
				codeLabelNotFound = 0;
				strcpy (label, line);
			}

			if (!codeLabelNotFound) { //If loops and other code labels found maintain address as value and the label name as key in dictionary
				codeLabelNotFound = 1;

				if ((line = strtok(NULL, LABEL_TERMINATOR)) == NULL || *line == 0)
					continue;

				//printf("InstructionAddress:%d->Label:%s\n", currentAddress, label);
				addDictionaryEntry (codeLabels, (void *) label, addrPtr);
			}

			if ((line = strtok(line, LINE_TERMINATOR)) == NULL || *line == 0)
				continue;

			//printf("InstructionAddress:%d->Instruction:%s\n", currentAddress, line);
			addDictionaryEntry (instructionCache, addrPtr, line);
		} else { //parse lines after DATA tag as memory values
			line = strtok(line, MEMORY_SEPARATOR);

			if(strcmp(line, MEMORY_LABEL) == 0) {
				memoryAddress = strtok(NULL, MEMORY_SEPARATOR);
				memoryValue = strtok(NULL, MEMORY_SEPARATOR);

				void *addrPtr = malloc(sizeof(int));
				*((int*)addrPtr) = atoi (memoryAddress);

				void *valuePtr = malloc(sizeof(double));
				*((double*)valuePtr) = atof (memoryValue);

				//printf("Mem(%d) = %0.2lf\n", atoi (memoryAddress), atof (memoryValue));
				addDictionaryEntry (dataCache, addrPtr, valuePtr);
			}
		}

		line = (char *) malloc (sizeof(char) * MAX_LINE);
		tempLine = (char *) malloc (sizeof(char) * MAX_LINE);
	}

	if (fp)
		fclose(fp);
}

/**
 * This is a helper method for instruction and data cache to generate hash value for address keys
 * @param address
 * @return
 */
int getHashCodeFromCacheAddress (void *address) {
	return *((int*)address);
}

/**
 * Helper method for uniqueness of values in Instruction cache dictionary. Compare two instances of instruction line.
 * @param instruction1
 * @param instruction2
 * @return
 */
int compareInstructions (void *instruction1, void *instruction2) {
	return strcmp (((char*)instruction1), ((char*)instruction2));
}

/**
 * Helper method for uniqueness of values in data cache dictionary. Compare two instances of memory value.
 * @param value1
 * @param value2
 * @return
 */
int compareMemoryValues (void *value1, void *value2) {
	return *((double*)value1) - *((double*)value2);
}

/**
 * This is a helper method for code label dictionary to generate hash value for label string keys
 * @param codeLabel
 * @return
 */
int getHashCodeFromCodeLabel (void *codeLabel) {
	unsigned long hash = 5381;
    int c;
    char *str = (char *) codeLabel;

    while ((c = *str++) != '\0')
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

/**
 * Helper method for uniqueness of values in code label dictionary. Compare two instances of address value.
 * @param codeLabelAddress1
 * @param codeLabelAddress2
 * @return
 */
int compareCodeLabelAddress (void *codeLabelAddress1, void *codeLabelAddress2) {
	return *((int*)codeLabelAddress1)  - *((int*)codeLabelAddress2);
}
