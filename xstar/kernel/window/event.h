#ifndef __XSTAR_KERNEL_WINDOW_EVENT_H__
#define __XSTAR_KERNEL_WINDOW_EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

enum event_type_t {
	EVENT_TYPE_KEY_DOWN					= 0x0100,
	EVENT_TYPE_KEY_UP					= 0x0101,

	EVENT_TYPE_ROTARY_STEP				= 0x0200,

	EVENT_TYPE_MOUSE_DOWN				= 0x0300,
	EVENT_TYPE_MOUSE_MOVE				= 0x0301,
	EVENT_TYPE_MOUSE_UP					= 0x0302,
	EVENT_TYPE_MOUSE_WHEEL				= 0x0303,

	EVENT_TYPE_TOUCH_BEGIN				= 0x0400,
	EVENT_TYPE_TOUCH_MOVE				= 0x0401,
	EVENT_TYPE_TOUCH_END				= 0x0402,

	EVENT_TYPE_JOYSTICK_LEFTSTICK		= 0x0500,
	EVENT_TYPE_JOYSTICK_RIGHTSTICK		= 0x0501,
	EVENT_TYPE_JOYSTICK_LEFTTRIGGER		= 0x0502,
	EVENT_TYPE_JOYSTICK_RIGHTTRIGGER	= 0x0503,
	EVENT_TYPE_JOYSTICK_BUTTONDOWN		= 0x0504,
	EVENT_TYPE_JOYSTICK_BUTTONUP		= 0x0505,

	EVENT_TYPE_SYSTEM_EXIT				= 0x1000,
};

enum {
	KB_KEY_POWER						= 1,
	KB_KEY_UP							= 2,
	KB_KEY_DOWN							= 3,
	KB_KEY_LEFT							= 4,
	KB_KEY_RIGHT						= 5,
	KB_KEY_VOLUME_UP					= 6,
	KB_KEY_VOLUME_DOWN					= 7,
	KB_KEY_VOLUME_MUTE					= 8,
	KB_KEY_TAB							= 9,
	KB_KEY_HOME							= 10,
	KB_KEY_BACK							= 11,
	KB_KEY_MENU							= 12,
	KB_KEY_ENTER						= 13,

	KB_KEY_L_CTRL						= 14,
	KB_KEY_R_CTRL						= 15,
	KB_KEY_L_ALT						= 16,
	KB_KEY_R_ALT						= 17,
	KB_KEY_L_SHIFT						= 18,
	KB_KEY_R_SHIFT						= 19,

	KB_KEY_F1							= 20,
	KB_KEY_F2							= 21,
	KB_KEY_F3							= 22,
	KB_KEY_F4							= 23,
	KB_KEY_F5							= 24,
	KB_KEY_F6							= 25,
	KB_KEY_F7							= 26,
	KB_KEY_F8							= 27,
	KB_KEY_F9							= 28,
	KB_KEY_F10							= 29,
	KB_KEY_F11							= 30,
	KB_KEY_F12							= 31,

	KB_KEY_SPACE						= 32,	/*   */
	KB_KEY_EXCLAMATION_MARK				= 33,	/* ! */
	KB_KEY_QUOTATION_MARK				= 34,	/* " */
	KB_KEY_POUNDSIGN					= 35,	/* # */
	KB_KEY_DOLLAR						= 36,	/* $ */
	KB_KEY_PERCENT						= 37,	/* % */
	KB_KEY_AMPERSAND					= 38,	/* & */
	KB_KEY_APOSTROPHE					= 39,	/* ' */
	KB_KEY_PARENTHESIS_LEFT				= 40,	/* ( */
	KB_KEY_PARENTHESIS_RIGHT			= 41,	/* ) */
	KB_KEY_ASTERISK						= 42,	/* * */
	KB_KEY_PLUS							= 43,	/* + */
	KB_KEY_COMMA						= 44,	/* , */
	KB_KEY_MINUS						= 45,	/* - */
	KB_KEY_FULL_STOP					= 46,	/* . */
	KB_KEY_SOLIDUS						= 47,	/* / */

	KB_KEY_0							= 48,	/* 0 */
	KB_KEY_1							= 49,	/* 1 */
	KB_KEY_2							= 50,	/* 2 */
	KB_KEY_3							= 51,	/* 3 */
	KB_KEY_4							= 52,	/* 4 */
	KB_KEY_5							= 53,	/* 5 */
	KB_KEY_6							= 54,	/* 6 */
	KB_KEY_7							= 55,	/* 7 */
	KB_KEY_8							= 56,	/* 8 */
	KB_KEY_9							= 57,	/* 9 */

