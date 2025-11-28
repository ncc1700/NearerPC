#include "nrmemory.h"
#include "nearerpc.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static VmMemory machineMem = {0};

/**
    The goals for this later on is implement some sort of Virtual memory
    like maybe paging for anyone in a specific privliage


 */

boolean vm_initalize_memory(uint64_t amount){
    machineMem.memory = malloc(amount * sizeof(uint64_t));
    if(machineMem.memory == NULL) return FALSE;
    machineMem.memorySize = amount;
    memset(machineMem.memory, 0, machineMem.memorySize);
    return TRUE;
}

VmMemory vm_return_system_memory(){
    return machineMem;
}

void vm_free_vm_memory(){
    free(machineMem.memory);
}
