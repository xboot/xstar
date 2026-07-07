# xui

立即模式 (Immediate Mode) GUI 库。提供声明式 UI 编程范式，无需维护窗口和控件状态，每帧通过 API 调用绘制界面。

## 核心概念

立即模式 GUI 的特点：

- **无状态**：UI 代码每帧从头执行，不维护控件树
- **声明式**：`xui_button()` 返回点击状态，无需注册回调
- **命令缓冲**：UI 调用记录到命令列表，批量渲染到 Surface
- **池化管理**：窗口、弹簧动画等保留状态通过 ID 池自动管理

## 典型用法

```c
void ui_frame(struct xui_context_t * ctx)
{
    if(xui_begin_window(ctx, "My Window", &(struct region_t){ 50, 50, 400, 300 }))
    {
        if(xui_button(ctx, "Click Me"))
        {
            /* 按钮被点击 */
        }

        xui_text(ctx, "Hello, XUI!");
        xui_end_window(ctx);
    }
}

/* 主循环 */
xui_loop(ctx, ui_frame);
```

## 创建与销毁

```c
/* 创建上下文 */
struct xui_context_t * xui_context_alloc(fb, input, orientation, data);

/* 设置窗口变换矩阵 */
void xui_set_matrix(ctx, matrix);

/* 加载主题样式（JSON） */
void xui_load_style(ctx, json, len);

/* 加载多语言翻译表（JSON） */
void xui_load_lang(ctx, json, len);

/* 加载图像到 LRU 缓存 */
struct surface_t * xui_load_surface(ctx, path);

/* 主循环，每帧调用用户函数 */
void xui_loop(ctx, func);

/* 销毁上下文 */
void xui_context_free(ctx);
```

## 帧开始/结束

```c
/* 开始一帧（清除命令列表、处理输入） */
void xui_begin(ctx);

/* 结束一帧（提交命令列表到 Surface 并呈现） */
void xui_end(ctx);

/* 退出主循环 */
void xui_exit(ctx);
```

## 容器

| 函数 | 说明 |
|------|------|
| `xui_begin_window_ex(ctx, title, rect, opt)` | 开始窗口，返回是否可见 |
| `xui_end_window(ctx)` | 结束窗口 |
| `xui_begin_panel_ex(ctx, title, opt)` | 开始面板 |
| `xui_end_panel(ctx)` | 结束面板 |
| `xui_begin_popup(ctx, rect)` | 开始弹出框 |
| `xui_end_popup(ctx)` | 结束弹出框 |

### 窗口选项

| 选项 | 说明 |
|------|------|
| `XUI_WINDOW_FULLSCREEN` | 全屏 |
| `XUI_WINDOW_TRANSPARENT` | 透明背景 |
| `XUI_WINDOW_NOTITLE` | 无标题栏 |
| `XUI_WINDOW_NOCLOSE` | 无关闭按钮 |
| `XUI_WINDOW_NORESIZE` | 不可调整大小 |
| `XUI_WINDOW_POPUP` | 弹出模式 |

## 控件

| 函数 | 说明 |
|------|------|
| `xui_button(ctx, label)` | 按钮，点击返回 TRUE |
| `xui_button_ex(ctx, icon, label, opt)` | 按钮（带图标和选项） |
| `xui_checkbox(ctx, label, state)` | 复选框，返回修改后的状态 |
| `xui_toggle(ctx, label, state)` | 开关，返回修改后的状态 |
| `xui_radio(ctx, label, state, index)` | 单选按钮，选中时返回 index |
| `xui_slider(ctx, value, low, high, step)` | 滑块，返回调整后的值 |
| `xui_number(ctx, value, step)` | 数字输入框，返回修改后的值 |
| `xui_textedit(ctx, buf, len)` | 文本编辑框，需要缓冲区和长度 |
| `xui_colorpicker(ctx, color)` | 颜色选择器，返回选择的颜色 |
| `xui_progress(ctx, value, low, high)` | 进度条 |
| `xui_radialbar(ctx, value, low, high)` | 径向进度条 |
| `xui_spinner(ctx)` | 旋转加载指示器 |
| `xui_collapse(ctx, title, opt)` | 折叠面板，返回展开状态 |
| `xui_tree(ctx, name)` | 树节点，返回展开状态 |
| `xui_tabbar(ctx, labels, n)` | 标签栏，返回当前选中索引 |

### 按钮样式选项

