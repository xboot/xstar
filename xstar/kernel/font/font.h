#ifndef __XSTAR_KERNEL_FONT_H__
#define __XSTAR_KERNEL_FONT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/xfs/xfs.h>

#define FONT_ICON_SYMBOL_BULLET			"\u2022"
#define FONT_ICON_SYMBOL_AUDIO			"\uf001"
#define FONT_ICON_SYMBOL_VIDEO			"\uf008"
#define FONT_ICON_SYMBOL_LIST			"\uf00b"
#define FONT_ICON_SYMBOL_OK				"\uf00c"
#define FONT_ICON_SYMBOL_CLOSE			"\uf00d"
#define FONT_ICON_SYMBOL_POWER			"\uf011"
#define FONT_ICON_SYMBOL_SETTINGS		"\uf013"
#define FONT_ICON_SYMBOL_HOME			"\uf015"
#define FONT_ICON_SYMBOL_DOWNLOAD		"\uf019"
#define FONT_ICON_SYMBOL_DRIVE			"\uf01c"
#define FONT_ICON_SYMBOL_REFRESH		"\uf021"
#define FONT_ICON_SYMBOL_MUTE			"\uf026"
#define FONT_ICON_SYMBOL_VOLUME_MID		"\uf027"
#define FONT_ICON_SYMBOL_VOLUME_MAX		"\uf028"
#define FONT_ICON_SYMBOL_IMAGE			"\uf03e"
#define FONT_ICON_SYMBOL_TINT			"\uf043"
#define FONT_ICON_SYMBOL_PREV			"\uf048"
#define FONT_ICON_SYMBOL_PLAY			"\uf04b"
#define FONT_ICON_SYMBOL_PAUSE			"\uf04c"
#define FONT_ICON_SYMBOL_STOP			"\uf04d"
#define FONT_ICON_SYMBOL_NEXT			"\uf051"
#define FONT_ICON_SYMBOL_EJECT			"\uf052"
#define FONT_ICON_SYMBOL_LEFT			"\uf053"
#define FONT_ICON_SYMBOL_RIGHT			"\uf054"
#define FONT_ICON_SYMBOL_PLUS			"\uf067"
#define FONT_ICON_SYMBOL_MINUS			"\uf068"
#define FONT_ICON_SYMBOL_EYE_OPEN		"\uf06e"
#define FONT_ICON_SYMBOL_EYE_CLOSE		"\uf070"
#define FONT_ICON_SYMBOL_WARNING		"\uf071"
#define FONT_ICON_SYMBOL_SHUFFLE		"\uf074"
#define FONT_ICON_SYMBOL_UP				"\uf077"
#define FONT_ICON_SYMBOL_DOWN			"\uf078"
#define FONT_ICON_SYMBOL_LOOP			"\uf079"
#define FONT_ICON_SYMBOL_DIRECTORY		"\uf07b"
#define FONT_ICON_SYMBOL_UPLOAD			"\uf093"
#define FONT_ICON_SYMBOL_CALL			"\uf095"
#define FONT_ICON_SYMBOL_CUT			"\uf0c4"
#define FONT_ICON_SYMBOL_COPY			"\uf0c5"
#define FONT_ICON_SYMBOL_SAVE			"\uf0c7"
#define FONT_ICON_SYMBOL_BARS			"\uf0c9"
#define FONT_ICON_SYMBOL_ENVELOPE		"\uf0e0"
#define FONT_ICON_SYMBOL_CHARGE			"\uf0e7"
#define FONT_ICON_SYMBOL_PASTE			"\uf0ea"
#define FONT_ICON_SYMBOL_BELL			"\uf0f3"
#define FONT_ICON_SYMBOL_KEYBOARD		"\uf11c"
#define FONT_ICON_SYMBOL_GPS			"\uf124"
#define FONT_ICON_SYMBOL_FILE			"\uf158"
#define FONT_ICON_SYMBOL_WIFI			"\uf1eb"
#define FONT_ICON_SYMBOL_BATTERY_FULL	"\uf240"
#define FONT_ICON_SYMBOL_BATTERY_3		"\uf241"
#define FONT_ICON_SYMBOL_BATTERY_2		"\uf242"
#define FONT_ICON_SYMBOL_BATTERY_1		"\uf243"
#define FONT_ICON_SYMBOL_BATTERY_EMPTY	"\uf244"
#define FONT_ICON_SYMBOL_USB			"\uf287"
#define FONT_ICON_SYMBOL_BLUETOOTH		"\uf293"
#define FONT_ICON_SYMBOL_TRASH			"\uf2ed"
#define FONT_ICON_SYMBOL_EDIT			"\uf304"
#define FONT_ICON_SYMBOL_BACKSPACE		"\uf55a"
#define FONT_ICON_SYMBOL_SD_CARD		"\uf7c2"
#define FONT_ICON_SYMBOL_NEW_LINE		"\uf8a2"

