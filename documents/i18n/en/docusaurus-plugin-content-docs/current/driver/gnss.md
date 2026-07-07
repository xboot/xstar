# GNSS (gnss)

Global navigation satellite system.

## Device Type

`DEVICE_TYPE_GNSS`

## Struct

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

## Key API

| Function | Description |
|------|------|
| `search_gnss(name)` | Find GNSS device by name |
| `register_gnss(gnss, drv)` | Register GNSS device |
| `unregister_gnss(gnss)` | Unregister GNSS device |
| `gnss_enable/disable(gnss)` | Enable/disable GNSS |
| `gnss_refresh(gnss)` | Refresh position data |

## Description

Global navigation satellite system (GPS/Beidou/GLONASS/Galileo) receiver. Parses NMEA sentences to provide position, velocity, heading and UTC time.
