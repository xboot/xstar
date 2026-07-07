/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <kernel/font/truetype.h>

enum {
	TRUETYPE_VMOVE = 1,
	TRUETYPE_VLINE = 2,
	TRUETYPE_VCURVE = 3,
	TRUETYPE_VCUBIC = 4,
};

enum {
	STBTT_PLATFORM_ID_UNICODE = 0,
	STBTT_PLATFORM_ID_MAC = 1,
	STBTT_PLATFORM_ID_ISO = 2,
	STBTT_PLATFORM_ID_MICROSOFT = 3,
};

enum {
	STBTT_MS_EID_SYMBOL = 0,
	STBTT_MS_EID_UNICODE_BMP = 1,
	STBTT_MS_EID_SHIFTJIS = 2,
	STBTT_MS_EID_UNICODE_FULL = 10,
};

struct truetype_point_t {
	float x, y;
};

struct truetype_edge_t {
	float x0, y0, x1, y1;
	int invert;
};

struct truetype_active_edge_t {
	struct truetype_active_edge_t * next;
	float fx, fdx, fdy;
	float direction;
	float sy;
	float ey;
};

struct truetype_hheap_chunk_t {
	struct truetype_hheap_chunk_t * next;
};

struct truetype_hheap_t {
	struct truetype_hheap_chunk_t * head;
	void * first_free;
	int num_remaining_in_head_chunk;
};

struct truetype_vertex_t {
	short int x, y, cx, cy, cx1, cy1;
	unsigned char type, padding;
};

struct truetype_csctx_t {
	int bounds;
	int started;
	float first_x, first_y;
	float x, y;
	int32_t min_x, max_x, min_y, max_y;
	struct truetype_vertex_t * pvertices;
	int num_vertices;
};

struct truetype_bitmap_t {
	int w, h, stride;
	unsigned char * pixels;
};

static uint8_t truetype_buf_get8(struct truetype_buf_t * b)
{
	if(b->cursor >= b->size)
		return 0;
	return b->data[b->cursor++];
}

static uint8_t truetype_buf_peek8(struct truetype_buf_t * b)
{
	if(b->cursor >= b->size)
		return 0;
	return b->data[b->cursor];
}

static void truetype_buf_seek(struct truetype_buf_t * b, int o)
{
	b->cursor = (o > b->size || o < 0) ? b->size : o;
}

static void truetype_buf_skip(struct truetype_buf_t * b, int o)
{
	truetype_buf_seek(b, b->cursor + o);
}

static uint32_t truetype_buf_get(struct truetype_buf_t * b, int n)
{
	uint32_t v = 0;
	int i;
	for(i = 0; i < n; i++)
		v = (v << 8) | truetype_buf_get8(b);
	return v;
}

static struct truetype_buf_t truetype_new_buf(const void * p, size_t size)
{
	struct truetype_buf_t r;
	r.data = (uint8_t *)p;
	r.size = (int)size;
	r.cursor = 0;
	return r;
}

#define truetype_buf_get16(b)	truetype_buf_get((b), 2)
#define truetype_buf_get32(b)	truetype_buf_get((b), 4)

static struct truetype_buf_t truetype_buf_range(const struct truetype_buf_t * b, int o, int s)
{
	struct truetype_buf_t r = truetype_new_buf(NULL, 0);
	if(o < 0 || s < 0 || o > b->size || s > b->size - o)
		return r;
	r.data = b->data + o;
	r.size = s;
	return r;
}

static struct truetype_buf_t truetype_cff_get_index(struct truetype_buf_t * b)
{
	int count, start, offsize;
	start = b->cursor;
	count = truetype_buf_get16(b);
	if(count)
	{
		offsize = truetype_buf_get8(b);
		truetype_buf_skip(b, offsize * count);
		truetype_buf_skip(b, truetype_buf_get(b, offsize) - 1);
	}
	return truetype_buf_range(b, start, b->cursor - start);
}

static uint32_t truetype_cff_int(struct truetype_buf_t * b)
{
	int b0 = truetype_buf_get8(b);
	if(b0 >= 32 && b0 <= 246)
		return b0 - 139;
	else if(b0 >= 247 && b0 <= 250)
		return (b0 - 247) * 256 + truetype_buf_get8(b) + 108;
	else if(b0 >= 251 && b0 <= 254)
		return -(b0 - 251) * 256 - truetype_buf_get8(b) - 108;
	else if(b0 == 28)
		return truetype_buf_get16(b);
	else if(b0 == 29)
		return truetype_buf_get32(b);
	return 0;
}

static void truetype_cff_skip_operand(struct truetype_buf_t * b)
{
	int v, b0 = truetype_buf_peek8(b);
	if(b0 == 30)
	{
		truetype_buf_skip(b, 1);
		while(b->cursor < b->size)
		{
			v = truetype_buf_get8(b);
			if((v & 0xF) == 0xF || (v >> 4) == 0xF)
				break;
		}
	}
	else
	{
		truetype_cff_int(b);
	}
}

static struct truetype_buf_t truetype_dict_get(struct truetype_buf_t * b, int key)
{
	truetype_buf_seek(b, 0);
	while(b->cursor < b->size)
	{
		int start = b->cursor, end, op;
		while(truetype_buf_peek8(b) >= 28)
			truetype_cff_skip_operand(b);
		end = b->cursor;
		op = truetype_buf_get8(b);
		if(op == 12)
			op = truetype_buf_get8(b) | 0x100;
		if(op == key)
			return truetype_buf_range(b, start, end - start);
	}
	return truetype_buf_range(b, 0, 0);
}

static void truetype_dict_get_ints(struct truetype_buf_t * b, int key, int outcount, uint32_t * out)
{
	struct truetype_buf_t operands = truetype_dict_get(b, key);
	for(int i = 0; i < outcount && operands.cursor < operands.size; i++)
		out[i] = truetype_cff_int(&operands);
}

static int truetype_cff_index_count(struct truetype_buf_t * b)
{
	truetype_buf_seek(b, 0);
	return truetype_buf_get16(b);
}

static struct truetype_buf_t truetype_cff_index_get(struct truetype_buf_t b, int i)
{
	int count, offsize, start, end;
	truetype_buf_seek(&b, 0);
	count = truetype_buf_get16(&b);
	offsize = truetype_buf_get8(&b);
	truetype_buf_skip(&b, i * offsize);
	start = truetype_buf_get(&b, offsize);
	end = truetype_buf_get(&b, offsize);
	return truetype_buf_range(&b, 2 + (count + 1) * offsize + start, end - start);
}

static inline uint16_t truetype_ushort(uint8_t * p)
{
	return p[0] * 256 + p[1];
}

static inline int16_t truetype_short(uint8_t * p)
{
	return p[0] * 256 + p[1];
}

static inline uint32_t truetype_ulong(uint8_t * p)
{
	return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
}

static uint32_t truetype_find_table(uint8_t * data, uint32_t fontstart, const char * tag)
{
	int32_t num_tables = truetype_ushort(data + fontstart + 4);
	uint32_t tabledir = fontstart + 12;
	int32_t i;
	for(i = 0; i < num_tables; ++i)
	{
		uint32_t loc = tabledir + 16 * i;
		if((data + loc + 0)[0] == (tag[0]) && (data + loc + 0)[1] == (tag[1]) && (data + loc + 0)[2] == (tag[2]) && (data + loc + 0)[3] == (tag[3]))
			return truetype_ulong(data + loc + 8);
	}
	return 0;
}

static struct truetype_buf_t truetype_get_subrs(struct truetype_buf_t cff, struct truetype_buf_t fontdict)
{
	uint32_t subrsoff = 0, private_loc[2] = { 0, 0 };
	struct truetype_buf_t pdict;
	truetype_dict_get_ints(&fontdict, 18, 2, private_loc);
	if(!private_loc[1] || !private_loc[0])
		return truetype_new_buf(NULL, 0);
	pdict = truetype_buf_range(&cff, private_loc[1], private_loc[0]);
	truetype_dict_get_ints(&pdict, 19, 1, &subrsoff);
	if(!subrsoff)
		return truetype_new_buf(NULL, 0);
	truetype_buf_seek(&cff, private_loc[1] + subrsoff);
	return truetype_cff_get_index(&cff);
}

