# ADC (adc)

Analog-to-digital converter.

## Device Type

`DEVICE_TYPE_ADC`

## Structure

```c
struct adc_t {
    char * name;
    int vreference;
    int resolution;
    int nchannel;
    int (*read)(struct adc_t * adc, int channel);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_adc(name)` | Find an ADC device by name |
| `register_adc(adc, drv)` | Register an ADC device |
| `unregister_adc(adc)` | Unregister an ADC device |
| `adc_read_raw(adc, channel)` | Read raw value |
| `adc_read_voltage(adc, channel)` | Read voltage value (millivolts) |

## Description

The ADC driver provides an interface for reading analog voltage values from hardware channels. Each ADC includes reference voltage, resolution, and channel count. `adc_read_voltage()` internally converts the raw value to millivolts.
