#ifndef TS_QUEUE_H
#define TS_QUEUE_H

#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

//定义队列结点
typedef struct QueueNode {
    int data;
    struct QueueNode *next;
} QueueNode;

//定义队列
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


//初始化队列
int InitQueue(ThreadSafeQueue *q) {
    if (q == NULL) return -1;

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return pthread_rwlock_init(&q->lock, NULL);
}

//入队
int EnQueue(ThreadSafeQueue *q, int value) {
    if (q == NULL) return -1;

    QueueNode *node = (QueueNode *)malloc(sizeof(*node));
    if (node == NULL) return -1;

    node->data = value;
    node->next = NULL;

    pthread_rwlock_wrlock(&q->lock);

    if (q->tail == NULL) {
        q->head = node;
        q->tail = node;
    } else {
        q->tail->next = node;
        q->tail = node;
    }
    q->size++;

    pthread_rwlock_unlock(&q->lock);
    return 0;
}

//出队列
int DeQueue(ThreadSafeQueue *q, int *out_value) {
    if (q == NULL) return -1;

    pthread_rwlock_wrlock(&q->lock);

    QueueNode *node = q->head;
    if (node == NULL) {
        pthread_rwlock_unlock(&q->lock);
        return -1;
    }

    q->head = node->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    q->size--;

    pthread_rwlock_unlock(&q->lock);

    if (out_value != NULL)
        *out_value = node->data;

    free(node);
    return 0;
}

//清空队列
void Clear(ThreadSafeQueue *q) {
    if (q == NULL) return;

    pthread_rwlock_wrlock(&q->lock);

    QueueNode *node = q->head;
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;

    pthread_rwlock_unlock(&q->lock);

    while (node != NULL) {
        QueueNode *next = node->next;
        free(node);
        node = next;
    }
}

//验证某个数据是否存在
int Contains(ThreadSafeQueue *q, int value) {
    if (q == NULL) return 0;

    pthread_rwlock_rdlock(&q->lock);

    for (QueueNode *cur = q->head; cur != NULL; cur = cur->next) {
        if (cur->data == value) {
            pthread_rwlock_unlock(&q->lock);
            return 1;
        }
    }

    pthread_rwlock_unlock(&q->lock);
    return 0;
}

//验证数据是否存在
QueueNode *Find(ThreadSafeQueue *q, int value) {
    if (q == NULL) return NULL;

    pthread_rwlock_rdlock(&q->lock);

    for (QueueNode *cur = q->head; cur != NULL; cur = cur->next) {
        if (cur->data == value) {
            pthread_rwlock_unlock(&q->lock);
            return cur;
        }
    }

    pthread_rwlock_unlock(&q->lock);
    return NULL;
}

//打印队列数据
void Print(ThreadSafeQueue *q) {
    if (q == NULL) return;

    pthread_rwlock_rdlock(&q->lock);

    printf("queue(size=%lu):", (unsigned long)q->size);
    for (QueueNode *cur = q->head; cur != NULL; cur = cur->next)
        printf(" %d", cur->data);
    putchar('\n');

    pthread_rwlock_unlock(&q->lock);
}

//销毁队列
void DestroyQueue(ThreadSafeQueue *q) {
    if (q == NULL) return;
    Clear(q);
    pthread_rwlock_destroy(&q->lock);
}

//队列长度
size_t QueueSize(ThreadSafeQueue *q) {
    if (q == NULL) return 0;

    pthread_rwlock_rdlock(&q->lock);
    size_t size = q->size;
    pthread_rwlock_unlock(&q->lock);

    return size;
}

#endif
