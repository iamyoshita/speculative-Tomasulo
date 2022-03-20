/*
 * CPU.c
 *
 *  Created on: Oct 3, 2015
 *      Author: DebashisGanguly
 */

# include "../Global/TomasuloSimulator.h"

int getHashCodeFromPCHash (void *PCHash);
int compareInstructions (void *instruction1, void *instruction2);
int getHashCodeFromInstructionAddress(void *InstructionAddress);
int compareDecodedInstructions(void *decodedInstruction1, void *decodedInstruction2);
int getHashCodeFromBranchAddress(void *branchAddress);
int compareTargetAddress(void *targetAddress1, void *targetAddress2);
int getHashCodeFromROBNumber (void *ROBNumber);
int compareROBNumber (void *ROBNumber1, void *ROBNumber2);
int getHashCodeFromRegNumber (void *RegNumber);
int compareRegNumber (void *RegNumber1, void *RegNumber2);

DictionaryValue *checkReservationStation(DictionaryEntry *dictEntry, int isFloat);
void printPipeline(void *instruction, char *pipeline, int entering);
void branchHelper(CompletedInstruction *instructionAndResult);

int fetchMultiInstructionUnit(int NF);
Instruction * decodeInstruction(char *instruction_str, int instructionAddress);
int decodeInstructionsUnit();
void updateFetchBuffer();
void updateInstructionQueue();
void updateReservationStations();
int renameRegIsFull(Dictionary *renameReg, int d);
int addInstruction2RSint(Dictionary *renameRegInt, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction, RegStatus **IntRegStatus);
int addInstruction2RSfloat(Dictionary *renameRegFP, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction, RegStatus **FPRegStatus);
int addInstruction2RSbranch(Dictionary *renameRegInt, Dictionary *resSta, Dictionary *resStaResult,
                            char* rsType, int maxLenRS, Instruction *instruction, RegStatus **IntRegStatus);
int issueInstruction(Instruction *instruction);
int issueUnit(int NW);



ROB * InitializeROBEntry(Instruction * instruction);
CompletedInstruction **execute(int NB);

void insertintoWriteBackBuffer(int NB);
int writeBackUnit(int NB, int returncount);
void updateOutputRES(CompletedInstruction *instruction);
int commitInstuctionCount();
int Commit(int NC, int NR, int returncount);
int CommitUnit(int NB, int NR);
void calculate(int perutilization);
int checkEnd();


/**
 * This method initializes CPU data structures and all its data members
 */
void initializeCPU (int NI, int NR) {
	int i;

	cpu = (CPU *) malloc (sizeof(CPU));

	cpu -> cycle = 0;
	cpu -> PC = instructionCacheBaseAddress;

	 //initialize integer registers
	cpu -> integerRegisters = (INTReg **) malloc (sizeof(INTReg *) * numberOfIntRegisters);

	for (i = 0; i < numberOfIntRegisters; i++) {
		cpu -> integerRegisters [i] = (INTReg *) malloc (sizeof(INTReg));
		cpu -> integerRegisters [i] -> data = 0;
		cpu -> integerRegisters [i] -> intResult = 0;
 	}

	 //initialize floating point registers
	cpu -> floatingPointRegisters = (FPReg **) malloc (sizeof(FPReg *) * numberOfFPRegisters);

	for (i = 0; i < numberOfFPRegisters; i++) {
		cpu -> floatingPointRegisters [i] = (FPReg *) malloc (sizeof(FPReg));
		cpu -> floatingPointRegisters [i] -> data = 0.0;
		cpu -> floatingPointRegisters [i] -> fpResult = 0;
 	}


	//initialize integer registers status
	cpu -> IntRegStatus = (RegStatus **) malloc (sizeof(RegStatus *) * numberOfIntRegisters);
	for (i = 0; i < numberOfIntRegisters; i++) {
		cpu -> IntRegStatus [i] = (RegStatus *) malloc (sizeof(RegStatus));
		cpu -> IntRegStatus [i] -> reorderNum = -1;
		cpu -> IntRegStatus [i] -> busy = 0;
 	}

 	//initialize floating point registers status
	cpu -> FPRegStatus = (RegStatus **) malloc (sizeof(RegStatus *) * numberOfFPRegisters);
	for (i = 0; i < numberOfFPRegisters; i++) {
		cpu -> FPRegStatus [i] = (RegStatus *) malloc (sizeof(RegStatus));
		cpu -> FPRegStatus [i] -> reorderNum = -1;
		cpu -> FPRegStatus [i] -> busy = 0;

 	}

    //initialize Pipelines with bubbles and appropriate size
    cpu -> INTPipeline = createCircularQueue(1);
    cpu -> LoadStorePipeline = createCircularQueue(1);
    cpu -> FPaddPipeline = createCircularQueue(3);
    cpu -> FPmultPipeline = createCircularQueue(4);
    cpu -> FPdivPipeline = createCircularQueue(8);
    cpu -> FPdivPipelineBusy = 0;
    cpu -> BUPipeline = createCircularQueue(1);
    for (i = 0; i < 7; i++) {
        if (i < 2) {
            enqueueCircular (cpu -> FPaddPipeline, NULL);
        }
        if (i < 3) {
            enqueueCircular (cpu -> FPmultPipeline, NULL);
	    }
	    enqueueCircular (cpu -> FPdivPipeline, NULL);
    }
    //Initialize fetch buffer and instruction queue
	cpu -> fetchBuffer = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> fetchBufferResult = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> instructionQueue = createCircularQueue(NI);
	cpu -> instructionQueueResult = createCircularQueue(NI);
	//Initialize BTB
	cpu -> branchTargetBuffer = createDictionary(getHashCodeFromBranchAddress, compareTargetAddress);

    //Flag for next fetch unit
	cpu -> stallNextFetch = 0;
    //Initialize renaming register
	cpu -> renameRegInt = createDictionary(getHashCodeFromRegNumber, compareRegNumber);
	cpu -> renameRegFP = createDictionary(getHashCodeFromRegNumber, compareRegNumber);


    //Initialize reservation stations and load/store buffers
    cpu -> resStaInt = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> loadBuffer = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> storeBuffer = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPadd = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPmult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPdiv = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaBU = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaIntResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> loadBufferResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> storeBufferResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPaddResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPmultResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPdivResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaBUResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
 
    //Initialize Stall counters
    cpu -> stallFullROB = 0;
    cpu -> stallFullRS = 0;

	// Initialize WB and ROB
	cpu -> reorderBuffer = createCircularQueue(NR);
	cpu -> WriteBackBuffer = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
	
	// CDB utilization
	cpu -> percentutilizationpercycle = 0;
	cpu -> avgutilization = 0.0;
	
    //Initialize Flag of instructions after branch
    cpu -> isAfterBranch = 0;
    //Initialize flag of program that last next cycle fetched and next cycle decodes
    cpu->lastCycleFetchProgram = 0;
    cpu->nextCycleDecodeProgram = 0;
	
	//Initialize commit counter
	cpu -> commitCounter = 0;

}

//Fetch Instructions Unit
int fetchMultiInstructionUnit(int NF){

    int i;

    if (cpu -> stallNextFetch == 0){
        for(i=0; i<NF; i++){
            if (cpu -> PC >= (instructionCacheBaseAddress + (cacheLineSize * numberOfInstruction))) { //check whether PC exceeds last instruction in cache
                printf ("All instructions are fetched...\n");
                return 0;
            }

            //*((int*)addrPtr) = cpu -> PC;
            int *addrPtr = (int*) malloc(sizeof(int));
            *addrPtr = cpu -> PC;

            DictionaryEntry *currentInstruction = getValueChainByDictionaryKey (instructionCache, addrPtr);

            char *instruction_str = (char *) malloc (sizeof(char) * MAX_LINE);
            strcpy (instruction_str, ((char*)currentInstruction -> value -> value));

            printf ("Fetched %d:%s\n", *addrPtr, instruction_str);

            addDictionaryEntry (cpu -> fetchBufferResult, addrPtr, instruction_str);

            //check if in BTB
            if (cpu -> branchTargetBuffer != NULL){
                DictionaryEntry *BTBEntry = getValueChainByDictionaryKey(cpu -> branchTargetBuffer, addrPtr);

                if (BTBEntry != NULL){

                    if (*((int*)BTBEntry -> key) == *addrPtr){
                        printf("Instruction %d is a branch in the BranchTargetBuffer with ", *addrPtr);
                        int targetAddress = *((int*)BTBEntry -> value -> value);

                        printf("target address %x.\n", targetAddress);
                        cpu -> PC = targetAddress;
                        return 1;
                    }
                }
            }
            cpu -> PC = cpu -> PC + 4;
        }
    }
    else{
        cpu -> stallNextFetch = 0;
        printf("Fetching stall in this cycle...\n");
    }
    return 1;
}


//Decode instruction unit
int decodeInstructionsUnit(){
    while(cpu -> fetchBuffer -> head!= NULL){
        if ((cpu->instructionQueue->count + cpu->instructionQueueResult->count) >= cpu->instructionQueue->size){
            printf("Cannot decode all fetched instructions because the instruction queue is full.\n");
            cpu -> stallNextFetch = 1;
            return 1;
        }else{
            DictionaryEntry *instructionEntry;
            Instruction *instruction;
            instructionEntry = popDictionaryEntry(cpu -> fetchBuffer);
            instruction = decodeInstruction(instructionEntry -> value -> value, *((int*)instructionEntry -> key));
            enqueueCircular(cpu -> instructionQueueResult, instruction);	
        }

    }
    return 1;
}


//Update fetch buffer
void updateFetchBuffer(){
    DictionaryEntry *instructionEntry;
    while((instructionEntry = popDictionaryEntry(cpu -> fetchBufferResult)) != NULL){
        appendDictionaryEntry(cpu -> fetchBuffer, instructionEntry);
    }
    return;
}


//Update instruction queue
void updateInstructionQueue(){
    Instruction *instruction;
    while((instruction = dequeueCircular(cpu->instructionQueueResult))!= NULL){
        enqueueCircular(cpu->instructionQueue, instruction);
    }
    return;
}


//Update reservation stations
void updateReservationStations(){
    DictionaryEntry *RSEntry;
    while((RSEntry = popDictionaryEntry(cpu -> resStaIntResult)) != NULL){
        appendDictionaryEntry(cpu -> resStaInt, RSEntry);
    }
    while((RSEntry = popDictionaryEntry(cpu -> loadBufferResult)) != NULL){
        appendDictionaryEntry(cpu -> loadBuffer, RSEntry);
    }
    while((RSEntry = popDictionaryEntry(cpu -> storeBufferResult)) != NULL){
        appendDictionaryEntry(cpu -> storeBuffer, RSEntry);
    }
    while ((RSEntry = popDictionaryEntry (cpu -> loadBufferResult)) != NULL) {
        appendDictionaryEntry (cpu -> loadBuffer, RSEntry);
    }
    while ((RSEntry = popDictionaryEntry (cpu -> storeBufferResult)) != NULL) {
        appendDictionaryEntry (cpu -> storeBuffer, RSEntry);
    }
    while((RSEntry = popDictionaryEntry(cpu -> resStaFPaddResult)) != NULL){
        appendDictionaryEntry(cpu -> resStaFPadd, RSEntry);
    }
    while((RSEntry = popDictionaryEntry(cpu -> resStaFPmultResult)) != NULL){
        appendDictionaryEntry(cpu -> resStaFPmult, RSEntry);
    }
    while((RSEntry = popDictionaryEntry(cpu -> resStaFPdivResult)) != NULL){
        appendDictionaryEntry(cpu -> resStaFPdiv, RSEntry);
    }
    while((RSEntry = popDictionaryEntry(cpu -> resStaBUResult)) != NULL){
        appendDictionaryEntry(cpu -> resStaBU, RSEntry);
    }
    return;
}

