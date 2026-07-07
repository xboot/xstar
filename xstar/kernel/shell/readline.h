#ifndef __XSTAR_KERNEL_SHELL_READLINE_H__
#define __XSTAR_KERNEL_SHELL_READLINE_H__

#ifdef __cplusplus
extern "C" {
#endif

char * shell_readline(const char * prompt);
char * shell_password(const char * prompt);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_SHELL_READLINE_H__ */
