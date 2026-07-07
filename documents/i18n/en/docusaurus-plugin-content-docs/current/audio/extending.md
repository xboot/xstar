# Extending Guide (extending)

This document explains how to write custom components for the XSTAR audio framework, covering **three extension approaches** with complete implementation templates for **effect / source / sink**.

## Three Extension Approaches Comparison

| Approach | When to Use | Registration | Complexity |
|----------|-------------|-------------|------------|
| Callback source | Quick integration of external audio data | No, just `audio_source_alloc_custom()` | ★☆☆ |
| Custom effect | New signal processing algorithm: filter/effect/analysis | Yes, `register_audio_effect()` | ★★★ |
| Custom source/sink | New file format / hardware interface / analysis tool | No, implement `xxx_pdata_t` + hooks | ★★☆ |

---

## Approach 1: Custom Effect Plugin (most common)

### Template Code

```c
#include <kernel/audio/effect.h>

/* Private data structure */
struct myeffect_pdata_t {
    float param1;
    int   param2;
    /* state variables if needed */
};

static void * myeffect_create(void)
{
    struct myeffect_pdata_t * pdat;
    pdat = xos_mem_malloc(sizeof(struct myeffect_pdata_t));
    if(!pdat)
        return NULL;
    pdat->param1 = 1.0f;
    pdat->param2 = 0;
    return pdat;
}

static void myeffect_setup(void * ctx, struct json_value_t * v)
{
    struct myeffect_pdata_t * pdat = (struct myeffect_pdata_t *)ctx;
    if(v && (v->type == JSON_OBJECT))
    {
        for(int i = 0; i < v->u.object.length; i++)
        {
            struct json_value_t * o = v->u.object.values[i].value;
            switch(shash(v->u.object.values[i].name))
            {
            case 0x01234567: /* "param1" — replace with actual shash("param1") */
                if(o && (o->type == JSON_DOUBLE))
                    pdat->param1 = (float)o->u.dbl;
                break;
            /* more params... */
            }
        }
    }
}

static void myeffect_prepare(void * ctx, struct audio_frame_t * input)
{
    /* Optional: pre-allocate buffers based on input->rate/channel/frames */
}

static struct audio_frame_t * myeffect_process(void * ctx, struct audio_frame_t * input)
{
    struct myeffect_pdata_t * pdat = (struct myeffect_pdata_t *)ctx;
    float * pi = input->samples;

    for(int n = 0; n < input->frames; n++)
    {
        for(int c = 0; c < input->channel; c++)
        {
            pi[c] = pi[c] * pdat->param1;   /* Example: amplitude scaling */
        }
        pi += input->channel;
    }
    return input;  /* In-place processing or return new buffer */
}

static void myeffect_destroy(void * ctx)
{
    struct myeffect_pdata_t * pdat = (struct myeffect_pdata_t *)ctx;
    if(pdat)
        xos_mem_free(pdat);
}

static struct audio_effect_t myeffect = {
    .name    = "myeffect",
    .create  = myeffect_create,
    .setup   = myeffect_setup,
    .prepare = myeffect_prepare,
    .process = myeffect_process,
    .destroy = myeffect_destroy,
};

static void myeffect_init(void)   { register_audio_effect(&myeffect); }
static void myeffect_exit(void)   { unregister_audio_effect(&myeffect); }
core_initcall(myeffect_init);
core_exitcall(myeffect_exit);
```

### The 5 Hooks Explained

| Hook | When Called | Required | Responsibility |
|------|-------------|----------|----------------|
| `create` | Filter allocation | Yes | Allocate private data, set defaults |
| `setup` | JSON config/re-config | Yes | Parse JSON to update params (may be called multiple times) |
| `prepare` | Before each `process` | Yes | Pre-allocate output buffer per input format |
| `process` | Per frame | Yes | Core algorithm: process input → return output |
| `destroy` | Filter deallocation | Yes | Free private data |

**Two important conventions**:
1. `setup` may be called externally via `audio_filter_setup()` multiple times to modify parameters
2. `prepare` and `process` come in pairs — `prepare` handles resource pre-check/allocation, `process` does the actual computation

### Getting shash Values

Run the following command from the source tree to get the shash value for any field name:

```bash
# Using the shash tool (located at developments/shash/)
echo -n "myparam" | ../developments/shash/shash
```

Or write a quick C snippet:
```c
#include <stdio.h>
#include "libx/string-hash.h"
int main(void) { printf("0x%08x\n", shash("myparam")); return 0; }
```

---

## Approach 2: Custom Source

### Source Template