static int truetype_init_font_internal(struct truetype_fontinfo_t * info, unsigned char * data, int fontstart)
{
	uint32_t cmap, t;
	int32_t i, numTables;

	info->data = data;
	info->fontstart = fontstart;
	info->cff = truetype_new_buf(NULL, 0);

	cmap = truetype_find_table(data, fontstart, "cmap");
	info->loca = truetype_find_table(data, fontstart, "loca");
	info->head = truetype_find_table(data, fontstart, "head");
	info->glyf = truetype_find_table(data, fontstart, "glyf");
	info->hhea = truetype_find_table(data, fontstart, "hhea");
	info->hmtx = truetype_find_table(data, fontstart, "hmtx");
	info->kern = truetype_find_table(data, fontstart, "kern");
	info->gpos = truetype_find_table(data, fontstart, "GPOS");

	if(!cmap || !info->head || !info->hhea || !info->hmtx)
		return 0;
	if(info->glyf)
	{
		if(!info->loca)
			return 0;
	}
	else
	{
		struct truetype_buf_t b, topdict, topdictidx;
		uint32_t cstype = 2, charstrings = 0, fdarrayoff = 0, fdselectoff = 0;
		uint32_t cff;

		cff = truetype_find_table(data, fontstart, "CFF ");
		if(!cff)
			return 0;

		info->fontdicts = truetype_new_buf(NULL, 0);
		info->fdselect = truetype_new_buf(NULL, 0);

		info->cff = truetype_new_buf(data + cff, 512 * 1024 * 1024);
		b = info->cff;

		truetype_buf_skip(&b, 2);
		truetype_buf_seek(&b, truetype_buf_get8(&b));

		truetype_cff_get_index(&b);
		topdictidx = truetype_cff_get_index(&b);
		topdict = truetype_cff_index_get(topdictidx, 0);
		truetype_cff_get_index(&b);
		info->gsubrs = truetype_cff_get_index(&b);

		truetype_dict_get_ints(&topdict, 17, 1, &charstrings);
		truetype_dict_get_ints(&topdict, 0x100 | 6, 1, &cstype);
		truetype_dict_get_ints(&topdict, 0x100 | 36, 1, &fdarrayoff);
		truetype_dict_get_ints(&topdict, 0x100 | 37, 1, &fdselectoff);
		info->subrs = truetype_get_subrs(b, topdict);

		if(cstype != 2)
			return 0;
		if(charstrings == 0)
			return 0;

		if(fdarrayoff)
		{
			if(!fdselectoff)
				return 0;
			truetype_buf_seek(&b, fdarrayoff);
			info->fontdicts = truetype_cff_get_index(&b);
			info->fdselect = truetype_buf_range(&b, fdselectoff, b.size - fdselectoff);
		}
		truetype_buf_seek(&b, charstrings);
		info->charstrings = truetype_cff_get_index(&b);
	}

	t = truetype_find_table(data, fontstart, "maxp");
	if(t)
		info->nglyphs = truetype_ushort(data + t + 4);
	else
		info->nglyphs = 0xffff;

	info->svg = -1;
	numTables = truetype_ushort(data + cmap + 2);
	info->index_map = 0;
	for(i = 0; i < numTables; ++i)
	{
		uint32_t encoding_record = cmap + 4 + 8 * i;
		switch(truetype_ushort(data + encoding_record))
		{
		case STBTT_PLATFORM_ID_MICROSOFT:
			switch(truetype_ushort(data + encoding_record + 2))
			{
			case STBTT_MS_EID_UNICODE_BMP:
			case STBTT_MS_EID_UNICODE_FULL:
				info->index_map = cmap + truetype_ulong(data + encoding_record + 4);
				break;
			}
			break;
		case STBTT_PLATFORM_ID_UNICODE:
			info->index_map = cmap + truetype_ulong(data + encoding_record + 4);
			break;
		}
	}
	if(info->index_map == 0)
		return 0;
	info->index_to_loc_format = truetype_ushort(data + info->head + 50);
	return 1;
}

int truetype_init(struct truetype_fontinfo_t * info, unsigned char * data, int offset)
{
	return truetype_init_font_internal(info, (unsigned char *)data, offset);
}

int truetrype_find_glyph_index(struct truetype_fontinfo_t * info, int unicode)
{
	uint8_t * data = info->data;
	uint32_t index_map = info->index_map;
	uint16_t format = truetype_ushort(data + index_map + 0);
	if(format == 0)
	{
		int32_t bytes = truetype_ushort(data + index_map + 2);
		if(unicode < bytes - 6)
			return (*(uint8_t *)(data + index_map + 6 + unicode));
		return 0;
	}
	else if(format == 6)
	{
		uint32_t first = truetype_ushort(data + index_map + 6);
		uint32_t count = truetype_ushort(data + index_map + 8);
		if((uint32_t)unicode >= first && (uint32_t)unicode < first + count)
			return truetype_ushort(data + index_map + 10 + (unicode - first) * 2);
		return 0;
	}
	else if(format == 2)
	{
		return 0;
	}
	else if(format == 4)
	{
		uint16_t segcount = truetype_ushort(data + index_map + 6) >> 1;
		uint16_t searchRange = truetype_ushort(data + index_map + 8) >> 1;
		uint16_t entrySelector = truetype_ushort(data + index_map + 10);
		uint16_t rangeShift = truetype_ushort(data + index_map + 12) >> 1;
		uint32_t endCount = index_map + 14;
		uint32_t search = endCount;

		if(unicode > 0xffff)
			return 0;
		if(unicode >= truetype_ushort(data + search + rangeShift * 2))
			search += rangeShift * 2;
		search -= 2;
		while(entrySelector)
		{
			uint16_t end;
			searchRange >>= 1;
			end = truetype_ushort(data + search + searchRange * 2);
			if(unicode > end)
				search += searchRange * 2;
			--entrySelector;
		}
		search += 2;
		{
			uint16_t offset, start, last;
			uint16_t item = (uint16_t)((search - endCount) >> 1);

			start = truetype_ushort(data + index_map + 14 + segcount * 2 + 2 + 2 * item);
			last = truetype_ushort(data + endCount + 2 * item);
			if(unicode < start || unicode > last)
				return 0;
			offset = truetype_ushort(data + index_map + 14 + segcount * 6 + 2 + 2 * item);
			if(offset == 0)
				return (uint16_t)(unicode + truetype_short(data + index_map + 14 + segcount * 4 + 2 + 2 * item));
			return truetype_ushort(data + offset + (unicode - start) * 2 + index_map + 14 + segcount * 6 + 2 + 2 * item);
		}
	}
	else if(format == 12 || format == 13)
	{
		uint32_t ngroups = truetype_ulong(data + index_map + 12);
		int32_t low, high;
		low = 0;
		high = (int32_t)ngroups;
		while(low < high)
		{
			int32_t mid = low + ((high - low) >> 1);
			uint32_t start_char = truetype_ulong(data + index_map + 16 + mid * 12);
			uint32_t end_char = truetype_ulong(data + index_map + 16 + mid * 12 + 4);
			if((uint32_t)unicode < start_char)
				high = mid;
			else if((uint32_t)unicode > end_char)
				low = mid + 1;
			else
			{
				uint32_t start_glyph = truetype_ulong(data + index_map + 16 + mid * 12 + 8);
				if(format == 12)
					return start_glyph + unicode - start_char;
				else
					return start_glyph;
			}
		}
		return 0;
	}
	return 0;
}

static void truetype_setvertex(struct truetype_vertex_t * v, uint8_t type, int32_t x, int32_t y, int32_t cx, int32_t cy)
{
	v->type = type;
	v->x = (int16_t)x;
	v->y = (int16_t)y;
	v->cx = (int16_t)cx;
	v->cy = (int16_t)cy;
}

static int truetype_get_gly_offset(struct truetype_fontinfo_t * info, int glyph_index)
{
	int g1, g2;

	if(glyph_index >= info->nglyphs)
		return -1;
	if(info->index_to_loc_format >= 2)
		return -1;
	if(info->index_to_loc_format == 0)
	{
		g1 = info->glyf + truetype_ushort(info->data + info->loca + glyph_index * 2) * 2;
		g2 = info->glyf + truetype_ushort(info->data + info->loca + glyph_index * 2 + 2) * 2;
	}
	else
	{
		g1 = info->glyf + truetype_ulong(info->data + info->loca + glyph_index * 4);
		g2 = info->glyf + truetype_ulong(info->data + info->loca + glyph_index * 4 + 4);
	}
	return g1 == g2 ? -1 : g1;
}

static int truetype_get_glyph_info_t2(struct truetype_fontinfo_t * info, int glyph_index, int * x0, int * y0, int * x1, int * y1);

static int truetype_get_glyph_box(struct truetype_fontinfo_t * info, int glyph_index, int * x0, int * y0, int * x1, int * y1)
{
	if(info->cff.size)
	{
		truetype_get_glyph_info_t2(info, glyph_index, x0, y0, x1, y1);
	}
	else
	{
		int g = truetype_get_gly_offset(info, glyph_index);
		if(g < 0)
			return 0;

		if(x0)
			*x0 = truetype_short(info->data + g + 2);
		if(y0)
			*y0 = truetype_short(info->data + g + 4);
		if(x1)
			*x1 = truetype_short(info->data + g + 6);
		if(y1)
			*y1 = truetype_short(info->data + g + 8);
	}
	return 1;
}

static int truetype_close_shape(struct truetype_vertex_t * vertices, int num_vertices, int was_off, int start_off, int32_t sx, int32_t sy, int32_t scx, int32_t scy, int32_t cx, int32_t cy)
{
	if(start_off)
	{
		if(was_off)
			truetype_setvertex(&vertices[num_vertices++], TRUETYPE_VCURVE, (cx + scx) >> 1, (cy + scy) >> 1, cx, cy);
		truetype_setvertex(&vertices[num_vertices++], TRUETYPE_VCURVE, sx, sy, scx, scy);
	}
	else
	{
		if(was_off)
			truetype_setvertex(&vertices[num_vertices++], TRUETYPE_VCURVE, sx, sy, cx, cy);
		else
			truetype_setvertex(&vertices[num_vertices++], TRUETYPE_VLINE, sx, sy, 0, 0);
	}
	return num_vertices;
}

static int truetype_get_glyph_shape(struct truetype_fontinfo_t * info, int glyph_index, struct truetype_vertex_t ** pvertices);

