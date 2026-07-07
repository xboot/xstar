#ifndef __CH347_H__
#define __CH347_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstar.h>
#include <inc/ch347_lib.h>

int ch347_detect(void);
int ch347_lock(void);
int ch347_unlock(void);

#ifdef __cplusplus
}
#endif

#endif /* __CH347_H__ */
