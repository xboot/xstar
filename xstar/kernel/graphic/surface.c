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

#include <png.h>
#include <pngstruct.h>
#include <jpeglib.h>
#include <jerror.h>
#include <xos/xos.h>
#include <kernel/graphic/surface.h>

struct cg_surface_t * surface_get_cg_surface(struct surface_t * s)
{
	if(s)
	{
		struct cg_render_t * cg = s->cg;
		if(!cg)
		{
			cg = xos_mem_malloc(sizeof(struct cg_render_t));
			if(cg)
			{
				cg->surface = cg_surface_create_for_data(s->width, s->height, s->pixels);
				cg->ctx = cg_create(cg->surface);
				s->cg = cg;
			}
		}
		if(cg)
			return s->cg->surface;
	}
	return NULL;
}

struct cg_ctx_t * surface_get_cg_ctx(struct surface_t * s)
{
	if(s)
	{
		struct cg_render_t * cg = s->cg;
		if(!cg)
		{
			cg = xos_mem_malloc(sizeof(struct cg_render_t));
			if(cg)
			{
				cg->surface = cg_surface_create_for_data(s->width, s->height, s->pixels);
				cg->ctx = cg_create(cg->surface);
				s->cg = cg;
			}
		}
		if(cg)
			return s->cg->ctx;
	}
	return NULL;
}

struct surface_t * surface_alloc(int width, int height)
{
	if(width < 0 || height < 0)
		return NULL;

	struct surface_t * s = xos_mem_malloc(sizeof(struct surface_t));
	if(!s)
		return NULL;

	int stride = width << 2;
	int pixlen = height * stride;
	void * pixels = xos_mem_malloc(pixlen);
	if(!pixels)
	{
		xos_mem_free(s);
		return NULL;
	}
	xos_memset(pixels, 0, pixlen);

	s->width = width;
	s->height = height;
	s->stride = stride;
	s->pixlen = pixlen;
	s->pixels = pixels;
	s->g2d = search_first_g2d();
	s->cg = NULL;
	s->priv = NULL;
	return s;
}

void surface_free(struct surface_t * s)
{
	if(s)
	{
		if(s->cg)
		{
			cg_destroy(s->cg->ctx);
			cg_surface_destroy(s->cg->surface);
			xos_mem_free(s->cg);
		}
		xos_mem_free(s->pixels);
		xos_mem_free(s);
	}
}

enum {
	QOI_OP_INDEX	= 0x00, /* 00xxxxxx */
	QOI_OP_DIFF		= 0x40, /* 01xxxxxx */
	QOI_OP_LUMA		= 0x80, /* 10xxxxxx */
	QOI_OP_RUN		= 0xc0, /* 11xxxxxx */
	QOI_OP_RGB		= 0xfe, /* 11111110 */
	QOI_OP_RGBA		= 0xff, /* 11111111 */
	QOI_MASK_2		= 0xc0, /* 11000000 */
};

static inline uint32_t qoi_read32(uint8_t * buf, int * p)
{
	uint32_t a = buf[(*p)++];
	uint32_t b = buf[(*p)++];
	uint32_t c = buf[(*p)++];
	uint32_t d = buf[(*p)++];
	return (a << 24) | (b << 16) | (c << 8) | (d << 0);
}

static struct surface_t * surface_alloc_from_buf_qoi(const void * buf, int len)
{
	struct color_t idx[64];
	struct color_t px;
	int p = 0, run = 0;

	if(!buf || len < 14 + 8)
		return NULL;

	uint8_t * pbuf = (uint8_t *)buf;
	uint32_t magic = qoi_read32(pbuf, &p);
	uint32_t width = qoi_read32(pbuf, &p);
	uint32_t height = qoi_read32(pbuf, &p);
	uint8_t channels = pbuf[p++];
	uint8_t colorspace = pbuf[p++];

	if((width <= 0) || (height <= 0) || (channels < 3) || (channels > 4) || (colorspace > 1) || (magic != 0x716f6966) || (height >= 400000000 / width))
		return NULL;

	struct surface_t * s = surface_alloc(width, height);
	if(!s)
		return NULL;

	uint32_t * pixels = (uint32_t *)surface_get_pixels(s);
	int npixels = surface_get_pixlen(s) >> 2;
	int chklen = len - 8;

	xos_memset(idx, 0, sizeof(idx));
	color_init(&px, 0, 0, 0, 255);

