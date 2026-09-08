# Common Macros (xdef)

`libx/xdef.h` provides the global, foundational macro definitions and helper utilities used throughout XSTAR, including common size constants, boolean/null pointer definitions, container and offset calculations, branch prediction hints, static assertions, math helpers, and bit-scanning and atomic inline functions. The header is platform-independent and may be included by any source file.

## Size Constants

Predefined memory size constants in hexadecimal form, convenient for buffer sizes, page alignment, heap/stack sizing, etc.

| Macro | Value | Meaning |
| --- | --- | --- |
| `SZ_16` | `0x00000010` | 16 bytes |
| `SZ_256` | `0x00000100` | 256 bytes |
| `SZ_512` | `0x00000200` | 512 bytes |
| `SZ_1K` | `0x00000400` | 1 KiB |
| `SZ_4K` | `0x00001000` | 4 KiB |
| `SZ_8K` | `0x00002000` | 8 KiB |
| `SZ_16K` | `0x00004000` | 16 KiB |
| `SZ_32K` | `0x00008000` | 32 KiB |
| `SZ_64K` | `0x00010000` | 64 KiB |
| `SZ_128K` | `0x00020000` | 128 KiB |
| `SZ_256K` | `0x00040000` | 256 KiB |
| `SZ_512K` | `0x00080000` | 512 KiB |
| `SZ_1M` | `0x00100000` | 1 MiB |
| `SZ_2M` | `0x00200000` | 2 MiB |
| `SZ_4M` | `0x00400000` | 4 MiB |
| `SZ_8M` | `0x00800000` | 8 MiB |
| `SZ_16M` | `0x01000000` | 16 MiB |
| `SZ_32M` | `0x02000000` | 32 MiB |
| `SZ_64M` | `0x04000000` | 64 MiB |
| `SZ_128M` | `0x08000000` | 128 MiB |
| `SZ_256M` | `0x10000000` | 256 MiB |
| `SZ_512M` | `0x20000000` | 512 MiB |
| `SZ_1G` | `0x40000000` | 1 GiB |
| `SZ_2G` | `0x80000000` | 2 GiB |

Example:

```c
char buf[SZ_4K];
void * heap = xos_mem_malloc(SZ_1M);
```

## Basic Constants

### `NULL`

Null pointer constant. Defined as `0` under C++ and `((void *)0)` under C. Only defined when not already provided by the compiler.

### `TRUE` / `FALSE`

Boolean true/false constants, with values `1` and `0` respectively. XSTAR uses these macros uniformly to represent boolean results, avoiding the `<stdbool.h>` dependency.

```c
if(register_driver(&drv) == FALSE)
    return FALSE;
return TRUE;
```

## Struct and Pointer Operations

### `offsetof(type, member)`

Computes the byte offset of a struct member relative to the start of the struct. Uses the compiler built-in `__builtin_offsetof` under GCC 4 and above, falling back to the classic macro otherwise.

```c
struct foo_t {
    int    a;
    char   b;
    void * c;
};

size_t off = offsetof(struct foo_t, c);
```

### `container_of(ptr, type, member)`

Given the address of a struct member, recovers the address of the containing struct. This Linux-kernel-style macro is the foundation of intrusive data structures in XSTAR, such as `list_head_t`, `hlist_node_t`, and `rb_node_t`.

```c
struct list_head_t * pos;
struct my_node_t * node = container_of(pos, struct my_node_t, list);
```

## Branch Prediction

### `likely(expr)` / `unlikely(expr)`

Provides the compiler with the expected result of a conditional expression to optimize instruction layout. Uses `__builtin_expect` on GCC 3+ and degrades to a plain boolean expression on other compilers.

```c
if(unlikely(ptr == NULL))
    return FALSE;

if(likely(count > 0))
    do_work();
```

> Only use these on performance-sensitive hot paths with a clearly skewed branch direction; avoid them in ordinary code.

## Static Assertion

### `STATIC_ASSERT(cond)`

Compile-time assertion. Emits no code when the condition is true; when false, it defines an array of size `-1` to trigger a compilation error. Commonly used to constrain struct sizes, enum ranges, etc.

```c
STATIC_ASSERT(sizeof(uint32_t) == 4);
STATIC_ASSERT(DEVICE_TYPE_MAX_COUNT < 256);
```

## Math and Numeric Helpers

### `XMAP(x, ia, ib, oa, ob)`

Linearly maps `x` from the input range `[ia, ib]` to the output range `[oa, ob]`. Useful for ADC sampling, brightness/volume normalization, etc.

```c
int duty = XMAP(adc_value, 0, 4095, 0, 100);
```

### `XMIN(a, b)` / `XMAX(a, b)`

