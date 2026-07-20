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
#include <f133/f133-gpio.h>
#include <f133/reg-de.h>
#include <f133/reg-tconlcd.h>

struct fb_f133_rgb_pdata_t {
	io_addr_t addr_de;
	io_addr_t addr_tconlcd;

	char * clk_de;
	char * clk_tconlcd;
	int rst_de;
	int rst_tconlcd;
	int width;
	int height;
	int pwidth;
	int pheight;
	int bits_per_pixel;
	int bytes_per_pixel;
	int pixlen;
	int index;
	void * vram[2];
	struct dirtylist_t * nl, * ol;

	struct {
		int pixel_clock_hz;
		int h_front_porch;
		int h_back_porch;
		int h_sync_len;
		int v_front_porch;
		int v_back_porch;
		int v_sync_len;
		int h_sync_active;
		int v_sync_active;
		int den_active;
		int clk_active;
	} timing;

	struct led_t * backlight;
	int brightness;
};

static void inline f133_de_enable(struct fb_f133_rgb_pdata_t * pdat)
{
	struct de_glb_t * glb = (struct de_glb_t *)(pdat->addr_de + F133_DE_MUX_GLB);
	xos_io_write32((io_addr_t)&glb->dbuff, 1);
}

static inline void f133_de_set_address(struct fb_f133_rgb_pdata_t * pdat, void * vram)
{
	struct de_ui_t * ui = (struct de_ui_t *)(pdat->addr_de + F133_DE_MUX_CHAN + 0x1000 * 1);
	xos_io_write32((io_addr_t)&ui->cfg[0].top_laddr, (uint32_t)(unsigned long)vram);
}

static inline void f133_de_set_mode(struct fb_f133_rgb_pdata_t * pdat)
{
	struct de_clk_t * clk = (struct de_clk_t *)(pdat->addr_de);
	struct de_glb_t * glb = (struct de_glb_t *)(pdat->addr_de + F133_DE_MUX_GLB);
	struct de_bld_t * bld = (struct de_bld_t *)(pdat->addr_de + F133_DE_MUX_BLD);
	struct de_ui_t * ui = (struct de_ui_t *)(pdat->addr_de + F133_DE_MUX_CHAN + 0x1000 * 1);
	uint32_t size = (((pdat->height - 1) << 16) | (pdat->width - 1));
	uint32_t val;
	int i;

	val = xos_io_read32((io_addr_t)&clk->rst_cfg);
	val |= 1 << 0;
	xos_io_write32((io_addr_t)&clk->rst_cfg, val);

	val = xos_io_read32((io_addr_t)&clk->gate_cfg);
	val |= 1 << 0;
	xos_io_write32((io_addr_t)&clk->gate_cfg, val);

	val = xos_io_read32((io_addr_t)&clk->bus_cfg);
	val |= 1 << 0;
	xos_io_write32((io_addr_t)&clk->bus_cfg, val);

	val = xos_io_read32((io_addr_t)&clk->sel_cfg);
	val &= ~(1 << 0);
	xos_io_write32((io_addr_t)&clk->sel_cfg, val);

	xos_io_write32((io_addr_t)&glb->ctl, (1 << 0));
	xos_io_write32((io_addr_t)&glb->status, 0);
	xos_io_write32((io_addr_t)&glb->dbuff, 1);
	xos_io_write32((io_addr_t)&glb->size, size);

	for(i = 0; i < 4; i++)
	{
		void * chan = (void *)(pdat->addr_de + F133_DE_MUX_CHAN + 0x1000 * i);
		xos_memset(chan, 0, i == 0 ? sizeof(struct de_vi_t) : sizeof(struct de_ui_t));
	}
	xos_memset(bld, 0, sizeof(struct de_bld_t));

	xos_io_write32((io_addr_t)&bld->fcolor_ctl, 0x00000101);
	xos_io_write32((io_addr_t)&bld->route, 1);
	xos_io_write32((io_addr_t)&bld->premultiply, 0);
	xos_io_write32((io_addr_t)&bld->bkcolor, 0xff000000);
	xos_io_write32((io_addr_t)&bld->bld_mode[0], 0x03010301);
	xos_io_write32((io_addr_t)&bld->bld_mode[1], 0x03010301);
	xos_io_write32((io_addr_t)&bld->output_size, size);
	xos_io_write32((io_addr_t)&bld->out_ctl, 0);
	xos_io_write32((io_addr_t)&bld->ck_ctl, 0);
	for(i = 0; i < 4; i++)
	{
		xos_io_write32((io_addr_t)&bld->attr[i].fcolor, 0xff000000);
		xos_io_write32((io_addr_t)&bld->attr[i].insize, size);
	}

	xos_io_write32(pdat->addr_de + F133_DE_MUX_VSU, 0);
	xos_io_write32(pdat->addr_de + F133_DE_MUX_GSU1, 0);
	xos_io_write32(pdat->addr_de + F133_DE_MUX_GSU2, 0);
	xos_io_write32(pdat->addr_de + F133_DE_MUX_GSU3, 0);
	xos_io_write32(pdat->addr_de + F133_DE_MUX_FCE, 0);
	xos_io_write32(pdat->addr_de + F133_DE_MUX_BWS, 0);
	xos_io_write32(pdat->addr_de + F133_DE_MUX_LTI, 0);
	xos_io_write32(pdat->addr_de + F133_DE_MUX_PEAK, 0);
	xos_io_write32(pdat->addr_de + F133_DE_MUX_ASE, 0);
	xos_io_write32(pdat->addr_de + F133_DE_MUX_FCC, 0);
	xos_io_write32(pdat->addr_de + F133_DE_MUX_DCSC, 0);

	xos_io_write32((io_addr_t)&ui->cfg[0].attr, (1 << 0) | (4 << 8) | (1 << 1) | (0xff << 24));
	xos_io_write32((io_addr_t)&ui->cfg[0].size, size);
	xos_io_write32((io_addr_t)&ui->cfg[0].coord, 0);
	xos_io_write32((io_addr_t)&ui->cfg[0].pitch, 4 * pdat->width);
	xos_io_write32((io_addr_t)&ui->cfg[0].top_laddr, (uint32_t)(unsigned long)pdat->vram[pdat->index]);
	xos_io_write32((io_addr_t)&ui->ovl_size, size);
}

