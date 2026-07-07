# 摄像头 (camera)

摄像头/图像传感器。

## 设备类型

`DEVICE_TYPE_CAMERA`

## 结构体

```c
struct camera_t {
    char * name;
    int (*start)(struct camera_t * camera);
    int (*stop)(struct camera_t * camera);
    int (*capture)(struct camera_t * camera, struct camera_frame_t * frame);
    int (*ioctl)(struct camera_t * camera, const char * cmd, void * arg);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_camera(name)` | 按名称查找摄像头 |
| `register_camera(camera, drv)` | 注册摄像头 |
| `unregister_camera(camera)` | 注销摄像头 |
| `camera_start/stop(camera)` | 启动/停止采集 |
| `camera_capture(camera, frame)` | 捕获帧 |

## 说明

摄像头接口。以多种像素格式捕获帧，提供增益、曝光、白平衡等图像控制。
