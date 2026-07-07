# rm

Remove files or directories.

## Usage

```
rm [-v] <DIRECTORY> ...
```

## Options

| Option | Description |
|--------|-------------|
| `-v` | Verbose mode, prints `removed '<path>'` on success |

## Notes

- Supports removing multiple files or directories at once
- Operates within the XFS virtual file system

## Examples

```bash
rm -v /tmp/test.txt
rm -v /dir1 /dir2
```
