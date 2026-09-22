# Window

The core module of the window subsystem. It manages framebuffers and input devices, and provides a frame loop, damage tracking, an event queue and presentation, with screen rotation and reflection support.

## Data Structure

```c
struct window_t {
    struct list_head_t list;        /* Global window list node */
    struct matrix2d_t lmatrix;      /* Local transform matrix */
    struct matrix2d_t gmatrix;      /* Global transform matrix */
    struct framebuffer_t * fb;      /* Framebuffer device */
    struct surface_t * fbsurface[2];/* Presentation surfaces in framebuffer coordinates, created by fb->create; single buffer mode only uses [0] */
    struct surface_t * surface[2];  /* Software draw surfaces, single buffer mode only uses [0]; acquire returns fbsurface directly while they are NULL */
    struct dirtylist_t * pending;   /* Damage accumulated for the current frame (window local coordinates) */
    struct dirtylist_t * inflight;  /* Damage submitted by the last release */
    struct dirtylist_t * fbspace[2];/* Damage handed to the driver (framebuffer coordinates) */
    struct mutex_t lock;            /* Frame state transition lock */
    struct fifo_t * event;          /* Event FIFO queue */
    struct hmap_t * map;            /* Input device map */
    int bufdbl;                     /* Non-zero when double buffering is enabled */
    int bufidx;                     /* Current draw buffer index */
    int copyright;                  /* Copyright flag */
    int gmflag;                     /* Global matrix type flag */
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
    WINDOW_ORIENTATION_FLIP_H      = 4,  /* Flip across y-axis */
    WINDOW_ORIENTATION_FLIP_MD     = 5,  /* Flip across main diagonal */
    WINDOW_ORIENTATION_FLIP_V      = 6,  /* Flip across x-axis */
    WINDOW_ORIENTATION_FLIP_AD     = 7,  /* Flip across anti-diagonal */
};
```

Rotation is counter-clockwise positive. Odd orientation values (90 or 270 degree rotation, or diagonal flips) swap the window dimensions relative to the framebuffer and use transposed software draw surfaces allocated by `surface_alloc()`; 180 degree and horizontal/vertical flips use full-size software draw surfaces. Every orientation except `ROTATE_0` creates its software draw surfaces up front in `window_alloc()`. For `ROTATE_0`, as long as `window_set_matrix()` has never been called, `surface[]` stays NULL, acquire returns `fbsurface[]` directly, drawing writes into the presentation buffer, flip-capable drivers present with zero copies, and the software surface memory is saved entirely. The first call to `window_set_matrix()` allocates the software draw surfaces and copies the current content in full from `fbsurface`; from then on drawing always goes through the software surfaces regardless of later matrix changes (a non-NULL `surface[0]` means software drawing). The switch is one-way, which avoids repeated full-screen copies when a matrix oscillates between identity and non-identity.

The buffer count is controlled by the Kconfig `Window buffer mode` choice, which defaults to `Single buffer` and sets `CONFIG_XSTAR_WINDOW_BUFFER_MODE` to 0. Selecting `Double buffer` sets it to 1 and keeps a ping-pong pair with copy-back for presenters that complete asynchronously, such as asynchronous DMA or DRM page flip. Synchronous drivers gain no overlap and incur per-frame damage copy-back (one copy while drawing directly into `fbsurface`, or two with software draw surfaces due to the extra framebuffer-surface sync), so they should keep the default single-buffer configuration.

`CONFIG_XSTAR_WINDOW_DIRTY_RECTANGLE_LIMIT` controls the maximum number of damage rectangles retained per frame. Its range is 1 to 32 and its default is 4. Smaller values reduce per-rectangle processing and bus transaction overhead, while larger values reduce overdraw introduced by bounding-box merges.

## How It Works

### Frame Loop

