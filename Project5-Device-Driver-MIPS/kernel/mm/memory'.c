#include "mm.h"

#include "sched.h"
#include "queue.h"

PTE_t PTE[PTE_NUMBER];
queue_t PF;

int handle_page_fault(uint32_t VPN) {
    uint8_t *m;
    int i, j;
    i = VPN;
    m = (uint8_t *) queue_dequeue(&PF);
    j = *(m);
    PTE[i].PFN = j;
    //printk("%d",j);
    PTE[i].pid = current_running->pid;
    PTE[i].valid = 1;
    m = (uint8_t *) queue_dequeue(&PF);
    j = *(m);
    PTE[i + 1].PFN = j;
    PTE[i + 1].pid = current_running->pid;
    PTE[i + 1].valid = 1;

    return i;
}


void tlb_helper() {
    static id = 0;
    int index;
    int i, P;
    uint32_t VPN = (current_running->user_context.cp0_badvaddr & 0xffffe000) >> 12;
    uint8_t PID = current_running->pid;
    if (PTE[VPN].valid == 0)
        i = handle_page_fault(VPN);


    index = search_TLB(VPN << 12 | PID);
    P = index >> 31;

    if (P)  // TLB_Refill
    {
        id++;
        id = id % 31;
        set_cp0_Index(id);
    }
    set_EntryLo0(PTE[i].PFN << 6 | 0x16);
    set_EntryLo1(PTE[i + 1].PFN << 6 | 0x16);
    set_PageMask();
    return;
}
