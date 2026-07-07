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

#include <ch347.h>

static struct mutex_t __ch347_lock = { 0 };
static int __ch347_fd = -1;

int ch347_detect(void)
{
	if(__ch347_fd < 0)
	{
		int fd = CH347OpenDevice("/dev/ch34x_pis");
		if(fd < 0)
		{
			char path[64];
			for(int i = 0; i < 10; i++)
			{
				xos_snprintf(path, sizeof(path), "/dev/ch34x_pis%d", i);
				fd = CH347OpenDevice(path);
				if(fd >= 0)
					break;
			}
		}
		if(fd >= 0)
		{
			CHIP_TYPE chip;
			if(CH34x_GetChipType(fd, &chip) && ((chip == CHIP_CH347T) || (chip == CHIP_CH347F)))
			{
				CH34xSetTimeout(fd, 20, 20);
				CH347_OE_Enable(fd);
				xos_mutex_init(&__ch347_lock);
				__ch347_fd = fd;
			}
			else
				CH347CloseDevice(fd);
		}
	}
	return __ch347_fd;
}

int ch347_lock(void)
{
	return xos_mutex_lock(&__ch347_lock);
}

int ch347_unlock(void)
{
	return xos_mutex_unlock(&__ch347_lock);
}

static void ch347_device_init(void)
{
	const char * json = X({
		"gpio-ch347:0": {
			"gpio-base": 1024,
			"interrupt-offset": -1
		},
		"i2c-ch347:0": {
		},
		"spi-ch347:0": {
		}
	});
	probe_device(json, xos_strlen(json));
}
late_initcall(ch347_device_init);
