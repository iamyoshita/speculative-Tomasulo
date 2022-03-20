/*
 * PrintUtility.c
 *
 *  Created on: Oct 2, 2015
 *      Author: DebashisGanguly
 */

#include "../TomasuloSimulator.h"

/**
 * Utility method to print instruction cache
 */
void printInstructionCache () {
	DictionaryEntry *current;

	if (instructionCache != NULL) {
		printf ("-----------------\n");
		printf ("Instruction Cache\n");
		printf ("-----------------\n");

		for (current = instructionCache -> head; current != NULL; current = current -> next)
			printf ("%d: %s\n", *((int*)current -> key), ((char*)current -> value -> value));
	}
}

/***
 * Utility method to print code lables
 */
void printCodeLabels () {
	DictionaryEntry *current;

	if (codeLabels != NULL) {
		printf ("-----------\n");
		printf ("Code Lables\n");
		printf ("-----------\n");

		for (current = codeLabels -> head; current != NULL; current = current -> next)
			printf ("%s: %d\n", ((char*)current -> key), *((int*)current -> value -> value));
	}
}

/***
 * Utility method to print data cache
 */
void printDataCache () {
	DictionaryEntry *current;

	if (dataCache != NULL) {
		printf ("----------\n");
		printf ("Data Cache\n");
		printf ("----------\n");

		for (current = dataCache -> head; current != NULL; current = current -> next)
			printf ("Mem(%d) = %0.1lf\n", *((int*)current -> key), *((double*)current -> value -> value));
	}
}






/**
 * Utility method to print integer registers
 */
void printIntegerRegisters () {
	int i;

	printf ("-----------------\n");
	printf ("Integer Registers\n");
	printf ("-----------------\n");

	for (i = 0; i < numberOfIntRegisters; i++) {
		printf ("Int_Reg[%d]=%d\n", i, cpu -> integerRegisters [i] -> data);
	}
}



/***
 * Utility method to print floating point registers
 */
void printFPRegisters () {
	int i;

	printf ("------------------------\n");
	printf ("Floating Point Registers\n");
	printf ("------------------------\n");

	for (i = 0; i < numberOfFPRegisters; i++) {
		printf ("FP_Reg[%d]=%0.1lf\n", i, cpu -> floatingPointRegisters [i] -> data);
	}
}



void printIntegerRegistersStatus () {
	int i;

	printf ("-----------------\n");
	printf ("Integer Registers Status\n");
	printf ("-----------------\n");

	for (i = 0; i < numberOfIntRegisters; i++) {
	    if(cpu -> IntRegStatus [i]->busy == 1)
		    printf ("Int_Reg[%d] is busy with reorder buffer number %d\n", i, cpu -> IntRegStatus [i] -> reorderNum);
	}
}

void printFPRegistersStatus () {
	int i;

	printf ("-----------------\n");
	printf ("Floating Point Registers Status\n");
	printf ("-----------------\n");

	for (i = 0; i < numberOfIntRegisters; i++) {
	    if(cpu -> FPRegStatus [i]->busy == 1)
		    printf ("FP_Reg[%d] is busy with reorder buffer number %d\n", i, cpu -> FPRegStatus [i] -> reorderNum);
	}
}


void printFetchBuffer () {
	DictionaryEntry *current;

	if (cpu -> fetchBuffer != NULL) {
		printf ("-----------------\n");
		printf ("Fetch Buffer\n");
		printf ("-----------------\n");

		for (current = cpu -> fetchBuffer -> head; current != NULL; current = current -> next)
			printf ("%d: %s\n", *((int*)current -> key), ((char*)current -> value -> value));
	}
}

