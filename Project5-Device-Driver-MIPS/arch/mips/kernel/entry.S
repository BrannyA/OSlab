#include "asm.h"

.equ    KERNEL, 0
.equ    USER,   156

.macro STI
	mfc0	k0, CP0_STATUS
	ori		k0, 0x1
	mtc0	k0, CP0_STATUS
.endm

.macro CLI
	mfc0	k0, CP0_STATUS
	li		k1, 0xfffffffe
	and		k0, k0, k1
	mtc0	k0, CP0_STATUS
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

LEAF(get_cp0_status)
    mfc0    v0, CP0_STATUS

    jr      ra
END(get_cp0_status)

LEAF(set_cp0_status)
    mtc0    a0, CP0_STATUS

    jr      ra
END(set_cp0_status)

LEAF(pidget)
    lw      k0, current_running
    addi    k0, k0, USER
    sw      a0, OFFSET_REG2(k0)
    jr      ra
END(pidget)

LEAF(gettime)

	mfc0	s0, CP0_COMPARE
	mfc0	s1, CP0_COUNT
	jr 		ra
END(gettime)

LEAF(init_exce)
	li		t1, 0xff03
	mtc0  t1, CP0_STATUS
	
	li		t1, 150000
	mtc0	t1, CP0_COMPARE
	
	jr		ra
END(init_exce)

LEAF(reset_time_counter)
	mtc0	zero, CP0_COUNT
	
	jr 		ra
	
END(reset_time_counter)

LEAF(enable_interrupt)
	mtc0	zero, CP0_COUNT	
	//li		t1, 0x8001
	//mtc0    t1, CP0_STATUS
	mtc0	ra,	CP0_EPC
	//jr		ra
	eret
END(enable_interrupt)

.set noat
.macro SAVE_CONTEXT offset
    // TODO save context
     lw      k0, current_running
    addi    k0, k0, \offset         #offset is KERNEL
    sw      $1, OFFSET_REG1(k0)
    sw      $2, OFFSET_REG2(k0)
    sw      $3, OFFSET_REG3(k0)
	sw      $4, OFFSET_REG4(k0)
	sw      $5, OFFSET_REG5(k0)
	sw      $6, OFFSET_REG6(k0)
	sw      $7, OFFSET_REG7(k0)
	sw      $8, OFFSET_REG8(k0)
	sw      $9, OFFSET_REG9(k0)
	sw      $10, OFFSET_REG10(k0)
	sw      $11, OFFSET_REG11(k0)
	sw      $12, OFFSET_REG12(k0)
	sw      $13, OFFSET_REG13(k0)
	sw      $14, OFFSET_REG14(k0)
	sw      $15, OFFSET_REG15(k0)
	sw      $16, OFFSET_REG16(k0)
	sw      $17, OFFSET_REG17(k0)
	sw      $18, OFFSET_REG18(k0)
	sw      $19, OFFSET_REG19(k0)
	sw      $20, OFFSET_REG20(k0)
	sw      $21, OFFSET_REG21(k0)
	sw      $22, OFFSET_REG22(k0)
	sw      $23, OFFSET_REG23(k0)
	sw      $24, OFFSET_REG24(k0)
	sw      $25, OFFSET_REG25(k0)
	sw      $28, OFFSET_REG28(k0)
	sw      $29, OFFSET_REG29(k0)
  sw      $30, OFFSET_REG30(k0)
	sw      $31, OFFSET_REG31(k0)
  mfc0	k1, CP0_BADVADDR
	nop
	sw		k1, OFFSET_BADVADDR(k0)
 	mfc0	  k1, CP0_EPC
	nop
	sw		  k1, OFFSET_EPC(k0)

.endm

.set noat
.macro RESTORE_CONTEXT offset
    // TODO restore context
    lw 		$26,current_running
    addi 	$26, $26, \offset
    lw 		$1,OFFSET_REG1($26)
    lw 		$2,OFFSET_REG2($26)
    lw 		$3,OFFSET_REG3($26)
    lw 		$4,OFFSET_REG4($26)
    lw 		$5,OFFSET_REG5($26)
    lw 		$6,OFFSET_REG6($26)
    lw 		$7,OFFSET_REG7($26)
    lw 		$8,OFFSET_REG8($26)
    lw 		$9,OFFSET_REG9($26)
    lw 		$10,OFFSET_REG10($26)
    lw 		$11,OFFSET_REG11($26)
    lw 		$12,OFFSET_REG12($26)
    lw 		$13,OFFSET_REG13($26)
    lw 		$14,OFFSET_REG14($26)
    lw 		$15,OFFSET_REG15($26)
    lw 		$16,OFFSET_REG16($26)
    lw 		$17,OFFSET_REG17($26)
    lw 		$18,OFFSET_REG18($26)
    lw 		$19,OFFSET_REG19($26)
    lw 		$20,OFFSET_REG20($26)
    lw 		$21,OFFSET_REG21($26)
    lw 		$22,OFFSET_REG22($26)
    lw 		$23,OFFSET_REG23($26)
    lw 		$24,OFFSET_REG24($26)
    lw 		$25,OFFSET_REG25($26)
    lw 		$28,OFFSET_REG28($26)
    lw 		$29,OFFSET_REG29($26)
    lw 		$30,OFFSET_REG30($26)
    lw 		$31,OFFSET_REG31($26)
    lw		$27,OFFSET_EPC($26)
    mtc0	$27,CP0_EPC
	  lw		k1, OFFSET_BADVADDR(k0)
	  mtc0	k1, CP0_BADVADDR

