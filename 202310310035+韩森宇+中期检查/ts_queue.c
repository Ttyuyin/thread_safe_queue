#include "ts_queue.h"

#include <stdio.h>
#include <stdlib.h>


int InitQueue(ThreadSafeQueue *q) {
    if (q == NULL) return -1;

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return pthread_mutex_init(&q->lock, NULL);
}

int EnQueue(ThreadSafeQueue *q, int value) {
    if (q == NULL) return -1;

    QueueNode *node = (QueueNode *)malloc(sizeof(*node));
    if (node == NULL) return -1;

    node->data = value;
    node->next = NULL;

    pthread_mutex_lock(&q->lock);

    if (q->tail == NULL) {       
        q->head = node;
        q->tail = node;
    } else {                         
        q->tail->next = node;
        q->tail = node;
    }
    q->size++;

    pthread_mutex_unlock(&q->lock);
    return 0;
}

int DeQueue(ThreadSafeQueue *q, int *out_value) {
    if (q == NULL) return -1;

    pthread_mutex_lock(&q->lock);

    QueueNode *node = q->head;
    if (node == NULL) {              
        pthread_mutex_unlock(&q->lock);
        return -1;
    }

    q->head = node->next;          
    if (q->head == NULL) {          
        q->tail = NULL;
    }
    q->size--;

    pthread_mutex_unlock(&q->lock);

    if (out_value != NULL)
        *out_value = node->data;

    free(node);                    
    return 0;
}

void Clear(ThreadSafeQueue *q) {
    if (q == NULL) return;

    pthread_mutex_lock(&q->lock);

    QueueNode *node = q->head;
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;

    pthread_mutex_unlock(&q->lock);

    while (node != NULL) {
        QueueNode *next = node->next;
        free(node);
        node = next;
    }
}

int Contains(ThreadSafeQueue *q, int value) {
    if (q == NULL) return 0;

    pthread_mutex_lock(&q->lock);

    for (QueueNode *cur = q->head; cur != NULL; cur = cur->next) {
        if (cur->data == value) {
            pthread_mutex_unlock(&q->lock);
            return 1;
        }
    }

    pthread_mutex_unlock(&q->lock);
    return 0;
}

QueueNode *Find(ThreadSafeQueue *q, int value) {
    if (q == NULL) return NULL;

    pthread_mutex_lock(&q->lock);

    QueueNode *cur = q->head;
    while (cur != NULL) {
        if (cur->data == value) {
            pthread_mutex_unlock(&q->lock);
            return cur;
        }
        cur = cur->next;
    }

    pthread_mutex_unlock(&q->lock);
    return NULL;
}

void Print(ThreadSafeQueue *q) {
    if (q == NULL) return;

    pthread_mutex_lock(&q->lock);

    printf("queue(size=%lu):", (unsigned long)q->size);
    for (QueueNode *cur = q->head; cur != NULL; cur = cur->next)
        printf(" %d", cur->data);
    putchar('\n');

    pthread_mutex_unlock(&q->lock);
}

void DestroyQueue(ThreadSafeQueue *q) {
    if (q == NULL) return;
    Clear(q);
    pthread_mutex_destroy(&q->lock);
}

size_t QueueSize(ThreadSafeQueue *q) {
    if (q == NULL) return 0;

    pthread_mutex_lock(&q->lock);
    size_t size = q->size;
    pthread_mutex_unlock(&q->lock);

    return size;
}
