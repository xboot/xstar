# ls

List directory contents.

## Usage

```
ls [-l] [-a] [FILE]...
```

## Options

| Option | Description |
|--------|-------------|
| `-l` | Long format: file type (`d`=directory, `-`=file), permissions, size (bytes), name |
| `-a` | Show all files (including hidden files starting with `.`) |

## Notes

- With no arguments, lists the current directory
- Defaults to multi-column output (auto-adjusts to terminal width)

## Examples

```bash
# List current directory
ls

# Long format listing
ls -l

# List all files
ls -la /kobj/device/
```
