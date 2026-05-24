# ThreadSafeQueue

基于 pthread_rwlock 的 C 语言线程安全队列，支持多读单写并发访问。

## 特性

- 使用读写锁（pthread_rwlock）实现并发安全，读操作可并行
- 基础操作：入队、出队、清空、查找、打印
- 头文件仅需包含 `ts_queue.h`，零依赖

## 使用

```c
#include "ts_queue.h"

ThreadSafeQueue q;
InitQueue(&q);

EnQueue(&q, 42);
EnQueue(&q, 100);

int val;
DeQueue(&q, &val);   // val = 42

printf("%zu\n", QueueSize(&q)); // 1
Contains(&q, 100);             // 1

DestroyQueue(&q);
```

## API

| 函数 | 说明 |
|------|------|
| `InitQueue` | 初始化队列 |
| `EnQueue` | 入队（写锁） |
| `DeQueue` | 出队（写锁） |
| `QueueSize` | 队列长度（读锁） |
| `Contains` | 查找值是否存在（读锁） |
| `Find` | 查找并返回结点指针（读锁） |
| `Clear` | 清空所有元素（写锁） |
| `Print` | 打印队列（读锁） |
| `DestroyQueue` | 销毁队列 |

## 测试

```bash
gcc -o test pressure_test.c -lpthread
./test
```

测试覆盖：单线程功能测试、多线程读写并发测试、多生产者-消费者压力测试（默认 4 生产者 × 50 万，共 200 万次入队）。