static int truetype_get_glyph_shape_tt(struct truetype_fontinfo_t * info, int glyph_index, struct truetype_vertex_t ** pvertices)
{
	int16_t numberOfContours;
	uint8_t * endPtsOfContours;
	uint8_t * data = info->data;
	struct truetype_vertex_t * vertices = 0;
	int num_vertices = 0;
	int g = truetype_get_gly_offset(info, glyph_index);

	*pvertices = NULL;

	if(g < 0)
		return 0;

	numberOfContours = truetype_short(data + g);
	if(numberOfContours > 0)
	{
		uint8_t flags = 0, flagcount;
		int32_t ins, i, j = 0, m, n, next_move, was_off = 0, off, start_off = 0;
		int32_t x, y, cx, cy, sx, sy, scx, scy;
		uint8_t * points;
		endPtsOfContours = (data + g + 10);
		ins = truetype_ushort(data + g + 10 + numberOfContours * 2);
		points = data + g + 10 + numberOfContours * 2 + 2 + ins;
		n = 1 + truetype_ushort(endPtsOfContours + numberOfContours * 2 - 2);
		m = n + 2 * numberOfContours;
		vertices = (struct truetype_vertex_t *)xos_mem_malloc(m * sizeof(vertices[0]));
		if(vertices == 0)
			return 0;

		next_move = 0;
		flagcount = 0;
		off = m - n;

		for(i = 0; i < n; ++i)
		{
			if(flagcount == 0)
			{
				flags = *points++;
				if(flags & 8)
					flagcount = *points++;
			}
			else
				--flagcount;
			vertices[off + i].type = flags;
		}

		x = 0;
		for(i = 0; i < n; ++i)
		{
			flags = vertices[off + i].type;
			if(flags & 2)
			{
				int16_t dx = *points++;
				x += (flags & 16) ? dx : -dx;
			}
			else
			{
				if(!(flags & 16))
				{
					x = x + (int16_t)(points[0] * 256 + points[1]);
					points += 2;
				}
			}
			vertices[off + i].x = (int16_t)x;
		}

		y = 0;
		for(i = 0; i < n; ++i)
		{
			flags = vertices[off + i].type;
			if(flags & 4)
			{
				int16_t dy = *points++;
				y += (flags & 32) ? dy : -dy;
			}
			else
			{
				if(!(flags & 32))
				{
					y = y + (int16_t)(points[0] * 256 + points[1]);
					points += 2;
				}
			}
			vertices[off + i].y = (int16_t)y;
		}

		num_vertices = 0;
		sx = sy = cx = cy = scx = scy = 0;
		for(i = 0; i < n; ++i)
		{
			flags = vertices[off + i].type;
			x = (int16_t)vertices[off + i].x;
			y = (int16_t)vertices[off + i].y;

			if(next_move == i)
			{
				if(i != 0)
					num_vertices = truetype_close_shape(
						vertices, num_vertices, was_off, start_off, sx, sy, scx, scy, cx, cy);

				start_off = !(flags & 1);
				if(start_off)
				{
					scx = x;
					scy = y;
					if(!(vertices[off + i + 1].type & 1))
					{
						sx = (x + (int32_t)vertices[off + i + 1].x) >> 1;
						sy = (y + (int32_t)vertices[off + i + 1].y) >> 1;
					}
					else
					{
						sx = (int32_t)vertices[off + i + 1].x;
						sy = (int32_t)vertices[off + i + 1].y;
						++i;
					}
				}
				else
				{
					sx = x;
					sy = y;
				}
				truetype_setvertex(&vertices[num_vertices++], TRUETYPE_VMOVE, sx, sy, 0, 0);
				was_off = 0;
				next_move = 1 + truetype_ushort(endPtsOfContours + j * 2);
				++j;
			}
			else
			{
				if(!(flags & 1))
				{
					if(was_off)
						truetype_setvertex(&vertices[num_vertices++], TRUETYPE_VCURVE, (cx + x) >> 1, (cy + y) >> 1, cx, cy);
					cx = x;
					cy = y;
					was_off = 1;
				}
				else
				{
					if(was_off)
						truetype_setvertex(&vertices[num_vertices++], TRUETYPE_VCURVE, x, y, cx, cy);
					else
						truetype_setvertex(&vertices[num_vertices++], TRUETYPE_VLINE, x, y, 0, 0);
					was_off = 0;
				}
			}
		}
		num_vertices = truetype_close_shape(vertices, num_vertices, was_off, start_off, sx, sy, scx, scy, cx, cy);
	}
	else if(numberOfContours < 0)
	{
		int more = 1;
		uint8_t * comp = data + g + 10;
		num_vertices = 0;
		vertices = 0;
		while(more)
		{
			uint16_t flags, gidx;
			int comp_num_verts = 0, i;
			struct truetype_vertex_t *comp_verts = 0, *tmp = 0;
			float mtx[6] = { 1, 0, 0, 1, 0, 0 }, m, n;

			flags = truetype_short(comp);
			comp += 2;
			gidx = truetype_short(comp);
			comp += 2;

			if(flags & 2)
			{
				if(flags & 1)
				{
					mtx[4] = truetype_short(comp);
					comp += 2;
					mtx[5] = truetype_short(comp);
					comp += 2;
				}
				else
				{
					mtx[4] = (*(int8_t *)(comp));
					comp += 1;
					mtx[5] = (*(int8_t *)(comp));
					comp += 1;
				}
			}
			else
			{
			}
			if(flags & (1 << 3))
			{
				mtx[0] = mtx[3] = truetype_short(comp) / 16384.0f;
				comp += 2;
				mtx[1] = mtx[2] = 0;
			}
			else if(flags & (1 << 6))
			{
				mtx[0] = truetype_short(comp) / 16384.0f;
				comp += 2;
				mtx[1] = mtx[2] = 0;
				mtx[3] = truetype_short(comp) / 16384.0f;
				comp += 2;
			}
			else if(flags & (1 << 7))
			{
				mtx[0] = truetype_short(comp) / 16384.0f;
				comp += 2;
				mtx[1] = truetype_short(comp) / 16384.0f;
				comp += 2;
				mtx[2] = truetype_short(comp) / 16384.0f;
				comp += 2;
				mtx[3] = truetype_short(comp) / 16384.0f;
				comp += 2;
			}

			m = (float)sqrtf(mtx[0] * mtx[0] + mtx[1] * mtx[1]);
			n = (float)sqrtf(mtx[2] * mtx[2] + mtx[3] * mtx[3]);

			comp_num_verts = truetype_get_glyph_shape(info, gidx, &comp_verts);
			if(comp_num_verts > 0)
			{
				for(i = 0; i < comp_num_verts; ++i)
				{
					struct truetype_vertex_t * v = &comp_verts[i];
					short int x, y;
					x = v->x;
					y = v->y;
					v->x = (short int)(m * (mtx[0] * x + mtx[2] * y + mtx[4]));
					v->y = (short int)(n * (mtx[1] * x + mtx[3] * y + mtx[5]));
					x = v->cx;
					y = v->cy;
					v->cx = (short int)(m * (mtx[0] * x + mtx[2] * y + mtx[4]));
					v->cy = (short int)(n * (mtx[1] * x + mtx[3] * y + mtx[5]));
				}
				tmp = (struct truetype_vertex_t *)xos_mem_malloc((num_vertices + comp_num_verts) * sizeof(struct truetype_vertex_t));
				if(!tmp)
				{
					if(vertices)
						xos_mem_free(vertices);
					if(comp_verts)
						xos_mem_free(comp_verts);
					return 0;
				}
				if(num_vertices > 0 && vertices)
					xos_memcpy(tmp, vertices, num_vertices * sizeof(struct truetype_vertex_t));
				xos_memcpy(tmp + num_vertices, comp_verts, comp_num_verts * sizeof(struct truetype_vertex_t));
				if(vertices)
					xos_mem_free(vertices);
				vertices = tmp;
				xos_mem_free(comp_verts);
				num_vertices += comp_num_verts;
			}
			more = flags & (1 << 5);
		}
	}
	else
	{
	}
	*pvertices = vertices;
	return num_vertices;
}

#define TRUETYPE_CSCTX_INIT(bounds)		{bounds, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0}

static void truetype_track_vertex(struct truetype_csctx_t * c, int32_t x, int32_t y)
{
	if(x > c->max_x || !c->started)
		c->max_x = x;
	if(y > c->max_y || !c->started)
		c->max_y = y;
	if(x < c->min_x || !c->started)
		c->min_x = x;
	if(y < c->min_y || !c->started)
		c->min_y = y;
	c->started = 1;
}

static void truetype_csctx_v(struct truetype_csctx_t * c, uint8_t type, int32_t x, int32_t y, int32_t cx, int32_t cy, int32_t cx1, int32_t cy1)
{
	if(c->bounds)
	{
		truetype_track_vertex(c, x, y);
		if(type == TRUETYPE_VCUBIC)
		{
			truetype_track_vertex(c, cx, cy);
			truetype_track_vertex(c, cx1, cy1);
		}
	}
	else
	{
		truetype_setvertex(&c->pvertices[c->num_vertices], type, x, y, cx, cy);
		c->pvertices[c->num_vertices].cx1 = (int16_t)cx1;
		c->pvertices[c->num_vertices].cy1 = (int16_t)cy1;
	}
	c->num_vertices++;
}

