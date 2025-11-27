#include "nrcpu.h"
#include "nearerpc.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SKIP_PER_INSTR 4


static inline boolean check_if_valid_register(uint64_t reg){
    if(reg >= REG_AMOUNT) return FALSE;
    return TRUE;
}

static inline boolean begin_virtual_machine(VmProgram* program){
    program->vmMem.memory = (uint64_t*)malloc(program->vmMem.memorySize * sizeof(uint64_t));
    if(program->vmMem.memory == NULL) return FALSE;
    memset(program->registers, 0, REG_AMOUNT * sizeof(uint64_t));
    if(program->vmCode.codeSize == 0){
        uint64_t* arr = program->vmCode.code;
        while(*arr != OP_HALT){
            program->vmCode.codeSize++;
            arr++;
        }
    }
    program->programCounter = 0;
    memset(program->vmMem.memory, 0, program->vmMem.memorySize * sizeof(uint64_t));
    program->isCurrentlyRunning = TRUE;
    return TRUE;
}

static inline void end_virtual_machine(VmProgram* program, char* reason){
    if(program->vmMem.memory != NULL){
        free((uint64_t*)program->vmMem.memory);
    }
    program->programCounter = 0;
    program->vmCode.codeSize = 0;
    program->isCurrentlyRunning = FALSE;
    printf("This program has been killed: %s\n", reason);
}

