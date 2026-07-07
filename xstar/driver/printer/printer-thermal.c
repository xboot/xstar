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
#include <driver/printer/printer.h>

/*
 * Printer - Portable Thermal Printer
 *
 * Required properties:
 * - spi-bus: Name of the SPI bus the device is attached to
 *
 * Optional properties:
 * - clk-gpio: Clock pin in gpio mode (unused if spi-bus is provided)
 * - di-gpio: Data pin in gpio mode (unused if spi-bus is provided)
 *
 * Example device tree node:
 * "printer-thermal:0": {
 *     "spi-bus": "spi-ch347.0",
 *     "thermometer": null,
 *     "chip-select": 0,
 *     "type": 0,
 *     "mode": 0,
 *     "speed": 7500000,
 *     "clk-gpio": -1,
 *     "clk-gpio-config": -1,
 *     "di-gpio": -1,
 *     "di-gpio-config": -1,
 *     "stb-gpio": 512,
 *     "stb-gpio-config": 1,
 *     "lat-gpio": 515,
 *     "lat-gpio-config": 1,
 *     "pa-gpio": 516,
 *     "pa-gpio-config": 1,
 *     "na-gpio": 517,
 *     "na-gpio-config": 1,
 *     "pb-gpio": 518,
 *     "pb-gpio-config": 1,
 *     "nb-gpio": 519,
 *     "nb-gpio-config": 1,
 *     "paper-detect-gpio": -1,
 *     "paper-detect-gpio-config": -1,
 *     "paper-detect-active-low": 0,
 *     "dots-per-line": 384,
 *     "physical-width": 48,
 *     "step-per-pixel": 2,
 *     "step-time-us": 1000,
 *     "heating-base-time-us": 2000,
 *     "heating-min-time-us": 1000,
 *     "heating-max-time-us": 10000,
 *     "heating-coeff-ko": 2.0,
 *     "heating-coeff-kn": -0.2
 * }
 */

struct printer_thermal_pdata_t {
	struct spi_device_t * spidev;
	struct thermometer_t * thermometer;
	struct gpiodesc_t * clk;
	struct gpiodesc_t * di;
	struct gpiodesc_t * stb;
	struct gpiodesc_t * lat;
	struct gpiodesc_t * pa;
	struct gpiodesc_t * na;
	struct gpiodesc_t * pb;
	struct gpiodesc_t * nb;
	struct gpiodesc_t * paper;
	int paper_active_low;
	int paper_status;
	int dpl;
	int pwidth;
	int step_per_pixel;
	int step_time;
	int heating_time_base;
	int heating_time_min;
	int heating_time_max;
	int stepidx;
	int odots, ndots;
	float density;
	float ko, kn;
};

static void printer_motor_step(struct printer_thermal_pdata_t * pdat, int step)
{
	if(step != 0)
	{
		int cnt = xos_abs(step);
		while(cnt-- > 0)
		{
			if(step > 0)
				pdat->stepidx = (pdat->stepidx + 4 + 1) & 0x3;
			else
				pdat->stepidx = (pdat->stepidx + 4 - 1) & 0x3;
			switch(pdat->stepidx)
			{
			case 0:
				gpiod_set_value(pdat->pa, 1);
				gpiod_set_value(pdat->na, 0);
				gpiod_set_value(pdat->pb, 0);
				gpiod_set_value(pdat->nb, 1);
				break;
			case 1:
				gpiod_set_value(pdat->pa, 0);
				gpiod_set_value(pdat->na, 1);
				gpiod_set_value(pdat->pb, 0);
				gpiod_set_value(pdat->nb, 1);
				break;
			case 2:
				gpiod_set_value(pdat->pa, 0);
				gpiod_set_value(pdat->na, 1);
				gpiod_set_value(pdat->pb, 1);
				gpiod_set_value(pdat->nb, 0);
				break;
			case 3:
				gpiod_set_value(pdat->pa, 1);
				gpiod_set_value(pdat->na, 0);
				gpiod_set_value(pdat->pb, 1);
				gpiod_set_value(pdat->nb, 0);
				break;
			default:
				break;
			}
			udelay(pdat->step_time);
		}
	}
}

