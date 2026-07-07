/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limiteration the rights
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

#include <driver/limiter/limiter.h>

static ssize_t limiter_read_status(struct kobj_t * kobj, void * buf, size_t size)
{
	struct limiter_t * limiter = (struct limiter_t *)kobj->priv;
	int status = limiter_get_status(limiter);
	return xos_sprintf(buf, "%d", status);
}

struct limiter_t * search_limiter(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_LIMITER);
	if(!dev)
		return NULL;
	return (struct limiter_t *)dev->priv;
}

struct device_t * register_limiter(struct limiter_t * limiter, struct driver_t * drv)
{
	struct device_t * dev;

	if(!limiter || !limiter->name)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(limiter->name);
	dev->type = DEVICE_TYPE_LIMITER;
	dev->driver = drv;
	dev->priv = limiter;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "status", limiter_read_status, NULL, limiter);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_limiter(struct limiter_t * limiter)
{
	struct device_t * dev;

	if(limiter && limiter->name)
	{
		dev = search_device(limiter->name, DEVICE_TYPE_LIMITER);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

int limiter_get_status(struct limiter_t * limiter)
{
	if(limiter && limiter->get)
		return limiter->get(limiter);
	return 0;
}
