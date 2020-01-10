#include "mm.h"
#include "sched.h"

PTE_t PTE[PTE_NUMBER];
int8_t PF[PF_NUMBER];

int handle_page_fault(uint32_t VPN)
{
    int i, j;
    for (i = 0; i < PTE_NUMBER; i++)
    {
        if (PTE[i].valid == 0)
            break;
    }
    if (i == PTE_NUMBER)
    {
        printk("No PTE to allocate\n");
        while (1);
    }
    for (j = 0; j < PF_NUMBER; j++)
    {
        if (PF[j] == 0)
            break;
    }
    if (j == PF_NUMBER)
    {
        printk("No PF to allocate\n");
        while (1);
    }
    PTE[i].VPN = VPN;
    PTE[i].PFN = j;
    PTE[i].pid = current_running->pid;
    PTE[i].valid = 1;

    PTE[i + 1].VPN = VPN + 1;
    PTE[i + 1].PFN = j + 1;
    PTE[i + 1].pid = current_running->pid;
    PTE[i + 1].valid = 1;

    PF[j] = 1;
    PF[j + 1] = 1;
    return i;
}

void tlb_helper()
{
    static id = 0;
    int index;
    int i, P;
    uint32_t VPN = (current_running->user_context.cp0_badvaddr & 0xffffe000) >> 12;
    uint32_t PID = current_running->pid;
    for (i = 0; i < PTE_NUMBER; i++)
    {
        if (PTE[i].VPN == VPN && PTE[i].pid == PID && PTE[i].valid == 1)
            break;
    }
    if (i == PTE_NUMBER) // page fault
        i = handle_page_fault(VPN);

    index = search_TLB(PTE[i].VPN << 12 | PID);
    P = index >> 31;

    if (P)
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
