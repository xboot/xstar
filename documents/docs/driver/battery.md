# 电池监控 (battery)

电池/电源监控。

## 设备类型

`DEVICE_TYPE_BATTERY`

## 结构体

```c
struct battery_info_t {
    enum power_supply_type_t supply;
    enum battery_status_t status;
    enum battery_health_t health;
    int design_capacity;
    int design_voltage;
    int voltage, current, temperature;
    int cycle, level;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_battery(name)` | 按名称查找电池设备 |
| `register_battery(battery, drv)` | 注册电池设备 |
| `unregister_battery(battery)` | 注销电池设备 |
| `battery_update(battery, info)` | 更新电池信息 |

## 说明

电池/电源监控驱动。跟踪电压、电流、温度、电量百分比、循环次数和充电状态。
