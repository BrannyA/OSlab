#include "irq.h"
#include "time.h"
#include "sched.h"
#include "string.h"
#include "mac.h"

static void irq_timer() {
    time_elapsed = time_elapsed + 150000;
    screen_reflush();
    // if(!queue_is_empty(&recv_block_queue))
    //     check_recv();

    do_scheduler();

}

void other_exception_handler() {
    printk("other exception");
}

void interrupt_helper(uint32_t status, uint32_t cause) {
    uint32_t int_type;
    int_type = (status & cause & 0xff00) >> 8;
    if (int_type == 0x80)
        irq_timer();
    else
        mac_irq_handle();
}

void check_recv(void)
{
    int i;
    for(i = 0; i < PNUM; i++)
    {
        if(!(reg_read_32(test_mac_recv_glb + 16*i) & 0x80000000) && recv_flag[i]==0)
		{
            recv_flag[i] = 1;
            if(i == PNUM - 1)
            {
                do_unblock_one(&recv_block_queue);
                // sys_move_cursor(1, 10);
                // printf("unblocked! ", i);
            }
        }
    }
}