static void f133_tconlcd_enable(struct fb_f133_rgb_pdata_t * pdat)
{
	struct f133_tconlcd_reg_t * tcon = (struct f133_tconlcd_reg_t *)pdat->addr_tconlcd;
	uint32_t val;

	val = xos_io_read32((io_addr_t)&tcon->gctrl);
	val |= (1 << 31);
	xos_io_write32((io_addr_t)&tcon->gctrl, val);
}

static void f133_tconlcd_disable(struct fb_f133_rgb_pdata_t * pdat)
{
	struct f133_tconlcd_reg_t * tcon = (struct f133_tconlcd_reg_t *)pdat->addr_tconlcd;
	uint32_t val;

	val = xos_io_read32((io_addr_t)&tcon->dclk);
	val &= ~(0xf << 28);
	xos_io_write32((io_addr_t)&tcon->dclk, val);

	xos_io_write32((io_addr_t)&tcon->gctrl, 0);
	xos_io_write32((io_addr_t)&tcon->gint0, 0);
}

static void f133_tconlcd_set_timing(struct fb_f133_rgb_pdata_t * pdat)
{
	struct f133_tconlcd_reg_t * tcon = (struct f133_tconlcd_reg_t *)pdat->addr_tconlcd;
	int bp, total;
	uint32_t val;

	val = (pdat->timing.v_front_porch + pdat->timing.v_back_porch + pdat->timing.v_sync_len) / 2;
	xos_io_write32((io_addr_t)&tcon->ctrl, (1 << 31) | (0 << 24) | (0 << 23) | ((val & 0x1f) << 4) | (0 << 0));
	val = clk_get_rate(pdat->clk_tconlcd) / pdat->timing.pixel_clock_hz;
	xos_io_write32((io_addr_t)&tcon->dclk, (0xf << 28) | (val << 0));
	xos_io_write32((io_addr_t)&tcon->timing0, ((pdat->width - 1) << 16) | ((pdat->height - 1) << 0));
	bp = pdat->timing.h_sync_len + pdat->timing.h_back_porch;
	total = pdat->width + pdat->timing.h_front_porch + bp;
	xos_io_write32((io_addr_t)&tcon->timing1, ((total - 1) << 16) | ((bp - 1) << 0));
	bp = pdat->timing.v_sync_len + pdat->timing.v_back_porch;
	total = pdat->height + pdat->timing.v_front_porch + bp;
	xos_io_write32((io_addr_t)&tcon->timing2, ((total * 2) << 16) | ((bp - 1) << 0));
	xos_io_write32((io_addr_t)&tcon->timing3, ((pdat->timing.h_sync_len - 1) << 16) | ((pdat->timing.v_sync_len - 1) << 0));

	val = (0 << 31) | (1 << 28);
	if(!pdat->timing.h_sync_active)
		val |= (1 << 25);
	if(!pdat->timing.v_sync_active)
		val |= (1 << 24);
	if(!pdat->timing.den_active)
		val |= (1 << 27);
	if(!pdat->timing.clk_active)
		val |= (1 << 26);
	xos_io_write32((io_addr_t)&tcon->io_polarity, val);
	xos_io_write32((io_addr_t)&tcon->io_tristate, 0);
}

