#include "barrier.h"

void do_barrier_init(barrier_t *barrier, int goal) {
    barrier->max_task = goal;
    barrier->task_num = 0;
    queue_init(&(barrier->barrier_queue));
}

void do_barrier_wait(barrier_t *barrier) {
    barrier->task_num = barrier->task_num + 1;
    if (barrier->task_num < barrier->max_task)
        do_block(&(barrier->barrier_queue));
    else {
        barrier->task_num = 0;
        do_unblock_all(&(barrier->barrier_queue));
    }
}