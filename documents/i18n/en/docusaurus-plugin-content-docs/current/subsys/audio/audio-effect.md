# Audio Effect (audioeffect)

A pluggable audio effect filter chain.

## Effect Structure

```c
struct audio_effect_t {
    struct list_head_t list;
    const char * name;
    void * (*create)(void);
    void (*setup)(void * ctx, struct json_value_t * v);
    void (*prepare)(void * ctx, struct audio_frame_t * input);
    struct audio_frame_t * (*process)(void * ctx, struct audio_frame_t * input);
    void (*destroy)(void * ctx);
};
```

## Filter Chain

```c
struct audio_filter_t {
    struct audio_effect_node_t * nodes;
    int nlen;
};
```

## API

| Function | Description |
|------|------|
| `register_audio_effect(ae)` | Register an audio effect |
| `unregister_audio_effect(ae)` | Unregister an audio effect |
| `audio_filter_alloc(json, len)` | Create a filter chain from JSON config |
| `audio_filter_free(filter)` | Free a filter chain |
| `audio_filter_bypass(filter, id, bypass)` | Set bypass state |
| `audio_filter_setup(filter, id, json, len)` | Reconfigure effect parameters |
| `audio_filter_process(filter, input)` | Process an audio frame |

## Available Effects

| Effect | Description |
|------|------|
| `"volume"` | Volume/gain adjustment |
| `"compressor"` | Dynamic range compressor (soft-knee) |
| `"limiter"` | Peak limiter (output protection) |
| `"panning"` | Stereo balance |
| `"resample"` | Sample rate conversion |
| `"crystalizer"` | Audio enhancement |
| `"iir"` | IIR infinite impulse response filter |
| `"reshape"` | Channel/sample rearrangement |
| `"duplicate"` | Copy frame to private buffer (isolate downstream in-place modifications) |
| `"tremolo"` | Tremolo effect (amplitude modulation) |
| `"mono"` | Mono downmix |

See [Effect Plugins](./effect-plugins) for each effect's JSON syntax, parameter ranges and algorithms.

## JSON Configuration Example

```json
{
    "resample": { "rate": 48000 },
    "reshape":  { "channel": 2 },
    "volume":   { "factor": 0.7 }
}
```

The top level is an object where **keys are effect names** and **values are parameter objects**. Effects are applied serially in JSON declaration order.

> 📌 **Unknown keys are silently skipped**: Unregistered or mistyped effect names (e.g. `"limitter"`, `"echo"`) inside the chain are ignored by `audio_filter_alloc()` without error, and do not break parsing of the remaining effects. Recognized effects are still chained in their original JSON order. If **none** of the names in the object can be recognized, `audio_filter_alloc()` returns `NULL`. This means typos will not crash the system, but they are reported nowhere either — please double-check effect names carefully.

## Key Mechanisms

### shash String Hashing
All JSON field-name parsing uses `shash()` hashes inside `switch-case`, replacing runtime string comparison and balancing performance with readability. When adding a new effect, compute the field-name hash via the `developments/shash/shash` tool or in code.

### Five Lifecycle Hooks
- `create` allocates private data; `destroy` releases it
- `setup` parses JSON params (may be called multiple times)
- `prepare` pre-allocates output buffer before each process
- `process` is the core algorithm that handles one frame

### Bypass Use Cases
`audio_filter_bypass()` temporarily skips a node while preserving its state. A typical pattern: skip resample/reshape when input already matches target params, to reduce CPU usage. For example, [`sink-playback.c:40`](https://github.com/anomalyco/xstar) dynamically toggles bypass based on input frame rate/channel.

## Notes

The filter chain is created from a JSON configuration and supports runtime dynamic parameter adjustment and bypassing of specific nodes. Both Source and Sink have built-in filter support, applied via `audio_source_filter_apply()` and `audio_sink_filter_apply()`. To extend with custom effects, see the [Extending Guide](./extending).
