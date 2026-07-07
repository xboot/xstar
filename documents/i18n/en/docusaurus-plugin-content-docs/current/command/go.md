# go

Jump to a specified address and execute a program.

## Usage

```
go address [args ...]
```

## Notes

- Calls the specified memory address as `int func(int argc, char ** argv)`
- Remaining arguments are passed to the called function
- Used to load and run bare-metal programs in RAM

## Examples

```bash
go 0x80200000 arg1 arg2
```
