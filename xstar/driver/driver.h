#ifndef __XSTAR_DRIVER_DRIVER_H__
#define __XSTAR_DRIVER_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>

struct device_t;

struct driver_t {
	struct kobj_t * kobj;
	struct hlist_node_t node;

	char * name;
	struct device_t * (*probe)(struct driver_t * drv, struct dtnode_t * dt);
	void (*remove)(struct device_t * dev);
	void (*suspend)(struct device_t * dev);
	void (*resume)(struct device_t * dev);
};

struct driver_t * search_driver(const char * name);
int register_driver(struct driver_t * drv);
int unregister_driver(struct driver_t * drv);
void probe_device(const char * json, int length);
void remove_device(struct device_t * dev);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_DRIVER_H__ */
