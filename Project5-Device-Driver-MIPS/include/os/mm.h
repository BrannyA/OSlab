#ifndef INCLUDE_MM_H_
#define INCLUDE_MM_H_

#include "type.h"
#include "queue.h"

#define TLB_ENTRY_NUMBER 64
#define PTE_NUMBER       1024
#define PF_NUMBER        1024
#define PAGE_SIZE        0x1000

typedef struct {
    // design here
    uint32_t VPN;
    uint32_t PFN;
    pid_t pid;
    int8_t valid;

} PTE_t;
extern uint8_t pframe[PF_NUMBER];
extern PTE_t PTE[PTE_NUMBER];
extern PTE_t PTE_1[PTE_NUMBER];
extern PTE_t PTE_2[PTE_NUMBER];
extern PTE_t PTE_3[PTE_NUMBER];
extern int8_t PF[PF_NUMBER];
//extern queue_t PF;

void tlb_helper();

#endif
