# tscal

Graphical touchscreen calibration.

## Usage

```
tscal <framebuffer> <input>
```

## Description

- Interactive graphical touchscreen calibration tool
- Displays 5 target points (4 corners + center), user taps each in sequence
- Computes a 7-parameter affine calibration matrix using least squares
- Applies calibration via `input_ioctl(dev, "touchscreen-set-calibration", ...)`
- Outputs calibration array: `[a0, a1, a2, a3, a4, a5, a6]`
- Automatically restarts on calibration failure (near-singular matrix)
- Press Ctrl-C to exit

## Example

```bash
tscal fb.0 input.0
```
