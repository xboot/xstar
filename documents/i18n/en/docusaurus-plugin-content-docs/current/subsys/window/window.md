# Window (window)

The core module of the window subsystem, managing framebuffer and input devices, providing rendering Surface, dirty rectangle management, event queue, and frame presentation, with support for screen rotation and reflection.

## Data Structure

```c
struct window_t {
    struct list_head_t list;        /* Global window list node */
    struct matrix2d_t lmatrix;      /* Local transform matrix */
    struct matrix2d_t gmatrix;      /* Global transform matrix */
    struct framebuffer_t * fb;      /* Framebuffer device */
    struct surface_t * fbsurface;   /* Framebuffer Surface */
    struct surface_t * surface;     /* Rendering Surface */
    struct dirtylist_t * dirtylist; /* Dirty rectangle list */
    struct fifo_t * event;          /* Event FIFO queue */
    struct hmap_t * map;            /* Input device mapping */
    struct {
        struct surface_t * s;       /* Watermark Surface */
        struct region_t r;          /* Watermark region */
    } watermark;
    int gmflag;                     /* Global matrix update flag */
    int dpi;                        /* Screen DPI */
};
```

## Screen Orientation

```c
enum window_orientation_t {
    WINDOW_ORIENTATION_ROTATE_0    = 0,  /* 0 degrees (default) */
    WINDOW_ORIENTATION_ROTATE_90   = 1,  /* 90 degrees counter-clockwise */
    WINDOW_ORIENTATION_ROTATE_180  = 2,  /* 180 degrees counter-clockwise */
    WINDOW_ORIENTATION_ROTATE_270  = 3,  /* 270 degrees counter-clockwise */
    WINDOW_ORIENTATION_FLIP_H      = 4,  /* Horizontal flip */
    WINDOW_ORIENTATION_FLIP_MD     = 5,  /* Main diagonal flip */
    WINDOW_ORIENTATION_FLIP_V      = 6,  /* Vertical flip */
    WINDOW_ORIENTATION_FLIP_AD     = 7,  /* Anti-diagonal flip */
};
```

Rotation is counter-clockwise. When rotated 90 or 270 degrees, the window width and height are swapped relative to the framebuffer dimensions.

## How It Works

### Rendering Flow

1. `window_alloc()` creates the window, binds the framebuffer and input devices, and allocates the rendering Surface
2. Obtain the rendering Surface via `window_get_surface()` and perform graphics drawing on it
3. `window_dirtylist_add()` marks regions that need updating
4. `window_present_commit()` composites dirty regions from the rendering Surface to the framebuffer and calls `framebuffer_present()` to refresh the screen
5. `window_present_clear()` clears the dirty rectangle list, preparing for the next frame

### Event Handling

- Input device drivers push events to the global event queue via `push_event_*()` functions
- `push_event()` looks up the corresponding window based on the event's `device` field and injects the event into the window's FIFO queue
- `window_pump_event()` retrieves events from the window's event queue for the application to process

### Screen Transform

The window implements screen rotation and reflection via the local transform matrix (`lmatrix`) and global transform matrix (`gmatrix`):

- `window_alloc()` initializes the transform matrix based on the `orientation` parameter
- `window_set_matrix()` can modify the local transform matrix at runtime
- The global matrix is automatically applied during rendering, mapping rendering Surface coordinates to framebuffer coordinates

### DPI and Unit Conversion

The window maintains DPI (dots per inch) information, supporting dp to pixel conversion:

```c
int px = window_dp_to_px(w, 16);  /* Convert 16dp to pixels */
```

Conversion formula: `px = max(dpi * dp / 160, 1)`

## API

### Lifecycle

| Function | Description |
|----------|-------------|
| `window_alloc(fb, input, orientation)` | Create a window; `fb` is the framebuffer device name, `input` is the input device name, `orientation` is the screen orientation |
| `window_free(w)` | Destroy the window and free all resources |
| `window_exit(w)` | Request window exit |

### Properties

| Function | Description |
|----------|-------------|
| `window_get_surface(w)` | Get the rendering Surface (inline) |
| `window_get_width(w)` | Get the window width (inline) |
| `window_get_height(w)` | Get the window height (inline) |
| `window_get_pwidth(w)` | Get the physical width in mm (inline) |
| `window_get_pheight(w)` | Get the physical height in mm (inline) |
| `window_get_dpi(w)` | Get the screen DPI (inline) |
| `window_dp_to_px(w, dp)` | Convert dp to pixels (inline) |
| `window_set_backlight(w, brightness)` | Set backlight brightness (inline) |
| `window_get_backlight(w)` | Get backlight brightness (inline) |
| `window_set_watermark(w, buf, len)` | Set watermark image (PNG data) |
| `window_set_matrix(w, m)` | Set the local transform matrix |

### Dirty Rectangle and Presentation

| Function | Description |
|----------|-------------|
| `window_dirtylist_fullscreen(w)` | Mark the entire screen as dirty |
| `window_dirtylist_clear(w)` | Clear the dirty rectangle list |
| `window_dirtylist_add(w, r)` | Add a region to the dirty rectangle list |
| `window_present_clear(w)` | Clear dirty rectangles and clear the rendering Surface |
| `window_present_commit(w)` | Commit dirty regions to the framebuffer and present |

### Events

| Function | Description |
|----------|-------------|
| `window_pump_event(w, e)` | Retrieve an event from the event queue; returns 1 if an event is available, 0 if the queue is empty |
| `push_event(e)` | Inject an event into the corresponding window's event queue |

## Usage Examples

### Basic Window Loop

```c
#include <kernel/window/window.h>

struct window_t * w = window_alloc("fb-linux-sdl.0", "input-linux.0", WINDOW_ORIENTATION_ROTATE_0);

struct event_t e;
while(window_pump_event(w, &e) || 1)
{
    while(window_pump_event(w, &e))
    {
        if(e.type == EVENT_TYPE_KEY_DOWN && e.e.key_down.key == KB_KEY_BACK)
            window_exit(w);
    }

    struct surface_t * s = window_get_surface(w);
    /* Draw content on s ... */

    window_dirtylist_fullscreen(w);
    window_present_commit(w);
}

window_free(w);
```

### Screen Rotation

```c
/* Rotate 90 degrees counter-clockwise */
struct window_t * w = window_alloc("fb.0", "input.0", WINDOW_ORIENTATION_ROTATE_90);
/* window_get_width/height return the rotated dimensions */
```

### Backlight Control

```c
window_set_backlight(w, 500);  /* Set backlight to 50% (range 0-1000) */
int bl = window_get_backlight(w);
```

## Notes

- If the specified framebuffer device does not exist, `fb-dummy` (128x128 virtual framebuffer) is used automatically
- The rendering Surface pixel format is 32-bit premultiplied ARGB
- The dirty rectangle mechanism avoids full-screen refresh, improving rendering efficiency
- `window_present_commit()` uses G2D hardware acceleration (if available) for Surface compositing
- A default XSTAR watermark is set during `window_alloc()`; it can be customized via `window_set_watermark()`
- `window_pump_event()` is non-blocking; returns 0 immediately when no events are available