//Decode an instruction
Instruction * decodeInstruction(char *instruction_str, int instructionAddress){
    Instruction *instruction;

    char *token = (char *) malloc (sizeof(char) * MAX_LINE);

    OpCode op;

    int rd;
    int rs;
    int rt;

    int rsValue;
    int rtValue;

    int fd;
    int fs;
    int ft;

    double fsValue;
    double ftValue;

    int immediate;

    int target;


	op = NOOP, rd = -1, rs = -1, rt = -1, rsValue = -1, rtValue = -1, fd = -1, fs = -1, ft = -1, fsValue = -1, ftValue = -1, immediate = 0, target = 0;

	token = (char *) malloc (sizeof(char) * MAX_LINE);

	strcpy (token, instruction_str);

	token = strtok(token, " ,()\t\n");

    if(strcmp(token, "add") == 0) {
		op = ADD;

		token = strtok(NULL, " ,()RF\t\n");
		rd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rt = atoi(token);
	} else if(strcmp(token, "addi") == 0) {
		op = ADDI;

		token = strtok(NULL, " ,()RF\t\n");
		rd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");

		immediate = atoi(token);
	} else if(strcmp(token, "fld") == 0) {
		op = FLD;

		token = strtok(NULL, " ,()RF\t\n");
		ft = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		immediate = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);
	} else if(strcmp(token, "fsd") == 0) {
		op = FSD;

		token = strtok(NULL, " ,()RF\t\n");
		ft = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		immediate = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);
	} else if(strcmp(token, "fadd") == 0) {
		op = FADD;

		token = strtok(NULL, " ,()RF\t\n");
		fd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		fs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		ft = atoi(token);
	} else if(strcmp(token, "fsub") == 0) {
		op = FSUB;

		token = strtok(NULL, " ,()RF\t\n");
		fd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		fs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		ft = atoi(token);
	} else if(strcmp(token, "fmul") == 0) {
		op = FMUL;

		token = strtok(NULL, " ,()RF\t\n");
		fd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		fs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		ft = atoi(token);
	} else if(strcmp(token, "fdiv") == 0) {
		op = FDIV;

		token = strtok(NULL, " ,()RF\t\n");
		fd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		fs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		ft = atoi(token);
	} else if(strcmp(token, "bne") == 0) {
		op = BNE;

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rt = atoi(token);

		token = strtok(NULL, " ,()\t\n");
	} else {
		printf("Invalid instruction %s...\n", instruction_str);
		exit (EXIT_FAILURE);
	}

	if (op == BNE) {
		DictionaryEntry *codeLabel = getValueChainByDictionaryKey (codeLabels, (void *) token);

		if (codeLabel == NULL) {
			printf("Invalid code label cannot be resolved...\n");
			exit (EXIT_FAILURE);
		} else {
			target = *((int*)codeLabel -> value -> value);
		}
	}

	instruction = (Instruction *) malloc (sizeof(Instruction));

	instruction -> op = op;

	instruction -> rd = rd;
	instruction -> rs = rs;
	instruction -> rt = rt;

	instruction -> rsValue = rsValue;
	instruction -> rtValue = rtValue;

	instruction -> fd = fd;
	instruction -> fs = fs;
	instruction -> ft = ft;

	instruction -> fsValue = fsValue;
	instruction -> ftValue = ftValue;

	instruction -> immediate = immediate;

	instruction -> target = target;

	instruction -> address = instructionAddress;

	printf("Decoded %x:%s -> %s, rd=%d, rs=%d, rt=%d, fd=%d, fs=%d, ft=%d, immediate=%d, target=%d\n", instruction -> address, instruction_str,
		 getOpcodeString ((int) op), rd, rs, rt, fd, fs, ft, immediate, target);

    return instruction;
}



//Add instruction to reservation stations of type integer
int addInstruction2RSint(Dictionary *renameRegInt, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction, RegStatus **IntRegStatus){
    int counterUnit;
    int counterUnitResult;
    RSint* RS = (RSint*) malloc (sizeof(RSint));
    counterUnit = countDictionaryLen(resSta);
    counterUnitResult = countDictionaryLen(resStaResult);
    if (maxLenRS - counterUnit - counterUnitResult > 0){
        int DestROBnum = cpu -> reorderBuffer->tail;
        RegStatus *RegStatusEntry = IntRegStatus[instruction->rs];
        if (RegStatusEntry->busy == 1){
            int robNum = RegStatusEntry -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(renameRegInt, &(RegStatusEntry -> reorderNum));
                RS -> Vj = *((int *)renameRegIntEntry->value->value);
                RS -> Qj = -1;
                RS->isReady = 1;
            }
            else{
                RS -> Qj = robNum;
                RS->isReady = 0;
            }
        }else{
            RS -> Vj = cpu->integerRegisters[instruction->rs]->data;
            RS -> Qj = -1;
            RS->isReady = 1;
        }
        if (instruction->rt >= 0){
            RegStatus *RegStatusEntry = IntRegStatus[instruction->rt];
            if (RegStatusEntry->busy == 1){
                int robNum = RegStatusEntry -> reorderNum;
                if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                    DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(renameRegInt, &(RegStatusEntry -> reorderNum));
                    RS -> Vk = *((int *)renameRegIntEntry->value->value);
                    RS -> Qk = -1;
                    if(RS->isReady == 1)
                        RS->isReady = 1;
                }
                else{
                    RS -> Qk = robNum;
                    RS->isReady = 0;
                }
            }else{
                RS -> Vk = cpu->integerRegisters[instruction->rt]->data;
                RS -> Qk = -1;
                if(RS->isReady == 1)
                    RS->isReady = 1;
            }
        }else{
            RS -> Qk = -1;
        }
        //Append to reservation stations
        KeyRS *keyRS = (KeyRS*)malloc(sizeof(KeyRS));
        keyRS->reorderNum = DestROBnum;
        keyRS->progNum = 1;
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        RS->isExecuting = 0;
        addDictionaryEntry(resStaResult, keyRS, RS);
        //Update register status
        RegStatusEntry = IntRegStatus[instruction->rd];
        RegStatusEntry->busy = 1;
        RegStatusEntry->reorderNum = DestROBnum;
        //Add to renaming registers
        void *valuePtr = malloc(sizeof(int));
	    *((int*)valuePtr) = 0;
        removeDictionaryEntriesByKey(renameRegInt, &(RS->Dest));
        addDictionaryEntry(renameRegInt, &(RS->Dest), valuePtr);
        printf("Issued instruction %x: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        return 1;

    }else{
        cpu -> stallFullRS ++;
        printf("Stall during IssueUnit because reservation stations %s is full.\n", rsType);
        return 0;
    }
}

//Add instruction to reservation stations of type floating pointer
int addInstruction2RSfloat(Dictionary *renameRegFP, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction, RegStatus **FPRegStatus){
    int counterUnit;
    int counterUnitResult;
    RSfloat* RS = (RSfloat*) malloc (sizeof(RSfloat));
    counterUnit = countDictionaryLen(resSta);
    counterUnitResult = countDictionaryLen(resStaResult);
    if (maxLenRS - counterUnit - counterUnitResult > 0){
        int DestROBnum = cpu -> reorderBuffer->tail;
        RegStatus *RegStatusEntry = FPRegStatus[instruction->fs];
        if (RegStatusEntry->busy == 1){
            int robNum = RegStatusEntry -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                DictionaryEntry *renameRegFPEntry = getValueChainByDictionaryKey(renameRegFP, &(RegStatusEntry -> reorderNum));
                RS -> Vj = *((double *)renameRegFPEntry->value->value);
                RS -> Qj = -1;
                RS->isReady = 1;
            }
            else{
                RS -> Qj = robNum;
                RS->isReady = 0;
            }
        }else{
            RS -> Vj = cpu->floatingPointRegisters[instruction->fs]->data;
            RS -> Qj = -1;
            RS->isReady = 1;
        }
        if (instruction->ft >= 0){
            RegStatus *RegStatusEntry = FPRegStatus[instruction->ft];
            if (RegStatusEntry->busy == 1){
                int robNum = RegStatusEntry -> reorderNum;
                if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                    DictionaryEntry *renameRegFPEntry = getValueChainByDictionaryKey(renameRegFP, &(RegStatusEntry -> reorderNum));
                    RS -> Vk = *((double *)renameRegFPEntry->value->value);
                    RS -> Qk = -1;
                    if(RS->isReady == 1)
                        RS->isReady = 1;
                }
                else{
                    RS -> Qk = robNum;
                    RS->isReady = 0;
                }
            }else{
                RS -> Vk = cpu->floatingPointRegisters[instruction->ft]->data;
                RS -> Qk = -1;
                if(RS->isReady == 1)
                    RS->isReady = 1;
            }
        }else{
            RS -> Qk = -1;
        }
         //Append to reservation stations
        KeyRS *keyRS = (KeyRS*)malloc(sizeof(KeyRS));
        keyRS->reorderNum = DestROBnum;
        keyRS->progNum = 1;
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        RS->isExecuting = 0;
        addDictionaryEntry(resStaResult, keyRS, RS);
        //Update register status
        RegStatusEntry = FPRegStatus[instruction->fd];
        RegStatusEntry->busy = 1;
        RegStatusEntry->reorderNum = DestROBnum;
        //Add to renaming registers
        void *valuePtr = malloc(sizeof(double));
	    *((double*)valuePtr) = 0;
        removeDictionaryEntriesByKey(renameRegFP, &(RS->Dest));
        addDictionaryEntry(renameRegFP, &(RS->Dest), valuePtr);
        printf("Issued instruction %x: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        return 1;
    }else{
        cpu -> stallFullRS ++;
        printf("Stall during IssueUnit because reservation stations %s is full.\n", rsType);
        return 0;
    }
}