Type-safe minimum/maximum macros. Implemented with GCC statement expressions and `typeof`, avoiding argument side effects and producing a compile warning when the operand types disagree.

```c
int m = XMIN(a, b);
int n = XMAX(x + 1, y - 1);
```

### `XCLAMP(v, a, b)`

Clamps `v` to the closed range `[a, b]`. Equivalent to `XMIN(XMAX(a, v), b)`.

```c
int volume = XCLAMP(user_input, 0, 100);
```

### `XFLOOR(x)` / `XROUND(x)` / `XCEIL(x)`

Floating-point to `int` rounding macros for floor, round-half-up, and ceiling respectively. They avoid the `<math.h>` dependency and handle negative numbers symmetrically.

```c
int a = XFLOOR(1.7);   /* 1 */
int b = XROUND(1.5);   /* 2 */
int c = XCEIL(-1.2);   /* -1 */
```

### `XDIV255(x)`

Fast integer division by 255, equivalent to `(x + 1) * 257 >> 16`. Commonly used in alpha blending for 8-bit color channels; faster than a regular division.

```c
uint8_t blended = XDIV255((uint32_t)src * alpha);
```

### `XBYTEMUL(x, a)`

Simultaneously multiplies the two interleaved groups of 8-bit byte channels in a 32-bit integer (such as the R/B and G/A channels of RGBA) by an 8-bit constant `a` and normalizes back to 8-bit. This is the core optimization macro for color multiplication in the graphics subsystem.

## Arrays and Strings

### `ARRAY_SIZE(array)`

Returns the number of elements in a static array. Only valid for true array objects (not pointers).

```c
static const char * names[] = { "a", "b", "c" };
for(int i = 0; i < ARRAY_SIZE(names); i++)
    use(names[i]);
```

### `X(...)`

Converts variadic arguments verbatim into a string literal. Useful for embedding JSON, device-tree fragments, or other multi-line text without extra escaping.

```c
const char * json = X({
    "name": "demo",
    "value": 42
});
```

## Bit-Scanning Inline Functions

All of the following inline functions are implemented with GCC built-ins and have zero overhead.

### `int xffs(int x)`

Returns the position of the lowest set bit in `x` (1-based). Returns `0` when `x` is `0`.

### `int xfls(int x)`

Returns the position of the highest set bit in `x` (1-based). Returns `0` when `x` is `0`. Commonly used to compute base-2 logarithms or round up to the next power of two.

```c
int order = xfls(size - 1);   /* smallest 2^order >= size */
```

### `unsigned long __xffs(unsigned long x)`

Returns the index (0-based) of the lowest set bit in `x`, equivalent to `__builtin_ctzl`. `x` must not be `0`.

### `unsigned long __xfls(unsigned long x)`

Returns the index (0-based) of the highest set bit in `x`, equivalent to `(sizeof(long) * 8) - 1 - __builtin_clzl(x)`. `x` must not be `0`.

```c
unsigned long bit = __xffs(mask);   /* lowest set bit index */
mask &= ~(1UL << bit);              /* clear that bit */
```

## Atomic Inline Functions

32-bit integer atomic operations built on the GCC `__atomic` compiler built-ins. Every function is declared `always_inline`, so there is no function-call overhead even at `-O0`; when the target architecture provides a native instruction, each operation compiles down to a single instruction (x86 `lock` prefix, ARMv7 `ldrex/strex`, AArch64 `ldxr/stxr`, RISC-V `amo`/`lr.w/sc.w`). No runtime library is involved.

The API comes in two tiers: six defaults that uniformly use sequential-consistency (`seq_cst`) semantics, plus four weak-ordering variants named after their memory order (acquire/release/relaxed), reserved strictly for performance-sensitive hot paths.

The operand is `xatomic_t` — a struct wrapping a single `volatile int32_t` (exactly 32 bits). The struct wrapper provides compile-time type isolation: plain `int *` / `volatile int *` pointers cannot silently flow into this API, so the classic bug of mixing plain and atomic accesses on the same variable surfaces at compile time (the same approach as the Linux kernel `atomic_t`). For inspection while debugging, read the `.v` member directly. No 64-bit or sub-word (8/16-bit) widths are provided — on platforms such as RV32 those would degrade into libatomic library calls, breaking the zero-dependency rule.

### `int xatomic_load(const xatomic_t * p)`

Atomically reads `*p`. The parameter is `const`-qualified, so both `const volatile int *` and plain `volatile int *` objects are accepted.

### `void xatomic_store(xatomic_t * p, int v)`

Atomically writes `v` to `*p`.

### `int xatomic_add(xatomic_t * p, int v)`

Atomic add `*p += v`, returning the value **before** the addition. Suitable for multi-threaded counters and reference counting.

### `int xatomic_sub(xatomic_t * p, int v)`