boolean vm_execute_program(VmProgram* program){
    if(program->isCurrentlyRunning == FALSE){
        boolean result = begin_virtual_machine(program);
        if(result == FALSE){
            end_virtual_machine(program, "Couldn't begin the virtual machine");
            return result;
        }
    }
    printf("codeSize is %zu, programCounter is %zu\n", program->vmCode.codeSize, program->programCounter);

    while((program->programCounter + 3) < program->vmCode.codeSize){
        uint64_t op = program->vmCode.code[program->programCounter];
        if(op == OP_HALT) break;
        uint64_t arg1 = program->vmCode.code[program->programCounter + 1];
        uint64_t arg2 = program->vmCode.code[program->programCounter + 2];
        uint64_t arg3 = program->vmCode.code[program->programCounter + 3];

        switch(op){
            case OP_ADD:{
                if(check_if_valid_register(arg1) == FALSE ||
                    check_if_valid_register(arg2) == FALSE ||
                    check_if_valid_register(arg3) == FALSE)
                {
                    end_virtual_machine(program, "Invalid register in OP_ADD\n");
                    return FALSE;
                }
                program->registers[arg1] = program->registers[arg2] + program->registers[arg3];
                program->programCounter += SKIP_PER_INSTR;
                break;
            }
            case OP_SUB:{
                if(check_if_valid_register(arg1) == FALSE ||
                    check_if_valid_register(arg2) == FALSE ||
                    check_if_valid_register(arg3) == FALSE)
                {
                    end_virtual_machine(program, "Invalid register in OP_SUB\n");
                    return FALSE;
                }
                program->registers[arg1] = program->registers[arg2] - program->registers[arg3];
                program->programCounter += SKIP_PER_INSTR;
                break;
            }
            case OP_MUL:{
                if(check_if_valid_register(arg1) == FALSE ||
                    check_if_valid_register(arg2) == FALSE ||
                    check_if_valid_register(arg3) == FALSE)
                {
                    end_virtual_machine(program, "Invalid register in OP_MUL\n");
                    return FALSE;
                }
                program->registers[arg1] = program->registers[arg2] * program->registers[arg3];
                program->programCounter += SKIP_PER_INSTR;
                break;
            }
            case OP_DIV:{
                if(check_if_valid_register(arg1) == FALSE ||
                    check_if_valid_register(arg2) == FALSE ||
                    check_if_valid_register(arg3) == FALSE)
                {
                    end_virtual_machine(program, "Invalid register in OP_DIV\n");
                    return FALSE;
                }
                if(program->registers[arg3] == 0){
                    program->registers[arg1] = 0;
                } else {
                    program->registers[arg1] = program->registers[arg2] / program->registers[arg3];
                }
                program->programCounter += SKIP_PER_INSTR;
                break;
            }
            case OP_MOV:{
                if(check_if_valid_register(arg1) == FALSE || check_if_valid_register(arg2) == FALSE){
                    end_virtual_machine(program, "Invalid register in OP_MOV\n");
                    return FALSE;
                }
                program->registers[arg1] = program->registers[arg2];
                program->programCounter += SKIP_PER_INSTR;
                break;
            }
            case OP_MOVI:{
                if(check_if_valid_register(arg1) == FALSE){
                    end_virtual_machine(program, "Invalid register in OP_MOVI\n");
                    return FALSE;
                }

                program->registers[arg1] = arg2;
                program->programCounter += SKIP_PER_INSTR;
                break;
            }
            case OP_VMCALL:{
                switch (arg1) {
                    case 0:{
                        if(check_if_valid_register(arg2) == FALSE){
                            end_virtual_machine(program, "Invalid register in OP_VMCALL vmcall 0\n");
                            return FALSE;
                        }
                        printf("x%lu has %lu\n", arg2, program->registers[arg2]);
                        break;
                    }
                    default:{
                        end_virtual_machine(program, "Invalid VMCALL\n");
                        return FALSE;
                    }
                }
                program->programCounter += SKIP_PER_INSTR;
                break;
            }
            case OP_STORE:{
                if(check_if_valid_register(arg1) == FALSE){
                    end_virtual_machine(program, "Invalid register in OP_STORE\n");
                    return FALSE;
                }
                if(program->vmMem.memorySize <= arg2){
                    end_virtual_machine(program, "Attempted memory Overflow\n");
                    return FALSE;
                }
                program->vmMem.memory[arg2] = program->registers[arg1];
                program->programCounter += SKIP_PER_INSTR;
                break;
            }
            case OP_LOAD:{
                if(check_if_valid_register(arg2) == FALSE){
                    end_virtual_machine(program, "Invalid register in OP_LOAD\n");
                    return FALSE;
                }
                if(program->vmMem.memorySize <= arg1){
                    end_virtual_machine(program, "Attempted load in address further away from memory\n");
                    return FALSE;
                }
                program->registers[arg2] = program->vmMem.memory[arg1];
                program->programCounter += SKIP_PER_INSTR;
                break;
            }
            case OP_JMP:{
                if(arg1 >= program->vmCode.codeSize){
                    end_virtual_machine(program, "Invalid JMP in OP_JMP, greater then code size");
                    return FALSE;
                }
                program->programCounter = arg1;
                break;
            }
            case OP_JEQ:{
                if(arg1 >= program->vmCode.codeSize){
                    end_virtual_machine(program, "Invalid JMP in OP_JEQ, greater then code size");
                    return FALSE;
                }
                if(check_if_valid_register(arg2) == FALSE || check_if_valid_register(arg3) == FALSE){
                    end_virtual_machine(program, "Invalid register in OP_JEQ\n");
                    return FALSE;
                }
                if(program->registers[arg2] == program->registers[arg3]){
                    program->programCounter = arg1;
                } else {
                    program->programCounter += SKIP_PER_INSTR;
                }
                break;
            }
            case OP_JNEQ:{
                if(arg1 >= program->vmCode.codeSize){
                    end_virtual_machine(program, "Invalid JMP in OP_JNEQ, greater then code size");
                    return FALSE;;
                }
                if(check_if_valid_register(arg2) == FALSE || check_if_valid_register(arg3) == FALSE){
                    end_virtual_machine(program, "Invalid register in OP_JNEQ\n");
                    return FALSE;
                }
                if(program->registers[arg2] != program->registers[arg3]){
                    program->programCounter = arg1;
                } else {
                    program->programCounter += SKIP_PER_INSTR;
                }
                break;
            }
            case OP_JGT:{
                if(arg1 >= program->vmCode.codeSize){
                    end_virtual_machine(program, "Invalid JMP in OP_JGT, greater then code size");
                    return FALSE;
                }
                if(check_if_valid_register(arg2) == FALSE || check_if_valid_register(arg3) == FALSE){
                    end_virtual_machine(program, "Invalid register in OP_JGT\n");
                    return FALSE;
                }
                if(program->registers[arg2] > program->registers[arg3]){
                    program->programCounter = arg1;
                } else {
                    program->programCounter += SKIP_PER_INSTR;
                }
                break;
            }
            case OP_JLT:{
                if(arg1 >= program->vmCode.codeSize){
                    end_virtual_machine(program, "Invalid JMP in OP_JLT, greater then code size");
                    return FALSE;
                }
                if(check_if_valid_register(arg2) == FALSE || check_if_valid_register(arg3) == FALSE){
                    end_virtual_machine(program, "Invalid register in OP_JLT\n");
                    return FALSE;
                }
                if(program->registers[arg2] < program->registers[arg3]){
                    program->programCounter = arg1;
                } else {
                    program->programCounter += SKIP_PER_INSTR;
                }
                break;
            }
            case OP_CMP:{
                if(check_if_valid_register(arg1) == FALSE ||
                    check_if_valid_register(arg2) == FALSE ||
                    check_if_valid_register(arg3) == FALSE)
                {
                    end_virtual_machine(program, "Invalid register in OP_CMP\n");
                    return FALSE;
                }
                if(program->registers[arg2] == program->registers[arg3]){
                    program->registers[arg1] = 1;
                } else {
                    program->registers[arg1] = 0;
                }
                break;
            }
            case OP_CMPNE:{
                if(check_if_valid_register(arg1) == FALSE ||
                    check_if_valid_register(arg2) == FALSE ||
                    check_if_valid_register(arg3) == FALSE)
                {
                    end_virtual_machine(program, "Invalid register in OP_CMPNE\n");
                    return FALSE;
                }
                if(program->registers[arg2] != program->registers[arg3]){
                    program->registers[arg1] = 1;
                } else {
                    program->registers[arg1] = 0;
                }
                break;
            }
            case OP_CMPGT:{
                if(check_if_valid_register(arg1) == FALSE ||
                    check_if_valid_register(arg2) == FALSE ||
                    check_if_valid_register(arg3) == FALSE)
                {
                    end_virtual_machine(program, "Invalid register in OP_CMPGT\n");
                    return FALSE;
                }
                if(program->registers[arg2] >= program->registers[arg3]){
                    program->registers[arg1] = 1;
                } else {
                    program->registers[arg1] = 0;
                }
                break;
            }
            case OP_CMPLT:{
                if(check_if_valid_register(arg1) == FALSE ||
                    check_if_valid_register(arg2) == FALSE ||
                    check_if_valid_register(arg3) == FALSE)
                {
                    end_virtual_machine(program, "Invalid register in OP_CMPLT\n");
                    return FALSE;
                }
                if(program->registers[arg2] <= program->registers[arg3]){
                    program->registers[arg1] = 1;
                } else {
                    program->registers[arg1] = 0;
                }
                break;
            }
            default:{
                printf("%lu is INVALID!!!\n", op);
                end_virtual_machine(program, "Invalid Instruction\n");
                return FALSE;
            }

        }
    }
    end_virtual_machine(program, "Program Exited\n");
    return TRUE;
}
