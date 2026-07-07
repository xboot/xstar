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

#include <driver/nvmem/nvmem.h>

/*
 * NVMEM OTP - One-Time Programmable Non-Volatile Memory
 *
 * Required properties:
 * - capacity: storage capacity of otp
 *
 * Example:
 *   "nvmem-otp@0x80ffff00": {
 *       "capacity": 256
 *   }
 */

struct nvmem_otp_pdata_t {
	io_addr_t addr;
	int capacity;
};

static int nvmem_otp_capacity(struct nvmem_t * m)
{
	struct nvmem_otp_pdata_t * pdat = (struct nvmem_otp_pdata_t *)m->priv;
	return pdat->capacity;
}

static int nvmem_otp_read(struct nvmem_t * m, void * buf, int offset, int count)
{
	struct nvmem_otp_pdata_t * pdat = (struct nvmem_otp_pdata_t *)m->priv;
	uint8_t * p = buf;
	int i;

	for(i = 0; i < count; i++)
		p[i] = xos_io_read8(pdat->addr + offset + i);
	return i;
}

static struct device_t * nvmem_otp_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct nvmem_otp_pdata_t * pdat;
	struct nvmem_t * m;
	struct device_t * dev;
	io_addr_t addr = dt_read_address(n);
	int capacity = dt_read_int(n, "capacity", 0);

	if(capacity <= 0)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct nvmem_otp_pdata_t));
	if(!pdat)
		return NULL;

	m = xos_mem_malloc(sizeof(struct nvmem_t));
	if(!m)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->addr = addr;
	pdat->capacity = capacity;

	m->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	m->capacity = nvmem_otp_capacity;
	m->read = nvmem_otp_read;
	m->write = NULL;
	m->priv = pdat;

	if(!(dev = register_nvmem(m, drv)))
	{
		free_device_name(m->name);
		xos_mem_free(m->priv);
		xos_mem_free(m);
		return NULL;
	}
	return dev;
}

static void nvmem_otp_remove(struct device_t * dev)
{
	struct nvmem_t * m = (struct nvmem_t *)dev->priv;

	if(m)
	{
		unregister_nvmem(m);
		free_device_name(m->name);
		xos_mem_free(m->priv);
		xos_mem_free(m);
	}
}

static void nvmem_otp_suspend(struct device_t * dev)
{
}

static void nvmem_otp_resume(struct device_t * dev)
{
}

static struct driver_t nvmem_otp = {
	.name		= "nvmem-otp",
	.probe		= nvmem_otp_probe,
	.remove		= nvmem_otp_remove,
	.suspend	= nvmem_otp_suspend,
	.resume		= nvmem_otp_resume,
};

static void nvmem_otp_driver_init(void)
{
	register_driver(&nvmem_otp);
}

static void nvmem_otp_driver_exit(void)
{
	unregister_driver(&nvmem_otp);
}

driver_initcall(nvmem_otp_driver_init);
driver_exitcall(nvmem_otp_driver_exit);
