#include "simulator.h"

int startAddress = 0x00000;//base address for instructions

int NF;
int NW;
int NR;
int NB;

//comparision functions required for storing in hash dictionary
int compareInstructions(void* instr1, void* instr2) {
	return strcmp(((char*)instr1), ((char*)instr2));
}

int compareMemVal(void* val1, void* val2) {
	return *((float*)val1) - *((float*)val2);
}

int compareAddress(void* adr1, void* adr2) {
    return *((int*)adr1) - *((int*)adr2);
}

int hashBranchLabel(void* blbl) {

	//Common Hash function method for string by Dan Bernstein.
	unsigned long hash = 5381;
	int c;
	char* str = (char*)blbl;

	while ((c = *str++) != '\0')
		hash = ((hash << 5) + hash) + c;

	return hash;//returns the hash code for branch label. This is used as key in dictionary
}

//returns the address of the instruction
int hashInstrAddr(void* InstructionAddress) {
    return *((int*)InstructionAddress);
}

int hashBranchAddr(void* branchAddress) {
    //use bits 7-4 for hashing the BTB entry
    int mask = 0b0000000011110000;
	int result = *((int*)branchAddress) & mask;
	result = result >> 4;
	result = result << 24;
	return result;
       
}

Processor* processor;

int main(int argc, char** argv) {

	//read the data from user
    printf("Enter NF: ");
    scanf("%d", &NF);

    printf("\nEnter NW: ");
    scanf("%d", &NW);

    printf("\nEnter NR: ");
    scanf("%d", &NR);

    printf("\nEnter NB: ");
    scanf("%d", &NB);

	// OPEN TEST FILE
	FILE* fp = fopen(argv[1], "r");

	if (fp == NULL) {
		printf("file error!\n");
		return 0;
	}

	//store the instructions in a dictionary with address as key and instr as value
	instructionMemory = createDictionary(hashInstrAddr, compareInstructions);

	//store the memory content and its value in a dictionary with address of memory location as key
	dataMemory = createDictionary(hashInstrAddr, compareMemVal);

	//store the branch label and its address in a dictionary with branch label as key
	branchLabels = createDictionary(hashBranchLabel, compareAddress);

	//store the total number of instructions
	lineNumber = 0;

	//flag=1 if the instructions are being read. flag=0 if memory content is being read
	int instructionsStart = 0;

	char* line = (char*)malloc(sizeof(char) * 1024);
	size_t len = 0;
	ssize_t y;
	//READ FILE
	while ((y=getline(&line, &len, fp))>=0)
	{
		char* temp = (char*)malloc(sizeof(char) * 1024);
		char label[1024];
		strcpy(temp, line);
		temp = strtok(temp, " \t\n");
		//skip empty line and indicate start of instructions
		if (temp == NULL || *temp==0) {
			instructionsStart = 1;
			continue;
		}
		int* addrPtr = (int*)malloc(sizeof(int));
		if (instructionsStart==0)
		{ //parse lines after memory content tag as memory values
			*(addrPtr) = atoi(strtok(line, ", "));

			float* valuePtr = (float*)malloc(sizeof(float));
			*(valuePtr) = atof(strtok(NULL, ", "));
			addDictionaryEntry(dataMemory, addrPtr, valuePtr);
		}
		else 
		{
			//start reading the instructions
			*(addrPtr) =startAddress + (lineNumber * 4);
			lineNumber += 1;
			if (strchr(line, ':') != NULL) {//if a branch lable is found
				if ((line = strtok(line, ":")) == NULL || *line == 0)
					continue;
				strcpy(label, line);
				if ((line = strtok(NULL, ":")) == NULL || *line == 0)
					continue;
				//store label and its address
				addDictionaryEntry(branchLabels, (void*)label, addrPtr);
			}

			if ((line = strtok(line, "\n")) == NULL || *line == 0)
				continue;
			//store instruction and its address
			addDictionaryEntry(instructionMemory, addrPtr, line);
		}
		line = (char*)malloc(sizeof(char) * 1024);
	}
	fclose(fp);
	//finished reading the testfile.

	//initialize the processor struct
	processor = (Processor*)malloc(sizeof(Processor));
    initializeProcessor ();
	    DictionaryEntry* instr = instructionMemory->head;
    printf("Instructions:\n");
    while (instr) {
        printf("%d: %s\n", *((int*)instr->key), ((char*)instr->value->value));
        instr = instr->next;
    }
	printf("initial Memory values:\n");
	instr = dataMemory->head;
	while (instr) {
		printf("Mem[%d] = %0.2lf\n", *((int*)instr->key), *((float*)instr->value->value));
		instr = instr->next;
	}
    while(1){
		processor->cycle++;

		printf("\nCycle: %d\n", processor->cycle);
		fetch();//fetch stage
		decode();
		issue();
		ExecutedInstr** unitOutputs = execute();
		memStage(unitOutputs);
				    	int returncount = Commit();
    	processor->totalcommits += returncount;//total committed instructions until now
		WB(returncount);//should writeback NB-commits number only. as the cdb busses are limited.

		//fdPipelineres has fetched instructions from previous cycle
		//updates fdpipeline so that the decode stage can read from it.
		instrstring* instructionEntry;
		while ((instructionEntry = dequeue(processor->fdPipelineres)) != NULL) {
			enqueue(processor->fdPipeline, instructionEntry);
		}

        displayRenamedReg();
	displayTomasuloTable();

		//check if we must exit.
		if (processor->PC >= (startAddress + (4 * lineNumber))) {//if all instructions have been fetched
			if ((!getSize(processor->dIPipeline)) && (!getSize(processor->fdPipeline))) {//if not instruction to decode
				if (!getSize(processor->reorderBuffer)) {//no instructionleft to commit
					//if (!(countDictionaryLen(processor->WriteBackBuffer))) {
					//	if(processor->FPdivPipelineBusy==0 && processor->FPaddPipelineBusy ==0&& processor->FPmulPipelineBusy ==0)
						//finished all instructions
						break;
					//}
				}
			}
		}
    }
		
	printf("Final Memory values:\n");
 instr = dataMemory->head;
	while (instr) {
		printf("Mem[%d] = %0.2lf\n", *((int*)instr->key), *((float*)instr->value->value));
		instr = instr->next;
	}


    //print registers for functional
	int i;
	printf("\nRegisters:\n");
	for (i = 0; i < 32; i++) {
		printf("R%d = %d\t\t", i, processor->integerRegisters[i]);
		printf("F%d = %0.2lf\n", i, processor->floatingPointRegisters[i]);
	}

	printf("**************STATISTICS**************\n");
	printf("Total instructions committed: %d\n", processor->totalcommits);

	printf("\nStalls because the reservation stations are occupied: %d", processor->stallFullRS);
	printf("\nStalls because the Reorder Buffer is full: %d", processor->stallFullROB);
	float avg = (float)processor->cdb_usage / processor->cycle;
	printf("\nAverage CDB utilization: %f",avg);

    printf("\nTotal cycles taken: %d\n", processor->cycle);
	displayTomasuloTable();

	return 0;

}