static void truetype_csctx_close_shape(struct truetype_csctx_t * ctx)
{
	if(ctx->first_x != ctx->x || ctx->first_y != ctx->y)
		truetype_csctx_v(ctx, TRUETYPE_VLINE, (int)ctx->first_x, (int)ctx->first_y, 0, 0, 0, 0);
}

static void truetype_csctx_rmove_to(struct truetype_csctx_t * ctx, float dx, float dy)
{
	truetype_csctx_close_shape(ctx);
	ctx->first_x = ctx->x = ctx->x + dx;
	ctx->first_y = ctx->y = ctx->y + dy;
	truetype_csctx_v(ctx, TRUETYPE_VMOVE, (int)ctx->x, (int)ctx->y, 0, 0, 0, 0);
}

static void truetype_csctx_rline_to(struct truetype_csctx_t * ctx, float dx, float dy)
{
	ctx->x += dx;
	ctx->y += dy;
	truetype_csctx_v(ctx, TRUETYPE_VLINE, (int)ctx->x, (int)ctx->y, 0, 0, 0, 0);
}

static void truetype_csctx_rccurve_to(struct truetype_csctx_t * ctx, float dx1, float dy1, float dx2, float dy2, float dx3, float dy3)
{
	float cx1 = ctx->x + dx1;
	float cy1 = ctx->y + dy1;
	float cx2 = cx1 + dx2;
	float cy2 = cy1 + dy2;
	ctx->x = cx2 + dx3;
	ctx->y = cy2 + dy3;
	truetype_csctx_v(ctx, TRUETYPE_VCUBIC, (int)ctx->x, (int)ctx->y, (int)cx1, (int)cy1, (int)cx2, (int)cy2);
}

static struct truetype_buf_t truetype_get_subr(struct truetype_buf_t idx, int n)
{
	int count = truetype_cff_index_count(&idx);
	int bias = 107;
	if(count >= 33900)
		bias = 32768;
	else if(count >= 1240)
		bias = 1131;
	n += bias;
	if(n < 0 || n >= count)
		return truetype_new_buf(NULL, 0);
	return truetype_cff_index_get(idx, n);
}

static struct truetype_buf_t truetype_cid_get_glyph_subrs(struct truetype_fontinfo_t * info, int glyph_index)
{
	struct truetype_buf_t fdselect = info->fdselect;
	int nranges, start, end, v, fmt, fdselector = -1, i;

	truetype_buf_seek(&fdselect, 0);
	fmt = truetype_buf_get8(&fdselect);
	if(fmt == 0)
	{
		truetype_buf_skip(&fdselect, glyph_index);
		fdselector = truetype_buf_get8(&fdselect);
	}
	else if(fmt == 3)
	{
		nranges = truetype_buf_get16(&fdselect);
		start = truetype_buf_get16(&fdselect);
		for(i = 0; i < nranges; i++)
		{
			v = truetype_buf_get8(&fdselect);
			end = truetype_buf_get16(&fdselect);
			if(glyph_index >= start && glyph_index < end)
			{
				fdselector = v;
				break;
			}
			start = end;
		}
	}
	if(fdselector == -1)
		truetype_new_buf(NULL, 0);
	return truetype_get_subrs(info->cff, truetype_cff_index_get(info->fontdicts, fdselector));
}

static int truetype_run_charstring(struct truetype_fontinfo_t * info, int glyph_index, struct truetype_csctx_t * c)
{
	int in_header = 1, maskbits = 0, subr_stack_height = 0, sp = 0, v, i, b0;
	int has_subrs = 0, clear_stack;
	float s[48];
	struct truetype_buf_t subr_stack[10], subrs = info->subrs, b;
	float f;

	b = truetype_cff_index_get(info->charstrings, glyph_index);
	while(b.cursor < b.size)
	{
		i = 0;
		clear_stack = 1;
		b0 = truetype_buf_get8(&b);
		switch(b0)
		{
		case 0x13:
		case 0x14:
			if(in_header)
				maskbits += (sp / 2);
			in_header = 0;
			truetype_buf_skip(&b, (maskbits + 7) / 8);
			break;

		case 0x01:
		case 0x03:
		case 0x12:
		case 0x17:
			maskbits += (sp / 2);
			break;

		case 0x15:
			in_header = 0;
			if(sp < 2)
				return 0;
			truetype_csctx_rmove_to(c, s[sp - 2], s[sp - 1]);
			break;
		case 0x04:
			in_header = 0;
			if(sp < 1)
				return 0;
			truetype_csctx_rmove_to(c, 0, s[sp - 1]);
			break;
		case 0x16:
			in_header = 0;
			if(sp < 1)
				return 0;
			truetype_csctx_rmove_to(c, s[sp - 1], 0);
			break;

		case 0x05:
			if(sp < 2)
				return 0;
			for(; i + 1 < sp; i += 2)
				truetype_csctx_rline_to(c, s[i], s[i + 1]);
			break;

		case 0x07:
			if(sp < 1)
				return 0;
			goto vlineto;
		case 0x06:
			if(sp < 1)
				return 0;
			for(;;)
			{
				if(i >= sp)
					break;
				truetype_csctx_rline_to(c, s[i], 0);
				i++;
			vlineto:
				if(i >= sp)
					break;
				truetype_csctx_rline_to(c, 0, s[i]);
				i++;
			}
			break;

		case 0x1F:
			if(sp < 4)
				return 0;
			goto hvcurveto;
		case 0x1E:
			if(sp < 4)
				return 0;
			for(;;)
			{
				if(i + 3 >= sp)
					break;
				truetype_csctx_rccurve_to(c, 0, s[i], s[i + 1], s[i + 2], s[i + 3], (sp - i == 5) ? s[i + 4] : 0.0f);
				i += 4;
hvcurveto:
				if(i + 3 >= sp)
					break;
				truetype_csctx_rccurve_to(c, s[i], 0, s[i + 1], s[i + 2], (sp - i == 5) ? s[i + 4] : 0.0f, s[i + 3]);
				i += 4;
			}
			break;

		case 0x08:
			if(sp < 6)
				return 0;
			for(; i + 5 < sp; i += 6)
				truetype_csctx_rccurve_to(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5]);
			break;

		case 0x18:
			if(sp < 8)
				return 0;
			for(; i + 5 < sp - 2; i += 6)
				truetype_csctx_rccurve_to(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5]);
			if(i + 1 >= sp)
				return 0;
			truetype_csctx_rline_to(c, s[i], s[i + 1]);
			break;

		case 0x19:
			if(sp < 8)
				return 0;
			for(; i + 1 < sp - 6; i += 2)
				truetype_csctx_rline_to(c, s[i], s[i + 1]);
			if(i + 5 >= sp)
				return 0;
			truetype_csctx_rccurve_to(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5]);
			break;

		case 0x1A:
		case 0x1B:
			if(sp < 4)
				return 0;
			f = 0.0;
			if(sp & 1)
			{
				f = s[i];
				i++;
			}
			for(; i + 3 < sp; i += 4)
			{
				if(b0 == 0x1B)
					truetype_csctx_rccurve_to(c, s[i], f, s[i + 1], s[i + 2], s[i + 3], 0.0);
				else
					truetype_csctx_rccurve_to(c, f, s[i], s[i + 1], s[i + 2], 0.0, s[i + 3]);
				f = 0.0;
			}
			break;

		case 0x0A:
			if(!has_subrs)
			{
				if(info->fdselect.size)
					subrs = truetype_cid_get_glyph_subrs(info, glyph_index);
				has_subrs = 1;
			}
		case 0x1D:
			if(sp < 1)
				return 0;
			v = (int)s[--sp];
			if(subr_stack_height >= 10)
				return 0;
			subr_stack[subr_stack_height++] = b;
			b = truetype_get_subr(b0 == 0x0A ? subrs : info->gsubrs, v);
			if(b.size == 0)
				return 0;
			b.cursor = 0;
			clear_stack = 0;
			break;

		case 0x0B:
			if(subr_stack_height <= 0)
				return 0;
			b = subr_stack[--subr_stack_height];
			clear_stack = 0;
			break;

		case 0x0E:
			truetype_csctx_close_shape(c);
			return 1;

		case 0x0C:
		{
			float dx1, dx2, dx3, dx4, dx5, dx6, dy1, dy2, dy3, dy4, dy5, dy6;
			float dx, dy;
			int b1 = truetype_buf_get8(&b);
			switch(b1)
			{
			case 0x22:
				if(sp < 7)
					return 0;
				dx1 = s[0];
				dx2 = s[1];
				dy2 = s[2];
				dx3 = s[3];
				dx4 = s[4];
				dx5 = s[5];
				dx6 = s[6];
				truetype_csctx_rccurve_to(c, dx1, 0, dx2, dy2, dx3, 0);
				truetype_csctx_rccurve_to(c, dx4, 0, dx5, -dy2, dx6, 0);
				break;

			case 0x23:
				if(sp < 13)
					return 0;
				dx1 = s[0];
				dy1 = s[1];
				dx2 = s[2];
				dy2 = s[3];
				dx3 = s[4];
				dy3 = s[5];
				dx4 = s[6];
				dy4 = s[7];
				dx5 = s[8];
				dy5 = s[9];
				dx6 = s[10];
				dy6 = s[11];
				truetype_csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
				truetype_csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
				break;

			case 0x24:
				if(sp < 9)
					return 0;
				dx1 = s[0];
				dy1 = s[1];
				dx2 = s[2];
				dy2 = s[3];
				dx3 = s[4];
				dx4 = s[5];
				dx5 = s[6];
				dy5 = s[7];
				dx6 = s[8];
				truetype_csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, 0);
				truetype_csctx_rccurve_to(c, dx4, 0, dx5, dy5, dx6, -(dy1 + dy2 + dy5));
				break;

			case 0x25:
				if(sp < 11)
					return 0;
				dx1 = s[0];
				dy1 = s[1];
				dx2 = s[2];
				dy2 = s[3];
				dx3 = s[4];
				dy3 = s[5];
				dx4 = s[6];
				dy4 = s[7];
				dx5 = s[8];
				dy5 = s[9];
				dx6 = dy6 = s[10];
				dx = dx1 + dx2 + dx3 + dx4 + dx5;
				dy = dy1 + dy2 + dy3 + dy4 + dy5;
				if(fabsf(dx) > fabsf(dy))
					dy6 = -dy;
				else
					dx6 = -dx;
				truetype_csctx_rccurve_to(c, dx1, dy1, dx2, dy2, dx3, dy3);
				truetype_csctx_rccurve_to(c, dx4, dy4, dx5, dy5, dx6, dy6);
				break;

			default:
				return 0;
			}
		}
		break;

		default:
			if(b0 != 255 && b0 != 28 && b0 < 32)
				return 0;
			if(b0 == 255)
			{
				f = (float)(int32_t)truetype_buf_get32(&b) / 0x10000;
			}
			else
			{
				truetype_buf_skip(&b, -1);
				f = (float)(int16_t)truetype_cff_int(&b);
			}
			if(sp >= 48)
				return 0;
			s[sp++] = f;
			clear_stack = 0;
			break;
		}
		if(clear_stack)
			sp = 0;
	}
	return 0;
}

