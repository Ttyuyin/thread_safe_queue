# 线程安全队列（Thread-Safe Queue）

基于单链表的线程安全队列，使用 **读者-写者锁（`pthread_rwlock_t`）** 实现读写并发控制。

相比互斥锁，读写锁允许多个读线程同时访问，只在写操作时互斥，在多读少写的场景下并发性能更高。

## 线程安全原理

### 不加锁会怎样？

以 `说明/list.cpp` 为例，两个线程同时向链表尾部追加结点，**不加锁**：

```
线程A: if (tail == NULL) → true
线程B: if (tail == NULL) → true      ← 同时看到空队列
线程A: head = nodeA, tail = nodeA
线程B: head = nodeB, tail = nodeB    ← nodeA 被覆盖，彻底丢失！
```

DeQueue 同理：
```
线程A: node = head  → 获取结点 X
线程B: node = head  → 同样获取 X
线程A: head = node->next  → 推进
线程B: head = node->next  → 再次推进，跳过了下一个结点
线程A: free(X)
线程B: free(X)  → double-free，崩溃！
```

### 读写锁策略

使用 `pthread_rwlock_t` 代替互斥锁，区分读/写操作，允许多个读线程并发执行：

```
黄金法则：
  写操作 → wrlock → 修改共享数据 → unlock
  读操作 → rdlock → 读取共享数据 → unlock
```

| 操作 | 锁类型 | 流程 |
|------|--------|------|
| EnQueue | **写锁** | `wrlock` → 修改 tail → `unlock` |
| DeQueue | **写锁** | `wrlock` → 修改 head → `unlock` → free |
| Clear | **写锁** | `wrlock` → 摘除整个链表 → `unlock` → 逐个 free |
| Contains | **读锁** | `rdlock` → 遍历 → `unlock` |
| Find | **读锁** | `rdlock` → 遍历查找 → `unlock` |
| Print | **读锁** | `rdlock` → 遍历打印 → `unlock` |
| QueueSize | **读锁** | `rdlock` → 读 size → `unlock` |

### 如何验证线程安全？

测试程序的核心验证：**最终队列状态一致**。

压力测试启动 4 个生产者 + 4 个消费者并发操作百万级数据，结束后验证：

- 总入队数 == 总出队数 == TOTAL_ITEMS
- 队列 size == 0，head/tail == NULL
- 并发读写测试中不崩溃、不死锁

## 文件说明

- `ts_queue.h` / `ts_queue.c` — 线程安全队列实现（读写锁保护）
- `pressure_test.c` — 综合测试：单线程功能测试 + 多线程并发测试 + 压力测试
- `Makefile` — 编译脚本
- `说明/` — 课程设计文档
  - `list.cpp` — 不加锁的链表（对照实验，演示竞态条件）
  - `reqiured.txt` — 课设要求
  - `什么是线程安全的链表-队列-栈.doc` — 课设说明文档

## 编译

```sh
make            # 编译
make run        # 编译并运行全部测试
make clean      # 清理二进制文件
```

或手动编译：

```sh
gcc -std=c11 -O2 -Wall -Wextra -pedantic -pthread pressure_test.c ts_queue.c -o pressure_test
```

## 测试结果

```
========================================
   ThreadSafeQueue 综合测试
========================================

--- 单线程功能测试 ---
  TEST InitQueue                    PASS
  TEST EnQueue 单个元素             PASS
  TEST EnQueue 三个元素 size        PASS
  TEST EnQueue FIFO 顺序            PASS
  TEST DeQueue 空队列               PASS
  TEST DeQueue 单个元素             PASS
  TEST DeQueue 取到空               PASS
  TEST QueueSize 空队列             PASS
  TEST QueueSize 入队两个           PASS
  TEST QueueSize 出队一个           PASS
  TEST Contains 值存在              PASS
  TEST Contains 值不存在            PASS
  TEST Contains 空队列              PASS
  TEST Find 值存在                  PASS
  TEST Find 值不存在                PASS
  TEST Find 空队列                  PASS
  TEST Print 空队列                 PASS
  TEST Print 非空队列 (FIFO)        PASS
  TEST Clear 清空                   PASS
  TEST Clear 连续两次               PASS
  TEST DestroyQueue 空队列          PASS
  TEST DestroyQueue 单元素          PASS
  TEST DestroyQueue 多元素          PASS
  TEST EnQueue(NULL, 0)             PASS
  TEST DeQueue(NULL, NULL)          PASS
  TEST Clear(NULL)                  PASS
  TEST Contains(NULL, 0)            PASS
  TEST Find(NULL, 0)                PASS
  TEST QueueSize(NULL)              PASS
  TEST Print(NULL)                  PASS
  TEST DestroyQueue(NULL)           PASS

--- 多线程并发测试 ---
  [ 读写并发: 3 readers + 2 writers(500000) + 1 clearer ]
  TEST 读写并发（不崩溃、不死锁）    PASS

--- 压力测试 ---
  [ 压力测试: 4 生产者 x 500000 = 2000000 入队, 4 消费者 ]
  TEST 压力测试                      PASS

========================================
   结果: 33 通过, 0 失败  全部通过
========================================
```
