# include "simulator.h"


/**
 * Fetch stage: Fetches upto NF instructions from the instruction memory, checks BTB and updates PC accordingly
 */
void fetch() {

    int i = NF;
    // Fetch upto NF instructions
    while (i>0)
    {
        int progc = processor->PC;
        //reached the end of instruction memory
        int lastadr = startAddress + 4 * lineNumber;
        if (lastadr <= progc) {
            printf("Finished Fetching for now\n");
            break;
        }

        // get the instruction from the dictionary using PC as key
        DictionaryEntry* currentInstruction = getValueChainByDictionaryKey(instructionMemory, &progc);
        //printf("Fetching %d: %s\n", progc, (char*)currentInstruction->value->value);

        char* instrPtr = (char*)malloc(sizeof(char) * 1024);
        int* pcPtr = (int*)malloc(sizeof(int));
        *pcPtr = progc;
        strcpy(instrPtr, ((char*)currentInstruction->value->value));
        instrstring* entr = (instrstring*)malloc(sizeof(instrstring));
        entr->pc = progc;
        entr->instrPtr = instrPtr;
        
        // //if it is the load from prev store, then fetch in next cycle

        // char* token = (char*)malloc(sizeof(char) * 1024);
        // strcpy(token, (char*)entr->instrPtr);
        // char* delimiter = " R,F\t(\n)$#rf";
        // token = strtok(token, "\n\t(, )");
        // //parsing the operation of the instruction
        // if (strcmp(token, "fld") == 0 ||strcmp(token, "FLD") == 0)
        // {
        //     int g = atoi(strtok(NULL, delimiter));
        //     int h = atoi(strtok(NULL, delimiter));
        //     int k = atoi(strtok(NULL, delimiter));
        //     if(h==processor->previmme && k==processor->prevrs){
        //         printf("immediate load after store with dependency. fetch in next cycle\n");
        //         break;
        //     }
        // }
        // else if (strcmp(token, "fsd") == 0||strcmp(token, "FSD") == 0)
        // {
        //     int g = atoi(strtok(NULL, delimiter));
        //     processor->previmme = atoi(strtok(NULL, delimiter));
        //     processor->prevrs = atoi(strtok(NULL, delimiter));

        // }
        // else{
        //     processor->previmme =-1;
        //      processor->prevrs=-1;

        // }
        enqueue(processor->fdPipelineres,entr);//add the instruction into queue which is read by decode stage in next cycle

        //Check the Branch target buffer for this address/PC
        DictionaryEntry* targetAddr = getValueChainByDictionaryKey(processor->branchTargetBuffer, &progc);
        if (targetAddr) {//if not null, then found an entry in BTB
            if (progc == *((int*)targetAddr->key)) {
               // printf("\nsetting pc based on branch prediction here\n");
                processor->PC = *((int*)targetAddr->value->value);
                i -= 1;
                continue;//fetch the instructions from the new address
            }
        }
        processor->PC = processor->PC + 4;
        i-=1;
    }
            //     processor->previmme =-1;
            //  processor->prevrs=-1;
}


/**
 * Instruction Decode stage: Decode the fetched instructions one by one
 */
void decode() {

    //read instructions that were fetched in previous cycle
    while (!isEmpty(processor->fdPipeline))
    {
        Instruction* instruction = (Instruction*)malloc(sizeof(Instruction));
        instruction->imme = 0;
        //initialise the cycle numbers to -1. these entries later used in printing tomasulo table
        instruction->fetchcycle = -1;
        instruction->decodecycle = -1;
        instruction->issuecycle = -1;
        instruction->excycle = -1;
        instruction->memcycle = -1;
        instruction->wbcycle = -1;
        instruction->commitcycle = -1;

        //int register source,target,destination
        instruction->rs = -1;
        instruction->rt = -1;
        instruction->rd = -1;
        //float register source,target,destination
        instruction->fs = -1;
        instruction->ft = -1;
        instruction->fd = -1;

        instrstring* fetchedInstruction = dequeue(processor->fdPipeline);
        instruction->address = fetchedInstruction->pc;
        char* token = (char*)malloc(sizeof(char) * 1024);
        strcpy(token, (char*)fetchedInstruction->instrPtr);
        char* delimiter = " R,F\t(\n)$#rf";
        token = strtok(token, "\n\t(, )");
        //parsing the operation of the instruction
        if (strcmp(token, "bne") == 0 || strcmp(token, "BNE") == 0)
        {
            instruction->op = BNE;
            instruction->rs = atoi(strtok(NULL, delimiter));
            instruction->rt = atoi(strtok(NULL, delimiter));
            token = strtok(NULL, "\n\t(, )");
            //store the branch label as target for branch instruction
            DictionaryEntry* lbl = getValueChainByDictionaryKey(branchLabels, (void*)token);
            instruction->target = *((int*)lbl->value->value);
        }
        else if (strcmp(token, "fld") == 0 ||strcmp(token, "FLD") == 0)
        {
            instruction->op = FLD;
            instruction->ft = atoi(strtok(NULL, delimiter));
            instruction->imme = atoi(strtok(NULL, delimiter));
            instruction->rs = atoi(strtok(NULL, delimiter));
        }
        else if (strcmp(token, "fsd") == 0||strcmp(token, "FSD") == 0)
        {
            instruction->op = FSD;
            instruction->ft = atoi(strtok(NULL, delimiter));
            instruction->imme = atoi(strtok(NULL, delimiter));
            instruction->rs = atoi(strtok(NULL, delimiter));

        }
        else if (strcmp(token, "add") == 0||strcmp(token, "ADD") == 0)
        {
            instruction->op = ADD;
            instruction->rd = atoi(strtok(NULL, delimiter));
            instruction->rs = atoi(strtok(NULL, delimiter));
            instruction->rt = atoi(strtok(NULL, delimiter));
        }
        else if (strcmp(token, "addi") == 0||strcmp(token, "ADDI") == 0)
        {
            instruction->op = ADDI;
            instruction->rd = atoi(strtok(NULL, delimiter));
            instruction->rs = atoi(strtok(NULL, delimiter));
            instruction->imme = atoi(strtok(NULL, delimiter));

        }
        else
        {
            if (strcmp(token, "fadd") == 0||strcmp(token, "FADD") == 0)
            {
                instruction->op = FADD;
            }
            else if (strcmp(token, "fsub") == 0||strcmp(token, "FSUB") == 0)
            {
                instruction->op = FSUB;
            }
            else if (strcmp(token, "fmul") == 0||strcmp(token, "FMUL") == 0)
            {
                instruction->op = FMUL;
            }
            else if (strcmp(token, "fdiv") == 0||strcmp(token, "FDIV") == 0)
            {
                instruction->op = FDIV;
            }

            instruction->fd = atoi(strtok(NULL, delimiter));
            instruction->fs = atoi(strtok(NULL, delimiter));
            instruction->ft = atoi(strtok(NULL, delimiter));
        }

        instruction->decodecycle = processor->cycle;//this instr is decoded in cycle number pprocessor->cycle
        instruction->fetchcycle = processor->cycle - 1;
        //add to dIPipeline which is used by ISSUE function. will be issued if reservation station is empty.
        enqueue(processor->dIPipeline, instruction);
        //printf("decode");

    }
}

