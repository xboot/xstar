# 字体系统 (font-system)

XSTAR 字体系统基于 TrueType 字体解析，支持多字体族、多风格安装，提供文本测量与渲染能力。

## 架构

```
font.json → font_install_from_xfs() → 哈希表(按风格分组)
                                       ↓
                     font_lookup() ← 3级回退查找策略
                                       ↓
                 font_text_bound / font_text_render / font_icon_bound / font_icon_render
                                       ↓
                              surface_text / surface_icon
```

## 字体风格

| 风格 | 枚举值 | 说明 |
|------|--------|------|
| FONT_STYLE_REGULAR | 0 | 常规 |
| FONT_STYLE_ITALIC | 1 | 斜体 |
| FONT_STYLE_BOLD | 2 | 粗体 |
| FONT_STYLE_BOLDITALIC | 3 | 粗斜体 |

## 字体配置

字体通过 `/romdisk/assets/fonts/font.json` 配置，启动时由 `do_init_font()` 加载。

```json
{
    "roboto": {
        "regular": "Roboto-Regular.ttf",
        "italic": "Roboto-Italic.ttf",
        "bold": "Roboto-Bold.ttf",
        "bolditalic": "Roboto-BoldItalic.ttf"
    },
    "fallback": {
        "regular": "DroidSansFallback.ttf"
    }
}
```

- 顶层键为**字体族名称**
- 每个族下的键为风格名称（`regular`、`italic`、`bold`、`bolditalic`）
- 值为 TTF 文件名，相对于 `/romdisk/assets/fonts/` 目录
- 一个族不必包含全部4种风格，可只安装需要的

## API

### 字体安装

```c
void font_install_from_xfs(const char * family, enum font_style_t style, struct xfs_context_t * xfs, const char * path);
```

从 XFS 虚拟文件系统安装字体。若该族+风格已存在则跳过。

```c
void font_install_from_buf(const char * family, enum font_style_t style, const void * buf, int len);
```

从内存缓冲区安装字体。调用者负责缓冲区生命周期。

```c
void font_uninstall(const char * family, enum font_style_t style);
```

卸载指定族+风格的字体，释放相关资源。

### 文本测量

```c
int font_text_bound(const char * family, enum font_style_t style, int size, int wrap, const char * str, int * width, int * height);
```

测量文本渲染后的边界尺寸。`size` 为像素高度，`wrap` 为自动换行宽度（0 表示不换行），`str` 为 UTF-8 字符串。返回值写入 `width` 和 `height`。

特殊字符处理：
- `\n` — 换行，pen y 前进一个行高
- `\r` — 回车，pen x 归零
- `\t` — 制表符，对齐到 `size * 2` 的整数倍

### 文本渲染

```c
void font_text_render(const char * family, enum font_style_t style, int size, int x, int y, int wrap, const char * str, void (*cb)(void *, int, int, void *, int, int), void * data);
```

逐字形渲染文本，每个字形渲染后回调 `cb(data, x0, y0, gray_bitmap, width, height)`。`gray_bitmap` 为 8bit 灰度 alpha 图。

### 图标测量

```c
int font_icon_bound(const char * family, int size, uint32_t code, int * width, int * height);
```

测量图标（单个 Unicode 码点）的边界尺寸。使用 `FONT_STYLE_REGULAR` 风格查找。

### 图标渲染

```c
void font_icon_render(const char * family, int size, int x, int y, uint32_t code, void (*cb)(void *, int, int, void *, int, int), void * data);
```

渲染单个图标字形，回调格式与文本渲染相同。

### Surface 层便捷接口

```c
void surface_text(struct surface_t * s, struct region_t * clip, int x, int y, int wrap,
    const char * family, enum font_style_t style, int size, struct color_t * c,
    const char * fmt, ...);
```

在 surface 上渲染文本，支持 printf 风格的格式化字符串。`c` 为颜色（NULL 时默认白色）。

```c
void surface_icon(struct surface_t * s, struct region_t * clip, int x, int y,
    const char * family, int size, uint32_t code, struct color_t * c);
```

在 surface 上渲染图标。

## 字体查找策略

`font_lookup()` 采用 3 级回退策略，确保字符总能找到可用字体：

1. **族名匹配** — `family` 支持逗号分隔的族名列表（如 `"roboto,fallback"`），按顺序在指定风格的哈希表中查找，返回第一个包含该字符的字体
2. **同风格全局搜索** — 若族名均未命中，遍历指定风格下所有字体，返回第一个包含该字符的
3. **跨风格全局搜索** — 若仍未命中，遍历其他所有风格的字体
4. **内置回退** — 以上均未命中时，返回编译时内置的回退字体

这种策略保证了即使目标字体缺少某个字符（如 CJK 字符），也能自动回退到包含该字符的字体。

## 使用示例

### 安装自定义字体

```c
struct xfs_context_t * ctx = xfs_alloc();
font_install_from_xfs("mysans", FONT_STYLE_REGULAR, ctx, "/romdisk/assets/fonts/MySans-Regular.ttf");
font_install_from_xfs("mysans", FONT_STYLE_BOLD, ctx, "/romdisk/assets/fonts/MySans-Bold.ttf");
xfs_free(ctx);
```

### 测量文本尺寸

```c
int w, h;
font_text_bound("roboto", FONT_STYLE_REGULAR, 24, 0, "Hello", &w, &h);
```

### 在 surface 上绘制文本

```c
struct color_t c = { 255, 0, 0, 255 };
surface_text(s, NULL, 10, 10, 0, "roboto,fallback", FONT_STYLE_REGULAR, 24, &c, "你好 World");
```

### 渲染图标

```c
struct color_t c = { 255, 255, 255, 255 };
surface_icon(s, NULL, 10, 10, "material", 32, 0xE001, &c);
```
