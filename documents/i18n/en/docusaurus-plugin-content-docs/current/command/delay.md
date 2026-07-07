# delay

Delay for a specified time.

## Usage

```
delay [millisecond]
```

## Notes

- Default delay is 1000ms
- Uses kernel `mdelay()` busy-wait

## Examples

```bash
# Delay for default 1 second
delay

# Delay for 5 seconds
delay 5000
```
