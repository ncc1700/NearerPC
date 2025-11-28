# NearerPC

A small "64bit" fantasy "computer" (WIP) (very bad idea)

# Architecture

!! (Alot of this hasn't been implemented yet, changes might happen and itll be very blunt and not explanatory) !!

Now lets talk about privlieges, we kind of copy the ARM64 model of 4 privliege levels (like their 4 exception levels)


- PL0: this is where the user will be, itll be very restrictive and itll only work when memory state is set to virtual memory, other then that you cant switch
- PL1: this is where the kernel and boot ROM will be, they have all the privlieges they need
- PL2: this is where the hypervisor will be (idk why im doing this)
- PL3: unknown, will be used for *something* not sure yet
- PL4: this is purely for the NearerPC emulator for debugging stuff, operations like OP_VMCALL will be avaliable here


# How to build

run xmake build to get a binary. its wip so this might work idk