static int truetype_get_glyph_shape_t2(struct truetype_fontinfo_t * info, int glyph_index, struct truetype_vertex_t ** pvertices)
{
	struct truetype_csctx_t count_ctx = TRUETYPE_CSCTX_INIT(1);
	struct truetype_csctx_t output_ctx = TRUETYPE_CSCTX_INIT(0);
	if(truetype_run_charstring(info, glyph_index, &count_ctx))
	{
		*pvertices = (struct truetype_vertex_t *)xos_mem_malloc(count_ctx.num_vertices * sizeof(struct truetype_vertex_t));
		output_ctx.pvertices = *pvertices;
		if(truetype_run_charstring(info, glyph_index, &output_ctx))
			return output_ctx.num_vertices;
	}
	*pvertices = NULL;
	return 0;
}

static int truetype_get_glyph_info_t2(struct truetype_fontinfo_t * info, int glyph_index, int * x0, int * y0, int * x1, int * y1)
{
	struct truetype_csctx_t c = TRUETYPE_CSCTX_INIT(1);
	int r = truetype_run_charstring(info, glyph_index, &c);
	if(x0)
		*x0 = r ? c.min_x : 0;
	if(y0)
		*y0 = r ? c.min_y : 0;
	if(x1)
		*x1 = r ? c.max_x : 0;
	if(y1)
		*y1 = r ? c.max_y : 0;
	return r ? c.num_vertices : 0;
}

static int truetype_get_glyph_shape(struct truetype_fontinfo_t * info, int glyph_index, struct truetype_vertex_t ** pvertices)
{
	if(!info->cff.size)
		return truetype_get_glyph_shape_tt(info, glyph_index, pvertices);
	else
		return truetype_get_glyph_shape_t2(info, glyph_index, pvertices);
}

void truetype_get_glyph_hmetrics(struct truetype_fontinfo_t * info, int glyph, int * advance, int * lsb)
{
	uint16_t numOfLongHorMetrics = truetype_ushort(info->data + info->hhea + 34);
	if(glyph < numOfLongHorMetrics)
	{
		if(advance)
			*advance = truetype_short(info->data + info->hmtx + 4 * glyph);
		if(lsb)
			*lsb = truetype_short(info->data + info->hmtx + 4 * glyph + 2);
	}
	else
	{
		if(advance)
			*advance = truetype_short(info->data + info->hmtx + 4 * (numOfLongHorMetrics - 1));
		if(lsb)
			*lsb = truetype_short(info->data + info->hmtx + 4 * numOfLongHorMetrics + 2 * (glyph - numOfLongHorMetrics));
	}
}

static int truetype_get_glyph_kern_info_advance(struct truetype_fontinfo_t * info, int glyph1, int glyph2)
{
	uint8_t * data = info->data + info->kern;
	uint32_t needle, straw;
	int l, r, m;

	if(!info->kern)
		return 0;
	if(truetype_ushort(data + 2) < 1)
		return 0;
	if(truetype_ushort(data + 8) != 1)
		return 0;

	l = 0;
	r = truetype_ushort(data + 10) - 1;
	needle = glyph1 << 16 | glyph2;
	while(l <= r)
	{
		m = (l + r) >> 1;
		straw = truetype_ulong(data + 18 + (m * 6));
		if(needle < straw)
			r = m - 1;
		else if(needle > straw)
			l = m + 1;
		else
			return truetype_short(data + 22 + (m * 6));
	}
	return 0;
}

static int32_t truetype_get_coverage_index(uint8_t * coverageTable, int glyph)
{
	uint16_t coverageFormat = truetype_ushort(coverageTable);
	switch(coverageFormat)
	{
	case 1:
	{
		uint16_t glyphCount = truetype_ushort(coverageTable + 2);
		int32_t l = 0, r = glyphCount - 1, m;
		int straw, needle = glyph;
		while(l <= r)
		{
			uint8_t * glyphArray = coverageTable + 4;
			uint16_t glyphID;
			m = (l + r) >> 1;
			glyphID = truetype_ushort(glyphArray + 2 * m);
			straw = glyphID;
			if(needle < straw)
				r = m - 1;
			else if(needle > straw)
				l = m + 1;
			else
			{
				return m;
			}
		}
		break;
	}
	case 2:
	{
		uint16_t rangeCount = truetype_ushort(coverageTable + 2);
		uint8_t * rangeArray = coverageTable + 4;
		int32_t l = 0, r = rangeCount - 1, m;
		int strawStart, strawEnd, needle = glyph;
		while(l <= r)
		{
			uint8_t * rangeRecord;
			m = (l + r) >> 1;
			rangeRecord = rangeArray + 6 * m;
			strawStart = truetype_ushort(rangeRecord);
			strawEnd = truetype_ushort(rangeRecord + 2);
			if(needle < strawStart)
				r = m - 1;
			else if(needle > strawEnd)
				l = m + 1;
			else
			{
				uint16_t startCoverageIndex = truetype_ushort(rangeRecord + 4);
				return startCoverageIndex + glyph - strawStart;
			}
		}
		break;
	}
	default:
		return -1;
	}
	return -1;
}

static int32_t true_get_glyph_class(uint8_t * classDefTable, int glyph)
{
	uint16_t classDefFormat = truetype_ushort(classDefTable);
	switch(classDefFormat)
	{
	case 1:
	{
		uint16_t startGlyphID = truetype_ushort(classDefTable + 2);
		uint16_t glyphCount = truetype_ushort(classDefTable + 4);
		uint8_t * classDef1ValueArray = classDefTable + 6;
		if(glyph >= startGlyphID && glyph < startGlyphID + glyphCount)
			return (int32_t)truetype_ushort(classDef1ValueArray + 2 * (glyph - startGlyphID));
		break;
	}
	case 2:
	{
		uint16_t classRangeCount = truetype_ushort(classDefTable + 2);
		uint8_t * classRangeRecords = classDefTable + 4;
		int32_t l = 0, r = classRangeCount - 1, m;
		int strawStart, strawEnd, needle = glyph;
		while(l <= r)
		{
			uint8_t * classRangeRecord;
			m = (l + r) >> 1;
			classRangeRecord = classRangeRecords + 6 * m;
			strawStart = truetype_ushort(classRangeRecord);
			strawEnd = truetype_ushort(classRangeRecord + 2);
			if(needle < strawStart)
				r = m - 1;
			else if(needle > strawEnd)
				l = m + 1;
			else
				return (int32_t)truetype_ushort(classRangeRecord + 4);
		}
		break;
	}
	default:
		return -1;
	}
	return 0;
}

