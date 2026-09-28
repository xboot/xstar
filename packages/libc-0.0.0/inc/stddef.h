#ifndef __STDDEF_H__
#define __STDDEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#if !(defined(NULL))
#if defined(__cplusplus)
#define NULL	(0)
#else
#define NULL	((void *)0)
#endif
#endif

#if !defined(__cplusplus) && !defined(_WCHAR_T)
#define _WCHAR_T
typedef __WCHAR_TYPE__ wchar_t;
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STDDEF_H__ */
