# SD/MMC控制器 (sd)

SD/MMC 主机控制器。

## 设备类型

`DEVICE_TYPE_SDHCI`

## 结构体

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

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_sdhci(name)` | 按名称查找 SDHCI |
| `register_sdhci(sdhci, drv)` | 注册 SDHCI |
| `unregister_sdhci(sdhci)` | 注销 SDHCI |
| `sdhci_detect(sdhci)` | 检测卡 |
| `sdhci_transfer(sdhci, cmd, data)` | 传输命令/数据 |

## 说明

SD/MMC 主机控制器接口。管理 SD/SDHC/SDXC 和 MMC/eMMC 卡，支持 SPI 模式和可移除卡检测。
