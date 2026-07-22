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

#include <xstar.h>

/*
 * romdisk
 */
static void do_init_romdisk(void)
{
	extern unsigned char __romdisk_start[];
	extern unsigned char __romdisk_end[];
	char json[256];
	int length;

	length = xos_snprintf(json, sizeof(json),
		"{\"blk-romdisk:0\":{\"address\":%lld,\"size\":%lld}}",
		(unsigned long long)((io_addr_t)(__romdisk_start)),
		(unsigned long long)((io_addr_t)(__romdisk_end - __romdisk_start)));
	probe_device(json, length);
}

/*
 * dtree
 */
static void do_init_dtree(const char * dtree)
{
	size_t len;

	if(dtree && ((len = xos_strlen(dtree)) > 0) && (dtree[0] == '{') && (dtree[len - 1] == '}'))
	{
		probe_device(dtree, len);
	}
	else
	{
		struct xfs_context_t * ctx = xfs_alloc();
		if(ctx)
		{
			struct xfs_file_t * file = xfs_open_read(ctx, dtree ? dtree : "/romdisk/dtree/default.json");
			if(file)
			{
				int64_t l = xfs_length(file);
				if(l > 0)
				{
					char * json = xos_mem_malloc(l);
					if(json)
					{
						int64_t length = xfs_read(file, json, l);
						if(length > 0)
							probe_device(json, length);
						xos_mem_free(json);
					}
				}
				xfs_close(file);
			}
			xfs_free(ctx);
		}
	}
}

/*
 * memory
 */
static struct kobj_t * search_class_memory_kobj(void)
{
	struct kobj_t * kclass = kobj_search_directory_with_create(kobj_get_root(), "class");
	return kobj_search_directory_with_create(kclass, "memory");
}

static ssize_t memory_read_meminfo(struct kobj_t * kobj, void * buf, size_t size)
{
	size_t mused = 0;
	size_t mfree = 0;
	char * p = buf;
	int len = 0;

	xos_mem_meminfo(&mused, &mfree);
	len += xos_sprintf((char *)(p + len), " memory used: %zu\r\n", mused);
	len += xos_sprintf((char *)(p + len), " memory free: %zu\r\n", mfree);
	return len;
}

static void do_init_memory(void)
{
	kobj_add_regular(search_class_memory_kobj(), "meminfo", memory_read_meminfo, NULL, NULL);
}

/*
 * logger
 */
static struct kobj_t * search_class_logger_kobj(void)
{
	struct kobj_t * kclass = kobj_search_directory_with_create(kobj_get_root(), "class");
	return kobj_search_directory_with_create(kclass, "logger");
}

static ssize_t logger_read_status(struct kobj_t * kobj, void * buf, size_t size)
{
	return xos_sprintf(buf, "%d", logger_status());
}

static ssize_t logger_write_status(struct kobj_t * kobj, void * buf, size_t size)
{
	if(xos_strtol(buf, NULL, 0) != 0)
		logger_enable();
	else
		logger_disable();
	return size;
}

static void do_init_logger(void)
{
	kobj_add_regular(search_class_logger_kobj(), "status", logger_read_status, logger_write_status, NULL);
}

/*
 * version
 */
int xstar_version(void)
{
	return (XSTAR_VERSION_MAJOR << 16) | (XSTAR_VERSION_MINOR << 8) | (XSTAR_VERSION_PATCH << 0);
}

const char * xstar_banner(void)
{
	#define STR(x)		#x
	#define TOSTR(x)	STR(x)
	return "V"TOSTR(XSTAR_VERSION_MAJOR)"."TOSTR(XSTAR_VERSION_MINOR)"."TOSTR(XSTAR_VERSION_PATCH)" ("__DATE__" - "__TIME__")";
}

/*
 * __  __  ____  _____  ____  _____
 * \ \/ / (_ (_ |_   _|/ () \ | () )
 * /_/\_\ __)__)  |_| /__/\__\|_|\_\
 */
const char * xstar_clogo(void)
{
	static const char clogo[] = {
		"__  __  ____  _____  ____  _____ \r\n"
		"\\ \\/ / (_ (_ |_   _|/ () \\ | () )\r\n"
		"/_/\\_\\ __)__)  |_| /__/\\__\\|_|\\_\\"
	};
	return &clogo[0];
}

static struct kobj_t * search_class_version_kobj(void)
{
	struct kobj_t * kclass = kobj_search_directory_with_create(kobj_get_root(), "class");
	return kobj_search_directory_with_create(kclass, "version");
}

