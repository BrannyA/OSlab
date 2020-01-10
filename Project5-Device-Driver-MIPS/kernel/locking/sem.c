#include "sem.h"
#include "stdio.h"

void do_semaphore_init(semaphore_t *s, int val) {
    s->value = val;
    queue_init(&(s->sem_queue));
}

void do_semaphore_up(semaphore_t *s) {
    if (s->value < 0)
        do_unblock_one(&(s->sem_queue));
    s->value = s->value + 1;
}

void do_semaphore_down(semaphore_t *s) {
    s->value = s->value - 1;
    if (s->value < 0)
        do_block(&(s->sem_queue));
}