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

enum {
	CST816D_GESTURE_ID			= 0x01,
	CST816D_FINGER_NUM			= 0x02,
	CST816D_XPOS_H				= 0x03,
	CST816D_XPOS_L				= 0x04,
	CST816D_YPOS_H				= 0x05,
	CST816D_YPOS_L				= 0x06,
	CST816D_BPC0H				= 0xB0,
	CST816D_BPC0L				= 0xB1,
	CST816D_BPC1H				= 0xB2,
	CST816D_BPC1L				= 0xB3,
	CST816D_POWER_MODE			= 0xA5,
	CST816D_CHIP_ID				= 0xA7,
	CST816D_PROJ_ID				= 0xA8,
	CST816D_FW_VERSION			= 0xA9,
	CST816D_MOTION_MASK			= 0xEC,
	CST816D_IRQ_PULSE_WIDTH		= 0xED,
	CST816D_NOR_SCAN_PER		= 0xEE,
	CST816D_MOTION_S1_ANGLE		= 0xEF,
	CST816D_LP_SCAN_RAW1H		= 0xF0,
	CST816D_LP_SCAN_RAW1L		= 0xF1,
	CST816D_LP_SCAN_RAW2H		= 0xF2,
	CST816D_LP_SCAN_RAW2L		= 0xF3,
	CST816D_LP_AUTO_WAKEUP_TIME	= 0xF4,
	CST816D_LP_SCAN_TH			= 0xF5,
	CST816D_LP_SCAN_WIN			= 0xF6,
	CST816D_LP_SCAN_FREQ		= 0xF7,
	CST816D_LP_SCAN_I_DAC		= 0xF8,
	CST816D_AUTOSLEEP_TIME		= 0xF9,
	CST816D_IRQ_CTL				= 0xFA,
	CST816D_DEBOUNCE_TIME		= 0xFB,
	CST816D_LONG_PRESS_TIME		= 0xFC,
	CST816D_IOCTL				= 0xFD,
	CST816D_DIS_AUTO_SLEEP		= 0xFE,
};

struct ts_cst816d_pdata_t {
	struct i2c_device_t * dev;
	int irq;
	int sizex, sizey;
	int invertx, inverty;
	int swapxy;
	int x, y;
	int press;
};

