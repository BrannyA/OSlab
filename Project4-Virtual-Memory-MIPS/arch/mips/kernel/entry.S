#include "asm.h"

.equ    KERNEL, 0
.equ    USER,   156

.macro STI  // open  interrupt
  mfc0  k0, CP0_STATUS
  ori   k0, 0x01
  mtc0  k0, CP0_STATUS
.endm

.macro CLI  // close interrupt
  mfc0  k0, CP0_STATUS
  xori  k0, 0x01
  mtc0  k0, CP0_STATUS
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
    la	k0, current_running
    lw	k0, 0(k0)
    addi k0, k0, \offset

    sw	$0 , OFFSET_REG0(k0)
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

    sw	$28, OFFSET_REG28(k0)
    sw	$29, OFFSET_REG29(k0)
    sw	$30, OFFSET_REG30(k0)
    sw	$31, OFFSET_REG31(k0)

    mfc0 k1, CP0_STATUS
    sw   k1, OFFSET_STATUS(k0)

    mfhi k1
    sw   k1, OFFSET_HI(k0)
    
    mflo k1
    sw   k1, OFFSET_LO(k0)
    
    mfc0 k1, CP0_BADVADDR
    sw   k1, OFFSET_BADVADDR(k0)
    
    mfc0 k1, CP0_CAUSE
    sw   k1, OFFSET_CAUSE(k0)
    
    mfc0 k1, CP0_EPC
    sw   k1, OFFSET_EPC(k0)
.endm

.macro RESTORE_CONTEXT offset
    .set noat

    la	k0, current_running
    lw	k0, 0(k0)
    addi k0, k0, \offset

    lw	$0 , OFFSET_REG0(k0)
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

    lw	$28, OFFSET_REG28(k0)
    lw	$29, OFFSET_REG29(k0)
    lw	$30, OFFSET_REG30(k0)
    lw	$31, OFFSET_REG31(k0)

    lw   k1, OFFSET_STATUS(k0)
    mtc0 k1, CP0_STATUS

    lw   k1, OFFSET_HI(k0)
    mthi k1

    lw   k1, OFFSET_LO(k0)
    mtlo k1

    lw   k1, OFFSET_BADVADDR(k0)
    mtc0 k1, CP0_BADVADDR

    lw   k1, OFFSET_CAUSE(k0)
    mtc0 k1, CP0_CAUSE

    lw   k1, OFFSET_EPC(k0)
    mtc0 k1, CP0_EPC
.endm

NESTED(do_scheduler, 0, ra)

    SAVE_CONTEXT(KERNEL)
    jal     scheduler
    RESTORE_CONTEXT(KERNEL)
    jr      ra
END(do_scheduler)

.global exception_handler_begin
.global exception_handler_end

NESTED(exception_handler_entry, 0, sp)   
exception_handler_begin:
  // Leve2 exception Handler.
  // close interrupt
  CLI

  // save contex
  SAVE_CONTEXT(USER)
  
//    la	k0, current_running
//    lw	k0, 0(k0)
//    lw  $29, OFFSET_REG29(k0)

  // jmp exception_handler[i] which decided by CP0_CAUSE
  mfc0  k0, CP0_CAUSE
  andi  k0, k0, CAUSE_EXCCODE
  la    k1, exception_handler
  add   k0, k0, k1
  lw    k0, 0(k0)
  jr    k0
exception_handler_end:
END(exception_handler_entry)

NESTED(handle_int, 0, sp)
  // interrupt handler
  mfc0  a0, CP0_STATUS
  mfc0  a1, CP0_CAUSE
  addiu sp, sp, -8
  jal   interrupt_helper
  addiu sp, sp, 8

  RESTORE_CONTEXT(USER)

  STI

  li   k0,   150000
  mtc0 k0, CP0_COMPARE

  eret
END(handle_int)

NESTED(handle_syscall, 0, sp)
  // system call handler
  addiu sp, sp, -16
  jal  system_call_helper
  addiu sp, sp, 16

  RESTORE_CONTEXT(USER)
  lw   k1, OFFSET_EPC(k0)
  addi k1, 0x4
  mtc0 k1, CP0_EPC

  STI
  eret
END(handle_syscall)

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

NESTED(handle_other, 0, sp)
  mfc0  a0, CP0_STATUS
  mfc0  a1, CP0_CAUSE
  addiu sp, sp, -8
  jal   other_exception_handler
  addiu sp, sp, 8
END(handle_other)

LEAF(get_IP)
  mfc0  v0, CP0_CAUSE
  andi  v0, 0xff00
  srl   v0, 8
END(get_IP)

LEAF(init_timer)
  mtc0 $0, CP0_COUNT
  li   k0, 150000
  mtc0 k0, CP0_COMPARE
  jr   ra
END(init_timer)

LEAF(dis_interrupt)
  mtc0 zero, CP0_STATUS
END(dis_interrupt)

LEAF(get_cp0_status)
  mfc0 v0, CP0_STATUS
  jr   ra
END(get_cp0_status)

LEAF(init_cp0_status)
  mtc0  a0, CP0_STATUS
  jr    ra
END(init_cp0_status)

LEAF(get_cp0_compare)
  mfc0 v0, CP0_COMPARE
  jr   ra
END(get_cp0_compare)

LEAF(get_cp0_epc)
  mfc0 v0, CP0_EPC
  jr   ra
END(get_cp0_epc)

LEAF(get_cp0_count)
  mfc0 v0, CP0_COUNT
  jr   ra
END(get_cp0_count)  

LEAF(get_cp0_badvaddr)
  mfc0 v0, CP0_BADVADDR
  jr   ra
END(get_cp0_badvaddr)

LEAF(set_cp0_status)
  mtc0 a0, CP0_STATUS
  jr   ra
END(set_cp0_status)

.global TLBexception_handler_begin
.global TLBexception_handler_end

NESTED(TLBexception_handler_entry, 0, sp)
TLBexception_handler_begin:

  la	k0, current_running
  lw	k0, 0(k0)
  lw    $29, OFFSET_REG29(k0)

  li    k0, 0x80000180
  jr    k0
TLBexception_handler_end:
END(TLBexception_handler_entry)

LEAF(search_TLB)
  mtc0 a0, CP0_ENTRYHI
  tlbp
  mfc0 v0, CP0_INDEX
  jr   ra
END(search_TLB)

LEAF(set_EntryHi)
  mtc0 a0, CP0_ENTRYHI
  jr   ra
END(set_EntryHi)

LEAF(set_EntryLo0)
  mtc0 a0, CP0_ENTRYLO0
  jr   ra
END(set_EntryLo0)

LEAF(set_EntryLo1)
  mtc0 a0, CP0_ENTRYLO1
  jr   ra
END(set_EntryLo1)

LEAF(set_cp0_Index)
  mtc0 a0, CP0_INDEX
  jr   ra
END(set_cp0_Index)

LEAF(set_PageMask)
  mtc0	zero, CP0_PAGEMASK
  tlbwi

  jr ra
END(set_PageMask)
