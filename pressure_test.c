#include "ts_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* 测试结果统计：同时记录每条结果，最后汇总输出 */
int g_pass = 0;
int g_fail = 0;

#define MAX_TESTS 64

/* 一条测试记录 */
typedef struct {
    const char *name;   /* 测试名（指针，TEST 传进来的字符串常量） */
    int passed;         /* 1 = 通过, 0 = 失败 */
} TestRecord;

TestRecord g_records[MAX_TESTS];
int g_test_count = 0;

/* 当前正在跑的测试名字，由 TEST 设置，PASS/FAIL 读取 */
const char *g_current_test = NULL;

/* 宏：记录测试名并输出 */
#define TEST(name)                                   \
    g_current_test = name;                            \
    printf("  TEST %-28s ", name)

/* 宏：记录通过 */
#define PASS()                                       \
    g_records[g_test_count].name   = g_current_test;  \
    g_records[g_test_count].passed = 1;               \
    g_test_count++;                                   \
    g_pass++;                                         \
    printf("PASS\n")

/* 宏：记录失败 */
#define FAIL(msg)                                    \
    g_records[g_test_count].name   = g_current_test;  \
    g_records[g_test_count].passed = 0;               \
    g_test_count++;                                   \
    g_fail++;                                         \
    printf("FAIL: %s\n", msg)

/* =========================================================
 *  一、单线程功能测试
 *  逐个函数验证基本功能
 * ========================================================= */

/* ---------- InitQueue ---------- */
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

