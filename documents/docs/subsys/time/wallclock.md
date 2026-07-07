# 墙上时钟 (wallclock)

系统墙上时钟管理模块，基于时钟源（clocksource）维持时间，支持 RTC 持久化、时区转换和日期时间读写。

## 数据结构

### 时间值

```c
struct wallclock_timeval_t {
    int64_t tv_sec;   /* 自 Unix 纪元起的秒数 */
    int64_t tv_usec;  /* 微秒部分 [0 - 999999] */
};
```

### 日历时间

```c
struct wallclock_time_t {
    uint8_t  second;  /* 秒 [0 - 59] */
    uint8_t  minute;  /* 分 [0 - 59] */
    uint8_t  hour;    /* 时 [0 - 23] */
    uint8_t  week;    /* 星期 [0 - 6] */
    uint8_t  day;     /* 日 [1 - 31] */
    uint8_t  month;   /* 月 [1 - 12] */
    uint32_t year;    /* 年 */
};
```

## 工作原理

### 时间维持

系统使用 `ktime_get()` 获取单调递增的纳秒时间戳，通过 `time_of_day_adjust` 偏移量将其转换为墙上时钟时间。`time_of_day_adjust` 的初始值对应一个固定基准时间，启动时从 RTC 读取时间进行校准。

### RTC 同步

- **启动同步**（`do_init_wallclock`）：系统启动时从 RTC 设备读取时间，若时间有效（年份 >= 2026），则校准 `time_of_day_adjust`
- **运行时回写**（`wallclock_gettimeofday`）：每隔约 10 分钟，将当前墙上时钟时间回写到所有 RTC 设备，保持 RTC 与系统时间同步

### 时区转换

`wallclock_timezone()` 通过字符串哈希（`shash`）匹配时区名称，返回对应的 UTC 偏移秒数。支持全球 300+ 时区，覆盖 UTC-11:00 到 UTC+14:00 的全部主要时区。

## API

| 函数 | 说明 |
|------|------|
| `wallclock_timezone(tz)` | 查询时区名称对应的 UTC 偏移量（秒），如 `"Asia/Shanghai"` 返回 28800 |
| `wallclock_gettimeofday(tv)` | 获取当前墙上时钟时间（Unix 时间戳格式） |
| `wallclock_settimeofday(tv)` | 设置墙上时钟时间，并同步到 RTC 设备 |
| `wallclock_gettime(tm, tz)` | 获取当前时间，按指定时区转换为日历格式 |
| `wallclock_settime(tm, tz)` | 设置当前时间，输入为指定时区的日历格式 |
| `do_init_wallclock()` | 初始化墙上时钟，从 RTC 读取时间校准（系统启动时自动调用） |

## 用法示例

### 获取当前时间

```c
#include <kernel/time/wallclock.h>

/* 获取 Unix 时间戳 */
struct wallclock_timeval_t tv;
wallclock_gettimeofday(&tv);
LOG("epoch: %lld.%06lld\n", tv.tv_sec, tv.tv_usec);

/* 获取上海时区的日历时间 */
struct wallclock_time_t tm;
wallclock_gettime(&tm, "Asia/Shanghai");
LOG("%04u-%02u-%02u %02u:%02u:%02u\n",
    tm.year, tm.month, tm.day, tm.hour, tm.minute, tm.second);
```

### 设置系统时间

```c
/* 通过 Unix 时间戳设置 */
struct wallclock_timeval_t tv = { .tv_sec = 1782432000LL, .tv_usec = 0 };
wallclock_settimeofday(&tv);

/* 通过日历时间设置（上海时区） */
struct wallclock_time_t tm = {
    .year = 2026, .month = 7, .day = 7,
    .hour = 12, .minute = 0, .second = 0,
};
wallclock_settime(&tm, "Asia/Shanghai");
```

### 时区查询

```c
int offset = wallclock_timezone("America/New_York");
LOG("UTC offset: %d seconds (%d hours)\n", offset, offset / 3600);
/* 输出: UTC offset: -18000 seconds (-5 hours) */
```

## 说明

- 墙上时钟基于时钟源（clocksource）维持，即使无 RTC 设备也能正常工作
- 若系统存在 RTC 设备，启动时自动校准时间，运行时定期回写
- `time_of_day_adjust` 的默认基准值为 `362361600000000000` 纳秒（对应 2021-05-01 00:00:00 UTC）
- RTC 时间有效性检查要求 Unix 时间戳 >= `1782432000`（对应 2026-07-01 00:00:00 UTC）
- 时区名称使用 IANA 时区标识符，如 `"Asia/Shanghai"`、`"America/New_York"`、`"Europe/London"`、`"Etc/UTC"` 等
- `do_init_wallclock()` 在 `xstar_init()` 中自动调用，无需手动调用