static int32_t truetype_get_glyph_gpos_info_advance(struct truetype_fontinfo_t * info, int glyph1, int glyph2)
{
	uint16_t lookupListOffset;
	uint8_t * lookupList;
	uint16_t lookupCount;
	uint8_t * data;
	int32_t i, sti;

	if(!info->gpos)
		return 0;
	data = info->data + info->gpos;
	if(truetype_ushort(data + 0) != 1)
		return 0;
	if(truetype_ushort(data + 2) != 0)
		return 0;

	lookupListOffset = truetype_ushort(data + 8);
	lookupList = data + lookupListOffset;
	lookupCount = truetype_ushort(lookupList);

	for(i = 0; i < lookupCount; ++i)
	{
		uint16_t lookupOffset = truetype_ushort(lookupList + 2 + 2 * i);
		uint8_t * lookupTable = lookupList + lookupOffset;

		uint16_t lookupType = truetype_ushort(lookupTable);
		uint16_t subTableCount = truetype_ushort(lookupTable + 4);
		uint8_t * subTableOffsets = lookupTable + 6;
		if(lookupType != 2)
			continue;

		for(sti = 0; sti < subTableCount; sti++)
		{
			uint16_t subtableOffset = truetype_ushort(subTableOffsets + 2 * sti);
			uint8_t * table = lookupTable + subtableOffset;
			uint16_t posFormat = truetype_ushort(table);
			uint16_t coverageOffset = truetype_ushort(table + 2);
			int32_t coverageIndex = truetype_get_coverage_index(table + coverageOffset, glyph1);
			if(coverageIndex == -1)
				continue;

			switch(posFormat)
			{
			case 1:
			{
				int32_t l, r, m;
				int straw, needle;
				uint16_t valueFormat1 = truetype_ushort(table + 4);
				uint16_t valueFormat2 = truetype_ushort(table + 6);
				if(valueFormat1 == 4 && valueFormat2 == 0)
				{
					int32_t valueRecordPairSizeInBytes = 2;
					uint16_t pairSetCount = truetype_ushort(table + 8);
					uint16_t pairPosOffset = truetype_ushort(table + 10 + 2 * coverageIndex);
					uint8_t * pairValueTable = table + pairPosOffset;
					uint16_t pairValueCount = truetype_ushort(pairValueTable);
					uint8_t * pairValueArray = pairValueTable + 2;

					if(coverageIndex >= pairSetCount)
						return 0;

					needle = glyph2;
					r = pairValueCount - 1;
					l = 0;

					while(l <= r)
					{
						uint16_t secondGlyph;
						uint8_t * pairValue;
						m = (l + r) >> 1;
						pairValue = pairValueArray + (2 + valueRecordPairSizeInBytes) * m;
						secondGlyph = truetype_ushort(pairValue);
						straw = secondGlyph;
						if(needle < straw)
							r = m - 1;
						else if(needle > straw)
							l = m + 1;
						else
						{
							int16_t xAdvance = truetype_short(pairValue + 2);
							return xAdvance;
						}
					}
				}
				else
					return 0;
				break;
			}
			case 2:
			{
				uint16_t valueFormat1 = truetype_ushort(table + 4);
				uint16_t valueFormat2 = truetype_ushort(table + 6);
				if(valueFormat1 == 4 && valueFormat2 == 0)
				{
					uint16_t classDef1Offset = truetype_ushort(table + 8);
					uint16_t classDef2Offset = truetype_ushort(table + 10);
					int glyph1class = true_get_glyph_class(table + classDef1Offset, glyph1);
					int glyph2class = true_get_glyph_class(table + classDef2Offset, glyph2);

					uint16_t class1Count = truetype_ushort(table + 12);
					uint16_t class2Count = truetype_ushort(table + 14);
					uint8_t *class1Records, *class2Records;
					int16_t xAdvance;

					if(glyph1class < 0 || glyph1class >= class1Count)
						return 0;
					if(glyph2class < 0 || glyph2class >= class2Count)
						return 0;
					class1Records = table + 16;
					class2Records = class1Records + 2 * (glyph1class * class2Count);
					xAdvance = truetype_short(class2Records + 2 * glyph2class);
					return xAdvance;
				}
				else
					return 0;
				break;
			}
			default:
				return 0;
			}
		}
	}
	return 0;
}

int truetype_get_glyph_kern_advance(struct truetype_fontinfo_t * info, int g1, int g2)
{
	int xadvance = 0;
	if(info->gpos)
		xadvance += truetype_get_glyph_gpos_info_advance(info, g1, g2);
	else if(info->kern)
		xadvance += truetype_get_glyph_kern_info_advance(info, g1, g2);
	return xadvance;
}

void turetype_get_font_vmetrics(struct truetype_fontinfo_t * info, int * ascent, int * descent, int * linegap)
{
	if(ascent)
		*ascent = truetype_short(info->data + info->hhea + 4);
	if(descent)
		*descent = truetype_short(info->data + info->hhea + 6);
	if(linegap)
		*linegap = truetype_short(info->data + info->hhea + 8);
}

float truetype_scale_for_pixel_height(struct truetype_fontinfo_t * info, float height)
{
	int fheight = truetype_short(info->data + info->hhea + 4) - truetype_short(info->data + info->hhea + 6);
	return (float)height / fheight;
}

static void truetype_get_glyph_bitmap_box_subpixel(struct truetype_fontinfo_t * font, int glyph, float scale_x, float scale_y, float shift_x, float shift_y, int * ix0, int * iy0, int * ix1, int * iy1)
{
	int x0 = 0, y0 = 0, x1, y1;
	if(!truetype_get_glyph_box(font, glyph, &x0, &y0, &x1, &y1))
	{
		if(ix0)
			*ix0 = 0;
		if(iy0)
			*iy0 = 0;
		if(ix1)
			*ix1 = 0;
		if(iy1)
			*iy1 = 0;
	}
	else
	{
		if(ix0)
			*ix0 = (int)floorf(x0 * scale_x + shift_x);
		if(iy0)
			*iy0 = (int)floorf(-y1 * scale_y + shift_y);
		if(ix1)
			*ix1 = (int)ceilf(x1 * scale_x + shift_x);
		if(iy1)
			*iy1 = (int)ceilf(-y0 * scale_y + shift_y);
	}
}

void truetype_get_glyph_bitmap_box(struct truetype_fontinfo_t * font, int glyph, float scale_x, float scale_y, int * ix0, int * iy0, int * ix1, int * iy1)
{
	truetype_get_glyph_bitmap_box_subpixel(font, glyph, scale_x, scale_y, 0.0f, 0.0f, ix0, iy0, ix1, iy1);
}

static void * truetype_hheap_alloc(struct truetype_hheap_t * hh, size_t size)
{
	if(hh->first_free)
	{
		void * p = hh->first_free;
		hh->first_free = *(void **)p;
		return p;
	}
	else
	{
		if(hh->num_remaining_in_head_chunk == 0)
		{
			int count = (size < 32 ? 2000 : size < 128 ? 800 : 100);
			struct truetype_hheap_chunk_t * c = (struct truetype_hheap_chunk_t *)xos_mem_malloc(sizeof(struct truetype_hheap_chunk_t) + size * count);
			if(c == NULL)
				return NULL;
			c->next = hh->head;
			hh->head = c;
			hh->num_remaining_in_head_chunk = count;
		}
		--hh->num_remaining_in_head_chunk;
		return (char *)(hh->head) + sizeof(struct truetype_hheap_chunk_t) +
			   size * hh->num_remaining_in_head_chunk;
	}
}

static void truetype_hheap_free(struct truetype_hheap_t * hh, void * p)
{
	*(void **)p = hh->first_free;
	hh->first_free = p;
}

static void truetype_hheap_cleanup(struct truetype_hheap_t * hh)
{
	struct truetype_hheap_chunk_t * c = hh->head;
	while(c)
	{
		struct truetype_hheap_chunk_t * n = c->next;
		xos_mem_free(c);
		c = n;
	}
}

static struct truetype_active_edge_t * truetype_new_active(struct truetype_hheap_t * hh, struct truetype_edge_t * e, int off_x, float start_point)
{
	struct truetype_active_edge_t * z = (struct truetype_active_edge_t *)truetype_hheap_alloc(hh, sizeof(*z));
	float dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
	if(!z)
		return z;
	z->fdx = dxdy;
	z->fdy = dxdy != 0.0f ? (1.0f / dxdy) : 0.0f;
	z->fx = e->x0 + dxdy * (start_point - e->y0);
	z->fx -= off_x;
	z->direction = e->invert ? 1.0f : -1.0f;
	z->sy = e->y0;
	z->ey = e->y1;
	z->next = 0;
	return z;
}

static void truetype_handle_clipped_edge(float * scanline, int x, struct truetype_active_edge_t * e, float x0, float y0, float x1, float y1)
{
	if(y0 == y1)
		return;
	if(y0 > e->ey)
		return;
	if(y1 < e->sy)
		return;
	if(y0 < e->sy)
	{
		x0 += (x1 - x0) * (e->sy - y0) / (y1 - y0);
		y0 = e->sy;
	}
	if(y1 > e->ey)
	{
		x1 += (x1 - x0) * (e->ey - y1) / (y1 - y0);
		y1 = e->ey;
	}
	if(x0 <= x && x1 <= x)
	{
		scanline[x] += e->direction * (y1 - y0);
	}
	else if(x0 >= x + 1 && x1 >= x + 1)
	{
	}
	else
	{
		scanline[x] += e->direction * (y1 - y0) * (1 - ((x0 - x) + (x1 - x)) / 2);
	}
}

static float truetype_sized_trapezoid_area(float height, float top_width, float bottom_width)
{
	return (top_width + bottom_width) / 2.0f * height;
}

static float truetype_position_trapezoid_area(float height, float tx0, float tx1, float bx0, float bx1)
{
	return truetype_sized_trapezoid_area(height, tx1 - tx0, bx1 - bx0);
}

static float truetype_sized_triangle_area(float height, float width)
{
	return height * width / 2;
}