	KB_KEY_COLON						= 58,	/* : */
	KB_KEY_SEMICOLON					= 59,	/* ; */
	KB_KEY_LESS_THAN					= 60,	/* < */
	KB_KEY_EQUAL						= 61,	/* = */
	KB_KEY_GREATER_THAN					= 62,	/* > */
	KB_KEY_QUESTION_MARK				= 63,	/* ? */
	KB_KEY_AT							= 64,	/* @ */

	KB_KEY_A							= 65,	/* A */
	KB_KEY_B							= 66,	/* B */
	KB_KEY_C							= 67,	/* C */
	KB_KEY_D							= 68,	/* D */
	KB_KEY_E							= 69,	/* E */
	KB_KEY_F							= 70,	/* F */
	KB_KEY_G							= 71,	/* G */
	KB_KEY_H							= 72,	/* H */
	KB_KEY_I							= 73,	/* I */
	KB_KEY_J							= 74,	/* J */
	KB_KEY_K							= 75,	/* K */
	KB_KEY_L							= 76,	/* L */
	KB_KEY_M							= 77,	/* M */
	KB_KEY_N							= 78,	/* N */
	KB_KEY_O							= 79,	/* O */
	KB_KEY_P							= 80,	/* P */
	KB_KEY_Q							= 81,	/* Q */
	KB_KEY_R							= 82,	/* R */
	KB_KEY_S							= 83,	/* S */
	KB_KEY_T							= 84,	/* T */
	KB_KEY_U							= 85,	/* U */
	KB_KEY_V							= 86,	/* V */
	KB_KEY_W							= 87,	/* W */
	KB_KEY_X							= 88,	/* X */
	KB_KEY_Y							= 89,	/* Y */
	KB_KEY_Z							= 90,	/* Z */

	KB_KEY_SQUARE_BRACKET_LEFT			= 91,	/* [ */
	KB_KEY_REVERSE_SOLIDUS				= 92,	/* \ */
	KB_KEY_SQUARE_BRACKET_RIGHT			= 93,	/* ] */
	KB_KEY_CIRCUMFLEX_ACCENT			= 94,	/* ^ */
	KB_KEY_LOW_LINE						= 95,	/* _ */
	KB_KEY_GRAVE_ACCENT					= 96,	/* ` */

	KB_KEY_a							= 97,	/* a */
	KB_KEY_b							= 98,	/* b */
	KB_KEY_c							= 99,	/* c */
	KB_KEY_d							= 100,	/* d */
	KB_KEY_e							= 101,	/* e */
	KB_KEY_f							= 102,	/* f */
	KB_KEY_g							= 103,	/* g */
	KB_KEY_h							= 104,	/* h */
	KB_KEY_i							= 105,	/* i */
	KB_KEY_j							= 106,	/* j */
	KB_KEY_k							= 107,	/* k */
	KB_KEY_l							= 108,	/* l */
	KB_KEY_m							= 109,	/* m */
	KB_KEY_n							= 110,	/* n */
	KB_KEY_o							= 111,	/* o */
	KB_KEY_p							= 112,	/* p */
	KB_KEY_q							= 113,	/* q */
	KB_KEY_r							= 114,	/* r */
	KB_KEY_s							= 115,	/* s */
	KB_KEY_t							= 116,	/* t */
	KB_KEY_u							= 117,	/* u */
	KB_KEY_v							= 118,	/* v */
	KB_KEY_w							= 119,	/* w */
	KB_KEY_x							= 120,	/* x */
	KB_KEY_y							= 121,	/* y */
	KB_KEY_z							= 122,	/* z */

	KB_KEY_CURLY_BRACKET_LEFT			= 123,	/* { */
	KB_KEY_VERTICAL_LINE				= 124,	/* | */
	KB_KEY_CURLY_BRACKET_RIGHT			= 125,	/* } */
	KB_KEY_TILDE						= 126,	/* ~ */
	KB_KEY_DELETE						= 127,

