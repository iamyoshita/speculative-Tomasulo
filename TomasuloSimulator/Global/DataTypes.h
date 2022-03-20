/*
 * DataTypes.h
 *
 *  Created on: Sep 29, 2015
 *      Author: DebashisGanguly
 */

#ifndef GLOBAL_DATATYPES_H_
#define GLOBAL_DATATYPES_H_

#include "./ADT/CircularQueue.h"
#include "./ADT/Dictionary.h"

typedef enum _opCode {
    ADD,
	ADDI,
	FLD,
	FSD,
	FADD,
	FSUB,
	FMUL,
	FDIV,
	BNE,
	NOOP
} OpCode;

typedef enum _unit {
    INT,
    LS,
    FPadd,
    FPmult,
    FPdiv,
    BU
} Unit;

static inline char *getOpcodeString (int opcode) {
	char *opcodeString[10] = {"add", "addi","fld", "fsd", "fadd","fsub", "fmul", "fdiv", "bne", "NOOP"};

	return opcodeString[opcode];
}

//data structure for decoded instruction
typedef struct _instruction {
	OpCode op;
	int address; //Instruction address (PC)

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
} Instruction;

//data structure to bind instruction and its result in pipeline
typedef struct _completedInstruction {
    Instruction *instruction;
    int intResult;
    double fpResult;
    int address; //used to keep address for SD and S.D
    int isCorrectPredict;
    int ROB_number;
} CompletedInstruction;

//data structure for each item in integer registers and work as well register status
typedef struct _INTReg {
	int data;
	int intResult;
} INTReg;

//data structure for each item in floating point registers and work as well register status
typedef struct _FPReg {
	double data;
        double fpResult;
} FPReg;


// data structure for Reorder Buffer
typedef struct _ROB{
	Instruction * instruction;
	char * state;
	int DestReg; //Destination register number
	int DestRenameReg; //Destination renaming register number
	int isReady;
	int isINT;
	int isStore;
	int DestAddr; // optional, memory address to write
	int isBranch;
	int isAfterBranch;
	int isCorrectPredict;
}ROB;

//Data structure for reservation stations
typedef struct _RSint{
	Instruction *instruction;
	int Vj; //value of input register j
	int Vk; //value of input register k
	int Qj; //ROB number of input register j
	int Qk;  //ROB number of input register k
	int Dest; //ROB number of destination register
	int isReady;
	int isExecuting;
}RSint;

typedef struct _RSfloat{
	Instruction *instruction;
	double Vj; //value of input register j
	double Vk; //value of input register k
	int Qj; //ROB number of input register j
	int Qk;  //ROB number of input register j
	int Dest; //ROB number of destination register
	int isReady;
	int isExecuting;
}RSfloat;

typedef struct _RSmem{
    Instruction *instruction;
    int Vj; //value of input register j
    int address; //address for memory operation (Vj + instruction -> immediate)
    int iVk; //value of input register k if int
    double fpVk; //value of input register k if float
    int Qj; //ROB number of input register j
	int Qk;  //ROB number of input register k
	int Dest; //ROB number of destination register
	int isReady;
	int isExecuting;
} RSmem;

//Data Structure for register status table
typedef struct _RegStatus{
    int reorderNum;
    int busy;
}RegStatus;

//Data structure for reservation station key
typedef struct _keyRS{
    int reorderNum;
    int progNum; // 1 or 2 on our case
}KeyRS;


//main data structure representing CPU
typedef struct _cpu {
	int cycle; //counter for number of cycles

	int numberOfInstructionExecuted; //counter for total number of instructions committed

	int PC; //program counter
	int PC2;
	//doubled

	INTReg **integerRegisters; //integer register
    FPReg **floatingPointRegisters; //FP registers
    //doubled
    //Fetch and decode
    Dictionary *fetchBuffer;
    Dictionary *fetchBufferResult;
    CircularQueue *instructionQueue;
    CircularQueue *instructionQueueResult;
    Dictionary *branchTargetBuffer;
    //doubled
    //Reorder buffer
    CircularQueue *reorderBuffer;
	Dictionary *WriteBackBuffer;
	
    //doubled
    //Reservation station
    Dictionary *resStaInt;
    Dictionary *resStaFPadd;
    Dictionary *resStaFPmult;
    Dictionary *resStaFPdiv;
    Dictionary *resStaBU;
 	//Load and Store buffer
    Dictionary *loadBuffer;
    Dictionary *storeBuffer;

    Dictionary *resStaIntResult;
    Dictionary *loadBufferResult;
    Dictionary *storeBufferResult;
    Dictionary *resStaFPaddResult;
    Dictionary *resStaFPmultResult;
    Dictionary *resStaFPdivResult;
    Dictionary *resStaBUResult;
    //Renaming registers
    Dictionary *renameRegInt;
    Dictionary *renameRegFP;
    //doubled
	//Register status table
    RegStatus **IntRegStatus;
    RegStatus **FPRegStatus;
    //doubled
    //Pipelines
    CircularQueue *INTPipeline;
    CircularQueue *LoadStorePipeline;
    CircularQueue *FPaddPipeline;
    CircularQueue *FPmultPipeline;
    CircularQueue *FPdivPipeline;
    int FPdivPipelineBusy;
    CircularQueue *BUPipeline;

    //Install flag
    int stallNextFetch;
    //doubled
    //Stall counter
    int stallFullROB;
    //doubled
    int stallFullRS;
    //Flag of instructions after a predicted branch.
    int isAfterBranch;
    //doubled

	//CDB utilization
	int percentutilizationpercycle;
	float avgutilization;
	
    //Flag of program that last cycle fetch and next cycle decode
    int lastCycleFetchProgram;
    int nextCycleDecodeProgram;
	
	
	//Commit counter
	int commitCounter;

} CPU;

#endif /* GLOBAL_DATATYPES_H_ */
