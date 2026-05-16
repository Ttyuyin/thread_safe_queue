# 线程安全队列（Thread-Safe Queue）

基于单链表的线程安全队列，使用 `pthread_mutex_t` 实现互斥锁保护。

## 线程安全原理

### 什么是线程安全？

一个数据结构是**线程安全**的，当多个线程同时操作它时，无论操作系统如何调度，数据结构的内部状态始终保持一致，不会出现数据丢失、重复、脏读等异常。

### 不加锁会怎样？

以 `list.cpp` 为例，两个线程同时向链表尾部追加结点，**不加锁**：

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

### 加锁如何解决？

一把 `pthread_mutex_t`，保护所有共享状态（head、tail、size、next 指针）：

```
黄金法则：lock → 访问/修改共享数据 → unlock
```

| 操作 | 流程 |
|------|------|
| EnQueue | `lock` → 修改 tail → `unlock` |
| DeQueue | `lock` → 修改 head → `unlock` → free |
| Clear | `lock` → 摘除整个链表 → `unlock` → 逐个 free |
| Find | `lock` → 遍历 → `unlock` |
| Print | `lock` → 遍历打印 → `unlock` |
| QueueSize | `lock` → 读 size → `unlock` |

### 如何验证线程安全？

测试程序的核心验证公式：

```
final_size == initial_nodes + enq_total - deq_total
```

- `enq_total` = 所有线程成功入队次数之和
- `deq_total` = 所有线程成功出队次数之和

如果锁有缺陷，多线程并发修改 size/head/tail 会导致**计数与实际结点数不一致**：
- 两个线程同时 EnQueue，size 只加 1 → size < 实际结点数
- DeQueue 推进 head 出错，结点留在链表中但 size 已减 → size < 实际结点数

公式不成立 → **线程不安全，锁没保护住临界资源**。

## 验证方法

```
./test_queue              跑全部 3 种场景，自动 PASS/FAIL
./test_queue rr 8 1000 100  跑单场景 + 自定义参数
```

### 测试场景

| 场景 | 操作比例 | 测试目标 |
|------|---------|---------|
| **READ-READ** | 100% Find | 多线程并发遍历不崩溃 |
| **WRITE-WRITE** | 50% EnQueue + 50% DeQueue | 高压写竞争，最容易触发 race |
| **READ-WRITE** | 60% Find + 20% EnQueue + 20% DeQueue | 真实混合负载 |

## 测试结果

### 完整自动化套件

```
========================================
  Thread-Safe Queue  -  Automated Test
  Lock: pthread_mutex_t
========================================

--- Test 1/3: READ-READ ---
[READ-READ]  4 threads x 500000 loops, init=1000
  find=2000000  enq=0  deq=0
  size=1000  expected=1000  ==>  PASS

--- Test 2/3: WRITE-WRITE ---
[WRITE-WRITE]  4 threads x 200000 loops, init=10000
  find=0  enq=399960  deq=400040
  size=9920  expected=9920  ==>  PASS

--- Test 3/3: READ-WRITE ---
[READ-WRITE]  4 threads x 200000 loops, init=10000
  find=480360  enq=159308  deq=160332
  size=8976  expected=8976  ==>  PASS

========================================
  Result: 3/3 passed  >>> ALL TESTS PASSED <<<
========================================
```

## 文件说明

- `ts_queue.h` / `ts_queue.c` — 线程安全队列实现（mutex 保护）
- `test_queue.c` — 自动化多线程压力测试
- `说明/list.cpp` — 不加锁的链表（对照实验，演示竞态条件）
- `说明/reqiured.txt` — 课设要求
- `说明/什么是线程安全的链表-队列-栈.doc` — 课设说明文档
- `Makefile` — 编译脚本

## 编译

```sh
gcc -std=c11 -O2 -Wall -Wextra -pedantic -pthread ts_queue.c test_queue.c -o test_queue
make           # or just 'make'
make run       # run full test suite
make run-ww    # run write-only test only
```