	for(int i = 0; i < npixels; i++)
	{
		if(run > 0)
		{
			run--;
		}
		else if(p < chklen)
		{
			int b1 = pbuf[p++];
			if(b1 == QOI_OP_RGB)
			{
				px.r = pbuf[p++];
				px.g = pbuf[p++];
				px.b = pbuf[p++];
			}
			else if(b1 == QOI_OP_RGBA)
			{
				px.r = pbuf[p++];
				px.g = pbuf[p++];
				px.b = pbuf[p++];
				px.a = pbuf[p++];
			}
			else if((b1 & QOI_MASK_2) == QOI_OP_INDEX)
			{
				px = idx[b1];
			}
			else if((b1 & QOI_MASK_2) == QOI_OP_DIFF)
			{
				px.r += ((b1 >> 4) & 0x03) - 2;
				px.g += ((b1 >> 2) & 0x03) - 2;
				px.b += (b1 & 0x03) - 2;
			}
			else if((b1 & QOI_MASK_2) == QOI_OP_LUMA)
			{
				int b2 = pbuf[p++];
				int vg = (b1 & 0x3f) - 32;
				px.r += vg - 8 + ((b2 >> 4) & 0x0f);
				px.g += vg;
				px.b += vg - 8 + (b2 & 0x0f);
			}
			else if((b1 & QOI_MASK_2) == QOI_OP_RUN)
			{
				run = (b1 & 0x3f);
			}
			idx[(px.r * 3 + px.g * 5 + px.b * 7 + px.a * 11) & 0x3f] = px;
		}
		pixels[i] = color_get_premult(&px);
	}
	return s;
}

static struct surface_t * surface_alloc_from_xfs_qoi(struct xfs_context_t * ctx, const char * filename)
{
	struct surface_t * s = NULL;

	struct xfs_file_t * file = xfs_open_read(ctx, filename);
	if(file)
	{
		int64_t len = xfs_length(file);
		if(len > 0)
		{
			char * buf = xos_mem_malloc(len);
			if(buf)
			{
				int64_t n = xfs_read(file, buf, len);
				if(n > 0)
					s = surface_alloc_from_buf_qoi(buf, n);
				xos_mem_free(buf);
			}
		}
		xfs_close(file);
	}
	return s;
}

static void premultiply_data(png_structp png, png_row_infop row_info, png_bytep data)
{
	for(unsigned int i = 0; i < row_info->rowbytes; i += 4)
	{
		uint8_t * base = &data[i];
		uint8_t alpha = base[3];
		uint32_t p;

		if(alpha == 0)
		{
			p = 0;
		}
		else
		{
			uint8_t red = base[0];
			uint8_t green = base[1];
			uint8_t blue = base[2];
			if(alpha != 0xff)
			{
				red = XDIV255(alpha * red);
				green = XDIV255(alpha * green);
				blue = XDIV255(alpha * blue);
			}
			p = (alpha << 24) | (red << 16) | (green << 8) | (blue << 0);
		}
		xos_memcpy(base, &p, sizeof(uint32_t));
	}
}

static void convert_bytes_to_data(png_structp png, png_row_infop row_info, png_bytep data)
{
	for(unsigned int i = 0; i < row_info->rowbytes; i += 4)
	{
		uint8_t * base = &data[i];
		uint8_t red = base[0];
		uint8_t green = base[1];
		uint8_t blue = base[2];
		uint32_t pixel = (0xff << 24) | (red << 16) | (green << 8) | (blue << 0);
		xos_memcpy(base, &pixel, sizeof(uint32_t));
	}
}

static void png_xfs_read_data(png_structp png, png_bytep data, size_t length)
{
	if(png == NULL)
		return;
	size_t check = xfs_read((struct xfs_file_t *)png->io_ptr, data, length);
	if(check != length)
		png_error(png, "Read Error");
}

static void png_silent_error(png_structp png, png_const_charp msg)
{
	png_longjmp(png, 1);
}

static void png_silent_warning(png_structp png, png_const_charp msg)
{
}

static struct surface_t * surface_alloc_from_xfs_png(struct xfs_context_t * ctx, const char * filename)
{
	struct surface_t * s;
	png_struct * png;
	png_info * info;
	png_byte * data = NULL;
	png_byte ** row_pointers = NULL;
	png_uint_32 png_width, png_height;
	int depth, color_type, interlace, stride;
	unsigned int i;
	struct xfs_file_t * file;