/* ---------- EnQueue ---------- */
void test_EnQueue(void) {
    ThreadSafeQueue q;
    InitQueue(&q);

    /* 入队一个元素 */
    int ret = EnQueue(&q, 42);
    TEST("EnQueue 单个元素");
    if (ret == 0 && q.size == 1) {
        PASS();
    } else {
        FAIL("入队失败");
    }

    /* 入队多个元素 */
    EnQueue(&q, 1);
    EnQueue(&q, 2);
    TEST("EnQueue 三个元素 size");
    if (q.size == 3) {
        PASS();
    } else {
        FAIL("size != 3");
    }

    /* 验证 FIFO 顺序 */
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

/* ---------- DeQueue ---------- */
void test_DeQueue(void) {
    ThreadSafeQueue q;
    InitQueue(&q);

    /* 空队列出队 */
    int val = -1;
    int ret = DeQueue(&q, &val);
    TEST("DeQueue 空队列");
    if (ret == -1) {
        PASS();
    } else {
        FAIL("应返回 -1");
    }

    /* 出队一个元素 */
    EnQueue(&q, 99);
    val = -1;
    ret = DeQueue(&q, &val);
    TEST("DeQueue 单个元素");
    if (ret == 0 && val == 99) {
        PASS();
    } else {
        FAIL("值或返回值错误");
    }

    /* 一直出队到空 */
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

/* ---------- QueueSize ---------- */
void test_QueueSize(void) {
    ThreadSafeQueue q;
    InitQueue(&q);

    /* 空队列 */
    TEST("QueueSize 空队列");
    if (QueueSize(&q) == 0) {
        PASS();
    } else {
        FAIL("应为 0");
    }

    /* 入队后 */
    EnQueue(&q, 10);
    EnQueue(&q, 20);
    TEST("QueueSize 入队两个");
    if (QueueSize(&q) == 2) {
        PASS();
    } else {
        FAIL("应为 2");
    }

    /* 出队后 */
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

/* ---------- Contains ---------- */
void test_Contains(void) {
    ThreadSafeQueue q;
    InitQueue(&q);
    EnQueue(&q, 10);
    EnQueue(&q, 20);
    EnQueue(&q, 30);

    /* 找一个存在的值 */
    TEST("Contains 值存在");
    if (Contains(&q, 20) == 1) {
        PASS();
    } else {
        FAIL("应返回 1");
    }

    /* 找一个不存在的值 */
    TEST("Contains 值不存在");
    if (Contains(&q, 99) == 0) {
        PASS();
    } else {
        FAIL("应返回 0");
    }

    /* 空队列找 */
    ThreadSafeQueue empty;
    InitQueue(&empty);
    TEST("Contains 空队列");
    if (Contains(&empty, 10) == 0) {
        PASS();
    } else {
        FAIL("应返回 0");
    }

    DestroyQueue(&q);
    DestroyQueue(&empty);
}

/* ---------- Find ---------- */
void test_Find(void) {
    ThreadSafeQueue q;
    InitQueue(&q);
    EnQueue(&q, 10);
    EnQueue(&q, 20);
    EnQueue(&q, 30);

    /* 找一个存在的值 */
    QueueNode *n = Find(&q, 20);
    TEST("Find 值存在");
    if (n != NULL && n->data == 20) {
        PASS();
    } else {
        FAIL("应找到 20");
    }

    /* 找一个不存在的值 */
    TEST("Find 值不存在");
    if (Find(&q, 99) == NULL) {
        PASS();
    } else {
        FAIL("应返回 NULL");
    }

    /* 空队列找 */
    ThreadSafeQueue empty;
    InitQueue(&empty);
    TEST("Find 空队列");
    if (Find(&empty, 10) == NULL) {
        PASS();
    } else {
        FAIL("应返回 NULL");
    }

    DestroyQueue(&q);
    DestroyQueue(&empty);
}

/* ---------- Print ---------- */
void test_Print(void) {
    ThreadSafeQueue q;
    InitQueue(&q);

    /* 空队列打印，看会不会崩溃 */
    TEST("Print 空队列");
    printf("\n        输出 => ");
    Print(&q);
    printf("         ");
    PASS();

    /* 非空队列打印 */
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

/* ---------- Clear ---------- */
void test_Clear(void) {
    ThreadSafeQueue q;
    InitQueue(&q);
    EnQueue(&q, 1);
    EnQueue(&q, 2);
    EnQueue(&q, 3);

    /* 清空 */
    Clear(&q);
    TEST("Clear 清空");
    if (q.size == 0 && q.head == NULL && q.tail == NULL) {
        PASS();
    } else {
        FAIL("清空后状态不对");
    }

    /* 连续两次 Clear */
    Clear(&q);
    TEST("Clear 连续两次");
    if (q.size == 0) {
        PASS();
    } else {
        FAIL("二次 Clear 后 size != 0");
    }

    DestroyQueue(&q);
}

/* ---------- DestroyQueue ---------- */
void test_DestroyQueue(void) {
    ThreadSafeQueue q;

    /* 情况1：销毁一个空队列 */
    InitQueue(&q);
    TEST("DestroyQueue 空队列");
    DestroyQueue(&q);
    PASS();

    /* 情况2：销毁只有一个元素的队列 */
    InitQueue(&q);
    EnQueue(&q, 42);
    TEST("DestroyQueue 单元素");
    DestroyQueue(&q);
    PASS();

    /* 情况3：销毁有多个元素的队列 */
    InitQueue(&q);
    for (int i = 0; i < 100; i++) {
        EnQueue(&q, i);
    }
    TEST("DestroyQueue 多元素");
    DestroyQueue(&q);
    PASS();
}

/* ---------- NULL 鲁棒性 ---------- */
/* 所有函数传 NULL 进去，不崩溃就算通过 */
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

/* =========================================================
 *  二、多线程并发测试
 *  读线程随机调 Contains/Find/QueueSize/Print
 *  写线程轮流 EnQueue/DeQueue
 *  额外一条 Clear 线程低频清空
 * ========================================================= */

/* 控制线程退出的标志 */
int g_stop = 0;

/*
 * 读线程
 * 不停调用读操作，不修改队列
 */
void *reader_thread(void *arg) {
    ThreadSafeQueue *q = (ThreadSafeQueue *)arg;
    unsigned seed = (unsigned)(uintptr_t)pthread_self();

    while (g_stop == 0) {
        int r = rand_r(&seed);
        unsigned op = (unsigned)r % 100;

        if (op < 33) {
            /* Contains */
            Contains(q, r % 10000);
        } else if (op < 66) {
            /* Find */
            Find(q, r % 10000);
        } else if (op < 98) {
            /* QueueSize */
            QueueSize(q);
        } else {
            /* Print：~2%，低频验证不崩溃 */
            Print(q);
        }
    }
    return NULL;
}

/* 每个写线程循环次数 */
#define WRITER_OPS 500000

/*
 * 写线程
 * 不停 EnQueue 再 DeQueue
 */
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

/*
 * Clear 线程
 * 每次做 EnQueue + DeQueue，每 CLEAR_INTERVAL 次清空队列一次
 */
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

    /* 先预填 100 个元素，让读线程一开始就有东西可读 */
    for (i = 0; i < 100; i++) {
        EnQueue(&q, i);
    }

    printf("\n  [ 读写并发: 3 readers + 2 writers(%d) + 1 clearer ]\n",
           WRITER_OPS);

    /* 启动所有线程 */
    pthread_create(&clearer, NULL, clearer_thread, &q);
    for (i = 0; i < 3; i++) {
        pthread_create(&readers[i], NULL, reader_thread, &q);
    }
    for (i = 0; i < 2; i++) {
        pthread_create(&writers[i], NULL, writer_thread, &q);
    }

    /* 等两个写线程结束 */
    for (i = 0; i < 2; i++) {
        pthread_join(writers[i], NULL);
    }

    /* 通知读线程和 Clear 线程退出 */
    g_stop = 1;

    pthread_join(clearer, NULL);
    for (i = 0; i < 3; i++) {
        pthread_join(readers[i], NULL);
    }

    /* 清理剩下的元素 */
    while (DeQueue(&q, &val) == 0) {
        /* 什么都不做，只是把剩下的节点清掉 */
    }

    TEST("读写并发（不崩溃、不死锁）");
    if (q.size == 0 && q.head == NULL && q.tail == NULL) {
        PASS();
    } else {
        FAIL("最终队列状态不正确");
    }

    DestroyQueue(&q);
}

/* =========================================================
 *  三、压力测试
 *  多个生产者同时入队，多个消费者同时出队
 *  验证最终数据一致
 * ========================================================= */

#define PRODUCERS          4
#define CONSUMERS          4
#define ITEMS_PER_PRODUCER  500000
#define TOTAL_ITEMS        (PRODUCERS * ITEMS_PER_PRODUCER)

/* 统计总入队和总出队次数 */
atomic_long g_total_enq;
atomic_long g_total_deq;

/* 传给生产者线程的参数 */
typedef struct {
    ThreadSafeQueue *q;
    int id;
} ProducerArg;

/* 生产者：一直入队 */
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

/* 消费者：一直出队，直到取够总数 */
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

/*
 * 检查队列的最终状态：
 * 所有数据出队后 head/tail 应该是 NULL，size 应该是 0
 */
int check_final_state(ThreadSafeQueue *q) {
    pthread_mutex_lock(&q->lock);
    int ok = (q->head == NULL && q->tail == NULL && q->size == 0);
    pthread_mutex_unlock(&q->lock);
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

    /* 先启动消费者 */
    for (i = 0; i < CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer_routine, &queue);
    }

    /* 再启动生产者 */
    for (i = 0; i < PRODUCERS; i++) {
        p_args[i].q  = &queue;
        p_args[i].id = i;
        pthread_create(&producers[i], NULL, producer_routine, &p_args[i]);
    }

    /* 等生产者结束 */
    for (i = 0; i < PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }

    /* 等消费者结束 */
    for (i = 0; i < CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    /* 如果队列里还有剩的，全部取出 */
    int val;
    while (DeQueue(&queue, &val) == 0) {
        atomic_fetch_add(&g_total_deq, 1);
    }

    long enq = atomic_load(&g_total_enq);
    long deq = atomic_load(&g_total_deq);
    size_t size = QueueSize(&queue);
    int state_ok = check_final_state(&queue);

    /* 判断是否通过 */
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

/* =========================================================
 *  主函数：按顺序执行所有测试
 * ========================================================= */
int main(void) {
    printf("========================================\n");
    printf("   ThreadSafeQueue 综合测试\n");
    printf("========================================\n\n");

    /* ---- 第一步：单线程功能测试 ---- */
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

    /* ---- 第二步：多线程并发测试 ---- */
    printf("\n--- 多线程并发测试 ---\n");
    test_ConcurrentReadWrite();

    /* ---- 第三步：压力测试 ---- */
    printf("\n--- 压力测试 ---\n");
    test_Stress();

    /* 汇总输出 */
    printf("\n========================================\n");
    printf("   结果: %d 通过, %d 失败  %s\n",
           g_pass, g_fail,
           g_fail > 0 ? "SOME FAILED" : "全部通过");

    /* 详细清单（方便在刷屏之后查看） */
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
