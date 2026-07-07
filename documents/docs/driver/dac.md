# 数字模拟转换器 (dac)

数字模拟转换器。

## 设备类型

`DEVICE_TYPE_DAC`

## 结构体

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

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_dac(name)` | 按名称查找 DAC |
| `register_dac(dac, drv)` | 注册 DAC |
| `unregister_dac(dac)` | 注销 DAC |
| `dac_write_raw(dac, channel, data)` | 写入原始值 |
| `dac_write_voltage(dac, channel, voltage)` | 写入电压值（毫伏） |

## 说明

数字模拟转换器接口。基于参考电压和分辨率将数字值转换为模拟电压输出。