1. `window_frame_acquire()` returns the drawable surface for the frame: ping-pong mode immediately returns the back buffer selected by the previous release, while single-buffer mode waits until the previous submission is no longer referenced by the hardware
2. Draw onto the returned surface
3. `window_frame_damage()` reports damaged regions in window local coordinates; it must be called between acquire and release
4. `window_frame_release()` submits the damage: it first compresses the damage to the limit configured by `CONFIG_XSTAR_WINDOW_DIRTY_RECTANGLE_LIMIT`, then the software-surface path transforms it by the global matrix into the current `fbsurface` (which can overlap the previous frame's DMA). It retires the previous in-flight submission, copies the current damage to the other draw surface, and on the software-surface path also copies the transformed result to the other `fbsurface` (keeping both alternating buffers of flip-based drivers consistent). It then transfers ownership of the damage list, presents the frame, and switches the draw buffer. A synchronously completed submission immediately releases `inflight`

A release without reported damage produces no driver traffic at all, so frame skipping comes for free. The buffer count is decided by the compile-time switch `CONFIG_XSTAR_WINDOW_BUFFER_MODE` (see the "Screen Orientation" section).

### Double-buffer Pipeline

Double buffering allows the CPU/GPU to render the next frame while DMA or the display controller processes the previous frame asynchronously:

1. Render frame N into the current draw surface (for example `surface[0]`, or `fbsurface[0]` when drawing directly)
2. Release copies its damage to the other draw surface, submits the frame's presentation surface asynchronously, and switches `bufidx` to 1
3. The next acquire immediately returns the switched draw surface, so frame N+1 can be rendered while hardware reads the submitted content
4. On the software-surface path, release of frame N+1 first transforms by the global matrix into the current `fbsurface/fbspace` pair and copies the result to the other pair, while frame N DMA can remain active
5. Release of frame N+1 waits for frame N before copy-back, reusing its resources, or starting the next submission

To obtain this overlap, release must receive a non-NULL callback so that the framebuffer driver may submit asynchronously; by contract, `cb == NULL` forces synchronous completion. Copy-back happens before submission, so the standby surface already contains the complete previous frame and only incremental drawing is required.

### Damage Tracking

- `pending` is accumulated by the application within a frame, always in window local coordinates
- Release compresses `pending` to the rectangle limit configured by `CONFIG_XSTAR_WINDOW_DIRTY_RECTANGLE_LIMIT`, which defaults to 4
- `inflight` is atomically swapped with `pending` on release and owned by the submission path; synchronous submissions clear it immediately, while asynchronous submissions reclaim it after waiting in the next retire, wait, or single-buffer acquire
- `fbspace[2]` is paired by index with `fbsurface[2]` and stores damage transformed into framebuffer coordinates; regions are clipped to the `fbsurface` bounds before being added, so drivers may assume they never run out of bounds. While the driver reads the previous pair, the CPU can safely rebuild the current pair

### Event Handling

- Input device drivers push events into the global event queue via `push_event_*()` functions
- `push_event()` looks up the window matching the event's `device` field and injects the event into the window's FIFO queue
- `window_pump_event()` retrieves an event from the window event queue for the application

### Screen Transformation

The window implements rotation and reflection via the local matrix (`lmatrix`) and the global matrix (`gmatrix`):

- `window_alloc()` initializes the transform matrix from the `orientation` parameter
- `window_set_matrix()` sets an additional transform matrix at a frame boundary (global matrix = additional matrix × local matrix). The first call should preferably immediately follow `window_alloc()` and precede the first `window_frame_acquire()`; it must not occur between `window_frame_acquire()` and its matching `window_frame_release()`. Call `window_frame_wait()` first if an asynchronous present is in flight
- On the software-surface path the global matrix is applied automatically during release, mapping damage from window coordinates to framebuffer coordinates; the direct-draw path always has an identity global matrix and needs no transform

### DPI and Unit Conversion

The window maintains DPI (dots per inch) information and supports dp to pixel conversion:

```c
int px = window_dp_to_px(w, 16);  /* convert 16dp to pixels */
```

Formula: `px = max(dpi * dp / 160, 1)`

## API

### Lifecycle

| Function | Description |
|----------|-------------|
| `window_alloc(fb, input, orientation)` | Create a window; `fb` is the framebuffer device name, `input` the input device name, `orientation` the screen orientation |
| `window_free(w)` | Destroy the window and free all resources |
| `window_exit(w)` | Request window exit |

### Frame Loop

| Function | Description |
|----------|-------------|
| `window_frame_acquire(w)` | Get the drawable surface for the frame; returns `fbsurface` while `surface[0]` is NULL (initial `ROTATE_0`), otherwise the software draw surface, then single-buffer mode waits for the previous submission while double-buffer mode immediately returns the current standby draw surface |
| `window_frame_damage(w, r)` | Report a damaged region (window local coordinates); a NULL `r` means the whole window; must be called between acquire and release |
| `window_frame_damage_get(w)` | Read-only access to the damage accumulated for the current frame; valid only until release |
| `window_frame_clear(w)` | Fill the damaged regions of the current draw surface with the default checkerboard pattern |
| `window_frame_release(w, cb, data)` | Submit the damage. With `cb == NULL` it blocks until completion; with `cb != NULL` it returns 1 if the transfer is in flight (`cb(data)` invoked exactly once on completion, interrupt context allowed), or 0 if already completed (no callback). Returns 0 without transferring anything when no damage was reported |
| `window_frame_wait(w)` | Block until the in-flight asynchronous present completes and reclaim its `inflight` state |

### Properties

| Function | Description |
|----------|-------------|
| `window_get_width(w)` | Get the window width (inline) |
| `window_get_height(w)` | Get the window height (inline) |
| `window_get_pwidth(w)` | Get the physical width in millimeters (inline) |
| `window_get_pheight(w)` | Get the physical height in millimeters (inline) |
| `window_get_dpi(w)` | Get the screen DPI (inline) |
| `window_dp_to_px(w, dp)` | Convert dp to pixels (inline) |
| `window_set_backlight(w, brightness)` | Set backlight brightness (inline) |
| `window_get_backlight(w)` | Get backlight brightness (inline) |
| `window_set_matrix(w, m)` | Set an additional transform matrix at a frame boundary; the first call allocates the software draw surfaces (copying the current content in full from `fbsurface`) and permanently leaves the direct-draw zero-copy path |

### Events

| Function | Description |
|----------|-------------|
| `window_pump_event(w, e)` | Retrieve an event from the event queue; returns 1 if an event is available, 0 if the queue is empty |
| `push_event(e)` | Inject an event into the corresponding window's event queue |

## Usage Examples

### Basic Frame Loop

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

    struct surface_t * s = window_frame_acquire(w);
    /* Draw content on s ... */

    window_frame_damage(w, NULL);
    window_frame_release(w, NULL, NULL);
}

