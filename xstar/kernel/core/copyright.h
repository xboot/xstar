#ifndef __XSTAR_KERNEL_CORE_COPYRIGHT_H__
#define __XSTAR_KERNEL_CORE_COPYRIGHT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

struct copyright_t {
	const char * (*uniqueid)(void);
	int (*keygen)(const char * msg, void * key, int maxlen);
	int (*verify)(void);
};

const char * copyright_uniqueid(void);
int copyright_keygen(const char * msg, void * key, int maxlen);
int copyright_verify(void);

void register_copyright(struct copyright_t * c);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_CORE_COPYRIGHT_H__ */
