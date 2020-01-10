#include "asm.h"

LEAF(invoke_syscall)
    // TODO syscall entry
    //addiu	sp,	-16
    syscall
    jr		ra
    //addiu	sp,	16
END(invoke_syscall)
