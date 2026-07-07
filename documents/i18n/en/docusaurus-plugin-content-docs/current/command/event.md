# event

Display all input events.

## Usage

```
event [framebuffer] [input]
```

## Notes

- Allocates a window and dumps all input events to the console in real time
- Supported event types: `KeyDown`, `KeyUp`, `RotaryTurn`, `MouseDown`, `MouseMove`, `MouseUp`, `MouseWheel`, `TouchBegin`, `TouchMove`, `TouchEnd`, `SystemExit`
- Visualizes touch/mouse traces on screen with blue lines and red dots
- Runs at ~20 FPS, press Ctrl-C to exit

## Examples

```bash
event
event fb.0 input.0
```