void initializeProcessor() {
processor->previmme =-1;
             processor->prevrs=-1;
	//initialize all values to zero and allocate space for all queues
    processor->cycle = 0;
    processor->PC = startAddress;
    processor->StallIssue = 0;

    int i = 0;
    processor->registerstatus = (RegStatus**)malloc(sizeof(RegStatus*) * 64);
    while (i < 32) {
        processor->registerstatus[i] = (RegStatus*)malloc(sizeof(RegStatus));//int
        processor->registerstatus[i]->reorderNum = -1;
        processor->registerstatus[i]->busy = 0;
		processor->immeaddr[i].imme=-2;
				processor->immeaddr[i].rs=-2;
		processor->immeaddr[i].robnum=-2;
		processor->immeaddr[i+32].imme=-2;
				processor->immeaddr[i+32].rs=-2;
		processor->immeaddr[i+32].robnum=-2;
         processor->registerstatus[i+32] = (RegStatus*)malloc(sizeof(RegStatus));//float
        processor->registerstatus[i+32]->reorderNum = -1;
        processor->registerstatus[i+32]->busy = 0;
        processor->integerRegisters[i] = 0;
        processor->floatingPointRegisters[i] = 0.0;
        i++;
    }


    processor->INTPipeline = newQueue(1);
    processor->LoadStorePipeline = newQueue(1);
    processor->FPaddPipeline = newQueue(3);
    enqueue(processor->FPaddPipeline, NULL);
    enqueue(processor->FPaddPipeline, NULL);

    processor->FPmultPipeline = newQueue(4);
    enqueue(processor->FPmultPipeline, NULL);
    enqueue(processor->FPmultPipeline, NULL);
    enqueue(processor->FPmultPipeline, NULL);

    processor->FPdivPipeline = newQueue(8);
    processor->BUPipeline = newQueue(1);
    processor->FPdivPipelineBusy = 0;
    processor->FPaddPipelineBusy = 0;
    processor->FPmulPipelineBusy = 0;
    i=0;
    while(i < 7) {
        enqueue(processor->FPdivPipeline, NULL);
        i++;
    }
    processor->fdPipeline = newQueue(NF);
    processor->fdPipelineres = newQueue(NF);
    processor->dIPipeline = newQueue(150);
    processor->dIPipelineres = newQueue(150);
    processor->reorderBuffer = newQueue(NR);
    processor->tomTable = newQueue(200);
    processor->branchTargetBuffer = createDictionary(hashBranchAddr, compareAddress);
    processor->WBBuff = createDictionary(hashInstrAddr, compareAddress);
    processor->renameReg = createDictionary(hashInstrAddr, compareAddress);


    //Initialize reservation stations and load/store buffers
    for (i = 0; i < 14; i++)
    {
        //0-6 results
        //7-13 buffers
        processor->resv[i] = createDictionary(hashInstrAddr, compareAddress);
    }
    //statistics
    processor->cdb_usage = 0;
    processor->totalcommits = 0;
    processor->stallFullROB = 0;
    processor->stallFullRS = 0;

}

void displayRenamedReg () {
	DictionaryEntry *current;

	if (processor -> renameReg != NULL) {
		printf ("Registers Renamed:\n");
		for (current = processor -> renameReg -> head; current != NULL; current = current -> next){
			printf ("register p%d: value %f\n", *((int*)current -> key), *((float*)current -> value -> value));
	    }
	}
}

void displayTomasuloTable()
{
	ROB* ROBentry;
	int i = 0;
	if (processor->tomTable != NULL)
	{
		printf("Tomasulo Table:\n");
		printf("Instr address | FETCH| DECODE|  ISSUE | EX |  WB  | COMMIT \n");

		ROBentry = front(processor->tomTable);

		while (i < processor->tomTable->size) {
			printf("\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", ROBentry->instruction->address, ROBentry->instruction->fetchcycle, ROBentry->instruction->decodecycle, ROBentry->instruction->issuecycle, ROBentry->instruction->excycle,ROBentry->instruction->wbcycle, ROBentry->instruction->commitcycle);
			i++;
			ROBentry = processor->tomTable->items[(processor->tomTable->front + i) % processor->tomTable->maxsize];
		}
	}
}