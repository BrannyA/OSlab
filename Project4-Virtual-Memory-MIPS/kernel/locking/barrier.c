#include "barrier.h"

void do_barrier_init(barrier_t *barrier, int goal)
{
    barrier->total_num = goal;
    barrier->current_num = 0;
    do_mutex_lock_init( &(barrier->block_queue) );
}

void do_barrier_wait(barrier_t *barrier)
{
    //lock
    barrier->current_num ++;
    if(barrier->current_num == barrier->total_num)
    {
        do_unblock_all( &(barrier->block_queue) );
        barrier->current_num = 0;
    }
    else
        do_block( &(barrier->block_queue) );
}
