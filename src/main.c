#include "nearerpc.h"
#include "nrcpu.h"
#include "nrmemory.h"
#include <stdio.h>









int main(){
    // create the machine with 256 bytes of memory
    boolean result = vm_initalize_memory(256);
    if(result == FALSE){
        printf("Couldn't create memory for virtual machine\n");
        return -1;
    }

    uint64_t code[] = {
        OP_MOVI, 0, 10, 0,
        OP_STORE, 0, 10, 0,
        OP_LOAD, 10, 1, 0,
        OP_VMCALL, 0, 1, 0,
        OP_HALT, 0, 0, 0
    };
    VmCode vmCode = {code, 0};
    VmProgram program = {FALSE, vm_return_system_memory(), vmCode, 4, 0};
    program.privliegeLevel = 4;

    vm_execute_program(&program);
    vm_free_vm_memory();
}

