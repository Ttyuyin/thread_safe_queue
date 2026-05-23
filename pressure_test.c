#include "ts_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>

int g_pass = 0;
int g_fail = 0;

#define MAX_TESTS 64

typedef struct {
    const char *name;
    int passed;
} TestRecord;

TestRecord g_records[MAX_TESTS];
int g_test_count = 0;

const char *g_current_test = NULL;

#define TEST(name)                                   \
    g_current_test = name;                            \
    printf("  TEST %-28s ", name)

#define PASS()                                       \
    g_records[g_test_count].name   = g_current_test;  \
    g_records[g_test_count].passed = 1;               \
    g_test_count++;                                   \
    g_pass++;                                         \
    printf("PASS\n")

#define FAIL(msg)                                    \
    g_records[g_test_count].name   = g_current_test;  \
    g_records[g_test_count].passed = 0;               \
    g_test_count++;                                   \
    g_fail++;                                         \
    printf("FAIL: %s\n", msg)

void test_InitQueue(void) {
    ThreadSafeQueue q;

    int ret = InitQueue(&q);

    TEST("InitQueue");
    if (ret == 0 && q.size == 0 && q.head == NULL && q.tail == NULL) {
        PASS();
    } else {
        FAIL("返回值或初始状态错误");
    }

    DestroyQueue(&q);
}

void test_EnQueue(void) {
    ThreadSafeQueue q;
    InitQueue(&q);

    int ret = EnQueue(&q, 42);
    TEST("EnQueue 单个元素");
    if (ret == 0 && q.size == 1) {
        PASS();
    } else {
        FAIL("入队失败");
    }

    EnQueue(&q, 1);
    EnQueue(&q, 2);
    TEST("EnQueue 三个元素 size");
    if (q.size == 3) {
        PASS();
    } else {
        FAIL("size != 3");
    }

    int v1, v2, v3;
    DeQueue(&q, &v1);
    DeQueue(&q, &v2);
    DeQueue(&q, &v3);
    TEST("EnQueue FIFO 顺序");
    if (v1 == 42 && v2 == 1 && v3 == 2) {
        PASS();
    } else {
        FAIL("顺序不对");
    }

    DestroyQueue(&q);
}

void test_DeQueue(void) {
    ThreadSafeQueue q;
    InitQueue(&q);

    int val = -1;
    int ret = DeQueue(&q, &val);
    TEST("DeQueue 空队列");
    if (ret == -1) {
        PASS();
    } else {
        FAIL("应返回 -1");
    }

    EnQueue(&q, 99);
    val = -1;
    ret = DeQueue(&q, &val);
    TEST("DeQueue 单个元素");
    if (ret == 0 && val == 99) {
        PASS();
    } else {
        FAIL("值或返回值错误");
    }

    EnQueue(&q, 1);
    EnQueue(&q, 2);
    EnQueue(&q, 3);
    DeQueue(&q, &val);
    DeQueue(&q, &val);
    DeQueue(&q, &val);
    TEST("DeQueue 取到空");
    if (q.size == 0) {
        PASS();
    } else {
        FAIL("size != 0");
    }

    DestroyQueue(&q);
}

void test_QueueSize(void) {
    ThreadSafeQueue q;
    InitQueue(&q);

    TEST("QueueSize 空队列");
    if (QueueSize(&q) == 0) {
        PASS();
    } else {
        FAIL("应为 0");
    }

    EnQueue(&q, 10);
    EnQueue(&q, 20);
    TEST("QueueSize 入队两个");
    if (QueueSize(&q) == 2) {
        PASS();
    } else {
        FAIL("应为 2");
    }

    int v;
    DeQueue(&q, &v);
    TEST("QueueSize 出队一个");
    if (QueueSize(&q) == 1) {
        PASS();
    } else {
        FAIL("应为 1");
    }

    DestroyQueue(&q);
}

void test_Contains(void) {
    ThreadSafeQueue q;
    InitQueue(&q);

    TEST("Contains 空队列");
    if (Contains(&q, 10) == 0) {
        PASS();
    } else {
        FAIL("应返回 0");
    }

    EnQueue(&q, 10);
    EnQueue(&q, 20);
    EnQueue(&q, 30);

    TEST("Contains 值存在");
    if (Contains(&q, 20) == 1) {
        PASS();
    } else {
        FAIL("应返回 1");
    }

    TEST("Contains 值不存在");
    if (Contains(&q, 99) == 0) {
        PASS();
    } else {
        FAIL("应返回 0");
    }
    DestroyQueue(&q);

}

void test_Find(void) {
    ThreadSafeQueue q;
    InitQueue(&q);

    TEST("Find 空队列");
    if (Find(&q, 10) == NULL) {
        PASS();
    } else {
        FAIL("应返回 NULL");
    }

    EnQueue(&q, 10);
    EnQueue(&q, 20);
    EnQueue(&q, 30);

    QueueNode *n = Find(&q, 20);
    TEST("Find 值存在");
    if (n != NULL && n->data == 20) {
        PASS();
    } else {
        FAIL("应找到 20");
    }

    TEST("Find 值不存在");
    if (Find(&q, 99) == NULL) {
        PASS();
    } else {
        FAIL("应返回 NULL");
    }

    DestroyQueue(&q);
}

