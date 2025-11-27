#ifndef NRMEMORY_H_INCLUDED
#define NRMEMORY_H_INCLUDED


















#include "nearerpc.h"







typedef struct _VmMemory {
    uint64_t* memory;
    size_t memorySize;
} VmMemory;





boolean vm_initalize_memory(uint64_t amount);
VmMemory vm_return_system_memory();
void vm_free_vm_memory();
















#endif
