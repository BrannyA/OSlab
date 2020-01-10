#include "queue.h"
#include "sched.h"

typedef pcb_t item_t;

void queue_init(queue_t *queue) {
    queue->head = queue->tail = NULL;
}

int queue_is_empty(queue_t *queue) {
    if (queue->head == NULL) {
        return 1;
    }
    return 0;
}

void queue_push(queue_t *queue, void *item) {
    item_t *_item = (item_t *) item;
    /* queue is empty */
    if (queue->head == NULL) {
        queue->head = item;
        queue->tail = item;
        _item->next = NULL;
        _item->prev = NULL;
    } else {
        ((item_t *) (queue->tail))->next = item;
        _item->next = NULL;
        _item->prev = queue->tail;
        queue->tail = item;
    }
}

void *queue_dequeue(queue_t *queue) {
    item_t *temp = (item_t *) queue->head;

    /* this queue only has one item */
    if (temp->next == NULL) {
        queue->head = queue->tail = NULL;
    } else {
        queue->head = ((item_t *) (queue->head))->next;
        ((item_t *) (queue->head))->prev = NULL;
    }

    temp->prev = NULL;
    temp->next = NULL;

    return (void *) temp;
}

void priority_push(queue_t *queue, void *item) {
    item_t *_item = (item_t *) item;
    /* queue is empty */
    if (queue->head == NULL) {
        queue->head = item;
        queue->tail = item;
        _item->next = NULL;
        _item->prev = NULL;
    } else {
        if (((item_t *) (queue->head))->priority < _item->priority) {
            _item->prev = NULL;
            _item->next = queue->head;
            ((item_t *) (queue->head))->prev = item;
            queue->head = item;
        } else {
            item_t *find = queue->head;
            while (find->next != NULL && ((item_t *) (find->next))->priority >= _item->priority) {
                find = find->next;
            }
            if (find->next == NULL) {
                queue->tail = item;
            }
            _item->prev = find;
            _item->next = find->next;
            find->next = item;
        }
    }
}

void priority_increase(queue_t *queue) {
    item_t *find = queue->head;
    while (find->next != NULL) {
        find->priority += 3;
        find = find->next;
    }
    find->priority += 3;
}

/* remove this item and return next item */
void *queue_remove(queue_t *queue, void *item) {
    item_t *_item = (item_t *) item;
    item_t *next = (item_t *) _item->next;

    if (item == queue->head && item == queue->tail) {
        queue->head = NULL;
        queue->tail = NULL;
    } else if (item == queue->head) {
        queue->head = _item->next;
        ((item_t *) (queue->head))->prev = NULL;
    } else if (item == queue->tail) {
        queue->tail = _item->prev;
        ((item_t *) (queue->tail))->next = NULL;
    } else {
        ((item_t *) (_item->prev))->next = _item->next;
        ((item_t *) (_item->next))->prev = _item->prev;
    }

    _item->prev = NULL;
    _item->next = NULL;

    return (void *) next;
}
