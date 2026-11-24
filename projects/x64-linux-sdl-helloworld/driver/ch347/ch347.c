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
static int __ch347_ngpio = 0;

int ch347_detect(void)
{
	if(__ch347_fd < 0)
	{
		CH34X_DEVICE_INFO * infolist;
		uint8_t num = 0;
		int fd = -1;

		infolist = xos_mem_calloc(CH34X_MAX_NUMBER, sizeof(CH34X_DEVICE_INFO));
		if(infolist)
		{
			if(CH34X_GetDeviceInfoList(infolist, &num) == 0)
			{
				for(int i = 0; (i < num) && (fd < 0); i++)
				{
					if((infolist[i].ChipType == CHIP_CH347T_VENDOR) || (infolist[i].ChipType == CHIP_CH347F_VENDOR))
					{
						fd = CH34X_OpenDevice(infolist[i].iIndex);
						if(fd >= 0)
							__ch347_ngpio = infolist[i].MaxGPIOChannel;
					}
				}
			}
			xos_mem_free(infolist);
		}
		if(fd >= 0)
		{
			if(CH34X_SetUSBTimeout(fd, 20, 20) == 0)
			{
				xos_mutex_init(&__ch347_lock);
				__ch347_fd = fd;
			}
			else
				CH34X_CloseDevice(fd);
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
	char json[256];
	int length;

	ch347_detect();
	length = xos_snprintf(json, sizeof(json), X({
		"gpio-ch347:0": {
			"gpio-base": 1024,
			"gpio-count": %d,
			"interrupt-offset": -1
		},
		"i2c-ch347:0": {
		},
		"spi-ch347:0": {
		}
	}), __ch347_ngpio);
	probe_device(json, length);
}
late_initcall(ch347_device_init);
