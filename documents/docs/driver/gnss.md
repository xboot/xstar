# 卫星导航 (gnss)

全球导航卫星系统。

## 设备类型

`DEVICE_TYPE_GNSS`

## 结构体

```c
struct gnss_t {
    char * name;
    struct gnss_nmea_t nmea;
    int (*enable)(struct gnss_t * gnss);
    int (*disable)(struct gnss_t * gnss);
    int (*read)(struct gnss_t * gnss);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_gnss(name)` | 按名称查找 GNSS 设备 |
| `register_gnss(gnss, drv)` | 注册 GNSS 设备 |
| `unregister_gnss(gnss)` | 注销 GNSS 设备 |
| `gnss_enable/disable(gnss)` | 使能/禁用 GNSS |
| `gnss_refresh(gnss)` | 刷新位置数据 |

## 说明

全球导航卫星系统（GPS/北斗/GLONASS/伽利略）接收器。解析 NMEA 语句提供位置、速度、航向和 UTC 时间。
