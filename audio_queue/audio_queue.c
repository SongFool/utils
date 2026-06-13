#include "queue.h"
#include <stdint.h>
#include <stdio.h>
#define AUDIO_QUEUE_SIZE 3

typedef struct 
{
    uint8_t flag;
    uint8_t use;
    uint16_t frame[512];
}audio_data_t;

static queue_t audio_queue;
static audio_data_t audio_data[AUDIO_QUEUE_SIZE];
static void *audio_queue_buf[AUDIO_QUEUE_SIZE];
queue_t * audio_queue_get()
{
    return &audio_queue;
}

int audio_queue_init(int size, audio_data_t *data)
{ 
    queue_init(&audio_queue,
               AUDIO_QUEUE_SIZE,
               audio_queue_buf);

    return 0;
}
audio_data_t *audio_data_alloc(void)
{
    int i;

    for(i = 0; i < AUDIO_QUEUE_SIZE; i++)
    {
        if(audio_data[i].use == 0)
        {
            audio_data[i].use = 1;
            return &audio_data[i];
        }
    }

    return NULL;
}
void audio_data_free(audio_data_t *data)
{
    if(data)
    {
        data->use = 0;
    }
}
int audio_queue_push(audio_data_t *data)
{
    return queue_push(&audio_queue, data);
}

audio_data_t * audio_queue_pop()
{
    return queue_pop(&audio_queue);
}

#if 0
int main(void)
{
    audio_data_t *p1;
    audio_data_t *p2;
    audio_data_t *p3;
    audio_data_t *p4;

    audio_queue_init(0, NULL);

    printf("========== alloc test ==========\n");

    p1 = audio_data_alloc();
    p2 = audio_data_alloc();
    p3 = audio_data_alloc();
    p4 = audio_data_alloc();

    printf("p1=%p\n", p1);
    printf("p2=%p\n", p2);
    printf("p3=%p\n", p3);
    printf("p4=%p (should be NULL)\n", p4);

    if(p1)
    {
        p1->flag = 1;
        p1->frame[0] = 111;
    }

    if(p2)
    {
        p2->flag = 2;
        p2->frame[0] = 222;
    }

    if(p3)
    {
        p3->flag = 3;
        p3->frame[0] = 333;
    }

    printf("\n========== push test ==========\n");

    printf("push p1 ret=%d\n", audio_queue_push(p1));
    printf("push p2 ret=%d\n", audio_queue_push(p2));
    printf("push p3 ret=%d\n", audio_queue_push(p3));

    printf("\n========== pop test ==========\n");

    audio_data_t *data;

    data = audio_queue_pop();
    if(data)
    {
        printf("pop flag=%d frame0=%d\n",
               data->flag,
               data->frame[0]);

        audio_data_free(data);
    }

    data = audio_queue_pop();
    if(data)
    {
        printf("pop flag=%d frame0=%d\n",
               data->flag,
               data->frame[0]);

        audio_data_free(data);
    }

    data = audio_queue_pop();
    if(data)
    {
        printf("pop flag=%d frame0=%d\n",
               data->flag,
               data->frame[0]);

        audio_data_free(data);
    }

    data = audio_queue_pop();
    printf("pop empty=%p\n", data);

    printf("\n========== realloc test ==========\n");

    p1 = audio_data_alloc();

    printf("realloc p1=%p\n", p1);

    if(p1)
    {
        p1->flag = 99;
        p1->frame[0] = 999;

        audio_queue_push(p1);
    }

    data = audio_queue_pop();

    if(data)
    {
        printf("pop flag=%d frame0=%d\n",
               data->flag,
               data->frame[0]);

        audio_data_free(data);
    }

    return 0;
}
    #endif
    #include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void *audio_record_thread(void *arg)
{
    int cnt = 0;

    while(1)
    {
        audio_data_t *data = audio_data_alloc();

        if(data == NULL)
        {
            printf("[REC] no free buffer\n");
            usleep(10000);
            continue;
        }

        data->flag = cnt;
        data->frame[0] = cnt;

        if(audio_queue_push(data) != 0)
        {
            printf("[REC] queue full\n");
            audio_data_free(data);
        }
        else
        {
            printf("[REC] push %d\n", cnt);
            cnt++;
        }

        usleep(50000);
    }

    return NULL;
}

void *audio_play_thread(void *arg)
{
    while(1)
    {
        audio_data_t *data = audio_queue_pop();

        if(data == NULL)
        {
            usleep(10000);
            continue;
        }

        printf("[PLAY] flag=%d frame0=%d\n",
               data->flag,
               data->frame[0]);

        audio_data_free(data);

        usleep(100000);
    }

    return NULL;
}

int main(void)
{
    pthread_t tid_record;
    pthread_t tid_play;

    audio_queue_init(0, NULL);

    pthread_create(&tid_record,
                   NULL,
                   audio_record_thread,
                   NULL);

    pthread_create(&tid_play,
                   NULL,
                   audio_play_thread,
                   NULL);

    pthread_join(tid_record, NULL);
    pthread_join(tid_play, NULL);

    return 0;
}
