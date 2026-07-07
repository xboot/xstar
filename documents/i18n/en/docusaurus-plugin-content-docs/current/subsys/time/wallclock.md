# Wall Clock (wallclock)

System wall clock management module, maintaining time based on the clock source (clocksource), with RTC persistence, timezone conversion, and date-time read/write support.

## Data Structures

### Time Value

```c
struct wallclock_timeval_t {
    int64_t tv_sec;   /* Seconds since Unix epoch */
    int64_t tv_usec;  /* Microseconds [0 - 999999] */
};
```

### Calendar Time

```c
struct wallclock_time_t {
    uint8_t  second;  /* Second [0 - 59] */
    uint8_t  minute;  /* Minute [0 - 59] */
    uint8_t  hour;    /* Hour [0 - 23] */
    uint8_t  week;    /* Day of week [0 - 6] */
    uint8_t  day;     /* Day [1 - 31] */
    uint8_t  month;   /* Month [1 - 12] */
    uint32_t year;    /* Year */
};
```

## How It Works

### Time Maintenance

The system uses `ktime_get()` to obtain a monotonically increasing nanosecond timestamp, and converts it to wall clock time via the `time_of_day_adjust` offset. The initial value of `time_of_day_adjust` corresponds to a fixed baseline time, which is calibrated from the RTC at startup.

### RTC Synchronization

- **Startup Sync** (`do_init_wallclock`): At system startup, reads time from RTC devices; if the time is valid (year >= 2026), calibrates `time_of_day_adjust`
- **Runtime Writeback** (`wallclock_gettimeofday`): Approximately every 10 minutes, writes the current wall clock time back to all RTC devices, keeping RTC and system time in sync

### Timezone Conversion

`wallclock_timezone()` matches timezone names via string hashing (`shash`), returning the corresponding UTC offset in seconds. It supports 300+ timezones worldwide, covering all major timezones from UTC-11:00 to UTC+14:00.

## API

| Function | Description |
|----------|-------------|
| `wallclock_timezone(tz)` | Query the UTC offset (seconds) for a timezone name, e.g. `"Asia/Shanghai"` returns 28800 |
| `wallclock_gettimeofday(tv)` | Get the current wall clock time (Unix timestamp format) |
| `wallclock_settimeofday(tv)` | Set the wall clock time and sync to RTC devices |
| `wallclock_gettime(tm, tz)` | Get the current time, converted to calendar format for the specified timezone |
| `wallclock_settime(tm, tz)` | Set the current time, input is calendar format for the specified timezone |
| `do_init_wallclock()` | Initialize the wall clock, calibrate from RTC (automatically called at system startup) |

## Usage Examples

### Getting Current Time

```c
#include <kernel/time/wallclock.h>

/* Get Unix timestamp */
struct wallclock_timeval_t tv;
wallclock_gettimeofday(&tv);
LOG("epoch: %lld.%06lld\n", tv.tv_sec, tv.tv_usec);

/* Get calendar time in Shanghai timezone */
struct wallclock_time_t tm;
wallclock_gettime(&tm, "Asia/Shanghai");
LOG("%04u-%02u-%02u %02u:%02u:%02u\n",
    tm.year, tm.month, tm.day, tm.hour, tm.minute, tm.second);
```

### Setting System Time

```c
/* Set via Unix timestamp */
struct wallclock_timeval_t tv = { .tv_sec = 1782432000LL, .tv_usec = 0 };
wallclock_settimeofday(&tv);

/* Set via calendar time (Shanghai timezone) */
struct wallclock_time_t tm = {
    .year = 2026, .month = 6, .day = 26,
    .hour = 12, .minute = 0, .second = 0,
};
wallclock_settime(&tm, "Asia/Shanghai");
```

### Timezone Query

```c
int offset = wallclock_timezone("America/New_York");
LOG("UTC offset: %d seconds (%d hours)\n", offset, offset / 3600);
/* Output: UTC offset: -18000 seconds (-5 hours) */
```

## Notes

- The wall clock is maintained based on the clock source (clocksource); it works even without an RTC device
- If RTC devices are present, the time is automatically calibrated at startup and periodically written back during runtime
- The default baseline value of `time_of_day_adjust` is `362361600000000000` nanoseconds (corresponding to 1981-06-26 00:00:00 UTC)
- RTC time validity check requires Unix timestamp >= `1782432000` (corresponding to 2026-06-26 00:00:00 UTC)
- Timezone names use IANA timezone identifiers, such as `"Asia/Shanghai"`, `"America/New_York"`, `"Europe/London"`, `"Etc/UTC"`, etc.
- `do_init_wallclock()` is automatically called in `xstar_init()`; no manual invocation is needed