enum {
	FONT_ICON_CODE_BULLET				= 0x2022,
	FONT_ICON_CODE_AUDIO				= 0xf001,
	FONT_ICON_CODE_VIDEO				= 0xf008,
	FONT_ICON_CODE_LIST					= 0xf00b,
	FONT_ICON_CODE_OK					= 0xf00c,
	FONT_ICON_CODE_CLOSE				= 0xf00d,
	FONT_ICON_CODE_POWER				= 0xf011,
	FONT_ICON_CODE_SETTINGS				= 0xf013,
	FONT_ICON_CODE_HOME					= 0xf015,
	FONT_ICON_CODE_DOWNLOAD				= 0xf019,
	FONT_ICON_CODE_DRIVE				= 0xf01c,
	FONT_ICON_CODE_REFRESH				= 0xf021,
	FONT_ICON_CODE_MUTE					= 0xf026,
	FONT_ICON_CODE_VOLUME_MID			= 0xf027,
	FONT_ICON_CODE_VOLUME_MAX			= 0xf028,
	FONT_ICON_CODE_IMAGE				= 0xf03e,
	FONT_ICON_CODE_TINT					= 0xf043,
	FONT_ICON_CODE_PREV					= 0xf048,
	FONT_ICON_CODE_PLAY					= 0xf04b,
	FONT_ICON_CODE_PAUSE				= 0xf04c,
	FONT_ICON_CODE_STOP					= 0xf04d,
	FONT_ICON_CODE_NEXT					= 0xf051,
	FONT_ICON_CODE_EJECT				= 0xf052,
	FONT_ICON_CODE_LEFT					= 0xf053,
	FONT_ICON_CODE_RIGHT				= 0xf054,
	FONT_ICON_CODE_PLUS					= 0xf067,
	FONT_ICON_CODE_MINUS				= 0xf068,
	FONT_ICON_CODE_EYE_OPEN				= 0xf06e,
	FONT_ICON_CODE_EYE_CLOSE			= 0xf070,
	FONT_ICON_CODE_WARNING				= 0xf071,
	FONT_ICON_CODE_SHUFFLE				= 0xf074,
	FONT_ICON_CODE_UP					= 0xf077,
	FONT_ICON_CODE_DOWN					= 0xf078,
	FONT_ICON_CODE_LOOP					= 0xf079,
	FONT_ICON_CODE_DIRECTORY			= 0xf07b,
	FONT_ICON_CODE_UPLOAD				= 0xf093,
	FONT_ICON_CODE_CALL					= 0xf095,
	FONT_ICON_CODE_CUT					= 0xf0c4,
	FONT_ICON_CODE_COPY					= 0xf0c5,
	FONT_ICON_CODE_SAVE					= 0xf0c7,
	FONT_ICON_CODE_BARS					= 0xf0c9,
	FONT_ICON_CODE_ENVELOPE				= 0xf0e0,
	FONT_ICON_CODE_CHARGE				= 0xf0e7,
	FONT_ICON_CODE_PASTE				= 0xf0ea,
	FONT_ICON_CODE_BELL					= 0xf0f3,
	FONT_ICON_CODE_KEYBOARD				= 0xf11c,
	FONT_ICON_CODE_GPS					= 0xf124,
	FONT_ICON_CODE_FILE					= 0xf158,
	FONT_ICON_CODE_WIFI					= 0xf1eb,
	FONT_ICON_CODE_BATTERY_FULL			= 0xf240,
	FONT_ICON_CODE_BATTERY_3			= 0xf241,
	FONT_ICON_CODE_BATTERY_2			= 0xf242,
	FONT_ICON_CODE_BATTERY_1			= 0xf243,
	FONT_ICON_CODE_BATTERY_EMPTY		= 0xf244,
	FONT_ICON_CODE_USB					= 0xf287,
	FONT_ICON_CODE_BLUETOOTH			= 0xf293,
	FONT_ICON_CODE_TRASH				= 0xf2ed,
	FONT_ICON_CODE_EDIT					= 0xf304,
	FONT_ICON_CODE_BACKSPACE			= 0xf55a,
	FONT_ICON_CODE_SD_CARD				= 0xf7c2,
	FONT_ICON_CODE_NEW_LINE				= 0xf8a2,
};

enum font_style_t {
	FONT_STYLE_REGULAR					= 0,
	FONT_STYLE_ITALIC					= 1,
	FONT_STYLE_BOLD						= 2,
	FONT_STYLE_BOLDITALIC				= 3,
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