//Add instruction to reservation stations of type branch
int addInstruction2RSbranch(Dictionary *renameRegInt, Dictionary *resSta, Dictionary *resStaResult, char* rsType, int maxLenRS,
                            Instruction *instruction, RegStatus **IntRegStatus){
    int counterUnit;
    int counterUnitResult;
    RSint* RS = (RSint*) malloc (sizeof(RSint));
    counterUnit = countDictionaryLen(resSta);
    counterUnitResult = countDictionaryLen(resStaResult);
    if (maxLenRS - counterUnit - counterUnitResult > 0){
        int DestROBnum = cpu -> reorderBuffer->tail;
        RegStatus *RegStatusEntry = IntRegStatus[instruction->rs];
        if (RegStatusEntry->busy == 1){
            int robNum = RegStatusEntry -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(renameRegInt, &(RegStatusEntry -> reorderNum));
                RS -> Vj = *((int *)renameRegIntEntry->value->value);
                RS -> Qj = -1;
                RS->isReady = 1;
            }
            else{
                RS -> Qj = robNum;
                RS->isReady = 0;
            }
        }else{
            RS -> Vj = cpu->integerRegisters[instruction->rs]->data;
            RS -> Qj = -1;
            RS->isReady = 1;
        }
        if (instruction->rt >= 0){

            RegStatus *RegStatusEntry = IntRegStatus[instruction->rt];
            if (RegStatusEntry->busy == 1){
                int robNum = RegStatusEntry -> reorderNum;
                if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                    DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(renameRegInt, &(RegStatusEntry -> reorderNum));
                    RS -> Vk = *((int *)renameRegIntEntry->value->value);
                    RS -> Qk = -1;
                    if(RS->isReady == 1)
                        RS->isReady = 1;
                }
                else{
                    RS -> Qk = robNum;
                    RS->isReady = 0;
                }
            }else{
                RS -> Vk = cpu->integerRegisters[instruction->rt]->data;
                RS -> Qk = -1;
                if(RS->isReady == 1)
                    RS->isReady = 1;
            }
        }else{
            RS -> Qk = -1;
        }
        //Append to reservation stations
        KeyRS *keyRS = (KeyRS*)malloc(sizeof(KeyRS));
        keyRS->reorderNum = DestROBnum;
        keyRS->progNum = 1;
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        RS->isExecuting = 0;
        addDictionaryEntry(resStaResult, keyRS, RS);
        printf("Issued instruction %x: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        return 1;

    }else{
        cpu -> stallFullRS ++;
        printf("Stall during IssueUnit because reservation stations %s is full.\n", rsType);
        return 0;
    }
}

/**
 * Takes an instruction and adds it to the given reservation station.
 * @return int 1 if successfully added, 0 otherwise.
 */
int addLoadStore2Buffer(Dictionary *LOrSBuffer, Dictionary *LOrSBufferResult,
                         char *buffType, int maxLenBuff, Instruction *instruction) {
    int counterUnit;
    int counterUnitResult;
    RSmem* RS = (RSmem*) malloc (sizeof(RSmem));
    counterUnit = countDictionaryLen(LOrSBuffer);
    counterUnitResult = countDictionaryLen(LOrSBufferResult);
//    if (strcmp(buffType, "Load") == 0 && countDictionaryLen (cpu -> storeBuffer) != 0) {
//        printf("Stall Load during Issue Unit because of earlier store in queue.\n");
//        return 0;
//    }
    if (maxLenBuff - counterUnit - counterUnitResult > 0){
        RS -> isReady = 1; //Will be set to 0 later if necessary
        int DestROBnum = cpu -> reorderBuffer->tail;
        RegStatus *RegStatusEntry = cpu -> IntRegStatus[instruction->rs];
        if (RegStatusEntry -> busy == 1) {
            int robNum = RegStatusEntry -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(cpu -> renameRegInt, &(RegStatusEntry -> reorderNum));
                RS -> Vj = *((int *)renameRegIntEntry -> value -> value);
                RS -> Qj = -1;
            }
            else{
                RS -> Qj = robNum;
                RS->isReady = 0;
            }
        }else{
            RS -> Vj = cpu->integerRegisters[instruction->rs]->data;
            RS -> Qj = -1;
        }
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        if (strcmp(buffType, "Store") == 0) {
            if (instruction -> op == FSD) {
                RegStatusEntry = cpu -> FPRegStatus[instruction -> ft];
                if (RegStatusEntry -> busy == 1){
                    int robNum = RegStatusEntry -> reorderNum;
                    if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                        DictionaryEntry *renameRegFloatEntry = getValueChainByDictionaryKey (cpu -> renameRegFP, &(RegStatusEntry -> reorderNum));
                        RS -> fpVk = *((double *)renameRegFloatEntry -> value -> value);
                        RS -> Qk = -1;
                    }
                    else{
                        RS -> Qk = robNum;
                        RS->isReady = 0;
                    }
                }else{
                    RS -> fpVk = cpu -> floatingPointRegisters[instruction->ft] -> data;
                    RS -> Qk = -1;
                }
            } else {
                RegStatusEntry = cpu -> IntRegStatus[instruction -> rt];
                if (RegStatusEntry -> busy == 1){
                    int robNum = RegStatusEntry -> reorderNum;
                    if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                        DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey (cpu -> renameRegInt, &(RegStatusEntry -> reorderNum));
                        RS -> iVk = *((int *)renameRegIntEntry -> value -> value);
                        RS -> Qk = -1;
                    }
                    else{
                        RS -> Qk = robNum;
                        RS->isReady = 0;
                    }
                }else{
                    RS -> iVk = cpu -> integerRegisters[instruction->rt] -> data;
                    RS -> Qk = -1;
                }
            }
        }
        RS -> address = -1;
        RS->isExecuting = 0;
        KeyRS *keyRS = (KeyRS*)malloc(sizeof(KeyRS));
        keyRS->reorderNum = DestROBnum;
        keyRS->progNum = 1;
        addDictionaryEntry(LOrSBufferResult, keyRS, RS);
        //Add to renaming registers and update Register Status if load
        if (strcmp(buffType, "Load") == 0) {
            if (instruction -> op == FLD) {
                RegStatusEntry = cpu -> FPRegStatus[instruction -> ft];
                RegStatusEntry->busy = 1;
        		RegStatusEntry->reorderNum = DestROBnum;
                void *valuePtr = malloc(sizeof(double));
                *((double*)valuePtr) = 0;
                removeDictionaryEntriesByKey(cpu -> renameRegFP, &(RS -> Dest));
                addDictionaryEntry(cpu -> renameRegFP, &(RS -> Dest), valuePtr);
            } else {
                RegStatusEntry = cpu -> IntRegStatus[instruction -> rt];
                RegStatusEntry->busy = 1;
        		RegStatusEntry->reorderNum = DestROBnum;
                void *valuePtr = malloc(sizeof(int));
                *((int*)valuePtr) = 0;
                removeDictionaryEntriesByKey(cpu -> renameRegInt, &(RS->Dest));
                addDictionaryEntry(cpu -> renameRegInt, &(RS->Dest), valuePtr);
            }

        }
        printf("Issued instruction %x: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        return 1;
    } else {
        cpu -> stallFullRS ++;
        printf("Stall during IssueUnit because %s buffer is full.\n", buffType);
        return 0;
    }
}

//Check if renaming registers is full
int renameRegIsFull(Dictionary* renameReg, int d) {
    int counterRenameReg;
    counterRenameReg = countDictionaryLen(renameReg);
    if (counterRenameReg == numberRenameReg) {
        DictionaryEntry* renameRegDest = getValueChainByDictionaryKey(renameReg, &d);
        if (renameRegDest == NULL) {
            printf("Stall during IssueUnit because renaming register is full.\n");
            return 1;
        }
    }
    return 0;
}

//Issue an instruction
int issueInstruction(Instruction *instruction){
    if (isFullCircularQueue(cpu -> reorderBuffer)){
        cpu -> stallFullROB ++;
        printf("Stall during IssueUnit because reorder buffer is full.\n");
        return 0;
    }
    int issued = 0;
    int renameRegFull = 1;
    char* rsType;
    switch (instruction->op) {
        case ADDI:
        case ADD:
            renameRegFull = renameRegIsFull(cpu->renameRegInt, instruction -> rd);

            if (renameRegFull!=1){
                rsType = "INT";
                issued = addInstruction2RSint(cpu->renameRegInt, cpu->resStaInt, cpu->resStaIntResult, rsType, numberRSint,
                                            instruction, cpu->IntRegStatus);
            }
            break;
        case FADD:
        case FSUB:
            renameRegFull = renameRegIsFull(cpu->renameRegFP, instruction -> fd);
            if (renameRegFull!=1){
                char* rsType = "FPadd";
                issued = addInstruction2RSfloat(cpu->renameRegFP, cpu->resStaFPadd, cpu->resStaFPaddResult,
                                               rsType, numberRSfpAdd, instruction, cpu->FPRegStatus);
            }
            break;
        case FMUL:
            renameRegFull = renameRegIsFull(cpu->renameRegFP, instruction -> fd);
            if (renameRegFull!=1){
                rsType = "FPmult";
                issued = addInstruction2RSfloat(cpu->renameRegFP, cpu->resStaFPmult, cpu->resStaFPmultResult,
                                               rsType, numberRSfpMult, instruction, cpu->FPRegStatus);
            }
            break;
        case FDIV:
            renameRegFull = renameRegIsFull(cpu->renameRegFP, instruction -> fd);
            if (renameRegFull!=1){
                rsType = "FPdiv";
                issued = addInstruction2RSfloat(cpu->renameRegFP, cpu->resStaFPdiv, cpu->resStaFPdivResult,
                                               rsType, numberRSfpDiv, instruction, cpu->FPRegStatus);
            }
            break;
        case FLD:
        	renameRegFull = renameRegIsFull(cpu->renameRegFP, instruction -> ft);
            if (renameRegFull!=1){
                rsType = "Load";
                issued = addLoadStore2Buffer(cpu->loadBuffer, cpu->loadBufferResult,
                         rsType, numberBufferLoad, instruction);
            }
            break;

        case FSD:
            rsType = "Store";
            printf("Begin issuing store.\n");
            issued = addLoadStore2Buffer(cpu->storeBuffer, cpu->storeBufferResult,
                     rsType, numberBufferStore, instruction);
            break;
        case BNE:
            rsType = "BU";
            printf("Begin issuing %s .\n", rsType);
            issued = addInstruction2RSbranch(cpu->renameRegInt, cpu->resStaBU, cpu->resStaBUResult, rsType, numberRSbu,
                                             instruction, cpu->IntRegStatus);
            break;
        default:
            break;
    }
    if (issued == 1){
        enqueueCircular(cpu->reorderBuffer, InitializeROBEntry(instruction));
        return 1;
    }else
        return 0;
}


//Issue Unit, return the number of issued instructions in this cycle
int issueUnit(int NW){
    int i;
    int issued;
    Instruction *instruction;
    for(i=0; i<NW; i++){
        if((instruction = getHeadCircularQueue(cpu -> instructionQueue))!= NULL){
            issued = issueInstruction(instruction);
            if (issued == 0){
                return i;
            }
            else
                dequeueCircular(cpu -> instructionQueue);
        }
        else
            return i;
    }
    return i;
}


/**
 * Method that simulates pipelined Unit.
 * @return pointer to output instruction of the pipeline.
 */

CompletedInstruction *executePipelinedUnit (CircularQueue *pipeline) {
    CompletedInstruction *output = (CompletedInstruction *)dequeueCircular (pipeline);

    if (pipeline -> count < pipeline -> size - 1) {
        enqueueCircular (pipeline, NULL);
    }

    return output;
}


