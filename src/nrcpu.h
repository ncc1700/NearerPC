#ifndef NRCPU_H_INCLUDED
#define NRCPU_H_INCLUDED














#include <stddef.h>
#include <stdint.h>
#include "nearerpc.h"
#include "nrmemory.h"

#define REG_AMOUNT 16

typedef enum _VmOpCodes {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOV,
    OP_MOVI,
    OP_VMCALL,
    OP_HALT,
    OP_STORE,
    OP_LOAD,
    OP_JMP,
    OP_JEQ,
    OP_JNEQ,
    OP_JGT,
    OP_JLT,
    OP_CMP,
    OP_CMPNE,
    OP_CMPGT,
    OP_CMPLT,
} VmOpCodes;



typedef struct _VmCode {
    uint64_t* code;
    size_t codeSize;
} VmCode;


typedef struct _VmProgram {
    boolean isCurrentlyRunning;
    VmMemory vmMem;
    VmCode vmCode;
    int8_t privliegeLevel;
    size_t programCounter;
} VmProgram;

typedef struct _VmCPU {
    uint64_t registers[REG_AMOUNT];
    VmMemory globalMemory; // not used, might be used later
} VmCPU;


boolean vm_execute_program(VmProgram* program);







#endif
