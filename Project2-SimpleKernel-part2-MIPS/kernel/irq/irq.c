#include "irq.h"
#include "time.h"
#include "sched.h"
#include "string.h"

static void irq_timer()
{
    // TODO clock interrupt handler.
    // scheduler, time counter in here to do, emmmmmm maybe.
    // screen_reflush();
    init_timer();
//    printk("timer init\n");
    //save_kernel_context();
    scheduler();
    //restore_user_context();
    //restore_kernel_context();
}

void interrupt_helper(uint32_t status, uint32_t cause)
{
    // TODO interrupt handler.
    // Leve3 exception Handler.
    // read CP0 register to analyze the type of interrupt.
    uint32_t int_type;
    // int_type = (status & cause & 0xff00) >> 8; //ip7~0
//    printk(" cause = %x\n", cause);
    int_type = (cause & 0x0000ff00) >> 8;
//    printk("int_type = %x\n", int_type);
    if(int_type == 0x80)
      irq_timer();
    // else
    //    printk("in else, cause = %x, int_type = %x\n", cause, int_type);
}

void other_exception_handler()
{
    // TODO other exception handler
}