.endm

NESTED(do_scheduler, 0, ra)
	//addiu	sp,	sp,	8
    SAVE_CONTEXT(KERNEL)
    jal     scheduler
    RESTORE_CONTEXT(KERNEL)
    jr      ra
END(do_scheduler)

.global exception_handler_begin
.global exception_handler_end


.global TLBexception_handler_begin
.global TLBexception_handler_end
NESTED(exception_handler_entry, 0, sp)   
exception_handler_begin:
    // TODO close interrupt
	CLI
	SAVE_CONTEXT(USER)
    // jmp exception_handler[i] which decided by CP0_CAUSE
    // Leve2 exception Handler.
	mfc0	k0, CP0_CAUSE
	andi	k0, k0, CAUSE_EXCCODE
	la		k1, exception_handler
	add		k0, k0, k1
	lw		k0, 0(k0)
	jr		k0
exception_handler_end:
END(exception_handler_entry)


NESTED(TLBexception_handler_entry, 0, sp)
TLBexception_handler_begin:

  la	k0, current_running
  lw	k0, 0(k0)
  lw    $29, OFFSET_REG29(k0)
  li    k0, 0x80000180
  jr    k0
TLBexception_handler_end:
END(TLBexception_handler_entry)


NESTED(handle_tlb, 0, sp)
  // TLB exception handler
  mfc0  a0, CP0_STATUS
  mfc0  a1, CP0_CAUSE
  addiu sp, sp, -8
  jal   tlb_helper
  addiu sp, sp, 8

  RESTORE_CONTEXT(USER)

  STI
  eret
END(handle_tlb)


NESTED(handle_int, 0, sp)
    // interrupt handler
    mfc0  a0, CP0_STATUS
    mfc0  a1, CP0_CAUSE
    addiu	sp,	-8
    jal 	interrupt_helper
    addiu 	sp, 8
	  li		t1, 150000
	  mtc0	t1, CP0_COMPARE
    RESTORE_CONTEXT(USER)
    mtc0	zero, CP0_COUNT
    mfc0 	k0, CP0_STATUS
    ori 	k0, k0, 0x1
    mtc0 	k0, CP0_STATUS
    STI
    eret
    // Leve3 exception Handler.
END(handle_int)
    

NESTED(handle_syscall, 0, sp)
    // system call handler
    addiu	sp,	-16
    jal		system_call_helper
    addiu	sp,	16
    RESTORE_CONTEXT(USER)
	  mfc0	k0, CP0_EPC
	  addi	k0, k0, 4
	  mtc0	k0, CP0_EPC
    mfc0 	k0, CP0_STATUS
    ori 	k0, k0, 0x1
    mtc0 	k0, CP0_STATUS
    STI
    eret
END(handle_syscall)


NESTED(handle_other, 0, sp)
   jal interrupt_helper
END(handle_other)





LEAF(first_return)
    addiu 	sp, 8
	li		t1, 150000
	mtc0	t1, CP0_COMPARE
    RESTORE_CONTEXT(USER)
    mtc0	zero, CP0_COUNT
    mfc0 	k0, CP0_STATUS
    ori 	k0, k0, 0x1
    mtc0 	k0, CP0_STATUS

    eret
END(first_return)

LEAF(search_TLB)
  mtc0 a0, CP0_ENTRYHI
  tlbp
  mfc0 v0, CP0_INDEX
  jr   ra
END(search_TLB)

LEAF(set_EntryHi)
  mtc0 a0, CP0_ENTRYHI
  jr   ra
  nop
END(set_EntryHi)

LEAF(set_EntryLo0)
  mtc0 a0, CP0_ENTRYLO0
  jr   ra
  nop
END(set_EntryLo0)

LEAF(set_EntryLo1)
  mtc0 a0, CP0_ENTRYLO1
  jr   ra
  nop
END(set_EntryLo1)

LEAF(set_cp0_Index)
  mtc0 a0, CP0_INDEX
  jr   ra
  nop
END(set_cp0_Index)

LEAF(set_TLB_refill)
  tlbwi

  jr ra
END(set_TLB_refill)


LEAF(set_PageMask)
  mtc0	zero, CP0_PAGEMASK
  tlbwi

  jr ra
END(set_PageMask)


LEAF(GET_CP0_STATUS)
	mfc0	v0, CP0_STATUS

	jr		ra
END(GET_CP0_STATUS)

LEAF(SET_CP0_STATUS)
	mtc0	a0, CP0_STATUS

	jr		ra
END(SET_CP0_STATUS)