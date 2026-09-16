# start

Run a shell command in a background thread and return immediately without blocking the current shell.

## Usage

```
start <command> [args...]
```

## Description

- Joins all arguments after `start` with spaces into a single command, and executes it in a new thread via `shell_system()`
- The command runs asynchronously in the background; `start` returns immediately without waiting for it to finish
- Requires thread support on the platform, otherwise it prints `start: thread is not supported on this platform`
- Output of the background command is interleaved with the current shell's output
- Wrap the whole line in double quotes to chain multiple commands with `;` in the background

## Example

```bash
# Delay 1 second in the background without blocking the shell
start delay 1000

# List a directory in the background while doing something else
start ls -l /romdisk

# Wrap the whole line in quotes to chain: delay first, then echo
start "delay 1000; echo done"
```
