# Simple Argument Parser (sarg)

A lightweight command-line argument parser with zero allocation and no external dependencies. It splits arguments into two kinds — **options** and **positional arguments** — and provides presence checks, value access, typed parsing, validity checks, and iteration.

Core trade-off: **values are only recognized in the attached `=` form** (`--name=val`). A bare option is always a boolean flag that **never consumes** the following token — so in `--verbose pos1 pos2`, `pos1`/`pos2` are always positional arguments. "Unambiguous" is favored over "convenient", yielding parsing rules that are minimal and predictable.

## Command-Line Model

sarg classifies each argv token into one of three kinds:

| Token form | Kind | Notes |
|-----------|------|-------|
| `-x`, `--name`, `--name=val` | option | starts with `-`, second char is `-` or a letter |
| `--` | terminator | all following tokens are forced to positional; itself not counted |
| anything else | positional | including negatives `-1` and a lone `-` |

Notes:

- **Negatives and a lone `-` are positional**: `-1`, `-2`, `-` are not options and can be used directly as positional arguments.
- **A `-` followed by a letter is always an option**: `-file.txt`, `-abc` are treated as a single option (not split); pass as positional with `--`, e.g. `-- -file.txt`.
- **After `--`, nothing is parsed as an option**: lets option-like values (e.g. a filename `-weird`) be passed as positional arguments.

## Values and Matching

**An option value is only recognized in the attached `=` form**: the `=` must be in the same token as the option name.

| Form | Meaning |
|------|---------|
| `--flag` | bare option (boolean flag), no value |
| `--flag=val` | value, the value is `val` |
| `--flag=` | value, an empty string (distinct from "absent") |
| `--eq=a=b` | value, the value is `a=b` (values may contain `=`) |
| `--flag bar` | **no value**; `bar` is a positional argument |

The `name` parameter of every API is given **without `=`**, as the plain option name (e.g. `"--port"`). Matching compares a prefix of length `name` and requires the next character to be `\0` (bare form) or `=` (value form), so `--out` never matches `--output=bar`, `--foo` and `--foobar` do not interfere, and no abbreviation completion is done. This rule is shared by `sarg_has`, `sarg_get`, and `sarg_valid`.

## API

```c
struct sarg_t {
    int argc;
    char ** argv;
    struct {
        int index;     /* iterator cursor: next argv index to scan */
        int literal;   /* iterator state: whether "--" has been seen */
    } iter;
};
```

`argc`/`argv` reference the caller's array directly, without copying. `iter` holds the internal state of the positional-argument iterator; callers do not read or write it. The API is grouped by purpose into five groups.

### Initialization

```c
void sarg_init(struct sarg_t * sarg, int argc, char ** argv);
```

Bind argc/argv and reset the iterator cursor.

### Options

```c
int sarg_has(struct sarg_t * sarg, const char * name);
const char * sarg_get(struct sarg_t * sarg, const char * name, const char * def);
```

- `sarg_has` — whether an option is present. Both bare `--name` and `--name=val` count as present; tokens after `--` are ignored. Returns 1/0. Does plain token-presence matching and does not distinguish options from positional arguments.
- `sarg_get` — return the option value (`--name=val` only), or `def` when missing. When an option repeats, the **last** occurrence wins (reverse scan). The returned pointer refers into argv and must not be freed. For `--flag=` (empty value) it returns `""` (not `def`, not NULL), deliberately preserving the "user wrote `=` but left it empty" signal, so callers can distinguish "absent" from "written but empty".

> Use `sarg_has` for boolean flags and `sarg_get` for valued options — the two are complementary: `has` recognizes both the bare and value forms, `get` recognizes only the value form.

### Positional Arguments

```c
const char * sarg_at(struct sarg_t * sarg, int index);
void sarg_iter_reset(struct sarg_t * sarg);
const char * sarg_iter_next(struct sarg_t * sarg);
```

- `sarg_at` — return the positional argument at the given (0-based) index, skipping every option token. Returns NULL when out of range. Each call rescans from the start of argv, O(index).
- `sarg_iter_reset` / `sarg_iter_next` — iterate positional arguments in order, returning NULL when exhausted; a full pass is O(n). The cursor is shared inside `sarg_t`, so call `reset` to start over; `has`/`get`/`at` are read-only and do not touch the cursor, so they are safe to call inside the loop.

### Validity Check

```c
int sarg_valid(struct sarg_t * sarg, const char ** opts, int min, int max);
```

Checks whether argv conforms to the command's option and positional-argument constraints, returning 1 if valid, 0 otherwise. Returns 0 when `sarg` is NULL.

