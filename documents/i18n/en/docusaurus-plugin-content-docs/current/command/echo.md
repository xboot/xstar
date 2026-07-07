# echo

Output strings to standard output.

## Usage

```
echo [option] [string]...
    -n    do not output the trailing newline
```

## Options

| Option | Description |
|--------|-------------|
| `-n`   | Suppress trailing newline |

## Notes

- Arguments are space-separated, followed by `\r\n`
- The `-n` option suppresses the trailing newline

## Examples

```bash
# Output text
echo Hello World

# Output without newline
echo -n Hello
```