static ssize_t version_read_number(struct kobj_t * kobj, void * buf, size_t size)
{
	return xos_sprintf(buf, "0x%08x", xstar_version());
}

static ssize_t version_read_banner(struct kobj_t * kobj, void * buf, size_t size)
{
	return xos_sprintf(buf, "%s", xstar_banner());
}

static void do_init_version(void)
{
	kobj_add_regular(search_class_version_kobj(), "number", version_read_number, NULL, NULL);
	kobj_add_regular(search_class_version_kobj(), "banner", version_read_banner, NULL, NULL);
}

/*
 * copyright
 */
static struct kobj_t * search_class_copyright_kobj(void)
{
	struct kobj_t * kclass = kobj_search_directory_with_create(kobj_get_root(), "class");
	return kobj_search_directory_with_create(kclass, "copyright");
}

static ssize_t copyright_read_uniqueid(struct kobj_t * kobj, void * buf, size_t size)
{
	return xos_sprintf(buf, "%s", xos_copyright_uniqueid());
}

static ssize_t copyright_read_verify(struct kobj_t * kobj, void * buf, size_t size)
{
	return xos_sprintf(buf, "%s", xos_copyright_verify() ? "okay" : "fail");
}

static void do_init_copyright(void)
{
	kobj_add_regular(search_class_copyright_kobj(), "uniqueid", copyright_read_uniqueid, NULL, NULL);
	kobj_add_regular(search_class_copyright_kobj(), "verify", copyright_read_verify, NULL, NULL);
}

/*
 * random
 */
static struct kobj_t * search_class_random_kobj(void)
{
	struct kobj_t * kclass = kobj_search_directory_with_create(kobj_get_root(), "class");
	return kobj_search_directory_with_create(kclass, "random");
}

static ssize_t random_read_uuid(struct kobj_t * kobj, void * buf, size_t size)
{
	char tmp[37];
	return xos_sprintf(buf, "%s", uuid4(tmp));
}

static void do_init_random(void)
{
	struct rng_t * rng = search_first_rng();
	unsigned int seed = 0;

	if(!rng || (rng_read(rng, &seed, sizeof(unsigned int), 0) != sizeof(unsigned int)))
	{
		struct wallclock_timeval_t tv;
		if(wallclock_gettimeofday(&tv))
			seed = (unsigned int)tv.tv_sec;
	}
	xos_srand(seed);
	kobj_add_regular(search_class_random_kobj(), "uuid", random_read_uuid, NULL, NULL);
}

/*
 * feature
 */
static void probe_coroutine(struct scheduler_t * sched, void * data)
{
	*((int *)data) = 1;
}

int xstar_feature_coroutine(void)
{
	static int supported = -1;

	if(supported < 0)
	{
		supported = 0;
		struct scheduler_t sched;
		scheduler_init(&sched);
		coroutine_start(&sched, probe_coroutine, &supported, 0);
		scheduler_loop(&sched);
	}
	return supported;
}

static void probe_thread(void * data)
{
	*((int *)data) = 1;
}

int xstar_feature_thread(void)
{
	static int supported = -1;

	if(supported < 0)
	{
		supported = 0;
		struct thread_t * thread = xos_thread_create(NULL, probe_thread, &supported, 0);
		xos_thread_wait(thread);
		xos_thread_destroy(thread);
	}
	return supported;
}

static struct kobj_t * search_class_feature_kobj(void)
{
	struct kobj_t * kclass = kobj_search_directory_with_create(kobj_get_root(), "class");
	return kobj_search_directory_with_create(kclass, "feature");
}

static ssize_t feature_read_coroutine(struct kobj_t * kobj, void * buf, size_t size)
{
	return xos_sprintf(buf, "%d", xstar_feature_coroutine());
}

static ssize_t feature_read_thread(struct kobj_t * kobj, void * buf, size_t size)
{
	return xos_sprintf(buf, "%d", xstar_feature_thread());
}

static ssize_t feature_read_endian(struct kobj_t * kobj, void * buf, size_t size)
{
	return xos_sprintf(buf, "%s", cpu_is_big_endian() ? "big" : "little");
}

static void do_init_feature(void)
{
	kobj_add_regular(search_class_feature_kobj(), "coroutine", feature_read_coroutine, NULL, NULL);
	kobj_add_regular(search_class_feature_kobj(), "thread", feature_read_thread, NULL, NULL);
	kobj_add_regular(search_class_feature_kobj(), "endian", feature_read_endian, NULL, NULL);
}

/*
 * font
 */
