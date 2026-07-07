# autoshell

Wait and enter Shell mode.

## Usage

```
autoshell [millisecond]
```

## Description

- Wait for a key press during a countdown. Default timeout is 1000ms
- Displays: `Press any key to enter shell mode:0.000`
- Key press enters interactive Shell mode
- Timeout returns silently, allowing the startup process to continue
- Used in startup scripts to give the user a chance to interrupt boot

## Examples

```bash
# Wait 1 second
autoshell

# Wait 5 seconds
autoshell 5000
```
