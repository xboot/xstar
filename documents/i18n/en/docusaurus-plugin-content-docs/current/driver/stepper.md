# Stepper Motor (stepper)

Stepper motor.

## Device Type

`DEVICE_TYPE_STEPPER`

## Structure

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

## Key API

| Function | Description |
|------|------|
| `search_stepper(name)` | Find stepper motor by name |
| `register_stepper(stepper, drv)` | Register a stepper motor |
| `unregister_stepper(stepper)` | Unregister a stepper motor |
| `stepper_enable/disable(stepper)` | Enable/disable |
| `stepper_move(stepper, step, speed)` | Move specified number of steps |
| `stepper_busying(stepper)` | Query if busy |

## Driver Implementations

### stepper-bipolar-gpio

Bipolar stepper motor GPIO driver. Controls a bipolar stepper motor via an H-bridge using 4 GPIOs (PA, NA, PB, NB). Supports three drive modes:

| Mode | drive-mode value | Description |
|------|------------------|-------------|
| Wave drive | `wave` | Energizes one phase at a time, 4-step cycle, lower torque |
| Full step drive | `fullstep` | Energizes two phases at a time, 4-step cycle, higher torque |
| Half step drive | `halfstep` | Alternates single/dual phase excitation, 8-step cycle, doubled resolution |

Device tree configuration properties:

| Property | Type | Required | Description |
|----------|------|----------|-------------|
| `drive-mode` | string | No | Drive mode: `wave`, `fullstep`, `halfstep`, default `wave` |
| `pa-gpio` | int | Yes | Phase A positive GPIO |
| `pa-gpio-config` | int | No | Phase A positive GPIO config |
| `na-gpio` | int | Yes | Phase A negative GPIO |
| `na-gpio-config` | int | No | Phase A negative GPIO config |
| `pb-gpio` | int | Yes | Phase B positive GPIO |
| `pb-gpio-config` | int | No | Phase B positive GPIO config |
| `nb-gpio` | int | Yes | Phase B negative GPIO |
| `nb-gpio-config` | int | No | Phase B negative GPIO config |
| `default-speed` | int | No | Default speed (steps/sec), default 100 |

Device tree example:

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

Unipolar stepper motor GPIO driver. Directly drives a unipolar (5-wire or 6-wire) stepper motor using 4 GPIOs (A, B, C, D). Supports three drive modes:

| Mode | drive-mode value | Description |
|------|------------------|-------------|
| Wave drive | `wave` | Energizes one phase at a time, 4-step cycle, lower torque |
| Full step drive | `fullstep` | Energizes two phases at a time, 4-step cycle, higher torque |
| Half step drive | `halfstep` | Alternates single/dual phase excitation, 8-step cycle, doubled resolution |

Device tree configuration properties:

| Property | Type | Required | Description |
|----------|------|----------|-------------|
| `drive-mode` | string | No | Drive mode: `wave`, `fullstep`, `halfstep`, default `wave` |
| `a-gpio` | int | Yes | Phase A GPIO |
| `a-gpio-config` | int | No | Phase A GPIO config |
| `b-gpio` | int | Yes | Phase B GPIO |
| `b-gpio-config` | int | No | Phase B GPIO config |
| `c-gpio` | int | Yes | Phase C GPIO |
| `c-gpio-config` | int | No | Phase C GPIO config |
| `d-gpio` | int | Yes | Phase D GPIO |
| `d-gpio-config` | int | No | Phase D GPIO config |
| `default-speed` | int | No | Default speed (steps/sec), default 100 |

Device tree example:

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

Pulse/Direction stepper motor GPIO driver. Used to control stepper motors with external drivers (e.g., TB6600, DM542). Controls motor rotation via pulse and direction signals, with each pulse corresponding to one step. Supports signal polarity inversion configuration.

Device tree configuration properties:

| Property | Type | Required | Description |
|----------|------|----------|-------------|
| `pluse-gpio` | int | Yes | Pulse signal GPIO |
| `pluse-gpio-config` | int | No | Pulse GPIO config |
| `pluse-gpio-inverted` | int | No | Pulse signal polarity inversion, default 0 |
| `dir-gpio` | int | No | Direction signal GPIO (-1 for unused) |
| `dir-gpio-config` | int | No | Direction GPIO config |
| `dir-gpio-inverted` | int | No | Direction signal polarity inversion, default 0 |
| `enable-gpio` | int | No | Enable signal GPIO (-1 for unused) |
| `enable-gpio-config` | int | No | Enable GPIO config |
| `enable-gpio-inverted` | int | No | Enable signal polarity inversion, default 0 |
| `default-speed` | int | No | Default speed (steps/sec), default 100 |

Device tree example:

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

## Description

Stepper motor control interface. Moves by a specified number of steps at a given speed, supports busy status query. Used in 3D printers and CNC machines.

- **Bipolar**: 4-wire stepper motor, requires H-bridge driver, direction controlled by current polarity
- **Unipolar**: 5/6-wire stepper motor, uses Darlington array drivers (e.g., ULN2003), single-direction current per phase
- **Pulse/Direction**: Works with external stepper motor drivers, requires only pulse and direction signals, suitable for high-power applications