static void do_init_font(void)
{
	struct xfs_context_t * ctx = xfs_alloc();
	if(ctx)
	{
		struct xfs_file_t * file = xfs_open_read(ctx, "/romdisk/assets/fonts/font.json");
		if(file)
		{
			int64_t l = xfs_length(file);
			if(l > 0)
			{
				char * json = xos_mem_malloc(l);
				if(json)
				{
					int64_t len = xfs_read(file, json, l);
					if(len > 0)
					{
						struct json_value_t * root = json_parse(json, len, NULL);
						if(root && (root->type == JSON_OBJECT))
						{
							for(int i = 0; i < root->u.object.length; i++)
							{
								struct json_value_t * u = root->u.object.values[i].value;
								if(u && (u->type == JSON_OBJECT))
								{
									char * family = root->u.object.values[i].name;
									if(family)
									{
										for(int j = 0; j < u->u.object.length; j++)
										{
											char * style = u->u.object.values[j].name;
											if(style)
											{
												struct json_value_t * v = u->u.object.values[j].value;
												if(v && (v->type == JSON_STRING))
												{
													char path[512];
													xos_snprintf(path, sizeof(path), "/romdisk/assets/fonts/%s", (char *)v->u.string.ptr);
													if(xos_strcmp(style, "regular") == 0)
														font_install_from_xfs(family, FONT_STYLE_REGULAR, ctx, path);
													else if(xos_strcmp(style, "italic") == 0)
														font_install_from_xfs(family, FONT_STYLE_ITALIC, ctx, path);
													else if(xos_strcmp(style, "bold") == 0)
														font_install_from_xfs(family, FONT_STYLE_BOLD, ctx, path);
													else if(xos_strcmp(style, "bolditalic") == 0)
														font_install_from_xfs(family, FONT_STYLE_BOLDITALIC, ctx, path);
												}
											}
										}
									}
								}
							}
						}
						json_free(root);
					}
					xos_mem_free(json);
				}
			}
			xfs_close(file);
		}
		xfs_free(ctx);
	}
}

/*
 * logo
 */