	/* Latin 1 extensions */
	KB_KEY_NO_BREAK_SPACE				= 160,	/*   */
	KB_KEY_EXCLAM_DOWN					= 161,	/* ¡ */
	KB_KEY_CENT							= 162,	/* ¢ */
	KB_KEY_STERLING						= 163,	/* £ */
	KB_KEY_CURRENCY						= 164,	/* ¤ */
	KB_KEY_YEN							= 165,	/* ¥ */
	KB_KEY_BROKEN_BAR					= 166,	/* ¦ */
	KB_KEY_SECTION						= 167,	/* § */
	KB_KEY_DIAERESIS					= 168,	/* ¨ */
	KB_KEY_COPYRIGHT					= 169,	/* © */
	KB_KEY_ORDFEMININE					= 170,	/* ª */
	KB_KEY_GUILLEMOTLEFT				= 171,	/* « */
	KB_KEY_NOTSIGN						= 172,	/* ¬ */
	KB_KEY_HYPHEN						= 173,	/* ­  */
	KB_KEY_REGISTERED					= 174,	/* ® */
	KB_KEY_MACRON						= 175,	/* ¯ */
	KB_KEY_DEGREE						= 176,	/* ° */
	KB_KEY_PLUSMINUS					= 177,	/* ± */
	KB_KEY_TWOSUPERIOR					= 178,	/* ² */
	KB_KEY_THREESUPERIOR				= 179,	/* ³ */
	KB_KEY_ACUTE						= 180,	/* ´ */
	KB_KEY_MU							= 181,	/* µ */
	KB_KEY_PARAGRAPH					= 182,	/* ¶ */
	KB_KEY_PERIODCENTERED				= 183,	/* · */
	KB_KEY_CEDILLA						= 184,	/* ¸ */
	KB_KEY_ONESUPERIOR					= 185,	/* ¹ */
	KB_KEY_MASCULINE					= 186,	/* º */
	KB_KEY_GUILLEMOTRIGHT				= 187,	/* » */
	KB_KEY_ONEQUARTER 					= 188,	/* ¼ */
	KB_KEY_ONEHALF 						= 189,	/* ½ */
	KB_KEY_THREEQUARTERS 				= 190,	/* ¾ */
	KB_KEY_QUESTIONDOWN 				= 191,	/* ¿ */
	KB_KEY_AGRAVE 						= 192,	/* À */
	KB_KEY_AACUTE 						= 193,	/* Á */
	KB_KEY_ACIRCUMFLEX 					= 194,	/* Â */
	KB_KEY_ATILDE 						= 195,	/* Ã */
	KB_KEY_ADIAERESIS 					= 196,	/* Ä */
	KB_KEY_ARING 						= 197,	/* Å */
	KB_KEY_AE 							= 198,	/* Æ */
	KB_KEY_CCEDILLA 					= 199,	/* Ç */
	KB_KEY_EGRAVE 						= 200,	/* È */
	KB_KEY_EACUTE 						= 201,	/* É */
	KB_KEY_ECIRCUMFLEX 					= 202,	/* Ê */
	KB_KEY_EDIAERESIS 					= 203,	/* Ë */
	KB_KEY_IGRAVE 						= 204,	/* Ì */
	KB_KEY_IACUTE 						= 205,	/* Í */
	KB_KEY_ICIRCUMFLEX 					= 206,	/* Î */
	KB_KEY_IDIAERESIS 					= 207,	/* Ï */
	KB_KEY_ETH 							= 208,	/* Ð */
	KB_KEY_NTILDE 						= 209,	/* Ñ */
	KB_KEY_OGRAVE 						= 210,	/* Ò */
	KB_KEY_OACUTE 						= 211,	/* Ó */
	KB_KEY_OCIRCUMFLEX 					= 212,	/* Ô */
	KB_KEY_OTILDE 						= 213,	/* Õ */
	KB_KEY_ODIAERESIS 					= 214,	/* Ö */
	KB_KEY_MULTIPLY 					= 215,	/* × */
	KB_KEY_OOBLIQUE 					= 216,	/* Ø */
	KB_KEY_UGRAVE 						= 217,	/* Ù */
	KB_KEY_UACUTE 						= 218,	/* Ú */
	KB_KEY_UCIRCUMFLEX 					= 219,	/* Û */
	KB_KEY_UDIAERESIS 					= 220,	/* Ü */
	KB_KEY_YACUTE 						= 221,	/* Ý */
	KB_KEY_THORN 						= 222,	/* Þ */
	KB_KEY_ssharp 						= 223,	/* ß */
	KB_KEY_agrave 						= 224,	/* à */
	KB_KEY_aacute 						= 225,	/* á */
	KB_KEY_acircumflex					= 226,	/* â */
	KB_KEY_atilde 						= 227,	/* ã */
	KB_KEY_adiaeresis 					= 228,	/* ä */
	KB_KEY_aring						= 229,	/* å */
	KB_KEY_ae 							= 230,	/* æ */
	KB_KEY_ccedilla 					= 231,	/* ç */
	KB_KEY_egrave 						= 232,	/* è */
	KB_KEY_eacute 						= 233,	/* é */
	KB_KEY_ecircumflex 					= 234,	/* ê */
	KB_KEY_ediaeresis 					= 235,	/* ë */
	KB_KEY_igrave						= 236,	/* ì */
	KB_KEY_iacute						= 237,	/* í */
	KB_KEY_icircumflex 					= 238,	/* î */
	KB_KEY_idiaeresis					= 239,	/* ï */
	KB_KEY_eth 							= 240,	/* ð */
	KB_KEY_ntilde 						= 241,	/* ñ */
	KB_KEY_ograve 						= 242,	/* ò */
	KB_KEY_oacute 						= 243,	/* ó */
	KB_KEY_ocircumflex 					= 244,	/* ô */
	KB_KEY_otilde 						= 245,	/* õ */
	KB_KEY_odiaeresis 					= 246,	/* ö */
	KB_KEY_DIVISION 					= 247,	/* ÷ */
	KB_KEY_ooblique 					= 248,	/* ø */
	KB_KEY_ugrave 						= 249,	/* ù */
	KB_KEY_uacute 						= 250,	/* ú */
	KB_KEY_ucircumflex 					= 251,	/* û */
	KB_KEY_udiaeresis					= 252,	/* ü */
	KB_KEY_yacute 						= 253,	/* ý */
	KB_KEY_thorn 						= 254,	/* þ */
	KB_KEY_ydiaeresis 					= 255,	/* ÿ */
};

