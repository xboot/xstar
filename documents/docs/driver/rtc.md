# 实时时钟 (rtc)

实时时钟。

## 设备类型

`DEVICE_TYPE_RTC`

## 结构体

```c
struct rtc_time_t {
    int second, minute, hour;
    int week, day, month, year;
};

struct rtc_t {
    char * name;
    int (*settime)(struct rtc_t * rtc, struct rtc_time_t * time);
    int (*gettime)(struct rtc_t * rtc, struct rtc_time_t * time);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_rtc(name)` | 按名称查找 RTC |
| `register_rtc(rtc, drv)` | 注册 RTC |
| `unregister_rtc(rtc)` | 注销 RTC |
| `rtc_settime(rtc, time)` | 设置时间 |
| `rtc_gettime(rtc, time)` | 获取时间 |

## 说明

实时时钟接口。通过电池后备硬件维护跨系统重启的墙钟时间。提供 Unix 纪元秒与结构化 RTC 时间的转换。
