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

#include <driver/atnet/atnet.h>

struct atnet_t * search_atnet(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_ATNET);
	if(!dev)
		return NULL;
	return (struct atnet_t *)dev->priv;
}

struct atnet_t * search_first_atnet(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_ATNET);
	if(!dev)
		return NULL;
	return (struct atnet_t *)dev->priv;
}

struct device_t * register_atnet(struct atnet_t * atnet, struct driver_t * drv)
{
	struct device_t * dev;

	if(!atnet || !atnet->name)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(atnet->name);
	dev->type = DEVICE_TYPE_ATNET;
	dev->driver = drv;
	dev->priv = atnet;
	dev->kobj = kobj_alloc_directory(dev->name);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_atnet(struct atnet_t * atnet)
{
	struct device_t * dev;

	if(atnet && atnet->name)
	{
		dev = search_device(atnet->name, DEVICE_TYPE_ATNET);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}