static void true_fill_active_edges_new(float * scanline, float * scanline_fill, int len, struct truetype_active_edge_t * e, float y_top)
{
	float y_bottom = y_top + 1;

	while(e)
	{
		if(e->fdx == 0)
		{
			float x0 = e->fx;
			if(x0 < len)
			{
				if(x0 >= 0)
				{
					truetype_handle_clipped_edge(scanline, (int)x0, e, x0, y_top, x0, y_bottom);
					truetype_handle_clipped_edge(scanline_fill - 1, (int)x0 + 1, e, x0, y_top, x0, y_bottom);
				}
				else
				{
					truetype_handle_clipped_edge(scanline_fill - 1, 0, e, x0, y_top, x0, y_bottom);
				}
			}
		}
		else
		{
			float x0 = e->fx;
			float dx = e->fdx;
			float xb = x0 + dx;
			float x_top, x_bottom;
			float sy0, sy1;
			float dy = e->fdy;
			if(e->sy > y_top)
			{
				x_top = x0 + dx * (e->sy - y_top);
				sy0 = e->sy;
			}
			else
			{
				x_top = x0;
				sy0 = y_top;
			}
			if(e->ey < y_bottom)
			{
				x_bottom = x0 + dx * (e->ey - y_top);
				sy1 = e->ey;
			}
			else
			{
				x_bottom = xb;
				sy1 = y_bottom;
			}

			if(x_top >= 0 && x_bottom >= 0 && x_top < len && x_bottom < len)
			{
				if((int)x_top == (int)x_bottom)
				{
					float height;
					int x = (int)x_top;
					height = (sy1 - sy0) * e->direction;
					scanline[x] += truetype_position_trapezoid_area(height, x_top, x + 1.0f, x_bottom, x + 1.0f);
					scanline_fill[x] += height;
				}
				else
				{
					int x, x1, x2;
					float y_crossing, y_final, step, sign, area;
					if(x_top > x_bottom)
					{
						float t;
						sy0 = y_bottom - (sy0 - y_top);
						sy1 = y_bottom - (sy1 - y_top);
						t = sy0, sy0 = sy1, sy1 = t;
						t = x_bottom, x_bottom = x_top, x_top = t;
						dx = -dx;
						dy = -dy;
						t = x0, x0 = xb, xb = t;
					}

					x1 = (int)x_top;
					x2 = (int)x_bottom;
					y_crossing = y_top + dy * (x1 + 1 - x0);
					y_final = y_top + dy * (x2 - x0);
					if(y_crossing > y_bottom)
						y_crossing = y_bottom;
					sign = e->direction;
					area = sign * (y_crossing - sy0);
					scanline[x1] += truetype_sized_triangle_area(area, x1 + 1 - x_top);
					if(y_final > y_bottom)
					{
						y_final = y_bottom;
						dy = (y_final - y_crossing) / (x2 - (x1 + 1));
					}
					step = sign * dy * 1;
					for(x = x1 + 1; x < x2; ++x)
					{
						scanline[x] += area + step / 2;
						area += step;
					}
					scanline[x2] += area + sign * truetype_position_trapezoid_area(sy1 - y_final, (float)x2, x2 + 1.0f, x_bottom, x2 + 1.0f);
					scanline_fill[x2] += sign * (sy1 - sy0);
				}
			}
			else
			{
				int x;
				for(x = 0; x < len; ++x)
				{
					float y0 = y_top;
					float x1 = (float)(x);
					float x2 = (float)(x + 1);
					float x3 = xb;
					float y3 = y_bottom;
					float y1 = (x - x0) / dx + y_top;
					float y2 = (x + 1 - x0) / dx + y_top;

					if(x0 < x1 && x3 > x2)
					{
						truetype_handle_clipped_edge(scanline, x, e, x0, y0, x1, y1);
						truetype_handle_clipped_edge(scanline, x, e, x1, y1, x2, y2);
						truetype_handle_clipped_edge(scanline, x, e, x2, y2, x3, y3);
					}
					else if(x3 < x1 && x0 > x2)
					{
						truetype_handle_clipped_edge(scanline, x, e, x0, y0, x2, y2);
						truetype_handle_clipped_edge(scanline, x, e, x2, y2, x1, y1);
						truetype_handle_clipped_edge(scanline, x, e, x1, y1, x3, y3);
					}
					else if(x0 < x1 && x3 > x1)
					{
						truetype_handle_clipped_edge(scanline, x, e, x0, y0, x1, y1);
						truetype_handle_clipped_edge(scanline, x, e, x1, y1, x3, y3);
					}
					else if(x3 < x1 && x0 > x1)
					{
						truetype_handle_clipped_edge(scanline, x, e, x0, y0, x1, y1);
						truetype_handle_clipped_edge(scanline, x, e, x1, y1, x3, y3);
					}
					else if(x0 < x2 && x3 > x2)
					{
						truetype_handle_clipped_edge(scanline, x, e, x0, y0, x2, y2);
						truetype_handle_clipped_edge(scanline, x, e, x2, y2, x3, y3);
					}
					else if(x3 < x2 && x0 > x2)
					{
						truetype_handle_clipped_edge(scanline, x, e, x0, y0, x2, y2);
						truetype_handle_clipped_edge(scanline, x, e, x2, y2, x3, y3);
					}
					else
					{
						truetype_handle_clipped_edge(scanline, x, e, x0, y0, x3, y3);
					}
				}
			}
		}
		e = e->next;
	}
}

static void truetype_rasterize_sorted_edges(struct truetype_bitmap_t * result, struct truetype_edge_t * e, int n, int vsubsample, int off_x, int off_y)
{
	struct truetype_hheap_t hh = { 0, 0, 0 };
	struct truetype_active_edge_t * active = NULL;
	int y, j = 0, i;
	float scanline_data[129], *scanline, *scanline2;

	if(result->w > 64)
		scanline = (float *)xos_mem_malloc((result->w * 2 + 1) * sizeof(float));
	else
		scanline = scanline_data;

	scanline2 = scanline + result->w;
	y = off_y;
	e[n].y0 = (float)(off_y + result->h) + 1;

	while(j < result->h)
	{
		float scan_y_top = y + 0.0f;
		float scan_y_bottom = y + 1.0f;
		struct truetype_active_edge_t ** step = &active;

		xos_memset(scanline, 0, result->w * sizeof(scanline[0]));
		xos_memset(scanline2, 0, (result->w + 1) * sizeof(scanline[0]));
		while(*step)
		{
			struct truetype_active_edge_t * z = *step;
			if(z->ey <= scan_y_top)
			{
				*step = z->next;
				z->direction = 0;
				truetype_hheap_free(&hh, z);
			}
			else
			{
				step = &((*step)->next);
			}
		}
		while(e->y0 <= scan_y_bottom)
		{
			if(e->y0 != e->y1)
			{
				struct truetype_active_edge_t * z = truetype_new_active(&hh, e, off_x, scan_y_top);
				if(z != NULL)
				{
					if(j == 0 && off_y != 0)
					{
						if(z->ey < scan_y_top)
						{
							z->ey = scan_y_top;
						}
					}
					z->next = active;
					active = z;
				}
			}
			++e;
		}
		if(active)
			true_fill_active_edges_new(scanline, scanline2 + 1, result->w, active, scan_y_top);
		{
			float sum = 0;
			for(i = 0; i < result->w; ++i)
			{
				float k;
				int m;
				sum += scanline2[i];
				k = scanline[i] + sum;
				k = (float)fabsf(k) * 255 + 0.5f;
				m = (int)k;
				if(m > 255)
					m = 255;
				result->pixels[j * result->stride + i] = (unsigned char)m;
			}
		}
		step = &active;
		while(*step)
		{
			struct truetype_active_edge_t * z = *step;
			z->fx += z->fdx;
			step = &((*step)->next);
		}
		++y;
		++j;
	}
	truetype_hheap_cleanup(&hh);
	if(scanline != scanline_data)
		xos_mem_free(scanline);
}

#define TRUETYPE_COMPARE(a, b)	((a)->y0 < (b)->y0)

static void truetype_sort_edges_ins_sort(struct truetype_edge_t * p, int n)
{
	int i, j;
	for(i = 1; i < n; ++i)
	{
		struct truetype_edge_t t = p[i], *a = &t;
		j = i;
		while(j > 0)
		{
			struct truetype_edge_t * b = &p[j - 1];
			int c = TRUETYPE_COMPARE(a, b);
			if(!c)
				break;
			p[j] = p[j - 1];
			--j;
		}
		if(i != j)
			p[j] = t;
	}
}

static void truetype_sort_edges_quicksort(struct truetype_edge_t * p, int n)
{
	while(n > 12)
	{
		struct truetype_edge_t t;
		int c01, c12, c, m, i, j;

		m = n >> 1;
		c01 = TRUETYPE_COMPARE(&p[0], &p[m]);
		c12 = TRUETYPE_COMPARE(&p[m], &p[n - 1]);
		if(c01 != c12)
		{
			int z;
			c = TRUETYPE_COMPARE(&p[0], &p[n - 1]);
			z = (c == c12) ? 0 : n - 1;
			t = p[z];
			p[z] = p[m];
			p[m] = t;
		}
		t = p[0];
		p[0] = p[m];
		p[m] = t;

		i = 1;
		j = n - 1;
		for(;;)
		{
			for(;; ++i)
			{
				if(!TRUETYPE_COMPARE(&p[i], &p[0]))
					break;
			}
			for(;; --j)
			{
				if(!TRUETYPE_COMPARE(&p[0], &p[j]))
					break;
			}
			if(i >= j)
				break;
			t = p[i];
			p[i] = p[j];
			p[j] = t;

			++i;
			--j;
		}
		if(j < (n - i))
		{
			truetype_sort_edges_quicksort(p, j);
			p = p + i;
			n = n - i;
		}
		else
		{
			truetype_sort_edges_quicksort(p + i, n - i);
			n = j;
		}
	}
}

