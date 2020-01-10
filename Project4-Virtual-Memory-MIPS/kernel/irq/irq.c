#include "mm.h"
#include "irq.h"
#include "time.h"
#include "sched.h"
#include "string.h"

static void irq_timer() {
    // TODO clock interrupt handler.
    // scheduler, time counter in here to do, emmmmmm maybe.
    time_elapsed += 100000;
    screen_reflush();
    init_timer();

    do_scheduler();
};

void interrupt_helper(uint32_t status, uint32_t cause) {
    // TODO interrupt handler.
    // Leve3 exception Handler.
    // read CP0 register to analyze the type of interrupt.
    uint32_t int_type;
    int_type = (status & cause & 0xff00) >> 8;

    if (int_type == 0x80)
        irq_timer();
    else {
        vt100_move_cursor(1, 5);
        printk("Unknow Interrupt: STATUS = %x CAUSE = %x pid = %d, pc = %x", status, cause, process_id,
               current_running->user_context.regs[31]);
        while (1);
    }
}

void other_exception_handler(uint32_t status, uint32_t cause) {
    int badvaddr = get_cp0_badvaddr();
    int epc = get_cp0_epc();
    printk("Unknow Exception: STATUS = 0x%x CAUSE = 0x%x pid = %d, BadAddr = 0x%x, EPC = 0x%x\n", status, cause,
           process_id, badvaddr, epc);

    int i;
    for (i = 0; i < 10; i++)
        printk("PTE[%d]: VPN=%x, PFN=%x, PID=%x, valid =%x\n", i, PTE[i].VPN, PTE[i].PFN, PTE[i].pid, PTE[i].valid);
    while (1);
}

void print_count() {
    printk("%d\n", get_cp0_count());

    return;
}
