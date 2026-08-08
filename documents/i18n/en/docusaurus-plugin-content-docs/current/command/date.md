# date

Print system date and time.

## Usage

```
date [FORMAT]
```

## Description

- The default timezone is read from the global setting `timezone`, defaulting to `Asia/Shanghai` when unset
- Optional argument specifies a timezone string (e.g. `America/New_York`)
- Output format: `YYYY-MM-DD HH:MM:SS W` (W=weekday, 0=Sunday)

## Examples

```bash
# Default timezone
date

# Specify timezone
date America/New_York
```