void clearPipelineOrResetPC(ExecutedInstr* executedinstr) {
    int* targetAddress = &(executedinstr->instruction->target);
    //check if this address is stored in BTB
    DictionaryEntry* record = getValueChainByDictionaryKey(processor->branchTargetBuffer, &(executedinstr->instruction->address));
    if (record == NULL) {
        //if not stored in BTB, then it was predicted as not taken
        if (executedinstr->intResult == 0) {//but the branch supposed to be taken and the values weren't equal
            printf("wrong prediction, flusing and resetting pc\n");
            addDictionaryEntry(processor->branchTargetBuffer, &(executedinstr->instruction->address),targetAddress);
            //clear fetch and decode pipelines
            processor->fdPipeline = newQueue(NF);
            processor->fdPipelineres = newQueue(NF);
            processor->dIPipeline = newQueue(150);
            //reset PC to branch label address
            processor->PC = executedinstr->instruction->target;
            processor->StallIssue = 0;
        }
        else {//branch not taken and predicted not taken
            printf("predicted correctly\n");
            processor->StallIssue = 0;
        }
    }
    else {
        //address was stored in BTB
        if (executedinstr->intResult == 0) {
            //branch should be taken
            if (*(int*)(record->value->value) == executedinstr->instruction->target) {
                printf("predicted correctly\n");
                processor->StallIssue = 0;
            }
            else {
                printf("\nwrong prediction , flusing and resetting pc\n");
                removeDictionaryEntriesByKey(processor->branchTargetBuffer, &(executedinstr->instruction->address));
                addDictionaryEntry(processor->branchTargetBuffer, &(executedinstr->instruction->address),targetAddress);
                //clear pipeline
                processor->fdPipeline = newQueue(NF);
                processor->fdPipelineres = newQueue(NF);
                processor->dIPipeline = newQueue(150);
                //set PC to the branch label
                processor->PC = executedinstr->instruction->target;
                processor->StallIssue = 0;
            }
        }
        else {//branch not taken and predicted taken
            printf("\nwrong prediction , flusing and resetting pc\n");
            removeDictionaryEntriesByKey(processor->branchTargetBuffer, &(executedinstr->instruction->address));
            processor->fdPipeline = newQueue(NF);
            processor->fdPipelineres = newQueue(NF);
            processor->dIPipeline = newQueue(150);
            //set pc to instr addr + 4.
            processor->PC = executedinstr->instruction->address + 4;
            processor->StallIssue = 0;
        }
    }
}

int sendToIntRS(Dictionary* resvnext,char* rsType, Instruction* instruction) {

    //initialise a reservation station row in the reservation station status table
    RSint* RS = (RSint*)malloc(sizeof(RSint));
    //entry in register status table.
    RegStatus* RegStatusEntry = processor->registerstatus[instruction->rs];
    if (RegStatusEntry->busy == 1) {
        //if the register rs is the destination of a previous instruction
        int robNum = RegStatusEntry->reorderNum;
        // check if rs is ready in reorderbuffer
        if (((ROB*)processor->reorderBuffer->items[robNum])->isReady == 1) {
            //if rs is available then get its renamed register
            DictionaryEntry* renameRegIntEntry = getValueChainByDictionaryKey(processor->renameReg, &(RegStatusEntry->reorderNum));
            RS->Vj = *((int*)renameRegIntEntry->value->value);
            RS->Qj = -1;
            RS->isReady = 1;
        }
        else {
            //if rs is not ready, then we store the ROB number of the instruction that will generate rs.
            RS->Qj = robNum;
            RS->isReady = 0;
            if (strcmp(rsType, "BNE") == 0) {
                // if Branch instruction is waiting for a register value, then we stall the issue stage
                processor->StallIssue = 1;
                printf("FLUSHED INSTRUCTIONS in issue stage - because the branch had to wait\n");
            }
        }
    }
    else {
        //if the register rs is available and wasn't generated by another instr
        RS->Vj = processor->integerRegisters[instruction->rs];
        RS->Qj = -1;
        RS->isReady = 1;
    }

//now, rs is done. lets look at rt. Same process as above for rs.
    if (instruction->rt >= 0) {
        RegStatusEntry = processor->registerstatus[instruction->rt];
        if (RegStatusEntry->busy == 1) {
            int robNum = RegStatusEntry->reorderNum;
            if (((ROB*)processor->reorderBuffer->items[robNum])->isReady == 1) {
                DictionaryEntry* renameRegIntEntry = getValueChainByDictionaryKey(processor->renameReg, &(RegStatusEntry->reorderNum));
                RS->Vk =(int) *((float*)renameRegIntEntry->value->value);
                RS->Qk = -1;
                if ((RS->isReady == 1) && (strcmp(rsType, "BNE") == 0))
                {
                    //if branch instruction, and vj,vk are available then compute it.
                    ExecutedInstr* executedinstr = malloc(sizeof(ExecutedInstr));
                    executedinstr->instruction = instruction;
                    executedinstr->intResult = RS->Vj != RS->Vk ? 0 : -1;
                    executedinstr->ROB_number = processor->reorderBuffer->rear;
                    //check if the prediction for branch was correct. if not, then flush the fetch and decode stage
                    clearPipelineOrResetPC(executedinstr);
                    //branch has been computed, so no need to stall issue stage anymore.
                    processor->StallIssue = 0;
                }
            }
            else {
                //rt will be produced by a previous instr and we are waiting for it.
                RS->Qk = robNum;
                RS->isReady = 0;
                if (strcmp(rsType, "BNE") == 0) {
                    printf("FLUSHED INSTRUCTIONS in issue stage - because the branch had to wait2\n");
                    processor->StallIssue = 1;
                }
            }
        }
        else {
            //rt is available and it wasn't renamed/stored into by a previous instruction.
            RS->Vk = processor->integerRegisters[instruction->rt];
            RS->Qk = -1;
            if ((RS->isReady == 1) && (strcmp(rsType, "BNE") == 0))
            {
                ExecutedInstr* executedinstr = malloc(sizeof(ExecutedInstr));
                executedinstr->instruction = instruction;
                //check if the prediction for branch was correct. if not, then flush the fetch and decode stage
                executedinstr->intResult = RS->Vj != RS->Vk ? 0 : -1;
                executedinstr->ROB_number = processor->reorderBuffer->rear;
                clearPipelineOrResetPC(executedinstr);
                //branch has been computed, so no need to stall issue stage anymore.
                processor->StallIssue = 0;
            }
        }
    }
    else {
        RS->Qk = -1;
    }

    int* keyRS = (int*)malloc(sizeof(int));
    *keyRS = processor->reorderBuffer->rear;
    RS->Dest = processor->reorderBuffer->rear;
    RS->instruction = instruction;
    RS->isExecuting = 0;
    //store the ROB number and reservation station to be executed in next cycle in EXECUTE stage
    addDictionaryEntry(resvnext, keyRS, RS);
    //update the register status table for rd that this instr will store into it. So next instructions should wait for this
    if (strcmp(rsType, "BNE") != 0) {
        RegStatusEntry = processor->registerstatus[instruction->rd];
        RegStatusEntry->busy = 1;
        RegStatusEntry->reorderNum = processor->reorderBuffer->rear;
        void* valuePtr = malloc(sizeof(float));
        *((float*)valuePtr) = 0;
        //create new renamed register for rd
        removeDictionaryEntriesByKey(processor->renameReg, &(RS->Dest));
        addDictionaryEntry(processor->renameReg, &(RS->Dest), valuePtr);
    }
    instruction->issuecycle = processor->cycle;
   // printf("returned");
    return 1;
}

