#ifndef INCLUDE_BARRIER_H_
#define INCLUDE_BARRIER_H_

#include "queue.h"

typedef struct barrier {
    int max_task;
    int task_num;
    queue_t barrier_queue;
} barrier_t;

void do_barrier_init(barrier_t *, int);

void do_barrier_wait(barrier_t *);

#endif