enum {
	MOUSE_BUTTON_LEFT					= (0x1 << 0),
	MOUSE_BUTTON_RIGHT					= (0x1 << 1),
	MOUSE_BUTTON_MIDDLE					= (0x1 << 2),
	MOUSE_BUTTON_X1						= (0x1 << 3),
	MOUSE_BUTTON_X2						= (0x1 << 4),
};

enum {
	JOYSTICK_BUTTON_UP					= (0x1 << 0),
	JOYSTICK_BUTTON_DOWN				= (0x1 << 1),
	JOYSTICK_BUTTON_LEFT				= (0x1 << 2),
	JOYSTICK_BUTTON_RIGHT				= (0x1 << 3),
	JOYSTICK_BUTTON_A					= (0x1 << 4),
	JOYSTICK_BUTTON_B					= (0x1 << 5),
	JOYSTICK_BUTTON_X					= (0x1 << 6),
	JOYSTICK_BUTTON_Y					= (0x1 << 7),
	JOYSTICK_BUTTON_BACK				= (0x1 << 8),
	JOYSTICK_BUTTON_START				= (0x1 << 9),
	JOYSTICK_BUTTON_GUIDE				= (0x1 << 10),
	JOYSTICK_BUTTON_LBUMPER				= (0x1 << 11),
	JOYSTICK_BUTTON_RBUMPER				= (0x1 << 12),
	JOYSTICK_BUTTON_LSTICK				= (0x1 << 13),
	JOYSTICK_BUTTON_RSTICK				= (0x1 << 14),
};

struct event_t {
	void * device;
	enum event_type_t type;
	ktime_t timestamp;

	union {
		/* Key */
		struct {
			uint32_t key;
		} key_down;

		struct {
			uint32_t key;
		} key_up;

		/* Rotary */
		struct {
			int32_t delta;
		} rotary_step;

		/* Mouse */
		struct {
			int32_t x, y;
			uint32_t button;
		} mouse_down;

		struct {
			int32_t x, y;
		} mouse_move;

		struct {
			int32_t x, y;
			uint32_t button;
		} mouse_up;

		struct {
			int32_t dx, dy;
		} mouse_wheel;

		/* Touch */
		struct {
			int32_t x, y;
			uint32_t id;
		} touch_begin;

		struct {
			int32_t x, y;
			uint32_t id;
		} touch_move;

		struct {
			int32_t x, y;
			uint32_t id;
		} touch_end;

		/* Joystick */
		struct {
			int32_t x, y;
		} joystick_left_stick;

		struct {
			int32_t x, y;
		} joystick_right_stick;

		struct {
			int32_t v;
		} joystick_left_trigger;

		struct {
			int32_t v;
		} joystick_right_trigger;

		struct {
			uint32_t button;
		} joystick_button_down;

		struct {
			uint32_t button;
		} joystick_button_up;
	} e;
};

void push_event_key_down(void * device, uint32_t key);
void push_event_key_up(void * device, uint32_t key);
void push_event_rotary_step(void * device, int32_t delta);
void push_event_mouse_button_down(void * device, int32_t x, int32_t y, uint32_t button);
void push_event_mouse_button_up(void * device, int32_t x, int32_t y, uint32_t button);
void push_event_mouse_move(void * device, int32_t x, int32_t y);
void push_event_mouse_wheel(void * device, int32_t dx, int32_t dy);
void push_event_touch_begin(void * device, int32_t x, int32_t y, uint32_t id);
void push_event_touch_move(void * device, int32_t x, int32_t y, uint32_t id);
void push_event_touch_end(void * device, int32_t x, int32_t y, uint32_t id);
void push_event_joystick_left_stick(void * device, int32_t x, int32_t y);
void push_event_joystick_right_stick(void * device, int32_t x, int32_t y);
void push_event_joystick_left_trigger(void * device, int32_t v);
void push_event_joystick_right_trigger(void * device, int32_t v);
void push_event_joystick_button_down(void * device, uint32_t button);
void push_event_joystick_button_up(void * device, uint32_t button);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_WINDOW_EVENT_H__ */
