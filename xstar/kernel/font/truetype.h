#ifndef __XSTAR_KERNEL_FONT_TRUETYPE_H__
#define __XSTAR_KERNEL_FONT_TRUETYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

struct truetype_buf_t {
	unsigned char * data;
	int cursor;
	int size;
};

struct truetype_fontinfo_t {
	unsigned char * data;
	int fontstart;
	int nglyphs;
	int loca, head, glyf, hhea, hmtx, kern, gpos, svg;
	int index_map;
	int index_to_loc_format;

	struct truetype_buf_t cff;
	struct truetype_buf_t charstrings;
	struct truetype_buf_t gsubrs;
	struct truetype_buf_t subrs;
	struct truetype_buf_t fontdicts;
	struct truetype_buf_t fdselect;
};

int truetype_init(struct truetype_fontinfo_t * info, unsigned char * data, int offset);
int truetrype_find_glyph_index(struct truetype_fontinfo_t * info, int unicode);
float truetype_scale_for_pixel_height(struct truetype_fontinfo_t * info, float pixels);
void turetype_get_font_vmetrics(struct truetype_fontinfo_t * info, int * ascent, int * descent, int * linegap);
void truetype_get_glyph_hmetrics(struct truetype_fontinfo_t * info, int glyph, int * advance, int * lsb);
int truetype_get_glyph_kern_advance(struct truetype_fontinfo_t * info, int glyph1, int glyph2);
void truetype_get_glyph_bitmap_box(struct truetype_fontinfo_t * font, int glyph, float scale_x, float scale_y, int * ix0, int * iy0, int * ix1, int * iy1);
void truetype_make_glyph_bitmap(struct truetype_fontinfo_t * info, unsigned char * output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int glyph);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_FONT_TRUETYPE_H__ */
