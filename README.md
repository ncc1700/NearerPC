# NearerPC

A small "64bit" fantasy "computer" (WIP) (very bad idea)

# Architecture

!! (Alot of this hasn't been implemented yet, changes might happen and itll be very blunt and not explanatory) !!

The design of this "Computer" is weird and not natural and idk if its even possible but basically processes/tasks are hardware level 

they at boot get memory, normally if set to no VMM itll be the global memory and they can access it with all the other processes/tasks.

the software can dispatch and create these processes/tasks and theyll get assigned their own PC, its the software responsibility to schedule,

handle task switches (registers), and kill those processes as registers are global. If VMM is enabled they will get their own memory taken from Global Memory



Now lets talk about privlieges, we kind of copy the ARM64 model of 4 privliege levels (like their 4 exception levels)


- PL0: this is where the user will be, itll be very restrictive and itll only work when memory state is set to virtual memory, as they can access their own specific portion, other then that you cant switch
- PL1: this is where the kernel and boot ROM will be, they have all the privlieges they need, but they cant do hypervisor level stuff and can't do vmcalls
- PL2: this is where the hypervisor will be (idk why im doing this)
- PL3: unknown, will be used for *something* not sure yet
- PL4: this is purely for the NearerPC emulator for debugging stuff, operations like OP_VMCALL will be avaliable here


# How to build

run xmake build to get a binary. its wip so this might work idk