Atomic subtract `*p -= v`, returning the previous value. Symmetric with `xatomic_add`.

### `int xatomic_cas(xatomic_t * p, int o, int n)`

Compare-and-swap: writes `n` and returns 1 when `*p == o`, otherwise returns 0. It does **not** retry on failure; callers drive the retry loop themselves when needed.

### `int xatomic_xchg(xatomic_t * p, int n)`

Unconditional exchange: writes `n` and returns the old value. Maps to a single swap instruction where available (x86 `lock xchg`, RISC-V `amoswap`), well suited to "claim/steal" patterns.

### `int xatomic_load_acquire(const xatomic_t * p)`

Atomic read with acquire semantics: memory accesses after it will not be reordered ahead of it. This is the read half of an acquire/release pair (for example, read a ready flag and then consume the data it publishes); always paired with `xatomic_store_release`. Saves one memory barrier compared to `xatomic_load` on ARM/RISC-V.

### `void xatomic_store_release(xatomic_t * p, int v)`

Atomic write with release semantics: memory accesses before it will not be reordered after it. This is the write half of an acquire/release pair (for example, store the data and then publish a ready flag); always paired with `xatomic_load_acquire`.

### `int xatomic_add_relaxed(xatomic_t * p, int v)`

Relaxed atomic add `*p += v`, returning the value **before** the addition. Guarantees atomicity only, with no ordering constraints with respect to surrounding accesses; suitable for pure statistics counters that have no ordering relationship with the code around them.

### `int xatomic_sub_relaxed(xatomic_t * p, int v)`

Relaxed atomic subtract `*p -= v`, returning the previous value. Symmetric with `xatomic_add_relaxed`.

### Memory Ordering

The six defaults (`load` / `store` / `add` / `sub` / `cas` / `xchg`) use fixed `seq_cst` (sequential consistency) semantics — the hardest ordering to misuse. The failure path of `xatomic_cas` uses `relaxed` as required by C11 (the failure ordering must not be stronger than the success ordering).

A generic memory-ordering parameter is deliberately not exposed, to keep misused `relaxed`/`acquire` from introducing hard-to-reproduce concurrency defects. Instead, four semantically named weak-ordering variants serve the hot paths:

- `xatomic_load_acquire` + `xatomic_store_release`: the acquire/release pair, for spinlock critical-section edges, lock-free ring buffers, and other publish/consume patterns;
- `xatomic_add_relaxed` + `xatomic_sub_relaxed`: pure counting, atomicity only.

Anything finer-grained should still be implemented by invoking the `__atomic` built-ins directly at the implementation layer rather than extending this API further.

### Caveats

- The `volatile` qualifier on the parameters only exists so that both `volatile int` and plain `int` declarations are accepted without diagnostics; it provides **no synchronization semantics**. Atomicity and ordering come entirely from the compiler built-ins.
- Use these only for single-variable scenarios such as shared counters, flags, and lock words; for multi-variable composite state, use the XOS mutex (`xos_mutex_lock` and friends).
- The operand type is uniformly `xatomic_t` instead of a bare `volatile int`: the struct wrapper lets the type system reject plain pointers at compile time, preventing the hidden bug of mixing plain and atomic accesses on the same variable; read the `.v` member when inspecting in a debugger. API parameters and return values stay `int` (matching the Linux kernel `atomic_t` convention; the two are the same type on all supported platforms).

### Examples

```c
static xatomic_t counter;

xatomic_add(&counter, 1);          /* thread-safe increment */
int cur = xatomic_load(&counter);  /* atomic snapshot read */
```

Lock-free maximum tracking with a CAS loop:

```c
static xatomic_t max_val;

void update_max(int v)
{
    int o = xatomic_load(&max_val);
    while(v > o)
    {
        if(xatomic_cas(&max_val, o, v))
            break;
        o = xatomic_load(&max_val);    /* lost the race, retry with the latest */
    }
}
```

Acquire/release pairing (publish/consume ordering):

```c
/* producer: store the payload first, then publish the flag */
xatomic_store_release(&ready, 1);

/* consumer: once the flag is acquired, the payload is guaranteed visible */
if(xatomic_load_acquire(&ready))
    use(payload);
```

## Usage Guidelines

- Source files should bring these definitions in via `#include <xstar.h>` or directly via `#include <libx/xdef.h>`.
- Prefer `XMIN` / `XMAX` / `XCLAMP` over hand-written ternary expressions to avoid side-effect bugs caused by repeated argument evaluation.
- Do not overuse `likely` / `unlikely`; reserve them for hot paths where instruction layout actually matters.
- `STATIC_ASSERT` may be placed at file scope in headers and is equally valid inside functions.
- Together with the XOS abstraction layer and `libx/xtypes.h`, this header forms the minimal foundation required by all XSTAR source code.