int sendToFloatRS(Dictionary* resv, Dictionary* resvnext, int resvStationsCount, Instruction* instruction) {
   // printf("returned float");
//same as sendtoIntRS but with float values and float registers
    RSfloat* RS = (RSfloat*)malloc(sizeof(RSfloat));

    if (resvStationsCount <= countDictionaryLen(resv) + countDictionaryLen(resvnext)) {
        processor->stallFullRS++;
        printf("Stall during IssueUnit because reservation stations %s is full.\n",getInstructionName((int)instruction->op));
        return 0;
    }
    //entry in register status table.
    RegStatus* RegStatusEntry = processor->registerstatus[instruction->fs+32];
    if (RegStatusEntry->busy == 1) {
        int robNum = RegStatusEntry->reorderNum;
        if (((ROB*)processor->reorderBuffer->items[robNum])->isReady == 1) {
            DictionaryEntry* renameRegEntry = getValueChainByDictionaryKey(processor->renameReg, &(RegStatusEntry->reorderNum));
            RS->Vj = *((float*)renameRegEntry->value->value);
            RS->Qj = -1;
            RS->isReady = 1;
        }
        else {
            RS->Qj = robNum;
            RS->isReady = 0;
        }
    }
    else {
        RS->Vj = processor->floatingPointRegisters[instruction->fs];
        RS->Qj = -1;
        RS->isReady = 1;
    }
    if (instruction->ft >= 0) {
        RegStatus* RegStatusEntry = processor->registerstatus[instruction->ft+32];
        if (RegStatusEntry->busy == 1) {
            int robNum = RegStatusEntry->reorderNum;
            if (((ROB*)processor->reorderBuffer->items[robNum])->isReady == 1) {
                DictionaryEntry* renameRegEntry = getValueChainByDictionaryKey(processor->renameReg, &(RegStatusEntry->reorderNum));
                RS->Vk = *((float*)renameRegEntry->value->value);
                RS->Qk = -1;
                if (RS->isReady == 1)
                    RS->isReady = 1;
            }
            else {
                RS->Qk = robNum;
                RS->isReady = 0;
            }
        }
        else {
            RS->Vk = processor->floatingPointRegisters[instruction->ft];
            RS->Qk = -1;
            if (RS->isReady == 1)
                RS->isReady = 1;
        }
    }
    else {
        RS->Qk = -1;
    }
    int* keyRS = (int*)malloc(sizeof(int));
    *keyRS = processor->reorderBuffer->rear;
    RS->Dest = processor->reorderBuffer->rear;
    RS->instruction = instruction;
    RS->isExecuting = 0;
        //store the ROB number and reservation station to be executed in next cycle in EXECUTE stage
    addDictionaryEntry(resvnext, keyRS, RS);
    RegStatusEntry = processor->registerstatus[instruction->fd+32];
    RegStatusEntry->busy = 1;
    RegStatusEntry->reorderNum = processor->reorderBuffer->rear;
    void* valuePtr = malloc(sizeof(float));
    *((float*)valuePtr) = 0;
    removeDictionaryEntriesByKey(processor->renameReg, &(RS->Dest));
    addDictionaryEntry(processor->renameReg, &(RS->Dest), valuePtr);
    instruction->issuecycle = processor->cycle;
      //  printf("returned float");
    return 1;
}

int sendToLSRS(Dictionary* resv, Dictionary* resvnext, Instruction* instruction) {
   // printf("returned load");

//send to load store reservation stations

    RSfloat* RS = (RSfloat*)malloc(sizeof(RSfloat));
//if the reservation stations are full, then stall here
    if(2 <=countDictionaryLen(resv)+countDictionaryLen(resvnext)) {
        processor->stallFullRS++;
        printf("Stall during IssueUnit because Load/Store buffer is full.\n");
        return 0;
    }
    RS->isReady = 1;
    RegStatus* RegStatusEntry = processor->registerstatus[instruction->rs];
    if (RegStatusEntry->busy == 1) {
        //if the register rs is the destination of a previous instruction, get its ROB number
        int robNum = RegStatusEntry->reorderNum;
        if (((ROB*)processor->reorderBuffer->items[robNum])->isReady == 1) {
            //if the prev instr has executed, then simply get its renamed register value
            DictionaryEntry* renameRegIntEntry = getValueChainByDictionaryKey(processor->renameReg, &(RegStatusEntry->reorderNum));
            RS->Vj = *((int*)renameRegIntEntry->value->value);
            RS->Qj = -1;
        }
        else {
            //wait for the register rs to be available
            RS->Qj = robNum;
            RS->isReady = 0;
        }
    }
    else {
       // printf("ENTERED THIS ELSE");
            int x,y;
            int flag=-1;
            for(x=0;x<64;x++){
                    if(processor->immeaddr[x].rs==instruction->rs && processor->immeaddr[x].imme==instruction->imme){
                        flag=processor->immeaddr[x].robnum;//the robnum of store above the load
                        //printf("\nfound in IMME ARRAY\n");
                        break;
                    }

            }
        //rs is available.
        if(flag==-1){

        RS->Vj = processor->integerRegisters[instruction->rs];
        RS->Qj = -1;
        }else{
                                   // printf("\nsettt flag as %d\n",flag);
                                            RS->Vj = processor->integerRegisters[instruction->rs];

            RS->Qj = flag;
            RS->isReady = 0;
        }
    }
    RS->Dest = processor->reorderBuffer->rear;
    RS->instruction = instruction;
    if (instruction->op == FSD) {
        //for store instruction, check if target register is available
        RegStatusEntry = processor->registerstatus[instruction->ft+32];//registerstatus[0 to 31] for int and [32 to 64] for float
        if (RegStatusEntry->busy == 1) {
            int robNum = RegStatusEntry->reorderNum;
            if (((ROB*)processor->reorderBuffer->items[robNum])->isReady == 1) {
                DictionaryEntry* renameRegFloatEntry = getValueChainByDictionaryKey(processor->renameReg, &(RegStatusEntry->reorderNum));
                RS->Vk = *((float*)renameRegFloatEntry->value->value);
                RS->Qk = -1;
            }
            else {
                RS->Qk = robNum;
                RS->isReady = 0;
            }
        }
        else {
            RS->Vk = processor->floatingPointRegisters[instruction->ft];
            RS->Qk = -1;
        }
    }
    RS->address = -1;
    RS->isExecuting = 0;
    int* keyRS = (int*)malloc(sizeof(int));
    *keyRS = processor->reorderBuffer->rear;
        if (instruction->op == FSD) {
        int x;
            for(x=0;x<64;x++){
                    if(processor->immeaddr[x].rs==-2){
                        processor->immeaddr[x].rs=instruction->rs;
                        processor->immeaddr[x].imme=instruction->imme;
                        processor->immeaddr[x].robnum=*keyRS;
                        break;
                    }

            }
        }
    //store the ROB number and reservation station to be executed in next cycle in EXECUTE stage
    addDictionaryEntry(resvnext, keyRS, RS);
    if (instruction->op == FLD) {
        //if the instr is load, then update the register status table and
        // get new renamed register for it
        RegStatusEntry = processor->registerstatus[instruction->ft+32];
        RegStatusEntry->busy = 1;
        RegStatusEntry->reorderNum = processor->reorderBuffer->rear;
        void* valuePtr = malloc(sizeof(float));
        *((float*)valuePtr) = 0;
        removeDictionaryEntriesByKey(processor->renameReg, &(RS->Dest));
        addDictionaryEntry(processor->renameReg, &(RS->Dest), valuePtr);
    }
    instruction->issuecycle = processor->cycle;
       // printf("returned load store");
    return 1;
}