static void truetype_sort_edges(struct truetype_edge_t * p, int n)
{
	truetype_sort_edges_quicksort(p, n);
	truetype_sort_edges_ins_sort(p, n);
}

static void truetype__rasterize(struct truetype_bitmap_t * result, struct truetype_point_t * pts, int * wcount, int windings, float scale_x, float scale_y, float shift_x, float shift_y, int off_x, int off_y, int invert)
{
	float y_scale_inv = invert ? -scale_y : scale_y;
	struct truetype_edge_t * e;
	int n, i, j, k, m;
	int vsubsample = 1;

	n = 0;
	for(i = 0; i < windings; ++i)
		n += wcount[i];
	e = (struct truetype_edge_t *)xos_mem_malloc(sizeof(*e) * (n + 1));
	if(e == 0)
		return;
	n = 0;
	m = 0;
	for(i = 0; i < windings; ++i)
	{
		struct truetype_point_t * p = pts + m;
		m += wcount[i];
		j = wcount[i] - 1;
		for(k = 0; k < wcount[i]; j = k++)
		{
			int a = k, b = j;
			if(p[j].y == p[k].y)
				continue;
			e[n].invert = 0;
			if(invert ? p[j].y > p[k].y : p[j].y < p[k].y)
			{
				e[n].invert = 1;
				a = j, b = k;
			}
			e[n].x0 = p[a].x * scale_x + shift_x;
			e[n].y0 = (p[a].y * y_scale_inv + shift_y) * vsubsample;
			e[n].x1 = p[b].x * scale_x + shift_x;
			e[n].y1 = (p[b].y * y_scale_inv + shift_y) * vsubsample;
			++n;
		}
	}
	truetype_sort_edges(e, n);
	truetype_rasterize_sorted_edges(result, e, n, vsubsample, off_x, off_y);
	xos_mem_free(e);
}

static void truetype_add_point(struct truetype_point_t * points, int n, float x, float y)
{
	if(!points)
		return;
	points[n].x = x;
	points[n].y = y;
}

static int truetype_tesselate_curve(struct truetype_point_t * points, int * num_points, float x0, float y0, float x1, float y1, float x2, float y2, float objspace_flatness_squared, int n)
{
	float mx = (x0 + 2 * x1 + x2) / 4;
	float my = (y0 + 2 * y1 + y2) / 4;
	float dx = (x0 + x2) / 2 - mx;
	float dy = (y0 + y2) / 2 - my;
	if(n > 16)
		return 1;
	if(dx * dx + dy * dy > objspace_flatness_squared)
	{
		truetype_tesselate_curve(points, num_points, x0, y0, (x0 + x1) / 2.0f, (y0 + y1) / 2.0f, mx, my, objspace_flatness_squared, n + 1);
		truetype_tesselate_curve(points, num_points, mx, my, (x1 + x2) / 2.0f, (y1 + y2) / 2.0f, x2, y2, objspace_flatness_squared, n + 1);
	}
	else
	{
		truetype_add_point(points, *num_points, x2, y2);
		*num_points = *num_points + 1;
	}
	return 1;
}

static void truetype_tesselate_cubic(struct truetype_point_t * points, int * num_points, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float objspace_flatness_squared, int n)
{
	float dx0 = x1 - x0;
	float dy0 = y1 - y0;
	float dx1 = x2 - x1;
	float dy1 = y2 - y1;
	float dx2 = x3 - x2;
	float dy2 = y3 - y2;
	float dx = x3 - x0;
	float dy = y3 - y0;
	float longlen = (float)(sqrtf(dx0 * dx0 + dy0 * dy0) + sqrtf(dx1 * dx1 + dy1 * dy1) + sqrtf(dx2 * dx2 + dy2 * dy2));
	float shortlen = (float)sqrtf(dx * dx + dy * dy);
	float flatness_squared = longlen * longlen - shortlen * shortlen;

	if(n > 16)
		return;
	if(flatness_squared > objspace_flatness_squared)
	{
		float x01 = (x0 + x1) / 2;
		float y01 = (y0 + y1) / 2;
		float x12 = (x1 + x2) / 2;
		float y12 = (y1 + y2) / 2;
		float x23 = (x2 + x3) / 2;
		float y23 = (y2 + y3) / 2;

		float xa = (x01 + x12) / 2;
		float ya = (y01 + y12) / 2;
		float xb = (x12 + x23) / 2;
		float yb = (y12 + y23) / 2;

		float mx = (xa + xb) / 2;
		float my = (ya + yb) / 2;

		truetype_tesselate_cubic(points, num_points, x0, y0, x01, y01, xa, ya, mx, my, objspace_flatness_squared, n + 1);
		truetype_tesselate_cubic(points, num_points, mx, my, xb, yb, x23, y23, x3, y3, objspace_flatness_squared, n + 1);
	}
	else
	{
		truetype_add_point(points, *num_points, x3, y3);
		*num_points = *num_points + 1;
	}
}

static struct truetype_point_t * truetype_flatten_curves(struct truetype_vertex_t * vertices, int num_verts, float objspace_flatness, int ** contour_lengths, int * num_contours)
{
	struct truetype_point_t * points = 0;
	int num_points = 0;
	float objspace_flatness_squared = objspace_flatness * objspace_flatness;
	int i, n = 0, start = 0, pass;

	for(i = 0; i < num_verts; ++i)
	{
		if(vertices[i].type == TRUETYPE_VMOVE)
			++n;
	}
	*num_contours = n;
	if(n == 0)
		return 0;

	*contour_lengths = (int *)xos_mem_malloc(sizeof(**contour_lengths) * n);
	if(*contour_lengths == 0)
	{
		*num_contours = 0;
		return 0;
	}
	for(pass = 0; pass < 2; ++pass)
	{
		float x = 0, y = 0;
		if(pass == 1)
		{
			points = (struct truetype_point_t *)xos_mem_malloc(num_points * sizeof(points[0]));
			if(points == NULL)
				goto error;
		}
		num_points = 0;
		n = -1;
		for(i = 0; i < num_verts; ++i)
		{
			switch(vertices[i].type)
			{
			case TRUETYPE_VMOVE:
				if(n >= 0)
					(*contour_lengths)[n] = num_points - start;
				++n;
				start = num_points;
				x = vertices[i].x, y = vertices[i].y;
				truetype_add_point(points, num_points++, x, y);
				break;
			case TRUETYPE_VLINE:
				x = vertices[i].x, y = vertices[i].y;
				truetype_add_point(points, num_points++, x, y);
				break;
			case TRUETYPE_VCURVE:
				truetype_tesselate_curve(points, &num_points, x, y, vertices[i].cx, vertices[i].cy, vertices[i].x, vertices[i].y, objspace_flatness_squared, 0);
				x = vertices[i].x, y = vertices[i].y;
				break;
			case TRUETYPE_VCUBIC:
				truetype_tesselate_cubic(points, &num_points, x, y, vertices[i].cx, vertices[i].cy, vertices[i].cx1, vertices[i].cy1, vertices[i].x, vertices[i].y, objspace_flatness_squared, 0);
				x = vertices[i].x, y = vertices[i].y;
				break;
			}
		}
		(*contour_lengths)[n] = num_points - start;
	}
	return points;
error:
	xos_mem_free(points);
	xos_mem_free(*contour_lengths);
	*contour_lengths = 0;
	*num_contours = 0;
	return NULL;
}

static void truetype_rasterize(struct truetype_bitmap_t * result, float flatness_in_pixels, struct truetype_vertex_t * vertices, int num_verts, float scale_x, float scale_y, float shift_x, float shift_y, int x_off, int y_off, int invert)
{
	float scale = scale_x > scale_y ? scale_y : scale_x;
	int winding_count = 0;
	int * winding_lengths = NULL;
	struct truetype_point_t * windings = truetype_flatten_curves(vertices, num_verts, flatness_in_pixels / scale, &winding_lengths, &winding_count);
	if(windings)
	{
		truetype__rasterize(result, windings, winding_lengths, winding_count, scale_x, scale_y, shift_x, shift_y, x_off, y_off, invert);
		xos_mem_free(winding_lengths);
		xos_mem_free(windings);
	}
}

static void truetype_make_glyph_bitmap_subpixel(struct truetype_fontinfo_t * info, unsigned char * output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph)
{
	struct truetype_vertex_t * vertices;
	struct truetype_bitmap_t gbm;
	int num_verts = truetype_get_glyph_shape(info, glyph, &vertices);
	int ix0, iy0;

	truetype_get_glyph_bitmap_box_subpixel(info, glyph, scale_x, scale_y, shift_x, shift_y, &ix0, &iy0, 0, 0);
	gbm.pixels = output;
	gbm.w = out_w;
	gbm.h = out_h;
	gbm.stride = out_stride;
	if(gbm.w && gbm.h)
		truetype_rasterize(&gbm, 0.35f, vertices, num_verts, scale_x, scale_y, shift_x, shift_y, ix0, iy0, 1);
	xos_mem_free(vertices);
}

void truetype_make_glyph_bitmap(struct truetype_fontinfo_t * info, unsigned char * output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int glyph)
{
	truetype_make_glyph_bitmap_subpixel(info, output, out_w, out_h, out_stride, scale_x, scale_y, 0.0f, 0.0f, glyph);
}
