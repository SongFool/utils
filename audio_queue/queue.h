
typedef struct{
    void **data;
    int count;
    int size;
    int read;
    int write;
}queue_t;
int queue_push(queue_t *q, void *data);
void * queue_pop(queue_t *q);
int queue_init(queue_t *q, int size,void **data);
