#ifndef __XSTAR_KERNEL_FONT_H__
#define __XSTAR_KERNEL_FONT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/xfs/xfs.h>

enum font_style_t {
	FONT_STYLE_REGULAR		= 0,
	FONT_STYLE_ITALIC		= 1,
	FONT_STYLE_BOLD			= 2,
	FONT_STYLE_BOLDITALIC	= 3,
};

void font_install_from_xfs(const char * family, enum font_style_t style, struct xfs_context_t * xfs, const char * path);
void font_install_from_buf(const char * family, enum font_style_t style, const void * buf, int len);
void font_uninstall(const char * family, enum font_style_t style);

int font_icon_bound(const char * family, int size, uint32_t code, int * width, int * height);
void font_icon_render(const char * family, int size, int x, int y, uint32_t code, void (*cb)(void *, int, int, void *, int, int), void * data);

int font_text_bound(const char * family, enum font_style_t style, int size, int wrap, const char * str, int * width, int * height);
void font_text_render(const char * family, enum font_style_t style, int size, int x, int y, int wrap, const char * str, void (*cb)(void *, int, int, void *, int, int), void * data);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_FONT_H__ */