static void printer_motor_stop(struct printer_thermal_pdata_t * pdat)
{
	gpiod_set_value(pdat->pa, 0);
	gpiod_set_value(pdat->na, 0);
	gpiod_set_value(pdat->pb, 0);
	gpiod_set_value(pdat->nb, 0);
}

static inline int printer_heating_time(struct printer_thermal_pdata_t * pdat)
{
	int us = (pdat->heating_time_base * (0.5f + pdat->density)) * (1.0f + pdat->kn * pdat->ndots / pdat->dpl + pdat->ko * pdat->odots / pdat->dpl);
	return XCLAMP(us, pdat->heating_time_min, pdat->heating_time_max);
}

static int printer_paper_detect(struct printer_t * p)
{
	struct printer_thermal_pdata_t * pdat = (struct printer_thermal_pdata_t *)p->priv;
	int status = 1;

	if(pdat->paper)
	{
		if(gpiod_get_value(pdat->paper))
			status = pdat->paper_active_low ? 0 : 1;
		else
			status = pdat->paper_active_low ? 1 : 0;
		if(pdat->paper_status != status)
		{
			pdat->paper_status = status;
			if(status)
				psub_publish("printer.paper.ready", p);
			else
				psub_publish("printer.paper.out", p);
		}
	}
	return status;
}

static void printer_thermal_print(struct printer_t * p, unsigned char * buf, unsigned int len)
{
	struct printer_thermal_pdata_t * pdat = (struct printer_thermal_pdata_t *)p->priv;

	if(printer_paper_detect(p))
	{
		unsigned char line[pdat->dpl];
		int dots = 0;

		len = XMIN((int)len, pdat->dpl);
		for(int i = 0; i < len; i++)
		{
			line[i] = buf[i];
			if(line[i] > 127)
				dots++;
		}
		for(int i = len; i < pdat->dpl; i++)
			line[i] = 0;

		pdat->odots = pdat->ndots;
		pdat->ndots = dots;
		if(pdat->ndots > 0)
		{
			if(pdat->spidev)
			{
				unsigned char txbuf[pdat->dpl >> 3];
				int pending, pos = 0;
				do {
					xos_memset(txbuf, 0, sizeof(txbuf));
					pending = 0;
					for(int i = pos, n = 0; i < pdat->dpl; i++)
					{
						int byteidx = i >> 3;
						int bitpos = 7 - (i & 0x7);
						if(line[i] > 127)
						{
							txbuf[byteidx] |= (1 << bitpos);
							if(++n >= 128)
							{
								pending = 1;
								pos = i + 1;
								break;
							}
						}
					}
					spi_device_write_then_read(pdat->spidev, txbuf, sizeof(txbuf), 0, 0);
					gpiod_set_value(pdat->lat, 1);
					gpiod_set_value(pdat->lat, 0);
					gpiod_set_value(pdat->lat, 1);
					gpiod_set_value(pdat->stb, 1);
					udelay(printer_heating_time(pdat));
					gpiod_set_value(pdat->stb, 0);
				} while((pos < pdat->dpl) && pending);
			}
			else
			{
				for(int i = 0; i < pdat->dpl; i++)
				{
					gpiod_set_value(pdat->clk, 0);
					gpiod_set_value(pdat->di, (line[i] > 127) ? 1 : 0);
					gpiod_set_value(pdat->clk, 1);
				}
				gpiod_set_value(pdat->clk, 0);
				gpiod_set_value(pdat->lat, 1);
				gpiod_set_value(pdat->lat, 0);
				gpiod_set_value(pdat->lat, 1);
				gpiod_set_value(pdat->stb, 1);
				udelay(printer_heating_time(pdat));
				gpiod_set_value(pdat->stb, 0);
			}
		}
		printer_motor_step(pdat, pdat->step_per_pixel);
	}
}

