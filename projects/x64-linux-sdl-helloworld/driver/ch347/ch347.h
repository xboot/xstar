#ifndef __CH347_H__
#define __CH347_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstar.h>
#pragma push_macro("offsetof")
#pragma push_macro("container_of")
#undef offsetof
#undef container_of
#include <inc/ch34x_lib.h>
#pragma pop_macro("container_of")
#pragma pop_macro("offsetof")

int ch347_detect(void);
int ch347_lock(void);
int ch347_unlock(void);

#ifdef __cplusplus
}
#endif

#endif /* __CH347_H__ */
