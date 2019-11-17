#include "string.h"
#include "mailbox.h"

static mailbox_t mbox[MAX_NUM_BOX];
static mutex_lock_t mutex;

void mbox_init()
{
    int i;
    mutex_lock_init(&mutex);

    for(i = 0; i < MAX_NUM_BOX; i++)
    {
        mbox[i].name[0] = '\0';
        mbox[i].user_num = 0;
        mbox[i].num_item = 0;
        mbox[i].read_index = 0;
        mbox[i].write_index = 0;
        bzero( (void *)mbox[i].buffer,  MAX_BUFFER_LENGTH);

        condition_init(&mbox[i].full);
        condition_init(&mbox[i].empty);
        mutex_lock_init(&mbox[i].mutex);
    }
}

mailbox_t *mbox_open(char *name)
{
    sys_move_cursor(0, 0);
    int i;
    mutex_lock_acquire(&mutex);
    for(i = 0; i < MAX_NUM_BOX; i++)
    {
        if( strcmp(mbox[i].name, name) == 0 && mbox[i].user_num != 0)
        {
            mbox[i].user_num++;
            mutex_lock_release(&mutex);
            return &mbox[i];
        }
    }
    //new mbox
    for(i = 0; i < MAX_NUM_BOX; i++)
    {
        // printk("hi mbox[%d]\n",i);
        if( mbox[i].user_num == 0 )
        {
            strcpy(mbox[i].name, name);
            mbox[i].user_num++;
            mutex_lock_release(&mutex);
            // printk("before return %s", name);
            return &mbox[i];
        }
    }
}

void mbox_close(mailbox_t *mailbox)
{
    mutex_lock_acquire(&mutex);
    mailbox->user_num -- ;
    if(mailbox->user_num == 0)
    {
        mailbox->name[0] = '\0';
        mailbox->user_num = 0;
        mailbox->num_item = 0;
        mailbox->read_index = 0;
        mailbox->write_index = 0;
        bzero( (void *)mailbox->buffer,  MAX_BUFFER_LENGTH);

        condition_init(&(mailbox->full));
        condition_init(&(mailbox->empty));
        mutex_lock_init(&(mailbox->mutex));
    }
    mutex_lock_release(&mutex);
//!empty
}

void mbox_send(mailbox_t *mailbox, void *msg, int msg_length)
{
    // printk("sendhere ");
    mutex_lock_acquire(&mailbox->mutex);
    int i;
    // printk("222222222222 ");
    for(i = 0; i < msg_length; i++)
    {
        //full
        while(mailbox->num_item == MAX_BUFFER_LENGTH)
            condition_wait(&(mailbox->mutex), &(mailbox->empty));
        //not full
        mailbox->buffer[mailbox->write_index++] = *((char *)msg + i);
        mailbox->write_index %= MAX_BUFFER_LENGTH;
        mailbox->num_item++;
    }
    // printk("33333333333 ");
    condition_broadcast(&(mailbox->full)); //unblock all
    mutex_lock_release(&(mailbox->mutex));
    // printk("sendend ");
}

void mbox_recv(mailbox_t *mailbox, void *msg, int msg_length)
{
    int i;
    mutex_lock_acquire(&(mailbox->mutex));
    for(i = 0; i < msg_length; i++)
    {
        //empty
        while(mailbox->num_item == 0)
            condition_wait(&(mailbox->mutex), &(mailbox->full));
        //not empty
        *((char *)msg + i) = mailbox->buffer[mailbox->read_index++];
        mailbox->read_index %= MAX_BUFFER_LENGTH;
        mailbox->num_item--;
    }
    condition_broadcast(&(mailbox->empty));
    mutex_lock_release(&(mailbox->mutex));
}
