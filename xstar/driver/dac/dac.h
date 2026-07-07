#ifndef __XSTAR_DRIVER_DAC_H__
#define __XSTAR_DRIVER_DAC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct dac_t {
	char * name;
	int vreference;
	int resolution;
	int nchannel;

	void (*write)(struct dac_t * dac, int channel, uint32_t value);
	void * priv;
};

struct dac_t * search_dac(const char * name);
struct device_t * register_dac(struct dac_t * dac, struct driver_t * drv);
void unregister_dac(struct dac_t * dac);

void dac_write_raw(struct dac_t * dac, int channel, uint32_t value);
void dac_write_voltage(struct dac_t * dac, int channel, int voltage);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_DAC_H__ */
