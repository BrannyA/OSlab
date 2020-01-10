#include "sched.h"
#include "stdio.h"
#include "syscall.h"
#include "time.h"
#include "screen.h"
#include "test4.h"

#define RW_TIMES 3

int rand() {
    int current_time = get_timer();
    return current_time % 100000;
}


void rw_task1(unsigned long a[6]) {
    int mem1, mem2 = 0;
    int curs = 0;
    int memory[RW_TIMES];
    int i = 0;
    for (i = 0; i < RW_TIMES; i++) {
        mem1 = a[i];
        sys_move_cursor(1, curs + i);
        memory[i] = mem2 = rand();
        *(int *) mem1 = mem2;
        printf("Write: 0x%x, %d", mem1, mem2);
    }
    curs = RW_TIMES;
    for (i = 0; i < RW_TIMES; i++) {
        mem1 = a[RW_TIMES + i];
        sys_move_cursor(1, curs + i);
        memory[i + RW_TIMES] = *(int *) mem1;
        if (memory[i + RW_TIMES] == memory[i])
            printf("Read succeed: %d", memory[i + RW_TIMES]);
        else
            printf("Read error: %d", memory[i + RW_TIMES]);
    }
    while (1);
    //Input address from argv.
}