static void f133_tconlcd_set_dither(struct fb_f133_rgb_pdata_t * pdat)
{
	struct f133_tconlcd_reg_t * tcon = (struct f133_tconlcd_reg_t *)pdat->addr_tconlcd;

	if((pdat->bits_per_pixel == 16) || (pdat->bits_per_pixel == 18))
	{
		xos_io_write32((io_addr_t)&tcon->frm_seed[0], 0x11111111);
		xos_io_write32((io_addr_t)&tcon->frm_seed[1], 0x11111111);
		xos_io_write32((io_addr_t)&tcon->frm_seed[2], 0x11111111);
		xos_io_write32((io_addr_t)&tcon->frm_seed[3], 0x11111111);
		xos_io_write32((io_addr_t)&tcon->frm_seed[4], 0x11111111);
		xos_io_write32((io_addr_t)&tcon->frm_seed[5], 0x11111111);
		xos_io_write32((io_addr_t)&tcon->frm_table[0], 0x01010000);
		xos_io_write32((io_addr_t)&tcon->frm_table[1], 0x15151111);
		xos_io_write32((io_addr_t)&tcon->frm_table[2], 0x57575555);
		xos_io_write32((io_addr_t)&tcon->frm_table[3], 0x7f7f7777);

		if(pdat->bits_per_pixel == 16)
			xos_io_write32((io_addr_t)&tcon->frm_ctrl, (1 << 31) | (1 << 6) | (0 << 5)| (1 << 4));
		else if(pdat->bits_per_pixel == 18)
			xos_io_write32((io_addr_t)&tcon->frm_ctrl, (1 << 31) | (0 << 6) | (0 << 5)| (0 << 4));
	}
}

static void fb_f133_cfg_gpios(int base, int n, int cfg, enum gpio_pull_t pull, enum gpio_drv_t drv)
{
	for(; n > 0; n--, base++)
	{
		gpio_set_cfg(base, cfg);
		gpio_set_pull(base, pull);
		gpio_set_drv(base, drv);
	}
}

static void fb_f133_rgb_init(struct fb_f133_rgb_pdata_t * pdat)
{
	if(pdat->bits_per_pixel == 16)
	{
		fb_f133_cfg_gpios(F133_GPIOD1, 5, 0x2, GPIO_PULL_NONE, GPIO_DRV_STRONG);
		fb_f133_cfg_gpios(F133_GPIOD6, 6, 0x2, GPIO_PULL_NONE, GPIO_DRV_STRONG);
		fb_f133_cfg_gpios(F133_GPIOD13, 5, 0x2, GPIO_PULL_NONE, GPIO_DRV_STRONG);
		fb_f133_cfg_gpios(F133_GPIOD18, 4, 0x2, GPIO_PULL_NONE, GPIO_DRV_STRONG);
	}
	else if(pdat->bits_per_pixel == 18)
	{
		fb_f133_cfg_gpios(F133_GPIOD0, 6, 0x2, GPIO_PULL_NONE, GPIO_DRV_STRONG);
		fb_f133_cfg_gpios(F133_GPIOD6, 6, 0x2, GPIO_PULL_NONE, GPIO_DRV_STRONG);
		fb_f133_cfg_gpios(F133_GPIOD12, 6, 0x2, GPIO_PULL_NONE, GPIO_DRV_STRONG);
		fb_f133_cfg_gpios(F133_GPIOD18, 4, 0x2, GPIO_PULL_NONE, GPIO_DRV_STRONG);
	}

	f133_tconlcd_disable(pdat);
	f133_tconlcd_set_timing(pdat);
	f133_tconlcd_set_dither(pdat);
	f133_tconlcd_enable(pdat);
	f133_de_set_mode(pdat);
	f133_de_enable(pdat);
	f133_de_set_address(pdat, pdat->vram[pdat->index]);
	f133_de_enable(pdat);
}