	if(!(file = xfs_open_read(ctx, filename)))
		return NULL;

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png)
	{
		xfs_close(file);
		return NULL;
	}

	png_set_error_fn(png, NULL, png_silent_error, png_silent_warning);

	info = png_create_info_struct(png);
	if(!info)
	{
		xfs_close(file);
		png_destroy_read_struct(&png, NULL, NULL);
		return NULL;
	}

	png_set_read_fn(png, file, png_xfs_read_data);

#ifdef PNG_SETJMP_SUPPORTED
	if(setjmp(png_jmpbuf(png)))
	{
		png_destroy_read_struct(&png, &info, NULL);
		xfs_close(file);
		return NULL;
	}
#endif

	png_read_info(png, info);
	png_get_IHDR(png, info, &png_width, &png_height, &depth, &color_type, &interlace, NULL, NULL);

	if(color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);
	if(color_type == PNG_COLOR_TYPE_GRAY)
	{
#if PNG_LIBPNG_VER >= 10209
		png_set_expand_gray_1_2_4_to_8(png);
#else
		png_set_gray_1_2_4_to_8(png);
#endif
	}
	if(png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);
	if(depth == 16)
		png_set_strip_16(png);
	if(depth < 8)
		png_set_packing(png);
	if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);
	if(interlace != PNG_INTERLACE_NONE)
		png_set_interlace_handling(png);

	png_set_filler(png, 0xff, PNG_FILLER_AFTER);
	png_read_update_info(png, info);
	png_get_IHDR(png, info, &png_width, &png_height, &depth, &color_type, &interlace, NULL, NULL);

	if(depth != 8 || !(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA))
	{
		png_destroy_read_struct(&png, &info, NULL);
		xfs_close(file);
		return NULL;
	}

	switch(color_type)
	{
	case PNG_COLOR_TYPE_RGB_ALPHA:
		png_set_read_user_transform_fn(png, premultiply_data);
		break;
	case PNG_COLOR_TYPE_RGB:
		png_set_read_user_transform_fn(png, convert_bytes_to_data);
		break;
	default:
		break;
	}

	s = surface_alloc(png_width, png_height);
	data = surface_get_pixels(s);

	row_pointers = (png_byte **)xos_mem_malloc(png_height * sizeof(char *));
	stride = png_width * 4;

	for(i = 0; i < png_height; i++)
		row_pointers[i] = &data[i * stride];

	png_read_image(png, row_pointers);
	png_read_end(png, info);
	xos_mem_free(row_pointers);
	png_destroy_read_struct(&png, &info, NULL);
	xfs_close(file);

	return s;
}

struct png_source_t {
	unsigned char * data;
	int size;
	int offset;
};

static void png_source_read_data(png_structp png, png_bytep data, png_size_t length)
{
	struct png_source_t * src = (struct png_source_t *)png_get_io_ptr(png);

	if(src->offset + length <= src->size)
	{
		xos_memcpy(data, src->data + src->offset, length);
		src->offset += length;
	}
	else
		png_error(png, "read error");
}

static struct surface_t * surface_alloc_from_buf_png(const void * buf, int len)
{
	struct surface_t * s;
	png_struct * png;
	png_info * info;
	png_byte * data = NULL;
	png_byte ** row_pointers = NULL;
	png_uint_32 png_width, png_height;
	int depth, color_type, interlace, stride;
	unsigned int i;

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png)
		return NULL;

	png_set_error_fn(png, NULL, png_silent_error, png_silent_warning);

	info = png_create_info_struct(png);
	if(!info)
	{
		png_destroy_read_struct(&png, NULL, NULL);
		return NULL;
	}

	struct png_source_t src;
	src.data = (unsigned char *)buf;
	src.size = len;
	src.offset = 0;
	png_set_read_fn(png, &src, png_source_read_data);

#ifdef PNG_SETJMP_SUPPORTED
	if(setjmp(png_jmpbuf(png)))
	{
		png_destroy_read_struct(&png, &info, NULL);
		return NULL;
	}
