#ifndef TS_QUEUE_H
#define TS_QUEUE_H

#include <pthread.h>
#include <stddef.h>

typedef struct QueueNode {
    int data;
    struct QueueNode *next;
} QueueNode;

typedef struct ThreadSafeQueue {
    QueueNode *head;
    QueueNode *tail;
    size_t size;
    pthread_rwlock_t lock;
} ThreadSafeQueue;

int InitQueue(ThreadSafeQueue *q);

int EnQueue(ThreadSafeQueue *q, int value);

int DeQueue(ThreadSafeQueue *q, int *out_value);

void Clear(ThreadSafeQueue *q);

int Contains(ThreadSafeQueue *q, int value);

QueueNode *Find(ThreadSafeQueue *q, int value);

void Print(ThreadSafeQueue *q);

void DestroyQueue(ThreadSafeQueue *q);

size_t QueueSize(ThreadSafeQueue *q);

#endif
