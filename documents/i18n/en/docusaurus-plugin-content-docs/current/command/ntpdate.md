# ntpdate

Set date and time via NTP.

## Usage

```
ntpdate [device] [host]
```

## Notes

- Synchronizes system time using the Network Time Protocol
- `device`: Network device name (optional)
- `host`: NTP server address (optional, defaults to `pool.ntp.org`)
- Prints the current UTC time on success
- Displays `Can't sync date and time via ntp.` on failure

## Examples

```bash
# Synchronize with defaults
ntpdate

# Specify device and server
ntpdate net-linux.0 pool.ntp.org
```