#endif

	png_read_info(png, info);
	png_get_IHDR(png, info, &png_width, &png_height, &depth, &color_type, &interlace, NULL, NULL);

	if(color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);
	if(color_type == PNG_COLOR_TYPE_GRAY)
	{
#if PNG_LIBPNG_VER >= 10209
		png_set_expand_gray_1_2_4_to_8(png);
#else
		png_set_gray_1_2_4_to_8(png);
#endif
	}
	if(png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);
	if(depth == 16)
		png_set_strip_16(png);
	if(depth < 8)
		png_set_packing(png);
	if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);
	if(interlace != PNG_INTERLACE_NONE)
		png_set_interlace_handling(png);

	png_set_filler(png, 0xff, PNG_FILLER_AFTER);
	png_read_update_info(png, info);
	png_get_IHDR(png, info, &png_width, &png_height, &depth, &color_type, &interlace, NULL, NULL);

	if(depth != 8 || !(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA))
	{
		png_destroy_read_struct(&png, &info, NULL);
		return NULL;
	}

	switch(color_type)
	{
	case PNG_COLOR_TYPE_RGB_ALPHA:
		png_set_read_user_transform_fn(png, premultiply_data);
		break;
	case PNG_COLOR_TYPE_RGB:
		png_set_read_user_transform_fn(png, convert_bytes_to_data);
		break;
	default:
		break;
	}

	s = surface_alloc(png_width, png_height);
	data = surface_get_pixels(s);

	row_pointers = (png_byte **)xos_mem_malloc(png_height * sizeof(char *));
	stride = png_width * 4;

	for(i = 0; i < png_height; i++)
		row_pointers[i] = &data[i * stride];

	png_read_image(png, row_pointers);
	png_read_end(png, info);
	xos_mem_free(row_pointers);
	png_destroy_read_struct(&png, &info, NULL);

	return s;
}

struct x_error_mgr
{
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};

struct x_source_mgr
{
	struct jpeg_source_mgr pub;
	struct xfs_file_t * file;
	JOCTET * buffer;
	int start_of_file;
};

static void x_output_message(j_common_ptr dinfo)
{
}

static void x_error_exit(j_common_ptr dinfo)
{
	struct x_error_mgr * err = (struct x_error_mgr *)dinfo->err;
	longjmp(err->setjmp_buffer, 1);
}

static void x_emit_message(j_common_ptr dinfo, int msg_level)
{
	struct jpeg_error_mgr * err = dinfo->err;
	if(msg_level < 0)
		err->num_warnings++;
}

static void init_source(j_decompress_ptr dinfo)
{
	struct x_source_mgr * src = (struct x_source_mgr *)dinfo->src;
	src->start_of_file = 1;
}

static boolean fill_input_buffer(j_decompress_ptr dinfo)
{
	struct x_source_mgr * src = (struct x_source_mgr *)dinfo->src;
	size_t nbytes;

	nbytes = xfs_read(src->file, src->buffer, 4096);
	if(nbytes <= 0)
	{
		if(src->start_of_file)
			ERREXIT(dinfo, JERR_INPUT_EMPTY);
		WARNMS(dinfo, JWRN_JPEG_EOF);
		src->buffer[0] = (JOCTET)0xFF;
		src->buffer[1] = (JOCTET)JPEG_EOI;
		nbytes = 2;
	}
	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->start_of_file = 0;
	return 1;
}

static void skip_input_data(j_decompress_ptr dinfo, long num_bytes)
{
	struct jpeg_source_mgr * src = dinfo->src;

	if(num_bytes > 0)
	{
		while(num_bytes > (long)src->bytes_in_buffer)
		{
			num_bytes -= (long)src->bytes_in_buffer;
			(void)(*src->fill_input_buffer)(dinfo);
		}
		src->next_input_byte += (size_t)num_bytes;
		src->bytes_in_buffer -= (size_t)num_bytes;
	}
}

static void term_source(j_decompress_ptr dinfo)
{
}

static void jpeg_xfs_src(j_decompress_ptr dinfo, struct xfs_file_t * file)
{
	struct x_source_mgr * src;

	if(dinfo->src == NULL)
	{
		dinfo->src = (struct jpeg_source_mgr *)(*dinfo->mem->alloc_small)((j_common_ptr)dinfo, JPOOL_PERMANENT, sizeof(struct x_source_mgr));
		src = (struct x_source_mgr *)dinfo->src;
		src->buffer = (JOCTET *)(*dinfo->mem->alloc_small)((j_common_ptr)dinfo, JPOOL_PERMANENT, 4096 * sizeof(JOCTET));
	}

	src = (struct x_source_mgr *)dinfo->src;
	src->pub.init_source = init_source;
	src->pub.fill_input_buffer = fill_input_buffer;
	src->pub.skip_input_data = skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart;
	src->pub.term_source = term_source;
	src->file = file;
	src->pub.bytes_in_buffer = 0;
	src->pub.next_input_byte = NULL;
}