static void printer_thermal_feed(struct printer_t * p, int lines)
{
	struct printer_thermal_pdata_t * pdat = (struct printer_thermal_pdata_t *)p->priv;

	if(lines != 0)
	{
		if(printer_paper_detect(p))
			printer_motor_step(pdat, pdat->step_per_pixel * lines);
		pdat->odots = 0;
		pdat->ndots = 0;
	}
}

static void printer_thermal_cut(struct printer_t * p)
{
	if(printer_paper_detect(p))
	{
	}
}

static void printer_thermal_standby(struct printer_t * p)
{
	struct printer_thermal_pdata_t * pdat = (struct printer_thermal_pdata_t *)p->priv;
	printer_motor_stop(pdat);
}

static int printer_thermal_ioctl(struct printer_t * p, const char * cmd, void * arg)
{
	struct printer_thermal_pdata_t * pdat = (struct printer_thermal_pdata_t *)p->priv;

	switch(shash(cmd))
	{
	case 0x5b4b0083: /* "printer-get-dpl" */
		if(arg)
		{
			((int *)arg)[0] = pdat->dpl;
			return 0;
		}
		break;
	case 0x5b4b0080: /* "printer-get-dpi" */
		if(arg)
		{
			((int *)arg)[0] = (int)(25.4f * pdat->dpl / pdat->pwidth);
			return 0;
		}
		break;
	case 0x5ade381b: /* "printer-get-paper" */
		if(arg)
		{
			((int *)arg)[0] = printer_paper_detect(p);
			return 0;
		}
		break;
	case 0x7c5c7411: /* "printer-get-temperature" */
		if(arg)
		{
			if(!thermometer_get(pdat->thermometer, (int *)arg))
				((int *)arg)[0] = 25 * 1000;
			return 0;
		}
		break;
	case 0xf8c9d9c3: /* "printer-get-density" */
		if(arg)
		{
			((float *)arg)[0] = pdat->density;
			return 0;
		}
		break;
	case 0x392258cf: /* "printer-set-density" */
		if(arg)
		{
			pdat->density = XCLAMP(((float *)arg)[0], 0.0f, 1.0f);
			return 0;
		}
		break;
	default:
		break;
	}
	return -1;
}

