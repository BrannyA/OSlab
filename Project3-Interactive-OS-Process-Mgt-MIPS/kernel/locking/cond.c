#include "cond.h"
#include "lock.h"

void do_condition_init(condition_t *condition)
{
    condition->block_num = 0;
    queue_init( &(condition->block_queue) );
}

void do_condition_wait(mutex_lock_t *lock, condition_t *condition)
{
    condition->block_num ++ ;
    do_mutex_lock_release(lock);
    do_block( &(condition->block_queue) );
    do_mutex_lock_acquire(lock);
}

void do_condition_signal(condition_t *condition)
{
    if(condition->block_num > 0)
    {
        do_unblock_one( &(condition->block_queue) );
        condition->block_num -- ;
    }
}

void do_condition_broadcast(condition_t *condition)
{
    if(condition->block_num > 0)
    {
        do_unblock_all( &(condition->block_queue) );
        condition->block_num = 0;
    }
}
