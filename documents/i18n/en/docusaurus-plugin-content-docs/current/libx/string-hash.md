# String Hash (shash)

A string hash function based on the djb2 algorithm, used for hash table key calculation and switch-based string matching, among other scenarios.

## Algorithm

The djb2 algorithm, designed by Daniel J. Bernstein:

```
hash = ((hash << 5) + hash) + c
```

Equivalent to `hash = hash * 33 + c`, with initial value 5381. Simple implementation, uniform distribution, fast computation.

## API

```c
uint32_t shash(const char * s);
```

Compute a 32-bit hash value for a string. Passing NULL returns 5381 (initial value). The function is `static inline`, with zero call overhead.

## Typical Scenario: Switch-Based String Matching

C does not support switching on strings. Using `shash` to convert strings to integers enables this:

```c
switch(shash(name))
{
case 0x41f60f4b: /* "aliceblue" */
    c->r = 0xf0; c->g = 0xf8; c->b = 0xff;
    break;
case 0x0f294442: /* "black" */
    c->r = 0x00; c->g = 0x00; c->b = 0x00;
    break;
case 0x7c94a78d: /* "blue" */
    c->r = 0x00; c->g = 0x00; c->b = 0xff;
    break;
default:
    break;
}
```

No need to worry about hash collisions in this scenario — if two strings produce the same hash value, duplicate `case` labels will cause a compile error, allowing the issue to be caught at compile time. Comments annotate the original strings for readability.

### Generating Case Values

#### Using the shash Command-Line Tool

`shash` is a command-line tool included with the project (source at `developments/shash/`). It computes string hash values directly in the terminal, making it easy to look up values when writing `switch-case` code:

```
$ shash aliceblue black blue
0x41f60f4b: /* "aliceblue" */
0x0f294442: /* "black" */
0x7c94a78d: /* "blue" */
```

The output format matches `case` labels and can be copied directly into your code.

#### Generating in Code

```c
printf("case 0x%08x: /* \"%s\" */\n", shash("aliceblue"), "aliceblue");
/* Output: case 0x41f60f4b: "aliceblue" */
```

## Usage Examples

### Command Dispatch

```c
switch(shash(cmd))
{
case 0x106149d3: /* "start" */
    do_start();
    break;
case 0x7c9e1b4b: /* "stop" */
    do_stop();
    break;
}
```