void test_Print(void) {
    ThreadSafeQueue q;
    InitQueue(&q);

    TEST("Print 空队列");
    printf("\n        输出 => ");
    Print(&q);
    printf("         ");
    PASS();

    EnQueue(&q, 1);
    EnQueue(&q, 2);
    EnQueue(&q, 3);
    TEST("Print 非空队列 (FIFO)");
    printf("\n        输出 => ");
    Print(&q);
    printf("         ");
    PASS();

    DestroyQueue(&q);
}

void test_Clear(void) {
    ThreadSafeQueue q;
    InitQueue(&q);
    EnQueue(&q, 1);
    EnQueue(&q, 2);
    EnQueue(&q, 3);

    Clear(&q);
    TEST("Clear 清空");
    if (q.size == 0 && q.head == NULL && q.tail == NULL) {
        PASS();
    } else {
        FAIL("清空后状态不对");
    }

    Clear(&q);
    TEST("Clear 连续两次");
    if (q.size == 0) {
        PASS();
    } else {
        FAIL("二次 Clear 后 size != 0");
    }

    DestroyQueue(&q);
}

void test_DestroyQueue(void) {
    ThreadSafeQueue q;

    InitQueue(&q);
    TEST("DestroyQueue 空队列");
    DestroyQueue(&q);
    PASS();

    InitQueue(&q);
    EnQueue(&q, 42);
    TEST("DestroyQueue 单元素");
    DestroyQueue(&q);
    PASS();

    InitQueue(&q);
    for (int i = 0; i < 100; i++) {
        EnQueue(&q, i);
    }
    TEST("DestroyQueue 多元素");
    DestroyQueue(&q);
    PASS();
}

void test_NULL(void) {
    TEST("EnQueue(NULL, 0)");    EnQueue(NULL, 0);      PASS();
    TEST("DeQueue(NULL, NULL)"); DeQueue(NULL, NULL);   PASS();
    TEST("Clear(NULL)");         Clear(NULL);            PASS();

    TEST("Contains(NULL, 0)");
    if (Contains(NULL, 0) == 0) {
        PASS();
    } else {
        FAIL("应返回 0");
    }

    TEST("Find(NULL, 0)");
    if (Find(NULL, 0) == NULL) {
        PASS();
    } else {
        FAIL("应返回 NULL");
    }

    TEST("QueueSize(NULL)");
    if (QueueSize(NULL) == 0) {
        PASS();
    } else {
        FAIL("应返回 0");
    }

    TEST("Print(NULL)");        Print(NULL);             PASS();
    TEST("DestroyQueue(NULL)"); DestroyQueue(NULL);      PASS();
}

int g_stop = 0;

void *reader_thread(void *arg) {
    ThreadSafeQueue *q = (ThreadSafeQueue *)arg;
    unsigned seed = (unsigned)(uintptr_t)pthread_self();

    while (g_stop == 0) {
        int r = rand_r(&seed);
        unsigned op = (unsigned)r % 100;

        if (op < 33) {
            Contains(q, r % 10000);
        } else if (op < 66) {
            Find(q, r % 10000);
        } else if (op < 98) {
            QueueSize(q);
        } else {
            Print(q);
        }
    }
    return NULL;
}

#define WRITER_OPS 500000

void *writer_thread(void *arg) {
    ThreadSafeQueue *q = (ThreadSafeQueue *)arg;
    unsigned seed = (unsigned)(uintptr_t)pthread_self();

    for (int i = 0; i < WRITER_OPS; i++) {
        EnQueue(q, (int)(rand_r(&seed) % 100000));
        int val;
        DeQueue(q, &val);
    }
    return NULL;
}

#define CLEAR_INTERVAL 5000

void *clearer_thread(void *arg) {
    ThreadSafeQueue *q = (ThreadSafeQueue *)arg;
    unsigned seed = (unsigned)(uintptr_t)pthread_self();
    long ops = 0;

    while (g_stop == 0) {
        EnQueue(q, (int)(rand_r(&seed) % 100000));
        int val;
        DeQueue(q, &val);
        ops++;

        if (ops % CLEAR_INTERVAL == 0) {
            Clear(q);
        }
    }
    return NULL;
}