static struct surface_t * surface_alloc_from_xfs_jpg(struct xfs_context_t * ctx, const char * filename)
{
	struct jpeg_decompress_struct dinfo;
	struct x_error_mgr jerr;
	struct surface_t * s;
	struct xfs_file_t * file;
	JSAMPARRAY tmp;
	unsigned char * p;
	int scanline, offset, i;

	if(!(file = xfs_open_read(ctx, filename)))
		return NULL;
	dinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.output_message = x_output_message;
	jerr.pub.error_exit = x_error_exit;
	jerr.pub.emit_message = x_emit_message;
	if(setjmp(jerr.setjmp_buffer))
	{
		jpeg_destroy_decompress(&dinfo);
		xfs_close(file);
		return 0;
	}
	jpeg_create_decompress(&dinfo);
	jpeg_xfs_src(&dinfo, file);
	jpeg_read_header(&dinfo, 1);
	jpeg_start_decompress(&dinfo);
	tmp = (*dinfo.mem->alloc_sarray)((j_common_ptr)&dinfo, JPOOL_IMAGE, dinfo.output_width * dinfo.output_components, 1);
	s = surface_alloc(dinfo.image_width, dinfo.image_height);
	p = surface_get_pixels(s);
	while(dinfo.output_scanline < dinfo.output_height)
	{
		scanline = dinfo.output_scanline * surface_get_stride(s);
		jpeg_read_scanlines(&dinfo, tmp, 1);
		for(i = 0; i < dinfo.output_width; i++)
		{
			offset = scanline + (i * 4);
			p[offset + 3] = 0xff;
			p[offset + 2] = tmp[0][(i * 3) + 0];
			p[offset + 1] = tmp[0][(i * 3) + 1];
			p[offset + 0] = tmp[0][(i * 3) + 2];
		}
	}
	jpeg_finish_decompress(&dinfo);
	jpeg_destroy_decompress(&dinfo);
	xfs_close(file);

	return s;
}

static struct surface_t * surface_alloc_from_buf_jpg(const void * buf, int len)
{
	struct jpeg_decompress_struct dinfo;
	struct x_error_mgr jerr;
	struct surface_t * s;
	JSAMPARRAY tmp;
	unsigned char * p;
	int scanline, offset, i;

	dinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.output_message = x_output_message;
	jerr.pub.error_exit = x_error_exit;
	jerr.pub.emit_message = x_emit_message;
	if(setjmp(jerr.setjmp_buffer))
	{
		jpeg_destroy_decompress(&dinfo);
		return NULL;
	}
	jpeg_create_decompress(&dinfo);
	jpeg_mem_src(&dinfo, buf, len);
	jpeg_read_header(&dinfo, 1);
	jpeg_start_decompress(&dinfo);
	tmp = (*dinfo.mem->alloc_sarray)((j_common_ptr)&dinfo, JPOOL_IMAGE, dinfo.output_width * dinfo.output_components, 1);
	s = surface_alloc(dinfo.image_width, dinfo.image_height);
	p = surface_get_pixels(s);
	while(dinfo.output_scanline < dinfo.output_height)
	{
		scanline = dinfo.output_scanline * surface_get_stride(s);
		jpeg_read_scanlines(&dinfo, tmp, 1);
		for(i = 0; i < dinfo.output_width; i++)
		{
			offset = scanline + (i * 4);
			p[offset + 3] = 0xff;
			p[offset + 2] = tmp[0][(i * 3) + 0];
			p[offset + 1] = tmp[0][(i * 3) + 1];
			p[offset + 0] = tmp[0][(i * 3) + 2];
		}
	}
	jpeg_finish_decompress(&dinfo);
	jpeg_destroy_decompress(&dinfo);

	return s;
}

struct surface_t * surface_alloc_from_xfs(struct xfs_context_t * ctx, const char * filename)
{
	const char * ext = path_fileext(filename);

	if(ext)
	{
		if(xos_strcasecmp(ext, "qoi") == 0)
			return surface_alloc_from_xfs_qoi(ctx, filename);
		else if(xos_strcasecmp(ext, "png") == 0)
			return surface_alloc_from_xfs_png(ctx, filename);
		else if((xos_strcasecmp(ext, "jpg") == 0) || (xos_strcasecmp(ext, "jpeg") == 0))
			return surface_alloc_from_xfs_jpg(ctx, filename);
	}
	return NULL;
}

