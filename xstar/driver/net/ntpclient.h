#ifndef __XSTAR_DRIVER_NTPCLIENT_H__
#define __XSTAR_DRIVER_NTPCLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/net/net.h>

int ntpclient_sync(struct net_t * net, const char * host);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_NTPCLIENT_H__ */
