#ifndef __XSTAR_KERNEL_CORE_SETTING_H__
#define __XSTAR_KERNEL_CORE_SETTING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

void setting_set(const char * key, const char * value);
const char * setting_get(const char * key, const char * def);
void setting_clear(void);
void setting_sync(void);
void setting_foreach(void (*cb)(const char * key, const char * value));

void do_init_setting(void);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_CORE_SETTING_H__ */
