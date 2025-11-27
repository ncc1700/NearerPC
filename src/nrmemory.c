#include "nrmemory.h"
#include "nearerpc.h"
#include <stdint.h>
#include <stdlib.h>





static VmMemory machineMem = {0};


boolean vm_initalize_memory(uint64_t amount){
    machineMem.memory = malloc(amount * sizeof(uint64_t));
    if(machineMem.memory == NULL) return FALSE;
    machineMem.memorySize = amount;
    return TRUE;
}

VmMemory vm_return_system_memory(){
    return machineMem;
}

void vm_free_vm_memory(){
    free(machineMem.memory);
}