struct surface_t * surface_alloc_from_buf(const void * buf, int len)
{
	struct surface_t * s;

	if(buf && (len > 0))
	{
		s = surface_alloc_from_buf_qoi(buf, len);
		if(s)
			return s;
		s = surface_alloc_from_buf_png(buf, len);
		if(s)
			return s;
		s = surface_alloc_from_buf_jpg(buf, len);
		if(s)
			return s;
	}
	return NULL;
}

struct surface_t * surface_alloc_qrcode(int pixsz, const char * fmt, ...)
{
	if(fmt)
	{
		va_list ap;
		char * str = NULL;
		va_start(ap, fmt);
		int len = xos_vasprintf(&str, fmt, ap);
		va_end(ap);
		if(str && (len > 0))
		{
			uint8_t qrc[QRCGEN_BUFFER_LEN_MAX];
			uint8_t tmp[QRCGEN_BUFFER_LEN_MAX];
			if(qrcgen_encode_text(str, tmp, qrc, QRCGEN_ECC_MEDIUM, QRCGEN_VERSION_MIN, QRCGEN_VERSION_MAX, QRCGEN_MASK_AUTO, 1))
			{
				int qrs = qrcgen_get_size(qrc);
				if(qrs > 0)
				{
					pixsz = (pixsz > 0) ? pixsz : 1;
					struct surface_t * s = surface_alloc((qrs + 4) * pixsz, (qrs + 4) * pixsz);
					if(s)
					{
						xos_memset(s->pixels, 0xff, s->pixlen);
						int l = s->stride >> 2;
						for(int j = 0; j < qrs; j++)
						{
							for(int i = 0; i < qrs; i++)
							{
								if(qrcgen_get_pixel(qrc, i, j))
								{
									int x1 = (i + 2) * pixsz;
									int y1 = (j + 2) * pixsz;
									int x2 = x1 + pixsz;
									int y2 = y1 + pixsz;
									int x, y;
									uint32_t * p, * q = (uint32_t *)s->pixels + y1 * l + x1;
									for(y = y1; y < y2; y++, q += l)
									{
										for(x = x1, p = q; x < x2; x++, p++)
											*p = 0xff000000;
									}
								}
							}
						}
						return s;
					}
				}
			}
			xos_mem_free(str);
		}
	}
	return NULL;
}

struct surface_t * surface_clone(struct surface_t * s, int x, int y, int w, int h)
{
	struct surface_t * o;
	void * pixels;
	int width, height, stride, pixlen;
	int x1, y1, x2, y2;

	if(!s)
		return NULL;

	if((w <= 0) || (h <= 0))
	{
		width = s->width;
		height = s->height;
		stride = s->stride;
		pixlen = s->pixlen;

		o = xos_mem_malloc(sizeof(struct surface_t));
		if(!o)
			return NULL;
		pixels = xos_mem_malloc(pixlen);
		if(!pixels)
		{
			xos_mem_free(o);
			return NULL;
		}
		xos_memcpy(pixels, s->pixels, pixlen);
	}
	else
	{
		x1 = XMAX(0, x);
		x2 = XMIN(s->width, x + w);
		if(x1 <= x2)
		{
			y1 = XMAX(0, y);
			y2 = XMIN(s->height, y + h);
			if(y1 <= y2)
			{
				width = x2 - x1;
				height = y2 - y1;
				stride = width << 2;
				pixlen = height * stride;

				o = xos_mem_malloc(sizeof(struct surface_t));
				if(!o)
					return NULL;
				pixels = xos_mem_malloc(pixlen);
				if(!pixels)
				{
					xos_mem_free(o);
					return NULL;
				}
				int sstride = s->stride;
				unsigned char * p = (unsigned char *)pixels;
				unsigned char * q = (unsigned char *)s->pixels + y1 * sstride + (x1 << 2);
				for(int i = 0; i < height; i++, p += stride, q += sstride)
					xos_memcpy(p, q, stride);
			}
			else
				return NULL;
		}
		else
			return NULL;
	}

	o->width = width;
	o->height = height;
	o->stride = stride;
	o->pixlen = pixlen;
	o->pixels = pixels;
	o->g2d = search_first_g2d();
	o->cg = NULL;
	o->priv = NULL;
	return o;
}

