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

#include <xui-text.h>

void xui_text(struct xui_context_t * ctx, const char * utf8)
{
	struct region_t * r;
	int width, height;

	font_text_bound(ctx->style.font.font_family, ctx->style.font.style, ctx->style.font.size, xui_get_layout(ctx)->body.w, utf8, &width, &height);
	xui_layout_row(ctx, 1, (int[]){ -1 }, height);
	r = xui_layout_next(ctx);
	xui_draw_text(ctx, r->x, r->y, ctx->style.font.font_family, ctx->style.font.style, ctx->style.font.size, xui_get_layout(ctx)->body.w, utf8, &ctx->style.font.color);
}
