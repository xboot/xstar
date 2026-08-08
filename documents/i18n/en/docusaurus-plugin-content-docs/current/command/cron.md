# cron

A cron-expression-based scheduled task tool that runs a shell command at the matching time.

## Usage

```
cron                             - list all jobs
cron add <name> <expr> <cmd>     - add a recurring job
cron oneshot <name> <expr> <cmd> - add a one-shot job
cron remove <name>               - remove a job
```

## Description

- `expr` is a standard 5-field cron expression; since it contains spaces, quote it: `"<minute> <hour> <day> <month> <week>"`
- `cmd` is the shell command to run at the matching time; quote it if it contains spaces
- The timezone is read from the global setting `timezone`, defaulting to `Asia/Shanghai` when unset
- Jobs added with `add` (periodic) run on every match; `oneshot` jobs are removed automatically after firing once
- The job `name` must be unique; adding a duplicate name or an invalid expression fails
- For full expression syntax, see [Cron scheduler](../subsys/cron/cron.md)

### Expression fields

| Field | Range | Description |
| --- | --- | --- |
| Minute | 0-59 | minute |
| Hour | 0-23 | hour |
| Day | 1-31 | day of month |
| Month | 1-12 or JAN-DEC | month, name aliases supported |
| Week | 0-6 or SUN-SAT | day of week, 0=Sunday (7 is equivalent to 0) |

Each field supports `*`, `n`, `a-b`, `*/s`, `a,b,c`, etc. When both day and week are explicitly specified, matching either is enough (OR); otherwise both must match (AND).

## Examples

```bash
# List all jobs
cron

# Run every minute
cron add t1 "* * * * *" "echo hi"

# Run every 5 minutes
cron add t2 "*/5 * * * *" "echo tick"

# Run once at midnight on New Year's Day (one-shot)
cron oneshot newyear "0 0 1 1 *" "echo happy new year"

# Run at 8:30 every Monday
cron add weekly "30 8 * * 1" "echo monday"

# Remove a job
cron remove t1
```