static struct device_t * printer_thermal_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct printer_thermal_pdata_t * pdat;
	struct printer_t * p;
	struct device_t * dev;
	struct spi_device_t * spidev = NULL;

	int clk = dt_read_int(n, "clk-gpio", -1);
	int di = dt_read_int(n, "di-gpio", -1);
	int stb = dt_read_int(n, "stb-gpio", -1);
	int lat = dt_read_int(n, "lat-gpio", -1);
	int pa = dt_read_int(n, "pa-gpio", -1);
	int na = dt_read_int(n, "na-gpio", -1);
	int pb = dt_read_int(n, "pb-gpio", -1);
	int nb = dt_read_int(n, "nb-gpio", -1);
	int cfg;

	if(!gpio_is_valid(stb) || !gpio_is_valid(lat) || !gpio_is_valid(pa) || !gpio_is_valid(na) || !gpio_is_valid(pb) || !gpio_is_valid(nb))
		return NULL;

	if(search_spi(dt_read_string(n, "spi-bus", NULL)))
		spidev = spi_device_alloc(dt_read_string(n, "spi-bus", NULL), dt_read_int(n, "chip-select", 0), dt_read_int(n, "type", 0), dt_read_int(n, "mode", 0), 8, dt_read_int(n, "speed", 0));
	else
	{
		if(!gpio_is_valid(clk) || !gpio_is_valid(di))
			return NULL;
	}

	pdat = xos_mem_malloc(sizeof(struct printer_thermal_pdata_t));
	if(!pdat)
	{
		if(spidev)
			spi_device_free(spidev);
		return NULL;
	}

	p = xos_mem_malloc(sizeof(struct printer_t));
	if(!p)
	{
		if(spidev)
			spi_device_free(spidev);
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->spidev = spidev;
	pdat->thermometer = search_thermometer(dt_read_string(n, "thermometer", NULL));
	pdat->clk = gpiod_alloc(clk);
	pdat->di = gpiod_alloc(di);
	pdat->stb = gpiod_alloc(stb);
	pdat->lat = gpiod_alloc(lat);
	pdat->pa = gpiod_alloc(pa);
	pdat->na = gpiod_alloc(na);
	pdat->pb = gpiod_alloc(pb);
	pdat->nb = gpiod_alloc(nb);
	pdat->paper = gpiod_alloc(dt_read_int(n, "paper-detect-gpio", -1));
	pdat->paper_active_low = dt_read_bool(n, "paper-detect-active-low", 0);
	pdat->paper_status = 1;
	pdat->dpl = dt_read_int(n, "dots-per-line", 384);
	pdat->pwidth = dt_read_int(n, "physical-width", 48);
	pdat->step_per_pixel = dt_read_int(n, "step-per-pixel", 2);
	pdat->step_time = dt_read_int(n, "step-time-us", 1000);
	pdat->heating_time_base = dt_read_int(n, "heating-base-time-us", 2000);
	pdat->heating_time_min = dt_read_int(n, "heating-min-time-us", 1000);
	pdat->heating_time_max = dt_read_int(n, "heating-max-time-us", 10000);
	pdat->stepidx = 0;
	pdat->odots = 0;
	pdat->ndots = 0;
	pdat->density = 0.5f;
	pdat->ko = (float)dt_read_double(n, "heating-coeff-ko", 2.0f);
	pdat->kn = (float)dt_read_double(n, "heating-coeff-kn", -0.2f);

	p->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	p->print = printer_thermal_print;
	p->feed = printer_thermal_feed;
	p->cut = printer_thermal_cut;
	p->standby = printer_thermal_standby;
	p->ioctl = printer_thermal_ioctl;
	p->priv = pdat;

	if(pdat->clk)
	{
		cfg = dt_read_int(n, "clk-gpio-config", -1);
		if(cfg >= 0)
			gpiod_set_cfg(pdat->clk, cfg);
		gpiod_set_pull(pdat->clk, GPIO_PULL_UP);
		gpiod_set_direction(pdat->clk, GPIO_DIRECTION_OUTPUT);
		gpiod_set_value(pdat->clk, 0);
	}
	if(pdat->di)
	{
		cfg = dt_read_int(n, "di-gpio-config", -1);
		if(cfg >= 0)
			gpiod_set_cfg(pdat->di, cfg);
		gpiod_set_pull(pdat->di, GPIO_PULL_UP);
		gpiod_set_direction(pdat->di, GPIO_DIRECTION_OUTPUT);
		gpiod_set_value(pdat->di, 0);
	}
	if(pdat->stb)
	{
		cfg = dt_read_int(n, "stb-gpio-config", -1);
		if(cfg >= 0)
			gpiod_set_cfg(pdat->stb, cfg);
		gpiod_set_pull(pdat->stb, GPIO_PULL_UP);
		gpiod_set_direction(pdat->stb, GPIO_DIRECTION_OUTPUT);
		gpiod_set_value(pdat->stb, 0);
	}
	if(pdat->lat)
	{
		cfg = dt_read_int(n, "lat-gpio-config", -1);
		if(cfg >= 0)
			gpiod_set_cfg(pdat->lat, cfg);
		gpiod_set_pull(pdat->lat, GPIO_PULL_UP);
		gpiod_set_direction(pdat->lat, GPIO_DIRECTION_OUTPUT);
		gpiod_set_value(pdat->lat, 1);
	}
	if(pdat->pa)
	{
		cfg = dt_read_int(n, "pa-gpio-config", -1);
		if(cfg >= 0)
			gpiod_set_cfg(pdat->pa, cfg);
		gpiod_set_pull(pdat->pa, GPIO_PULL_UP);
		gpiod_set_direction(pdat->pa, GPIO_DIRECTION_OUTPUT);
		gpiod_set_value(pdat->pa, 0);
	}
	if(pdat->na)
	{
		cfg = dt_read_int(n, "na-gpio-config", -1);
		if(cfg >= 0)
			gpiod_set_cfg(pdat->na, cfg);
		gpiod_set_pull(pdat->na, GPIO_PULL_UP);
		gpiod_set_direction(pdat->na, GPIO_DIRECTION_OUTPUT);
		gpiod_set_value(pdat->na, 0);
	}
	if(pdat->pb)
	{
		cfg = dt_read_int(n, "pb-gpio-config", -1);
		if(cfg >= 0)
			gpiod_set_cfg(pdat->pb, cfg);
		gpiod_set_pull(pdat->pb, GPIO_PULL_UP);
		gpiod_set_direction(pdat->pb, GPIO_DIRECTION_OUTPUT);
		gpiod_set_value(pdat->pb, 0);
	}
	if(pdat->nb)
	{
		cfg = dt_read_int(n, "nb-gpio-config", -1);
		if(cfg >= 0)
			gpiod_set_cfg(pdat->nb, cfg);
		gpiod_set_pull(pdat->nb, GPIO_PULL_UP);
		gpiod_set_direction(pdat->nb, GPIO_DIRECTION_OUTPUT);
		gpiod_set_value(pdat->nb, 0);
	}
	if(pdat->paper)
	{
		cfg = dt_read_int(n, "paper-detect-gpio-config", -1);
		if(cfg >= 0)
			gpiod_set_cfg(pdat->paper, cfg);
		gpiod_set_pull(pdat->paper, pdat->paper_active_low ? GPIO_PULL_UP : GPIO_PULL_DOWN);
		gpiod_set_direction(pdat->paper, GPIO_DIRECTION_INPUT);
	}

	if(!(dev = register_printer(p, drv)))
	{
		if(pdat->spidev)
			spi_device_free(pdat->spidev);
		if(pdat->clk)
			gpiod_free(pdat->clk);
		if(pdat->di)
			gpiod_free(pdat->di);
		if(pdat->stb)
			gpiod_free(pdat->stb);
		if(pdat->lat)
			gpiod_free(pdat->lat);
		if(pdat->pa)
			gpiod_free(pdat->pa);
		if(pdat->na)
			gpiod_free(pdat->na);
		if(pdat->pb)
			gpiod_free(pdat->pb);
		if(pdat->nb)
			gpiod_free(pdat->nb);
		if(pdat->paper)
			gpiod_free(pdat->paper);
		free_device_name(p->name);
		xos_mem_free(p->priv);
		xos_mem_free(p);
		return NULL;
	}
	return dev;
}

