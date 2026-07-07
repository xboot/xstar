# DAC (dac)

Digital to analog converter.

## Device Type

`DEVICE_TYPE_DAC`

## Struct

```c
struct dac_t {
    char * name;
    int vreference;
    int resolution;
    int nchannel;
    int (*write)(struct dac_t * dac, int channel, int data);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_dac(name)` | Find DAC by name |
| `register_dac(dac, drv)` | Register DAC |
| `unregister_dac(dac)` | Unregister DAC |
| `dac_write_raw(dac, channel, data)` | Write raw value |
| `dac_write_voltage(dac, channel, voltage)` | Write voltage value (millivolts) |

## Description

Digital to analog converter interface. Converts digital values to analog voltage output based on reference voltage and resolution.