int canRename(int d) {
    //if rename registers are available OR if if this register has been renamed and is in rename register then return 0
    if ((countDictionaryLen(processor->renameReg) != 32) || (getValueChainByDictionaryKey(processor->renameReg, &d)!=NULL))
    {
        return 0;
    }
    else
    {     
        printf("Stall because hardware registers are unavailable.\n");
        return 1;
    }
    return 0;
}


//Instruction Issue stage: Issue upto NW decoded instructions
void issue() {
    int i;
    for (i = 0; i < NW; i++)
    {
        //printf("next loop\n");
        //read the instruction from decode pipeline
        Instruction* instruction = front(processor->dIPipeline);
        // check if we shouldnt issue because a branch instruction before this hasn't been resolved yet.
        //if the branch before this instr hasn't been decided, then we dont want this instruction to reach EX stage.
        if ((processor->StallIssue == 1)||(!instruction)) {
            break;
        }
        if (isFull(processor->reorderBuffer)) {
            printf("Instruction %d Stalled due to full ROB\n",instruction->address);
            processor->stallFullROB+=1;
            break;
        }
        int issued = 0;
        //check the opcode of the instruction to issue
        switch (instruction->op) {
        case ADDI:
        case ADD:
            //check if rename registers are available
            if (!canRename(instruction->rd)) {
                    //check if int reservation stations are available
                    if (4 <= countDictionaryLen(processor->resv[7]) + countDictionaryLen(processor->resv[0])) {
                        processor->stallFullRS++;
                        printf("Stall during IssueUnit because INT reservation stations are full.\n");
                        issued=0;
                    }else{
                        //send to int reservation station and if successfully issued then, issued=1.
                           // printf("\naddiiiii");
                        issued = sendToIntRS(processor->resv[0], "INT", instruction);
                    }
            }
            break;
        case FADD:
        case FSUB:
            if (!canRename(instruction->fd)) {
                //send to float reservation station and if the reservation stations are available, then issued=1.
                issued = sendToFloatRS(processor->resv[10], processor->resv[3], 3, instruction);
            }
            break;
        case FMUL:
            if (!canRename(instruction->fd)) {
                issued = sendToFloatRS(processor->resv[11], processor->resv[4], 4, instruction);
            }
            break;
        case FDIV:
            if (!canRename(instruction->fd)) {
                issued = sendToFloatRS(processor->resv[12], processor->resv[5], 2, instruction);
            }
            break;
        case FLD:
            if (!canRename(instruction->ft)) {
                issued = sendToLSRS(processor->resv[8], processor->resv[1],instruction);
            }
            break;
        case FSD:
            issued = sendToLSRS(processor->resv[9], processor->resv[2],instruction);
            break;
        case BNE:
                if (1 <= countDictionaryLen(processor->resv[13]) + countDictionaryLen(processor->resv[6])) {
                        processor->stallFullRS++;
                        printf("Stall during IssueUnit because reservation stations BNE is full.\n");
                        issued=0;
                }else{
                    issued = sendToIntRS(processor->resv[6], "BNE",instruction);
                }
            break;
        default:
            break;
        }
        if (!issued) {
            break;
        }
        //initialize the Reorder buffer record.
        ROB* robRecord = (ROB*)malloc(sizeof(ROB));
        robRecord->DestReg = -1;
        robRecord->DestRenameReg = processor->reorderBuffer->rear;
        robRecord->DestAddr = 0;
        robRecord->instruction = instruction;
        robRecord->state = "I";
        robRecord->isReady = 0;

        switch (instruction->op) {
        case ADDI:
        case ADD:
            robRecord->DestReg = instruction->rd;
            break;
        case FADD:
        case FSUB:
        case FMUL:
        case FDIV:
            robRecord->DestReg = instruction->fd;
            break;
        case FLD:
            robRecord->DestReg = instruction->ft;
            break;
        default:
            break;
        }
        //add this record to reorderbuffer.we maintain IN-Order in ROB. execution can be out of order
        enqueue(processor->reorderBuffer, robRecord);
        //instruction has been issued, so can be removed from decoded piepeline
        dequeue(processor->dIPipeline);
                                   // printf("\issue end");
 
    }
    //printf("\n returned from issue\n");
}


//takes care of the latency cycles for FP operations
ExecutedInstr* executeFPUnit(struct Cqueue* pipeline, int func) {
    ExecutedInstr* output;

    output = dequeue(pipeline);

    if (output != NULL) {
        if (func == 0)
        {
            processor->FPmulPipelineBusy = 0;
        }
        else if (func == 1)
        {
            processor->FPaddPipelineBusy = 0;
        }
        else
        {
            processor->FPdivPipelineBusy = 0;
        }
    }
    if (pipeline->size+1 < pipeline->maxsize) {
        enqueue(pipeline, NULL);
    }

    return output;
}
//takes care of the latency cycles for int,load/store and BU operations which has 1 latency cycle for execution
ExecutedInstr* executePipelinedUnit(struct Cqueue* pipeline) {
    ExecutedInstr* output = (ExecutedInstr*)dequeue(pipeline);

    if (pipeline->size < pipeline->maxsize - 1) {
        enqueue(pipeline, NULL);
    }

    return output;
}


//returns the next available reservation station
DictionaryValue* fifoGetFromRSList(DictionaryEntry* current, int rstype) {
    DictionaryEntry* temp = current;
    int flag = 1;
    //traverse throught rs list
    while (current) {
        if (rstype==1 || rstype==2) {
            //if float / store resv station is  needed 
            RSfloat* RS = (RSfloat*)((DictionaryEntry*)current->value->value);
            if (RS->isReady==1 && !RS->isExecuting) {
                //found a resv station that is ready to execute
                return ((DictionaryEntry*)current)->value;
            }
            else {
                current = current->next;
            }
        }
        else if (rstype == 3) {
            //if load resv station needed
            RSfloat* RS = (RSfloat*)((DictionaryEntry*)current->value->value);

            //                 if(RS->isReady==6){
            //     RS->isReady=1;
            //     printf("ENTERED READY=6");
            //     return NULL;
            // }
            if (!flag && RS->isReady==1) {//found a reservation station that is ready to execute
            //printf("ENTERED READY1");
                return ((DictionaryEntry*)current)->value;
            }
            if (RS->isReady==1 && RS->address == -1) {//found a reservation station that is ready to execute
            //printf("ENTERED READY2");
                return ((DictionaryEntry*)current)->value;
            }
            else {
                current = current->next;
                if (current == NULL && flag) {
                    flag = 0;
                    current = temp;
                }
            }
        }
        else {
            //if int resv station is needed
            RSint* RS = (RSint*)((DictionaryEntry*)current->value->value);
            if (RS->isReady==1 && !RS->isExecuting) {//if found
                return ((DictionaryEntry*)current)->value;
            }
            else {
                current = current->next;
            }
        }
    }
    // either resv stations were empty or were executing
    //printf("returned NULL");
        return NULL;
}

//instr executed, so update the renamed register with new value and remove the instr from reservation station
void executefpunits(int unit,int fparg,ExecutedInstr** results){
    int* key = malloc(sizeof(int));
    *key = results[unit]->ROB_number;
    int r = results[unit]->ROB_number;
    if (isValid(processor->reorderBuffer, r)) {
        removeDictionaryEntriesByKey(processor->renameReg, &(results[unit]->ROB_number));
        addDictionaryEntry(processor->renameReg, &(results[unit]->ROB_number), &(results[unit]->fpResult));
    }
    removeDictionaryEntriesByKey(processor->resv[fparg], key);
}