static void printer_thermal_remove(struct device_t * dev)
{
	struct printer_t * p = (struct printer_t *)dev->priv;
	struct printer_thermal_pdata_t * pdat = (struct printer_thermal_pdata_t *)p->priv;

	if(p)
	{
		unregister_printer(p);
		if(pdat->spidev)
			spi_device_free(pdat->spidev);
		if(pdat->clk)
			gpiod_free(pdat->clk);
		if(pdat->di)
			gpiod_free(pdat->di);
		if(pdat->stb)
			gpiod_free(pdat->stb);
		if(pdat->lat)
			gpiod_free(pdat->lat);
		if(pdat->pa)
			gpiod_free(pdat->pa);
		if(pdat->na)
			gpiod_free(pdat->na);
		if(pdat->pb)
			gpiod_free(pdat->pb);
		if(pdat->nb)
			gpiod_free(pdat->nb);
		if(pdat->paper)
			gpiod_free(pdat->paper);
		free_device_name(p->name);
		xos_mem_free(p->priv);
		xos_mem_free(p);
	}
}

static void printer_thermal_suspend(struct device_t * dev)
{
}

static void printer_thermal_resume(struct device_t * dev)
{
}

static struct driver_t printer_thermal = {
	.name		= "printer-thermal",
	.probe		= printer_thermal_probe,
	.remove		= printer_thermal_remove,
	.suspend	= printer_thermal_suspend,
	.resume		= printer_thermal_resume,
};

static void printer_thermal_driver_init(void)
{
	register_driver(&printer_thermal);
}

static void printer_thermal_driver_exit(void)
{
	unregister_driver(&printer_thermal);
}

driver_initcall(printer_thermal_driver_init);
driver_exitcall(printer_thermal_driver_exit);