void printInstructionQueue () {

	if (cpu -> instructionQueue != NULL) {
		printf ("-----------------\n");
		printf ("Instruction Queue\n");
		printf ("-----------------\n");

        Instruction *instruction;
        int i;
        int i_max = cpu->instructionQueue->head + cpu->instructionQueue->count;
        int index;
        for(i=cpu->instructionQueue->head; i<i_max; i++){
            index = i % cpu->instructionQueue -> size;
            instruction = cpu->instructionQueue->items[index];
            printf("Instruction %d: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        }
	}
}


void printInstructionQueueResult () {

	if (cpu -> instructionQueueResult != NULL) {
		printf ("-----------------\n");
		printf ("Instruction Queue Result\n");
		printf ("-----------------\n");

        Instruction *instruction;
        int i;
        int i_max = cpu->instructionQueueResult->head + cpu->instructionQueueResult->count;
        int index;
        for(i=cpu->instructionQueueResult->head; i<i_max; i++){
            index = i % cpu->instructionQueueResult -> size;
            instruction = cpu->instructionQueueResult->items[index];
            printf("Instruction %d: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        }
	}
}

void printRenamingRegisters () {
	DictionaryEntry *current;

	if (cpu -> renameRegInt != NULL) {
		printf ("-----------------\n");
		printf ("Renaming Integer Registers\n");
		printf ("-----------------\n");
		for (current = cpu -> renameRegInt -> head; current != NULL; current = current -> next){
			printf ("Renaming register number %d: value %d\n", *((int*)current -> key), *((int*)current -> value -> value));
	    }
	}
	if (cpu -> renameRegFP != NULL) {
		printf ("-----------------\n");
		printf ("Renaming Floating Pointer Registers\n");
		printf ("-----------------\n");
		for (current = cpu -> renameRegFP -> head; current != NULL; current = current -> next){
			printf ("Renaming register number %d: value %f\n", *((int*)current -> key), *((double*)current -> value -> value));
	    }
	}
}


void printReservationStations () {
	if (cpu -> resStaInt != NULL) {
		printf ("-----------------\n");
		printf ("Reservation Station INT\n");
		printf ("-----------------\n");
		printf("Instruction address|  Op code  |  Vj  |  Vk  |  Qj  |  Qk  |Destination ROB#|isReady|isExecuting\n");
        printf ("------------------------------------------------------------------------------------------------\n");

        DictionaryEntry *current;
        RSint *RS;
		for (current = cpu -> resStaInt -> head; current != NULL; current = current -> next){
		    RS = current -> value -> value;
		    printf("%d\t\t\t%s\t\t%d\t\t%d\t\t%d\t\t%d\t\t\t%d\t\t\t%d\t\t%d\n", RS->instruction->address, getOpcodeString ((int)RS->instruction->op),
		    RS->Vj, RS->Vk, RS->Qj, RS->Qk, RS->Dest, RS->isReady, RS->isExecuting);
		}
	}
	if (cpu -> resStaFPadd != NULL) {
		printf ("-----------------\n");
		printf ("Reservation Station FPadd\n");
		printf ("-----------------\n");
		printf("Instruction address|  Op code  |  Vj  |  Vk  |  Qj  |  Qk  |Destination ROB#|isReady|isExecuting\n");
        printf ("------------------------------------------------------------------------------------------------\n");

        DictionaryEntry *current;
        RSfloat *RS;
		for (current = cpu -> resStaFPadd -> head; current != NULL; current = current -> next){
		    RS = current -> value -> value;
		    printf("%d\t\t\t%s\t\t%.1f\t\t%.1f\t\t%d\t\t%d\t\t\t%d\t\t\t%d\t\t%d\n", RS->instruction->address, getOpcodeString ((int)RS->instruction->op),
		    RS->Vj, RS->Vk, RS->Qj, RS->Qk, RS->Dest, RS->isReady, RS->isExecuting);
		}
	}
	if (cpu -> resStaFPmult != NULL) {
		printf ("-----------------\n");
		printf ("Reservation Station FPmult\n");
		printf ("-----------------\n");
		printf("Instruction address|  Op code  |  Vj  |  Vk  |  Qj  |  Qk  |Destination ROB#|isReady|isExecuting\n");
        printf ("------------------------------------------------------------------------------------------------\n");

        DictionaryEntry *current;
        RSfloat *RS;
		for (current = cpu -> resStaFPmult -> head; current != NULL; current = current -> next){
		    RS = current -> value -> value;
		    printf("%d\t\t\t%s\t\t%.1f\t\t%.1f\t\t%d\t\t%d\t\t\t%d\t\t\t%d\t\t%d\n", RS->instruction->address, getOpcodeString ((int)RS->instruction->op),
		    RS->Vj, RS->Vk, RS->Qj, RS->Qk, RS->Dest, RS->isReady, RS->isExecuting);
		}
	}
	if (cpu -> resStaFPdiv != NULL) {
		printf ("-----------------\n");
		printf ("Reservation Station FPdiv\n");
		printf ("-----------------\n");
		printf("Instruction address|  Op code  |  Vj  |  Vk  |  Qj  |  Qk  |Destination ROB#|isReady|isExecuting\n");
        printf ("------------------------------------------------------------------------------------------------\n");

        DictionaryEntry *current;
        RSfloat *RS;
		for (current = cpu -> resStaFPdiv -> head; current != NULL; current = current -> next){
		    RS = current -> value -> value;
		    printf("%d\t\t\t%s\t\t%.1f\t\t%.1f\t\t%d\t\t%d\t\t\t%d\t\t\t%d\t\t%d\n", RS->instruction->address, getOpcodeString ((int)RS->instruction->op),
		    RS->Vj, RS->Vk, RS->Qj, RS->Qk, RS->Dest, RS->isReady, RS->isExecuting);
		}
	}

	if (cpu -> resStaBU != NULL) {
		printf ("-----------------\n");
		printf ("Reservation Station BU\n");
		printf ("-----------------\n");
		printf("Instruction address|  Op code  |  Vj  |  Vk  |  Qj  |  Qk  |Destination ROB#|isReady|isExecuting\n");
        printf ("------------------------------------------------------------------------------------------------\n");

        DictionaryEntry *current;
        RSint *RS;
		for (current = cpu -> resStaBU -> head; current != NULL; current = current -> next){
		    RS = current -> value -> value;
		    printf("%d\t\t\t%s\t\t%d\t\t%d\t\t%d\t\t%d\t\t\t%d\t\t\t%d\t\t%d\n", RS->instruction->address, getOpcodeString ((int)RS->instruction->op),
		    RS->Vj, RS->Vk, RS->Qj, RS->Qk, RS->Dest, RS->isReady, RS->isExecuting);
		}
	}

	if (cpu -> loadBuffer != NULL) {
		printf ("-----------------\n");
		printf ("Load Buffer\n");
		printf ("-----------------\n");
		printf("Instruction address|  Op code  |  Vj  |  iVk  |  fpVk |  Qj  |  Qk  |Destination ROB#|isReady|isExecuting\n");
        printf ("--------------------------------------------------------------------------------------------------------\n");

        DictionaryEntry *current;
        RSmem *RS;
		for (current = cpu -> loadBuffer -> head; current != NULL; current = current -> next){
		    RS = current -> value -> value;
		    printf("%d\t\t\t%s\t\t%d\t\t%d\t\t%.1f\t\t%d\t\t%d\t\t\t%d\t\t\t%d\t\t%d\n", RS->instruction->address, getOpcodeString ((int)RS->instruction->op),
		    RS->Vj, RS->iVk, RS->fpVk, RS->Qj, RS->Qk, RS->Dest, RS->isReady, RS->isExecuting);
		}
	}

	if (cpu -> storeBuffer != NULL) {
		printf ("-----------------\n");
		printf ("Store Buffer\n");
		printf ("-----------------\n");
		printf("Instruction address|  Op code  |  Vj  |  iVk  |  fpVk |  Qj  |  Qk  |Destination ROB#|isReady|isExecuting\n");
        printf ("--------------------------------------------------------------------------------------------------------\n");

        DictionaryEntry *current;
        RSmem *RS;
		for (current = cpu -> storeBuffer -> head; current != NULL; current = current -> next){
		    RS = current -> value -> value;
		    printf("%d\t\t\t%s\t\t%d\t\t%d\t\t%.1f\t\t%d\t\t%d\t\t\t%d\t\t\t%d\t\t%d\n", RS->instruction->address, getOpcodeString ((int)RS->instruction->op),
		    RS->Vj, RS->iVk, RS->fpVk, RS->Qj, RS->Qk, RS->Dest, RS->isReady, RS->isExecuting);
		}
	}

}


void printROB()
{
	ROB *ROBentry;
	int i = 0;
	if(cpu->reorderBuffer != NULL)
	{
		printf ("-----------------\n");
		printf ("Reorder Buffer\n");
		printf ("-----------------\n");
		printf("ROB_number  |Instr add|State| Dest Reg | DestAddr | isReady  |  isIntegerReg | isStore | isBranch   | isAfterBranch  | isCorrectPredict\n");

			printf ("--------------------------------------------------------------------------------------------------------------------------------------\n");

        ROBentry = getHeadCircularQueue(cpu -> reorderBuffer);

        //while(ROBentry != NULL){
        while (i < cpu->reorderBuffer->count) {
            printf("%d\t\t%d\t%s\t%d\t\t%d\t%d\t%d\t\t%d\t\t%d\t%d\t\t%d\n", (cpu-> reorderBuffer -> head + i)%cpu->reorderBuffer->size, ROBentry->instruction->address , ROBentry->state, ROBentry -> DestReg, ROBentry ->DestAddr,  ROBentry -> isReady, ROBentry->isINT, ROBentry ->isStore,ROBentry ->  isBranch,ROBentry -> isAfterBranch, ROBentry ->  isCorrectPredict);
            i++;
            ROBentry = cpu->reorderBuffer -> items[(cpu-> reorderBuffer -> head + i)%cpu->reorderBuffer->size];

		}
	}
}


void printWriteBackBuffer () {
	DictionaryEntry *current;

	if (cpu -> WriteBackBuffer != NULL) {
		printf ("-----------------\n");
		printf ("Write Back Buffer\n");
		printf ("-----------------\n");

		for (current = cpu -> WriteBackBuffer -> head; current != NULL; current = current -> next)
			printf ("%d: \n", *((int*)current -> key)); //getOpcodeString ((int) (current -> value -> value -> instruction -> op)));
	}
}

