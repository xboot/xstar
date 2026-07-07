#ifndef __XSTAR_DRIVER_ADC_H__
#define __XSTAR_DRIVER_ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct adc_t {
	char * name;
	int vreference;
	int resolution;
	int nchannel;

	int32_t (*read)(struct adc_t * adc, int channel);
	void * priv;
};

struct adc_t * search_adc(const char * name);
struct device_t * register_adc(struct adc_t * adc, struct driver_t * drv);
void unregister_adc(struct adc_t * adc);

int32_t adc_read_raw(struct adc_t * adc, int channel);
int adc_read_voltage(struct adc_t * adc, int channel);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_ADC_H__ */
