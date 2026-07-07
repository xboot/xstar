# Visual Effect (visual-effect)

Built-in visual effects for Surface.

## API

| Function | Description |
|------|------|
| `surface_effect_glass(s, clip, x, y, w, h, radius)` | Frosted glass blur effect |
| `surface_effect_shadow(s, clip, x, y, w, h, radius, c)` | Drop shadow |
| `surface_effect_gradient(s, clip, x, y, w, h, lt, rt, rb, lb)` | Four-corner bilinear gradient |
| `surface_effect_checkerboard(s, clip, x, y, w, h)` | Checkerboard transparency pattern |

## Description

| Effect | Description |
|------|------|
| **Frosted Glass** | Applies a blur effect to the specified region, simulating frosted glass texture |
| **Drop Shadow** | Draws a blurred colored shadow below the region |
| **Gradient** | Specifies colors at four corners with bilinear interpolation filling the interior |
| **Checkerboard** | Draws a checkerboard pattern to display transparent regions |
