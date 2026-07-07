# mkdir

Create directories.

## Usage

```
mkdir -v <DIRECTORY> ...
```

## Options

| Option | Description |
|--------|-------------|
| `-v` | Verbose mode, prints `mkdir '<path>'` on success |

## Notes

- Supports creating multiple directories at once
- Directories are created in the XFS virtual file system

## Examples

```bash
# Create a single directory
mkdir -v /mydir

# Create multiple directories
mkdir -v /dir1 /dir2 /dir3
```
