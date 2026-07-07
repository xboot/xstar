#ifndef __XSTAR_DRIVER_SD_SDHCI_H__
#define __XSTAR_DRIVER_SD_SDHCI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>
#include <driver/sd/sdmmc.h>

struct sdhci_cmd_t {
	uint32_t cmdidx;
	uint32_t cmdarg;
	uint32_t resptype;
	uint32_t response[4];
};

struct sdhci_data_t {
	uint8_t * buf;
	uint32_t flag;
	uint32_t blksz;
	uint32_t blkcnt;
};

struct sdhci_t {
	char * name;
	uint32_t voltage;
	uint32_t width;
	uint32_t clock;
	int removable;
	int isspi;
	void * sdcard;

	int (*detect)(struct sdhci_t * hci);
	int (*reset)(struct sdhci_t * hci);
	int (*setvoltage)(struct sdhci_t * hci, uint32_t voltage);
	int (*setwidth)(struct sdhci_t * hci, uint32_t width);
	int (*setclock)(struct sdhci_t * hci, uint32_t clock);
	int (*transfer)(struct sdhci_t * hci, struct sdhci_cmd_t * cmd, struct sdhci_data_t * dat);
	void * priv;
};

struct sdhci_t * search_sdhci(const char * name);
struct device_t * register_sdhci(struct sdhci_t * hci, struct driver_t * drv);
void unregister_sdhci(struct sdhci_t * hci);

int sdhci_detect(struct sdhci_t * hci);
int sdhci_reset(struct sdhci_t * hci);
int sdhci_set_voltage(struct sdhci_t * hci, uint32_t voltage);
int sdhci_set_width(struct sdhci_t * hci, uint32_t width);
int sdhci_set_clock(struct sdhci_t * hci, uint32_t clock);
int sdhci_transfer(struct sdhci_t * hci, struct sdhci_cmd_t * cmd, struct sdhci_data_t * dat);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_SD_SDHCI_H__ */
