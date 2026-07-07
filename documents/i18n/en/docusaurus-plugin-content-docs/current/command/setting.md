# setting

Global settings management tool.

## Usage

```
setting                     - Print all key-value pairs
setting set <key> [<value>] - Set or clear a key
setting get <key> [...]     - Get the value of a key
setting clear               - Clear all settings
```

## Notes

- Manages a global key-value store for system configuration persistence
- `set` without a value clears the key
- `clear` resets all settings

## Examples

```bash
# View all settings
setting

# Set a value
setting set wifi.ssid "MyNetwork"

# Get a value
setting get wifi.ssid

# Clear settings
setting clear
```
