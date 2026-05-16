#include "ts_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#define PRODUCERS          4
#define CONSUMERS          4
#define ITEMS_PER_PRODUCER  2500000
#define TOTAL_ITEMS        (PRODUCERS * ITEMS_PER_PRODUCER)

static atomic_long g_total_enq;
static atomic_long g_total_deq;

typedef struct {
    ThreadSafeQueue *q;
    int id;
} ProducerArg;

static void *producer_routine(void *arg) {
    ProducerArg *pa = (ProducerArg *)arg;
    long start = (long)pa->id * ITEMS_PER_PRODUCER;
    long end   = start + ITEMS_PER_PRODUCER;

    for (long i = start; i < end; i++) {
        EnQueue(pa->q, (int)i);
        atomic_fetch_add(&g_total_enq, 1);
    }
    return NULL;
}

static void *consumer_routine(void *arg) {
    ThreadSafeQueue *q = (ThreadSafeQueue *)arg;
    int val;

    while (atomic_load(&g_total_deq) < TOTAL_ITEMS) {
        if (DeQueue(q, &val) == 0) {
            atomic_fetch_add(&g_total_deq, 1);
        }
    }
    return NULL;
}

static int check_final_state(ThreadSafeQueue *q) {
    pthread_mutex_lock(&q->lock);
    int ok = (q->head == NULL && q->tail == NULL && q->size == 0);
    pthread_mutex_unlock(&q->lock);
    return ok;
}

int main() {
    ThreadSafeQueue queue;
    pthread_t producers[PRODUCERS];
    pthread_t consumers[CONSUMERS];
    ProducerArg p_args[PRODUCERS];

    printf("压力测试：%d 生产者 x %d = %d 次入队, %d 消费者\n\n",
           PRODUCERS, ITEMS_PER_PRODUCER, TOTAL_ITEMS, CONSUMERS);

    if (InitQueue(&queue) != 0) {
        fprintf(stderr, "InitQueue 失败\n");
        return 1;
    }

    for (int i = 0; i < CONSUMERS; i++)
        pthread_create(&consumers[i], NULL, consumer_routine, &queue);

    for (int i = 0; i < PRODUCERS; i++) {
        p_args[i].q  = &queue;
        p_args[i].id = i;
        pthread_create(&producers[i], NULL, producer_routine, &p_args[i]);
    }

    for (int i = 0; i < PRODUCERS; i++)
        pthread_join(producers[i], NULL);

    for (int i = 0; i < CONSUMERS; i++)
        pthread_join(consumers[i], NULL);

    long drain = 0;
    int val;
    while (DeQueue(&queue, &val) == 0) {
        drain++;
        atomic_fetch_add(&g_total_deq, 1);
    }

    long enq = atomic_load(&g_total_enq);
    long deq = atomic_load(&g_total_deq);
    size_t size = QueueSize(&queue);
    int state_ok = check_final_state(&queue);

    printf("total_enq      = %ld\n", enq);
    printf("total_deq      = %ld\n", deq);
    printf("drain          = %ld\n", drain);
    printf("QueueSize      = %lu\n", (unsigned long)size);
    printf("head/tail NULL = %s\n", state_ok ? "yes" : "no");

    int pass = (enq == TOTAL_ITEMS)
            && (deq == TOTAL_ITEMS)
            && (size == 0)
            && state_ok;

    printf("result = %s\n", pass ? "PASS" : "FAIL");

    DestroyQueue(&queue);
    return pass ? 0 : 1;
}
