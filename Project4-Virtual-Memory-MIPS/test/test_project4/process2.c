#include "sched.h"
#include "stdio.h"
#include "syscall.h"
#include "time.h"
#include "screen.h"
#include "test4.h"

#define RW_TIMES 3

int rand()
{
    int current_time = get_timer();
    return current_time % 1000;
}

void rw_task1()
{
    int mem1, mem2 = 0;
    int curs = 0;
    int memory[RW_TIMES * 2];
    int i = 0;
    for (i = 0; i < RW_TIMES; i++)
    {
        // mem1 will become the address
        // mem2 will become the number to be put in
        sys_move_cursor(1, curs + i);
        mem1 = a[i];
        sys_move_cursor(1, curs + i);
        memory[i] = mem2 = rand();
        *(int *) mem1 = mem2;
        printf("Write: 0x%x, %d\n", mem1, mem2);
    }

    curs = RW_TIMES;
    for (i = 0; i < RW_TIMES; i++)
    {
        // mem1 will become the address from which number is withdrawn
        sys_move_cursor(1, curs + i);
        mem1 = a[RW_TIMES + i];
        sys_move_cursor(1, curs + i);
        memory[i + RW_TIMES] = *(int *) mem1;
        if (memory[i + RW_TIMES] == memory[i])
            printf("Read succeed: %d\n", memory[i + RW_TIMES]);
        else
            printf("Read error: %d   \n", memory[i + RW_TIMES]);
    }
    while (1);
}
