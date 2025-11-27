#include "nrcpu.h"










int main(){
    uint64_t code[] = {
        OP_MOVI, 0, 1, 0,
        OP_MOVI, 2, 10, 0,
        OP_ADD, 1, 1, 0,
        OP_VMCALL, 0, 1, 0,
        OP_JNEQ, 8, 2, 1,
        OP_HALT, 0, 0, 0
    };
    VmCode vmCode = {code, 0};
    VmMemory memory = {NULL, 190};
    VmProgram program;
    program.isCurrentlyRunning = FALSE;
    program.vmCode = vmCode;
    vm_execute_program(&program);
}

