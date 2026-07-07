# Battery Monitor (battery)

Battery/power monitoring.

## Device Type

`DEVICE_TYPE_BATTERY`

## Structure

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

## Key API

| Function | Description |
|------|------|
| `search_battery(name)` | Find a battery device by name |
| `register_battery(battery, drv)` | Register a battery device |
| `unregister_battery(battery)` | Unregister a battery device |
| `battery_update(battery, info)` | Update battery information |

## Description

Battery/power monitoring driver. Tracks voltage, current, temperature, charge percentage, cycle count, and charging status.
