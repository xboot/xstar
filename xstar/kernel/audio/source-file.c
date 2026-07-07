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

#include <kernel/audio/source.h>

extern struct audio_source_t * audio_source_alloc_from_xfs_wav(struct xfs_context_t * ctx, const char * filename);
extern struct audio_source_t * audio_source_alloc_from_xfs_qoa(struct xfs_context_t * ctx, const char * filename);

struct audio_source_t * audio_source_alloc_from_xfs(struct xfs_context_t * ctx, const char * filename)
{
	const char * ext = path_fileext(filename);

	if(ext)
	{
		if(xos_strcasecmp(ext, "wav") == 0)
			return audio_source_alloc_from_xfs_wav(ctx, filename);
		else if(xos_strcasecmp(ext, "qoa") == 0)
			return audio_source_alloc_from_xfs_qoa(ctx, filename);
	}
	return NULL;
}
