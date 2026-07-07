#ifndef __XSTAR_DRIVER_ATNET_H__
#define __XSTAR_DRIVER_ATNET_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct atnet_tcp_t {
	int (*isconnect)(struct atnet_tcp_t * tcp);
	int (*connect)(struct atnet_tcp_t * tcp, const char * host, int port);
	int (*disconnect)(struct atnet_tcp_t * tcp);
	int (*send)(struct atnet_tcp_t * tcp, void * buf, int len);
	void (*on_stream)(struct atnet_tcp_t * tcp, void * buf, int len);
	void (*on_connect)(struct atnet_tcp_t * tcp);
	void (*on_disconnect)(struct atnet_tcp_t * tcp);
	void (*destroy)(struct atnet_tcp_t * tcp);
	void * priv;
};

struct atnet_udp_t {
	int (*isconnect)(struct atnet_udp_t * udp);
	int (*connect)(struct atnet_udp_t * udp, const char * host, int port);
	int (*disconnect)(struct atnet_udp_t * udp);
	int (*send)(struct atnet_udp_t * udp, void * buf, int len);
	void (*on_stream)(struct atnet_udp_t * udp, void * buf, int len);
	void (*on_connect)(struct atnet_udp_t * udp);
	void (*on_disconnect)(struct atnet_udp_t * udp);
	void (*destroy)(struct atnet_udp_t * udp);
	void * priv;
};

struct atnet_t {
	char * name;

	struct atnet_tcp_t * (*tcp_create)(struct atnet_t * net);
	struct atnet_tcp_t * (*ssl_create)(struct atnet_t * net);
	struct atnet_udp_t * (*udp_create)(struct atnet_t * net);
	int (*ioctl)(struct atnet_t * net, const char * cmd, void * arg);
	void * priv;
};

struct atnet_t * search_atnet(const char * name);
struct atnet_t * search_first_atnet(void);
struct device_t * register_atnet(struct atnet_t * net, struct driver_t * drv);
void unregister_atnet(struct atnet_t * net);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_ATNET_H__ */
