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

static ssize_t nvmem_read_capacity(struct kobj_t * kobj, void * buf, size_t size)
{
	struct nvmem_t * m = (struct nvmem_t *)kobj->priv;
	return xos_sprintf(buf, "%d", nvmem_capacity(m));
}

struct nvmem_t * search_nvmem(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_NVMEM);
	if(!dev)
		return NULL;
	return (struct nvmem_t *)dev->priv;
}

struct nvmem_t * search_first_nvmem(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_NVMEM);
	if(!dev)
		return NULL;
	return (struct nvmem_t *)dev->priv;
}

struct device_t * register_nvmem(struct nvmem_t * m, struct driver_t * drv)
{
	struct device_t * dev;

	if(!m || !m->name)
		return NULL;

	if(!m->capacity && !m->read)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(m->name);
	dev->type = DEVICE_TYPE_NVMEM;
	dev->driver = drv;
	dev->priv = m;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "capacity", nvmem_read_capacity, NULL, m);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_nvmem(struct nvmem_t * m)
{
	struct device_t * dev;

	if(m && m->name)
	{
		dev = search_device(m->name, DEVICE_TYPE_NVMEM);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

int nvmem_capacity(struct nvmem_t * m)
{
	if(m && m->capacity)
		return m->capacity(m);
	return 0;
}

int nvmem_read(struct nvmem_t * m, void * buf, int offset, int count)
{
	if(m && m->read)
	{
		int capacity = m->capacity(m);
		offset = XCLAMP(offset, 0, capacity);
		count = XCLAMP(count, 0, capacity - offset);
		return m->read(m, buf, offset, count);
	}
	return 0;
}

int nvmem_write(struct nvmem_t * m, void * buf, int offset, int count)
{
	if(m && m->write)
	{
		int capacity = m->capacity(m);
		offset = XCLAMP(offset, 0, capacity);
		count = XCLAMP(count, 0, capacity - offset);
		return m->write(m, buf, offset, count);
	}
	return 0;
}
