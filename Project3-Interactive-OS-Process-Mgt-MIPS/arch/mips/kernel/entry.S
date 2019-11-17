#include "asm.h"

.equ    KERNEL, 0
.equ    USER,   156
.macro STI
    mfc0    k0, CP0_STATUS  
    li      k1, 0x1
    or      k0, k1, k0
    mtc0    k0, CP0_STATUS  //Ä©Î»ÖÃ1 
.endm

.macro CLI
    mfc0    k0, CP0_STATUS
    li      k1, 0xfffffffe
    and     k0, k1, k0
    mtc0    k0, CP0_STATUS  //Ä©Î»ÖÃ0 
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

LEAF(GET_CP0_STATUS)
	mfc0	v0, CP0_STATUS
	jr		ra
END(GET_CP0_STATUS)

LEAF(SET_CP0_STATUS)
	mtc0	a0, CP0_STATUS
	jr		ra
END(SET_CP0_STATUS)

LEAF(GET_CP0_COUNT)
	mfc0	v0, CP0_COUNT
	jr		ra
END(GET_CP0_COUNT)

LEAF(SET_CP0_COUNT)
	mtc0	a0, CP0_COUNT
	jr		ra
END(SET_CP0_COUNT)

LEAF(GET_CP0_COMPARE)
	mfc0	v0, CP0_COMPARE
	jr		ra
END(GET_CP0_COMPARE)

LEAF(SET_CP0_COMPARE)
	mtc0	a0, CP0_COMPARE
	jr		ra
END(SET_CP0_COMPARE)

.macro SAVE_CONTEXT offset
.set noat
    lw k0,current_running 
    addi k0,k0,\offset
    
    sw $0, 0(k0)
    sw $1, 4(k0)
    sw $2, 8(k0)
    sw $3, 12(k0)
    sw $4, 16(k0)
    sw $5, 20(k0)
    sw $6, 24(k0)
    sw $7, 28(k0)
	sw $8, 32(k0)   
    sw $9, 36(k0)
    sw $10, 40(k0)
    sw $11, 44(k0)
    sw $12, 48(k0)
    sw $13, 52(k0)
    sw $14, 56(k0)
    sw $15, 60(k0)
    sw $16, 64(k0)
    sw $17, 68(k0)
    sw $18, 72(k0)
    sw $19, 76(k0)
    sw $20, 80(k0)
    sw $21, 84(k0)
    sw $22, 88(k0)
    sw $23, 92(k0)
    sw $24, 96(k0)
    sw $25, 100(k0)
    
    sw $28, 112(k0)
    sw $29, 116(k0)
    sw $30, 120(k0)
    sw $31, 124(k0)
    
    mfc0    k1,CP0_STATUS
    nop
    sw      k1,128(k0)
    
    mfc0    k1, CP0_TAGHI
    nop
    sw      k1, 132(k0)

    mfc0    k1, CP0_TAGLO
    nop
    sw      k1, 136(k0)

    mfc0    k1, CP0_BADVADDR
    nop
    sw      k1, 140(k0)

    mfc0    k1, CP0_CAUSE
    nop
    sw      k1, 144(k0)

    mfc0    k1, CP0_EPC
    nop
    sw      k1, 148(k0) 
.endm

.macro RESTORE_CONTEXT offset
.set noat
   lw  k0,current_running 
   addi k0,k0, \offset
    lw $0, 0(k0)
    lw $1, 4(k0)
    lw $2, 8(k0)
    lw $3, 12(k0)
    lw $4, 16(k0)
    lw $5, 20(k0)
    lw $6, 24(k0)
    lw $7, 28(k0)
    lw $8, 32(k0)
    lw $9, 36(k0)
    lw $10, 40(k0)
    lw $11, 44(k0)
    lw $12, 48(k0)
    lw $13, 52(k0)
    lw $14, 56(k0)
    lw $15, 60(k0)
    lw $16, 64(k0)
    lw $17, 68(k0)
    lw $18, 72(k0)
    lw $19, 76(k0)
    lw $20, 80(k0)
    lw $21, 84(k0)
    lw $22, 88(k0)
    lw $23, 92(k0)
    lw $24, 96(k0)
    lw $25, 100(k0)

    lw $28, 112(k0)
    lw $29, 116(k0)
    lw $30, 120(k0)
    lw $31, 124(k0)
    lw      k1, 128(k0)
    mtc0    k1, CP0_STATUS
    nop 

    lw      k1, 132(k0)
    mtc0    k1, CP0_TAGHI
    nop

    lw      k1, 136(k0)
    mtc0    k1, CP0_TAGLO
    nop

    lw      k1, 140(k0)
    mtc0    k1, CP0_BADVADDR
    nop

    lw      k1, 144(k0)
    mtc0    k1, CP0_CAUSE
    nop

    lw      k1, 148(k0)
    mtc0    k1, CP0_EPC
    nop
.endm

NESTED(do_scheduler, 0, ra)
    SAVE_CONTEXT(KERNEL)
    jal     scheduler
    RESTORE_CONTEXT(KERNEL)
    jr      ra
END(do_scheduler)

.global exception_handler_begin
.global exception_handler_end
.global int_finish

NESTED(exception_handler_entry, 0, sp) 
exception_handler_begin:
	mfc0 k0,CP0_EPC
    CLI
    SAVE_CONTEXT(USER)
    
    mfc0  k0, CP0_CAUSE
    nop
    andi  k0, k0, CAUSE_EXCCODE   

    la    k1, exception_handler
    add   k0, k0, k1
    lw    k0, 0(k0)
    jr    k0     /* exception_handler[exc_code](); */
exception_handler_end:
END(exception_handler_entry)

NESTED(handle_int, 0, sp)

	mfc0	a0, CP0_STATUS
	mfc0	a1, CP0_CAUSE
	addiu	sp, sp, -8
	jal		interrupt_helper
	addiu	sp, sp, 8
	
int_finish:
	mtc0	zero, CP0_COUNT
	li		k0, 150000
	mtc0	k0, CP0_COMPARE
	
	j		exception_handler_finish
END(handle_int)

NESTED(handle_syscall, 0, sp)
	addiu	sp, sp, -16
	jal		system_call_helper
	addiu	sp, sp, 16
	
	j		exception_handler_finish 
END(handle_syscall)

NESTED(handle_other, 0, sp)
    // other exception handler
	j		exception_handler_finish
END(handle_other)

LEAF(exception_handler_finish)
	RESTORE_CONTEXT(USER)
	STI
	eret
END(exception_handler_finish)
