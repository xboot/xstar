# Cron Expression Parser (cron)

Parser and matcher for standard 5-field cron expressions, supporting ranges, steps, lists, and name aliases.

## API

```
*    *    *    *    *
│    │    │    │    │
│    │    │    │    └── day of week (0 - 6, 0 = Sunday)
│    │    │    └─────── month (1 - 12)
│    │    └──────────── day of month (1 - 31)
│    └───────────────── hour (0 - 23)
└────────────────────── minute (0 - 59)
```

```c
struct cron_t {
	uint64_t minute;
	uint32_t hour;
	uint32_t day;
	uint16_t month;
	uint8_t week;
	uint8_t restricted;
};

int cron_parse(struct cron_t * cron, const char * expr);
int cron_match(struct cron_t * cron, int minute, int hour, int day, int month, int week);
```

- `cron_parse` - Parse the expression and fill `cron_t`, returns `1` on success, `0` on failure; on failure the contents of `cron_t` are undefined, rely on the return value
- `cron_match` - Test whether the given time matches, returns `1` on match, `0` otherwise

## Expression Format

The expression consists of 5 fields separated by spaces:

| Field | Range | Description |
| --- | --- | --- |
| Minute | 0-59 | Minute of the hour |
| Hour | 0-23 | Hour, 0-based |
| Day of month | 1-31 | Day of the month, 1-based |
| Month | 1-12 or JAN-DEC | Month, name abbreviations allowed |
| Day of week | 0-6 or SUN-SAT | Day of week, 0=Sunday (7 equals 0) |

Each field accepts:

| Syntax | Description |
| --- | --- |
| `*` | Any value |
| `n` | A single value |
| `a-b` | Range |
| `a-b/s` | Range with step `s` |
| `*/s` | Any value with step `s` |
| `a/s` | From `a` up to the field maximum with step `s`, e.g. `5/15` means 5,20,35,50 |
| `a,b,c` | List, combining any of the above |

Month and day-of-week accept case-insensitive 3-letter abbreviations: `JAN`..`DEC`, `SUN`..`SAT`.

## Day of Month and Day of Week

### `7` Equals `0`

In the day-of-week field, `7` is an alias for `0` (Sunday). Thus `0-7`, `0,7`, and `1-7` all correctly include Sunday.

### Combination Rule

Follows Vixie cron semantics:

- When both **day of month** and **day of week** are explicitly specified (neither is `*`), a match on either wins (**OR**)
- Otherwise both must match (**AND**)

For example, `30 4 1,15 * 5` runs at 4:30 on the 1st and 15th of each month, or every Friday.

## Example

### Basic Usage

```c
struct cron_t cron;
struct wallclock_time_t tm;

if(cron_parse(&cron, "30 3 * * *"))       /* every day at 3:30 */
{
	wallclock_gettime(&tm, NULL);
	if(cron_match(&cron, tm.minute, tm.hour, tm.day, tm.month, tm.week))
	{
		/* run the scheduled job */
	}
}
```

`wallclock_time_t.week` is defined as `(days since 1970-01-01 + 4) % 7`, i.e. `0 = Sunday`, matching the cron standard, so it can be passed to `cron_match` directly. For scheduling, `cron_match` is typically invoked once per minute.

### Common Expressions

| Expression | Meaning |
| --- | --- |
| `*/5 * * * *` | Every 5 minutes |
| `0 * * * *` | Every hour on the hour |
| `30 3 * * *` | Daily at 3:30 |
| `0 9 * * 1` | Monday 9:00 |
| `0 0 1 * *` | 1st of every month at 0:00 |
| `0 0 * * 5` | Friday 0:00 |
| `0 0 * * 0` | Sunday 0:00 (`7` is equivalent) |
| `0 0 1 1 *` | January 1st at 0:00 |
| `30 4 1,15 * 5` | 1st and 15th of each month, or Friday, at 4:30 |
| `*/15 * * * *` | Every 15 minutes |