ExecutedInstr** execute() {
    int i, j;
    //initialize variables
    Instruction* instruction = malloc(sizeof(Instruction));
    void* valuePtr = malloc(sizeof(float));
    ExecutedInstr* executedinstr = malloc(sizeof(ExecutedInstr));
    RSint* rsint;
    RSfloat* rsfloat;
    RSfloat* rsloadstore;
    RSfloat* RS;
    DictionaryValue* dictVal;
    DictionaryEntry** issuedInstr = malloc(sizeof(DictionaryEntry*) * 8);
    static ExecutedInstr* results[7];

    int stallorcontinue;
    int loadStallROBNumber = -1;
    int moveOn;

    ExecutedInstr* tempbranch = NULL;
    int* key = malloc(sizeof(int));

//get INT reservation station
   DictionaryEntry* dictEntry = (DictionaryEntry*)processor->resv[13]->head;
    dictVal = fifoGetFromRSList(dictEntry, 0);
    if (dictVal != NULL) {
        //if BU Reservation station is ready to execute and had an instr to execute; 
        //then get the issued instruction to be executed in this cycle
        rsint = (RSint*)(dictVal->value);
        *key = rsint->Dest;
        issuedInstr[7] = getValueChainByDictionaryKey(processor->resv[13], key);//[7] will be used for BU Unit
        //extract the instruction to execute from the reservation station
        rsint = (RSint*)((DictionaryEntry*)issuedInstr[7]->value->value);
        instruction = rsint->instruction;
    }
    else {
        //Reservation station is unavailable/free. so no instruction to execute for INT unit.
        issuedInstr[7] = NULL;
    }

    executedinstr->instruction = instruction;
    if (instruction->op == BNE) {//check the instruction is branch instr. If yes, then execute it.
        rsint->isExecuting = 1;
        executedinstr->intResult = rsint->Vj != rsint->Vk ? 0 : -1;
        executedinstr->ROB_number = rsint->Dest;
        tempbranch = malloc(sizeof(ExecutedInstr) * 2);
        memcpy(tempbranch, executedinstr, sizeof(ExecutedInstr));
        instruction->excycle = processor->cycle;
    }

    //check how many executed instructions can goto writeback stage in next cycle
    int NBLeft = NB - (countDictionaryLen(processor->WBBuff));
    i = 0;
    int r;

    if (i < NBLeft) {
        results[BU] = executePipelinedUnit(processor->BUPipeline);
        if (results[BU] != NULL) {
            i++;
            *key = results[BU]->ROB_number;
            r = results[BU]->ROB_number;
            if (isValid(processor->reorderBuffer, r)) {
                removeDictionaryEntriesByKey(processor->renameReg, &(results[BU]->ROB_number));
                addDictionaryEntry(processor->renameReg, &(results[BU]->ROB_number), &(results[BU]->intResult));
            }
            removeDictionaryEntriesByKey(processor->resv[13], key);
            clearPipelineOrResetPC(results[BU]);
        }
        if (tempbranch != NULL) {
            enqueue(processor->BUPipeline, tempbranch);
        }
    }
    else {
        if (tempbranch != NULL) {
            *key = tempbranch->ROB_number;
            RSint* stalled = (RSint*)(getValueChainByDictionaryKey(processor->resv[13], key)->value->value);
            stalled->isExecuting = 0;
        }
    }


    dictEntry = (DictionaryEntry*)processor->resv[7]->head;
    dictVal = fifoGetFromRSList(dictEntry, 0);

    if (dictVal != NULL) {
        //if INT Reservation station is ready to execute and had an instr to execute; 
        //then get the issued instruction to be executed in this cycle
        rsint = (RSint*)dictVal->value;
        *key = rsint->Dest;
        issuedInstr[0] = getValueChainByDictionaryKey(processor->resv[7], key);//[0] will be used for INT Unit
    }
    else {
        issuedInstr[0] = NULL;
    }
    issuedInstr[1] = NULL;

    // fetch resservation station for load
    dictEntry = (DictionaryEntry*)processor->resv[8]->head;
    dictVal = fifoGetFromRSList(dictEntry, 3);
    if (dictVal != NULL) {
        rsloadstore = (RSfloat*)(dictVal->value);
        *key = rsloadstore->Dest;
        issuedInstr[2] = getValueChainByDictionaryKey(processor->resv[8], key);
    }
    else {
            //printf("returned NULL");
        issuedInstr[2] = NULL;
    }
    //fetch reservation station fo store
    dictEntry = (DictionaryEntry*)processor->resv[9]->head;
    dictVal = fifoGetFromRSList(dictEntry, 2);
    if (dictVal != NULL) {
        rsloadstore = (RSfloat*)(dictVal->value);
        *key = rsloadstore->Dest;
        issuedInstr[3] = getValueChainByDictionaryKey(processor->resv[9], key);
    }
    else {
        issuedInstr[3] = NULL;
    }
    //fetch reservation station for FPadd
    if (!(processor->FPaddPipelineBusy)) {
        dictEntry = (DictionaryEntry*)processor->resv[10]->head;
        dictVal = fifoGetFromRSList(dictEntry, 1);
        if (dictVal != NULL) {
            rsfloat = (RSfloat*)(dictVal->value);
            *key = rsfloat->Dest;
            issuedInstr[4] = getValueChainByDictionaryKey(processor->resv[10], key);
        }
        else {
            issuedInstr[4] = NULL;
        }
    }
        //fetch reservation station for FPmul
    if (!(processor->FPmulPipelineBusy)) {
        dictEntry = (DictionaryEntry*)processor->resv[11]->head;
        dictVal = fifoGetFromRSList(dictEntry, 1);
        if (dictVal != NULL) {
            rsfloat = (RSfloat*)(dictVal->value);
            *key = rsfloat->Dest;
            issuedInstr[5] = getValueChainByDictionaryKey(processor->resv[11], key);
        }
        else {
            issuedInstr[5] = NULL;
        }
    }
        //fetch reservation station for FPdiv
    if (!(processor->FPdivPipelineBusy)) {
        dictEntry = (DictionaryEntry*)processor->resv[12]->head;
        dictVal = fifoGetFromRSList(dictEntry, 1);
        if (dictVal != NULL) {
            rsfloat = (RSfloat*)(dictVal->value);
            *key = rsfloat->Dest;
            issuedInstr[6] = getValueChainByDictionaryKey(processor->resv[12], key);
        }
        else {
            issuedInstr[6] = NULL;
        }
    }
    int storeFirst = 0;
    ExecutedInstr* tempInt = NULL;
    ExecutedInstr* tempLoad = NULL;
    ExecutedInstr* tempStore = NULL;
    ExecutedInstr* tempfpadd = NULL;
    ExecutedInstr* tempfpmult = NULL;
    ExecutedInstr* tempfpdiv = NULL;
    i=0;
    for(i=0;i<8;i++){
        if (issuedInstr[i] == NULL) {
            //if no instruction to execute for this unit
            continue;
        }
        if (i < 2) {
            //for int/bu unit, fetch the instruction from the reservation station
            rsint = (RSint*)((DictionaryEntry*)issuedInstr[i]->value->value);
            instruction = rsint->instruction;
        }
        else if (i == 2 || i == 3) {
            //for load or store unit
            rsloadstore = (RSfloat*)(((DictionaryEntry*)issuedInstr[i])->value->value);
            instruction = rsloadstore->instruction;
        }
        else {
            // for fpadd,mul,div units
            rsfloat = (RSfloat*)(((DictionaryEntry*)issuedInstr[i])->value->value);
            instruction = rsfloat->instruction;
        }
        executedinstr->instruction = instruction;
        switch (instruction->op) {
        case ADDI:
        case ADD:
        //execute the addition and store in executedinstr
            rsint->isExecuting = 1;
            if (instruction->op == ADD) {
                executedinstr->intResult = rsint->Vj + rsint->Vk;
            }
            else {
                executedinstr->intResult = rsint->Vj + instruction->imme;
            }
            executedinstr->ROB_number = rsint->Dest;
            tempInt = malloc(sizeof(ExecutedInstr) * 2);
            memcpy(tempInt, executedinstr, sizeof(ExecutedInstr));
            instruction->excycle = processor->cycle;
            break;
        case FADD:
        case FSUB:
            rsfloat->isExecuting = 1;
            if (instruction->op == FSUB) {
                executedinstr->fpResult = rsfloat->Vj - rsfloat->Vk;//subtracted the register values
            }
            else {
                executedinstr->fpResult = rsfloat->Vj + rsfloat->Vk;
            }
            executedinstr->ROB_number = rsfloat->Dest;
            tempfpadd = malloc(sizeof(ExecutedInstr) * 2);
            memcpy(tempfpadd, executedinstr, sizeof(ExecutedInstr));
            processor->FPaddPipelineBusy = 1;
            instruction->excycle = processor->cycle;
            break;
        case FMUL:
            rsfloat->isExecuting = 1;
            executedinstr->fpResult = rsfloat->Vj * rsfloat->Vk;
            executedinstr->ROB_number = rsfloat->Dest;
            tempfpmult = malloc(sizeof(ExecutedInstr) * 2);
            memcpy(tempfpmult, executedinstr, sizeof(ExecutedInstr));
            processor->FPmulPipelineBusy = 1;
            instruction->excycle = processor->cycle;
            break;
        case FDIV:
            rsfloat->isExecuting = 1;
            executedinstr->fpResult = rsfloat->Vj / rsfloat->Vk;
            executedinstr->ROB_number = rsfloat->Dest;
            tempfpdiv = malloc(sizeof(ExecutedInstr) * 2);
            memcpy(tempfpdiv, executedinstr, sizeof(ExecutedInstr));
            processor->FPdivPipelineBusy = 1;
            instruction->excycle = processor->cycle;
            break;
        case FLD:
            if (rsloadstore->isExecuting == 0) {
                //execute load
                rsloadstore->isExecuting = 1;
                rsloadstore->address = rsloadstore->Vj + instruction->imme;
              // printf("\nload address: %d %d",rsloadstore->Vj,instruction->imme);
                loadStallROBNumber = rsloadstore->Dest;
                instruction->excycle = processor->cycle;
            }
            else {
                rsloadstore = NULL;
            }
            dictEntry = processor->resv[8]->head;
            stallorcontinue = 0;
            while (!stallorcontinue) {
                    //fprintf(stderr,"looping inner");
            if (dictEntry == NULL) {
                stallorcontinue = 2;
            }
            else {
                RS = (RSfloat*)((DictionaryEntry*)dictEntry->value->value);
                if (RS->isReady && RS->address != -1) {
                    j=0;
                    while (j < processor->reorderBuffer->size && j < ((RS->Dest - processor->reorderBuffer->front) % processor->reorderBuffer->maxsize) && j != -1) 
                    {
                        int x=(processor->reorderBuffer->front + j) % (processor->reorderBuffer->maxsize);
                        if (((ROB*)(processor->reorderBuffer->items[x]))->DestAddr == RS->address)
                        {
                            j = -2;
                        }
                        j++;
                    }
                    moveOn = 0;
                    if (rsloadstore != NULL && RS->Dest == rsloadstore->Dest) {
                        moveOn = 1;
                    }
                    if (j != -1 && !moveOn && RS->isExecuting != 2) {
                        stallorcontinue = 1;
                        rsloadstore = RS;
                    }
                    else {
                        dictEntry = dictEntry->next;
                    }
                }
                else {
                    dictEntry = dictEntry->next;
                }
            }
            }
            //MEM cycle for FLD. reads the mem loc data into register
            if (stallorcontinue == 1 && issuedInstr[3] == NULL) {
                executedinstr->instruction = rsloadstore->instruction;
                if (executedinstr->instruction->op != FLD) {
                    break;
                }
                rsloadstore->isExecuting = 2;
                void* addrPtr = malloc(sizeof(int));
                               //rsloadstore->address = processor->integerRegisters[instruction->rs] + instruction->imme;
                *((int*)addrPtr) = rsloadstore->address;
                
              // printf("\nloaded from address %d", *((int*)addrPtr));
             //  printf("\ninstr data is  %d %d", instruction->rs,instruction->imme);
                DictionaryEntry* dataMemoryElement = getValueChainByDictionaryKey(dataMemory, addrPtr);
                if (dataMemoryElement != NULL) {
                    valuePtr = dataMemoryElement->value->value;
                                    //  printf("read valueeeeeeeeeeeeee %f",*((float*)valuePtr));

                }
                else {
                    *((float*)valuePtr) = 0.0;
                }
                executedinstr->fpResult = *((float*)valuePtr);
                executedinstr->ROB_number = rsloadstore->Dest;
                tempLoad = malloc(sizeof(ExecutedInstr) * 2);
                memcpy(tempLoad, executedinstr, sizeof(ExecutedInstr));
                instruction->memcycle = processor->cycle;
            }
            break;
        case FSD:
            rsloadstore->isExecuting = 1;
            rsloadstore->address = rsloadstore->Vj + instruction->imme;
            executedinstr->intResult=rsloadstore->Vj;// added newly
            executedinstr->address = rsloadstore->address;
            executedinstr->fpResult = rsloadstore->Vk;
            executedinstr->ROB_number = rsloadstore->Dest;
            tempStore = malloc(sizeof(ExecutedInstr) * 2);
            memcpy(tempStore, executedinstr, sizeof(ExecutedInstr));
            storeFirst = 1;
            instruction->excycle = processor->cycle;
            break;
        default:
            break;
        }
    }

    //check how many executed instructions can goto writeback stage in next cycle
    NBLeft = NB - (countDictionaryLen(processor->WBBuff));
    i = 0;
    r=0;

    if (i < NBLeft) {
        results[INT] = executePipelinedUnit(processor->INTPipeline);
        //printf("ok2\n");
        if (results[INT] != NULL) {// latency cycles are over. so the dest register is available now
            i++;
            *key = results[INT]->ROB_number;
            r = results[INT]->ROB_number;
            //printf("ALL GOOD %d\n",r);
            if (isValid(processor->reorderBuffer, r)) {
                  //printf("ALL GOOD1\n");
         /*         if (!results[INT]) {
                      printf("NOT GOOD1\n");
                  }*/
                //update the renamed register value
                removeDictionaryEntriesByKey(processor->renameReg, &(results[INT]->ROB_number));
                addDictionaryEntry(processor->renameReg, &(results[INT]->ROB_number), &(results[INT]->intResult));
            }

            removeDictionaryEntriesByKey(processor->resv[7], key);
        }
        if (tempInt != NULL) {
            enqueue(processor->INTPipeline, tempInt);
        }
    }
    else {
        if (tempInt != NULL) {
            *key = tempInt->ROB_number;
            RSint* stalled = (RSint*)(getValueChainByDictionaryKey(processor->resv[7], key)->value->value);
            stalled->isExecuting = 0;
        }
    }
    //repeat the above process for all units load,store,fpadd,fpmult,fpdiv

    if (i < NBLeft) {
        results[LS] = executePipelinedUnit(processor->LoadStorePipeline);
        if (results[LS] != NULL) {
            i++;
            *key = results[LS]->ROB_number;
            r = results[LS]->ROB_number;
            if (isValid(processor->reorderBuffer, r)) {
                removeDictionaryEntriesByKey(processor->renameReg, &(results[LS]->ROB_number));
                addDictionaryEntry(processor->renameReg, &(results[LS]->ROB_number), &(results[LS]->fpResult));
            }
            if (results[LS]->instruction->op == FLD) {
                removeDictionaryEntriesByKey(processor->resv[8], key);
            }
            else if (results[LS]->instruction->op == FSD) {
                removeDictionaryEntriesByKey(processor->resv[9], key);
            }
        }
        if (!storeFirst && tempLoad != NULL) {
            enqueue(processor->LoadStorePipeline, tempLoad);
        }
        else if (tempStore != NULL) {
            enqueue(processor->LoadStorePipeline, tempStore);
        }
    }
    else {
        if (loadStallROBNumber != -1) {
            *key = loadStallROBNumber;
            RSfloat* stalled = (RSfloat*)(getValueChainByDictionaryKey(processor->resv[8], key)->value->value);
            stalled->isExecuting = 0;
        }
        if (tempLoad != NULL) {
            *key = tempLoad->ROB_number;
            RSfloat* stalled = (RSfloat*)(getValueChainByDictionaryKey(processor->resv[8], key)->value->value);
            stalled->isExecuting = 1;
        }
        if (tempStore != NULL) {
            *key = tempStore->ROB_number;
            RSfloat* stalled = (RSfloat*)(getValueChainByDictionaryKey(processor->resv[9], key)->value->value);
            stalled->isExecuting = 0;
        }
    }
    //printf("OOOOOOOOOO");

    if (i < NBLeft) {
        results[FPadd] = executeFPUnit(processor->FPaddPipeline, 1);
        if (results[FPadd] != NULL) {
            i++;
            executefpunits(FPadd,10,results);
        }
        if (tempfpadd != NULL) {
            enqueue(processor->FPaddPipeline, tempfpadd);
        }
    }
    else {
        if (tempfpadd != NULL) {
            *key = tempfpadd->ROB_number;
            RSfloat* stalled = (RSfloat*)(getValueChainByDictionaryKey(processor->resv[10], key)->value->value);
            stalled->isExecuting = 0;
            processor->FPaddPipelineBusy = 0;
        }
    }
    if (i < NBLeft) {
        results[FPmult] = executeFPUnit(processor->FPmultPipeline, 0);
        if (results[FPmult] != NULL) {
            i++;
            executefpunits(FPmult,11,results);
        }
        if (tempfpmult != NULL) {
            enqueue(processor->FPmultPipeline, tempfpmult);
        }
    }
    else {
        if (tempfpmult != NULL) {
            *key = tempfpmult->ROB_number;
            RSfloat* stalled = (RSfloat*)(getValueChainByDictionaryKey(processor->resv[11], key)->value->value);
            stalled->isExecuting = 0;
            processor->FPmulPipelineBusy = 0;

        }
    }
    if (i < NBLeft) {
        results[FPdiv] = executeFPUnit(processor->FPdivPipeline, 2);
        if (results[FPdiv] != NULL) {
            i++;
            executefpunits(FPdiv,12,results);
        }
        if (tempfpdiv != NULL) {
            enqueue(processor->FPdivPipeline, tempfpdiv);
        }
    }
    else {
        if (tempfpdiv != NULL) {
            *key = tempfpdiv->ROB_number;
            RSfloat* stalled = (RSfloat*)(getValueChainByDictionaryKey(processor->resv[12], key)->value->value);
            stalled->isExecuting = 0;
            processor->FPdivPipelineBusy = 0;
        }
    }
    return results;

}

