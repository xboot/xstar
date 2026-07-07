# 模拟数字转换器 (adc)

模拟数字转换器。

## 设备类型

`DEVICE_TYPE_ADC`

## 结构体

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

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_adc(name)` | 按名称查找 ADC 设备 |
| `register_adc(adc, drv)` | 注册 ADC 设备 |
| `unregister_adc(adc)` | 注销 ADC 设备 |
| `adc_read_raw(adc, channel)` | 读取原始值 |
| `adc_read_voltage(adc, channel)` | 读取电压值（毫伏） |

## 说明

ADC 驱动提供从硬件通道读取模拟电压值的接口。每个 ADC 包含参考电压、分辨率和通道数。`adc_read_voltage()` 内部将原始值转换为毫伏值。