/**
 * Method that simulates FPdiv unit.
 * @return Pointer to the output instruction of the pipeline. NULL in case of bubbles or while executing.
 */

CompletedInstruction *executeFPDivUnit (CircularQueue *pipeline) {
    CompletedInstruction *output;

    output = dequeueCircular (pipeline);

    if (output != NULL) {
        cpu -> FPdivPipelineBusy = 0;
    }
    if (pipeline -> count < pipeline -> size - 1) {
        enqueueCircular (pipeline, NULL);
    }

    return output;
}

//Execution
CompletedInstruction **execute(int NB){
    Instruction *instruction = malloc(sizeof(Instruction));
    void *valuePtr = malloc(sizeof(double));
    void *addrPtr = malloc(sizeof(int));
  	DictionaryEntry *dataCacheElement;
  	CompletedInstruction *instructionAndResult = malloc(sizeof(CompletedInstruction));
    RSint *rsint;
    RSfloat *rsfloat;
    RSmem *rsmem;
    DictionaryValue *dictVal;
    //Array for instructions moving from Reservation Stations to execution units. Contains DictionaryValues.
    DictionaryEntry **instructionsToExec = malloc(sizeof(DictionaryEntry *)*8);
    //Array for outputs of Units. See Unit enum in DataTypes.h
    static CompletedInstruction *unitOutputs[7];
    int i, j;
    char *pipelineString;
    //variables for loads
    RSmem *RS;
    int instructionFoundOrBubble;
    DictionaryEntry *dictEntry;
    CircularQueue *buff;
    int loadStallROBNumber = -1; //needed to stall step 1 of load pipeline
    int moveOn;
    //Temp pipelines to hold changes during execution
    CompletedInstruction *INTPipelineTemp = NULL;
    CompletedInstruction *LoadPipelineTemp = NULL;
    CompletedInstruction *StorePipelineTemp = NULL;
    int storeFirst = 0; //1 if store first
    CompletedInstruction *FPaddPipelineTemp = NULL;
    CompletedInstruction *FPmultPipelineTemp = NULL;
    CompletedInstruction *FPdivPipelineTemp = NULL;
    CompletedInstruction *BUPipelineTemp = NULL;
    KeyRS *key = malloc(sizeof(KeyRS));


    dictEntry = (DictionaryEntry *)cpu -> resStaInt -> head;
    dictVal = checkReservationStation (dictEntry, 0);
    if (dictVal != NULL) {
        rsint = (RSint *)dictVal -> value;
        key -> reorderNum = rsint -> Dest;
        instructionsToExec[0] = getValueChainByDictionaryKey (cpu -> resStaInt, key);
    } else {
        instructionsToExec[0] = NULL;
    }
    instructionsToExec[1] = NULL;
    dictEntry = (DictionaryEntry *)cpu -> loadBuffer -> head;
    dictVal = checkReservationStation (dictEntry, 3);
    if (dictVal != NULL) {
        rsmem = (RSmem *)(dictVal -> value);
        key -> reorderNum = rsmem -> Dest;
        instructionsToExec[2] = getValueChainByDictionaryKey (cpu -> loadBuffer, key);
    } else {
        instructionsToExec[2] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> storeBuffer -> head;
    dictVal = checkReservationStation (dictEntry, 2);
    if (dictVal != NULL) {
        rsmem = (RSmem *)(dictVal -> value);
        key -> reorderNum = rsmem -> Dest;
        instructionsToExec[3] = getValueChainByDictionaryKey (cpu -> storeBuffer, key);
    } else {
        instructionsToExec[3] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaFPadd -> head;
    dictVal = checkReservationStation (dictEntry, 1);
    if (dictVal != NULL) {
        rsfloat = (RSfloat *)(dictVal -> value);
        key -> reorderNum = rsfloat -> Dest;
        instructionsToExec[4] = getValueChainByDictionaryKey (cpu -> resStaFPadd, key);
    } else {
        instructionsToExec[4] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaFPmult -> head;
    dictVal = checkReservationStation (dictEntry, 1);
    if (dictVal != NULL) {
        rsfloat = (RSfloat *)(dictVal -> value);
        key -> reorderNum = rsfloat -> Dest;
        instructionsToExec[5] = getValueChainByDictionaryKey (cpu -> resStaFPmult, key);
    } else {
        instructionsToExec[5] = NULL;
    }
    if (!(cpu -> FPdivPipelineBusy)) {
        dictEntry = (DictionaryEntry *)cpu -> resStaFPdiv -> head;
        dictVal = checkReservationStation (dictEntry, 1);
        if (dictVal != NULL) {
            rsfloat = (RSfloat *)(dictVal -> value);
            key -> reorderNum = rsfloat -> Dest;
            instructionsToExec[6] = getValueChainByDictionaryKey (cpu -> resStaFPdiv, key);
        } else {
            instructionsToExec[6] = NULL;
        }
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaBU -> head;
    dictVal = checkReservationStation (dictEntry, 0);
    if (dictVal != NULL) {
        rsint = (RSint *)(dictVal -> value);
        key -> reorderNum = rsint -> Dest;
        instructionsToExec[7] = getValueChainByDictionaryKey (cpu -> resStaBU, key);
    } else {
        instructionsToExec[7] = NULL;
    }

    for (i = 0; i < 8; i++) {
        if (instructionsToExec[i] == NULL) { //if reservation station did not provide instruction
            continue;
        }
		//printf("instruction exceuting has unit number %d\n", i);
        if (i < 2 || i > 6) {
            rsint = (RSint *)((DictionaryEntry *)instructionsToExec[i] -> value -> value);
            instruction = rsint -> instruction;
			printf("instruction exceuting has address %x and ROB %d\n", instruction ->address, rsint -> Dest);
        } else if (i == 2 || i == 3) {
            rsmem = (RSmem *)(((DictionaryEntry *)instructionsToExec[i]) -> value -> value);
            instruction = rsmem -> instruction;
			printf("instruction exceuting has address %x and ROB %d\n", instruction ->address, rsmem -> Dest);
        } else {
            rsfloat = (RSfloat *)(((DictionaryEntry *)instructionsToExec[i]) -> value -> value);
            instruction = rsfloat -> instruction;
			printf("instruction exceuting has address %x and ROB %d\n", instruction ->address, rsfloat -> Dest);
        }
        instructionAndResult -> instruction = instruction;

        switch (instruction->op) {
            case ADDI:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj + instruction->immediate;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case ADD:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj + rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case FADD:
                rsfloat -> isExecuting = 1;
                instructionAndResult -> fpResult = rsfloat -> Vj + rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPaddPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPaddPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "FPadd";
                printPipeline(instruction, pipelineString, 1);
                break;
            case FSUB:
                rsfloat -> isExecuting = 1;
                instructionAndResult -> fpResult = rsfloat -> Vj - rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPaddPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPaddPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "FPadd";
                printPipeline(instruction, pipelineString, 1);
                break;
            case FMUL:
                rsfloat -> isExecuting = 1;
                instructionAndResult -> fpResult = rsfloat -> Vj * rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPmultPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPmultPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "FPmult";
                printPipeline(instruction, pipelineString, 1);
                break;
            case FDIV:
                rsfloat -> isExecuting = 1;
                instructionAndResult -> fpResult = rsfloat -> Vj / rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPdivPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPdivPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                cpu -> FPdivPipelineBusy = 1;
                pipelineString = "FPdiv";
                printPipeline(instruction, pipelineString, 1);
                break;
            case FLD:
                //Two-step
                //First calculate address for earliest load that needs it
                if (rsmem -> isExecuting == 0){
                    rsmem -> isExecuting = 1;
                    rsmem -> address = rsmem -> Vj + instruction->immediate;
                    loadStallROBNumber = rsmem -> Dest;
                    pipelineString = "Load/Store";
                    printPipeline(instruction, pipelineString, 1);
                } else {
                    rsmem = NULL;
                }
                //Then check if load can execute (i.e. no stores ahead of it in ROB with same address)
                //Buff is reorder buffer but just for readability
                buff = cpu -> reorderBuffer;
                dictEntry = cpu -> loadBuffer -> head;
                instructionFoundOrBubble = 0;
                while (!instructionFoundOrBubble) { //1 for instruction, 2 for bubble
                    if (dictEntry == NULL) {
                        instructionFoundOrBubble = 2;
                    } else {
                        RS = (RSmem *)((DictionaryEntry *)dictEntry -> value -> value);
                        if (RS -> isReady && RS -> address != -1) {
                            for (j = 0; j < buff -> count && j < ((RS->Dest - buff->head)%buff->size) && j != -1; j++) {
                                if (((ROB *)(buff -> items[(buff -> head + j) % (buff->size)]))-> DestAddr == RS -> address) {
                                    j = -2; //break out of for loop
                                }
                            }
                            moveOn = 0;
                            if (rsmem != NULL && RS->Dest == rsmem -> Dest) {
                                moveOn = 1;
                            }
                            if (j != -1 && !moveOn && RS -> isExecuting != 2) {
                                instructionFoundOrBubble = 1;
                                rsmem = RS;
                            } else {
                                dictEntry = dictEntry -> next;
                            }
                        } else {
                            dictEntry = dictEntry -> next;
                        }
                    }
                }
                if (instructionFoundOrBubble == 1 && instructionsToExec[3] == NULL) {
                    instructionAndResult->instruction = rsmem -> instruction;
                    if (instructionAndResult->instruction->op != FLD) {
                        break;
                    }
                    rsmem -> isExecuting = 2;
                    * ((int*)addrPtr) = rsmem -> address;
                    dataCacheElement = getValueChainByDictionaryKey(dataCache, addrPtr);
                    if (dataCacheElement != NULL) {
                        valuePtr = dataCacheElement->value->value;

                    } else {
                        *((double *)valuePtr) = 0.0;
                    }
                    instructionAndResult -> fpResult = *((double*)valuePtr);
                    instructionAndResult -> ROB_number = rsmem -> Dest;
                    LoadPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                    memcpy(LoadPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                    pipelineString = "MEM";
                    printPipeline(instructionAndResult->instruction, pipelineString, 1);
                }
                break;
            case FSD:
                rsmem -> isExecuting = 1;
                rsmem -> address = rsmem -> Vj + instruction->immediate;
                instructionAndResult -> address = rsmem -> address;
                instructionAndResult -> fpResult = rsmem -> fpVk;
                instructionAndResult -> ROB_number = rsmem -> Dest;
                StorePipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(StorePipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                storeFirst = 1;
                pipelineString = "Load/Store";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BNE:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj != rsint -> Vk ? 0 : -1;
                instructionAndResult -> ROB_number = rsint -> Dest;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            default:
                break;
        }
    }

    //Take outputs from Units, but only as many as can be accepted by WriteBack Buffer
    int maxOutput = NB - (countDictionaryLen (cpu -> WriteBackBuffer));
	//printf("max outputs - %d\n", maxOutput);
    i = 0;
    int r;
    if (i < maxOutput) {
        unitOutputs[INT] = executePipelinedUnit (cpu -> INTPipeline);
        if (unitOutputs[INT] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[INT] -> ROB_number;
            r = unitOutputs[INT] -> ROB_number;
            if (isValidCircularIndex(cpu->reorderBuffer,r)) {
                removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[INT] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[INT] -> ROB_number), &(unitOutputs[INT] -> intResult));
            }
            removeDictionaryEntriesByKey (cpu -> resStaInt, key);
            pipelineString = "INT";
            printPipeline(unitOutputs[INT], pipelineString, 0);
        }
        if (INTPipelineTemp != NULL) {
            enqueueCircular (cpu -> INTPipeline, INTPipelineTemp);
        }
    } else {
        if (INTPipelineTemp != NULL) {
            key -> reorderNum = INTPipelineTemp -> ROB_number;
            RSint *stalled = (RSint *)(getValueChainByDictionaryKey(cpu -> resStaInt, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
  
    if (i < maxOutput) {
        unitOutputs[LS] = executePipelinedUnit (cpu -> LoadStorePipeline);
        if (unitOutputs[LS] != NULL) {
            printf("execute---unitoutputs of LS");
            i++;
            key -> reorderNum = unitOutputs[LS] -> ROB_number;
            key->progNum = 1;
            r = unitOutputs[LS] -> ROB_number;
            if (unitOutputs[LS] -> instruction -> op == FLD) {
                if (isValidCircularIndex(cpu->reorderBuffer,r)) {
                    removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[LS] -> ROB_number));
                    addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> fpResult));
                }
                removeDictionaryEntriesByKey (cpu -> loadBuffer, key);
            } else if (unitOutputs[LS] -> instruction -> op == FSD) {
                if (isValidCircularIndex(cpu->reorderBuffer,r)) {
                    removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[LS] -> ROB_number));
                    addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> fpResult));
                }
                removeDictionaryEntriesByKey (cpu -> storeBuffer, key);
            }
            pipelineString = "Load/Store";
            printPipeline(unitOutputs[LS], pipelineString, 0);
        }
        if (!storeFirst && LoadPipelineTemp != NULL) {
            enqueueCircular (cpu -> LoadStorePipeline, LoadPipelineTemp);
        } else if (StorePipelineTemp != NULL) {
            enqueueCircular (cpu -> LoadStorePipeline, StorePipelineTemp);
        }
    } else {
        if (loadStallROBNumber != -1) {
            key -> reorderNum = loadStallROBNumber;
            RSmem *stalled = (RSmem *)(getValueChainByDictionaryKey(cpu -> loadBuffer, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
        if (LoadPipelineTemp != NULL) {
            key -> reorderNum = LoadPipelineTemp -> ROB_number;
            RSmem *stalled = (RSmem *)(getValueChainByDictionaryKey(cpu -> loadBuffer, key) -> value -> value);
            stalled -> isExecuting = 1;
        }
        if (StorePipelineTemp != NULL) {
            key -> reorderNum = StorePipelineTemp -> ROB_number;
            RSmem *stalled = (RSmem *)(getValueChainByDictionaryKey(cpu -> storeBuffer, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[FPadd] = executePipelinedUnit (cpu -> FPaddPipeline);
        if (unitOutputs[FPadd] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[FPadd] -> ROB_number;
            r = unitOutputs[FPadd] -> ROB_number;
            if (isValidCircularIndex(cpu->reorderBuffer,r)) {
                removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[FPadd] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[FPadd] -> ROB_number), &(unitOutputs[FPadd] -> fpResult));
            }
            removeDictionaryEntriesByKey (cpu -> resStaFPadd, key);
            pipelineString = "FPadd";
            printPipeline(unitOutputs[FPadd], pipelineString, 0);
        }
        if (FPaddPipelineTemp != NULL) {
            enqueueCircular (cpu -> FPaddPipeline, FPaddPipelineTemp);
        }
    } else {
        if (FPaddPipelineTemp != NULL) {
            key -> reorderNum = FPaddPipelineTemp -> ROB_number;
            RSfloat *stalled = (RSfloat *)(getValueChainByDictionaryKey(cpu -> resStaFPadd, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[FPmult] = executePipelinedUnit (cpu -> FPmultPipeline);
        if (unitOutputs[FPmult] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[FPmult] -> ROB_number;
            r = unitOutputs[FPmult] -> ROB_number;
            if (isValidCircularIndex(cpu->reorderBuffer,r)) {
                removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[FPmult] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[FPmult] -> ROB_number), &(unitOutputs[FPmult] -> fpResult));
            }
            removeDictionaryEntriesByKey (cpu -> resStaFPmult, key);
            pipelineString = "FPmult";
            printPipeline(unitOutputs[FPmult], pipelineString, 0);
        }
        if (FPmultPipelineTemp != NULL) {
            enqueueCircular (cpu -> FPmultPipeline, FPmultPipelineTemp);
        }
    } else {
        if (FPmultPipelineTemp != NULL) {
            key -> reorderNum = FPmultPipelineTemp -> ROB_number;
            RSfloat *stalled = (RSfloat *)(getValueChainByDictionaryKey(cpu -> resStaFPmult, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[FPdiv] = executeFPDivUnit (cpu -> FPdivPipeline);
        if (unitOutputs[FPdiv] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[FPdiv] -> ROB_number;
            r = unitOutputs[FPdiv] -> ROB_number;
            if (isValidCircularIndex(cpu->reorderBuffer,r)) {
                removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[FPdiv] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[FPdiv] -> ROB_number), &(unitOutputs[FPdiv] -> fpResult));
            }
            removeDictionaryEntriesByKey (cpu -> resStaFPdiv, key);
            pipelineString = "FPdiv";
            printPipeline(unitOutputs[FPdiv], pipelineString, 0);
        }
        if (FPdivPipelineTemp != NULL) {
            enqueueCircular (cpu -> FPdivPipeline, FPdivPipelineTemp);
        }
    } else {
        if (FPdivPipelineTemp != NULL) {
            key -> reorderNum = FPdivPipelineTemp -> ROB_number;
            RSfloat *stalled = (RSfloat *)(getValueChainByDictionaryKey(cpu -> resStaFPdiv, key) -> value -> value);
            stalled -> isExecuting = 0;
            cpu -> FPdivPipelineBusy = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[BU] = executePipelinedUnit (cpu -> BUPipeline);
        if (unitOutputs[BU] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[BU] -> ROB_number;
            r = unitOutputs[BU] -> ROB_number;
            if (isValidCircularIndex(cpu->reorderBuffer,r)) {
                removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[BU] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[BU] -> ROB_number), &(unitOutputs[BU] -> intResult));
            }
            removeDictionaryEntriesByKey (cpu -> resStaBU, key);
            branchHelper (unitOutputs[BU]);
            pipelineString = "BU";
            printPipeline(unitOutputs[BU], pipelineString, 0);
        }
        if (BUPipelineTemp != NULL) {
            enqueueCircular (cpu -> BUPipeline, BUPipelineTemp);
        }
    } else {
        if (BUPipelineTemp != NULL) {
            key -> reorderNum = BUPipelineTemp -> ROB_number;
            RSint *stalled = (RSint *)(getValueChainByDictionaryKey(cpu -> resStaBU, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }

    return unitOutputs;

}



//Initialize ROB struct
ROB * InitializeROBEntry(Instruction * instructionP)
{

	ROB * ROBEntry ;
	OpCode op;
	
	ROBEntry = (ROB*) malloc (sizeof(ROB));	
	ROBEntry -> isStore = 0;
	ROBEntry -> DestRenameReg = cpu->reorderBuffer -> tail;
	ROBEntry -> isBranch = 0 ;
	ROBEntry -> isCorrectPredict = 1;
	ROBEntry -> DestAddr = 0;
	op = instructionP -> op;
	
	  switch (op) {
        case ADDI:
            ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case ADD:
            ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case FADD:
		ROBEntry->DestReg = instructionP->fd;
			ROBEntry -> isINT = 0;
            break;
        case FSUB:
			ROBEntry->DestReg = instructionP->fd;
			ROBEntry -> isINT = 0;
            break;
        case FMUL:
			ROBEntry->DestReg = instructionP->fd;
			ROBEntry -> isINT = 0;
            break;
        case FDIV:
			ROBEntry->DestReg = instructionP->fd;
			ROBEntry -> isINT = 0;
            break;
        case FLD:
		ROBEntry->DestReg = instructionP->ft;
			ROBEntry -> isINT = 0;
            break;
        case FSD:
			ROBEntry->DestReg = -1;
			ROBEntry -> isINT = 0;
			ROBEntry -> isStore = 1;
			ROBEntry -> DestAddr = 0;

		
            break;
        case BNE:

				ROBEntry->DestReg = -1;
				ROBEntry -> isINT = 0;
				ROBEntry -> isBranch = 1;
            break;
        default:
            break;
    }
	
	ROBEntry->instruction = instructionP;
	ROBEntry->state = "I";
	ROBEntry->isReady = 0;
	ROBEntry->isAfterBranch = cpu->isAfterBranch;
	return ROBEntry;
}



//commit instructions or flush ROB
int Commit(int NC, int NR, int returncount)
{
	// commit instructions from ROB
	ROB * ROBEntry;
	RegStatus *RegStatusEntry;
	int robnum;
	int rcount = returncount;
		ROBEntry = cpu -> reorderBuffer -> items[cpu->reorderBuffer ->head];
//		while(ROBEntry != NULL && NC != 0)
		while (cpu->reorderBuffer->count != 0 && NC != 0)
		{
		        void *valuePtr = malloc(sizeof(double));
				//printf("Checking instruction %d for commiting\n", ROBEntry -> instruction -> address);
				if((strcmp(ROBEntry -> state, "W") == 0) && ROBEntry -> isReady == 1)
				{
				    robnum = cpu->reorderBuffer ->head;
					ROBEntry = dequeueCircular(cpu -> reorderBuffer);
					//printf("Checked instruction %d for commiting\n", ROBEntry -> instruction -> address);
					if(ROBEntry -> isINT == 1 && ROBEntry -> isStore == 0 && ROBEntry -> isBranch == 0){
							int DestRenameReg, DestVal, DestReg;
							DestRenameReg = ROBEntry -> DestRenameReg;
							DestReg = ROBEntry -> DestReg;
							DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegInt, &DestRenameReg);
							DestVal = *((int *)Current -> value -> value);
							cpu -> integerRegisters [DestReg] -> data = DestVal;
							RegStatusEntry = cpu -> IntRegStatus[DestReg];
							printf("reg status rob muber - %d\t Commit instruction ROB number - %d\n",RegStatusEntry -> reorderNum, robnum);
							if(RegStatusEntry -> reorderNum == robnum){
								RegStatusEntry->busy = 0;
							}
							removeDictionaryEntriesByKey(cpu -> renameRegInt, &DestRenameReg);
							printf("Committed instruction %d in integer register number %d with value %d \n", ROBEntry -> instruction -> address, DestReg, DestVal);
							NC --;
							rcount++;
					}
					else if(ROBEntry -> isINT == 0 && ROBEntry -> isStore == 0 && ROBEntry -> isBranch == 0){
						int DestRenameReg, DestReg; float DestVal;
							DestRenameReg = ROBEntry -> DestRenameReg;
							DestReg = ROBEntry -> DestReg;
							DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegFP , &DestRenameReg);
							DestVal = *((double *)Current -> value -> value);
							cpu -> floatingPointRegisters [DestReg] -> data = DestVal;
							RegStatusEntry = cpu -> FPRegStatus[DestReg];
							printf("reg status rob muber - %d\t Commit instruction ROB number - %d\n",RegStatusEntry -> reorderNum, robnum);
							if(RegStatusEntry -> reorderNum == robnum){
								RegStatusEntry->busy = 0;
							}
							removeDictionaryEntriesByKey(cpu -> renameRegFP, &DestRenameReg);
							printf("Committed instruction %d in floating point register number %d with value %f\n", ROBEntry -> instruction -> address, DestReg, DestVal);
							NC --;
							rcount++;
					}
					else if(ROBEntry -> isStore == 1)
					{
							double DestVal; int DestRenameReg;
							DestRenameReg = ROBEntry -> DestRenameReg;
							DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegFP, &DestRenameReg);
							DestVal = *((double *)Current -> value -> value);
							removeDictionaryEntriesByKey (dataCache, &(ROBEntry -> DestAddr));
							*((double*)valuePtr) = (double) DestVal; // value from rename register ;
							addDictionaryEntry (dataCache, &(ROBEntry -> DestAddr), valuePtr);
							removeDictionaryEntriesByKey(cpu -> renameRegFP, &DestRenameReg);
							printf("Committed instruction FSD %x in memory address %x \n", ROBEntry -> instruction -> address, ROBEntry -> DestAddr);							
							//DestVal = 0;
							NC --;
							rcount++;
					}
					else{
						//Branch
						if(ROBEntry ->isBranch == 1 ){
							removeDictionaryEntriesByKey(cpu -> renameRegInt, &(robnum));
							int i = 0;
							ROB *ROBentrySecond = cpu -> reorderBuffer-> items[(cpu->reorderBuffer->head + i)%cpu->reorderBuffer->size];
							int robnum2 = (cpu->reorderBuffer->head + i)%cpu->reorderBuffer->size;
						if(ROBEntry -> isCorrectPredict == 0){
							// move head to isafterbranch == 0
							
							while(ROBentrySecond != NULL){
								ROB *ROBentrySecond = cpu -> reorderBuffer-> items[(cpu->reorderBuffer->head + i)%cpu->reorderBuffer->size];
								robnum2 = (cpu->reorderBuffer->head + i)%cpu->reorderBuffer->size;
								printf("instruction being flushed has rob num %d\n", robnum2);
								if(ROBentrySecond != NULL){
									
									if(ROBentrySecond -> isAfterBranch == 0)
									{
										cpu -> reorderBuffer -> head = (cpu->reorderBuffer->head + i)%cpu->reorderBuffer->size;
										cpu -> reorderBuffer -> count = cpu -> reorderBuffer -> count - i;
										printf("Branch mispredicted so flushed ROB.\n");
										break;
									}
									else{
										KeyRS *robnumkey = (KeyRS *)malloc(sizeof(KeyRS));
										robnumkey -> reorderNum = robnum2;
										robnumkey -> progNum = 1;
										if(cpu -> WriteBackBuffer != NULL){
											removeDictionaryEntriesByKey(cpu -> WriteBackBuffer, robnumkey); 
										}
										if(ROBentrySecond -> isINT == 1){
											removeDictionaryEntriesByKey(cpu -> renameRegInt, &(robnum2)); 
											
										}
										else{
											removeDictionaryEntriesByKey(cpu -> renameRegFP, &(robnum2)); 
										}

										if(ROBentrySecond -> isINT == 1){
											int j = 0;
											for (j = 0; j < numberOfIntRegisters; j++) {
												if(cpu -> IntRegStatus [j] -> reorderNum == robnum2)
												{
													cpu -> IntRegStatus [j] -> busy = 0;
												}
											}
										}
										else 
										{
											int k = 0;
											for (k = 0; k < numberOfFPRegisters; k++) {
												if(cpu -> FPRegStatus [k] -> reorderNum == robnum2)
												{
													cpu -> FPRegStatus [k] -> busy = 0;
												}
											}
										}
										//go to next
									}
									
									if(robnum2 == cpu -> reorderBuffer -> tail){
										printf("ROB is empty now\n");
										//cpu -> reorderBuffer -> head = cpu -> reorderBuffer -> tail;
										cpu -> reorderBuffer = createCircularQueue(NR);
										break;
									}
									i++;
								}
								else{
									cpu -> reorderBuffer -> head = (cpu->reorderBuffer->head + i)%cpu->reorderBuffer->size;
									cpu -> reorderBuffer -> count = cpu -> reorderBuffer -> count - i;
									printf("Branch mispredicted so flushed ROB.\n");
									break;
								}
							}
						}
						printf("Committed branch instruction %x\n", ROBEntry -> instruction -> address);
						
						}
						NC--;
						rcount++;
					}
				}
		else{
		
			break;
		}
		
		ROBEntry = cpu -> reorderBuffer-> items[cpu->reorderBuffer->head];
		}
	return rcount;
}





// update RES with output from execution
void updateOutputRES(CompletedInstruction *instruction){
	printf("Updating reservation stations with ROB - %d\n", instruction -> ROB_number);
	int robnumber = instruction -> ROB_number;
	DictionaryEntry *current;
	RSint *RSint;
	RSfloat *RSfloat;
	RSmem *RSmem;
	 switch (instruction -> instruction -> op) {
        case ADDI:
        case ADD:
		case BNE:
			for (current = cpu -> resStaBU -> head; current != NULL; current = current -> next){
				RSint = current -> value -> value;
					if (RSint -> isReady == 0){
						if(RSint -> Qj == robnumber){
								RSint -> Vj = instruction -> intResult;
								RSint -> Qj = -1;
							}
						if(RSint -> Qk == robnumber){
							RSint -> Vk = instruction -> intResult;
							RSint -> Qk = -1;
						}

						if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
							RSint -> isReady = 1;
						}
					}
			}
            for (current = cpu -> resStaInt -> head; current != NULL; current = current -> next){
                RSint = current -> value -> value;
					if (RSint -> isReady == 0){
						if(RSint -> Qj == robnumber){
							RSint -> Vj = instruction -> intResult;
							RSint -> Qj = -1;
						}
						if(RSint -> Qk == robnumber){
							RSint -> Vk = instruction -> intResult;
							 RSint -> Qk = -1;
						}
						 if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
								RSint -> isReady = 1;
						}
					}
			}
			for (current = cpu -> storeBuffer -> head; current != NULL; current = current -> next){
					RSmem = current -> value -> value;
						if (RSmem -> isReady == 0){
							if(RSmem -> Qj == robnumber){
								RSmem -> Vj = instruction -> intResult;
								RSmem -> Qj = -1;						
							}
							if(RSmem -> Qk == robnumber){
								RSmem -> iVk = instruction -> intResult;
								RSmem -> Qk = -1;
							}
							if ((RSmem -> Qj == -1) && (RSmem -> Qk == -1)){
								RSmem -> isReady = 1;
							}
						}
			}
            for (current = cpu -> loadBuffer -> head; current != NULL; current = current -> next){
                RSmem = current -> value -> value;
					if (RSmem -> isReady == 0){
						if(RSmem -> Qj == robnumber){
							RSmem -> Vj = instruction -> intResult;
							RSmem -> Qj = -1;		
						}
						if (RSmem -> Qj == -1){
									RSmem -> isReady = 1;
						}
					}
            }
            break;
        case FADD:
        case FSUB:
		case FMUL:
		case FDIV:
		case FSD:
		case FLD:
				for (current = cpu -> resStaFPadd -> head; current != NULL; current = current -> next){
					RSfloat = current -> value -> value;
						if (RSfloat -> isReady == 0){
							if(RSfloat -> Qj == robnumber){
								RSfloat -> Vj = instruction -> fpResult;	
								RSfloat -> Qj = -1;
							}
							if(RSfloat -> Qk == robnumber){
								RSfloat -> Vk = instruction -> fpResult;
								RSfloat -> Qk = -1;
							}
							if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
									RSfloat -> isReady = 1;
							}
						}
				}
				for (current = cpu -> loadBuffer -> head; current != NULL; current = current -> next){
				RSmem = current -> value -> value;
				 if (RSmem -> isReady == 0){
					if(RSmem -> Qj == robnumber){
								RSmem -> Vj = instruction -> intResult;
								RSmem -> Qj = -1;							
					}
							if (RSmem -> Qj == -1){
									RSmem -> isReady = 1;
						    }
				 }
				}
				for (current = cpu -> resStaFPmult -> head; current != NULL; current = current -> next){
				RSfloat = current -> value -> value;
					if (RSfloat -> isReady == 0){
						if(RSfloat -> Qj == robnumber){
							RSfloat -> Vj = instruction -> fpResult;
							RSfloat -> Qj = -1;						
						}
						if(RSfloat -> Qk == robnumber){
							RSfloat -> Vk = instruction -> fpResult;
							RSfloat -> Qk = -1;
							
						}
						if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
									RSfloat -> isReady = 1;
						}
					}
				}  
			for (current = cpu -> resStaFPdiv -> head; current != NULL; current = current -> next){
					RSfloat = current -> value -> value;
						if (RSfloat -> isReady == 0){
							if(RSfloat -> Qj == robnumber){
								RSfloat -> Vj = instruction -> fpResult;
								RSfloat -> Qj = -1;						
							}
							if(RSfloat -> Qk == robnumber){
								RSfloat -> Vk = instruction -> fpResult;
								RSfloat -> Qk = -1;
							}
							if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
										RSfloat -> isReady = 1;
								}
						}
				}
			
            for (current = cpu -> storeBuffer -> head; current != NULL; current = current -> next){
					RSmem = current -> value -> value;
					 if (RSmem -> isReady == 0){
						if(RSmem -> Qj == robnumber){
							RSmem -> fpVk = instruction -> fpResult;	
							RSmem -> Qj = -1;
						}
						if(RSmem -> Qk == robnumber){
							RSmem -> fpVk = instruction -> fpResult;
							RSmem -> Qk = -1;
						}
						if ((RSmem -> Qj == -1) && (RSmem -> Qk == -1)){
									RSmem -> isReady = 1;
							}
					 }
				}
            break;
     
        default:
            break;

	 }
}