int Commit()
{
    if (processor->reorderBuffer == NULL) {
        return 0;
    }
//fetch the top entry in ROB to commit
    ROB* robRecord=front(processor->reorderBuffer);
    RegStatus* RegStatusEntry;
    int robnum;
    int loadOnCDB = 0;
    int NC=NB;
    //while cdb lines are available
    while (processor->reorderBuffer->size != 0 && NC != 0)
    {
        if (robRecord->isReady == 0 || (strcmp(robRecord->state, "I") == 0))
        {//if in issue stage or the instruction is waiting for a register
            return loadOnCDB;
        }
        void* valuePtr = malloc(sizeof(float));
        robnum = processor->reorderBuffer->front;
        robRecord = dequeue(processor->reorderBuffer);
        if (robRecord->instruction->op == BNE) {
            //if branch, then free rename register
            removeDictionaryEntriesByKey(processor->renameReg, &(robnum));
            robRecord->instruction->commitcycle = processor->cycle;
            enqueue(processor->tomTable, robRecord);
        }
        else if (robRecord->instruction->op == FSD)
        {
            //store instr. Store the value in renamed register into Destination address
            //free the renamed register.
            float DestVal; int DestRenameReg;
            DestRenameReg = robRecord->DestRenameReg;
            DictionaryEntry* Current = getValueChainByDictionaryKey(processor->renameReg, &DestRenameReg);
            DestVal = *((float*)Current->value->value);
            removeDictionaryEntriesByKey(dataMemory, &(robRecord->DestAddr));
            *((float*)valuePtr) = (float)DestVal;
            //printf("stored into memory...............\n");
            addDictionaryEntry(dataMemory, &(robRecord->DestAddr), valuePtr);
                    int x;
            for(x=0;x<64;x++){
                    if(processor->immeaddr[x].rs==robRecord->instruction->rs){
                        if(processor->immeaddr[x].imme==robRecord->instruction->imme){
                        processor->immeaddr[x].rs=-1;
                        processor->immeaddr[x].imme=-1;
                        processor->immeaddr[x].robnum=-1;}
                    }
            }
            removeDictionaryEntriesByKey(processor->renameReg, &DestRenameReg);
            robRecord->instruction->commitcycle = processor->cycle;
            enqueue(processor->tomTable, robRecord);
        }
        else if ((robRecord->instruction->op == ADD) || (robRecord->instruction->op == ADDI)) {
            //INT instr. Store the value in renamed register into Destination register
            //free the renamed register.
            int DestRenameReg, DestVal, DestReg;
            DestRenameReg = robRecord->DestRenameReg;
            DestReg = robRecord->DestReg;
            DictionaryEntry* Current = getValueChainByDictionaryKey(processor->renameReg, &DestRenameReg);
            DestVal = *((int*)Current->value->value);
            processor->integerRegisters[DestReg] = DestVal;
            RegStatusEntry = processor->registerstatus[DestReg];
            if (RegStatusEntry->reorderNum == robnum) {
                RegStatusEntry->busy = 0;
            }
            removeDictionaryEntriesByKey(processor->renameReg, &DestRenameReg);
            robRecord->instruction->commitcycle = processor->cycle;
            enqueue(processor->tomTable, robRecord);
        }
        else {
                        //Float instr. Store the value in renamed register into Destination register
            //free the renamed register.
            int DestRenameReg, DestReg; float DestVal;
            DestRenameReg = robRecord->DestRenameReg;
            DestReg = robRecord->DestReg;
            DictionaryEntry* Current = getValueChainByDictionaryKey(processor->renameReg, &DestRenameReg);
            DestVal = *((float*)Current->value->value);
            processor->floatingPointRegisters[DestReg] = DestVal;
            RegStatusEntry = processor->registerstatus[DestReg+32];
            if (RegStatusEntry->reorderNum == robnum) {
                RegStatusEntry->busy = 0;
            }
            removeDictionaryEntriesByKey(processor->renameReg, &DestRenameReg);
            robRecord->instruction->commitcycle = processor->cycle;
            enqueue(processor->tomTable, robRecord);
        }
        
        NC--;
        loadOnCDB++;
        robRecord = processor->reorderBuffer->items[processor->reorderBuffer->front];
    }

    return loadOnCDB;
}


