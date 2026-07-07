# 按键过滤器 (keyfilter)

按键过滤器，基于状态机将原始按下/抬起事件转换为高级按键语义（单击、双击、长按、重复）。

## 架构

```
feed(原始事件) → 环形队列 → poll(当前时间) → 过滤后事件
```

输入与处理解耦：`keyfilter_feed` 将原始事件入队，`keyfilter_poll` 从队列消费并结合时间判定产生过滤后事件。调用者需以固定频率轮询 `keyfilter_poll` 以触发超时类事件（CLICK、LONGPRESS、REPEAT）。

## API

### keyfilter_init

```c
void keyfilter_init(struct keyfilter_t * filter);
```

初始化过滤器，默认超时：click=200ms, longpress=500ms, repeat=100ms。

### keyfilter_settimeout

```c
void keyfilter_settimeout(struct keyfilter_t * filter, int click, int longpress, int repeat);
```

设置超时参数（单位：毫秒），带最小值保护：click >= 20, longpress >= 50, repeat >= 0。repeat 设为 0 禁用重复事件。

### keyfilter_feed

```c
int keyfilter_feed(struct keyfilter_t * filter, ktime_t timestamp, int pressed);
```

将原始按键事件入队。`pressed` 为 1 表示按下，0 表示抬起。

连续相同状态的事件会被去重丢弃（不入队），并返回 1。仅发生状态翻转（按下后抬起、或抬起后按下）的事件才真正入队，避免硬件抖动产生的重复事件占满队列。队列已满时返回 0。

### keyfilter_poll

```c
enum keyfilter_type_t keyfilter_poll(struct keyfilter_t * filter, ktime_t now);
```

从队列消费事件并结合 `now`（当前绝对时间）进行状态判定，返回过滤后的事件类型。需高频调用以保证超时判定的及时性。

### keyfilter_clear

```c
void keyfilter_clear(struct keyfilter_t * filter);
```

重置过滤器状态，清空队列。

## 事件类型

| 类型 | 值 | 说明 |
|------|---|------|
| KEYFILTER_TYPE_NONE | 0 | 无事件 |
| KEYFILTER_TYPE_DOWN | 1 | 按下 |
| KEYFILTER_TYPE_UP | 2 | 抬起 |
| KEYFILTER_TYPE_CLICK | 3 | 单击（抬起后超时确认） |
| KEYFILTER_TYPE_DBLCLICK | 4 | 双击 |
| KEYFILTER_TYPE_LONGPRESS | 5 | 长按 |
| KEYFILTER_TYPE_REPEAT | 6 | 长按重复 |

## 状态机

```
           按下              抬起              超时(click)
  IDLE ──────────→ PRESSED ──────────→ RELEASED ──────────→ IDLE
                      │                                        ↑
                      │ 超时(longpress)                        │
                      ↓                                        │
                    HELD ──────────────────────────────────────┘
                      ↑               抬起
                      │
                      │ 超时(longpress)         按下
                 REPRESSED ←──────── RELEASED
                      │
                      │ 抬起
                      ↓
                    IDLE (pending=DBLCLICK)
```

| 状态 | 含义 |
|------|------|
| IDLE | 空闲，等待按下 |
| PRESSED | 已按下，等待抬起或长按超时 |
| RELEASED | 已抬起，等待再次按下（双击）或超时确认单击 |
| REPRESSED | 短时间内再次按下，等待抬起确认双击或长按超时 |
| HELD | 长按保持中，等待抬起或重复超时 |

## 事件序列示例

### 单击

```
操作: 按下──────抬起──────────(200ms超时)
事件: DOWN    UP              CLICK
```

### 双击

```
操作: 按下──抬起──按下──抬起
事件: DOWN   UP    DOWN   UP  DBLCLICK
```

单击与双击互斥：双击时不会产生 CLICK，CLICK 需等待超时确认不是双击后才发出。

### 长按 + 重复

```
操作: 按下──────────────(500ms)────(100ms)──(100ms)──抬起
事件: DOWN              LONGPRESS  REPEAT    REPEAT    UP
```

## 使用示例

```c
struct keyfilter_t kf;
keyfilter_init(&kf);
keyfilter_settimeout(&kf, 200, 500, 100);

/* 按键回调中 */
keyfilter_feed(&kf, timestamp, pressed);

/* 定时轮询中（建议 >= 50Hz） */
ktime_t now = ktime_get();
enum keyfilter_type_t t = keyfilter_poll(&kf, now);
if(t != KEYFILTER_TYPE_NONE)
    handle_event(t);
```
