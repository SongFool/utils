
#include <stdint.h>
#include "queue.h"
int queue_push(queue_t *q, void *data)
{
    if(q->count < q->size){
        q->data[q->write] = data;
        q->write = (q->write + 1) % q->size;
        q->count++;
        return 0;
    }
    return -1;
}

void * queue_pop(queue_t *q)
{
    void *data = NULL;
    if(q->count > 0){
        data = q->data[q->read];
        q->read = (q->read + 1) % q->size;
        q->count--;
        return data;
    }
    return NULL;
}
int queue_init(queue_t *q, int size,void **data)
{
    q->count = 0;
    q->data = data;
    q->read = 0;
    q->size = size;
    q->write = 0;
    return 0;
}