static void do_show_logo(void)
{
	struct device_t * pos, * n;

	list_for_each_entry_safe(pos, n, &__device_head[DEVICE_TYPE_FRAMEBUFFER], head)
	{
		struct framebuffer_t * fb = (struct framebuffer_t *)(pos->priv);
		if(fb)
		{
			struct window_t * w = window_alloc(fb->name, "", -1);
			if(w)
			{
				window_dirtylist_fullscreen(w);
				window_present_clear(w);
				{
					struct surface_t * logo = NULL;
					struct xfs_context_t * ctx = xfs_alloc();
					if(ctx)
					{
						logo = surface_alloc_from_xfs(ctx, "/romdisk/assets/images/logo.png");
						xfs_free(ctx);
					}
					if(logo)
					{
						struct matrix2d_t m;
						matrix2d_init_identity(&m);
						surface_fill(window_get_surface(w), NULL, &m, window_get_width(w), window_get_height(w), &(struct color_t){0x00, 0x00, 0x00, 0xff});
						matrix2d_init_translate(&m, ((window_get_width(w) - surface_get_width(logo)) >> 1), ((window_get_height(w) - surface_get_height(logo)) >> 1));
						surface_blit(window_get_surface(w), NULL, &m, logo);
						surface_free(logo);
					}
					else
					{
						struct cg_ctx_t * cg = surface_get_cg_ctx(window_get_surface(w));
						int ww = window_get_width(w);
						int wh = window_get_height(w);
						float sw = (float)ww / 256.0f * 0.382f;
						float sh = (float)wh / 128.0f * 0.382f;
						float scale = (sw < sh) ? sw : sh;
						float tx = ((float)ww - scale * 256.0f) * 0.5f;
						float ty = ((float)wh - scale * 128.0f) * 0.5f;
						cg_save(cg);
						cg_set_source_rgba(cg, 0.2, 0.4, 0.6, 1.0);
						cg_paint(cg);
						cg_set_source_rgba(cg, 1.0, 1.0, 1.0, 1.0);
						cg_identity_matrix(cg);
						cg_translate(cg, tx, ty);
						cg_scale(cg, scale, scale);
						cg_move_to(cg, 18.2852, 0);
						cg_arc_to(cg, 18.285837, 17.066174, 0, 0, 0, 0, 17.0664);
						cg_line_to(cg, 0, 110.93);
						cg_rel_arc_to(cg, 18.285837, 17.066174, 0, 0, 0, 18.2852, 17.066);
						cg_line_to(cg, 237.717, 127.996);
						cg_arc_to(cg, 18.285837, 17.066174, 0, 0, 0, 256.002, 110.93);
						cg_line_to(cg, 256.002, 17.0664);
						cg_arc_to(cg, 18.285837, 17.066174, 0, 0, 0, 237.717, 0);
						cg_close_path(cg);
						cg_rel_move_to(cg, 1.83, 6.82617);
						cg_line_to(cg, 235.887, 6.82617);
						cg_arc_to(cg, 12.800087, 11.946321, 0, 0, 1, 248.688, 18.7734);
						cg_line_to(cg, 248.688, 99.8379);
						cg_line_to(cg, 7.31445, 99.8379);
						cg_line_to(cg, 7.31445, 18.7734);
						cg_arc_to(cg, 12.800087, 11.946321, 0, 0, 1, 20.1152, 6.82617);
						cg_close_path(cg);
						cg_move_to(cg, 54.6621, 105.846);
						cg_rel_cubic_to(cg, 3.1871, 0, 5.6743, 0.757, 7.4629, 2.275);
						cg_rel_cubic_to(cg, 1.7981, 1.518, 2.6973, 3.602, 2.6973, 6.25);
						cg_rel_cubic_to(cg, 0, 1.739, -0.4126, 3.268, -1.2402, 4.588);
						cg_rel_cubic_to(cg, -0.8277, 1.32, -2.0135, 2.332, -3.5547, 3.035);
						cg_rel_cubic_to(cg, -1.5412, 0.696, -3.3396, 1.045, -5.3945, 1.045);
						cg_rel_cubic_to(cg, -3.1585, 0, -5.6355, -0.771, -7.4336, -2.312);
						cg_rel_cubic_to(cg, -1.7886, -1.549, -2.6836, -3.668, -2.6836, -6.355);
						cg_rel_cubic_to(cg, 0, -2.68, 0.895, -4.771, 2.6836, -6.273);
						cg_rel_cubic_to(cg, 1.7886, -1.502, 4.2758, -2.252, 7.4629, -2.252);
						cg_close_path(cg);
						cg_rel_move_to(cg, -29.9805, 0.248);
						cg_rel_line_to(cg, 9.0332, 0);
						cg_rel_cubic_to(cg, 2.4069, 0, 4.2569, 0.462, 5.5508, 1.387);
						cg_rel_cubic_to(cg, 1.3034, 0.917, 1.9551, 2.219, 1.9551, 3.902);
						cg_rel_cubic_to(cg, 0, 1.075, -0.2949, 2.035, -0.8848, 2.881);
						cg_rel_cubic_to(cg, -0.5898, 0.846, -1.4355, 1.502, -2.5391, 1.969);
						cg_rel_cubic_to(cg, -1.0941, 0.459, -2.3975, 0.688, -3.9102, 0.688);
						cg_rel_line_to(cg, -4.9941, 0);
						cg_rel_line_to(cg, 0, 5.881);
						cg_rel_line_to(cg, -4.2109, 0);
						cg_close_path(cg);
						cg_rel_move_to(cg, 42.4941, 0);
						cg_rel_line_to(cg, 4.2676, 0);
						cg_rel_line_to(cg, 2.9102, 10.791);
						cg_rel_line_to(cg, 0.6562, 2.607);
						cg_rel_cubic_to(cg, 0.2664, -1.099, 0.5241, -2.145, 0.7715, -3.141);
						cg_rel_cubic_to(cg, 0.2569, -1.004, 1.2077, -4.424, 2.8535, -10.258);
						cg_rel_line_to(cg, 4.709, 0);
						cg_rel_line_to(cg, 2.541, 8.893);
						cg_rel_cubic_to(cg, 0.1998, 0.664, 0.536, 2.166, 1.0117, 4.506);
						cg_rel_line_to(cg, 0.3574, -1.375);
						cg_rel_line_to(cg, 0.7559, -2.727);
						cg_rel_line_to(cg, 2.4258, -9.297);
						cg_rel_line_to(cg, 4.2676, 0);
						cg_line_to(cg, 89.5078, 122.8);
						cg_rel_line_to(cg, -4.9941, 0);
						cg_rel_line_to(cg, -2.7246, -9.664);
						cg_rel_cubic_to(cg, -0.333, -1.138, -0.6135, -2.327, -0.8418, -3.568);
						cg_rel_cubic_to(cg, -0.2283, 1.036, -0.4139, 1.826, -0.5566, 2.371);
						cg_rel_cubic_to(cg, -0.1427, 0.538, -1.1573, 4.158, -3.041, 10.861);
						cg_rel_line_to(cg, -4.9941, 0);
						cg_close_path(cg);
						cg_rel_move_to(cg, 30.6074, 0);
						cg_rel_line_to(cg, 15.8129, 0);
						cg_rel_line_to(cg, 0, 2.703);
						cg_rel_line_to(cg, -11.602, 0);
						cg_rel_line_to(cg, 0, 4.197);
						cg_rel_line_to(cg, 10.73, 0);
						cg_rel_line_to(cg, 0, 2.705);
						cg_rel_line_to(cg, -10.73, 0);
						cg_rel_line_to(cg, 0, 4.398);
						cg_line_to(cg, 114.18, 120.097);
						cg_line_to(cg, 114.18, 122.8);
						cg_line_to(cg, 97.7832, 122.8);
						cg_close_path(cg);
						cg_rel_move_to(cg, 20.5899, 0);
						cg_rel_line_to(cg, 10.047, 0);
						cg_rel_cubic_to(cg, 2.397, 0, 4.247, 0.431, 5.551, 1.293);
						cg_rel_cubic_to(cg, 1.303, 0.854, 1.955, 2.083, 1.955, 3.688);
						cg_rel_cubic_to(cg, 0, 1.17, -0.398, 2.181, -1.197, 3.035);
						cg_rel_cubic_to(cg, -0.799, 0.846, -1.88, 1.403, -3.24, 1.672);
						cg_rel_line_to(cg, 5.438, 7.02);
						cg_rel_line_to(cg, -4.738, 0);
						cg_rel_line_to(cg, -4.666, -6.344);
						cg_rel_line_to(cg, -4.938, 0);
						cg_rel_line_to(cg, 0, 6.344);
						cg_rel_line_to(cg, -4.211, 0);
						cg_close_path(cg);
						cg_rel_move_to(cg, 22.203, 0);
						cg_rel_line_to(cg, 15.812, 0);
						cg_rel_line_to(cg, 0, 2.703);
						cg_rel_line_to(cg, -11.602, 0);
						cg_rel_line_to(cg, 0, 4.197);
						cg_rel_line_to(cg, 10.73, 0);
						cg_rel_line_to(cg, 0, 2.705);
						cg_rel_line_to(cg, -10.73, 0);
						cg_rel_line_to(cg, 0, 4.398);
						cg_rel_line_to(cg, 12.186, 0);
						cg_rel_line_to(cg, 0, 2.703);
						cg_rel_line_to(cg, -16.396, 0);
						cg_close_path(cg);
						cg_rel_move_to(cg, 20.592, 0);
						cg_rel_line_to(cg, 7.277, 0);
						cg_rel_cubic_to(cg, 3.387, 0, 6.008, 0.712, 7.863, 2.135);
						cg_rel_cubic_to(cg, 1.855, 1.415, 2.781, 3.446, 2.781, 6.094);
						cg_rel_cubic_to(cg, 0, 1.723, -0.408, 3.231, -1.227, 4.52);
						cg_rel_cubic_to(cg, -0.809, 1.281, -1.959, 2.26, -3.453, 2.939);
						cg_rel_cubic_to(cg, -1.484, 0.68, -3.188, 1.02, -5.109, 1.02);
						cg_rel_line_to(cg, -8.133, 0);
						cg_close_path(cg);
						cg_rel_move_to(cg, 31.42, 0);
						cg_rel_line_to(cg, 9.203, 0);
						cg_rel_cubic_to(cg, 2.454, 0, 4.311, 0.355, 5.566, 1.066);
						cg_rel_cubic_to(cg, 1.265, 0.704, 1.896, 1.747, 1.896, 3.131);
						cg_rel_cubic_to(cg, 0, 0.949, -0.318, 1.752, -0.955, 2.408);
						cg_rel_cubic_to(cg, -0.628, 0.648, -1.589, 1.087, -2.883, 1.316);
						cg_rel_cubic_to(cg, 1.627, 0.158, 2.864, 0.584, 3.711, 1.279);
						cg_rel_cubic_to(cg, 0.856, 0.688, 1.283, 1.602, 1.283, 2.74);
						cg_rel_cubic_to(cg, 0, 1.518, -0.685, 2.691, -2.055, 3.521);
						cg_rel_cubic_to(cg, -1.37, 0.83, -3.272, 1.244, -5.707, 1.244);
						cg_rel_line_to(cg, -10.061, 0);
						cg_close_path(cg);
						cg_rel_move_to(cg, 20.746, 0);
						cg_rel_line_to(cg, 4.41, 0);
						cg_rel_line_to(cg, 4.822, 7.066);
						cg_rel_line_to(cg, 4.881, -7.066);
						cg_rel_line_to(cg, 4.41, 0);
						cg_rel_line_to(cg, -7.164, 9.854);
						cg_rel_line_to(cg, 0, 6.854);
						cg_rel_line_to(cg, -4.195, 0);
						cg_rel_line_to(cg, 0, -6.854);
						cg_close_path(cg);
						cg_rel_move_to(cg, -158.6719, 2.502);
						cg_rel_cubic_to(cg, -1.8837, 0, -3.3397, 0.509, -4.3672, 1.529);
						cg_rel_cubic_to(cg, -1.0275, 1.012, -1.541, 2.428, -1.541, 4.246);
						cg_rel_cubic_to(cg, 0, 1.834, 0.5238, 3.281, 1.5703, 4.34);
						cg_rel_cubic_to(cg, 1.056, 1.051, 2.4915, 1.576, 4.3086, 1.576);
						cg_rel_cubic_to(cg, 1.8837, 0, 3.3356, -0.513, 4.3535, -1.541);
						cg_rel_cubic_to(cg, 1.0275, -1.028, 1.541, -2.486, 1.541, -4.375);
						cg_rel_cubic_to(cg, 0, -1.802, -0.5136, -3.215, -1.541, -4.234);
						cg_rel_cubic_to(cg, -1.0275, -1.028, -2.4691, -1.541, -4.3242, -1.541);
						cg_close_path(cg);
						cg_rel_move_to(cg, 142.1349, 0.095);
						cg_rel_line_to(cg, 0, 4.137);
						cg_rel_line_to(cg, 4.824, 0);
						cg_rel_cubic_to(cg, 1.189, 0, 2.049, -0.17, 2.582, -0.51);
						cg_rel_cubic_to(cg, 0.542, -0.348, 0.813, -0.865, 0.813, -1.553);
						cg_rel_cubic_to(cg, 0, -0.751, -0.289, -1.285, -0.869, -1.602);
						cg_rel_cubic_to(cg, -0.571, -0.316, -1.423, -0.473, -2.555, -0.473);
						cg_close_path(cg);
						cg_rel_move_to(cg, -31.42, 0.105);
						cg_rel_line_to(cg, 0, 11.301);
						cg_rel_line_to(cg, 3.568, 0);
						cg_rel_cubic_to(cg, 1.808, 0, 3.243, -0.517, 4.309, -1.553);
						cg_rel_cubic_to(cg, 1.066, -1.036, 1.598, -2.444, 1.598, -4.223);
						cg_rel_cubic_to(cg, 0, -1.794, -0.561, -3.161, -1.684, -4.102);
						cg_rel_cubic_to(cg, -1.123, -0.949, -2.725, -1.424, -4.809, -1.424);
						cg_close_path(cg);
						cg_rel_move_to(cg, -136.484, 0.012);
						cg_rel_line_to(cg, 0, 5.42);
						cg_rel_line_to(cg, 4.4668, 0);
						cg_rel_cubic_to(cg, 1.1607, 0, 2.0537, -0.237, 2.6816, -0.711);
						cg_rel_cubic_to(cg, 0.6279, -0.482, 0.9414, -1.175, 0.9414, -2.076);
						cg_rel_cubic_to(cg, 0, -1.755, -1.2457, -2.633, -3.7383, -2.633);
						cg_close_path(cg);
						cg_rel_move_to(cg, 93.691, 0);
						cg_rel_line_to(cg, 0, 4.934);
						cg_rel_line_to(cg, 5.508, 0);
						cg_rel_cubic_to(cg, 1.18, 0, 2.075, -0.221, 2.684, -0.664);
						cg_rel_cubic_to(cg, 0.609, -0.443, 0.912, -1.063, 0.912, -1.861);
						cg_rel_cubic_to(cg, 0, -1.605, -1.235, -2.408, -3.709, -2.408);
						cg_close_path(cg);
						cg_rel_move_to(cg, 74.213, 6.605);
						cg_rel_line_to(cg, 0, 4.791);
						cg_rel_line_to(cg, 5.494, 0);
						cg_rel_cubic_to(cg, 1.351, 0, 2.335, -0.202, 2.953, -0.605);
						cg_rel_cubic_to(cg, 0.628, -0.411, 0.943, -1.024, 0.943, -1.838);
						cg_rel_cubic_to(cg, 0, -1.565, -1.351, -2.348, -4.053, -2.348);
						cg_close_path(cg);
						cg_fill(cg);
						cg_new_path(cg);
						cg_move_to(cg, 49.0127, 79.6412);
						cg_line_to(cg, 36.5629, 61.2274);
						cg_line_to(cg, 24.1131, 79.6412);
						cg_line_to(cg, 13.1404, 79.6412);
						cg_line_to(cg, 30.3028, 55.3193);
						cg_line_to(cg, 14.5824, 33.3934);
						cg_line_to(cg, 25.5551, 33.3934);
						cg_line_to(cg, 36.563, 49.7393);
						cg_line_to(cg, 47.5709, 33.3934);
						cg_line_to(cg, 58.4731, 33.3934);
						cg_line_to(cg, 43.4208, 55.3193);
						cg_line_to(cg, 59.915, 79.6412);
						cg_close_path(cg);
						cg_move_to(cg, 103.705, 66.315);
						cg_rel_quad_to(cg, 0, 6.7944, -5.416, 10.4049);
						cg_rel_quad_to(cg, -5.3808, 3.5777, -15.826, 3.5777);
						cg_rel_quad_to(cg, -9.5308, 0, -14.9468, -3.151);
						cg_rel_quad_to(cg, -5.416, -3.151, -6.9634, -9.5515);
						cg_rel_line_to(cg, 10.0231, -1.5427);
						cg_rel_quad_to(cg, 1.0199, 3.6762, 3.9741, 5.3502);
						cg_rel_quad_to(cg, 2.9542, 1.6412, 8.1943, 1.6412);
						cg_rel_quad_to(cg, 10.8672, 0, 10.8672, -6.1707);
						cg_rel_quad_to(cg, 0, -1.9694, -1.2661, -3.2495);
						cg_rel_quad_to(cg, -1.2309, -1.2801, -3.5169, -2.1335);
						cg_rel_quad_to(cg, -2.2508, -0.8534, -8.6867, -2.0679);
						cg_rel_quad_to(cg, -5.5567, -1.2145, -7.7371, -1.9366);
						cg_rel_quad_to(cg, -2.1805, -0.7549, -3.9389, -1.7396);
						cg_rel_quad_to(cg, -1.7584, -1.0175, -2.9894, -2.4289);
						cg_rel_quad_to(cg, -1.2309, -1.4114, -1.9343, -3.3151);
						cg_rel_quad_to(cg, -0.6682, -1.9037, -0.6682, -4.3655);
						cg_rel_quad_to(cg, 0, -6.2692, 5.0291, -9.5843);
						cg_rel_quad_to(cg, 5.0643, -3.348, 14.7006, -3.348);
						cg_rel_quad_to(cg, 9.2142, 0, 13.8214, 2.6915);
						cg_rel_quad_to(cg, 4.642, 2.6915, 5.979, 8.8951);
						cg_rel_line_to(cg, -10.0583, 1.2801);
						cg_rel_quad_to(cg, -0.7737, -2.9869, -3.1652, -4.4968);
						cg_rel_quad_to(cg, -2.3563, -1.5099, -6.7876, -1.5099);
						cg_rel_quad_to(cg, -9.4253, 0, -9.4253, 5.5143);
						cg_rel_quad_to(cg, 0, 1.8053, 0.9847, 2.9541);
						cg_rel_quad_to(cg, 1.0199, 1.1488, 2.9894, 1.9694);
						cg_rel_quad_to(cg, 1.9695, 0.7878, 7.9833, 2.0022);
						cg_rel_quad_to(cg, 7.1393, 1.4114, 10.199, 2.6258);
						cg_rel_quad_to(cg, 3.0949, 1.1816, 4.888, 2.79);
						cg_rel_quad_to(cg, 1.794, 1.5755, 2.743, 3.8075);
						cg_rel_quad_to(cg, 0.95, 2.1991, 0.95, 5.0876);
						cg_close_path(cg);
						cg_move_to(cg, 131.634, 40.8771);
						cg_line_to(cg, 131.634, 79.6412);
						cg_line_to(cg, 121.259, 79.6412);
						cg_line_to(cg, 121.259, 40.8771);
						cg_rel_line_to(cg, -16.002, 0);
						cg_rel_line_to(cg, 0, -7.4837);
						cg_rel_line_to(cg, 42.414, 0);
						cg_rel_line_to(cg, 0, 7.4837);
						cg_close_path(cg);
						cg_rel_move_to(cg, 49.241, 38.7641);
						cg_rel_line_to(cg, -4.396, -11.8163);
						cg_rel_line_to(cg, -18.886, 0);
						cg_rel_line_to(cg, -4.396, 11.8163);
						cg_rel_line_to(cg, -10.375, 0);
						cg_rel_line_to(cg, 18.077, -46.2478);
						cg_rel_line_to(cg, 12.239, 0);
						cg_rel_line_to(cg, 18.006, 46.2477);
						cg_close_path(cg);
						cg_rel_move_to(cg, -13.857, -39.1251);
						cg_rel_line_to(cg, -0.211, 0.7221);
						cg_rel_quad_to(cg, -0.352, 1.1816, -0.844, 2.6915);
						cg_rel_quad_to(cg, -0.492, 1.5099, -6.049, 16.6085);
						cg_rel_line_to(cg, 14.243, 0);
						cg_rel_line_to(cg, -4.888, -13.2934);
						cg_rel_line_to(cg, -1.512, -4.4639);
						cg_close_path(cg);
						cg_rel_move_to(cg, 62.816, 39.1251);
						cg_rel_line_to(cg, -11.5, -17.5604);
						cg_line_to(cg, 206.166, 62.0808);
						cg_line_to(cg, 206.166, 79.6412);
						cg_line_to(cg, 195.791, 79.6412);
						cg_line_to(cg, 195.791, 33.3934);
						cg_rel_line_to(cg, 24.759, 0);
						cg_rel_quad_to(cg, 8.863, 0, 13.681, 3.5778);
						cg_rel_quad_to(cg, 4.818, 3.5449, 4.818, 10.208);
						cg_rel_quad_to(cg, 0, 4.8578, -2.954, 8.4027);
						cg_rel_quad_to(cg, -2.954, 3.5121, -7.983, 4.6281);
						cg_rel_line_to(cg, 13.399, 19.4313);
						cg_close_path(cg);
						cg_move_to(cg, 228.603, 47.573);
						cg_rel_quad_to(cg, 0, -6.6631, -9.144, -6.6631);
						cg_rel_line_to(cg, -13.294, 0);
						cg_rel_line_to(cg, 0, 13.6544);
						cg_rel_line_to(cg, 13.575, 0);
						cg_rel_quad_to(cg, 4.361, 0, 6.612, -1.8381);
						cg_rel_quad_to(cg, 2.251, -1.8381, 2.251, -5.1532);
						cg_close_path(cg);
						cg_fill(cg);
						cg_restore(cg);
					}
				}
				window_present_commit(w);
				char key[256];
				xos_sprintf(key, "backlight(%s)", w->fb->name);
				int brightness = xos_strtol(setting_get(key, "-1"), NULL, 0);
				if(brightness <= 0)
					brightness = 618;
				window_set_backlight(w, brightness);
				window_free(w);
			}
		}
	}
}

