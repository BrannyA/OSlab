#include "string.h"
#include "mailbox.h"

#define MAX_NUM_BOX 32

static mailbox_t mboxs[MAX_NUM_BOX];

void mbox_init() {
    int i;
    for (i = 0; i < MAX_NUM_BOX; i++) {
        mboxs[i].name[0] = '\0';
        mboxs[i].msg[0] = '\0';
        mboxs[i].msg_length = 0;
        mboxs[i].msg_num = 0;
        mboxs[i].access = 0;
        mutex_lock_init(&(mboxs[i].lock));
        condition_init(&(mboxs[i].full));
        condition_init(&(mboxs[i].empty));
    }
}

mailbox_t *mbox_open(char *name) {
    int i, j;
    for (i = 0; i < MAX_NUM_BOX; i++) {
        for (j = 0; mboxs[i].name[j] != '\0' && mboxs[i].name[j] == name[j]; j++);
        if (mboxs[i].name[j] == '\0' && name[j] == '\0') {
            mboxs[i].access = mboxs[i].access + 1;
            return &mboxs[i];
        }
    }
    for (i = 0; i < MAX_NUM_BOX && mboxs[i].name[0] != '\0'; i++);
    {
        if (i == MAX_NUM_BOX) {
            printf("open mailbox failed! TOO MANY MAILBOX\n");
            return 0;
        } else {
            for (j = 0; name[j] != '\0'; j++)
                mboxs[i].name[j] = name[j];
            mboxs[i].name[j] = '\0';
            mboxs[i].access = mboxs[i].access + 1;
            return &mboxs[i];
        }
    }
}

void mbox_close(mailbox_t *mailbox) {
    mailbox->access = mailbox->access - 1;
    if (mailbox->access == 0) {
        mailbox->name[0] = '\0';
        mailbox->msg[0] = '\0';
        mailbox->msg_length = 0;
        mailbox->msg_num = 0;
        mutex_lock_init(&(mailbox->lock));
        condition_init(&(mailbox->full));
        condition_init(&(mailbox->empty));
    }
}

void mbox_send(mailbox_t *mailbox, void *msg, int msg_length) {
    mutex_lock_acquire(&(mailbox->lock));
    if (mailbox->msg_num == 1)
        condition_wait(&(mailbox->lock), &(mailbox->empty));
    memcpy(mailbox->msg, msg, msg_length);
    mailbox->msg_length = msg_length;
    mailbox->msg_num = 1;
    condition_signal(&(mailbox->full));
    mutex_lock_release(&(mailbox->lock));
}

void mbox_recv(mailbox_t *mailbox, void *msg, int msg_length) {
    mutex_lock_acquire(&(mailbox->lock));
    if (mailbox->msg_num == 0)
        condition_wait(&(mailbox->lock), &(mailbox->full));
    memcpy(msg, mailbox->msg, msg_length);
    mailbox->msg[0] = '\0';
    mailbox->msg_length = 0;
    mailbox->msg_num = 0;
    condition_signal(&(mailbox->empty));
    mutex_lock_release(&(mailbox->lock));
}