# SD/MMC Controller (sd)

SD/MMC host controller.

## Device Type

`DEVICE_TYPE_SDHCI`

## Structure

```c
struct sdhci_t {
    char * name;
    int voltage, width, clock;
    int removable, isspi;
    void * sdcard;
    int (*detect)(struct sdhci_t * sdhci);
    int (*reset)(struct sdhci_t * sdhci);
    int (*setvoltage)(struct sdhci_t * sdhci, int voltage);
    int (*setwidth)(struct sdhci_t * sdhci, int width);
    int (*setclock)(struct sdhci_t * sdhci, int clock);
    int (*transfer)(struct sdhci_t * sdhci, struct sdhci_cmd_t * cmd, struct sdhci_data_t * dat);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_sdhci(name)` | Find SDHCI by name |
| `register_sdhci(sdhci, drv)` | Register an SDHCI |
| `unregister_sdhci(sdhci)` | Unregister an SDHCI |
| `sdhci_detect(sdhci)` | Detect card |
| `sdhci_transfer(sdhci, cmd, data)` | Transfer command/data |

## Description

SD/MMC host controller interface. Manages SD/SDHC/SDXC and MMC/eMMC cards, supports SPI mode and removable card detection.
