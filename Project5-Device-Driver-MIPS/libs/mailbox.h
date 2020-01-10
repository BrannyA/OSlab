#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#include "cond.h"
#include "string.h"

typedef struct mailbox {
    char name[20];
    char msg[20];
    int msg_length;
    int msg_num;
    int access;
    mutex_lock_t lock;
    condition_t full;
    condition_t empty;
} mailbox_t;


void mbox_init();

mailbox_t *mbox_open(char *);

void mbox_close(mailbox_t *);

void mbox_send(mailbox_t *, void *, int);

void mbox_recv(mailbox_t *, void *, int);

#endif