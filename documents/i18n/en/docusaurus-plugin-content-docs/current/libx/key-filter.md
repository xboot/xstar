# Key Filter (keyfilter)

Key filter, based on a state machine that converts raw press/release events into high-level key semantics (click, double-click, long press, repeat).

## Architecture

```
feed(raw event) → ring queue → poll(current time) → filtered event
```

Input and processing are decoupled: `keyfilter_feed` enqueues raw events, `keyfilter_poll` consumes events from the queue and produces filtered events based on time determination. The caller must poll `keyfilter_poll` at a fixed frequency to trigger timeout-based events (CLICK, LONGPRESS, REPEAT).

## API

### keyfilter_init

```c
void keyfilter_init(struct keyfilter_t * filter);
```

Initialize the filter with default timeouts: click=200ms, longpress=500ms, repeat=100ms.

### keyfilter_settimeout

```c
void keyfilter_settimeout(struct keyfilter_t * filter, int click, int longpress, int repeat);
```

Set timeout parameters (in milliseconds), with minimum value protection: click >= 20, longpress >= 50, repeat >= 0. Set repeat to 0 to disable repeat events.

### keyfilter_feed

```c
int keyfilter_feed(struct keyfilter_t * filter, ktime_t timestamp, int pressed);
```

Enqueue a raw key event. `pressed` is 1 for press, 0 for release.

Consecutive events with the same `pressed` state are deduplicated (not enqueued) and return 1. Only state transitions (release after press, or press after release) are actually enqueued, preventing repeated events from hardware jitter from filling the queue. Returns 0 only when the queue is full.

### keyfilter_poll

```c
enum keyfilter_type_t keyfilter_poll(struct keyfilter_t * filter, ktime_t now);
```

Consume events from the queue and perform state determination based on `now` (current absolute time), returning the filtered event type. Must be called at high frequency to ensure timely timeout determination.

### keyfilter_clear

```c
void keyfilter_clear(struct keyfilter_t * filter);
```

Reset the filter state and clear the queue.

## Event Types

| Type | Value | Description |
|------|-------|-------------|
| KEYFILTER_TYPE_NONE | 0 | No event |
| KEYFILTER_TYPE_DOWN | 1 | Press |
| KEYFILTER_TYPE_UP | 2 | Release |
| KEYFILTER_TYPE_CLICK | 3 | Click (confirmed after release timeout) |
| KEYFILTER_TYPE_DBLCLICK | 4 | Double-click |
| KEYFILTER_TYPE_LONGPRESS | 5 | Long press |
| KEYFILTER_TYPE_REPEAT | 6 | Long press repeat |

## State Machine

```
            Press             Release            Timeout(click)
  IDLE ──────────→ PRESSED ──────────→ RELEASED ──────────→ IDLE
                      │                                        ↑
                      │ Timeout(longpress)                     │
                      ↓                                        │
                    HELD ──────────────────────────────────────┘
                      ↑               Release
                      │
                      │ Timeout(longpress)         Press
                 REPRESSED ←──────── RELEASED
                      │
                      │ Release
                      ↓
                    IDLE (pending=DBLCLICK)
```

| State | Meaning |
|-------|---------|
| IDLE | Idle, waiting for press |
| PRESSED | Pressed, waiting for release or long press timeout |
| RELEASED | Released, waiting for another press (double-click) or timeout to confirm click |
| REPRESSED | Pressed again within short time, waiting for release to confirm double-click or long press timeout |
| HELD | Long press held, waiting for release or repeat timeout |

## Event Sequence Examples

### Click

```
Action: Press──────Release──────────(200ms timeout)
Event:  DOWN    UP                  CLICK
```

### Double-click

```
Action: Press──Release──Press──Release
Event:  DOWN   UP      DOWN   UP     DBLCLICK
```

Click and double-click are mutually exclusive: CLICK is not generated during a double-click; CLICK waits for the timeout to confirm it is not a double-click before being emitted.

### Long press + repeat

```
Action: Press──────────────(500ms)────(100ms)──(100ms)──Release
Event:  DOWN              LONGPRESS  REPEAT    REPEAT    UP
```

## Usage Example

```c
struct keyfilter_t kf;
keyfilter_init(&kf);
keyfilter_settimeout(&kf, 200, 500, 100);

/* In key callback */
keyfilter_feed(&kf, timestamp, pressed);

/* In periodic polling (recommended >= 50Hz) */
ktime_t now = ktime_get();
enum keyfilter_type_t t = keyfilter_poll(&kf, now);
if(t != KEYFILTER_TYPE_NONE)
    handle_event(t);
```
