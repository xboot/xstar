# xui

Immediate Mode GUI library. Provides a declarative UI programming paradigm without the need to maintain window and control state. The interface is drawn each frame through API calls.

## Core Concepts

Characteristics of Immediate Mode GUI:

- **Stateless**: UI code executes from scratch every frame, no control tree is maintained
- **Declarative**: `xui_button()` returns click state, no callback registration needed
- **Command buffer**: UI calls are recorded into a command list and batch-rendered to Surface
- **Pool management**: Retained state such as windows and spring animations are auto-managed through an ID pool

## Typical Usage

```c
void ui_frame(struct xui_context_t * ctx)
{
    if(xui_begin_window(ctx, "My Window", &(struct region_t){ 50, 50, 400, 300 }))
    {
        if(xui_button(ctx, "Click Me"))
        {
            /* Button was clicked */
        }

        xui_text(ctx, "Hello, XUI!");
        xui_end_window(ctx);
    }
}

/* Main loop */
xui_loop(ctx, ui_frame);
```

## Create and Destroy

```c
/* Create context */
struct xui_context_t * xui_context_alloc(fb, input, orientation, data);

/* Set window transform matrix */
void xui_set_matrix(ctx, matrix);

/* Load theme style (JSON) */
void xui_load_style(ctx, json, len);

/* Load language translation table (JSON) */
void xui_load_lang(ctx, json, len);

/* Load image into LRU cache */
struct surface_t * xui_load_surface(ctx, path);

/* Main loop, calls user function each frame */
void xui_loop(ctx, func);

/* Destroy context */
void xui_context_free(ctx);
```

## Frame Begin/End

```c
/* Begin a frame (clear command list, process input) */
void xui_begin(ctx);

/* End a frame (submit command list to Surface and present) */
void xui_end(ctx);

/* Exit main loop */
void xui_exit(ctx);
```

## Containers

| Function | Description |
|----------|-------------|
| `xui_begin_window_ex(ctx, title, rect, opt)` | Begin window, returns whether visible |
| `xui_end_window(ctx)` | End window |
| `xui_begin_panel_ex(ctx, title, opt)` | Begin panel |
| `xui_end_panel(ctx)` | End panel |
| `xui_begin_popup(ctx, rect)` | Begin popup |
| `xui_end_popup(ctx)` | End popup |

### Window Options

| Option | Description |
|--------|-------------|
| `XUI_WINDOW_FULLSCREEN` | Fullscreen |
| `XUI_WINDOW_TRANSPARENT` | Transparent background |
| `XUI_WINDOW_NOTITLE` | No title bar |
| `XUI_WINDOW_NOCLOSE` | No close button |
| `XUI_WINDOW_NORESIZE` | Not resizable |
| `XUI_WINDOW_POPUP` | Popup mode |

## Controls

| Function | Description |
|----------|-------------|
| `xui_button(ctx, label)` | Button, returns TRUE on click |
| `xui_button_ex(ctx, icon, label, opt)` | Button (with icon and options) |
| `xui_checkbox(ctx, label, state)` | Checkbox, returns modified state |
| `xui_toggle(ctx, label, state)` | Toggle, returns modified state |
| `xui_radio(ctx, label, state, index)` | Radio button, returns index when selected |
| `xui_slider(ctx, value, low, high, step)` | Slider, returns adjusted value |
| `xui_number(ctx, value, step)` | Number input, returns modified value |
| `xui_textedit(ctx, buf, len)` | Text edit box, requires buffer and length |
| `xui_colorpicker(ctx, color)` | Color picker, returns selected color |
| `xui_progress(ctx, value, low, high)` | Progress bar |
| `xui_radialbar(ctx, value, low, high)` | Radial progress bar |
| `xui_spinner(ctx)` | Spinning loading indicator |
| `xui_collapse(ctx, title, opt)` | Collapsible panel, returns expanded state |
| `xui_tree(ctx, name)` | Tree node, returns expanded state |
| `xui_tabbar(ctx, labels, n)` | Tab bar, returns currently selected index |

### Button Style Options

