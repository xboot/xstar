# Framebuffer (framebuffer)

Framebuffer display.

## Device Type

`DEVICE_TYPE_FRAMEBUFFER`

## Struct

```c
struct framebuffer_t {
    char * name;
    int width, height, pwidth, pheight;
    void (*setbl)(struct framebuffer_t * fb, int brightness);
    int (*getbl)(struct framebuffer_t * fb);
    struct surface_t * (*create)(struct framebuffer_t * fb);
    void (*destroy)(struct framebuffer_t * fb, struct surface_t * s);
    int (*present)(struct framebuffer_t * fb, struct surface_t * s, struct dirtylist_t * l, void (*cb)(void *), void * data);
    void (*wait)(struct framebuffer_t * fb);
    void * priv;
};
```

## Present Contract

`present` unifies synchronous and asynchronous modes. Completion means the source surface is no longer referenced by hardware:

- `cb == NULL`: synchronous mode, blocks until completion and returns 0
- `cb != NULL`: asynchronous hint. Returns 1 if the transfer is in flight, `cb(data)` will be invoked exactly once on completion (interrupt context allowed); returns 0 if it has already completed during the call, `cb` will never be invoked

`cb` is an asynchronous-completion callback, not an unconditional completion notification. A driver must not invoke `cb` when returning 0, which avoids re-entering the caller while it is still in the submission path or holding a lock. A caller that wants the same completion handler in both modes should use this pattern:

```c
if(!framebuffer_present_submit(fb, s, l, present_done, data))
	present_done(data);
```

The caller handles synchronous completion and the driver reports asynchronous completion, so `present_done(data)` executes exactly once in total.

`wait` is required and blocks until the in-flight present completes; drivers without asynchronous support may implement it as an empty function. For page-flip style drivers (DRM, double-buffered LCDC) the completion event is naturally aligned with vertical sync.

## Key API

| Function | Description |
|------|------|
| `search_framebuffer(name)` | Find framebuffer by name |
| `register_framebuffer(fb, drv)` | Register framebuffer |
| `unregister_framebuffer(fb)` | Unregister framebuffer |
| `framebuffer_get/set_backlight(fb, bl)` | Get/set backlight |
| `framebuffer_present_submit(fb, s, l, cb, data)` | Submit a present, blocks synchronously until completion when `cb` is NULL |
| `framebuffer_present_wait(fb)` | Wait for the in-flight present to complete |

## Description

Video framebuffer display interface. Manages pixel buffer, supports surface creation and dirty-rectangle based presentation, with DMA asynchronous refresh support.