static int cst816d_read(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
{
	struct i2c_msg_t msgs[2];

	msgs[0].addr = dev->addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = &reg;

	msgs[1].addr = dev->addr;
	msgs[1].flags = I2C_MODE_RD;
	msgs[1].len = len;
	msgs[1].buf = buf;

	if(i2c_transfer(dev->i2c, msgs, 2) != 2)
		return FALSE;
	return TRUE;
}

static int cst816d_write(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
{
	struct i2c_msg_t msg;
	uint8_t mbuf[256];

	if(len > sizeof(mbuf) - 1)
		len = sizeof(mbuf) - 1;
	mbuf[0] = reg;
	xos_memcpy(&mbuf[1], buf, len);

	msg.addr = dev->addr;
	msg.flags = 0;
	msg.len = len + 1;
	msg.buf = &mbuf[0];

	if(i2c_transfer(dev->i2c, &msg, 1) != 1)
		return FALSE;
	return TRUE;
}

static int cst816d_initial(struct i2c_device_t * dev)
{
	uint8_t id, version;

	if(!cst816d_read(dev, CST816D_CHIP_ID, &id, 1) || (id != 0xb6))
		return FALSE;
	if(!cst816d_read(dev, CST816D_FW_VERSION, &version, 1))
		return FALSE;
	if(!cst816d_write(dev, CST816D_MOTION_MASK, (uint8_t[]){ 0 }, 1))
		return FALSE;
	if(!cst816d_write(dev, CST816D_IRQ_PULSE_WIDTH, (uint8_t[]){ 10 }, 1))
		return FALSE;
	if(!cst816d_write(dev, CST816D_IRQ_CTL, (uint8_t[]){ (1 << 5) | (1 << 6) | (1 << 0) }, 1))
		return FALSE;
	if(!cst816d_write(dev, CST816D_DIS_AUTO_SLEEP, (uint8_t[]){ 0xff }, 1))
		return FALSE;
	LOG("CST816D chip id: 0x%02x, firmware version: 0x%02x\r\n", id, version);
	return TRUE;
}

static void cst816d_interrupt(void * data)
{
	struct input_t * input = (struct input_t *)data;
	struct ts_cst816d_pdata_t * pdat = (struct ts_cst816d_pdata_t *)input->priv;
	uint8_t buf[6];

	disable_irq(pdat->irq);
	if(cst816d_read(pdat->dev, CST816D_GESTURE_ID, buf, 6))
	{
		if(buf[1] == 1)
		{
			int x, y;
			if(pdat->swapxy)
			{
				x = ((buf[4] << 8) | buf[5]) & 0xfff;
				y = ((buf[2] << 8) | buf[3]) & 0xfff;
			}
			else
			{
				x = ((buf[2] << 8) | buf[3]) & 0xfff;
				y = ((buf[4] << 8) | buf[5]) & 0xfff;
			}
			if(pdat->invertx)
				x = pdat->sizex - x;
			if(pdat->inverty)
				y = pdat->sizey - y;
			if(!pdat->press)
			{
				push_event_touch_begin(input, x, y, 0);
				pdat->press = 1;
			}
			else
			{
				push_event_touch_move(input, x, y, 0);
			}
			pdat->x = x;
			pdat->y = y;
		}
		else if(buf[1] == 0)
		{
			if(pdat->press)
			{
				push_event_touch_end(input, pdat->x, pdat->y, 0);
				pdat->press = 0;
			}
		}
	}
	enable_irq(pdat->irq);
}

static int ts_cst816d_ioctl(struct input_t * input, const char * cmd, void * arg)
{
	return -1;
}

static struct device_t * ts_cst816d_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct ts_cst816d_pdata_t * pdat;
	struct input_t * input;
	struct device_t * dev;
	struct i2c_device_t * i2cdev;
	int gpio = dt_read_int(n, "interrupt-gpio", -1);
	int gpiocfg = dt_read_int(n, "interrupt-gpio-config", -1);
	int irq = gpio_to_irq(gpio);
	int rst = dt_read_int(n, "reset-gpio", -1);
	int rstcfg = dt_read_int(n, "reset-gpio-config", -1);

	if(!gpio_is_valid(gpio) || !irq_is_valid(irq))
		return NULL;

	i2cdev = i2c_device_alloc(dt_read_string(n, "i2c-bus", NULL), dt_read_int(n, "slave-address", 0x15), 0);
	if(!i2cdev)
		return NULL;

	if(rst >= 0)
	{
		if(rstcfg >= 0)
			gpio_set_cfg(rst, rstcfg);
		gpio_set_pull(rst, GPIO_PULL_UP);
		gpio_set_direction(rst, GPIO_DIRECTION_OUTPUT);
		gpio_set_value(rst, 0);
		mdelay(10);
		gpio_set_value(rst, 1);
		mdelay(10);
	}
	if(!cst816d_initial(i2cdev))
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	pdat = xos_mem_malloc(sizeof(struct ts_cst816d_pdata_t));
	if(!pdat)
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	input = xos_mem_malloc(sizeof(struct input_t));
	if(!input)
	{
		i2c_device_free(i2cdev);
		xos_mem_free(pdat);
		return NULL;
	}

	xos_memset(pdat, 0, sizeof(struct ts_cst816d_pdata_t));
	pdat->dev = i2cdev;
	pdat->irq = irq;
	pdat->sizex = dt_read_int(n, "max-size-x", 320);
	pdat->sizey = dt_read_int(n, "max-size-y", 170);
	pdat->invertx = dt_read_bool(n, "invert-x", 0);
	pdat->inverty = dt_read_bool(n, "invert-y", 0);
	pdat->swapxy = dt_read_bool(n, "swap-xy", 0);

	input->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	input->ioctl = ts_cst816d_ioctl;
	input->priv = pdat;

	if(gpio >= 0)
	{
		if(gpiocfg >= 0)
			gpio_set_cfg(gpio, gpiocfg);
		gpio_set_pull(gpio, GPIO_PULL_UP);
	}
	request_irq(pdat->irq, cst816d_interrupt, IRQ_TYPE_EDGE_FALLING, input);

	if(!(dev = register_input(input, drv)))
	{
		free_irq(pdat->irq);
		i2c_device_free(pdat->dev);
		free_device_name(input->name);
		xos_mem_free(input->priv);
		xos_mem_free(input);
		return NULL;
	}
	return dev;
}

static void ts_cst816d_remove(struct device_t * dev)
{
	struct input_t * input = (struct input_t *)dev->priv;
	struct ts_cst816d_pdata_t * pdat = (struct ts_cst816d_pdata_t *)input->priv;

	if(input)
	{
		unregister_input(input);
		free_irq(pdat->irq);
		i2c_device_free(pdat->dev);
		free_device_name(input->name);
		xos_mem_free(input->priv);
		xos_mem_free(input);
	}
}

static void ts_cst816d_suspend(struct device_t * dev)
{
}

static void ts_cst816d_resume(struct device_t * dev)
{
}

static struct driver_t ts_cst816d = {
	.name		= "ts-cst816d",
	.probe		= ts_cst816d_probe,
	.remove		= ts_cst816d_remove,
	.suspend	= ts_cst816d_suspend,
	.resume		= ts_cst816d_resume,
};

static void ts_cst816d_driver_init(void)
{
	register_driver(&ts_cst816d);
}

static void ts_cst816d_driver_exit(void)
{
	unregister_driver(&ts_cst816d);
}

driver_initcall(ts_cst816d_driver_init);
driver_exitcall(ts_cst816d_driver_exit);