void updateOutputRESresult(CompletedInstruction *instruction) {
    int robnumber = instruction -> ROB_number;
	RSint *RSint;
	RSfloat *RSfloat;
	RSmem *RSmem;
	DictionaryEntry *tempEntry;
	switch (instruction -> instruction -> op) {
        case ADDI:
        case ADD:
		case BNE:
		    for (tempEntry = cpu -> resStaBUResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL){
                    RSint = tempEntry -> value -> value;
						 if (RSint -> isReady == 0){
						if(RSint -> Qj == robnumber){
								RSint -> Vj = instruction -> intResult;
								RSint -> Qj = -1;
							}
						if(RSint -> Qk == robnumber){
							RSint -> Vk = instruction -> intResult;
							RSint -> Qk = -1;
						}

						if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
							RSint -> isReady = 1;
						}
					}
                }
            }
            for (tempEntry = cpu -> resStaIntResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL){
                    RSint = tempEntry -> value -> value;
						 if (RSint -> isReady == 0){
                    if(RSint -> Qj == robnumber){
                            RSint -> Vj = instruction -> intResult;
                            RSint -> Qj = -1;
                        }
                    if(RSint -> Qk == robnumber){
                        RSint -> Vk = instruction -> intResult;
                        RSint -> Qk = -1;
                    }

                    if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
                        RSint -> isReady = 1;
                    }
						 }
                }
            }
            for (tempEntry = cpu -> storeBufferResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSmem = tempEntry -> value -> value;
						 if (RSmem -> isReady == 0){
                    if(RSmem -> Qj == robnumber){
                        RSmem -> Vj = instruction -> intResult;
                        RSmem -> Qj = -1;
                    }
                    if(RSmem -> Qk == robnumber){
                        RSmem -> iVk = instruction -> intResult;
                        RSmem -> Qk = -1;
                    }
                    if ((RSmem -> Qj == -1) && (RSmem -> Qk == -1)){
                                RSmem -> isReady = 1;
                    }
						 }
				}
            }
            for (tempEntry = cpu -> loadBufferResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSmem = tempEntry -> value -> value;
						 if (RSmem -> isReady == 0){
                    if(RSmem -> Qj == robnumber){
                        RSmem -> Vj = instruction -> intResult;
                        RSmem -> Qj = -1;
                    }
                    if ((RSmem -> Qj == -1)){
                        RSmem -> isReady = 1;
                    }
					}
                }
            }
            break;
        case FADD:
        case FSUB:
		case FMUL:
		case FDIV:
		case FSD:
		case FLD:
		    for (tempEntry = cpu -> resStaFPaddResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSfloat = tempEntry -> value -> value;
							if (RSfloat -> isReady == 0){
                    if(RSfloat -> Qj == robnumber){
                        RSfloat -> Vj = instruction -> fpResult;
                        RSfloat -> Qj = -1;
                    }
                    if(RSfloat -> Qk == robnumber){
                        RSfloat -> Vk = instruction -> fpResult;
                        RSfloat -> Qk = -1;
                    }
                    if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
                                RSfloat -> isReady = 1;
                        }
					}
                }
            }
            for (tempEntry = cpu -> loadBufferResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSmem = tempEntry -> value -> value;
							 if (RSmem -> isReady == 0){
                    if(RSmem -> Qj == robnumber){
                        RSmem -> Vj = instruction -> intResult;
                        RSmem -> Qj = -1;
                    }
                    if ((RSmem -> Qj == -1)){
                                RSmem -> isReady = 1;
                    }
					}
                }
            }
            for (tempEntry = cpu -> resStaFPmultResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSfloat = tempEntry -> value -> value;
						if (RSfloat -> isReady == 0){
                    if(RSfloat -> Qj == robnumber){
                        RSfloat -> Vj = instruction -> fpResult;
                        RSfloat -> Qj = -1;
                    }
                    if(RSfloat -> Qk == robnumber){
                        RSfloat -> Vk = instruction -> fpResult;
                        RSfloat -> Qk = -1;
                    }
                    if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
                                RSfloat -> isReady = 1;
                        }
					}
                }
            }
            for (tempEntry = cpu -> resStaFPdivResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSfloat = tempEntry -> value -> value;
							if (RSfloat -> isReady == 0){
                    if(RSfloat -> Qj == robnumber){
                        RSfloat -> Vj = instruction -> fpResult;
                        RSfloat -> Qj = -1;
                    }
                    if(RSfloat -> Qk == robnumber){
                        RSfloat -> Vk = instruction -> fpResult;
                        RSfloat -> Qk = -1;
                    }
                    if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
                                RSfloat -> isReady = 1;
                        }
					}
                }
            }
            for (tempEntry = cpu -> storeBufferResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSmem = tempEntry -> value -> value;
						 if (RSmem -> isReady == 0){
                    if(RSmem -> Qj == robnumber){
                        RSmem -> Vj = instruction -> intResult;
                        RSmem -> Qj = -1;
                    }
                    if(RSmem -> Qk == robnumber){
                        RSmem -> fpVk = instruction -> fpResult;
                        RSmem -> Qk = -1;
                    }
                    if ((RSmem -> Qj == -1) && (RSmem -> Qk == -1)){
                                RSmem -> isReady = 1;
                    }
					}
                }
            }
            break;
        default:
            break;
	 }
}





