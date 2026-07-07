#ifndef __XSTAR_KERNEL_CORE_LOGGER_H__
#define __XSTAR_KERNEL_CORE_LOGGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

#ifdef CONFIG_XSTAR_LOG
#define LOG(fmt, arg...)	do { logger(fmt, ##arg); } while(0)
#else
#define LOG(fmt, arg...)	do { } while(0)
#endif

void logger_enable(void);
void logger_disable(void);
int logger_status(void);
int logger(const char * fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_CORE_LOGGER_H__ */
