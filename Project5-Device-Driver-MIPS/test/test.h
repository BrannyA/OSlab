#ifndef INCLUDE_TEST_H
#define INCLUDE_TEST_H

#include "test4.h"
#include "sched.h"

extern struct task_info *test_tasks[19];

void test_shell(void);

extern void mac_send_task(void);

extern void mac_recv_task(void);

extern void mac_init_task(void);

#endif