// insert ouput results into write back buffer and update rename register
void insertintoWriteBackBuffer(int NB)
{
	//int	*ROB_number = (int*) malloc(sizeof(int));
	KeyRS *ROB_number = (KeyRS *) malloc(sizeof(KeyRS));
	CompletedInstruction *instruction;
	CompletedInstruction **unitOutputs;
	unitOutputs = execute(NB);
	printf("Execution Complete ---------------\n");
	printf("Write Back Start ---------------\n");
	if(cpu -> WriteBackBuffer == NULL)
	{
		cpu -> WriteBackBuffer = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
	}
	if(unitOutputs != NULL){
		if(unitOutputs[INT] != NULL){
			instruction = unitOutputs[INT];
			//*ROB_number = instruction->ROB_number;
			ROB_number->reorderNum = instruction->ROB_number;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			int* intresult = &(instruction -> intResult);
			printf ("Added instruction %x to WriteBack Buffer with ROB no - %d  Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number, *intresult);	
		}
		if(unitOutputs[FPadd] != NULL){
			instruction = unitOutputs[FPadd];
			ROB_number->reorderNum = instruction->ROB_number;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			double *fpresult = &(instruction -> fpResult);
			printf ("Added instruction %x to WriteBack Buffer with ROB no - %d  Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);	
		}
		if(unitOutputs[FPmult]!= NULL){
			instruction = unitOutputs[FPmult];
			ROB_number->reorderNum = instruction->ROB_number;
			double *fpresult = &(instruction -> fpResult);
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			printf ("Added instruction %x to WriteBack Buffer with ROB no - %d  Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number,*fpresult);	
		}
		if(unitOutputs[FPdiv] != NULL){
			instruction = unitOutputs[FPdiv];
			ROB_number->reorderNum = instruction->ROB_number;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			double *fpresult = &(instruction -> fpResult);
			printf ("Added instruction %x to WriteBack Buffer with ROB no - %d  Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);	
		}
		if(unitOutputs[BU] != NULL){
				
			instruction = unitOutputs[BU];
			ROB_number->reorderNum = instruction->ROB_number;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			int* intresult = &(instruction -> intResult);
			printf ("Added instruction %x to WriteBack Buffer with ROB no - %d  Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number,  *intresult);	
		}
		if(unitOutputs[LS] != NULL){
			instruction = unitOutputs[LS];
			OpCode op = instruction -> instruction -> op;
            if((strcmp(getOpcodeString (op), "fsd") == 0))
			{
				ROB_number->reorderNum = instruction->ROB_number;
				addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
				double *fpresult = &(instruction -> fpResult);
				printf ("Added instruction %x to WriteBack Buffer with ROB no - %d  Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);
			}
            if((strcmp(getOpcodeString (op), "fld") == 0))
			{
				ROB_number->reorderNum = instruction->ROB_number;
				addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
				double *fpresult = &(instruction -> fpResult);
				printf ("Added instruction %x to WriteBack Buffer with ROB no - %d  Outputt - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);
			}
		}
	}
	//printf("Completed write back\n");
}



// returns count of instructions which can be committed
int commitInstuctionCount(){
	ROB *ROBentry;
	int count = 0 , i = 0;
	if (cpu -> reorderBuffer == NULL){
		return 0;
	}
	else {
		ROBentry = cpu-> reorderBuffer -> items[cpu->reorderBuffer -> head];
		//while(ROBentry != NULL){
		while (i < cpu->reorderBuffer->count) {
			if((strcmp(ROBentry -> state, "W") == 0) && ROBentry -> isReady == 1)
			{
				count ++;
			}
			else{
				break;
			}
			i++;
			ROBentry = cpu->reorderBuffer -> items[(cpu->reorderBuffer -> head + i)%cpu->reorderBuffer->size];
		}
	}
	return count;
}



// write back to RES
int writeBackUnit(int NB, int returncount){
	// update outputs to ROB and Reservation stations
	//int i;
	DictionaryEntry *current;
	CompletedInstruction *instruction;
	ROB *ROBentry;
	KeyRS *key = (KeyRS *)malloc(sizeof(KeyRS));
	int rcount = returncount;
	for (current = cpu -> WriteBackBuffer -> head; current != NULL && NB >= 0; current = current -> next){
      
			instruction = (CompletedInstruction *)current -> value -> value;
				int j = 0;
				key -> reorderNum = instruction ->ROB_number;
				if (cpu -> reorderBuffer != NULL){
					ROBentry = cpu-> reorderBuffer -> items[cpu->reorderBuffer -> head];
					//while(ROBentry != NULL){
					while (j < cpu->reorderBuffer->count) {
						if(ROBentry -> DestRenameReg == instruction -> ROB_number){
							ROBentry -> state = "W";
							ROBentry -> isReady = 1;
							if(ROBentry -> isBranch == 1){
								ROBentry -> isCorrectPredict = instruction -> isCorrectPredict;
								//printf("Branch instruction %d is correctly predicted %d", instruction -> address, instruction -> isCorrectPredict);
							}
							if(ROBentry -> isStore == 1){
								ROBentry -> DestAddr = instruction -> address;
								//printf("store address is updated in ROB.\n");
							}
							printf("instruction %x  with ROB_number - %d updated in reorder buffer \n", ROBentry -> instruction -> address, instruction -> ROB_number);
						}
						
						j++;
						ROBentry = cpu->reorderBuffer -> items[(cpu->reorderBuffer -> head + j)%cpu->reorderBuffer->size];
					}
				}
			updateOutputRES(instruction);
			updateOutputRESresult(instruction);
			removeDictionaryEntriesByKey(cpu -> WriteBackBuffer, key);
			rcount++;
		}
	return rcount;
}

	
	


// manage commit and write back
int CommitUnit(int NB, int NR)
{
	int wb_count, commit_count, returncount = 0;
	wb_count = countDictionaryLen(cpu -> WriteBackBuffer);
	commit_count = commitInstuctionCount();
	
	//printf("Write Back and Commit---------------\n");
	printf("commit count - %d, wb count - %d\n", commit_count, wb_count);
	if(wb_count == 0 && commit_count == 0){
		printf("No instruction in Writeback and in ROB for Commit(Program 1).\n");
	}
	else if(wb_count == 0 || commit_count >= NB)
	{
		returncount = Commit(NB, NR, returncount);
		cpu->commitCounter += returncount;
	}
	else if(commit_count == 0 || wb_count >= NB ){
		returncount = writeBackUnit(NB, returncount);
	}
	else{
			returncount = Commit(commit_count, NR, returncount);
			cpu->commitCounter += returncount;
			returncount = writeBackUnit(NB - commit_count, returncount);
		}
		
		printf("Count on CDB for program 1 is %d\n", returncount);
		printf("------------------------------------------------------\n");
		return returncount;
		// divide NB
}

/**
 * Method that simulates the looping cycle-wise
 * @return: When the simulator stops
 */
int runClockCycle (int NF, int NW, int NB, int NR) {
    int isEnd;

	cpu -> cycle++; //increment cycle counter

	printf ("\nCycle %d\n", cpu -> cycle);

    fetchMultiInstructionUnit(NF);

    printf("Finished fetching.\n");

    decodeInstructionsUnit();

    printf("Finished decoding.\n");

    issueUnit(NW);

    printf("Finished issue.\n");
    
	printf("Execution -----------\n");
	
	insertintoWriteBackBuffer(NB);
	
	int perutilization = 0;
	perutilization = CommitUnit(NB, NR);
	cpu -> percentutilizationpercycle = cpu -> percentutilizationpercycle  + perutilization *(int)(100/NB);
    printf("Commit finished -----------\n");
	calculate(cpu -> percentutilizationpercycle);
	
	updateFetchBuffer();
    updateInstructionQueue();
    updateReservationStations();

    printf("Finished update.\n");

	
	

    isEnd = checkEnd();

	if(isEnd==1){
	    printf("Processor has finished working in %d cycle(s).\n", cpu -> cycle);
	    printf("Stalls due to full Reservation Stations: %d\n", cpu -> stallFullRS);
	    printf("Stalls due to full Reorder Buffer: %d\n", cpu -> stallFullROB);
		printf("Commited instructions Count is %d\n", cpu->commitCounter);
	    return 0;
	}else
	    return 1;

}

/**
 * Helper method for BHT hash table to calculate hash value from PC
 * @param PCHash
 * @return
 */
int getHashCodeFromPCHash (void *PCHash) {
	return *((int*)PCHash);
}

int getHashCodeFromInstructionAddress(void *InstructionAddress){
    return *((int*)InstructionAddress);
}

int compareDecodedInstructions(void *decodedInstruction1, void *decodedInstruction2){
    return ((Instruction *)decodedInstruction1) -> address - ((Instruction *)decodedInstruction2) -> address;
}

int getHashCodeFromBranchAddress(void *branchAddress){
    int fullAddr = *((int*)branchAddress);
    int k;
    int hashAddr = 0;
    int base = 2;
    int i;
    for(i = 0; i < 4; i++){
        if (i == 0)
            base = 1;
        else
            base *= 2;
        k = fullAddr >> (i + 3);
        if (k & 1)
            hashAddr += 1 * base;
    }
    return hashAddr;
}

int compareTargetAddress(void *targetAddress1, void *targetAddress2){
    return *((int*)targetAddress1)  - *((int*)targetAddress2);
}
//This function is used for reservation stations and load/write buffers
int getHashCodeFromROBNumber (void *ROBNumber) {
    KeyRS* keyRS = (KeyRS*)ROBNumber;
    //if (keyRS->progNum == 1){
        return keyRS->reorderNum;
    //}
    //else{
    //    return -(keyRS->reorderNum + 1); //+1 is to avoid the case both of them are zero
    //}
}

int compareROBNumber (void *ROBNumber1, void *ROBNumber2) {
    KeyRS* keyRS1 = (KeyRS*)ROBNumber1;
    KeyRS* keyRS2 = (KeyRS*)ROBNumber2;
    //if(keyRS1->progNum == keyRS2->progNum){
        return keyRS1->reorderNum - keyRS2->reorderNum;
    //}else{
    //    return keyRS1->reorderNum + keyRS2->reorderNum + 1;
    //}
}

int getHashCodeFromRegNumber (void *RegNumber) {
    return *((int*)RegNumber);
}

int compareRegNumber (void *RegNumber1, void *RegNumber2) {
    return *((int *)RegNumber1) - *((int *)RegNumber2);
}

/*
 * Check reservation station for earliest ready value.
 * @param int selectRS 0 to for RSint, 1 for RSfloat, 2 for Store, 3 for Load
 */
DictionaryValue *checkReservationStation(DictionaryEntry *dictEntry, int selectRS) {
    int loadStep1First = 1;
    DictionaryEntry *temp = dictEntry;
    while (dictEntry != NULL) {
        if (selectRS == 1) {
            RSfloat *RS = (RSfloat *)((DictionaryEntry *)dictEntry -> value -> value);
            if (RS -> isReady && !RS -> isExecuting) {
                return ((DictionaryEntry *)dictEntry) -> value;
            } else {
                dictEntry = dictEntry -> next;
            }
        } else if (selectRS == 2) {
            RSmem *RS = (RSmem *)((DictionaryEntry *)dictEntry -> value -> value);
            if (RS -> isReady && !RS -> isExecuting) {
                return ((DictionaryEntry *)dictEntry) -> value;
            } else {
                dictEntry = dictEntry -> next;
            }
        } else if (selectRS == 3) {
            RSmem *RS = (RSmem *)((DictionaryEntry *)dictEntry -> value -> value);
            if (!loadStep1First && RS -> isReady) {
                return ((DictionaryEntry *)dictEntry) -> value;
            }
            if (RS -> isReady && RS -> address == -1) {
                return ((DictionaryEntry *)dictEntry) -> value;
            } else {
                dictEntry = dictEntry -> next;
                if (dictEntry == NULL && loadStep1First) {
                    loadStep1First = 0;
                    dictEntry = temp;
                }
            }
        } else {
            RSint *RS = (RSint *)((DictionaryEntry *)dictEntry -> value -> value);
            if (RS -> isReady && !RS -> isExecuting) {
                return ((DictionaryEntry *)dictEntry) -> value;
            } else {
                dictEntry = dictEntry -> next;
            }
        }
    }
    return NULL;
}

/**
 * Prints information about instruction entering or leaving a pipeline.
 * @param Instruction *instruction The instruction entering or leaving.
 * @param char *pipeline String name of pipeline.
 * @param int entering 1 if entering the pipeline, 0 if exiting.
 */
void printPipeline(void *instruction, char *pipeline, int entering) {
    if (entering) {
        Instruction *inst = (Instruction *)instruction;
        printf("Instruction %s at address %x entered %s unit.\n",
         getOpcodeString ((int) (inst -> op)), inst -> address ,pipeline);
    } else {
        CompletedInstruction *inst = (CompletedInstruction *)instruction;
        Instruction *i = inst -> instruction;
        char *instructionString = malloc (90 * sizeof(char));
        sprintf (instructionString, "%x: %s, rd=%d, rs=%d, rt=%d, fd=%d, fs=%d, ft=%d, immediate=%d, target=%d",
         i -> address, getOpcodeString ((int) i -> op), i-> rd, i-> rs, i-> rt, i-> fd, i-> fs, i-> ft, i-> immediate,
          i-> target);
        printf("%s unit output PC %s with ROB number %d\n", pipeline, instructionString, inst -> ROB_number);
        free (instructionString);
    }
}

//Flush instructions queue and fetch buffer if mis-predicted branch
void flushInstructionQueueFetchBuffer(int NI){
    cpu -> fetchBuffer = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> fetchBufferResult = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> instructionQueue = createCircularQueue(NI);
	cpu -> instructionQueueResult = createCircularQueue(NI);
	//Set flag to 0
	cpu->isAfterBranch = 0;
	cpu -> stallNextFetch = 0;
    }


/*
 * Determines if branch was predicted correctly or not and updates CPU accordingly.
 */
void branchHelper (CompletedInstruction *instructionAndResult) {
    int NI = cpu -> instructionQueue -> size;
    int *targetAddress = &(instructionAndResult -> instruction -> target);
    DictionaryEntry *BTBEntry = getValueChainByDictionaryKey (cpu -> branchTargetBuffer, &(instructionAndResult->instruction->address));
    if (instructionAndResult -> intResult == 0) { //branch taken
        if (BTBEntry == NULL) { //predicted not taken
            addDictionaryEntry (cpu -> branchTargetBuffer, &(instructionAndResult -> instruction -> address),
                targetAddress);
            instructionAndResult -> isCorrectPredict = 0;
            flushInstructionQueueFetchBuffer (NI);
            cpu -> PC = *targetAddress;
            printf("Branch taken but predicted as not taken\n");
        } else { //predicted taken
            if (*(int *)(BTBEntry -> value -> value) == *targetAddress) {
                instructionAndResult -> isCorrectPredict = 1;
                    printf("Branch taken and predicted as taken\n");
            } else {
                removeDictionaryEntriesByKey (cpu -> branchTargetBuffer, &(instructionAndResult -> instruction -> address));
                addDictionaryEntry (cpu -> branchTargetBuffer, &(instructionAndResult -> instruction -> address),
                    targetAddress);
                instructionAndResult -> isCorrectPredict = 0;
                flushInstructionQueueFetchBuffer (NI);
                cpu -> PC = *targetAddress;
                printf("Branch taken and predicted as not taken\n");
            }
        }
    } else { //branch not taken
        if (BTBEntry != NULL) { //predicted taken
            removeDictionaryEntriesByKey (cpu -> branchTargetBuffer, &(instructionAndResult -> instruction -> address));
            instructionAndResult -> isCorrectPredict = 0;
            flushInstructionQueueFetchBuffer (NI);
            cpu -> PC = instructionAndResult -> instruction -> address + 4;
            printf("Branch not taken but predicted as taken\n");
        } else { //predicted not taken
            printf("Branch not taken and predicted as not taken.\n");
            instructionAndResult -> isCorrectPredict = 1;
        }
    }
    
}

// CDB utilization
void calculate(int sumutilized)
{
	float avgutilization;
	avgutilization = sumutilized/cpu->cycle;
	cpu -> avgutilization = avgutilization;
	//printf("CDB utilization - %f\n", avgutilization);
}

//Determine if the run cycle should be ended
int checkEnd(){
    //check whether PC exceeds last instruction in cache
    int fetchEnd = 0;
    int robCount, iQueueCount, fetchBufferCount;
    if (cpu -> PC >= (instructionCacheBaseAddress + (cacheLineSize * numberOfInstruction))) {
        fetchEnd = 1;
    }
    //Check whether all instructions are issued
    iQueueCount = getCountCircularQueue(cpu->instructionQueue);
    //Check whether all instructions in ROB have been committed
    robCount = getCountCircularQueue(cpu->reorderBuffer);
    //Check whether all instructions are decoded
    fetchBufferCount = countDictionaryLen(cpu->fetchBuffer);
	printf("part 1 - IQ count -%d robcount - %d fetch end - %d FBcount - %d\n", iQueueCount, robCount,fetchEnd,fetchBufferCount );
    if((fetchEnd==1)&&(robCount==0)&&(iQueueCount == 0)&&(fetchBufferCount==0)){
        return 1;
    }else{
        return 0;
    }
}



