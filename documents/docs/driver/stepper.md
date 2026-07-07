# 步进电机 (stepper)

步进电机。

## 设备类型

`DEVICE_TYPE_STEPPER`

## 结构体

```c
struct stepper_t {
    char * name;
    int (*enable)(struct stepper_t * stepper);
    int (*disable)(struct stepper_t * stepper);
    int (*move)(struct stepper_t * stepper, int step, int speed);
    int (*busying)(struct stepper_t * stepper);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_stepper(name)` | 按名称查找步进电机 |
| `register_stepper(stepper, drv)` | 注册步进电机 |
| `unregister_stepper(stepper)` | 注销步进电机 |
| `stepper_enable/disable(stepper)` | 使能/禁用 |
| `stepper_move(stepper, step, speed)` | 移动指定步数 |
| `stepper_busying(stepper)` | 查询是否忙 |

## 驱动实现

### stepper-bipolar-gpio

双极性步进电机 GPIO 驱动，通过 4 个 GPIO（PA、NA、PB、NB）直接控制 H 桥驱动双极性步进电机。支持三种驱动模式：

| 模式 | drive-mode 值 | 说明 |
|------|--------------|------|
| 单波驱动 | `wave` | 每次只励磁一相，4 步一个周期，扭矩较小 |
| 整步驱动 | `fullstep` | 每次励磁两相，4 步一个周期，扭矩较大 |
| 半步驱动 | `halfstep` | 交替单相/双相励磁，8 步一个周期，分辨率提高一倍 |

设备树配置属性：

| 属性 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `drive-mode` | string | 否 | 驱动模式：`wave`、`fullstep`、`halfstep`，默认 `wave` |
| `pa-gpio` | int | 是 | A 相正极 GPIO |
| `pa-gpio-config` | int | 否 | A 相正极 GPIO 配置 |
| `na-gpio` | int | 是 | A 相负极 GPIO |
| `na-gpio-config` | int | 否 | A 相负极 GPIO 配置 |
| `pb-gpio` | int | 是 | B 相正极 GPIO |
| `pb-gpio-config` | int | 否 | B 相正极 GPIO 配置 |
| `nb-gpio` | int | 是 | B 相负极 GPIO |
| `nb-gpio-config` | int | 否 | B 相负极 GPIO 配置 |
| `default-speed` | int | 否 | 默认速度（步/秒），默认 100 |

设备树示例：

```json
{
    "bipolar-stepper": {
        "type": "stepper-bipolar-gpio",
        "drive-mode": "halfstep",
        "pa-gpio": 0,
        "na-gpio": 1,
        "pb-gpio": 2,
        "nb-gpio": 3,
        "default-speed": 200
    }
}
```

### stepper-unipolar-gpio

单极性步进电机 GPIO 驱动，通过 4 个 GPIO（A、B、C、D）直接驱动单极性（5 线或 6 线）步进电机。支持三种驱动模式：

| 模式 | drive-mode 值 | 说明 |
|------|--------------|------|
| 单波驱动 | `wave` | 每次只励磁一相，4 步一个周期，扭矩较小 |
| 整步驱动 | `fullstep` | 每次励磁两相，4 步一个周期，扭矩较大 |
| 半步驱动 | `halfstep` | 交替单相/双相励磁，8 步一个周期，分辨率提高一倍 |

设备树配置属性：

| 属性 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `drive-mode` | string | 否 | 驱动模式：`wave`、`fullstep`、`halfstep`，默认 `wave` |
| `a-gpio` | int | 是 | A 相 GPIO |
| `a-gpio-config` | int | 否 | A 相 GPIO 配置 |
| `b-gpio` | int | 是 | B 相 GPIO |
| `b-gpio-config` | int | 否 | B 相 GPIO 配置 |
| `c-gpio` | int | 是 | C 相 GPIO |
| `c-gpio-config` | int | 否 | C 相 GPIO 配置 |
| `d-gpio` | int | 是 | D 相 GPIO |
| `d-gpio-config` | int | 否 | D 相 GPIO 配置 |
| `default-speed` | int | 否 | 默认速度（步/秒），默认 100 |

设备树示例：

```json
{
    "unipolar-stepper": {
        "type": "stepper-unipolar-gpio",
        "drive-mode": "fullstep",
        "a-gpio": 10,
        "b-gpio": 11,
        "c-gpio": 12,
        "d-gpio": 13,
        "default-speed": 150
    }
}
```

### stepper-pluse-dir

脉冲/方向步进电机 GPIO 驱动，用于控制带外部驱动器（如 TB6600、DM542 等）的步进电机。通过脉冲信号和方向信号控制电机转动，每个脉冲对应一步。支持信号极性反转配置。

设备树配置属性：

| 属性 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `pluse-gpio` | int | 是 | 脉冲信号 GPIO |
| `pluse-gpio-config` | int | 否 | 脉冲 GPIO 配置 |
| `pluse-gpio-inverted` | int | 否 | 脉冲信号极性反转，默认 0 |
| `dir-gpio` | int | 否 | 方向信号 GPIO（-1 表示不使用） |
| `dir-gpio-config` | int | 否 | 方向 GPIO 配置 |
| `dir-gpio-inverted` | int | 否 | 方向信号极性反转，默认 0 |
| `enable-gpio` | int | 否 | 使能信号 GPIO（-1 表示不使用） |
| `enable-gpio-config` | int | 否 | 使能 GPIO 配置 |
| `enable-gpio-inverted` | int | 否 | 使能信号极性反转，默认 0 |
| `default-speed` | int | 否 | 默认速度（步/秒），默认 100 |

设备树示例：

```json
{
    "pluse-dir-stepper": {
        "type": "stepper-pluse-dir",
        "pluse-gpio": 20,
        "pluse-gpio-inverted": 0,
        "dir-gpio": 21,
        "dir-gpio-inverted": 0,
        "enable-gpio": 22,
        "enable-gpio-inverted": 1,
        "default-speed": 500
    }
}
```

## 说明

步进电机控制接口。按指定步数和速度移动，支持忙状态查询。用于 3D 打印机和 CNC 机床。

- **双极性 (bipolar)**：4 线步进电机，需要 H 桥驱动，通过正反向电流控制方向
- **单极性 (unipolar)**：5/6 线步进电机，使用 ULN2003 等达林顿阵列驱动，每相单方向电流
- **脉冲/方向 (pluse-dir)**：配合外部步进电机驱动器使用，仅需脉冲和方向两路信号，适合大功率应用