- **`opts`** — a **NULL-terminated** array of legal option names, matched as in `sarg_has` (see above). May be NULL, equivalent to "no legal options": any option present is invalid. Often inlined as a compound literal: `(const char *[]){ "-f", "-i", NULL }`.
- **`min`/`max`** — bounds on the **positional argument count**. `min` is the lower bound (negative treated as 0), `max` the upper bound (`-1` means unlimited). The whole argv is scanned (including after `--`; `--` itself not counted) to total positional arguments; a count outside `[min, max]` is invalid.
- **No reason detail**: it answers only "valid or not", not which constraint was violated. On failure a caller typically prints usage and exits.

```c
if(!sarg_valid(&sarg, (const char *[]){ "-f", "-i", NULL }, 0, 1))
{
    usage();
    return -1;
}
```

> sarg itself silently ignores unknown options (no schema); `sarg_valid` is an optional validation layer that fills this gap, called by the caller as needed.

### Typed Value Access

```c
int sarg_get_int(struct sarg_t * sarg, const char * name, int def);
unsigned int sarg_get_uint(struct sarg_t * sarg, const char * name, unsigned int def);
long sarg_get_long(struct sarg_t * sarg, const char * name, long def);
unsigned long sarg_get_ulong(struct sarg_t * sarg, const char * name, unsigned long def);
double sarg_get_double(struct sarg_t * sarg, const char * name, double def);
```

Build on `sarg_get` to parse the `--name=val` value into the target type, sparing callers from hand-writing `strtol`/`strtod`. Shared conventions:

| Aspect | Behavior |
|--------|----------|
| Value form | `--name=val` only; a bare `--name` yields `def` (use `sarg_has` for presence) |
| Integer base | auto: `0x` hex, `0` octal, otherwise decimal (`--n=010` is octal 8) |
| Full-string check | must parse entirely; empty, trailing garbage (`12abc`), or non-numeric (`abc`) yield `def` |
| Missing/invalid | always returns `def`; no error channel |
| Unsigned negative | `get_uint`/`get_ulong` reject a leading `-`, yielding `def` |
| double | accepts `1.5`, `1e3`, negatives; invalid yields `def` |

> **Integer overflow is not detected** (atoi-style): on overflow the value is silently truncated or saturated (`get_int` truncated to `int`, `get_long` to `LONG_MAX`/`LONG_MIN`, `get_uint`/`get_ulong` to `UINT_MAX`/`ULONG_MAX`), **not** returned as `def`. For parameters needing strict ranges (ports, IDs, etc.), fetch the raw string with `sarg_get` and call `strtol` yourself, checking `errno`/`endptr`/range.

```c
int port          = sarg_get_int(&sarg, "--port", 8080);
unsigned int sz   = sarg_get_uint(&sarg, "--size", 4096);
long off          = sarg_get_long(&sarg, "--offset", 0);
unsigned long crc = sarg_get_ulong(&sarg, "--crc", 0);
double scale      = sarg_get_double(&sarg, "--scale", 1.0);
```

## Example

```c
struct sarg_t sarg;
sarg_init(&sarg, argc, argv);

/* validate: only -f/-i allowed, at most 1 positional argument */
if(!sarg_valid(&sarg, (const char *[]){ "-f", "-i", NULL }, 0, 1))
{
    usage();
    return -1;
}

/* boolean flag */
if(sarg_has(&sarg, "--verbose"))
    printf("verbose mode\n");

/* values (must use the = form) */
const char * host = sarg_get(&sarg, "--host", "127.0.0.1");
const char * port = sarg_get(&sarg, "--port", "8080");

/* iterate all positional arguments */
sarg_iter_reset(&sarg);
const char * arg;
while((arg = sarg_iter_next(&sarg)) != NULL)
    printf("pos: %s\n", arg);
```

For the command line `prog --verbose --host=0.0.0.0 --port=9000 file1.txt file2.txt`: `--verbose` matches the flag, `--host`/`--port` yield their values, and `file1.txt`/`file2.txt` are positional arguments.

## Limitations

- **No space-separated values**: in `--foo bar`, `bar` is a positional argument; write `--foo=bar`.
- **No attached short value**: `-fbar` is an option named `-fbar`; use `-f=bar` for a short option value.
- **A `-` followed by a letter is one whole option**: `-abc` is not split into `-a -b -c`; pass such values as `--name=-file.txt` or `-- -file.txt`.
- **No abbreviation**: `--out` does not match `--output`.
- **No error reporting**: unknown options are silently ignored; call `sarg_valid` first when validation is wanted.
- **Returned-pointer lifetime**: returned pointers refer into argv, must not be freed, and live as long as argv.
- **Shared iterator cursor**: a single `sarg_t` cannot run two iterations in parallel; call `sarg_iter_reset` to start over.
