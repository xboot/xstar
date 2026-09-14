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
 * PWM-F101 - 4 channels pwm output on F101
 *
 * Required properties:
 * - channel: pwm channel number from 0 to 3
 *
 * Optional properties:
 * - clock-name: pwm bus gate clock name (default: "pwm")
 * - reset: pwm bus reset id (default: -1, 336 on f101)
 * - gpio: gpio pin for pwm output, -1 for none
 * - gpio-config: gpio mux config for the pwm output pin
 *
 * Example device tree node:
 *   "pwm-f101:0@0x02000c00": {
 *       "clock-name": "pwm",
 *       "reset": 336,
 *       "channel": 0,
 *       "gpio": 32,
 *       "gpio-config": 7
 *   }
 */

enum {
	PWM_PIER		= 0x0000,
	PWM_PISR		= 0x0004,
	PWM_CIER		= 0x0010,
	PWM_CISR		= 0x0014,
	PWM_PCCR01		= 0x0020,
	PWM_PCCR23		= 0x0024,
	PWM_PCGR		= 0x0040,
	PWM_PDZCR01		= 0x0060,
	PWM_PDZCR23		= 0x0064,
	PWM_PER			= 0x0080,
	PWM_CER			= 0x00c0,
	PWM_PCR_BASE	= 0x0100,
	PWM_PPR_BASE	= 0x0104,
	PWM_PCNTR_BASE	= 0x0108,
	PWM_CHN_OFFSET	= 0x0020,
};

#define PWM_CLK_SRC_OSC24M		(0x0 << 7)
#define PWM_CLK_SRC_APB0		(0x1 << 7)
#define PWM_CLK_SRC_MASK		(0x3 << 7)
#define PWM_CLK_DIV_M_MASK		(0xf << 0)
#define PWM_PRESCAL_K_MASK		(0xff << 0)
#define PWM_MODE_CYCLE			(0x0 << 9)
#define PWM_MODE_PULSE			(0x1 << 9)
#define PWM_ACT_STA_HIGH		(0x1 << 8)
#define PWM_ACT_STA_LOW			(0x0 << 8)
#define PWM_ACT_STA_MASK		(0x1 << 8)
#define PWM_ENTIRE_CYCLE(n)		(((n) & 0xffff) << 16)
#define PWM_ACT_CYCLE(n)		(((n) & 0xffff) << 0)

struct pwm_f101_pdata_t {
	io_addr_t addr;
	char * clk;
	int reset;
	int channel;
};

static void pwm_f101_config(struct pwm_t * pwm, int duty, int period, int polarity)
{
	struct pwm_f101_pdata_t * pdat = (struct pwm_f101_pdata_t *)pwm->priv;
	io_addr_t pccr = pdat->addr + ((pdat->channel < 2) ? PWM_PCCR01 : PWM_PCCR23);
	io_addr_t pcr = pdat->addr + PWM_PCR_BASE + pdat->channel * PWM_CHN_OFFSET;
	io_addr_t ppr = pdat->addr + PWM_PPR_BASE + pdat->channel * PWM_CHN_OFFSET;
	uint32_t div_m, prescal, nentire, act, val;

	uint64_t rate = clk_get_rate(pdat->clk);
	uint64_t entire = (rate * (uint64_t)period + 500000000ULL) / 1000000000ULL;
	if(entire < 2)
		entire = 2;

	int found = 0;
	for(int i = 0; (i <= 8) && !found; i++)
	{
		for(int j = 0; j < 256; j++)
		{
			if(entire / ((1ULL << i) * (j + 1)) <= 65536)
			{
				div_m = i;
				prescal = j;
				found = 1;
				break;
			}
		}
	}
	if(!found)
	{
		div_m = 8;
		prescal = 255;
	}

	uint64_t div = (1ULL << div_m) * (prescal + 1);
	nentire = (uint32_t)(entire / div);
	if(nentire == 0)
		nentire = 1;
	act = (uint32_t)(((uint64_t)nentire * duty + period / 2) / period);
	if(act > 65535)
		act = 65535;

	val = xos_io_read32(pccr);
	val &= ~(PWM_CLK_SRC_MASK | PWM_CLK_DIV_M_MASK);
	val |= PWM_CLK_SRC_APB0;
	val |= div_m;
	xos_io_write32(pccr, val);

	val = xos_io_read32(pdat->addr + PWM_PCGR);
	val |= (1 << pdat->channel);
	val &= ~(1 << (16 + pdat->channel));
	xos_io_write32(pdat->addr + PWM_PCGR, val);

	val = xos_io_read32(pcr);
	val &= ~(PWM_PRESCAL_K_MASK | PWM_ACT_STA_MASK | (0x1 << 9) | (0x1 << 10));
	val |= prescal;
	val |= polarity ? PWM_ACT_STA_HIGH : PWM_ACT_STA_LOW;
	xos_io_write32(pcr, val);

	xos_io_write32(ppr, PWM_ENTIRE_CYCLE(nentire - 1) | PWM_ACT_CYCLE(act));
}

