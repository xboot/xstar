# Real-Time Clock (rtc)

Real-time clock.

## Device Type

`DEVICE_TYPE_RTC`

## Structure

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

## Key API

| Function | Description |
|------|------|
| `search_rtc(name)` | Find RTC by name |
| `register_rtc(rtc, drv)` | Register an RTC |
| `unregister_rtc(rtc)` | Unregister an RTC |
| `rtc_settime(rtc, time)` | Set time |
| `rtc_gettime(rtc, time)` | Get time |

## Description

Real-time clock interface. Maintains wall-clock time across system reboots via battery-backed hardware. Provides conversion between Unix epoch seconds and structured RTC time.