| 选项 | 说明 |
|------|------|
| `XUI_BUTTON_PRIMARY/SECONDARY/SUCCESS/INFO/WARNING/DANGER` | 颜色样式 |
| `XUI_BUTTON_ROUNDED` | 圆角 |
| `XUI_BUTTON_OUTLINE` | 边框模式 |

## 标签与文本

| 函数 | 说明 |
|------|------|
| `xui_label(ctx, fmt, ...)` | 静态标签 |
| `xui_text(ctx, fmt, ...)` | 自动换行文本 |
| `xui_text_ex(ctx, opt, fmt, ...)` | 文本（带对齐等选项） |

## 图标与图像

| 函数 | 说明 |
|------|------|
| `xui_icon(ctx, family, code)` | 图标 |
| `xui_image(ctx, s, m)` | 图像显示 |
| `xui_badge(ctx, fmt, ...)` | 徽章 |
| `xui_chart(ctx, values, n)` | 图表 |
| `xui_split(ctx, vertical, fix)` | 分隔条（返回拖动调整后的值） |
| `xui_cursor(ctx, x, y)` | 光标位置 |
| `xui_glass(ctx, x, y, w, h, radius)` | 毛玻璃效果 |

## 布局

| 函数 | 说明 |
|------|------|
| `xui_layout_row(ctx, items, widths, height)` | 设置行布局 |
| `xui_layout_width(ctx, width)` | 设置控件宽度 |
| `xui_layout_height(ctx, height)` | 设置控件高度 |
| `xui_layout_begin_column(ctx)` | 开始列布局 |
| `xui_layout_end_column(ctx)` | 结束列布局 |
| `xui_layout_set_next(ctx, rect, relative)` | 设置下一个控件位置 |
| `xui_layout_next(ctx)` | 获取下一个控件区域 |

## 样式系统

通过 `xui_load_style(ctx, json, len)` 加载 JSON 样式配置，可定制：

| 类别 | 可定制项 |
|------|---------|
| 主题色 | primary/secondary/success/info/warning/danger/cancel 各含 normal/hover/active 三态 |
| 字体 | icon_family, font_family, style, color, size |
| 布局 | width, height, padding, spacing, indent |
| 窗口 | border_radius, border_width, shadow_radius, title_height |
| 面板 | border_radius, border_width, shadow_radius |
| 按钮 | border_radius, border_width, outline_width |
| 输入框 | border_radius, border_width, outline_width |
| 滚动条 | width, radius, color |
| 分隔条 | width |

## Shell 命令

`overview` 命令展示所有 XUI 控件的交互演示。

```
overview
```

## 选项标志

| 标志 | 说明 |
|------|------|
| `XUI_OPT_NOINTERACT` | 禁止交互 |
| `XUI_OPT_NOSCROLL` | 禁止滚动 |
| `XUI_OPT_HOLDFOCUS` | 保持焦点 |
| `XUI_OPT_CLOSED` | 默认关闭 |
| `XUI_OPT_TEXT_LEFT/RIGHT/TOP/BOTTOM/CENTER` | 文本对齐 |
| `XUI_OPT_TEXT_SCROLL` | 文本可滚动 |

## 输入事件

| 枚举 | 说明 |
|------|------|
| `XUI_KEY_UP/DOWN/LEFT/RIGHT` | 方向键 |
| `XUI_KEY_ENTER/BACK/HOME/MENU` | 功能键 |
| `XUI_KEY_VOLUME_UP/DOWN/MUTE` | 音量键 |
| `XUI_MOUSE_LEFT/RIGHT/MIDDLE` | 鼠标键 |

## 命令列表 (Command List)

UI 每帧的绘制命令记录到命令列表，结束后批量渲染。支持的命令类型：

| 命令 | 说明 |
|------|------|
| `xui_draw_line/polyline/curve` | 线 |
| `xui_draw_triangle/rectangle/polygon` | 面 |
| `xui_draw_circle/ellipse/arc` | 圆/弧 |
| `xui_draw_surface/icon/text` | 图像/文本 |
| `xui_draw_ripple/glass/shadow` | 效果 |
| `xui_draw_gradient/checkerboard` | 填充 |

## 线程安全

```c
/* 格式字符串（线程安全） */
const char * xui_format(ctx, fmt, ...);
```

## 多语言

```c
/* 加载翻译表（JSON 格式：{"key": "translation"}） */
xui_load_lang(ctx, json, len);

/* 在 UI 中使用翻译（宏） */
T("hello")  /* 查找翻译，找不到返回原字符串 */
```