static void fb_setbl(struct framebuffer_t * fb, int brightness)
{
	struct fb_f133_rgb_pdata_t * pdat = (struct fb_f133_rgb_pdata_t *)fb->priv;
	led_set_brightness(pdat->backlight, brightness);
}

static int fb_getbl(struct framebuffer_t * fb)
{
	struct fb_f133_rgb_pdata_t * pdat = (struct fb_f133_rgb_pdata_t *)fb->priv;
	return led_get_brightness(pdat->backlight);
}

static struct surface_t * fb_create(struct framebuffer_t * fb, int width, int height)
{
	return surface_alloc(width, height);
}

static void fb_destroy(struct framebuffer_t * fb, struct surface_t * s)
{
	surface_free(s);
}

static void fb_present(struct framebuffer_t * fb, struct surface_t * s, struct dirtylist_t * l)
{
	struct fb_f133_rgb_pdata_t * pdat = (struct fb_f133_rgb_pdata_t *)fb->priv;
	struct dirtylist_t * nl = pdat->nl;

	dirtylist_clear(nl);
	dirtylist_merge(nl, pdat->ol);
	dirtylist_merge(nl, l);
	dirtylist_clone(pdat->ol, l);

	pdat->index = (pdat->index + 1) & 0x1;
	present_surface(pdat->vram[pdat->index], s, nl);
	xos_dma_sync(pdat->vram[pdat->index], pdat->pixlen, DMA_SYNC_TO_DEVICE);
	f133_de_set_address(pdat, pdat->vram[pdat->index]);
	f133_de_enable(pdat);
}

