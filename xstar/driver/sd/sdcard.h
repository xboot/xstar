#ifndef __XSTAR_DRIVER_SD_SDCARD_H__
#define __XSTAR_DRIVER_SD_SDCARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <driver/sd/sdhci.h>

void * sdcard_probe(struct sdhci_t * hci);
void sdcard_remove(void * card);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_SD_SDCARD_H__ */
