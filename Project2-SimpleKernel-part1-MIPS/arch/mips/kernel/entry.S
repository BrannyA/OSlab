#include "asm.h"

.equ    KERNEL, 0
.equ    USER,   156

.macro STI
.endm

.macro CLI
.endm

LEAF(asm_start)
    mtc0    zero, CP0_STATUS
    mtc0    zero, CP0_WATCHLO
    mtc0    zero, CP0_WATCHHI

    mfc0    t0, CP0_CONFIG
    and     t0, ~0x7
    ori     t0, 0x2
    mtc0    t0, CP0_CONFIG

    jr  ra
END(asm_start)

.macro SAVE_CONTEXT offset
.set noat
    lw  k0, current_running
    addi k0, k0, \offset

    #sw	$0 , OFFSET_REG0(k0)
    sw	$1 , OFFSET_REG1(k0)
    sw	$2 , OFFSET_REG2(k0)
    sw	$3 , OFFSET_REG3(k0)
    sw	$4 , OFFSET_REG4(k0)
    sw	$5 , OFFSET_REG5(k0)
    sw	$6 , OFFSET_REG6(k0)
    sw	$7 , OFFSET_REG7(k0)
    sw	$8 , OFFSET_REG8(k0)
    sw	$9 , OFFSET_REG9(k0)
    sw	$10, OFFSET_REG10(k0)
    sw	$11, OFFSET_REG11(k0)
    sw	$12, OFFSET_REG12(k0)
    sw	$13, OFFSET_REG13(k0)
    sw	$14, OFFSET_REG14(k0)
    sw	$15, OFFSET_REG15(k0)
    sw	$16, OFFSET_REG16(k0)
    sw	$17, OFFSET_REG17(k0)
    sw	$18, OFFSET_REG18(k0)
    sw	$19, OFFSET_REG19(k0)
    sw	$20, OFFSET_REG20(k0)
    sw	$21, OFFSET_REG21(k0)
    sw	$22, OFFSET_REG22(k0)
    sw	$23, OFFSET_REG23(k0)
    sw	$24, OFFSET_REG24(k0)
    sw	$25, OFFSET_REG25(k0)
#    sw	$26, OFFSET_REG26(k0)
#    sw	$27, OFFSET_REG27(k0)
    sw	$28, OFFSET_REG28(k0)
    sw	$29, OFFSET_REG29(k0)
    sw	$30, OFFSET_REG30(k0)
    sw	$31, OFFSET_REG31(k0)

    mfc0 k1, CP0_STATUS
    nop
    sw   k1, OFFSET_STATUS(k0)

    mfc0 k1, CP0_TAGHI
    nop
    sw   k1, OFFSET_HI(k0)

    mfc0 k1, CP0_TAGLO
    nop
    sw   k1, OFFSET_LO(k0)

    mfc0 k1, CP0_BADVADDR
    nop
    sw   k1, OFFSET_BADVADDR(k0)

    mfc0 k1, CP0_CAUSE
    nop
    sw   k1, OFFSET_CAUSE(k0)

    mfc0 k1, CP0_EPC
    nop
    sw   k1, OFFSET_EPC(k0)
.endm

.macro RESTORE_CONTEXT offset
.set noat
    lw	k0, current_running
#    loopp:
#        j loopp
    addi k0, k0, \offset

#    lw	$0 , OFFSET_REG0(k0)
    lw	$1 , OFFSET_REG1(k0)
    lw	$2 , OFFSET_REG2(k0)
    lw	$3 , OFFSET_REG3(k0)
    lw	$4 , OFFSET_REG4(k0)
    lw	$5 , OFFSET_REG5(k0)
    lw	$6 , OFFSET_REG6(k0)
    lw	$7 , OFFSET_REG7(k0)
    lw	$8 , OFFSET_REG8(k0)
    lw	$9 , OFFSET_REG9(k0)
    lw	$10, OFFSET_REG10(k0)
    lw	$11, OFFSET_REG11(k0)
    lw	$12, OFFSET_REG12(k0)
    lw	$13, OFFSET_REG13(k0)
    lw	$14, OFFSET_REG14(k0)
    lw	$15, OFFSET_REG15(k0)
    lw	$16, OFFSET_REG16(k0)
    lw	$17, OFFSET_REG17(k0)
    lw	$18, OFFSET_REG18(k0)
    lw	$19, OFFSET_REG19(k0)
    lw	$20, OFFSET_REG20(k0)
    lw	$21, OFFSET_REG21(k0)
    lw	$22, OFFSET_REG22(k0)
    lw	$23, OFFSET_REG23(k0)
    lw	$24, OFFSET_REG24(k0)
    lw	$25, OFFSET_REG25(k0)
#    lw	$26, OFFSET_REG26(k0)
#    lw	$27, OFFSET_REG27(k0)
    lw	$28, OFFSET_REG28(k0)
    lw	$29, OFFSET_REG29(k0)
    lw	$30, OFFSET_REG30(k0)
    lw	$31, OFFSET_REG31(k0)

    lw   k1, OFFSET_STATUS(k0)
    mtc0 k1, CP0_STATUS
    nop
    lw   k1, OFFSET_HI(k0)
    mtc0 k1, CP0_TAGHI
    nop
    lw   k1, OFFSET_LO(k0)
    mtc0 k1, CP0_TAGLO
    nop
    lw   k1, OFFSET_BADVADDR(k0)
    mtc0 k1, CP0_BADVADDR
    nop
    lw   k1, OFFSET_CAUSE(k0)
    mtc0 k1, CP0_CAUSE
    nop
    lw   k1, OFFSET_EPC(k0)
    mtc0 k1, CP0_EPC
    nop
.endm

NESTED(do_scheduler, 0, ra)
    SAVE_CONTEXT(KERNEL)
    jal     scheduler
    RESTORE_CONTEXT(KERNEL)
#    loop:
#        j loop
    jr      ra
END(do_scheduler)

.global exception_handler_begin
.global exception_handler_end

NESTED(exception_handler_entry, 0, sp)
exception_handler_begin:
    // TODO close interrupt
    // jmp exception_handler[i] which decided by CP0_CAUSE
    // Leve2 exception Handler.
exception_handler_end:
END(exception_handler_entry)

NESTED(handle_int, 0, sp)
    // interrupt handler
    // Leve3 exception Handler.
END(handle_int)

NESTED(handle_syscall, 0, sp)
    // system call handler
END(handle_syscall)

NESTED(handle_other, 0, sp)
    // other exception handler
END(handle_other)
