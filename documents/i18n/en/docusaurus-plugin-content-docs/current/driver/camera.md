# Camera (camera)

Camera/image sensor.

## Device Type

`DEVICE_TYPE_CAMERA`

## Structure

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

## Key API

| Function | Description |
|------|------|
| `search_camera(name)` | Find a camera by name |
| `register_camera(camera, drv)` | Register a camera |
| `unregister_camera(camera)` | Unregister a camera |
| `camera_start/stop(camera)` | Start/stop capture |
| `camera_capture(camera, frame)` | Capture a frame |

## Description

Camera interface. Captures frames in multiple pixel formats, provides gain, exposure, white balance and other image controls.
