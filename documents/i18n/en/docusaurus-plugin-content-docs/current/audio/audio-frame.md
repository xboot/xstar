# Audio Frame (audioframe)

The basic unit for transferring audio data.

## Structure

```c
struct audio_frame_t {
    int rate;         /* Sample rate (Hz) */
    int channel;      /* Number of channels */
    int frames;       /* Number of frames (~5ms audio data) */
    float * samples;  /* Interleaved sample data */
};
```

## Helper Functions

| Function | Description |
|------|------|
| `audio_frame_is_valid(af)` | Check if audio frame is valid |
| `audio_frame_total_time(af)` | Get total duration of audio frame (ms) |
| `volume_to_factor(volume)` | Convert volume [0,1000] to factor [-50db,0db] |
| `factor_to_volume(factor)` | Convert factor to volume [0,1000] |

## Notes

The audio frame is the basic unit for transferring data in the audio processing pipeline. `samples` is float-type interleaved data (e.g., stereo is L/R/L/R...). `frames` typically corresponds to approximately 5ms of audio data.
