# 核心类型 (core-types)

## 像素格式

```c
enum vision_type_t {
    VISION_TYPE_GRAY = 0x0110,  /* 灰度 - unsigned char (0~255) */
    VISION_TYPE_RGB  = 0x0311,  /* RGB888 - unsigned char (0~255) */
};
```

类型编码规则：高 8 位为通道数，低 4 位为每通道字节数。

## 图像结构体

```c
struct vision_t {
    enum vision_type_t type;   /* 像素格式 */
    int width;                 /* 宽度 */
    int height;                /* 高度 */
    int npixel;                /* 像素总数 */
    void * datas;              /* 平面数据 */
    size_t ndata;              /* 数据总大小 */
};
```

灰度图为 1 平面，RGB 图为 3 平面。

## 内联函数

| 函数 | 说明 |
|------|------|
| `vision_type_get_bytes(type)` | 获取每通道字节数 |
| `vision_type_get_channels(type)` | 获取通道数 |
| `vision_get_type(v)` | 获取像素格式 |
| `vision_get_width(v)` | 获取宽度 |
| `vision_get_height(v)` | 获取高度 |
| `vision_get_npixel(v)` | 获取像素数 |
| `vision_get_datas(v)` | 获取数据指针 |
| `vision_get_ndata(v)` | 获取数据大小 |

## 生命周期

| 函数 | 说明 |
|------|------|
| `vision_alloc(type, width, height)` | 分配图像 |
| `vision_free(v)` | 释放图像 |
| `vision_clone(v, x, y, w, h)` | 克隆子区域为新图像 |
| `vision_convert(v, type)` | 转换像素格式 |
| `vision_clear(v)` | 清零所有像素 |

## 与 Surface 互操作

| 函数 | 说明 |
|------|------|
| `vision_apply_surface(v, s)` | 将 Surface 像素复制到 Vision |
| `surface_apply_vision(s, v)` | 将 Vision 像素复制到 Surface |