static void pwm_f101_enable(struct pwm_t * pwm)
{
	struct pwm_f101_pdata_t * pdat = (struct pwm_f101_pdata_t *)pwm->priv;
	uint32_t val;

	val = xos_io_read32(pdat->addr + PWM_PCGR);
	val |= (1 << pdat->channel);
	xos_io_write32(pdat->addr + PWM_PCGR, val);

	val = xos_io_read32(pdat->addr + PWM_PER);
	val |= (1 << pdat->channel);
	xos_io_write32(pdat->addr + PWM_PER, val);
}

static void pwm_f101_disable(struct pwm_t * pwm)
{
	struct pwm_f101_pdata_t * pdat = (struct pwm_f101_pdata_t *)pwm->priv;
	uint32_t val;

	val = xos_io_read32(pdat->addr + PWM_PER);
	val &= ~(1 << pdat->channel);
	xos_io_write32(pdat->addr + PWM_PER, val);

	val = xos_io_read32(pdat->addr + PWM_PCGR);
	val &= ~(1 << pdat->channel);
	xos_io_write32(pdat->addr + PWM_PCGR, val);
}

static struct device_t * pwm_f101_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct pwm_f101_pdata_t * pdat;
	struct pwm_t * pwm;
	struct device_t * dev;
	char * clk = dt_read_string(n, "clock-name", "pwm");
	int channel = dt_read_int(n, "channel", -1);
	int gpio = dt_read_int(n, "gpio", -1);

	if((channel < 0) || (channel > 3))
		return NULL;

	if(!search_clk(clk))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct pwm_f101_pdata_t));
	if(!pdat)
		return NULL;

	pwm = xos_mem_malloc(sizeof(struct pwm_t));
	if(!pwm)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->addr = dt_read_address(n);
	pdat->clk = xos_strdup(clk);
	pdat->reset = dt_read_int(n, "reset", -1);
	pdat->channel = channel;

	pwm->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	pwm->config = pwm_f101_config;
	pwm->enable = pwm_f101_enable;
	pwm->disable = pwm_f101_disable;
	pwm->priv = pdat;

	clk_enable(pdat->clk);
	if(pdat->reset >= 0)
		reset_deassert(pdat->reset);
	if((gpio >= 0) && gpio_is_valid(gpio))
		gpio_set_cfg(gpio, dt_read_int(n, "gpio-config", -1));

	if(!(dev = register_pwm(pwm, drv)))
	{
		clk_disable(pdat->clk);
		free_device_name(pwm->name);
		xos_mem_free(pdat->clk);
		xos_mem_free(pwm->priv);
		xos_mem_free(pwm);
		return NULL;
	}
	return dev;
}

static void pwm_f101_remove(struct device_t * dev)
{
	struct pwm_t * pwm = (struct pwm_t *)dev->priv;
	struct pwm_f101_pdata_t * pdat = (struct pwm_f101_pdata_t *)pwm->priv;

	if(pwm)
	{
		unregister_pwm(pwm);
		clk_disable(pdat->clk);
		free_device_name(pwm->name);
		xos_mem_free(pdat->clk);
		xos_mem_free(pwm->priv);
		xos_mem_free(pwm);
	}
}

static void pwm_f101_suspend(struct device_t * dev)
{
}

static void pwm_f101_resume(struct device_t * dev)
{
}

static struct driver_t pwm_f101 = {
	.name		= "pwm-f101",
	.probe		= pwm_f101_probe,
	.remove		= pwm_f101_remove,
	.suspend	= pwm_f101_suspend,
	.resume		= pwm_f101_resume,
};

static void pwm_f101_driver_init(void)
{
	register_driver(&pwm_f101);
}

static void pwm_f101_driver_exit(void)
{
	unregister_driver(&pwm_f101);
}

driver_initcall(pwm_f101_driver_init);
driver_exitcall(pwm_f101_driver_exit);