void test_ConcurrentReadWrite(void) {
    ThreadSafeQueue q;
    pthread_t readers[3], writers[2], clearer;
    int val;
    int i;

    g_stop = 0;
    InitQueue(&q);

    for (i = 0; i < 100; i++) {
        EnQueue(&q, i);
    }

    printf("\n  [ 读写并发: 3 readers + 2 writers(%d) + 1 clearer ]\n",
           WRITER_OPS);

    pthread_create(&clearer, NULL, clearer_thread, &q);
    for (i = 0; i < 3; i++) {
        pthread_create(&readers[i], NULL, reader_thread, &q);
    }
    for (i = 0; i < 2; i++) {
        pthread_create(&writers[i], NULL, writer_thread, &q);
    }

    for (i = 0; i < 2; i++) {
        pthread_join(writers[i], NULL);
    }

    g_stop = 1;

    pthread_join(clearer, NULL);
    for (i = 0; i < 3; i++) {
        pthread_join(readers[i], NULL);
    }

    while (DeQueue(&q, &val) == 0) {
    }

    TEST("读写并发（不崩溃、不死锁）");
    if (q.size == 0 && q.head == NULL && q.tail == NULL) {
        PASS();
    } else {
        FAIL("最终队列状态不正确");
    }

    DestroyQueue(&q);
}

#define PRODUCERS          4
#define CONSUMERS          4
#define ITEMS_PER_PRODUCER  500000
#define TOTAL_ITEMS        (PRODUCERS * ITEMS_PER_PRODUCER)

atomic_long g_total_enq;
atomic_long g_total_deq;

typedef struct {
    ThreadSafeQueue *q;
    int id;
} ProducerArg;

void *producer_routine(void *arg) {
    ProducerArg *pa = (ProducerArg *)arg;
    long start = (long)pa->id * ITEMS_PER_PRODUCER;
    long end   = start + ITEMS_PER_PRODUCER;

    for (long i = start; i < end; i++) {
        EnQueue(pa->q, (int)i);
        atomic_fetch_add(&g_total_enq, 1);
    }
    return NULL;
}

void *consumer_routine(void *arg) {
    ThreadSafeQueue *q = (ThreadSafeQueue *)arg;
    int val;

    while (atomic_load(&g_total_deq) < TOTAL_ITEMS) {
        if (DeQueue(q, &val) == 0) {
            atomic_fetch_add(&g_total_deq, 1);
        }
    }
    return NULL;
}

int check_final_state(ThreadSafeQueue *q) {
    pthread_rwlock_rdlock(&q->lock);
    int ok = (q->head == NULL && q->tail == NULL && q->size == 0);
    pthread_rwlock_unlock(&q->lock);
    return ok;
}

void test_Stress(void) {
    ThreadSafeQueue queue;
    pthread_t producers[PRODUCERS];
    pthread_t consumers[CONSUMERS];
    ProducerArg p_args[PRODUCERS];
    int i;

    printf("\n  [ 压力测试: %d 生产者 x %d = %d 入队, %d 消费者 ]\n",
           PRODUCERS, ITEMS_PER_PRODUCER, TOTAL_ITEMS, CONSUMERS);

    atomic_init(&g_total_enq, 0);
    atomic_init(&g_total_deq, 0);
    InitQueue(&queue);

    for (i = 0; i < CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer_routine, &queue);
    }

    for (i = 0; i < PRODUCERS; i++) {
        p_args[i].q  = &queue;
        p_args[i].id = i;
        pthread_create(&producers[i], NULL, producer_routine, &p_args[i]);
    }

    for (i = 0; i < PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }

    for (i = 0; i < CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    int val;
    while (DeQueue(&queue, &val) == 0) {
        atomic_fetch_add(&g_total_deq, 1);
    }

    long enq = atomic_load(&g_total_enq);
    long deq = atomic_load(&g_total_deq);
    size_t size = QueueSize(&queue);
    int state_ok = check_final_state(&queue);

    int pass = (enq == TOTAL_ITEMS) && (deq == TOTAL_ITEMS)
            && (size == 0) && state_ok;

    TEST("压力测试");
    if (pass) {
        PASS();
    } else {
        FAIL("数据不一致");
    }

    printf("           enq=%ld  deq=%ld  size=%lu  head/tail=%s\n",
           enq, deq, (unsigned long)size, state_ok ? "NULL" : "非NULL");

    DestroyQueue(&queue);
}

int main(void) {
    printf("========================================\n");
    printf("   ThreadSafeQueue 综合测试\n");
    printf("========================================\n\n");

    printf("--- 单线程功能测试 ---\n");
    test_InitQueue();
    test_EnQueue();
    test_DeQueue();
    test_QueueSize();
    test_Contains();
    test_Find();
    test_Print();
    test_Clear();
    test_DestroyQueue();
    test_NULL();

    printf("\n--- 多线程并发测试 ---\n");
    test_ConcurrentReadWrite();

    printf("\n--- 压力测试 ---\n");
    test_Stress();

    printf("\n========================================\n");
    printf("   结果: %d 通过, %d 失败  %s\n",
           g_pass, g_fail,
           g_fail > 0 ? "SOME FAILED" : "全部通过");

    printf("\n--- 测试详细清单 ---\n");
    for (int i = 0; i < g_test_count; i++) {
        printf("  [%s] %s\n",
               g_records[i].passed ? "PASS" : "FAIL",
               g_records[i].name);
    }
    printf("========================================\n");

    if (g_fail > 0) {
        return 1;
    } else {
        return 0;
    }
}