//store from temp buffer resv[7-13] into resv[0 to 6], to be used in next cycle. 
void reviseRS(ExecutedInstr* instruction) {
    //printf("REvisrs entered\n");
    DictionaryEntry* current;
    RSint* RSint;
    RSfloat* RSfloat;
    int i;
    for (i = 0; i < 14; i++) {
        current = processor->resv[i]->head;
        if (i == 13||i==7||i==0||i==6) {
            //printf("REvisrs entered loop1\n");
            while (current) {
                RSint = current->value->value;
                if (RSint->isReady == 0) {
                    if (RSint->Qj == instruction->ROB_number) {
                        RSint->Vj = instruction->intResult;
                        RSint->Qj = -1;
                    }
                    if (RSint->Qk == instruction->ROB_number) {
                        RSint->Vk = instruction->intResult;
                        RSint->Qk = -1;
                    }

                    if ((RSint->Qj == -1) && (RSint->Qk == -1)) {
                        RSint->isReady = 1;
                    }
                }
                current = current->next;
            }
          
        }
        else if (i == 10||i==11||i==12||i==3||i==4||i==5) {
            //printf("REvisrs entered loop2\n");
            while (current) {
                RSfloat = current->value->value;
                if (RSfloat->isReady == 0) {
                    if (RSfloat->Qj == instruction->ROB_number) {
                        RSfloat->Vj = instruction->fpResult;
                        RSfloat->Qj = -1;
                    }
                    if (RSfloat->Qk == instruction->ROB_number) {
                        RSfloat->Vk = instruction->fpResult;
                        RSfloat->Qk = -1;
                    }
                    if ((RSfloat->Qj == -1) && (RSfloat->Qk == -1)) {
                        RSfloat->isReady = 1;
                    }
                }
                current = current->next;
            }

        }
        else if (i == 8 || i == 1) {
            //printf("REvisrs entered loop3\n");
            while (current) {
                RSfloat = current->value->value;
                if (RSfloat->isReady == 0) {
                    if (RSfloat->Qj == instruction->ROB_number) {
                        RSfloat->Vj = instruction->intResult;
                        RSfloat->imme=instruction->instruction->imme;
                        RSfloat->Qj = -1;
                    }
                    if (RSfloat->Qj == -1) {
                        //printf("MADE IT 6");
                        RSfloat->isReady = 1;
                    }
                }
                current = current->next;
            }
        }
        else {
            //printf("REvisrs entered loop4\n");
            while (current) {
                //store buffer
                RSfloat = current->value->value;
                if (RSfloat->isReady == 0) {
                    if (RSfloat->Qj == instruction->ROB_number) {
                        RSfloat->Vj = instruction->intResult;
                        RSfloat->Qj = -1;
                        RSfloat->imme=instruction->instruction->imme;
                    }
                    if (RSfloat->Qk == instruction->ROB_number) {
                        RSfloat->Vk = instruction->fpResult;
                        RSfloat->Qk = -1;
                    }
                    if ((RSfloat->Qj == -1) && (RSfloat->Qk == -1)) {
                        RSfloat->isReady = 1;
                    }
                }
                current = current->next;
            }
        }

    }
    //printf("REvised\n");
}