```c
#include <kernel/audio/source.h>

struct audio_source_myfmt_pdata_t {
    int    rate;
    int    channel;
    int    pos;
    /* file handle, decoder state, etc. */

    struct audio_frame_t output;
    float * samples;
    int    nsample;
};

static int audio_source_myfmt_seek(struct audio_source_t * s, int offset)
{
    struct audio_source_myfmt_pdata_t * pdat = s->priv;
    /* implement seek logic */
    return pdat->pos;
}

static int audio_source_myfmt_tell(struct audio_source_t * s)
{
    struct audio_source_myfmt_pdata_t * pdat = s->priv;
    return pdat->pos;
}

static int audio_source_myfmt_length(struct audio_source_t * s)
{
    struct audio_source_myfmt_pdata_t * pdat = s->priv;
    return /* total length (frames) */;
}

static struct audio_frame_t * audio_source_myfmt_read(struct audio_source_t * s)
{
    struct audio_source_myfmt_pdata_t * pdat = s->priv;
    int frames = pdat->rate * pdat->channel / 200;  /* ~5ms */

    /* decode ~5ms worth of samples into pdat->samples */

    pdat->output.rate = pdat->rate;
    pdat->output.channel = pdat->channel;
    pdat->output.frames = frames;
    pdat->output.samples = pdat->samples;
    return &pdat->output;
}

static int audio_source_myfmt_ioctl(struct audio_source_t * s, const char * cmd, void * arg)
{
    /* support get/set-volume or custom commands */
    return -1;
}

static void audio_source_myfmt_destroy(struct audio_source_t * s)
{
    struct audio_source_myfmt_pdata_t * pdat = s->priv;
    if(pdat)
    {
        if(pdat->samples)
            xos_mem_free(pdat->samples);
        xos_mem_free(pdat);
    }
}

/* Factory function */
struct audio_source_t * audio_source_alloc_from_myfmt(const char * path)
{
    struct audio_source_myfmt_pdata_t * pdat;
    struct audio_source_t * s;

    pdat = xos_mem_malloc(sizeof(struct audio_source_myfmt_pdata_t));
    if(!pdat)
        return NULL;

    /* Initialization */
    pdat->rate = 48000;
    pdat->channel = 2;
    pdat->pos = 0;
    pdat->nsample = 0;
    pdat->samples = NULL;

    s = audio_source_alloc();
    if(!s)
    {
        xos_mem_free(pdat);
        return NULL;
    }

    s->priv    = pdat;
    s->seek    = audio_source_myfmt_seek;
    s->tell    = audio_source_myfmt_tell;
    s->length  = audio_source_myfmt_length;
    s->read    = audio_source_myfmt_read;
    s->ioctl   = audio_source_myfmt_ioctl;
    s->destroy = audio_source_myfmt_destroy;

    return s;
}
```

### Quick Callback Method (No Registration)

For a simple data source, use `audio_source_alloc_custom()`:

```c
int my_audio_callback(float * samples, int nsample, void * data)
{
    /* fill samples[0..nsample-1] */
    /* return -1 to stop, >= 0 to continue */
    return 0;
}

struct audio_source_t * src = audio_source_alloc_custom(48000, 2, my_audio_callback, NULL);
```

---

## Approach 3: Custom Sink

```c
#include <kernel/audio/sink.h>

struct audio_sink_myfmt_pdata_t {
    /* custom state */
};

static void audio_sink_myfmt_write(struct audio_sink_t * s, struct audio_frame_t * af)
{
    struct audio_sink_myfmt_pdata_t * pdat = s->priv;
    /* process af->samples[0..af->frames*af->channel-1] */
    /* e.g., write to file, feed analyzer, send to hardware, etc. */
}

static int audio_sink_myfmt_ioctl(struct audio_sink_t * s, const char * cmd, void * arg)
{
    switch(shash(cmd))
    {
    case 0x10cbc7b7: /* "audio-sink-set-volume" */
        if(arg) {
            int * p = arg;
            /* set volume */
            return 0;
        }
        break;
    case 0xe04cfa2b: /* "audio-sink-get-volume" */
        if(arg) {
            int * p = arg;
            p[0] = /* current volume */;
            return 0;
        }
        break;
    }
    return -1;
}

static void audio_sink_myfmt_destroy(struct audio_sink_t * s)
{
    struct audio_sink_myfmt_pdata_t * pdat = s->priv;
    if(pdat)
        xos_mem_free(pdat);
}

struct audio_sink_t * audio_sink_alloc_myfmt(void)
{
    struct audio_sink_myfmt_pdata_t * pdat;
    struct audio_sink_t * s;

    pdat = xos_mem_malloc(sizeof(struct audio_sink_myfmt_pdata_t));
    if(!pdat)
        return NULL;

    s = audio_sink_alloc();
    if(!s)
    {
        xos_mem_free(pdat);
        return NULL;
    }

    s->priv    = pdat;
    s->write   = audio_sink_myfmt_write;
    s->ioctl   = audio_sink_myfmt_ioctl;
    s->destroy = audio_sink_myfmt_destroy;

    return s;
}
```

---

## Common Pitfalls & Best Practices

### Memory Management
- Resources allocated in `create` must be freed in `destroy`
- If `prepare` pre-allocates buffers, free them in `destroy`
- If `process` needs to output a new frame (different format from input), allocate in `prepare`, free in `destroy`

### Performance Advice
- Each `process` call handles ~5ms of data (~200 samples/frame@48kHz); avoid heavy per-sample overhead
- Use `shash` instead of string comparison (already standardized in the framework)
- Minimize `prepare` memory allocations (only realloc when nsample increases)

### initcall Timing
- Effect plugins use `core_initcall()` for registration
- If depending on system components (e.g., XFS), use `driver_initcall()` or `postcore_initcall()`
- See the [Architecture Design](../guide/architecture-design) for initcall level reference

### Standard ioctl Commands
Sinks should support these standard commands (same for sources):

| Command | shash Value | Purpose |
|---------|-------------|---------|
| `"audio-sink-get-volume"` | `0xe04cfa2b` | Get volume [0, 1000] |
| `"audio-sink-set-volume"` | `0x10cbc7b7` | Set volume [0, 1000] |

### JSON Config Parsing
- Always check `v->type == JSON_OBJECT` before iterating
- Check type before assigning from each field value
- Set defaults in `create`; `setup` only needs to override

---

## More Resources

- Existing effect implementations are the **best reference**: `effect-iir.c` (complex state), `effect-volume.c` (simple processing), `effect-resample.c` (output format changes)
- [Usage Examples](./usage-examples) show how components compose together
- [Effect Plugins](./effect-plugins) lists all built-in effects with JSON config