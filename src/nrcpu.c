#include "nrcpu.h"
#include "nearerpc.h"
#include "nrmemory.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SKIP_PER_INSTR 4

static VmCPU vmCPU;

static inline boolean check_if_valid_register(uint64_t reg){
    if(reg >= REG_AMOUNT) return FALSE;
    return TRUE;
}

static inline boolean begin_virtual_machine(VmProgram* program){
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
    program->programCounter = 0;
    program->vmCode.codeSize = 0;
    program->isCurrentlyRunning = FALSE;
    printf("This program has been killed: %s\n", reason);
}


void vmcpu_setup_processor(){
    memset(vmCPU.registers, 0, REG_AMOUNT * sizeof(uint64_t));
    vmCPU.globalMemory = vm_return_system_memory();
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

        // prepare for the massive switch statement that composes this entire "processor"
        switch(op){
            case OP_ADD:{
                if(check_if_valid_register(arg1) == FALSE ||
                    check_if_valid_register(arg2) == FALSE ||
                    check_if_valid_register(arg3) == FALSE)
                {
                    end_virtual_machine(program, "Invalid register in OP_ADD\n");
                    return FALSE;
                }
                vmCPU.registers[arg1] = vmCPU.registers[arg2] + vmCPU.registers[arg3];
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
                vmCPU.registers[arg1] = vmCPU.registers[arg2] - vmCPU.registers[arg3];
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
                vmCPU.registers[arg1] = vmCPU.registers[arg2] * vmCPU.registers[arg3];
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
                if(vmCPU.registers[arg3] == 0){
                    vmCPU.registers[arg1] = 0;
                } else {
                    vmCPU.registers[arg1] = vmCPU.registers[arg2] / vmCPU.registers[arg3];
                }
                program->programCounter += SKIP_PER_INSTR;
                break;
            }
            case OP_MOV:{
                if(check_if_valid_register(arg1) == FALSE || check_if_valid_register(arg2) == FALSE){
                    end_virtual_machine(program, "Invalid register in OP_MOV\n");
                    return FALSE;
                }
                vmCPU.registers[arg1] = vmCPU.registers[arg2];
                program->programCounter += SKIP_PER_INSTR;
                break;
            }
            case OP_MOVI:{
                if(check_if_valid_register(arg1) == FALSE){
                    end_virtual_machine(program, "Invalid register in OP_MOVI\n");
                    return FALSE;
                }

                vmCPU.registers[arg1] = arg2;
                program->programCounter += SKIP_PER_INSTR;
                break;
            }
            case OP_VMCALL:{
                // this instruction exists just for debugging the NearerPC Virtual Machine
                // because of this it is in a privliege level that is higher then usual
                if(program->privliegeLevel == 4){
                    end_virtual_machine(program, "Invalid operation for privliege level");
                    return FALSE;
                }
                switch (arg1) {
                    case 0:{
                        if(check_if_valid_register(arg2) == FALSE){
                            end_virtual_machine(program, "Invalid register in OP_VMCALL vmcall 0\n");
                            return FALSE;
                        }
                        printf("x%lu has %lu\n", arg2, vmCPU.registers[arg2]);
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
                program->vmMem.memory[arg2] = vmCPU.registers[arg1];
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
                vmCPU.registers[arg2] = program->vmMem.memory[arg1];
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
                if(vmCPU.registers[arg2] == vmCPU.registers[arg3]){
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
                if(vmCPU.registers[arg2] != vmCPU.registers[arg3]){
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
                if(vmCPU.registers[arg2] > vmCPU.registers[arg3]){
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
                if(vmCPU.registers[arg2] < vmCPU.registers[arg3]){
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
                if(vmCPU.registers[arg2] == vmCPU.registers[arg3]){
                    vmCPU.registers[arg1] = 1;
                } else {
                    vmCPU.registers[arg1] = 0;
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
                if(vmCPU.registers[arg2] != vmCPU.registers[arg3]){
                    vmCPU.registers[arg1] = 1;
                } else {
                    vmCPU.registers[arg1] = 0;
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
                if(vmCPU.registers[arg2] >= vmCPU.registers[arg3]){
                    vmCPU.registers[arg1] = 1;
                } else {
                    vmCPU.registers[arg1] = 0;
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
                if(vmCPU.registers[arg2] <= vmCPU.registers[arg3]){
                    vmCPU.registers[arg1] = 1;
                } else {
                    vmCPU.registers[arg1] = 0;
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