// insert ouput results from execute stage into write back buffer
void memStage(ExecutedInstr** results)
{
    ExecutedInstr* instruction;
    if (results == NULL) {
        return;
    }
    if (processor->WBBuff == NULL)
    {
        processor->WBBuff = createDictionary(hashInstrAddr, compareAddress);
    }
    int k = 0;
    int* ROB_number = (int*)malloc(sizeof(int));
    for (k = 0; k < 6; k++) {
        if (results[k] != NULL) {
            instruction = results[k];
            *ROB_number = instruction->ROB_number;
            if (k == 1) {
                opCode_ op = instruction->instruction->op;
                if ((strcmp(getInstructionName(op), "fsd") == 0))
                {
                    instruction->instruction->memcycle = processor->cycle;
                }
            }
            addDictionaryEntry(processor->WBBuff, ROB_number, instruction);

        }
    }
    //printf("\n mem stage done");

}





//write back stage
void WB(int returncount)
{
    int NBC= NB-returncount;//cdb lines available/left to use
    DictionaryEntry* current = processor->WBBuff->head;

    ExecutedInstr* instruction;
    ROB* ROBentry;
    int* key = (int*)malloc(sizeof(int));
    int loadOnCDB = returncount;
    //while WBBuff is not empty and cdb busses available
    while(current != NULL && NBC > 0) {
        loadOnCDB+=1;
        instruction = (ExecutedInstr*)current->value->value;
        int j = 0;
        *key = instruction->ROB_number;
        if (processor->reorderBuffer != NULL) {
            //fetch the top entry in ROB
            ROBentry = processor->reorderBuffer->items[processor->reorderBuffer->front];
            while (j < processor->reorderBuffer->size) {//traverse through ROB
                if (ROBentry->DestRenameReg == instruction->ROB_number) {//if matched
                    ROBentry->state = "W";//write state
                    // if (ROBentry->instruction->op == FLD){
                    //     ROBentry->isReady = 6;
                    //     printf("\nmade load 6\n");
                    // }
                    // else{
                    ROBentry->isReady = 1;//we have wriiten back so this register is now available for use by next instrtuctions
                    // }
                    if (ROBentry->instruction->op == FSD) {
                        ROBentry->DestAddr = instruction->address;

                    }
                }

                j++;
                ROBentry = processor->reorderBuffer->items[(processor->reorderBuffer->front + j) % processor->reorderBuffer->maxsize];
            }
        }
        //printf("writeback line 1426\n");
        //update the reservation stations for the next cycle.
        reviseRS(instruction);
        instruction->instruction->wbcycle = processor->cycle;
        //wriiten back, so remove from the buffer
        removeDictionaryEntriesByKey(processor->WBBuff, key);
        NBC--;
        current = current->next;
    }



    //pop From Temp Buffers
    int i = 0;
	for (i = 0; i < 7; i++) {
		DictionaryEntry* exoutputs = popDictionaryEntry(processor->resv[i]);
		while (exoutputs) {
			appendDictionaryEntry(processor->resv[i + 7], exoutputs);
			exoutputs = popDictionaryEntry(processor->resv[i]);
		}
	}
    printf("Count on CDB is %d\n", loadOnCDB);
    //for calculating average cdb usage
    processor->cdb_usage += (float)loadOnCDB * (100 / NB);

}