| Option | Description |
|--------|-------------|
| `XUI_BUTTON_PRIMARY/SECONDARY/SUCCESS/INFO/WARNING/DANGER` | Color style |
| `XUI_BUTTON_ROUNDED` | Rounded corners |
| `XUI_BUTTON_OUTLINE` | Outline mode |

## Labels and Text

| Function | Description |
|----------|-------------|
| `xui_label(ctx, fmt, ...)` | Static label |
| `xui_text(ctx, fmt, ...)` | Auto-wrapping text |
| `xui_text_ex(ctx, opt, fmt, ...)` | Text (with alignment and other options) |

## Icons and Images

| Function | Description |
|----------|-------------|
| `xui_icon(ctx, family, code)` | Icon |
| `xui_image(ctx, s, m)` | Image display |
| `xui_badge(ctx, fmt, ...)` | Badge |
| `xui_chart(ctx, values, n)` | Chart |
| `xui_split(ctx, vertical, fix)` | Splitter (returns drag-adjusted value) |
| `xui_cursor(ctx, x, y)` | Cursor position |
| `xui_glass(ctx, x, y, w, h, radius)` | Frosted glass effect |

## Layout

| Function | Description |
|----------|-------------|
| `xui_layout_row(ctx, items, widths, height)` | Set row layout |
| `xui_layout_width(ctx, width)` | Set control width |
| `xui_layout_height(ctx, height)` | Set control height |
| `xui_layout_begin_column(ctx)` | Begin column layout |
| `xui_layout_end_column(ctx)` | End column layout |
| `xui_layout_set_next(ctx, rect, relative)` | Set next control position |
| `xui_layout_next(ctx)` | Get next control area |

## Style System

Load JSON style configuration via `xui_load_style(ctx, json, len)`, customizable:

| Category | Customizable Items |
|----------|-------------------|
| Theme colors | primary/secondary/success/info/warning/danger/cancel, each with normal/hover/active states |
| Font | icon_family, font_family, style, color, size |
| Layout | width, height, padding, spacing, indent |
| Window | border_radius, border_width, shadow_radius, title_height |
| Panel | border_radius, border_width, shadow_radius |
| Button | border_radius, border_width, outline_width |
| Input | border_radius, border_width, outline_width |
| Scrollbar | width, radius, color |
| Splitter | width |

## Shell Command

The `overview` command shows an interactive demo of all XUI controls.

```
overview
```

## Option Flags

| Flag | Description |
|------|-------------|
| `XUI_OPT_NOINTERACT` | Disable interaction |
| `XUI_OPT_NOSCROLL` | Disable scrolling |
| `XUI_OPT_HOLDFOCUS` | Hold focus |
| `XUI_OPT_CLOSED` | Closed by default |
| `XUI_OPT_TEXT_LEFT/RIGHT/TOP/BOTTOM/CENTER` | Text alignment |
| `XUI_OPT_TEXT_SCROLL` | Scrollable text |

## Input Events

| Enum | Description |
|------|-------------|
| `XUI_KEY_UP/DOWN/LEFT/RIGHT` | Directional keys |
| `XUI_KEY_ENTER/BACK/HOME/MENU` | Function keys |
| `XUI_KEY_VOLUME_UP/DOWN/MUTE` | Volume keys |
| `XUI_MOUSE_LEFT/RIGHT/MIDDLE` | Mouse buttons |

## Command List

Drawing commands for each UI frame are recorded into a command list, batched and rendered at the end. Supported command types:

| Command | Description |
|---------|-------------|
| `xui_draw_line/polyline/curve` | Lines |
| `xui_draw_triangle/rectangle/polygon` | Shapes |
| `xui_draw_circle/ellipse/arc` | Circles/arcs |
| `xui_draw_surface/icon/text` | Images/text |
| `xui_draw_ripple/glass/shadow` | Effects |
| `xui_draw_gradient/checkerboard` | Fills |

## Thread Safety

```c
/* Format string (thread-safe) */
const char * xui_format(ctx, fmt, ...);
```

## Multi-language

```c
/* Load translation table (JSON format: {"key": "translation"}) */
xui_load_lang(ctx, json, len);

/* Use translation in UI (macro) */
T("hello")  /* Look up translation, return original string if not found */
```