window_free(w);
```

### Partial Redraw

```c
struct surface_t * s = window_frame_acquire(w);
/* Redraw only the changed widget */
window_frame_damage(w, &(struct region_t){ 32, 64, 128, 48 });
window_frame_release(w, NULL, NULL);
```

### Asynchronous Submission

```c
static void present_done(void * data)
{
    /* The frame has been presented, trigger the next frame rendering here */
}

if(!window_frame_release(w, present_done, NULL))
	present_done(NULL); /* A return value of 0 means synchronous completion; the driver will not call back */

/* Double-buffer mode can immediately acquire the other surface and render in parallel with presentation */
struct surface_t * next = window_frame_acquire(w);

/* To wait synchronously for an in-flight transfer (e.g. before destroying the window, window_free waits internally) */
window_frame_wait(w);
```

### Integrating a Rendering Framework

A rendering framework's "draw buffer delivered" event is distinct from the framebuffer's "DMA completed" event. In LVGL direct mode, for example, the adapter registers one logical draw buffer but rebinds it at the start of every render to the surface returned by `window_frame_acquire()`. The last flush submits asynchronously with a non-NULL no-op callback and then immediately signals LVGL that flushing is ready; window continues to own the actual DMA lifetime. LVGL can therefore start the next frame, while single-buffer mode still waits correctly in the next acquire.

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

- If the specified framebuffer device does not exist, `fb-dummy` (a virtual 128x128 framebuffer) is used automatically
- The draw surface pixel format is 32-bit premultiplied ARGB
- The damage mechanism avoids full-screen refreshes; merging and optimization are performed automatically inside release
- In the ping-pong mode, the copy-back after each flip guarantees that the surface returned by acquire always holds the complete last frame, so applications only draw the increment
- The built-in watermark is clipped to the current frame's damage, keeping the modified, copied and submitted pixel ranges consistent
- The software-surface path composites into `fbsurface` by the global matrix (rotation, reflection, translation, etc.), preferring G2D hardware acceleration when available; an initial `ROTATE_0` window with no matrix ever set makes acquire return `fbsurface` directly for zero-copy presenting with flip-capable drivers
- `framebuffer_create_surface()` always creates the surface corresponding to the screen, so drivers may assume the returned surface maps onto the display buffer and can be presented directly
- Asynchronous completion means that hardware no longer references the source surface and it is safe to reuse; individual drivers may align completion with DMA completion or vertical sync
- `window_pump_event()` is non-blocking and returns 0 immediately when no event is pending
