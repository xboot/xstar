#ifndef __XSTAR_KERNEL_SHELL_CONTEXT_H__
#define __XSTAR_KERNEL_SHELL_CONTEXT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <kernel/xfs/xfs.h>

struct xfs_context_t * shell_getxfs(void);

int shell_realpath(const char * path, char * fpath);
const char * shell_getcwd(void);
int shell_setcwd(const char * path);

uint32_t * shell_history_prev(void);
uint32_t * shell_history_next(void);
void shell_history_add(uint32_t * ucs4, int len);

int shell_getchar(void);
int shell_putchar(int c);
int shell_vprintf(const char * fmt, va_list ap);
int shell_printf(const char * fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_SHELL_CONTEXT_H__ */
