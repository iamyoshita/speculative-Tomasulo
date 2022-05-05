#ifndef headerfile
#define headerfile

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "CircularQueue.h"
extern int NF; //4
extern int NW;//4
extern int NR;//16
extern int NB;//4
extern int startAddress;


int lineNumber;


Dictionary *instructionMemory;
Dictionary *dataMemory;
Dictionary *branchLabels;

void initializeProcessor ();
void displayRenamedReg ();
void displayTomasuloTable();
int hashInstrAddr(void* addr);
int compareAddress(void* addr1, void* addr2);
int compareInstructions(void* i1, void* i2);
int hashBranchAddr(void* addr);

DictionaryValue* fifoGetFromRSList(DictionaryEntry* current, int rstype);
typedef enum opCode {
    FLD,
    FSD,
    ADD,
    ADDI,
    FADD,
    FSUB,
    FMUL,
    FDIV,
    BNE,
    NOP
}opCode_;

enum unit {
    INT,
    LS,
    FPadd,
    FPmult,
    FPdiv,
    BU
};
enum unit unitType;
static char *getInstructionName (int instr) {
	char *instrname[10] = { "fld", "fsd","add", "addi", "fadd","fsub", "fmul", "fdiv", "bne", "nop"};
	return instrname[instr];
}

typedef struct instructionstruct {
    int address;
	
    int fetchcycle;
    int decodecycle;
    int issuecycle;
    int excycle;
    int memcycle;
    int wbcycle;
    int commitcycle;

    opCode_ op;

	int rs;
	int rt;
    int rd;

	int fs;
	int ft;
    int fd;

	int imme;

	int target;

} Instruction;

typedef struct executedInstrstruct {
    Instruction *instruction;
    int intResult;
    float fpResult;
    int address; 
    int ROB_number;
} ExecutedInstr;



typedef struct ROBstruct{
    int isReady;
	Instruction * instruction;
	char * state;
	int DestReg; 
	int DestRenameReg;
	int DestAddr;
}ROB;

typedef struct instrstringstruct {
    int pc;
    char* instrPtr;
}instrstring;

typedef struct RSintstruct{
	Instruction *instruction;
	int Vj;
	int Vk; 
	int Qj; 
	int Qk;  
	int Dest; 
	int isReady;
	int isExecuting;
}RSint;

typedef struct RSfloatstruct{
	Instruction *instruction;
	float Vj;
    int address;
	float Vk; 
	int Qj; 
	int Qk; 
	int Dest; 
	int isReady;
	int isExecuting;
    int imme;
}RSfloat;

typedef struct RegStatusstr{
    int reorderNum;
    int busy;
}RegStatus;


typedef struct forstr{
int imme;
int rs;
int robnum;
}forwarding;

typedef struct processorstr {
	int cycle;

	int lineNumberExecuted; 

	int PC; 

	int integerRegisters[32]; 
    float floatingPointRegisters[32]; 

    struct Cqueue* fdPipeline;
    struct Cqueue* fdPipelineres;
    struct Cqueue *dIPipeline;
    struct Cqueue *dIPipelineres;
    Dictionary *branchTargetBuffer;

    struct Cqueue *reorderBuffer;
    struct Cqueue *tomTable;
	Dictionary *WBBuff;

    Dictionary* resv[14];

    Dictionary *renameReg;

    RegStatus **registerstatus;
    forwarding immeaddr[64];
int previmme;
int prevrs;
    struct Cqueue *INTPipeline;
    struct Cqueue *LoadStorePipeline;
    struct Cqueue *FPaddPipeline;
    struct Cqueue *FPmultPipeline;
    struct Cqueue *FPdivPipeline;
    int FPdivPipelineBusy;
    int FPaddPipelineBusy;
    int FPmulPipelineBusy;
    struct Cqueue *BUPipeline;

    int stallNextFetch;
    int stallFullROB;
    int stallFullRS;
	float cdb_usage;		
	int totalcommits;
    int StallIssue;

} Processor;

extern Processor* processor;

#endif