void xstar_init(struct xos_environ_t * env, const char * dtree)
{
	/* Initial environ */
	xos_environ_init(env);

	/* Do initial calls */
	do_initcalls();

	/* Do initial romdisk */
	do_init_romdisk();

	/* Do initial dtree */
	do_init_dtree(dtree);

	/* Do initial wallclock */
	do_init_wallclock();

	/* Do initial memory */
	do_init_memory();

	/* Do initial logger */
	do_init_logger();

	/* Do initial version */
	do_init_version();

	/* Do initial copyright */
	do_init_copyright();

	/* Do initial random */
	do_init_random();

	/* Do initial feature */
	do_init_feature();

	/* Do initial font */
	do_init_font();

	/* Do initial setting */
	do_init_setting();

	/* Do initial final calls */
	do_init_final();

	/* Do show logo */
	do_show_logo();
}

void xstar_exit(void)
{
	xstar_sync();
	do_exit_final();
	do_exitcalls();
}

void xstar_sync(void)
{
	struct device_t * pos, * n;

	setting_sync();
	list_for_each_entry_safe(pos, n, &__device_head[DEVICE_TYPE_BLOCK], head)
	{
		block_sync((struct block_t *)(pos->priv));
	}
}

void xstar_shutdown(void)
{
	xstar_sync();
	xos_pm_shutdown();
}

void xstar_reboot(void)
{
	xstar_sync();
	xos_pm_reboot();
	watchdog_set_timeout(search_first_watchdog(), 1);
}

void xstar_standby(void)
{
	struct device_t * pos, * n;

	xstar_sync();
	list_for_each_entry_safe_reverse(pos, n, &__device_list, list)
	{
		suspend_device(pos);
	}
	xos_pm_standby();
	list_for_each_entry_safe(pos, n, &__device_list, list)
	{
		resume_device(pos);
	}
}
