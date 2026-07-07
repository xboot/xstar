# Event System (event)

The input event abstraction layer for the window subsystem, unifying keyboard, rotary encoder, mouse, touch, joystick, and system events, delivering events from input devices to windows via a FIFO queue.

## Event Types

| Type | Value | Description |
|------|-------|-------------|
| `EVENT_TYPE_KEY_DOWN` | 0x0100 | Key pressed |
| `EVENT_TYPE_KEY_UP` | 0x0101 | Key released |
| `EVENT_TYPE_ROTARY_STEP` | 0x0200 | Rotary encoder step |
| `EVENT_TYPE_MOUSE_DOWN` | 0x0300 | Mouse button pressed |
| `EVENT_TYPE_MOUSE_MOVE` | 0x0301 | Mouse moved |
| `EVENT_TYPE_MOUSE_UP` | 0x0302 | Mouse button released |
| `EVENT_TYPE_MOUSE_WHEEL` | 0x0303 | Mouse wheel scrolled |
| `EVENT_TYPE_TOUCH_BEGIN` | 0x0400 | Touch started |
| `EVENT_TYPE_TOUCH_MOVE` | 0x0401 | Touch moved |
| `EVENT_TYPE_TOUCH_END` | 0x0402 | Touch ended |
| `EVENT_TYPE_JOYSTICK_LEFTSTICK` | 0x0500 | Joystick left stick |
| `EVENT_TYPE_JOYSTICK_RIGHTSTICK` | 0x0501 | Joystick right stick |
| `EVENT_TYPE_JOYSTICK_LEFTTRIGGER` | 0x0502 | Joystick left trigger |
| `EVENT_TYPE_JOYSTICK_RIGHTTRIGGER` | 0x0503 | Joystick right trigger |
| `EVENT_TYPE_JOYSTICK_BUTTONDOWN` | 0x0504 | Joystick button pressed |
| `EVENT_TYPE_JOYSTICK_BUTTONUP` | 0x0505 | Joystick button released |
| `EVENT_TYPE_SYSTEM_EXIT` | 0x1000 | System exit |

## Data Structure

```c
struct event_t {
    void * device;              /* Source device */
    enum event_type_t type;     /* Event type */
    ktime_t timestamp;          /* Event timestamp */

    union {
        struct { uint32_t key; } key_down;          /* Key pressed */
        struct { uint32_t key; } key_up;             /* Key released */
        struct { int32_t delta; } rotary_step;       /* Rotary step */
        struct { int32_t x, y; uint32_t button; } mouse_down;   /* Mouse pressed */
        struct { int32_t x, y; } mouse_move;         /* Mouse moved */
        struct { int32_t x, y; uint32_t button; } mouse_up;     /* Mouse released */
        struct { int32_t dx, dy; } mouse_wheel;      /* Mouse wheel */
        struct { int32_t x, y; uint32_t id; } touch_begin;  /* Touch started */
        struct { int32_t x, y; uint32_t id; } touch_move;   /* Touch moved */
        struct { int32_t x, y; uint32_t id; } touch_end;    /* Touch ended */
        struct { int32_t x, y; } joystick_left_stick;   /* Left stick */
        struct { int32_t x, y; } joystick_right_stick;  /* Right stick */
        struct { int32_t v; } joystick_left_trigger;    /* Left trigger */
        struct { int32_t v; } joystick_right_trigger;   /* Right trigger */
        struct { uint32_t button; } joystick_button_down;  /* Joystick pressed */
        struct { uint32_t button; } joystick_button_up;    /* Joystick released */
    } e;
};
```

## Key Definitions

Key codes are ASCII-compatible (32-127), with extensions for function keys (1-31) and Latin-1 supplement characters (160-255). Common function keys:

