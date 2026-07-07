#ifndef __XSTAR_DRIVER_LEDTRIGGER_H__
#define __XSTAR_DRIVER_LEDTRIGGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct ledtrigger_t {
	char * name;
	void * priv;
};

struct ledtrigger_t * search_ledtrigger(const char * name);
struct device_t * register_ledtrigger(struct ledtrigger_t * trigger, struct driver_t * drv);
void unregister_ledtrigger(struct ledtrigger_t * trigger);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_LEDTRIGGER_H__ */
