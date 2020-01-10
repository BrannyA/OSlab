#ifndef INCLUDE_MM_H_
#define INCLUDE_MM_H_

#include "type.h"

#define TLB_ENTRY_NUMBER 32
#define PTE_NUMBER       0x80000//2^19
#define PF_NUMBER        1024
#define PAGE_SIZE        0x1000

typedef struct
{
    // design here
    uint32_t VPN;
    uint32_t PFN;

    pid_t pid;
    int8_t valid;

} PTE_t;

extern PTE_t PTE[PTE_NUMBER];
extern PTE_t PTE_1[PTE_NUMBER];
extern PTE_t PTE_2[PTE_NUMBER];
extern PTE_t PTE_3[PTE_NUMBER];
extern int8_t PF[PF_NUMBER];

void do_TLB_Refill();

void do_page_fault();

void tlb_helper();

#endif
