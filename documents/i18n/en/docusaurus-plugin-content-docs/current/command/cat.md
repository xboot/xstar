# cat

Display file contents.

## Usage

```
cat <file> ...
```

## Description

- Supports displaying multiple files at once
- Reads and outputs to console in 64KB blocks
- Non-printable characters (except `\r`, `\n`, `\t`, `\f`) are displayed as `.`
- Reports errors for directories or inaccessible files

## Examples

```bash
# View a single file
cat /kobj/class/memory/meminfo

# View multiple files
cat file1.txt file2.txt
```
