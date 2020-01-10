#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#define MAX_NUM_BOX 32
#define MAX_BUFFER_LENGTH 64
#define MAX_NAME_LENGTH 32
#include "cond.h"
typedef struct mailbox
{
    char name[MAX_NAME_LENGTH]; //the name of mailbox
    char buffer[MAX_BUFFER_LENGTH]; //mailbox data
    int user_num;
    int num_item; //total num of item in mailbox
    int read_index; //read pointer
    int write_index; //write pointer

    condition_t full;
    condition_t empty;
    mutex_lock_t mutex;
} mailbox_t;


void do_mbox_init();
mailbox_t *mbox_open(char *);
void mbox_close(mailbox_t *);
void mbox_send(mailbox_t *, void *, int);
void mbox_recv(mailbox_t *, void *, int);

#endif