| Key | Value | Description |
|-----|-------|-------------|
| `KB_KEY_POWER` | 1 | Power key |
| `KB_KEY_UP` | 2 | Direction up |
| `KB_KEY_DOWN` | 3 | Direction down |
| `KB_KEY_LEFT` | 4 | Direction left |
| `KB_KEY_RIGHT` | 5 | Direction right |
| `KB_KEY_ENTER` | 13 | Enter key |
| `KB_KEY_BACK` | 11 | Back key |
| `KB_KEY_HOME` | 10 | Home key |
| `KB_KEY_MENU` | 12 | Menu key |
| `KB_KEY_DELETE` | 127 | Delete key |

## Mouse Buttons

| Constant | Bit | Description |
|----------|-----|-------------|
| `MOUSE_BUTTON_LEFT` | bit 0 | Left button |
| `MOUSE_BUTTON_RIGHT` | bit 1 | Right button |
| `MOUSE_BUTTON_MIDDLE` | bit 2 | Middle button |
| `MOUSE_BUTTON_X1` | bit 3 | Side button 1 |
| `MOUSE_BUTTON_X2` | bit 4 | Side button 2 |

## Joystick Buttons

Joystick buttons use a bitmask, supporting D-pad, A/B/X/Y, Back/Start/Guide, bumpers, and stick presses:

| Constant | Bit | Description |
|----------|-----|-------------|
| `JOYSTICK_BUTTON_A` | bit 4 | A button |
| `JOYSTICK_BUTTON_B` | bit 5 | B button |
| `JOYSTICK_BUTTON_X` | bit 6 | X button |
| `JOYSTICK_BUTTON_Y` | bit 7 | Y button |
| `JOYSTICK_BUTTON_START` | bit 9 | Start button |
| `JOYSTICK_BUTTON_GUIDE` | bit 10 | Guide button |

## Event Push API

Input device drivers push events to the window system via the following functions:

| Function | Description |
|----------|-------------|
| `push_event_key_down(device, key)` | Push key down event |
| `push_event_key_up(device, key)` | Push key up event |
| `push_event_rotary_step(device, delta)` | Push rotary encoder step event |
| `push_event_mouse_button_down(device, x, y, button)` | Push mouse down event |
| `push_event_mouse_button_up(device, x, y, button)` | Push mouse up event |
| `push_event_mouse_move(device, x, y)` | Push mouse move event |
| `push_event_mouse_wheel(device, dx, dy)` | Push mouse wheel event |
| `push_event_touch_begin(device, x, y, id)` | Push touch begin event |
| `push_event_touch_move(device, x, y, id)` | Push touch move event |
| `push_event_touch_end(device, x, y, id)` | Push touch end event |
| `push_event_joystick_left_stick(device, x, y)` | Push left stick event |
| `push_event_joystick_right_stick(device, x, y)` | Push right stick event |
| `push_event_joystick_left_trigger(device, v)` | Push left trigger event |
| `push_event_joystick_right_trigger(device, v)` | Push right trigger event |
| `push_event_joystick_button_down(device, button)` | Push joystick button down event |
| `push_event_joystick_button_up(device, button)` | Push joystick button up event |

## Event Consumption

Events are retrieved from the window's event FIFO queue via `window_pump_event()`:

```c
struct event_t e;
while(window_pump_event(w, &e))
{
    switch(e.type)
    {
    case EVENT_TYPE_KEY_DOWN:
        LOG("key down: %d\n", e.e.key_down.key);
        break;
    case EVENT_TYPE_TOUCH_BEGIN:
        LOG("touch: (%d, %d) id=%u\n", e.e.touch_begin.x, e.e.touch_begin.y, e.e.touch_begin.id);
        break;
    default:
        break;
    }
}
```

## Notes

- All `push_event_*` functions internally construct an `event_t` structure and call `push_event()` to inject it into the window event queue
- The `device` field identifies the source device, useful for distinguishing multiple input devices of the same type
- Event timestamps are automatically filled via `ktime_get()`
- Mouse and joystick buttons use bitmasks, supporting simultaneous button presses
- Touch events use `id` to distinguish different touch points in multi-touch scenarios
- Joystick coordinates and trigger values are raw ADC values; calibration and normalization are required before use