static struct device_t * fb_f133_rgb_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct fb_f133_rgb_pdata_t * pdat;
	struct framebuffer_t * fb;
	struct device_t * dev;
	io_addr_t addr = dt_read_address(n);
	char * clk_de = dt_read_string(n, "clock-name-de", NULL);
	char * clk_tconlcd = dt_read_string(n, "clock-name-tconlcd", NULL);

	if(!search_clk(clk_de) || !search_clk(clk_tconlcd))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct fb_f133_rgb_pdata_t));
	if(!pdat)
		return NULL;

	fb = xos_mem_malloc(sizeof(struct framebuffer_t));
	if(!fb)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->addr_de = addr;
	pdat->addr_tconlcd = F133_TCONLCD_BASE;
	pdat->clk_de = xos_strdup(clk_de);
	pdat->clk_tconlcd = xos_strdup(clk_tconlcd);
	pdat->rst_de = dt_read_int(n, "reset-de", -1);
	pdat->rst_tconlcd = dt_read_int(n, "reset-tconlcd", -1);
	pdat->width = dt_read_int(n, "width", 800);
	pdat->height = dt_read_int(n, "height", 480);
	pdat->pwidth = dt_read_int(n, "physical-width", 216);
	pdat->pheight = dt_read_int(n, "physical-height", 135);
	pdat->bits_per_pixel = dt_read_int(n, "bits-per-pixel", 18);
	pdat->bytes_per_pixel = 4;
	pdat->pixlen = pdat->width * pdat->height * pdat->bytes_per_pixel;
	pdat->index = 0;
	pdat->vram[0] = xos_dma_alloc_noncoherent(pdat->pixlen);
	pdat->vram[1] = xos_dma_alloc_noncoherent(pdat->pixlen);
	pdat->nl = dirtylist_alloc(0);
	pdat->ol = dirtylist_alloc(0);
	xos_memset(pdat->vram[0], 0, pdat->pixlen);
	xos_memset(pdat->vram[1], 0, pdat->pixlen);

	pdat->timing.pixel_clock_hz = dt_read_long(n, "clock-frequency", 33000000);
	pdat->timing.h_front_porch = dt_read_int(n, "hfront-porch", 40);
	pdat->timing.h_back_porch = dt_read_int(n, "hback-porch", 87);
	pdat->timing.h_sync_len = dt_read_int(n, "hsync-len", 1);
	pdat->timing.v_front_porch = dt_read_int(n, "vfront-porch", 13);
	pdat->timing.v_back_porch = dt_read_int(n, "vback-porch", 31);
	pdat->timing.v_sync_len = dt_read_int(n, "vsync-len", 1);
	pdat->timing.h_sync_active = dt_read_bool(n, "hsync-active", 0);
	pdat->timing.v_sync_active = dt_read_bool(n, "vsync-active", 0);
	pdat->timing.den_active = dt_read_bool(n, "den-active", 0);
	pdat->timing.clk_active = dt_read_bool(n, "clk-active", 0);
	pdat->backlight = search_led(dt_read_string(n, "backlight", NULL));

	fb->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	fb->width = pdat->width;
	fb->height = pdat->height;
	fb->pwidth = pdat->pwidth;
	fb->pheight = pdat->pheight;
	fb->setbl = fb_setbl;
	fb->getbl = fb_getbl;
	fb->create = fb_create;
	fb->destroy = fb_destroy;
	fb->present = fb_present;
	fb->priv = pdat;

	clk_enable(pdat->clk_de);
	clk_enable(pdat->clk_tconlcd);
	if(pdat->rst_de >= 0)
		reset_deassert(pdat->rst_de);
	if(pdat->rst_tconlcd >= 0)
		reset_deassert(pdat->rst_tconlcd);
	fb_f133_rgb_init(pdat);

	if(!(dev = register_framebuffer(fb, drv)))
	{
		clk_disable(pdat->clk_de);
		clk_disable(pdat->clk_tconlcd);
		xos_mem_free(pdat->clk_de);
		xos_mem_free(pdat->clk_tconlcd);
		xos_dma_free_noncoherent(pdat->vram[0]);
		xos_dma_free_noncoherent(pdat->vram[1]);
		dirtylist_free(pdat->nl);
		dirtylist_free(pdat->ol);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
		return NULL;
	}
	return dev;
}

static void fb_f133_rgb_remove(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_f133_rgb_pdata_t * pdat = (struct fb_f133_rgb_pdata_t *)fb->priv;

	if(fb)
	{
		unregister_framebuffer(fb);
		clk_disable(pdat->clk_de);
		clk_disable(pdat->clk_tconlcd);
		xos_mem_free(pdat->clk_de);
		xos_mem_free(pdat->clk_tconlcd);
		xos_dma_free_noncoherent(pdat->vram[0]);
		xos_dma_free_noncoherent(pdat->vram[1]);
		dirtylist_free(pdat->nl);
		dirtylist_free(pdat->ol);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
	}
}

static void fb_f133_rgb_suspend(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_f133_rgb_pdata_t * pdat = (struct fb_f133_rgb_pdata_t *)fb->priv;

	pdat->brightness = led_get_brightness(pdat->backlight);
	led_set_brightness(pdat->backlight, 0);
}

static void fb_f133_rgb_resume(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_f133_rgb_pdata_t * pdat = (struct fb_f133_rgb_pdata_t *)fb->priv;

	led_set_brightness(pdat->backlight, pdat->brightness);
}

static struct driver_t fb_f133_rgb = {
	.name		= "fb-f133-rgb",
	.probe		= fb_f133_rgb_probe,
	.remove		= fb_f133_rgb_remove,
	.suspend	= fb_f133_rgb_suspend,
	.resume		= fb_f133_rgb_resume,
};

static void fb_f133_rgb_driver_init(void)
{
	register_driver(&fb_f133_rgb);
}

static void fb_f133_rgb_driver_exit(void)
{
	unregister_driver(&fb_f133_rgb);
}

driver_initcall(fb_f133_rgb_driver_init);
driver_exitcall(fb_f133_rgb_driver_exit);