struct surface_t * surface_extend(struct surface_t * s, int width, int height, const char * type)
{
	struct surface_t * o;
	uint32_t * dp, * sp;
	void * pixels, * spixels;
	int stride, pixlen;
	int sw, sh, x, y;

	if(!s || (width <= 0) || (height <= 0))
		return NULL;

	o = xos_mem_malloc(sizeof(struct surface_t));
	if(!o)
		return NULL;

	stride = width << 2;
	pixlen = height * stride;
	pixels = xos_mem_malloc(pixlen);
	if(!pixels)
	{
		xos_mem_free(s);
		return NULL;
	}
	spixels = s->pixels;
	sw = s->width;
	sh = s->height;

	switch(shash(type))
	{
	case 0x192dec66: /* "repeat" */
		for(y = 0, dp = (uint32_t *)pixels; y < height; y++)
		{
			for(x = 0, sp = (uint32_t *)spixels + (y % sh) * sw; x < width; x++)
			{
				*dp++ = *(sp + (x % sw));
			}
		}
		break;
	case 0x3e3a6a0a: /* "reflect" */
		for(y = 0, dp = (uint32_t *)pixels; y < height; y++)
		{
			for(x = 0, sp = (uint32_t *)spixels + (((y / sh) & 0x1) ? (sh - 1 - (y % sh)) : (y % sh)) * sw; x < width; x++)
			{
				*dp++ = *(sp + (((x / sw) & 0x1) ? (sw - 1 - (x % sw)) : (x % sw)));
			}
		}
		break;
	case 0x0b889c3a: /* "pad" */
		for(y = 0, dp = (uint32_t *)pixels; y < height; y++)
		{
			for(x = 0, sp = (uint32_t *)spixels + ((y < sh) ? y : sh - 1) * sw; x < width; x++)
			{
				*dp++ = *(sp + ((x < sw) ? x : sw - 1));
			}
		}
		break;
	default:
		for(y = 0, dp = (uint32_t *)pixels; y < height; y++)
		{
			if(y < sh)
			{
				for(x = 0, sp = (uint32_t *)spixels + y * sw; x < width; x++)
				{
					if(x < sw)
						*dp++ = *(sp + x);
					else
						*dp++ = 0;
				}
			}
			else
			{
				xos_memset(dp, 0, stride);
				dp += width;
			}
		}
		break;
	}

	o->width = width;
	o->height = height;
	o->stride = stride;
	o->pixlen = pixlen;
	o->pixels = pixels;
	o->g2d = search_first_g2d();
	o->cg = NULL;
	o->priv = NULL;
	return o;
}

void surface_clear(struct surface_t * s, struct color_t * c, int x, int y, int w, int h)
{
	uint32_t * q, * p, v;
	int x1, y1, x2, y2;
	int i, j, l;

	if(s)
	{
		v = c ? color_get_premult(c) : 0;
		if((w <= 0) || (h <= 0))
		{
			if(v)
			{
				if(v == 0xffffffff)
				{
					xos_memset(s->pixels, 0xff, s->pixlen);
				}
				else
				{
					p = (uint32_t * )s->pixels;
					l = s->width * s->height;
					while(l--)
						*p++ = v;
				}
			}
			else
			{
				xos_memset(s->pixels, 0, s->pixlen);
			}
		}
		else
		{
			x1 = XMAX(0, x);
			x2 = XMIN(s->width, x + w);
			if(x1 <= x2)
			{
				y1 = XMAX(0, y);
				y2 = XMIN(s->height, y + h);
				if(y1 <= y2)
				{
					l = s->stride >> 2;
					q = (uint32_t *)s->pixels + y1 * l + x1;
					for(j = y1; j < y2; j++, q += l)
					{
						for(i = x1, p = q; i < x2; i++, p++)
							*p = v;
					}
				}
			}
		}
	}
}

void surface_set_pixel(struct surface_t * s, int x, int y, struct color_t * c)
{
	if(c && s && (x < s->width) && (y < s->height))
	{
		uint32_t * p = (uint32_t *)s->pixels + y * (s->stride >> 2) + x;
		*p = color_get_premult(c);
	}
}

void surface_get_pixel(struct surface_t * s, int x, int y, struct color_t * c)
{
	if(c)
	{
		if(s && (x < s->width) && (y < s->height))
		{
			uint32_t * p = (uint32_t *)s->pixels + y * (s->stride >> 2) + x;
			color_set_premult(c, *p);
		}
		else
		{
			xos_memset(&c, 0, sizeof(struct color_t));
		}
	}
}

static void surface_callback(void * data, int x0, int y0, void * gray, int width, int height)
{
	struct {
		struct surface_t * s;
		struct region_t * clip;
		uint32_t c;
	} * ctx = data;

	struct region_t region, r;
	region_init(&r, 0, 0, surface_get_width(ctx->s), surface_get_height(ctx->s));
	if(ctx->clip)
	{
		if(!region_intersect(&r, &r, ctx->clip))
			return;
	}
	region_init(&region, x0, y0, width, height);
	if(!region_intersect(&r, &r, &region))
		return;

	uint32_t * p, * q = &((uint32_t *)surface_get_pixels(ctx->s))[r.y * surface_get_width(ctx->s) + r.x];
	uint8_t * pgray, * qgray = &((uint8_t *)gray)[(r.y - y0) * width + (r.x - x0)];
	int x, y;

	for(y = 0; y < r.h; y++, qgray += width, q += surface_get_width(ctx->s))
	{
		for(x = 0, pgray = qgray, p = q; x < r.w; x++, pgray++, p++)
		{
			color_premult_blend(p, 1, ctx->c, *pgray);
		}
	}
}

void surface_text(struct surface_t * s, struct region_t * clip, int x, int y, int wrap, const char * family, enum font_style_t style, int size, struct color_t * c, const char * fmt, ...)
{
	if(s)
	{
		va_list ap;
		char * str = NULL;
		va_start(ap, fmt);
		int len = xos_vasprintf(&str, fmt, ap);
		va_end(ap);
		if(str && (len > 0))
		{
			font_text_render(family, style, size, x, y, wrap, str, surface_callback, (void *)&(struct{struct surface_t * s;struct region_t * clip;uint32_t c;}){ s, clip, c ? color_get_premult(c) : 0xffffffff});
			xos_mem_free(str);
		}
	}
}

void surface_icon(struct surface_t * s, struct region_t * clip, int x, int y, const char * family, int size, uint32_t code, struct color_t * c)
{
	if(s)
		font_icon_render(family, size, x, y, code, surface_callback, (void *)&(struct{struct surface_t * s;struct region_t * clip;uint32_t c;}){ s, clip, c ? color_get_premult(c) : 0xffffffff});
}

void surface_blit(struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, struct surface_t * o)
{
	if(!g2d_blit(s->g2d, s, clip, m, o))
	{
		struct cg_ctx_t * cg = surface_get_cg_ctx(s);
		struct region_t r;

		cg_save(cg);
		if(clip)
		{
			region_init(&r, 0, 0, surface_get_width(s), surface_get_height(s));
			if(region_intersect(&r, &r, clip))
			{
				cg_rectangle(cg, r.x, r.y, r.w, r.h);
				cg_clip(cg);
			}
			else
			{
				cg_restore(cg);
				return;
			}
		}
		if(m)
			cg_set_matrix(cg, (struct cg_matrix_t *)m);
		cg_rectangle(cg, 0, 0, o->width, o->height);
		cg_clip(cg);
		cg_set_source_surface(cg, surface_get_cg_surface(o), 0, 0);
		cg_paint(cg);
		cg_restore(cg);
	}
}

void surface_fill(struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, int w, int h, struct color_t * c)
{
	if(!g2d_fill(s->g2d, s, clip, m, w, h, c))
	{
		struct cg_ctx_t * cg = surface_get_cg_ctx(s);
		struct region_t r;

		cg_save(cg);
		if(clip)
		{
			region_init(&r, 0, 0, surface_get_width(s), surface_get_height(s));
			if(region_intersect(&r, &r, clip))
			{
				cg_rectangle(cg, r.x, r.y, r.w, r.h);
				cg_clip(cg);
			}
			else
			{
				cg_restore(cg);
				return;
			}
		}
		if(m)
			cg_set_matrix(cg, (struct cg_matrix_t *)m);
		cg_rectangle(cg, 0, 0, w, h);
		cg_set_source_rgba(cg, c->r / 255.0, c->g / 255.0, c->b / 255.0, c->a / 255.0);
		cg_fill(cg);
		cg_restore(cg);
	}
}
